#include "backend/engine/coordinator/route_classifier_visitor.h"

#include <utility>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/node_dispositions.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/resolved_ast/resolved_collation.h"
#include "googlesql/resolved_ast/resolved_node.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

namespace transpiler = ::bigquery_emulator::backend::engine::duckdb::transpiler;

constexpr absl::string_view kTemplatedSqlFunctionGroup =
    "Templated_SQL_Function";

bool ArrayExprReferencesNamedConstant(const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) return false;
  if (expr->node_kind() == ::googlesql::RESOLVED_ARGUMENT_REF) {
    return true;
  }
  if (expr->node_kind() == ::googlesql::RESOLVED_CONSTANT) {
    const ::googlesql::Constant* constant =
        expr->GetAs<::googlesql::ResolvedConstant>()->constant();
    return constant != nullptr && !constant->Name().empty();
  }
  return false;
}

}  // namespace

int RouteClassifierPriority(Disposition d) {
  switch (d) {
    case Disposition::kDuckdbNative:
      return 0;
    case Disposition::kDuckdbRewrite:
      return 1;
    case Disposition::kDuckdbUdf:
      return 2;
    case Disposition::kControlOp:
      return 3;
    case Disposition::kSemanticExecutor:
      return 4;
    case Disposition::kLocalStub:
      return 5;
    case Disposition::kUnsupported:
      return 6;
  }
  return 0;
}

