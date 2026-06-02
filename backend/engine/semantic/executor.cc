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
#include "backend/engine/semantic/array_struct/array_scan.h"
#include "backend/engine/semantic/dml/dml_executor.h"
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

// Look through `ResolvedBarrierScan` wrappers, returning the
// underlying input scan. Barriers are optimizer-only markers (the
// analyzer inserts them around pipe-operator boundaries to block
// fusion across the barrier); the row stream itself is unchanged.
// The semantic executor passes them through transparently, so a
// barrier-wrapped SingleRowScan / ArrayScan still routes through
// the scalar-only / UNNEST-rooted handlers. Nested barriers are
// stripped recursively. See `advanced-relational-routing.plan.md`
// Family 2.
const ::googlesql::ResolvedScan* StripBarrierScans(
    const ::googlesql::ResolvedScan* scan) {
  while (scan != nullptr &&
         scan->node_kind() == ::googlesql::RESOLVED_BARRIER_SCAN) {
    scan = scan->GetAs<::googlesql::ResolvedBarrierScan>()->input_scan();
  }
  return scan;
}

// Check that `query_stmt` matches a shape the executor handles
// today and return the inner `ResolvedProjectScan*`.
//
// Supported shapes:
//
//   * Scalar-only SELECT: `ResolvedProjectScan(input_scan=
//     ResolvedSingleRowScan)`. Owned by
//     `.cursor/plans/semantic-executor-core.plan.md`.
//   * UNNEST-rooted SELECT: `ResolvedProjectScan(input_scan=
//     ResolvedArrayScan)`. Owned by Families 1-3 of
//     `.cursor/plans/array-struct-semantic-path.plan.md`.
//   * Barrier-wrapped variant of either of the above:
//     `ResolvedProjectScan(input_scan=ResolvedBarrierScan(...))`.
//     The barrier is a planner-only marker for pipe-operator
//     fusion; rows pass through unchanged. Owned by
//     `advanced-relational-routing.plan.md` Family 2.
//
// Anything else (FROM <table>, JOIN, AGGREGATE, ...) surfaces a
// structured `kNotImplemented` so the gateway envelope is the
// same as for any other "planned but not landed" route.
absl::StatusOr<const ::googlesql::ResolvedProjectScan*> AcceptKnownShape(
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
        absl::StrCat("semantic: SELECT path expects a ProjectScan; got ",
                     query->node_kind_string()));
  }
  const auto* project = query->GetAs<::googlesql::ResolvedProjectScan>();
  const ::googlesql::ResolvedScan* input =
      StripBarrierScans(project->input_scan());
  if (input == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedProjectScan has no input_scan");
  }
  switch (input->node_kind()) {
    case ::googlesql::RESOLVED_SINGLE_ROW_SCAN:
      // scalar-only SELECT -- owned by semantic-executor-core.
      return project;
    case ::googlesql::RESOLVED_ARRAY_SCAN:
      // UNNEST-rooted SELECT -- owned by Families 1-3 of
      // `array-struct-semantic-path.plan.md`. The array-scan
      // evaluator surfaces `kNotImplemented` for the correlated
      // / FLATTEN subsets it does not yet handle, so the
      // executor's accept path stays uniform.
      return project;
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat(
              "semantic: SELECT path expects a SingleRowScan or ArrayScan "
              "input; got ",
              input->node_kind_string(),
              ". Other FROM-clause shapes are owned by "
              "array-struct-semantic-path.plan.md / "
              "cte-subquery-routing.plan.md"));
  }
}

