#include "backend/engine/reference_impl/reference_impl_engine.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

// The reference-impl engine is the canonical caller of GoogleSQL's
// analyzer + reference-impl evaluator. We gate every GoogleSQL
// inclusion behind `BIGQUERY_EMULATOR_HAS_GOOGLESQL` so the legacy
// CMake build (which still compiles `backend_engine_reference_impl`
// alongside `backend_storage_memory` and `backend_storage_duckdb`
// without GoogleSQL linked in) keeps building. The Bazel build sets
// the define on the `cc_library`, flipping the engine into its real
// implementation; the CMake build leaves it unset and gets the
// `UNIMPLEMENTED` stubs back -- identical to how
// `frontend/handlers/query.cc` handles `DryRun`.

#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/evaluator.h"
#include "googlesql/public/evaluator_base.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace reference_impl {

#if !defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)

namespace {

constexpr char kNoGoogleSqlMsg[] =
    "reference_impl engine: this build was produced without GoogleSQL "
    "linked in; rebuild via Bazel to enable Analyze / DryRun / "
    "ExecuteQuery";

}  // namespace

ReferenceImplEngine::ReferenceImplEngine(storage::Storage* storage)
    : storage_(storage) {}

ReferenceImplEngine::~ReferenceImplEngine() = default;

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> ReferenceImplEngine::Analyze(
    const QueryRequest& /*request*/, ::googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(kNoGoogleSqlMsg);
}

absl::StatusOr<DryRunResult> ReferenceImplEngine::DryRun(
    const QueryRequest& /*request*/, ::googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(kNoGoogleSqlMsg);
}

absl::StatusOr<std::unique_ptr<RowSource>> ReferenceImplEngine::ExecuteQuery(
    const QueryRequest& /*request*/, ::googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(kNoGoogleSqlMsg);
}

absl::StatusOr<DmlStats> ReferenceImplEngine::ExecuteDml(
    const QueryRequest& /*request*/, ::googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(kNoGoogleSqlMsg);
}

#else  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

namespace {

// Builds the AnalyzerOptions the reference-impl engine uses for both
// `Analyze` and `ExecuteQuery`. Mirrors the configuration in
// `frontend/handlers/query.cc::MakeAnalyzerOptions` so the two paths
// resolve names the same way; centralizing this is on the followup
// plan (engine + handler share one helper).
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  // The analyzer's default `supported_statement_kinds_` only allows
  // `RESOLVED_QUERY_STMT`, which would reject INSERT/UPDATE/DELETE/
  // MERGE in `Prepare()` with a generic "Statement not supported"
  // error before our handler ever sees the resolved AST. Phase 6a's
  // statement classifier in `frontend/handlers/query.cc` is the right
  // place to gate which kinds we *implement*, so we opt in to all
  // statement kinds at the language layer and let the handler
  // surface UNIMPLEMENTED for the ones the engine does not yet run.
  language.SetSupportsAllStatementKinds();
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  // The reference-impl evaluator requires every resolved column to
  // map to a physical column on the source table, so we leave
  // `prune_unused_columns` at its default (false). Flipping it on
  // breaks `CreateEvaluatorTableIterator`'s assumption that the
  // requested column indexes are valid offsets into the underlying
  // table.
  options.CreateDefaultArenasIfNotSet();
  return options;
}

