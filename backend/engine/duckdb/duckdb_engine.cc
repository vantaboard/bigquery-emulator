#include "backend/engine/duckdb/duckdb_engine.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

// The DuckDB engine needs both the GoogleSQL analyzer + resolved AST
// (so it can transpile and walk for referenced tables) and libduckdb
// (so it can execute the lowered SQL). The Bazel build sets both
// defines; the legacy CMake build leaves `BIGQUERY_EMULATOR_HAS_GOOGLESQL`
// unset and the engine compiles down to UNIMPLEMENTED stubs the same
// way `backend/engine/reference_impl/reference_impl_engine.cc` does.
#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL) && \
    defined(BIGQUERY_EMULATOR_HAS_DUCKDB)
#define BIGQUERY_EMULATOR_DUCKDB_ENGINE_ENABLED 1
#endif

#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
#include "duckdb.h"
#endif

#ifdef BIGQUERY_EMULATOR_DUCKDB_ENGINE_ENABLED
#include <cmath>
#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"
#endif  // BIGQUERY_EMULATOR_DUCKDB_ENGINE_ENABLED

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

namespace {

// Returns the libduckdb C-API version when the build has it linked,
// or a stub when it hasn't. Calling this in the constructor pulls
// libduckdb's symbol table into the link line under `--as-needed` so
// libduckdb.so stays on the binary's DT_NEEDED list.
const char* DuckDBLibraryVersionOrStub() {
#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
  return ::duckdb_library_version();
#else
  return "<duckdb-disabled>";
#endif
}

}  // namespace

DuckDBEngine::DuckDBEngine(storage::Storage* storage) : storage_(storage) {
  (void)DuckDBLibraryVersionOrStub();
}

DuckDBEngine::~DuckDBEngine() = default;

#ifndef BIGQUERY_EMULATOR_DUCKDB_ENGINE_ENABLED

namespace {

constexpr char kDisabledMsg[] =
    "duckdb engine: this build was produced without GoogleSQL + "
    "libduckdb linked in; rebuild via Bazel to enable Analyze / "
    "DryRun / ExecuteQuery";

}  // namespace

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> DuckDBEngine::Analyze(
    const QueryRequest& /*request*/, ::googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(kDisabledMsg);
}

absl::StatusOr<DryRunResult> DuckDBEngine::DryRun(
    const QueryRequest& /*request*/, ::googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(kDisabledMsg);
}

absl::StatusOr<std::unique_ptr<RowSource>> DuckDBEngine::ExecuteQuery(
    const QueryRequest& /*request*/, ::googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(kDisabledMsg);
}

#else  // BIGQUERY_EMULATOR_DUCKDB_ENGINE_ENABLED

namespace {

// Mirror of `reference_impl_engine::MakeAnalyzerOptions`. The two
// engines have to resolve names through the same `LanguageOptions`
// snapshot or function dispatch drifts between Analyze and the
// per-engine ExecuteQuery; centralizing this helper is on the
// followup plan that lands the gateway-shared analyzer helper.
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  // We do NOT flip `prune_unused_columns` on: the reference-impl
  // evaluator needs every analyzer-allocated column to live (see the
  // matching note in `reference_impl_engine.cc`), and we keep the
  // same setting here so a `--on_unknown_fn=fallback` retry against
  // the reference-impl engine sees the same resolved shape.
  options.CreateDefaultArenasIfNotSet();
  return options;
}

absl::Status ValidateRequest(const QueryRequest& request,
                              ::googlesql::Catalog* catalog) {
  if (catalog == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDBEngine: catalog must be non-null");
  }
  if (request.sql.empty()) {
    return absl::InvalidArgumentError(
        "DuckDBEngine: request.sql is required");
  }
  if (request.use_legacy_sql) {
    return absl::InvalidArgumentError(
        "DuckDBEngine: useLegacySql=true is not supported; only "
        "GoogleSQL is implemented");
  }
  return absl::OkStatus();
}

// Build the BigQuery-shaped output schema from the analyzer's
// resolved-output-column list. Routed through the proto round-trip we
// already use in the reference-impl engine so the two paths agree on
// the REPEATED-mode contract for ARRAY columns.
absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt) {
  v1::TableSchema proto;
  absl::Status s = backend::schema::OutputColumnListToTableSchema(
      stmt.output_column_list(), &proto);
  if (!s.ok()) return s;
  return backend::schema::TableSchemaFromProto(proto);
}

