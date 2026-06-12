#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/virtual_table.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/engine/engine.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

namespace {

const char* DuckDBLibraryVersion() {
  return ::duckdb_library_version();
}

}  // namespace

DuckDbExecutor::DuckDbExecutor(storage::Storage* storage) : storage_(storage) {
  (void)DuckDBLibraryVersion();
}

DuckDbExecutor::~DuckDbExecutor() = default;

namespace internal {

// Build the BigQuery-shaped output schema from the analyzer's
// resolved-output-column list. Routed through the proto round-trip
// so the REPEATED-mode contract for ARRAY columns matches the
// surface the gateway emits.
absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt) {
  v1::TableSchema proto;
  absl::Status s = backend::schema::OutputColumnListToTableSchema(
      stmt.output_column_list(), &proto);
  if (!s.ok()) return s;
  return backend::schema::TableSchemaFromProto(proto);
}

}  // namespace internal

// Streams rows out of a DuckDB result by fetching one columnar data
// chunk at a time -- the C-API equivalent of pulling Arrow
// RecordBatches off the result iterator. Each chunk's vectors are
// what `duckdb_data_chunk_to_arrow` exports, so this is the same
// columnar interface the Storage Read API path will stream straight
// onto the wire.
//
// Each cell is rendered through `arrow_to_bq::ChunkRowToCells` so
// the resulting `storage::Value` shape lines up with what
// `frontend/handlers/query.cc::ValueToCell` lowers onto the proto
// Cell wire shape. The chunked path replaces the previous
// row-at-a-time `duckdb_value_*` accessors so the engine no longer
// pays one C-API call per cell.
//
// Declaration order pins destruction order: chunk_ first (releases
// the columnar buffers it borrowed from result_), then result_, then
// conn_, then db_.
class DuckDBRowSource : public RowSource {
 public:
  DuckDBRowSource(::duckdb_database db,
                  ::duckdb_connection conn,
                  ::duckdb_result result,
                  schema::TableSchema schema)
      : db_(db), conn_(conn), result_(result), schema_(std::move(schema)) {}

  ~DuckDBRowSource() override {
    if (chunk_ != nullptr) ::duckdb_destroy_data_chunk(&chunk_);
    ::duckdb_destroy_result(&result_);
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
  }

  DuckDBRowSource(const DuckDBRowSource&) = delete;
  DuckDBRowSource& operator=(const DuckDBRowSource&) = delete;

  const schema::TableSchema& schema() const override {
    return schema_;
  }

  absl::StatusOr<bool> Next(storage::Row* row) override {
    if (row == nullptr) {
      return absl::InvalidArgumentError(
          "DuckDBEngine row source: Next called with null row");
    }
    while (chunk_ == nullptr || next_in_chunk_ >= chunk_size_) {
      if (chunk_ != nullptr) {
        ::duckdb_destroy_data_chunk(&chunk_);
        chunk_ = nullptr;
      }
      chunk_ = ::duckdb_fetch_chunk(result_);
      if (chunk_ == nullptr) return false;
      chunk_size_ = ::duckdb_data_chunk_get_size(chunk_);
      next_in_chunk_ = 0;
      // A zero-sized chunk can show up at end-of-stream; loop to the
      // next fetch which will hand back nullptr and terminate.
    }
    absl::StatusOr<storage::Row> rendered =
        arrow_to_bq::ChunkRowToCells(chunk_, next_in_chunk_, schema_);
    if (!rendered.ok()) return rendered.status();
    *row = std::move(rendered).value();
    ++next_in_chunk_;
    return true;
  }

 private:
  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
  ::duckdb_result result_{};
  schema::TableSchema schema_{};
  ::duckdb_data_chunk chunk_ = nullptr;
  ::idx_t chunk_size_ = 0;
  ::idx_t next_in_chunk_ = 0;
};