// Convert a `googlesql::Value` back into the engine-agnostic storage
// `Value` shape so the engine's `RowSource::Next` contract holds. The
// translation is the inverse of `StorageValueToGoogleSqlValue` in
// `backend/catalog/storage_table.cc`; we do it here (instead of in
// the storage layer) because the engine is the only caller and we
// don't want the storage layer to acquire a GoogleSQL dependency.
absl::StatusOr<storage::Value> GoogleSqlValueToStorageValue(
    const ::googlesql::Value& value) {
  if (value.is_null()) {
    return storage::Value::Null();
  }
  const ::googlesql::Type* type = value.type();
  if (type == nullptr) {
    return absl::InvalidArgumentError(
        "ReferenceImplEngine: googlesql Value has null type");
  }
  switch (type->kind()) {
    case ::googlesql::TYPE_BOOL:
      return storage::Value::Bool(value.bool_value());
    case ::googlesql::TYPE_INT32:
      return storage::Value::Int64(value.int32_value());
    case ::googlesql::TYPE_INT64:
      return storage::Value::Int64(value.int64_value());
    case ::googlesql::TYPE_UINT32:
      return storage::Value::Int64(
          static_cast<int64_t>(value.uint32_value()));
    case ::googlesql::TYPE_UINT64:
      return storage::Value::Int64(
          static_cast<int64_t>(value.uint64_value()));
    case ::googlesql::TYPE_FLOAT:
      return storage::Value::Float64(value.float_value());
    case ::googlesql::TYPE_DOUBLE:
      return storage::Value::Float64(value.double_value());
    case ::googlesql::TYPE_STRING:
      return storage::Value::String(std::string(value.string_value()));
    case ::googlesql::TYPE_BYTES:
      return storage::Value::Bytes(std::string(value.bytes_value()));
    case ::googlesql::TYPE_ARRAY: {
      std::vector<storage::Value> elements;
      elements.reserve(value.num_elements());
      for (int i = 0; i < value.num_elements(); ++i) {
        absl::StatusOr<storage::Value> v =
            GoogleSqlValueToStorageValue(value.element(i));
        if (!v.ok()) return v.status();
        elements.push_back(std::move(v).value());
      }
      return storage::Value::Array(std::move(elements));
    }
    case ::googlesql::TYPE_STRUCT: {
      std::vector<storage::Value> fields;
      fields.reserve(value.num_fields());
      for (int i = 0; i < value.num_fields(); ++i) {
        absl::StatusOr<storage::Value> v =
            GoogleSqlValueToStorageValue(value.field(i));
        if (!v.ok()) return v.status();
        fields.push_back(std::move(v).value());
      }
      return storage::Value::Struct(std::move(fields));
    }
    default:
      // Date / Time / Timestamp / Numeric / Bignumeric / JSON / etc.
      // ride out as the string form of the value so the gateway
      // still gets a usable rendering. Future plans (`wire-marshal-go`
      // and on) will replace this with the matching BigQuery REST
      // encodings.
      return storage::Value::String(value.DebugString());
  }
}

// Builds the engine-agnostic `schema::TableSchema` from an analyzer
// `ResolvedQueryStmt`. Routed through the proto round-trip we already
// use in DryRun so the two paths agree on field ordering and on the
// REPEATED-mode contract for ARRAY columns.
absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt) {
  v1::TableSchema proto;
  absl::Status s = backend::schema::OutputColumnListToTableSchema(
      stmt.output_column_list(), &proto);
  if (!s.ok()) return s;
  return backend::schema::TableSchemaFromProto(proto);
}

// Concrete `AnalyzedQuery` returned by `Analyze`. It captures the
// resolved AST so a follow-up `ExecuteQuery` could prepare directly
// against it without re-parsing -- the wrapper itself doesn't expose
// that yet (the gateway still goes through `Analyze` for `DryRun`
// only), but keeping the output here is what lets the engine reuse
// analysis work across the two methods if that lands later.
class AnalyzedQueryImpl : public AnalyzedQuery {
 public:
  AnalyzedQueryImpl(std::unique_ptr<const ::googlesql::AnalyzerOutput> output,
                    schema::TableSchema schema)
      : output_(std::move(output)), schema_(std::move(schema)) {}

  const schema::TableSchema& output_schema() const override {
    return schema_;
  }

 private:
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output_;
  schema::TableSchema schema_;
};

// Concrete `RowSource` returned by `ExecuteQuery`. It owns the
// `PreparedQuery` and the `EvaluatorTableIterator` so the caller can
// pull rows until end-of-stream without thinking about the GoogleSQL
// lifetime contract -- destroying the `RowSource` shuts everything
// down in the right order (iterator first, then prepared query).
class RowSourceImpl : public RowSource {
 public:
  RowSourceImpl(std::unique_ptr<::googlesql::TypeFactory> type_factory,
                std::unique_ptr<::googlesql::PreparedQuery> query,
                std::unique_ptr<::googlesql::EvaluatorTableIterator> iter,
                schema::TableSchema schema)
      : type_factory_(std::move(type_factory)),
        query_(std::move(query)),
        iter_(std::move(iter)),
        schema_(std::move(schema)) {}