// Concrete `AnalyzedQuery` returned by `Analyze`. Mirrors the
// reference-impl engine's wrapper; the resolved AST lives on the
// instance so a `DryRun` -> `ExecuteQuery` flow could reuse the
// analysis without re-parsing (today both methods re-analyze
// independently for simplicity).
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

// Walks the resolved AST and collects every distinct
// `ResolvedTableScan::table()` pointer. We dedupe on identity (the
// catalog hands out one `Table*` per `<project>.<dataset>.<table>`
// path within a query) so a self-join only materializes the table
// in DuckDB once.
class TableScanCollector : public ::googlesql::ResolvedASTVisitor {
 public:
  absl::Status VisitResolvedTableScan(
      const ::googlesql::ResolvedTableScan* node) override {
    if (node != nullptr && node->table() != nullptr) {
      tables_.insert(node->table());
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedTableScan(node);
  }

  const std::set<const ::googlesql::Table*>& tables() const {
    return tables_;
  }

 private:
  std::set<const ::googlesql::Table*> tables_;
};

// --- DuckDB SQL literal rendering -----------------------------------------
//
// Mirrors the helpers in `backend/storage/duckdb/duckdb_storage.cc`
// for the same Value -> DuckDB-literal job. The two copies stay in
// lockstep because the storage layer renders literals for INSERT
// statements against a Parquet-backed table and the engine renders
// literals for INSERT statements against a per-query in-memory
// DuckDB table; folding them into a shared helper is on the followup
// plan that consolidates DuckDB plumbing.

std::string QuoteIdent(absl::string_view ident) {
  std::string escaped = absl::StrReplaceAll(ident, {{"\"", "\"\""}});
  return absl::StrCat("\"", escaped, "\"");
}

std::string EscapeStringLiteralInner(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "''"}});
}

std::string RenderBlobLiteral(absl::string_view bytes) {
  static const char* kHex = "0123456789abcdef";
  std::string out;
  out.reserve(bytes.size() * 2 + 4);
  absl::StrAppend(&out, "X'");
  for (unsigned char c : bytes) {
    out += kHex[c >> 4];
    out += kHex[c & 0x0f];
  }
  absl::StrAppend(&out, "'");
  return out;
}

absl::StatusOr<std::string> RenderCellLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column);

