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
  if (stmt->node_kind() != ::googlesql::RESOLVED_INSERT_STMT) {
    return absl::UnimplementedError(absl::StrCat(
        "ReferenceImplEngine::ExecuteDml: only INSERT is implemented "
        "today; got ", stmt->node_kind_string()));
  }
  const auto* insert_stmt =
      stmt->GetAs<::googlesql::ResolvedInsertStmt>();
  const ::googlesql::ResolvedTableScan* table_scan =
      insert_stmt->table_scan();
  if (table_scan == nullptr || table_scan->table() == nullptr) {
    return absl::InternalError(
        "ReferenceImplEngine::ExecuteDml: INSERT has no resolved table "
        "scan");
  }
  // The catalog hands out `StorageTable*` for every materialized
  // table; downcast so we can recover the engine-agnostic
  // `storage::TableId`. A non-StorageTable means the catalog adapter
  // returned a different shape (e.g. a built-in) and we cannot
  // meaningfully append rows to it.
  const auto* storage_table = dynamic_cast<const catalog::StorageTable*>(
      table_scan->table());
  if (storage_table == nullptr) {
    return absl::FailedPreconditionError(absl::StrCat(
        "ReferenceImplEngine::ExecuteDml: INSERT target table '",
        table_scan->table()->FullName(),
        "' is not backed by a StorageTable; cannot append rows"));
  }
  const storage::TableId target_id = storage_table->storage_table_id();
  const schema::TableSchema& target_schema = storage_table->bq_schema();

  // PreparedModify's iterator returns a row per insert with one
  // googlesql::Value per *table* column (in catalog order). For an
  // INSERT VALUES with a column subset, the analyzer fills in
  // defaults / NULLs for the unmentioned columns automatically before
  // the iterator hands the row back, so we always project across the
  // full `target_schema.columns` list here.
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

  std::vector<storage::Row> pending;
  while (rows->NextRow()) {
    if (rows->GetOperation() !=
        ::googlesql::EvaluatorTableModifyIterator::Operation::kInsert) {
      // We classified the statement as INSERT above, so anything
      // other than `kInsert` here is a reference-impl invariant
      // violation; surface it as INTERNAL so the caller sees the
      // bug instead of a silently dropped row.
      return absl::InternalError(absl::StrCat(
          "ReferenceImplEngine::ExecuteDml: INSERT iterator yielded a "
          "non-insert operation kind ",
          static_cast<int>(rows->GetOperation())));
    }
    storage::Row row;
    row.cells.reserve(target_schema.columns.size());
    for (size_t i = 0; i < target_schema.columns.size(); ++i) {
      const ::googlesql::Value& cell = rows->GetColumnValue(static_cast<int>(i));
      absl::StatusOr<storage::Value> converted =
          GoogleSqlValueToStorageValue(cell);
      if (!converted.ok()) return converted.status();
      row.cells.push_back(std::move(converted).value());
    }
    pending.push_back(std::move(row));
  }
  absl::Status iter_status = rows->Status();
  if (!iter_status.ok()) return iter_status;

  if (!pending.empty()) {
    absl::Status appended = storage_->AppendRows(
        target_id, absl::MakeConstSpan(pending));
    if (!appended.ok()) return appended;
  }

  DmlStats stats;
  stats.inserted_row_count = static_cast<int64_t>(pending.size());
  return stats;
}

#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

}  // namespace reference_impl
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