  const schema::TableSchema& schema() const override { return schema_; }

  absl::StatusOr<bool> Next(storage::Row* row) override {
    if (row == nullptr) {
      return absl::InvalidArgumentError(
          "ReferenceImplEngine row source: Next called with null row");
    }
    if (iter_ == nullptr) {
      return absl::FailedPreconditionError(
          "ReferenceImplEngine row source: iterator is not initialized");
    }
    if (!iter_->NextRow()) {
      absl::Status s = iter_->Status();
      if (!s.ok()) return s;
      return false;
    }
    row->cells.clear();
    row->cells.reserve(iter_->NumColumns());
    for (int i = 0; i < iter_->NumColumns(); ++i) {
      absl::StatusOr<storage::Value> cell =
          GoogleSqlValueToStorageValue(iter_->GetValue(i));
      if (!cell.ok()) return cell.status();
      row->cells.push_back(std::move(cell).value());
    }
    return true;
  }

 private:
  // Declaration order pins destruction order: iter_ goes first
  // (releases its hold on the evaluator state), then query_ (releases
  // the algebrized plan + resolved AST), then type_factory_ (owns
  // the Type pointers everything above references).
  std::unique_ptr<::googlesql::TypeFactory> type_factory_;
  std::unique_ptr<::googlesql::PreparedQuery> query_;
  std::unique_ptr<::googlesql::EvaluatorTableIterator> iter_;
  schema::TableSchema schema_;
};

// Sanity-check the inbound request shape. Catches the same misuse
// the gateway is supposed to reject; the engine repeats the check
// because the engine can also be called directly from C++ tests that
// bypass the gateway.
absl::Status ValidateRequest(const QueryRequest& request,
                              ::googlesql::Catalog* catalog) {
  if (catalog == nullptr) {
    return absl::FailedPreconditionError(
        "ReferenceImplEngine: catalog must be non-null");
  }
  if (request.sql.empty()) {
    return absl::InvalidArgumentError(
        "ReferenceImplEngine: request.sql is required");
  }
  if (request.use_legacy_sql) {
    return absl::InvalidArgumentError(
        "ReferenceImplEngine: useLegacySql=true is not supported; only "
        "GoogleSQL is implemented");
  }
  return absl::OkStatus();
}

}  // namespace

ReferenceImplEngine::ReferenceImplEngine(storage::Storage* storage)
    : storage_(storage) {}

ReferenceImplEngine::~ReferenceImplEngine() = default;

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> ReferenceImplEngine::Analyze(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;

  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyze = ::googlesql::AnalyzeStatement(
      request.sql, options, catalog, &type_factory, &output);
  if (!analyze.ok()) return analyze;
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "ReferenceImplEngine::Analyze: analyzer returned no resolved "
        "statement");
  }
  const ::googlesql::ResolvedStatement* stmt = output->resolved_statement();
  if (stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(absl::StrCat(
        "ReferenceImplEngine::Analyze: only SELECT-shaped queries are "
        "supported; got ", stmt->node_kind_string()));
  }
  absl::StatusOr<schema::TableSchema> output_schema =
      ReflectOutputSchema(*stmt->GetAs<::googlesql::ResolvedQueryStmt>());
  if (!output_schema.ok()) return output_schema.status();
  return std::make_unique<AnalyzedQueryImpl>(std::move(output),
                                              std::move(*output_schema));
}

absl::StatusOr<DryRunResult> ReferenceImplEngine::DryRun(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> analyzed =
      Analyze(request, catalog);
  if (!analyzed.ok()) return analyzed.status();
  DryRunResult result;
  result.schema = (*analyzed)->output_schema();
  // BigQuery's dry-run statistic surfaces a per-table byte-size
  // estimate; we don't have a cost model yet (same caveat as the
  // gateway's `DryRun` handler), so emit zero.
  result.estimated_bytes_processed = 0;
  return result;
}