absl::StatusOr<std::string> RenderScalarLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  // Like `duckdb_storage.cc::RenderScalarLiteral`, we accept both the
  // natively-typed Value variants (Int64, Float64, Bool) and a
  // String carrying the textual representation -- the Phase 1
  // gateway lowers every JSON cell to a string regardless of the
  // column's declared type, and `Storage::ScanRows` on an
  // InMemoryStorage path returns the exact same Value::String back.
  // The DuckDBStorage path on the other hand returns natively-typed
  // values because the Parquet file enforces the schema, so this
  // branch only fires for the in-memory storage backend.
  switch (column.type) {
    case schema::ColumnType::kBool:
      if (cell.kind() == storage::Value::Kind::kString) {
        return absl::StrCat("CAST('",
                             EscapeStringLiteralInner(cell.string_value()),
                             "' AS BOOLEAN)");
      }
      return std::string(cell.bool_value() ? "TRUE" : "FALSE");
    case schema::ColumnType::kInt64:
      if (cell.kind() == storage::Value::Kind::kString) {
        return absl::StrCat("CAST('",
                             EscapeStringLiteralInner(cell.string_value()),
                             "' AS BIGINT)");
      }
      return absl::StrCat(cell.int64_value());
    case schema::ColumnType::kFloat64: {
      if (cell.kind() == storage::Value::Kind::kString) {
        return absl::StrCat("CAST('",
                             EscapeStringLiteralInner(cell.string_value()),
                             "' AS DOUBLE)");
      }
      const double v = cell.float64_value();
      if (std::isnan(v)) return std::string("'NaN'::DOUBLE");
      if (std::isinf(v)) {
        return std::string(v > 0 ? "'Infinity'::DOUBLE"
                                  : "'-Infinity'::DOUBLE");
      }
      return absl::StrFormat("%.17g", v);
    }
    case schema::ColumnType::kString:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
      return absl::StrCat("'", EscapeStringLiteralInner(cell.string_value()),
                           "'");
    case schema::ColumnType::kBytes:
      return RenderBlobLiteral(cell.string_value());
    case schema::ColumnType::kDate:
      return absl::StrCat("DATE '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTime:
      return absl::StrCat("TIME '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kDatetime:
      return absl::StrCat("TIMESTAMP '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTimestamp:
      return absl::StrCat("TIMESTAMPTZ '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
      return absl::StrCat(
          "CAST('", EscapeStringLiteralInner(cell.string_value()),
          "' AS ", schema::ToDuckDBType(column.type), ")");
    case schema::ColumnType::kStruct: {
      if (cell.kind() != storage::Value::Kind::kStruct) {
        return absl::InvalidArgumentError(absl::StrCat(
            "DuckDBEngine: column '", column.name,
            "' expects STRUCT but row provided non-struct cell"));
      }
      const auto& fields = cell.struct_value();
      if (fields.size() != column.fields.size()) {
        return absl::InvalidArgumentError(absl::StrCat(
            "DuckDBEngine: STRUCT column '", column.name, "' has ",
            column.fields.size(), " fields but row provided ", fields.size()));
      }
      std::string out = "{";
      for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) absl::StrAppend(&out, ", ");
        absl::StrAppend(&out, "'",
                         EscapeStringLiteralInner(column.fields[i].name),
                         "': ");
        auto inner_or = RenderCellLiteral(fields[i], column.fields[i]);
        if (!inner_or.ok()) return inner_or.status();
        absl::StrAppend(&out, *inner_or);
      }
      absl::StrAppend(&out, "}");
      return out;
    }
    case schema::ColumnType::kArray:
    case schema::ColumnType::kUnknown:
      return absl::StrCat("'",
                           EscapeStringLiteralInner(cell.string_value()),
                           "'");
  }
  return absl::InternalError("RenderScalarLiteral: unreachable");
}

absl::StatusOr<std::string> RenderCellLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  if (cell.is_null()) return std::string("NULL");
  if (column.mode == schema::ColumnMode::kRepeated) {
    // REPEATED cells lower onto DuckDB LIST literals (`[v1, v2, ...]`).
    // The element renderer goes through a synthetic non-repeated
    // column so it does not re-enter the array branch.
    schema::ColumnSchema element = column;
    element.mode = schema::ColumnMode::kNullable;
    std::string out = "[";
    const auto& elems = cell.array_value();
    for (size_t i = 0; i < elems.size(); ++i) {
      if (i > 0) absl::StrAppend(&out, ", ");
      auto inner_or = RenderCellLiteral(elems[i], element);
      if (!inner_or.ok()) return inner_or.status();
      absl::StrAppend(&out, *inner_or);
    }
    absl::StrAppend(&out, "]");
    return out;
  }
  return RenderScalarLiteral(cell, column);
}

// Emit the parenthesized column-list-with-types for a CREATE TABLE
// statement against the storage table's schema.
std::string RenderColumnList(const schema::TableSchema& schema) {
  std::string out = "(";
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(&out, QuoteIdent(schema.columns[i].name), " ",
                     schema::ColumnSchemaToDuckDBType(schema.columns[i]));
  }
  absl::StrAppend(&out, ")");
  return out;
}

// Read one DuckDB cell into a storage::Value using `column` to pick
// the right C-API accessor. The mode/type discrimination mirrors
// the read path in `backend/storage/duckdb/duckdb_storage.cc` for
// the same reason: storage layer is engine-agnostic and the result
// of `Storage::ScanRows` flows through this conversion regardless of
// which engine ran the query.
absl::StatusOr<storage::Value> ReadCell(::duckdb_result* result, idx_t col,
                                         idx_t row,
                                         const schema::ColumnSchema& column) {
  if (::duckdb_value_is_null(result, col, row)) return storage::Value::Null();
  if (column.mode == schema::ColumnMode::kRepeated ||
      column.type == schema::ColumnType::kArray ||
      column.type == schema::ColumnType::kStruct) {
    // ARRAY / STRUCT round-trip through the canonical DuckDB
    // textual rendering. The transpiler doesn't yet lower queries
    // that return container columns, so a fall-into-string here is
    // the expected hand-off to the reference-impl fallback path for
    // any caller that actually exercises this branch.
    char* str = ::duckdb_value_varchar(result, col, row);
    storage::Value out = storage::Value::String(
        str == nullptr ? std::string("") : str);
    if (str != nullptr) ::duckdb_free(str);
    return out;
  }
  switch (column.type) {
    case schema::ColumnType::kBool:
      return storage::Value::Bool(::duckdb_value_boolean(result, col, row));
    case schema::ColumnType::kInt64:
      return storage::Value::Int64(::duckdb_value_int64(result, col, row));
    case schema::ColumnType::kFloat64:
      return storage::Value::Float64(::duckdb_value_double(result, col, row));
    case schema::ColumnType::kBytes: {
      ::duckdb_blob blob = ::duckdb_value_blob(result, col, row);
      std::string bytes;
      if (blob.data != nullptr) {
        bytes.assign(static_cast<const char*>(blob.data), blob.size);
        ::duckdb_free(blob.data);
      }
      return storage::Value::Bytes(std::move(bytes));
    }
    case schema::ColumnType::kString:
    case schema::ColumnType::kDate:
    case schema::ColumnType::kTime:
    case schema::ColumnType::kDatetime:
    case schema::ColumnType::kTimestamp:
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
    case schema::ColumnType::kArray:
    case schema::ColumnType::kStruct:
    case schema::ColumnType::kUnknown: {
      char* str = ::duckdb_value_varchar(result, col, row);
      storage::Value out = storage::Value::String(
          str == nullptr ? std::string("") : str);
      if (str != nullptr) ::duckdb_free(str);
      return out;
    }
  }
  return absl::InternalError("ReadCell: unreachable column type");
}