// Evaluate one row of the project against the row-local bindings.
// Pulled out so the scalar-only path (one empty binding) and the
// array-scan path (N bindings) share the projection-evaluation
// code.
absl::StatusOr<storage::Row> ProjectOneRow(
    const ::googlesql::ResolvedQueryStmt& query_stmt,
    const ::googlesql::ResolvedProjectScan& project,
    const absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*>&
        expr_by_column_id,
    EvalContext& ctx) {
  storage::Row row;
  row.cells.reserve(query_stmt.output_column_list_size());
  for (int i = 0; i < query_stmt.output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* oc =
        query_stmt.output_column_list(i);
    if (oc == nullptr) {
      return absl::InternalError("semantic: ResolvedOutputColumn is null");
    }
    const int col_id = oc->column().column_id();
    Value v;
    auto eit = expr_by_column_id.find(col_id);
    if (eit != expr_by_column_id.end()) {
      auto eval_v = EvalExpr(*eit->second, ctx);
      if (!eval_v.ok()) return eval_v.status();
      v = *std::move(eval_v);
    } else if (ctx.columns != nullptr) {
      // Pass-through from the input scan: the analyzer attached
      // no `expr_list` entry for this column id, so the value
      // comes directly from the row's `ColumnBindings`.
      auto cit = ctx.columns->find(col_id);
      if (cit == ctx.columns->end()) {
        return absl::InternalError(absl::StrCat(
            "semantic: output column '",
            oc->name(),
            "' has no expression in ProjectScan.expr_list and no row "
            "binding for column_id=",
            col_id));
      }
      v = cit->second;
    } else {
      return absl::InternalError(absl::StrCat(
          "semantic: output column '",
          oc->name(),
          "' has no matching expression in ProjectScan.expr_list and the "
          "current scan provides no row bindings"));
    }
    auto cell = ToStorageValue(v);
    if (!cell.ok()) return cell.status();
    row.cells.push_back(*std::move(cell));
  }
  (void)project;
  return row;
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
      AcceptKnownShape(query_stmt);
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
  // computed-column list. Pass-through columns (no entry in
  // `expr_list`) come from the input scan's row bindings.
  absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*> expr_by_column_id;
  for (int i = 0; i < project.expr_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc = project.expr_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError(
          "semantic: ResolvedComputedColumn has null expr");
    }
    expr_by_column_id[cc->column().column_id()] = cc->expr();
  }

  // Build the output schema. The schema is row-independent; we
  // populate it once and reuse across every emitted row.
  schema::TableSchema output_schema;
  output_schema.columns.reserve(query_stmt.output_column_list_size());
  for (int i = 0; i < query_stmt.output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* oc =
        query_stmt.output_column_list(i);
    if (oc == nullptr) {
      return absl::InternalError("semantic: ResolvedOutputColumn is null");
    }
    auto schema_or = ColumnSchemaForType(oc->column().type(), oc->name());
    if (!schema_or.ok()) return schema_or.status();
    output_schema.columns.push_back(*std::move(schema_or));
  }

  // Dispatch on the input scan kind:
  //
  //   * `RESOLVED_SINGLE_ROW_SCAN` -- scalar-only SELECT. Evaluate
  //     each output column against an empty row binding.
  //   * `RESOLVED_ARRAY_SCAN` -- UNNEST-rooted SELECT (Families
  //     1-3). Walk the array-scan evaluator to materialize the
  //     row bindings, then evaluate each output column per row.
  //
  // `ResolvedBarrierScan` wrappers are stripped here too -- the
  // barrier is a planner-only optimizer marker (pipe-operator
  // fusion blocker). Rows pass through unchanged, so the inner
  // SingleRowScan / ArrayScan handler is what we actually need to
  // dispatch on. Owned by `advanced-relational-routing.plan.md`
  // Family 2.
  //
  // `AcceptKnownShape` already rejected every other input kind.
  std::vector<storage::Row> rows;
  const ::googlesql::ResolvedScan* input =
      StripBarrierScans(project.input_scan());
  if (input->node_kind() == ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
    auto row = ProjectOneRow(query_stmt, project, expr_by_column_id, ctx);
    if (!row.ok()) return row.status();
    rows.push_back(*std::move(row));
  } else {
    // `AcceptKnownShape` guarantees an ArrayScan here.
    const auto& array_scan = *input->GetAs<::googlesql::ResolvedArrayScan>();
    auto array_rows = array_struct::EvaluateArrayScan(array_scan, ctx);
    if (!array_rows.ok()) return array_rows.status();
    rows.reserve(array_rows->size());
    for (const ColumnBindings& bind : *array_rows) {
      ctx.columns = &bind;
      auto row = ProjectOneRow(query_stmt, project, expr_by_column_id, ctx);
      if (!row.ok()) return row.status();
      rows.push_back(*std::move(row));
    }
    ctx.columns = nullptr;
  }

  return std::unique_ptr<RowSource>(
      new MaterializedRowSource(std::move(output_schema), std::move(rows)));
}

absl::StatusOr<DmlStats> SemanticExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  return dml::ExecuteDml(request, stmt, catalog, storage_);
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