absl::StatusOr<std::unique_ptr<RowSource>> ReferenceImplEngine::ExecuteQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;

  // The reference-impl evaluator constructs `googlesql::Value`s from
  // the analyzer-allocated `Type*`s; both must come from the same
  // `TypeFactory` so the algebrizer can compare type pointers by
  // identity. We allocate a fresh factory per query and pin it on
  // both the analyzer (via AnalyzerOptions) and the evaluator (via
  // EvaluatorOptions).
  auto type_factory = std::make_unique<::googlesql::TypeFactory>();

  ::googlesql::AnalyzerOptions analyzer_options = MakeAnalyzerOptions();
  ::googlesql::EvaluatorOptions evaluator_options;
  evaluator_options.type_factory = type_factory.get();

  auto query =
      std::make_unique<::googlesql::PreparedQuery>(request.sql, evaluator_options);
  absl::Status prepare = query->Prepare(analyzer_options, catalog);
  if (!prepare.ok()) return prepare;

  const ::googlesql::ResolvedQueryStmt* stmt = query->resolved_query_stmt();
  if (stmt == nullptr) {
    return absl::InternalError(
        "ReferenceImplEngine::ExecuteQuery: PreparedQuery did not expose a "
        "resolved query statement");
  }
  absl::StatusOr<schema::TableSchema> output_schema =
      ReflectOutputSchema(*stmt);
  if (!output_schema.ok()) return output_schema.status();

  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>> iter =
      query->Execute();
  if (!iter.ok()) return iter.status();

  return std::unique_ptr<RowSource>(new RowSourceImpl(
      std::move(type_factory), std::move(query), std::move(iter).value(),
      std::move(*output_schema)));
}

namespace {

// Recover the `StorageTable*` behind the resolved table scan of a DML
// statement. The catalog hands out `StorageTable*` for every
// materialized table; a non-StorageTable means the analyzer resolved
// against a different shape (e.g. a built-in) and we cannot
// meaningfully apply DML to it.
absl::StatusOr<const catalog::StorageTable*> StorageTargetFor(
    const ::googlesql::ResolvedTableScan* scan, absl::string_view what) {
  if (scan == nullptr || scan->table() == nullptr) {
    return absl::InternalError(absl::StrCat(
        "ReferenceImplEngine::ExecuteDml: ", what,
        " has no resolved table scan"));
  }
  const auto* storage_table =
      dynamic_cast<const catalog::StorageTable*>(scan->table());
  if (storage_table == nullptr) {
    return absl::FailedPreconditionError(absl::StrCat(
        "ReferenceImplEngine::ExecuteDml: ", what, " target table '",
        scan->table()->FullName(),
        "' is not backed by a StorageTable; cannot apply DML"));
  }
  return storage_table;
}

// Project the iterator's "new" row values onto a fresh
// `storage::Row` in catalog column order. The iterator carries one
// `googlesql::Value` per table column; for INSERT VALUES that does
// not name every column the analyzer has already filled defaults /
// NULLs into the unmentioned slots, so we walk the full schema.
absl::StatusOr<storage::Row> BuildRowFromIterator(
    const ::googlesql::EvaluatorTableModifyIterator& iter,
    const schema::TableSchema& schema) {
  storage::Row row;
  row.cells.reserve(schema.columns.size());
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    const ::googlesql::Value& cell =
        iter.GetColumnValue(static_cast<int>(i));
    absl::StatusOr<storage::Value> converted =
        GoogleSqlValueToStorageValue(cell);
    if (!converted.ok()) return converted.status();
    row.cells.push_back(std::move(converted).value());
  }
  return row;
}

// Pull every row out of the storage backend for the table behind
// `target_id`. Used by UPDATE / DELETE so we can rewrite the snapshot
// in-place via `OverwriteRows`. A NOT_FOUND on the table propagates
// to the caller verbatim so the gateway can map it to BigQuery's
// matching REST error.
absl::StatusOr<std::vector<storage::Row>> ScanAllRows(
    const storage::Storage& storage, const storage::TableId& target_id) {
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage.ScanRows(target_id);
  if (!iter.ok()) return iter.status();
  std::unique_ptr<storage::RowIterator> rows_iter = std::move(iter).value();
  std::vector<storage::Row> all_rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows_iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    all_rows.push_back(row);
  }
  return all_rows;
}