// Streams rows out of a materialized DuckDB result. Owns the
// underlying database + connection so the per-query DuckDB state is
// destroyed in the right order when the gateway finishes pulling
// rows. Declaration order pins destruction order: result_ first
// (releases the rendered chunk), then conn_, then db_.
class DuckDBRowSource : public RowSource {
 public:
  DuckDBRowSource(::duckdb_database db, ::duckdb_connection conn,
                  ::duckdb_result result, schema::TableSchema schema,
                  idx_t row_count)
      : db_(db),
        conn_(conn),
        result_(result),
        schema_(std::move(schema)),
        row_count_(row_count),
        next_row_(0) {}

  ~DuckDBRowSource() override {
    ::duckdb_destroy_result(&result_);
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
  }

  DuckDBRowSource(const DuckDBRowSource&) = delete;
  DuckDBRowSource& operator=(const DuckDBRowSource&) = delete;

  const schema::TableSchema& schema() const override { return schema_; }

  absl::StatusOr<bool> Next(storage::Row* row) override {
    if (row == nullptr) {
      return absl::InvalidArgumentError(
          "DuckDBEngine row source: Next called with null row");
    }
    if (next_row_ >= row_count_) return false;
    // We trust the analyzer-driven output schema for the column shape
    // rather than what DuckDB reports on the result: for the queries
    // the transpiler covers today the two agree column-for-column,
    // and surfacing the analyzer's BigQuery-typed column metadata is
    // what the gateway needs to render the REST `f`/`v` envelope.
    row->cells.clear();
    row->cells.reserve(schema_.columns.size());
    for (size_t c = 0; c < schema_.columns.size(); ++c) {
      absl::StatusOr<storage::Value> cell = ReadCell(
          &result_, static_cast<idx_t>(c), next_row_, schema_.columns[c]);
      if (!cell.ok()) return cell.status();
      row->cells.push_back(std::move(cell).value());
    }
    ++next_row_;
    return true;
  }

 private:
  ::duckdb_database db_;
  ::duckdb_connection conn_;
  ::duckdb_result result_;
  schema::TableSchema schema_;
  idx_t row_count_;
  idx_t next_row_;
};

