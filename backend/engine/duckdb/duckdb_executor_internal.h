#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INTERNAL_H_

// Shared helpers for the DuckDB executor translation units.

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/wildcard_table_suffix_filter.h"
#include "backend/engine/duckdb/duckdb_executor_security.h"
#include "backend/engine/duckdb/duckdb_executor_time_travel.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/engine.h"
#include "backend/engine/phase_recorder.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"

namespace googlesql {
class ResolvedQueryStmt;
class Table;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

std::string QuoteIdent(absl::string_view ident);
std::string EscapeStringLiteralInner(absl::string_view s);
std::string RenderColumnList(const schema::TableSchema& schema);
void RecordPhase(PhaseRecorder* recorder,
                 absl::string_view name,
                 absl::Duration elapsed);
absl::StatusOr<std::string> RenderCellLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column);

absl::StatusOr<std::string> SubstituteDuckdbParameters(
    std::string sql,
    const std::vector<transpiler::Transpiler::ParameterRef>& order,
    absl::Span<const QueryParameter> parameters);

absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql);

absl::Status AttachStorageTableAt(
    ::duckdb_connection conn,
    storage::Storage* storage,
    const catalog::StorageTable& table,
    absl::string_view quoted_table_name,
    std::optional<std::int64_t> as_of_ms = std::nullopt,
    absl::string_view row_access_filter_sql = {},
    PhaseRecorder* phase_recorder = nullptr);

absl::Status AttachStorageTable(
    ::duckdb_connection conn,
    storage::Storage* storage,
    const catalog::StorageTable& table,
    std::optional<std::int64_t> as_of_ms = std::nullopt,
    absl::string_view row_access_filter_sql = {},
    PhaseRecorder* phase_recorder = nullptr);

absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt);

class TableScanCollector : public ::googlesql::ResolvedASTVisitor {
 public:
  absl::Status VisitResolvedTableScan(
      const ::googlesql::ResolvedTableScan* node) override {
    if (node == nullptr) return absl::OkStatus();
    if (node->table() != nullptr) {
      tables_.insert(node->table());
      if (node->for_system_time_expr() != nullptr) {
        absl::StatusOr<std::int64_t> as_of_ms =
            EvaluateForSystemTimeAsOfMs(*node->for_system_time_expr());
        if (!as_of_ms.ok()) return as_of_ms.status();
        system_time_as_of_ms_[node->table()] = *as_of_ms;
      }
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedTableScan(node);
  }

  absl::Status VisitResolvedFilterScan(
      const ::googlesql::ResolvedFilterScan* node) override {
    if (node != nullptr && node->input_scan() != nullptr) {
      const ::googlesql::ResolvedScan* input = node->input_scan();
      if (input->node_kind() == ::googlesql::RESOLVED_TABLE_SCAN) {
        const auto* table_scan = input->GetAs<::googlesql::ResolvedTableScan>();
        if (table_scan != nullptr && table_scan->table() != nullptr) {
          if (auto allow = catalog::FindTableSuffixAllowListForWildcardScan(
                  node, table_scan->table()->Name())) {
            wildcard_suffix_allowlists_[table_scan->table()] = *allow;
          }
        }
      }
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedFilterScan(node);
  }

  const std::set<const ::googlesql::Table*>& tables() const {
    return tables_;
  }

  std::optional<std::vector<std::string>> WildcardSuffixAllowList(
      const ::googlesql::Table* table) const {
    if (table == nullptr) return std::nullopt;
    auto it = wildcard_suffix_allowlists_.find(table);
    if (it == wildcard_suffix_allowlists_.end()) return std::nullopt;
    return it->second;
  }

  std::optional<std::int64_t> SystemTimeAsOfMs(
      const ::googlesql::Table* table) const {
    if (table == nullptr) return std::nullopt;
    auto it = system_time_as_of_ms_.find(table);
    if (it == system_time_as_of_ms_.end()) return std::nullopt;
    return it->second;
  }

  void SetRowAccessFilter(const ::googlesql::Table* table,
                          std::string filter_sql) {
    if (table == nullptr || filter_sql.empty()) return;
    row_access_filter_sql_[table] = std::move(filter_sql);
  }

  std::optional<std::string> RowAccessFilterSql(
      const ::googlesql::Table* table) const {
    if (table == nullptr) return std::nullopt;
    auto it = row_access_filter_sql_.find(table);
    if (it == row_access_filter_sql_.end()) return std::nullopt;
    return it->second;
  }

 private:
  std::set<const ::googlesql::Table*> tables_{};
  std::map<const ::googlesql::Table*, std::int64_t> system_time_as_of_ms_{};
  std::map<const ::googlesql::Table*, std::vector<std::string>>
      wildcard_suffix_allowlists_{};
  std::map<const ::googlesql::Table*, std::string> row_access_filter_sql_{};
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

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INTERNAL_H_