// Stable string representation of a `storage::Value` so primary-key
// lookups land on a single map entry regardless of the underlying
// variant kind. The first column of every `StorageTable` is treated
// as the synthetic primary key (see the constructor in
// `backend/catalog/storage_table.cc`); the engine compares two
// PK Values by serializing them through this helper so a `Bool(true)`
// from one path and a `String("true")` from another do not collide.
std::string SerializeForPkLookup(const storage::Value& value) {
  using Kind = storage::Value::Kind;
  switch (value.kind()) {
    case Kind::kNull:
      return "n:";
    case Kind::kBool:
      return std::string(value.bool_value() ? "b:1" : "b:0");
    case Kind::kInt64:
      return absl::StrCat("i:", value.int64_value());
    case Kind::kFloat64:
      return absl::StrCat("f:", value.float64_value());
    case Kind::kString:
      return absl::StrCat("s:", value.string_value());
    case Kind::kBytes:
      return absl::StrCat("y:", value.string_value());
    case Kind::kArray: {
      std::string out = "a:[";
      for (const auto& e : value.array_value()) {
        absl::StrAppend(&out, SerializeForPkLookup(e), ",");
      }
      absl::StrAppend(&out, "]");
      return out;
    }
    case Kind::kStruct: {
      std::string out = "t:{";
      for (const auto& f : value.struct_value()) {
        absl::StrAppend(&out, SerializeForPkLookup(f), ",");
      }
      absl::StrAppend(&out, "}");
      return out;
    }
  }
  return "?:";
}

// Run the INSERT path: every iterator row is a fresh row to append
// to the storage backend.
absl::StatusOr<DmlStats> RunInsert(
    storage::Storage& storage, const storage::TableId& target_id,
    const schema::TableSchema& target_schema,
    ::googlesql::EvaluatorTableModifyIterator& rows) {
  std::vector<storage::Row> pending;
  while (rows.NextRow()) {
    if (rows.GetOperation() !=
        ::googlesql::EvaluatorTableModifyIterator::Operation::kInsert) {
      return absl::InternalError(absl::StrCat(
          "ReferenceImplEngine::ExecuteDml: INSERT iterator yielded a "
          "non-insert operation kind ",
          static_cast<int>(rows.GetOperation())));
    }
    absl::StatusOr<storage::Row> row =
        BuildRowFromIterator(rows, target_schema);
    if (!row.ok()) return row.status();
    pending.push_back(*std::move(row));
  }
  absl::Status iter_status = rows.Status();
  if (!iter_status.ok()) return iter_status;

  if (!pending.empty()) {
    absl::Status appended = storage.AppendRows(
        target_id, absl::MakeConstSpan(pending));
    if (!appended.ok()) return appended;
  }
  DmlStats stats;
  stats.inserted_row_count = static_cast<int64_t>(pending.size());
  return stats;
}

// Run the UPDATE path: iterator yields one row per matched row, where
// each row carries the post-mutation values for every catalog column.
// We pair them with the original PK via `GetOriginalKeyValue(0)`
// (the catalog wires `PrimaryKey = {0}` in `StorageTable`), scan the
// existing storage snapshot, and rewrite each matching row.
absl::StatusOr<DmlStats> RunUpdate(
    storage::Storage& storage, const storage::TableId& target_id,
    const schema::TableSchema& target_schema,
    ::googlesql::EvaluatorTableModifyIterator& rows) {
  absl::flat_hash_map<std::string, storage::Row> updates;
  int64_t modified = 0;
  while (rows.NextRow()) {
    if (rows.GetOperation() !=
        ::googlesql::EvaluatorTableModifyIterator::Operation::kUpdate) {
      return absl::InternalError(absl::StrCat(
          "ReferenceImplEngine::ExecuteDml: UPDATE iterator yielded a "
          "non-update operation kind ",
          static_cast<int>(rows.GetOperation())));
    }
    const ::googlesql::Value& key = rows.GetOriginalKeyValue(0);
    absl::StatusOr<storage::Value> pk = GoogleSqlValueToStorageValue(key);
    if (!pk.ok()) return pk.status();
    absl::StatusOr<storage::Row> new_row =
        BuildRowFromIterator(rows, target_schema);
    if (!new_row.ok()) return new_row.status();
    updates.insert_or_assign(SerializeForPkLookup(*pk), *std::move(new_row));
    ++modified;
  }
  absl::Status iter_status = rows.Status();
  if (!iter_status.ok()) return iter_status;

  if (updates.empty()) {
    DmlStats stats;
    stats.updated_row_count = 0;
    return stats;
  }

  absl::StatusOr<std::vector<storage::Row>> all_rows =
      ScanAllRows(storage, target_id);
  if (!all_rows.ok()) return all_rows.status();

  // Walk the existing rows in storage order; swap any row whose PK
  // (column 0) shows up in the iterator's update map. Rows not
  // touched by the UPDATE keep their original cell vector.
  for (storage::Row& row : *all_rows) {
    if (row.cells.empty()) continue;
    auto it = updates.find(SerializeForPkLookup(row.cells.front()));
    if (it == updates.end()) continue;
    row = it->second;
  }
  absl::Status applied = storage.OverwriteRows(
      target_id, absl::MakeConstSpan(*all_rows));
  if (!applied.ok()) return applied;

  DmlStats stats;
  stats.updated_row_count = modified;
  return stats;
}