// Runs `sql` on `conn`; returns OK or INTERNAL with the DuckDB
// error message attached. Use this for INSERT / CREATE statements
// where the result rowset is uninteresting.
absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql) {
  ::duckdb_result result;
  const std::string sql_str(sql);
  const auto state = ::duckdb_query(conn, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail =
        err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat(
        "DuckDBEngine: query failed: ", sql_str, ": ", detail));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

// Materialize one storage table inside `conn` so the transpiled
// SQL's bare table reference (`FROM "people"`, etc.) resolves at
// execute time. We CREATE OR REPLACE TABLE with the matching
// schema, then stream every row through `Storage::ScanRows` and
// INSERT them with one multi-row VALUES batch.
//
// The DuckDB-side table name mirrors the transpiler's
// `EmitTableScan` output (the bare `Table::Name()` the catalog
// returns). Note: this is the engine-agnostic path; when the
// active storage is DuckDB-backed, a future optimization can
// substitute a `CREATE VIEW ... AS SELECT * FROM read_parquet(...)`
// against the storage's Parquet snapshot for a no-copy attach.
absl::Status AttachStorageTable(::duckdb_connection conn,
                                 storage::Storage* storage,
                                 const catalog::StorageTable& table) {
  const schema::TableSchema& schema = table.bq_schema();
  const std::string table_name = QuoteIdent(table.Name());
  const std::string columns = RenderColumnList(schema);

  absl::Status status = RunSqlNoResult(
      conn,
      absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ", columns));
  if (!status.ok()) return status;

  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage->ScanRows(table.storage_table_id());
  if (!iter.ok()) return iter.status();

  std::unique_ptr<storage::RowIterator> rows_iter = std::move(iter).value();
  std::vector<storage::Row> rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows_iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  if (rows.empty()) return absl::OkStatus();

  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", table_name, " VALUES ");
  const size_t ncols = schema.columns.size();
  for (size_t r = 0; r < rows.size(); ++r) {
    if (rows[r].cells.size() != ncols) {
      return absl::InvalidArgumentError(absl::StrCat(
          "DuckDBEngine: row[", r, "] has ", rows[r].cells.size(),
          " cells but table '", table.Name(), "' has ", ncols, " columns"));
    }
    if (r > 0) absl::StrAppend(&insert_sql, ", ");
    absl::StrAppend(&insert_sql, "(");
    for (size_t c = 0; c < ncols; ++c) {
      if (c > 0) absl::StrAppend(&insert_sql, ", ");
      auto cell_or = RenderCellLiteral(rows[r].cells[c], schema.columns[c]);
      if (!cell_or.ok()) return cell_or.status();
      absl::StrAppend(&insert_sql, *cell_or);
    }
    absl::StrAppend(&insert_sql, ")");
  }
  return RunSqlNoResult(conn, insert_sql);
}

// Drop wrapping pass-through ResolvedProjectScans before handing
// the inner scan to the transpiler. The transpiler's
// `EmitProjectScan` / `EmitQueryStmt` are still `not_started` (see
// `SHAPE_TRACKER.md`), so we have to peel the wrapping scan the
// analyzer adds for `SELECT *` shapes ourselves.
//
// "Pass-through" means: no computed columns (`expr_list_size() == 0`)
// AND the project's `column_list` matches the input's `column_list`
// element-for-element by `ResolvedColumn::column_id()`. The second
// check rejects projections that reorder or subset the input scan's
// columns (`SELECT name, id FROM t`, `SELECT id FROM t`) — those
// would otherwise have the DuckDB result rows arrive in a different
// order from the analyzer's `output_column_list` and the per-cell
// shape would silently disagree with the gateway's BigQuery schema.
// Non-strippable shapes return the original scan; the engine then
// returns UNIMPLEMENTED for the wrapping ProjectScan and the
// `--on_unknown_fn=fallback` policy hands the query to the
// reference-impl evaluator.
const ::googlesql::ResolvedScan* StripPassThroughProjectScans(
    const ::googlesql::ResolvedScan* scan) {
  while (scan != nullptr &&
         scan->node_kind() == ::googlesql::RESOLVED_PROJECT_SCAN) {
    const auto* p = scan->GetAs<::googlesql::ResolvedProjectScan>();
    if (p->expr_list_size() != 0) return scan;
    const ::googlesql::ResolvedScan* input = p->input_scan();
    if (input == nullptr) return scan;
    if (p->column_list_size() != input->column_list_size()) return scan;
    bool same_columns = true;
    for (int i = 0; i < p->column_list_size(); ++i) {
      if (p->column_list(i).column_id() !=
          input->column_list(i).column_id()) {
        same_columns = false;
        break;
      }
    }
    if (!same_columns) return scan;
    scan = input;
  }
  return scan;
}

}  // namespace

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> DuckDBEngine::Analyze(
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
        "DuckDBEngine::Analyze: analyzer returned no resolved statement");
  }
  const ::googlesql::ResolvedStatement* stmt = output->resolved_statement();
  if (stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(absl::StrCat(
        "DuckDBEngine::Analyze: only SELECT-shaped queries are "
        "supported; got ", stmt->node_kind_string()));
  }
  absl::StatusOr<schema::TableSchema> output_schema =
      ReflectOutputSchema(*stmt->GetAs<::googlesql::ResolvedQueryStmt>());
  if (!output_schema.ok()) return output_schema.status();
  return std::make_unique<AnalyzedQueryImpl>(std::move(output),
                                              std::move(*output_schema));
}

