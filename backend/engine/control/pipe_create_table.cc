#include "backend/engine/control/pipe_create_table.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace {

using ::bigquery_emulator::backend::engine::semantic::ColumnBindings;
using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::MaterializeScan;
using ::bigquery_emulator::backend::engine::semantic::ToStorageValue;

absl::StatusOr<std::vector<storage::Row>> MaterializedRowsForCtas(
    const ::googlesql::ResolvedCreateTableAsSelectStmt& ctas,
    const std::vector<ColumnBindings>& rows,
    const schema::TableSchema& table_schema) {
  if (ctas.output_column_list_size() != table_schema.columns.size()) {
    return absl::InternalError(
        "semantic: pipe CTAS output_column_list size mismatch");
  }
  std::vector<int> source_col_ids;
  source_col_ids.reserve(ctas.output_column_list_size());
  for (int i = 0; i < ctas.output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* oc = ctas.output_column_list(i);
    if (oc == nullptr) {
      return absl::InternalError("semantic: pipe CTAS output column is null");
    }
    source_col_ids.push_back(oc->column().column_id());
  }

  std::vector<storage::Row> out;
  out.reserve(rows.size());
  for (const ColumnBindings& bind : rows) {
    storage::Row row;
    row.cells.assign(table_schema.columns.size(), storage::Value::Null());
    for (size_t i = 0; i < source_col_ids.size(); ++i) {
      auto it = bind.find(source_col_ids[i]);
      if (it == bind.end()) {
        return absl::InternalError(absl::StrCat(
            "semantic: pipe CTAS row missing column_id=", source_col_ids[i]));
      }
      auto cell = ToStorageValue(it->second);
      if (!cell.ok()) return cell.status();
      row.cells[i] = *std::move(cell);
    }
    out.push_back(std::move(row));
  }
  return out;
}

}  // namespace

absl::StatusOr<std::unique_ptr<RowSource>> RunPipeCreateTable(
    storage::Storage& storage,
    ::googlesql::Catalog* catalog,
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt) {
  (void)catalog;
  const ::googlesql::ResolvedScan* query = nullptr;
  if (stmt.node_kind() == ::googlesql::RESOLVED_QUERY_STMT) {
    query = stmt.GetAs<::googlesql::ResolvedQueryStmt>()->query();
  } else if (stmt.node_kind() == ::googlesql::RESOLVED_GENERALIZED_QUERY_STMT) {
    query = stmt.GetAs<::googlesql::ResolvedGeneralizedQueryStmt>()->query();
  } else {
    return absl::InternalError(
        absl::StrCat("RunPipeCreateTable: coordinator dispatched the wrong "
                     "statement kind (expected ResolvedQueryStmt or "
                     "ResolvedGeneralizedQueryStmt; got ",
                     stmt.node_kind_string(),
                     ")"));
  }
  if (query == nullptr ||
      query->node_kind() != ::googlesql::RESOLVED_PIPE_CREATE_TABLE_SCAN) {
    return absl::InternalError(absl::StrCat(
        "RunPipeCreateTable: coordinator dispatched the wrong "
        "statement body (expected ResolvedPipeCreateTableScan; got ",
        query == nullptr ? "null" : query->node_kind_string(),
        ")"));
  }
  const auto* pipe_scan =
      query->GetAs<::googlesql::ResolvedPipeCreateTableScan>();
  const ::googlesql::ResolvedCreateTableAsSelectStmt* ctas =
      pipe_scan->create_table_as_select_stmt();
  if (ctas == nullptr || ctas->query() == nullptr) {
    return absl::InternalError(
        "RunPipeCreateTable: pipe CREATE TABLE missing inner query scan");
  }

  EvalContext ctx;
  ctx.project_id = request.project_id;
  auto materialized = MaterializeScan(ctas->query(), ctx);
  if (!materialized.ok()) return materialized.status();

  absl::StatusOr<storage::TableId> target = internal::NamePathToTableId(
      ctas->name_path(), request.project_id, request.default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds = internal::EnsureDatasetExists(
          storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }

  absl::StatusOr<schema::TableSchema> bq_schema =
      internal::ColumnDefinitionListToTableSchema(
          ctas->column_definition_list());
  if (!bq_schema.ok()) return bq_schema.status();

  if (ctas->create_mode() ==
      ::googlesql::ResolvedCreateStatement::CREATE_OR_REPLACE) {
    absl::Status dropped = storage.DropTable(*target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      return dropped;
    }
  }

  absl::StatusOr<std::vector<storage::Row>> rows =
      MaterializedRowsForCtas(*ctas, *materialized, *bq_schema);
  if (!rows.ok()) return rows.status();

  absl::Status created = internal::ApplyCreateMode(
      storage.CreateTable(*target, *bq_schema), ctas->create_mode());
  if (!created.ok()) return created;
  if (!rows->empty()) {
    absl::Status appended =
        // cpp-lint:allow(status-discarded) -- captured into appended
        storage.AppendRows(*target, absl::MakeConstSpan(*rows));
    if (!appended.ok()) return appended;
  }
  return std::make_unique<semantic::MaterializedRowSource>(
      schema::TableSchema{}, std::vector<storage::Row>{});
}

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