// Run the DELETE path: iterator yields one row per matched row; we
// only need each row's original PK to drop the matching row from the
// storage snapshot. `GetColumnValue` returns an invalid Value for
// kDelete operations (see `evaluator_base.cc`) so we never call it.
absl::StatusOr<DmlStats> RunDelete(
    storage::Storage& storage, const storage::TableId& target_id,
    ::googlesql::EvaluatorTableModifyIterator& rows) {
  absl::flat_hash_set<std::string> deleted_pks;
  int64_t deleted = 0;
  while (rows.NextRow()) {
    if (rows.GetOperation() !=
        ::googlesql::EvaluatorTableModifyIterator::Operation::kDelete) {
      return absl::InternalError(absl::StrCat(
          "ReferenceImplEngine::ExecuteDml: DELETE iterator yielded a "
          "non-delete operation kind ",
          static_cast<int>(rows.GetOperation())));
    }
    const ::googlesql::Value& key = rows.GetOriginalKeyValue(0);
    absl::StatusOr<storage::Value> pk = GoogleSqlValueToStorageValue(key);
    if (!pk.ok()) return pk.status();
    deleted_pks.insert(SerializeForPkLookup(*pk));
    ++deleted;
  }
  absl::Status iter_status = rows.Status();
  if (!iter_status.ok()) return iter_status;

  if (deleted_pks.empty()) {
    DmlStats stats;
    stats.deleted_row_count = 0;
    return stats;
  }

  absl::StatusOr<std::vector<storage::Row>> all_rows =
      ScanAllRows(storage, target_id);
  if (!all_rows.ok()) return all_rows.status();

  std::vector<storage::Row> kept;
  kept.reserve(all_rows->size());
  for (storage::Row& row : *all_rows) {
    if (row.cells.empty()) {
      kept.push_back(std::move(row));
      continue;
    }
    if (deleted_pks.contains(SerializeForPkLookup(row.cells.front()))) {
      continue;
    }
    kept.push_back(std::move(row));
  }
  absl::Status applied = storage.OverwriteRows(
      target_id, absl::MakeConstSpan(kept));
  if (!applied.ok()) return applied;

  DmlStats stats;
  stats.deleted_row_count = deleted;
  return stats;
}

}  // namespace

