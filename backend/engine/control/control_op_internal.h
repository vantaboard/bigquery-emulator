#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_INTERNAL_H_

// Shared helpers for the control-op executor translation units.
// Literal rendering lives in `control_op_literals.cc`; DDL handlers
// live in `control_op_ddl.cc`; the public dispatch surface is
// `control_op_executor.cc`.

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"

namespace googlesql {
class ResolvedAnalyzeStmt;
class ResolvedCreateTableAsSelectStmt;
class ResolvedCreateTableStmt;
class ResolvedDropStmt;
class Table;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

std::string QuoteIdent(absl::string_view ident);

absl::StatusOr<std::string> SubstituteDuckdbParameters(
    std::string sql,
    const std::vector<duckdb::transpiler::Transpiler::ParameterRef>& order,
    absl::Span<const QueryParameter> parameters);

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

absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql);

absl::Status AttachStorageTableAt(::duckdb_connection conn,
                                  storage::Storage* storage,
                                  const catalog::StorageTable& table,
                                  absl::string_view quoted_table_name);

absl::StatusOr<std::vector<storage::Row>> DrainTableRows(
    ::duckdb_connection conn,
    absl::string_view quoted_table_name,
    const schema::TableSchema& bq_schema);

absl::StatusOr<storage::TableId> NamePathToTableId(
    const std::vector<std::string>& name_path,
    absl::string_view default_project_id,
    absl::string_view default_dataset_id);

absl::Status EnsureDatasetExists(storage::Storage& storage,
                                 absl::string_view project_id,
                                 absl::string_view dataset_id);

absl::StatusOr<schema::TableSchema> ColumnDefinitionListToTableSchema(
    const std::vector<
        std::unique_ptr<const ::googlesql::ResolvedColumnDefinition>>&
        column_definition_list);

absl::Status ApplyCreateMode(
    absl::Status existing_status,
    ::googlesql::ResolvedCreateStatement::CreateMode create_mode);

absl::Status RunCreateTable(storage::Storage& storage,
                            absl::string_view project_id,
                            absl::string_view default_dataset_id,
                            const ::googlesql::ResolvedCreateTableStmt* stmt);

absl::StatusOr<std::string> BuildDuckdbCtasSql(
    absl::string_view request_sql,
    const ::googlesql::ResolvedCreateTableAsSelectStmt* stmt,
    const storage::TableId& target,
    const schema::TableSchema& bq_schema,
    absl::Span<const QueryParameter> parameters);

absl::Status RunCreateTableAsSelect(
    storage::Storage& storage,
    absl::string_view project_id,
    const QueryRequest& request,
    const ::googlesql::ResolvedCreateTableAsSelectStmt* stmt,
    const ::googlesql::ResolvedStatement* root_stmt);

absl::Status RunDropTable(storage::Storage& storage,
                          absl::string_view project_id,
                          absl::string_view default_dataset_id,
                          const ::googlesql::ResolvedDropStmt* stmt);

absl::Status RunAnalyze(storage::Storage& storage,
                        absl::string_view project_id,
                        const ::googlesql::ResolvedAnalyzeStmt* stmt);

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_INTERNAL_H_