absl::StatusOr<std::unique_ptr<RowSource>> DuckDbExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis has already happened on the coordinator.

  if (stmt.node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(absl::StrCat(
        "DuckDbExecutor::ExecuteQuery: only SELECT-shaped queries are "
        "supported; got ",
        stmt.node_kind_string()));
  }
  const auto* query_stmt = stmt.GetAs<::googlesql::ResolvedQueryStmt>();
  absl::StatusOr<schema::TableSchema> output_schema =
      internal::ReflectOutputSchema(*query_stmt);
  if (!output_schema.ok()) return output_schema.status();

  // 2. Lower the query through the transpiler's `EmitQueryStmt`,
  // which (a) walks the scan tree under the QueryStmt and (b) wraps
  // the result in the outermost `output_column_list()` projection so
  // the DuckDB result columns line up with the analyzer's output
  // schema 1:1. Handing `EmitQueryStmt` the QueryStmt itself
  // (instead of the inner scan, like the legacy path did) is what
  // makes `SELECT id, name FROM ds.t` narrow correctly even when
  // the analyzer is configured with `prune_unused_columns=false`
  // and the underlying TableScan keeps all of `ds.t`'s columns
  // (e.g. an extra `tags` column the SELECT discards): the inner
  // scan emit reflects the table's full column list, but the
  // outermost projection brings the cell stream back down to the 2
  // analyzer-output columns the gateway / `arrow_to_bq` expect.
  //
  // If the transpiler cannot lower the shape (a `""` return per the
  // empty-string contract) we surface UNIMPLEMENTED so the gateway
  // emits BigQuery's `notImplemented` reason. Once the route
  // classifier promotes every property-level fast-path gate (see
  // `route_classifier.cc`'s `Visit*` overrides) the only paths left
  // returning `""` are the genuinely-defensive ones (null pointers,
  // analyzer contract violations); reaching this branch is then a
  // signal of either a missing classifier rule or an analyzer-shape
  // surprise we should pin a fixture against.
  if (query_stmt->query() == nullptr) {
    return absl::UnimplementedError("duckdb engine: query has no scan tree");
  }
  const absl::Time transpile_start = absl::Now();
  transpiler::Transpiler t;
  std::string sql = t.Transpile(query_stmt);
  if (request.phase_recorder != nullptr) {
    request.phase_recorder->Record(
        "transpile", absl::ToInt64Microseconds(absl::Now() - transpile_start));
  }
  if (sql.empty()) {
    const std::string kind = query_stmt->query()->node_kind_string();
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: transpiler does not yet cover this query shape "
        "(family: node:",
        kind,
        ", route: duckdb_native); see docs/ENGINE_POLICY.md"));
  }
  if (!t.parameter_order().empty()) {
    auto substituted = internal::SubstituteDuckdbParameters(
        std::move(sql), t.parameter_order(), request.parameters);
    if (!substituted.ok()) return substituted.status();
    sql = *std::move(substituted);
  }

  // 3. Collect every referenced table so we can materialize them
  // inside the DuckDB connection under the bare names the
  // transpiler emitted.
  internal::TableScanCollector collector;
  absl::Status visit_status = stmt.Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  const absl::Time setup_start = absl::Now();
  // 4. Open a fresh in-memory DuckDB. The connection / database are
  // per-query: tables we materialize live only for this RPC and
  // are torn down when the returned RowSource is destroyed.
  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteQuery: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError(
        "DuckDbExecutor::ExecuteQuery: duckdb_connect failed");
  }

  // 4b. Register the BigQuery polyfill UDF library on the fresh
  // connection. Every BigQuery function whose disposition is
  // `duckdb_udf` lowers to a UDF / macro the registrar installs
  // here; the transpiler then emits a plain `<udf_name>(<args>)`
  // call below. Registration failure is fail-fast: there is no
  // runtime "missing UDF -> fall back to another route" path
  // (per `docs/ENGINE_POLICY.md`'s Done Criterion 2).
  if (auto reg = udf::RegisterAll(conn); !reg.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return reg;
  }

  // 5. Materialize each storage table inside the DuckDB connection.
  // The transpiler assumes `Table::Name()` resolves to a relation
  // already present in the connection's default schema.
  for (const ::googlesql::Table* tbl : collector.tables()) {
    if (const auto* virtual_table =
            dynamic_cast<const catalog::VirtualCatalogTable*>(tbl)) {
      absl::Status status = virtual_table->MaterializeInDuckDB(
          conn, storage_, internal::QuoteIdent(tbl->Name()));
      if (!status.ok()) {
        ::duckdb_disconnect(&conn);
        ::duckdb_close(&db);
        return status;
      }
      continue;
    }
    const auto* storage_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (storage_table == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::UnimplementedError(absl::StrCat(
          "duckdb engine: cannot attach non-StorageTable '",
          tbl->Name(),
          "'; rebuild against a GoogleSqlCatalog-backed analyzer"));
    }
    absl::Status status =
        internal::AttachStorageTable(conn, storage_, *storage_table);
    if (!status.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return status;
    }
  }

  if (request.phase_recorder != nullptr) {
    request.phase_recorder->Record(
        "duckdb_setup", absl::ToInt64Microseconds(absl::Now() - setup_start));
  }

  // 6. Execute the transpiled SQL. A DuckDB rejection folds into
  // UNIMPLEMENTED instead of INTERNAL because a transpiled SQL that
  // DuckDB cannot run is, by definition, a query the DuckDB engine
  // "cannot yet execute".
  const absl::Time execute_start = absl::Now();
  ::duckdb_result result;
  if (::duckdb_query(conn, sql.c_str(), &result) != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::UnimplementedError(
        absl::StrCat("duckdb engine: DuckDB rejected transpiled SQL: ",
                     detail,
                     " (sql=",
                     sql,
                     ")"));
  }
  if (request.phase_recorder != nullptr) {
    request.phase_recorder->Record(
        "duckdb_execute",
        absl::ToInt64Microseconds(absl::Now() - execute_start));
  }
  return std::unique_ptr<RowSource>(
      new DuckDBRowSource(db, conn, result, std::move(*output_schema)));
}

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