absl::StatusOr<DmlStats> ReferenceImplEngine::ExecuteDml(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "ReferenceImplEngine::ExecuteDml: storage backend is not "
        "configured");
  }

  // Mirror the analyzer plumbing in ExecuteQuery so the resolved AST
  // we hand to `PreparedModify` shares the same `LanguageOptions` and
  // `TypeFactory` snapshot as the SELECT path.
  auto type_factory = std::make_unique<::googlesql::TypeFactory>();
  ::googlesql::AnalyzerOptions analyzer_options = MakeAnalyzerOptions();
  ::googlesql::EvaluatorOptions evaluator_options;
  evaluator_options.type_factory = type_factory.get();

  auto modify = std::make_unique<::googlesql::PreparedModify>(
      request.sql, evaluator_options);
  absl::Status prepare = modify->Prepare(analyzer_options, catalog);
  if (!prepare.ok()) return prepare;

  const ::googlesql::ResolvedStatement* stmt = modify->resolved_statement();
  if (stmt == nullptr) {
    return absl::InternalError(
        "ReferenceImplEngine::ExecuteDml: PreparedModify did not expose a "
        "resolved statement");
  }

  // Resolve the target table once via the statement-shape switch
  // below; every DML kind PreparedModify supports (INSERT / UPDATE /
  // DELETE) wraps a `ResolvedTableScan` we have to downcast back to
  // a `StorageTable` to recover the storage::TableId.
  const ::googlesql::ResolvedTableScan* table_scan = nullptr;
  absl::string_view stmt_kind;
  switch (stmt->node_kind()) {
    case ::googlesql::RESOLVED_INSERT_STMT:
      table_scan =
          stmt->GetAs<::googlesql::ResolvedInsertStmt>()->table_scan();
      stmt_kind = "INSERT";
      break;
    case ::googlesql::RESOLVED_UPDATE_STMT:
      table_scan =
          stmt->GetAs<::googlesql::ResolvedUpdateStmt>()->table_scan();
      stmt_kind = "UPDATE";
      break;
    case ::googlesql::RESOLVED_DELETE_STMT:
      table_scan =
          stmt->GetAs<::googlesql::ResolvedDeleteStmt>()->table_scan();
      stmt_kind = "DELETE";
      break;
    case ::googlesql::RESOLVED_MERGE_STMT:
      // MERGE is intentionally deferred to a follow-up plan: the
      // reference-impl algebrizer does not yet algebrize
      // ResolvedMergeStmt at the statement root (see the
      // `// TODO: Add MERGE support.` comment in
      // `googlesql/reference_impl/algebrizer.cc::AlgebrizeStatement`),
      // so PreparedModify cannot run a MERGE end-to-end. We return
      // UNIMPLEMENTED with the standard prefix so the FallbackEngine
      // wrapper can route the query to another engine if one is
      // configured. Phase 6c will land a scan-and-diff MERGE
      // implementation that does not depend on PreparedModify.
      return absl::UnimplementedError(
          "ReferenceImplEngine::ExecuteDml: MERGE is not yet implemented "
          "(the GoogleSQL reference-impl algebrizer does not yet support "
          "MERGE at the statement root); rewrite as INSERT / UPDATE / "
          "DELETE or wait for Phase 6c");
    default:
      return absl::UnimplementedError(absl::StrCat(
          "ReferenceImplEngine::ExecuteDml: statement kind ",
          stmt->node_kind_string(),
          " is not a DML statement supported by PreparedModify"));
  }

  absl::StatusOr<const catalog::StorageTable*> storage_table =
      StorageTargetFor(table_scan, stmt_kind);
  if (!storage_table.ok()) return storage_table.status();
  const storage::TableId target_id = (*storage_table)->storage_table_id();
  const schema::TableSchema& target_schema = (*storage_table)->bq_schema();

  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableModifyIterator>>
      iter = modify->Execute();
  if (!iter.ok()) return iter.status();
  std::unique_ptr<::googlesql::EvaluatorTableModifyIterator> rows =
      std::move(iter).value();
  if (rows == nullptr) {
    return absl::InternalError(
        "ReferenceImplEngine::ExecuteDml: PreparedModify::Execute "
        "returned a null iterator");
  }

  switch (stmt->node_kind()) {
    case ::googlesql::RESOLVED_INSERT_STMT:
      return RunInsert(*storage_, target_id, target_schema, *rows);
    case ::googlesql::RESOLVED_UPDATE_STMT:
      return RunUpdate(*storage_, target_id, target_schema, *rows);
    case ::googlesql::RESOLVED_DELETE_STMT:
      return RunDelete(*storage_, target_id, *rows);
    default:
      // Unreachable: the switch above already filtered to the three
      // supported kinds.
      return absl::InternalError(absl::StrCat(
          "ReferenceImplEngine::ExecuteDml: unexpected statement kind ",
          stmt->node_kind_string()));
  }
}

#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

}  // namespace reference_impl
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
