#include "frontend/handlers/query_internal.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "googlesql/public/error_helpers.h"
#include "googlesql/public/error_location.pb.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

constexpr absl::string_view kGatewayPositionalParameterPrefix = "__pos_";

int GatewayPositionalParameterIndex(absl::string_view name) {
  if (!absl::StartsWith(name, kGatewayPositionalParameterPrefix)) {
    return -1;
  }
  name.remove_prefix(kGatewayPositionalParameterPrefix.size());
  int idx = -1;
  if (!absl::SimpleAtoi(name, &idx) || idx < 0) return -1;
  return idx;
}

}  // namespace

namespace internal {

::grpc::Status AnalyzeStatusToGrpc(const absl::Status& status) {
  if (status.ok()) return ::grpc::Status::OK;
  ::grpc::StatusCode code = ::grpc::StatusCode::INTERNAL;
  switch (status.code()) {
    case absl::StatusCode::kInvalidArgument:
      code = ::grpc::StatusCode::INVALID_ARGUMENT;
      break;
    case absl::StatusCode::kNotFound:
      code = ::grpc::StatusCode::NOT_FOUND;
      break;
    case absl::StatusCode::kFailedPrecondition:
      code = ::grpc::StatusCode::FAILED_PRECONDITION;
      break;
    case absl::StatusCode::kPermissionDenied:
      code = ::grpc::StatusCode::PERMISSION_DENIED;
      break;
    case absl::StatusCode::kUnimplemented:
      code = ::grpc::StatusCode::UNIMPLEMENTED;
      break;
    default:
      code = ::grpc::StatusCode::INTERNAL;
      break;
  }
  std::string message(status.message());
  ::googlesql::ErrorLocation location;
  if (::googlesql::GetErrorLocation(status, &location)) {
    message =
        absl::StrCat(location.line(), ":", location.column(), ": ", message);
  }
  return ::grpc::Status(code, message);
}

// Builds an AnalyzerOptions configured the way BigQuery uses
// GoogleSQL: external product mode (NUMERIC / BIGNUMERIC /
// BIGNUMERIC dialects rather than internal scalar names),
// maximum language features (all GoogleSQL surface allowed since
// BigQuery itself enables everything), strict resolution mode.
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeaturesForDevelopment();
  language.EnableLanguageFeature(::googlesql::FEATURE_WITH_EXPRESSION);
  language.EnableLanguageFeature(::googlesql::FEATURE_MATCH_RECOGNIZE);
  language.EnableLanguageFeature(
      ::googlesql::FEATURE_STRATIFIED_RESERVOIR_TABLESAMPLE);
  language.EnableLanguageFeature(::googlesql::FEATURE_KLL_WEIGHTS);
  language.EnableLanguageFeature(::googlesql::FEATURE_CREATE_TABLE_CLONE);
  language.EnableLanguageFeature(::googlesql::FEATURE_CREATE_SNAPSHOT_TABLE);
  language.EnableLanguageFeature(::googlesql::FEATURE_CLONE_DATA);
  language.EnableLanguageFeature(::googlesql::FEATURE_REMOTE_MODEL);
  language.EnableLanguageFeature(::googlesql::FEATURE_ENABLE_MEASURES);
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  // Without this opt-in the analyzer rejects every non-SELECT
  // statement kind in `Prepare()` with a generic
  // "Statement not supported" error. The DML classifier needs
  // INSERT/UPDATE/DELETE/MERGE to flow through to the classifier in
  // `StreamQueryResults` so the handler can return UNIMPLEMENTED
  // (or run INSERT) instead of a misleading INVALID_ARGUMENT.
  language.SetSupportsAllStatementKinds();
  ::googlesql::AnalyzerOptions options(language);
  // Single-line error messages so the gRPC error string stays
  // one-line-friendly. The `attach_error_location_payload` flag is
  // what tells the analyzer to leave the ErrorLocation payload on
  // the returned `absl::Status`; without it `GetErrorLocation`
  // returns false even on parse errors.
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  // Keep the route classifier aligned with `LocalCoordinatorEngine`:
  // raw `ResolvedPivotScan` / `ResolvedUnpivotScan` nodes disposition
  // to `duckdb_rewrite` (see `MakeCoordinatorAnalyzerOptions`).
  options.disable_rewrite(::googlesql::REWRITE_PIVOT);
  options.disable_rewrite(::googlesql::REWRITE_UNPIVOT);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

bool NeedsScriptStatementLoop(absl::string_view sql) {
  if (!absl::StrContains(sql, ";")) {
    return false;
  }
  if (absl::StrContains(sql, "DECLARE") || absl::StrContains(sql, "CALL ") ||
      absl::StrContains(sql, "CREATE CONSTANT") ||
      absl::StrContains(sql, "BEGIN")) {
    return true;
  }
  return absl::StrContains(sql, " SET ") || absl::StrContains(sql, "\nSET ") ||
         absl::StartsWithIgnoreCase(absl::StripAsciiWhitespace(sql), "SET ");
}

// BigQuery REST query results render FLOAT64 cells using the same
// decimal string rules as bench/runner/bqValueToString (the golden
// baseline was captured from the Go client's float64 values, not the
// REST "2.0" integer-float convention).
std::string FormatQueryResultFloat64(double v) {
  if (std::isnan(v)) return "NaN";
  if (std::isinf(v)) return v > 0 ? "Infinity" : "-Infinity";
  std::array<char, 64> buf{};
  const auto [ptr, ec] = std::to_chars(
      buf.data(), buf.data() + buf.size(), v, std::chars_format::fixed);
  if (ec != std::errc()) {
    return absl::StrCat(v);
  }
  std::string s(buf.data(), static_cast<size_t>(ptr - buf.data()));
  if (const size_t dot = s.find('.'); dot != std::string::npos) {
    while (s.size() > dot + 1 && s.back() == '0') {
      s.pop_back();
    }
    if (s.back() == '.') {
      s.pop_back();
    }
  }
  return s;
}

// Query-result marshaller: same shape as `handler_common::ValueToCell`
// except BYTES columns are base64-encoded to match
// `gateway/bqtypes/wire.go::ValueToCell`.
void QueryResultValueToCell(const backend::storage::Value& value,
                            v1::Cell* out) {
  using Kind = backend::storage::Value::Kind;
  out->Clear();
  switch (value.kind()) {
    case Kind::kNull:
      out->set_null_value(true);
      return;
    case Kind::kBool:
      out->set_string_value(value.bool_value() ? "true" : "false");
      return;
    case Kind::kInt64:
      out->set_string_value(absl::StrCat(value.int64_value()));
      return;
    case Kind::kFloat64:
      out->set_string_value(FormatQueryResultFloat64(value.float64_value()));
      return;
    case Kind::kString:
      out->set_string_value(value.string_value());
      return;
    case Kind::kBytes:
      out->set_string_value(absl::Base64Escape(value.string_value()));
      return;
    case Kind::kArray: {
      auto* arr = out->mutable_array();
      for (const auto& el : value.array_value()) {
        QueryResultValueToCell(el, arr->add_elements());
      }
      return;
    }
    case Kind::kStruct: {
      auto* st = out->mutable_struct_value();
      for (const auto& f : value.struct_value()) {
        QueryResultValueToCell(f, st->add_fields());
      }
      return;
    }
  }
}

// Translates a `bigquery_emulator.v1.QueryRequest` proto into the
// engine-facing `backend::engine::QueryRequest` struct. The two have
// the same fields but live in different packages (proto vs. plain
// C++ struct) so the engine never has to depend on the proto
// runtime.
backend::engine::QueryRequest ProtoToEngineRequest(
    const v1::QueryRequest& request) {
  backend::engine::QueryRequest engine_request;
  engine_request.project_id = request.project_id();
  engine_request.default_dataset_id = request.default_dataset_id();
  engine_request.sql = request.sql();
  engine_request.use_legacy_sql = request.use_legacy_sql();
  engine_request.principal_email = request.principal_email();
  engine_request.parameters.reserve(request.parameters_size());
  for (const auto& kv : request.parameters()) {
    backend::engine::QueryParameter parameter;
    parameter.name = kv.first;
    parameter.type_kind = kv.second.type_kind();
    parameter.value_json = kv.second.value_json();
    parameter.type_json = kv.second.type_json();
    engine_request.parameters.push_back(std::move(parameter));
  }
  // Proto map iteration order is undefined; sort positional keys (p0, p1,
  // ...) so `?` placeholders bind in SQL order. Sort before clearing
  // synthetic names — clearing first makes the comparator blind to pN.
  std::sort(engine_request.parameters.begin(),
            engine_request.parameters.end(),
            [](const backend::engine::QueryParameter& a,
               const backend::engine::QueryParameter& b) {
              int pa = 0;
              int pb = 0;
              pa = GatewayPositionalParameterIndex(a.name);
              pb = GatewayPositionalParameterIndex(b.name);
              if (pa >= 0 && pb >= 0) return pa < pb;
              return a.name < b.name;
            });
  for (auto& parameter : engine_request.parameters) {
    if (GatewayPositionalParameterIndex(parameter.name) >= 0) {
      parameter.name.clear();
    }
  }
  return engine_request;
}

// Statement classes the gateway needs to distinguish. The analyzer
// returns a richer `ResolvedNodeKind`; we collapse that down to the
// four categories BigQuery's REST API treats differently:
//   * `kSelect` -> `Query.ExecuteQuery` streams a schema + rows;
//     `Query.DryRun` returns the analyzed schema.
//   * `kDml`    -> `Query.ExecuteQuery` runs INSERT/UPDATE/DELETE/
//     MERGE through the engine's DML path and emits a final
//     `dml_stats` summary; `Query.DryRun` for DML is allowed and
//     returns an empty schema with zero estimated bytes (BigQuery
//     does the same).
//   * `kDdl`    -> reserved for CREATE/DROP/ALTER once those land;
//     today we surface `UNIMPLEMENTED` so client libraries see the
//     standard `notImplemented` reason.
//   * `kOther`  -> unclassified statement shape (CALL, EXPORT,
//     scripting, ...); also `UNIMPLEMENTED`.

StatementClass ClassifyStatement(::googlesql::ResolvedNodeKind kind) {
  switch (kind) {
    case ::googlesql::RESOLVED_QUERY_STMT:
    case ::googlesql::RESOLVED_EXPLAIN_STMT:
    case ::googlesql::RESOLVED_MULTI_STMT:
      return StatementClass::kSelect;
    case ::googlesql::RESOLVED_INSERT_STMT:
    case ::googlesql::RESOLVED_UPDATE_STMT:
    case ::googlesql::RESOLVED_DELETE_STMT:
    case ::googlesql::RESOLVED_MERGE_STMT:
    case ::googlesql::RESOLVED_TRUNCATE_STMT:
      return StatementClass::kDml;
    case ::googlesql::RESOLVED_CREATE_DATABASE_STMT:
    case ::googlesql::RESOLVED_CREATE_INDEX_STMT:
    case ::googlesql::RESOLVED_CREATE_SCHEMA_STMT:
    case ::googlesql::RESOLVED_CREATE_EXTERNAL_SCHEMA_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_AS_SELECT_STMT:
    case ::googlesql::RESOLVED_CREATE_EXTERNAL_TABLE_STMT:
    case ::googlesql::RESOLVED_CREATE_MODEL_STMT:
    case ::googlesql::RESOLVED_CREATE_VIEW_STMT:
    case ::googlesql::RESOLVED_CREATE_MATERIALIZED_VIEW_STMT:
    case ::googlesql::RESOLVED_CREATE_APPROX_VIEW_STMT:
    case ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT:
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT:
    case ::googlesql::RESOLVED_CREATE_CONSTANT_STMT:
    case ::googlesql::RESOLVED_CREATE_ENTITY_STMT:
    case ::googlesql::RESOLVED_CREATE_ROW_ACCESS_POLICY_STMT:
    case ::googlesql::RESOLVED_CREATE_PRIVILEGE_RESTRICTION_STMT:
    case ::googlesql::RESOLVED_CREATE_SNAPSHOT_TABLE_STMT:
    case ::googlesql::RESOLVED_CREATE_PROPERTY_GRAPH_STMT:
    case ::googlesql::RESOLVED_CREATE_CONNECTION_STMT:
    case ::googlesql::RESOLVED_CREATE_SEQUENCE_STMT:
    case ::googlesql::RESOLVED_CLONE_DATA_STMT:
    case ::googlesql::RESOLVED_DROP_STMT:
    case ::googlesql::RESOLVED_DROP_FUNCTION_STMT:
    case ::googlesql::RESOLVED_DROP_TABLE_FUNCTION_STMT:
    case ::googlesql::RESOLVED_DROP_INDEX_STMT:
    case ::googlesql::RESOLVED_DROP_MATERIALIZED_VIEW_STMT:
    case ::googlesql::RESOLVED_DROP_PRIVILEGE_RESTRICTION_STMT:
    case ::googlesql::RESOLVED_DROP_ROW_ACCESS_POLICY_STMT:
    case ::googlesql::RESOLVED_DROP_SNAPSHOT_TABLE_STMT:
    case ::googlesql::RESOLVED_RENAME_STMT:
    case ::googlesql::RESOLVED_ALTER_DATABASE_STMT:
    case ::googlesql::RESOLVED_ALTER_INDEX_STMT:
    case ::googlesql::RESOLVED_ALTER_MATERIALIZED_VIEW_STMT:
    case ::googlesql::RESOLVED_ALTER_APPROX_VIEW_STMT:
    case ::googlesql::RESOLVED_ALTER_MODEL_STMT:
    case ::googlesql::RESOLVED_ALTER_PRIVILEGE_RESTRICTION_STMT:
    case ::googlesql::RESOLVED_ALTER_ROW_ACCESS_POLICY_STMT:
    case ::googlesql::RESOLVED_ALTER_ALL_ROW_ACCESS_POLICIES_STMT:
    case ::googlesql::RESOLVED_ALTER_SCHEMA_STMT:
    case ::googlesql::RESOLVED_ALTER_EXTERNAL_SCHEMA_STMT:
    case ::googlesql::RESOLVED_ALTER_TABLE_SET_OPTIONS_STMT:
    case ::googlesql::RESOLVED_ALTER_TABLE_STMT:
    case ::googlesql::RESOLVED_ALTER_VIEW_STMT:
    case ::googlesql::RESOLVED_ALTER_CONNECTION_STMT:
    case ::googlesql::RESOLVED_ALTER_SEQUENCE_STMT:
    case ::googlesql::RESOLVED_ALTER_ENTITY_STMT:
    case ::googlesql::RESOLVED_GRANT_STMT:
    case ::googlesql::RESOLVED_REVOKE_STMT:
    case ::googlesql::RESOLVED_UNDROP_STMT:
      return StatementClass::kDdl;
    // `docs/ENGINE_POLICY.md` Family 5: ASSERT is
    // a no-row-stream statement that surfaces a structured
    // `invalidQuery` envelope on failure (BigQuery's documented
    // behavior) and produces no observable output on success. The
    // gateway routes it through the same `ExecuteDdl` plumbing
    // every other no-row-stream statement uses; the semantic
    // executor owns the predicate evaluation.
    case ::googlesql::RESOLVED_ASSERT_STMT:
    case ::googlesql::RESOLVED_ASSIGNMENT_STMT:
    case ::googlesql::RESOLVED_CALL_STMT:
    case ::googlesql::RESOLVED_AUX_LOAD_DATA_STMT:
    case ::googlesql::RESOLVED_EXPORT_DATA_STMT:
      return StatementClass::kDdl;
    default:
      return StatementClass::kOther;
  }
}

// Map a `ResolvedStatement` to the canonical BigQuery REST
// `Job.statistics.query.statementType` string. Mirrors the
// `statementType` enum documented at
// `docs/bigquery/docs/reference/rest/v2/Job.md`. Empty string means
// "no statementType envelope" (the gateway omits the field
// altogether for shapes BigQuery itself does not enumerate, e.g.
// internal or non-BigQuery surfaces).
//
// Plan ownership: `docs/ENGINE_POLICY.md`
// "Item 5 (statementType)". Each handler in
// `backend/engine/control/control_op_executor.cc` is the source of
// truth for what the statement does; this helper is the source of
// truth for what BigQuery REST calls that statement.
absl::string_view StatementTypeFor(const ::googlesql::ResolvedStatement& stmt) {
  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_QUERY_STMT:
      return "SELECT";
    case ::googlesql::RESOLVED_INSERT_STMT:
      return "INSERT";
    case ::googlesql::RESOLVED_UPDATE_STMT:
      return "UPDATE";
    case ::googlesql::RESOLVED_DELETE_STMT:
      return "DELETE";
    case ::googlesql::RESOLVED_MERGE_STMT:
      return "MERGE";
    case ::googlesql::RESOLVED_TRUNCATE_STMT:
      return "TRUNCATE_TABLE";
    case ::googlesql::RESOLVED_CREATE_TABLE_STMT:
      return "CREATE_TABLE";
    case ::googlesql::RESOLVED_CREATE_TABLE_AS_SELECT_STMT:
      return "CREATE_TABLE_AS_SELECT";
    case ::googlesql::RESOLVED_CREATE_VIEW_STMT:
      return "CREATE_VIEW";
    case ::googlesql::RESOLVED_CREATE_MATERIALIZED_VIEW_STMT:
      return "CREATE_MATERIALIZED_VIEW";
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT:
      return "CREATE_FUNCTION";
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT:
      return "CREATE_TABLE_FUNCTION";
    case ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT:
      return "CREATE_PROCEDURE";
    case ::googlesql::RESOLVED_CREATE_SCHEMA_STMT:
      return "CREATE_SCHEMA";
    case ::googlesql::RESOLVED_CREATE_SNAPSHOT_TABLE_STMT:
      return "CREATE_SNAPSHOT_TABLE";
    case ::googlesql::RESOLVED_CREATE_ROW_ACCESS_POLICY_STMT:
      return "CREATE_ROW_ACCESS_POLICY";
    case ::googlesql::RESOLVED_DROP_STMT:
      return "DROP_TABLE";
    case ::googlesql::RESOLVED_DROP_FUNCTION_STMT:
      return "DROP_FUNCTION";
    case ::googlesql::RESOLVED_DROP_TABLE_FUNCTION_STMT:
      return "DROP_TABLE_FUNCTION";
    case ::googlesql::RESOLVED_DROP_MATERIALIZED_VIEW_STMT:
      return "DROP_MATERIALIZED_VIEW";
    case ::googlesql::RESOLVED_DROP_ROW_ACCESS_POLICY_STMT:
      return "DROP_ROW_ACCESS_POLICY";
    case ::googlesql::RESOLVED_DROP_SNAPSHOT_TABLE_STMT:
      return "DROP_SNAPSHOT_TABLE";
    case ::googlesql::RESOLVED_ALTER_TABLE_STMT:
      return "ALTER_TABLE";
    case ::googlesql::RESOLVED_ALTER_VIEW_STMT:
      return "ALTER_VIEW";
    case ::googlesql::RESOLVED_ALTER_MATERIALIZED_VIEW_STMT:
      return "ALTER_MATERIALIZED_VIEW";
    case ::googlesql::RESOLVED_ALTER_SCHEMA_STMT:
      return "ALTER_SCHEMA";
    case ::googlesql::RESOLVED_ANALYZE_STMT:
      return "ANALYZE";
    case ::googlesql::RESOLVED_ASSERT_STMT:
      return "ASSERT";
    case ::googlesql::RESOLVED_AUX_LOAD_DATA_STMT:
      return "LOAD_DATA";
    case ::googlesql::RESOLVED_EXPORT_DATA_STMT:
      return "EXPORT_DATA";
    case ::googlesql::RESOLVED_GRANT_STMT:
      return "GRANT";
    case ::googlesql::RESOLVED_REVOKE_STMT:
      return "REVOKE";
    default:
      return "";
  }
}
}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
