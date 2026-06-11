#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

namespace {

absl::StatusOr<schema::TableSchema> BuildReturningSchema(
    const ::googlesql::ResolvedReturningClause& returning) {
  schema::TableSchema schema;
  schema.columns.reserve(returning.output_column_list_size());
  for (int i = 0; i < returning.output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* oc =
        returning.output_column_list(i);
    if (oc == nullptr) {
      return absl::InternalError(
          "semantic/dml: ResolvedReturningClause output column is null");
    }
    auto col = ColumnSchemaForType(oc->column().type(), oc->name());
    if (!col.ok()) return col.status();
    schema.columns.push_back(*std::move(col));
  }
  return schema;
}

absl::StatusOr<storage::Row> ProjectReturningRow(
    const ::googlesql::ResolvedReturningClause& returning,
    const ColumnBindings& row_ctx,
    absl::string_view action,
    EvalContext& ctx) {
  absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*> expr_by_id;
  for (int i = 0; i < returning.expr_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc = returning.expr_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError(
          "semantic/dml: returning computed column has null expr");
    }
    expr_by_id[cc->column().column_id()] = cc->expr();
  }

  storage::Row out;
  out.cells.reserve(returning.output_column_list_size());
  ctx.columns = &row_ctx;
  for (int i = 0; i < returning.output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* oc =
        returning.output_column_list(i);
    if (oc == nullptr) {
      return absl::InternalError(
          "semantic/dml: returning output column is null");
    }
    const int col_id = oc->column().column_id();
    Value v;
    if (returning.action_column() != nullptr &&
        col_id == returning.action_column()->column().column_id()) {
      v = Value::String(std::string(action));
    } else {
      auto eit = expr_by_id.find(col_id);
      if (eit != expr_by_id.end()) {
        auto eval_v = EvalExpr(*eit->second, ctx);
        if (!eval_v.ok()) return eval_v.status();
        v = *std::move(eval_v);
      } else {
        auto cit = row_ctx.find(col_id);
        if (cit == row_ctx.end()) {
          return absl::InternalError(
              absl::StrCat("semantic/dml: returning column '",
                           oc->name(),
                           "' has no binding for column_id=",
                           col_id));
        }
        v = cit->second;
      }
    }
    auto cell = ToStorageValue(v, &ctx);
    if (!cell.ok()) return cell.status();
    out.cells.push_back(*std::move(cell));
  }
  ctx.columns = nullptr;
  return out;
}

}  // namespace

absl::StatusOr<std::unique_ptr<RowSource>> BuildReturningRowSource(
    const ::googlesql::ResolvedReturningClause& returning,
    const std::vector<ColumnBindings>& row_contexts,
    const std::vector<std::string>& actions,
    EvalContext& ctx) {
  if (row_contexts.size() != actions.size()) {
    return absl::InternalError(
        "semantic/dml: returning row contexts and action labels size mismatch");
  }
  auto schema_or = BuildReturningSchema(returning);
  if (!schema_or.ok()) return schema_or.status();

  std::vector<storage::Row> rows;
  rows.reserve(row_contexts.size());
  for (size_t i = 0; i < row_contexts.size(); ++i) {
    auto projected =
        ProjectReturningRow(returning, row_contexts[i], actions[i], ctx);
    if (!projected.ok()) return projected.status();
    rows.push_back(*std::move(projected));
  }
  return std::unique_ptr<RowSource>(
      new MaterializedRowSource(*std::move(schema_or), std::move(rows)));
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
