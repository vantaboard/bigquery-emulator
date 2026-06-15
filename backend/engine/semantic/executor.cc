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
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/dml/dml_executor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/explain_stmt.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/script/assert_stmt.h"
#include "backend/engine/semantic/script/assignment_stmt.h"
#include "backend/engine/semantic/script/declare_stmt.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "backend/engine/semantic/system_variables.h"
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

absl::StatusOr<ParameterBindings> BuildParameterBindings(
    const QueryRequest& request) {
  ParameterBindings bindings;
  bool any_positional = false;
  bool any_named = false;
  for (const QueryParameter& p : request.parameters) {
    auto value = ParseParameterValue(p.value_json, p.type_kind, p.type_json);
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

absl::StatusOr<storage::Row> ProjectOneRow(
    const ::googlesql::ResolvedQueryStmt& query_stmt,
    const absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*>&
        expr_by_column_id,
    const ColumnBindings& row_bindings,
    EvalContext& ctx) {
  storage::Row row;
  row.cells.reserve(query_stmt.output_column_list_size());
  ctx.columns = &row_bindings;
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
    } else {
      auto cit = row_bindings.find(col_id);
      if (cit == row_bindings.end()) {
        return absl::InternalError(absl::StrCat(
            "semantic: output column '",
            oc->name(),
            "' has no expression and no row binding for column_id=",
            col_id));
      }
      v = cit->second;
    }
    auto cell = ToStorageValue(v, &ctx);
    if (!cell.ok()) return cell.status();
    row.cells.push_back(*std::move(cell));
  }
  return row;
}

}  // namespace

SemanticExecutor::~SemanticExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> ExecuteResolvedQueryStmt(
    const QueryRequest& request,
    const ::googlesql::ResolvedQueryStmt& query_stmt,
    const FrameStack* script_variables,
    const ::googlesql::SystemVariableValuesMap* script_system_variables) {
  const absl::Time execute_start = absl::Now();
  const ::googlesql::ResolvedScan* query = query_stmt.query();
  if (query == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedQueryStmt has no inner scan");
  }

  absl::StatusOr<const ::googlesql::ResolvedProjectScan*> project_or =
      FindOutputProjectScan(query);
  if (!project_or.ok()) return project_or.status();
  const ::googlesql::ResolvedProjectScan* project = *project_or;

  ParameterBindings bindings;
  if (!request.parameters.empty()) {
    auto built = BuildParameterBindings(request);
    if (!built.ok()) return built.status();
    bindings = *std::move(built);
  }
  EvalContext ctx;
  ctx.project_id = request.project_id;
  ctx.parameters = &bindings;
  ctx.script_variables = script_variables;
  ctx.script_system_variables = script_system_variables;

  absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*> expr_by_column_id;
  if (project != nullptr) {
    for (int i = 0; i < project->expr_list_size(); ++i) {
      const ::googlesql::ResolvedComputedColumn* cc = project->expr_list(i);
      if (cc == nullptr || cc->expr() == nullptr) {
        return absl::InternalError(
            "semantic: ResolvedComputedColumn has null expr");
      }
      expr_by_column_id[cc->column().column_id()] = cc->expr();
    }
  }

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

  auto materialized = MaterializeScan(query, ctx);
  if (!materialized.ok()) return materialized.status();

  std::vector<storage::Row> rows;
  rows.reserve(materialized->size());
  for (const ColumnBindings& bind : *materialized) {
    auto row = ProjectOneRow(query_stmt, expr_by_column_id, bind, ctx);
    if (!row.ok()) return row.status();
    rows.push_back(*std::move(row));
  }

  if (request.phase_recorder != nullptr) {
    request.phase_recorder->Record(
        "semantic_execute",
        absl::ToInt64Microseconds(absl::Now() - execute_start));
  }

  return std::unique_ptr<RowSource>(
      new MaterializedRowSource(std::move(output_schema), std::move(rows)));
}

absl::StatusOr<std::unique_ptr<RowSource>> SemanticExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;
  if (stmt.node_kind() == ::googlesql::RESOLVED_EXPLAIN_STMT) {
    return ExecuteExplainStmt(*stmt.GetAs<::googlesql::ResolvedExplainStmt>());
  }
  if (stmt.node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat(
            "semantic: only SELECT-shaped statements route here today; got ",
            stmt.node_kind_string()));
  }
  return ExecuteResolvedQueryStmt(request,
                                  *stmt.GetAs<::googlesql::ResolvedQueryStmt>(),
                                  /*script_variables=*/nullptr);
}

absl::StatusOr<DmlResult> SemanticExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  return dml::ExecuteDml(request, stmt, catalog, storage_);
}

namespace {

const ::googlesql::ResolvedSystemVariable* AsSystemVariableTarget(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) return nullptr;
  switch (expr->node_kind()) {
    case ::googlesql::RESOLVED_SYSTEM_VARIABLE:
      return expr->GetAs<::googlesql::ResolvedSystemVariable>();
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD: {
      const auto* gsf = expr->GetAs<::googlesql::ResolvedGetStructField>();
      return gsf == nullptr ? nullptr : AsSystemVariableTarget(gsf->expr());
    }
    default:
      return nullptr;
  }
}

absl::Status ExecuteAssignment(
    const QueryRequest& request,
    const ::googlesql::ResolvedAssignmentStmt& stmt) {
  if (stmt.target() == nullptr || stmt.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: AssignmentStmt has null target or expr");
  }
  const ::googlesql::ResolvedSystemVariable* sys =
      AsSystemVariableTarget(stmt.target());
  if (sys == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: AssignmentStmt target is not a system variable");
  }
  EvalContext ctx;
  ctx.project_id = request.project_id;
  auto value = EvalExpr(*stmt.expr(), ctx);
  if (!value.ok()) return value.status();
  return SetSystemVariable(
      request.project_id, sys->name_path(), *std::move(value));
}

}  // namespace

absl::Status SemanticExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;
  if (stmt.node_kind() == ::googlesql::RESOLVED_ASSERT_STMT) {
    script::ScriptDriver driver;
    return script::ExecuteAssert(
        request, *stmt.GetAs<::googlesql::ResolvedAssertStmt>(), driver);
  }
  if (stmt.node_kind() == ::googlesql::RESOLVED_CREATE_CONSTANT_STMT) {
    script::ScriptDriver driver;
    return script::ExecuteDeclare(
        request,
        *stmt.GetAs<::googlesql::ResolvedCreateConstantStmt>(),
        driver);
  }
  if (stmt.node_kind() == ::googlesql::RESOLVED_ASSIGNMENT_STMT) {
    const auto* assign = stmt.GetAs<::googlesql::ResolvedAssignmentStmt>();
    if (assign == nullptr) {
      return absl::InternalError(
          "semantic: RESOLVED_ASSIGNMENT_STMT cast returned null");
    }
    if (AsSystemVariableTarget(assign->target()) != nullptr) {
      return ExecuteAssignment(request, *assign);
    }
    script::ScriptDriver driver;
    return script::ExecuteScriptAssignment(request, *assign, driver);
  }
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
