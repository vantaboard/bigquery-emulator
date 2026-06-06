#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INTERNAL_H_

// Shared helpers for the DuckDB executor translation units.

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"

namespace googlesql {
class ResolvedAlterTableStmt;
class ResolvedQueryStmt;
class Table;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

std::string QuoteIdent(absl::string_view ident);

absl::StatusOr<std::string> SubstituteDuckdbParameters(
    std::string sql,
    const std::vector<transpiler::Transpiler::ParameterRef>& order,
    absl::Span<const QueryParameter> parameters);

absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql);

absl::Status AttachStorageTableAt(::duckdb_connection conn,
                                  storage::Storage* storage,
                                  const catalog::StorageTable& table,
                                  absl::string_view quoted_table_name);

absl::Status AttachStorageTable(::duckdb_connection conn,
                                storage::Storage* storage,
                                const catalog::StorageTable& table);

absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt);

class TableScanCollector : public ::googlesql::ResolvedASTVisitor {
 public:
  absl::Status VisitResolvedTableScan(
      const ::googlesql::ResolvedTableScan* node) override {
    if (node == nullptr) return absl::OkStatus();
    if (node->table() != nullptr) {
      tables_.insert(node->table());
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedTableScan(node);
  }

  const std::set<const ::googlesql::Table*>& tables() const {
    return tables_;
  }

 private:
  std::set<const ::googlesql::Table*> tables_{};
};

std::string SerializeForPkLookup(const storage::Value& value);

bool ValuesEqual(const storage::Value& a, const storage::Value& b);

absl::StatusOr<std::vector<storage::Row>> DrainResultToRows(
    ::duckdb_result* result, const schema::TableSchema& schema);

absl::StatusOr<std::vector<storage::Row>> ReadBackTable(
    ::duckdb_connection conn,
    absl::string_view quoted_table_name,
    const schema::TableSchema& bq_schema);

DmlStats DiffByPrimaryKey(absl::Span<const storage::Row> before,
                          absl::Span<const storage::Row> after);

absl::StatusOr<DmlStats> RunInsertSelect(
    const QueryRequest& request,
    storage::Storage* storage,
    const ::googlesql::ResolvedInsertStmt& insert);

absl::Status RunAlterTableAddColumn(
    storage::Storage& storage,
    absl::string_view project_id,
    const ::googlesql::ResolvedAlterTableStmt* stmt);

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INTERNAL_H_