absl::StatusOr<DryRunResult> DuckDBEngine::DryRun(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> analyzed =
      Analyze(request, catalog);
  if (!analyzed.ok()) return analyzed.status();
  DryRunResult result;
  result.schema = (*analyzed)->output_schema();
  // We don't have a cost model yet (same caveat as the reference-impl
  // engine); surface zero so the gateway's wire envelope stays
  // well-formed.
  result.estimated_bytes_processed = 0;
  return result;
}

absl::StatusOr<std::unique_ptr<RowSource>> DuckDBEngine::ExecuteQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;

  // 1. Analyze. We do this ourselves (rather than through
  // `PreparedQuery::Prepare`) because we need the resolved AST in
  // hand for the transpiler and the table-scan visitor; the
  // algebrizer the reference-impl engine drives is what we are
  // intentionally skipping on this path.
  ::googlesql::AnalyzerOptions analyzer_options = MakeAnalyzerOptions();
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyze = ::googlesql::AnalyzeStatement(
      request.sql, analyzer_options, catalog, &type_factory, &output);
  if (!analyze.ok()) return analyze;
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "DuckDBEngine::ExecuteQuery: analyzer returned no resolved "
        "statement");
  }
  const ::googlesql::ResolvedStatement* stmt = output->resolved_statement();
  if (stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(absl::StrCat(
        "DuckDBEngine::ExecuteQuery: only SELECT-shaped queries are "
        "supported; got ", stmt->node_kind_string()));
  }
  const auto* query_stmt = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  absl::StatusOr<schema::TableSchema> output_schema =
      ReflectOutputSchema(*query_stmt);
  if (!output_schema.ok()) return output_schema.status();

  // 2. Lower the query. The transpiler's `EmitQueryStmt` /
  // `EmitProjectScan` are still `not_started`, so we strip wrapping
  // pass-through `ProjectScan`s (added by the analyzer for
  // `SELECT *` shapes) before dispatching to the transpiler. If the
  // resulting scan does not lower we return UNIMPLEMENTED; the
  // engine factory's `--on_unknown_fn=fallback` wrapper then retries
  // via the reference-impl engine.
  const ::googlesql::ResolvedScan* inner =
      StripPassThroughProjectScans(query_stmt->query());
  if (inner == nullptr) {
    return absl::UnimplementedError(
        "duckdb engine: query has no scan tree");
  }
  transpiler::Transpiler t;
  std::string sql = t.Transpile(inner);
  if (sql.empty()) {
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: transpiler does not yet cover this query shape "
        "(node_kind=", inner->node_kind_string(), ")"));
  }

  // 3. Collect every referenced table so we can materialize them
  // inside the DuckDB connection under the bare names the
  // transpiler emitted.
  TableScanCollector collector;
  absl::Status visit_status = stmt->Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  // 4. Open a fresh in-memory DuckDB. The connection / database are
  // per-query: tables we materialize live only for this RPC and
  // are torn down when the returned RowSource is destroyed.
  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "DuckDBEngine::ExecuteQuery: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError(
        "DuckDBEngine::ExecuteQuery: duckdb_connect failed");
  }

  // 5. Materialize each storage table inside the DuckDB connection.
  // The transpiler assumes `Table::Name()` resolves to a relation
  // already present in the connection's default schema.
  for (const ::googlesql::Table* tbl : collector.tables()) {
    const auto* storage_table =
        dynamic_cast<const catalog::StorageTable*>(tbl);
    if (storage_table == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::UnimplementedError(absl::StrCat(
          "duckdb engine: cannot attach non-StorageTable '", tbl->Name(),
          "'; rebuild against a GoogleSqlCatalog-backed analyzer"));
    }
    absl::Status status =
        AttachStorageTable(conn, storage_, *storage_table);
    if (!status.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return status;
    }
  }

  // 6. Execute the transpiled SQL. We fold a DuckDB rejection into
  // UNIMPLEMENTED instead of INTERNAL so the fallback policy can
  // route the query to the reference-impl engine: a transpiled SQL
  // that DuckDB cannot run is, by definition, a query the DuckDB
  // engine "cannot yet execute".
  ::duckdb_result result;
  if (::duckdb_query(conn, sql.c_str(), &result) != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail =
        err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: DuckDB rejected transpiled SQL: ", detail,
        " (sql=", sql, ")"));
  }
  const idx_t row_count = ::duckdb_row_count(&result);
  return std::unique_ptr<RowSource>(new DuckDBRowSource(
      db, conn, result, std::move(*output_schema), row_count));
}

#endif  // BIGQUERY_EMULATOR_DUCKDB_ENGINE_ENABLED

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