absl::Status RouteClassifierVisitor::DefaultVisit(
    const ::googlesql::ResolvedNode* node) {
  if (node != nullptr) {
    CheckNodeClass(node);
  }
  return ::googlesql::ResolvedASTVisitor::DefaultVisit(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedFunctionCall(
    const ::googlesql::ResolvedFunctionCall* node) {
  CheckFunction(node);
  return ::googlesql::ResolvedASTVisitor::VisitResolvedFunctionCall(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedAggregateFunctionCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node) {
  CheckFunction(node);
  if (node != nullptr) {
    if (node->having_modifier() != nullptr) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedAggregateFunctionCall(having_modifier)");
    }
    if (node->group_by_list_size() > 0 ||
        node->group_by_aggregate_list_size() > 0 ||
        node->having_expr() != nullptr) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedAggregateFunctionCall(multi_level_group_by)");
    }
    if (node->error_mode() ==
        ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedAggregateFunctionCall(safe_agg)");
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedAggregateFunctionCall(
      node);
}

absl::Status RouteClassifierVisitor::VisitResolvedAnalyticFunctionCall(
    const ::googlesql::ResolvedAnalyticFunctionCall* node) {
  CheckFunction(node);
  if (node != nullptr) {
    for (int i = 0; i < node->argument_list_size(); ++i) {
      const ::googlesql::ResolvedExpr* arg = node->argument_list(i);
      if (arg != nullptr &&
          arg->node_kind() == ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL) {
        MaybePromote(Disposition::kSemanticExecutor,
                     "ResolvedAnalyticFunctionCall(nested_aggregate)");
        break;
      }
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedAnalyticFunctionCall(
      node);
}

absl::Status RouteClassifierVisitor::VisitResolvedAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan* node) {
  if (node != nullptr && node->input_scan() != nullptr &&
      node->input_scan()->node_kind() == ::googlesql::RESOLVED_AGGREGATE_SCAN) {
    MaybePromote(Disposition::kSemanticExecutor,
                 "ResolvedAnalyticScan(aggregate_input)");
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedAnalyticScan(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedQueryStmt(
    const ::googlesql::ResolvedQueryStmt* node) {
  if (node != nullptr) {
    if (node->is_value_table()) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedQueryStmt(is_value_table=true)");
    } else if (IsScalarOnlySelect(node)) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedQueryStmt(scalar-only SELECT)");
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedQueryStmt(node);
}

bool RouteClassifierVisitor::IsScalarOnlySelect(
    const ::googlesql::ResolvedQueryStmt* node) {
  if (node == nullptr || node->query() == nullptr) return false;
  const ::googlesql::ResolvedScan* query = node->query();
  if (query->node_kind() != ::googlesql::RESOLVED_PROJECT_SCAN) {
    return false;
  }
  const auto* project = query->GetAs<::googlesql::ResolvedProjectScan>();
  const ::googlesql::ResolvedScan* input = project->input_scan();
  return input != nullptr &&
         input->node_kind() == ::googlesql::RESOLVED_SINGLE_ROW_SCAN;
}

absl::Status RouteClassifierVisitor::VisitResolvedJoinScan(
    const ::googlesql::ResolvedJoinScan* node) {
  if (node != nullptr && node->is_lateral()) {
    MaybePromote(Disposition::kSemanticExecutor,
                 "ResolvedJoinScan(is_lateral=true)");
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedJoinScan(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr* node) {
  if (node != nullptr) {
    if (node->parameter_list_size() > 0) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedSubqueryExpr(correlated)");
    }
    // `x IN UNNEST(@arr)` lowers to an IN subquery over a standalone
    // ArrayScan; the DuckDB transpiler does not cover that filter shape
    // yet, but the semantic executor's EvalSubqueryExpr(IN) path does.
    if (node->subquery_type() == ::googlesql::ResolvedSubqueryExpr::IN &&
        node->subquery() != nullptr &&
        node->subquery()->node_kind() == ::googlesql::RESOLVED_ARRAY_SCAN) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedSubqueryExpr(IN_UNNEST)");
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedSubqueryExpr(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedArrayScan(
    const ::googlesql::ResolvedArrayScan* node) {
  if (node != nullptr) {
    if (node->array_offset_column() != nullptr) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedArrayScan(array_offset_column)");
    } else if (node->is_outer()) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedArrayScan(is_outer=true)");
    } else if (node->array_zip_mode() != nullptr ||
               node->array_expr_list_size() > 1) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedArrayScan(array_zip_mode)");
    } else if (node->join_expr() != nullptr) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedArrayScan(join_expr)");
    } else if (node->input_scan() != nullptr &&
               node->input_scan()->node_kind() !=
                   ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedArrayScan(correlated_input_scan)");
    } else if (node->array_expr_list_size() > 0 &&
               ArrayExprReferencesNamedConstant(node->array_expr_list(0))) {
      // Script variables are registered as named catalog constants.
      // UNNEST(top_names) over a script binding must evaluate in the
      // semantic executor (in-memory array + filter/subquery shapes
      // the DuckDB transpiler does not cover yet).
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedArrayScan(script_constant)");
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedArrayScan(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedCast(
    const ::googlesql::ResolvedCast* node) {
  if (node != nullptr &&
      (node->format() != nullptr || node->time_zone() != nullptr ||
       node->extended_cast() != nullptr || !node->type_modifiers().IsEmpty())) {
    MaybePromote(Disposition::kSemanticExecutor, "ResolvedCast(extended)");
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedCast(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedOrderByScan(
    const ::googlesql::ResolvedOrderByScan* node) {
  if (node != nullptr) {
    for (int i = 0; i < node->order_by_item_list_size(); ++i) {
      const ::googlesql::ResolvedOrderByItem* item =
          node->order_by_item_list(i);
      if (item != nullptr && (item->collation_name() != nullptr ||
                              item->collation().HasCollation())) {
        MaybePromote(Disposition::kSemanticExecutor,
                     "ResolvedOrderByScan(collation)");
        break;
      }
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedOrderByScan(node);
}

bool RouteClassifierVisitor::MergeRequiresSemanticExecutor(
    const ::googlesql::ResolvedMergeStmt* merge) {
  if (merge == nullptr) return false;
  int matched_clauses = 0;
  int not_matched_target_clauses = 0;
  int not_matched_source_clauses = 0;
  int update_actions = 0;
  int delete_actions = 0;
  int insert_actions = 0;
  for (int i = 0; i < merge->when_clause_list_size(); ++i) {
    const ::googlesql::ResolvedMergeWhen* when = merge->when_clause_list(i);
    if (when == nullptr) continue;
    switch (when->match_type()) {
      case ::googlesql::ResolvedMergeWhen::MATCHED:
        ++matched_clauses;
        break;
      case ::googlesql::ResolvedMergeWhen::NOT_MATCHED_BY_TARGET:
        ++not_matched_target_clauses;
        break;
      case ::googlesql::ResolvedMergeWhen::NOT_MATCHED_BY_SOURCE:
        ++not_matched_source_clauses;
        break;
    }
    switch (when->action_type()) {
      case ::googlesql::ResolvedMergeWhen::UPDATE:
        ++update_actions;
        break;
      case ::googlesql::ResolvedMergeWhen::DELETE:
        ++delete_actions;
        break;
      case ::googlesql::ResolvedMergeWhen::INSERT:
        ++insert_actions;
        break;
    }
  }
  if (not_matched_source_clauses > 0) return true;
  if (matched_clauses > 1 || not_matched_target_clauses > 1) return true;
  if (update_actions > 1 || delete_actions > 1 || insert_actions > 1) {
    return true;
  }
  return false;
}

absl::Status RouteClassifierVisitor::VisitResolvedInsertStmt(
    const ::googlesql::ResolvedInsertStmt* node) {
  if (node != nullptr && node->returning() != nullptr) {
    MaybePromote(Disposition::kSemanticExecutor,
                 "ResolvedInsertStmt(returning)");
  }
  bool duckdb_insert_select = false;
  if (node != nullptr && node->query() != nullptr) {
    RouteClassifierVisitor inner;
    absl::Status inner_walk = node->query()->Accept(&inner);
    if (!inner_walk.ok()) return inner_walk;
    const Disposition inner_d = inner.disposition();
    if (inner_d == Disposition::kDuckdbNative ||
        inner_d == Disposition::kDuckdbRewrite ||
        inner_d == Disposition::kDuckdbUdf) {
      MaybePromote(inner_d, "ResolvedInsertStmt(SELECT)");
      duckdb_insert_select = true;
    }
  }
  if (node != nullptr && !duckdb_insert_select) {
    CheckNodeClass(node);
  }
  if (node == nullptr) return absl::OkStatus();
  if (node->table_scan() != nullptr) {
    absl::Status s = node->table_scan()->Accept(this);
    if (!s.ok()) return s;
  }
  if (!duckdb_insert_select && node->query() != nullptr) {
    absl::Status s = node->query()->Accept(this);
    if (!s.ok()) return s;
  }
  for (int i = 0; i < node->row_list_size(); ++i) {
    const ::googlesql::ResolvedInsertRow* row = node->row_list(i);
    if (row != nullptr) {
      absl::Status s = row->Accept(this);
      if (!s.ok()) return s;
    }
  }
  return absl::OkStatus();
}

absl::Status RouteClassifierVisitor::VisitResolvedUpdateStmt(
    const ::googlesql::ResolvedUpdateStmt* node) {
  if (node != nullptr) {
    if (node->returning() != nullptr) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedUpdateStmt(returning)");
    }
    if (node->from_scan() != nullptr) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedUpdateStmt(from_scan)");
    }
    CheckNodeClass(node);
    if (node->table_scan() != nullptr) {
      absl::Status s = node->table_scan()->Accept(this);
      if (!s.ok()) return s;
    }
    if (node->from_scan() != nullptr) {
      absl::Status s = node->from_scan()->Accept(this);
      if (!s.ok()) return s;
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedUpdateStmt(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedDeleteStmt(
    const ::googlesql::ResolvedDeleteStmt* node) {
  if (node != nullptr) {
    if (node->returning() != nullptr) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedDeleteStmt(returning)");
    }
    CheckNodeClass(node);
    if (node->table_scan() != nullptr) {
      absl::Status s = node->table_scan()->Accept(this);
      if (!s.ok()) return s;
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedDeleteStmt(node);
}

absl::Status RouteClassifierVisitor::VisitResolvedMergeStmt(
    const ::googlesql::ResolvedMergeStmt* node) {
  if (node != nullptr) {
    if (MergeRequiresSemanticExecutor(node)) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedMergeStmt(hard_matrix)");
    } else {
      CheckNodeClass(node);
    }
    if (node->table_scan() != nullptr) {
      absl::Status s = node->table_scan()->Accept(this);
      if (!s.ok()) return s;
    }
    if (node->from_scan() != nullptr) {
      absl::Status s = node->from_scan()->Accept(this);
      if (!s.ok()) return s;
    }
  }
  return ::googlesql::ResolvedASTVisitor::VisitResolvedMergeStmt(node);
}

void RouteClassifierVisitor::CheckNodeClass(
    const ::googlesql::ResolvedNode* node) {
  std::string class_name = absl::StrCat("Resolved", node->node_kind_string());
  const auto* entry = transpiler::LookupNodeDisposition(class_name);
  if (entry == nullptr) return;
  if (entry->planned) return;
  MaybePromote(entry->disposition, std::move(class_name));
}

void RouteClassifierVisitor::CheckFunction(
    const ::googlesql::ResolvedNode* node) {
  if (node == nullptr) return;
  const ::googlesql::Function* fn = nullptr;
  if (node->Is<::googlesql::ResolvedFunctionCall>()) {
    fn = node->GetAs<::googlesql::ResolvedFunctionCall>()->function();
  } else if (node->Is<::googlesql::ResolvedAggregateFunctionCall>()) {
    fn = node->GetAs<::googlesql::ResolvedAggregateFunctionCall>()->function();
  } else if (node->Is<::googlesql::ResolvedAnalyticFunctionCall>()) {
    fn = node->GetAs<::googlesql::ResolvedAnalyticFunctionCall>()->function();
  }
  if (fn == nullptr) return;
  if (fn->GetGroup() == kTemplatedSqlFunctionGroup ||
      fn->GetGroup() == ::googlesql::SQLFunction::kSQLFunctionGroup) {
    MaybePromote(Disposition::kSemanticExecutor,
                 absl::StrCat("sql_udf:", fn->Name()));
    return;
  }
  const std::string name = fn->FullName(/*include_group=*/false);
  const auto* entry = transpiler::LookupFunction(name);
  if (entry == nullptr) {
    return;
  }
  if (entry->planned) return;
  MaybePromote(entry->disposition,
               absl::StrCat("function:", absl::AsciiStrToLower(name)));
}

void RouteClassifierVisitor::MaybePromote(Disposition d, std::string name) {
  if (RouteClassifierPriority(d) > RouteClassifierPriority(disposition_)) {
    disposition_ = d;
    offending_node_ = std::move(name);
  }
}

std::string RouteClassifierReasonFor(Disposition d,
                                     absl::string_view root_class,
                                     absl::string_view offending_node) {
  switch (d) {
    case Disposition::kDuckdbNative:
      return std::string("");
    case Disposition::kDuckdbRewrite:
      return absl::StrCat(
          "query lowers via duckdb_rewrite (promoted by ", offending_node, ")");
    case Disposition::kDuckdbUdf:
      return absl::StrCat(
          "query lowers via duckdb_udf (promoted by ", offending_node, ")");
    case Disposition::kSemanticExecutor:
      return absl::StrCat("query requires the semantic executor (promoted by ",
                          offending_node,
                          ")");
    case Disposition::kControlOp:
      return absl::StrCat(
          "statement ", root_class, " routes to the control-op executor");
    case Disposition::kLocalStub:
      return absl::StrCat(
          "query routes to the local-stub executor (specialized feature "
          "family promoted by ",
          offending_node,
          "); see docs/ENGINE_POLICY.md");
    case Disposition::kUnsupported:
      return absl::StrCat(
          "query is unsupported (offending node: ", offending_node, ")");
  }
  return std::string("");
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
