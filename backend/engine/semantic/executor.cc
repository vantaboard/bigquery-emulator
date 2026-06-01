#include "backend/engine/semantic/executor.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_column.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

// Resolve every `QueryParameter` carried on the request into a
// `googlesql::Value` for the matching name / positional slot. The
// analyzer has already declared each parameter's type
// (`AddQueryParameter` was called before `AnalyzeStatement`) so the
// values lined up here only need to match those declared types --
// the analyzer's `ResolvedParameter` nodes guarantee the type tag
// the evaluator sees inside `EvalExpr`.
absl::StatusOr<ParameterBindings> BuildParameterBindings(
    const QueryRequest& request) {
  ParameterBindings bindings;
  bool any_positional = false;
  bool any_named = false;
  for (const QueryParameter& p : request.parameters) {
    auto value = ParseParameterValue(p.value_json, p.type_kind);
    if (!value.ok()) return value.status();
    if (p.name.empty()) {
      bindings.by_position.push_back(*std::move(value));
      any_positional = true;
    } else {
      bindings.by_name[absl::AsciiStrToLower(p.name)] = *std::move(value);
      any_named = true;
    }
  }
  if (any_positional && any_named) {
    return absl::InvalidArgumentError(
        "semantic: request mixes named and positional parameters");
  }
  return bindings;
}

// Check that `query_stmt` matches the scalar-only SELECT shape the
// executor handles: `ResolvedProjectScan` over `ResolvedSingleRowScan`
// with no FROM-side columns referenced. Returns the inner
// `ResolvedProjectScan*` when accepted, or NOT_IMPLEMENTED naming
// the downstream plan that picks up the remaining shape.
absl::StatusOr<const ::googlesql::ResolvedProjectScan*> AcceptScalarOnlyShape(
    const ::googlesql::ResolvedQueryStmt& stmt) {
  if (stmt.is_value_table()) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: SELECT AS VALUE / VALUE TABLE shapes are owned by "
        "array-struct-semantic-path.plan.md");
  }
  const ::googlesql::ResolvedScan* query = stmt.query();
  if (query == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedQueryStmt has no inner scan");
  }
  if (query->node_kind() != ::googlesql::RESOLVED_PROJECT_SCAN) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat(
            "semantic: scalar-only SELECT path expects a ProjectScan; got ",
            query->node_kind_string()));
  }
  const auto* project = query->GetAs<::googlesql::ResolvedProjectScan>();
  const ::googlesql::ResolvedScan* input = project->input_scan();
  if (input == nullptr ||
      input->node_kind() != ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat(
            "semantic: scalar-only SELECT path expects a SingleRowScan "
            "(no FROM); got ",
            input == nullptr ? "<null>" : input->node_kind_string(),
            ". The FROM-clause path is owned by "
            "array-struct-semantic-path.plan.md / "
            "cte-subquery-routing.plan.md"));
  }
  return project;
}

}  // namespace

SemanticExecutor::~SemanticExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> SemanticExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;
  if (stmt.node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat(
            "semantic: only SELECT-shaped statements route here today; got ",
            stmt.node_kind_string()));
  }
  const auto& query_stmt = *stmt.GetAs<::googlesql::ResolvedQueryStmt>();
  absl::StatusOr<const ::googlesql::ResolvedProjectScan*> project_or =
      AcceptScalarOnlyShape(query_stmt);
  if (!project_or.ok()) return project_or.status();
  const ::googlesql::ResolvedProjectScan& project = **project_or;

  ParameterBindings bindings;
  if (!request.parameters.empty()) {
    auto built = BuildParameterBindings(request);
    if (!built.ok()) return built.status();
    bindings = *std::move(built);
  }
  EvalContext ctx;
  ctx.parameters = &bindings;

  // Build `column_id -> ResolvedExpr*` from the project's
  // `expr_list`. The output column list references projects by
  // `ResolvedColumn` (which carries the analyzer-assigned column
  // id); we need to look up the matching expression in the
  // computed-column list.
  absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*> expr_by_column_id;
  for (int i = 0; i < project.expr_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc = project.expr_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError(
          "semantic: ResolvedComputedColumn has null expr");
    }
    expr_by_column_id[cc->column().column_id()] = cc->expr();
  }

  // Walk the output column list (the analyzer-canonical projection
  // order) and evaluate each expression into a `storage::Value`.
  schema::TableSchema output_schema;
  storage::Row row;
  output_schema.columns.reserve(query_stmt.output_column_list_size());
  row.cells.reserve(query_stmt.output_column_list_size());
  for (int i = 0; i < query_stmt.output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* oc =
        query_stmt.output_column_list(i);
    if (oc == nullptr) {
      return absl::InternalError("semantic: ResolvedOutputColumn is null");
    }
    auto schema_or = ColumnSchemaForType(oc->column().type(), oc->name());
    if (!schema_or.ok()) return schema_or.status();
    output_schema.columns.push_back(*std::move(schema_or));

    auto it = expr_by_column_id.find(oc->column().column_id());
    if (it == expr_by_column_id.end()) {
      return absl::InternalError(absl::StrCat(
          "semantic: output column '",
          oc->name(),
          "' has no matching expression in ProjectScan.expr_list"));
    }
    auto value_or = EvalExpr(*it->second, ctx);
    if (!value_or.ok()) return value_or.status();
    auto cell = ToStorageValue(*value_or);
    if (!cell.ok()) return cell.status();
    row.cells.push_back(*std::move(cell));
  }

  std::vector<storage::Row> rows;
  rows.push_back(std::move(row));
  return std::unique_ptr<RowSource>(
      new MaterializedRowSource(std::move(output_schema), std::move(rows)));
}

absl::StatusOr<DmlStats> SemanticExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat(
          "semantic: ExecuteDml on route 'semantic_executor' is owned by "
          "dml-local-executor.plan.md; got statement kind ",
          stmt.node_kind_string()));
}

absl::Status SemanticExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: ExecuteDdl is not a semantic-executor route; "
                   "got statement kind ",
                   stmt.node_kind_string()));
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
