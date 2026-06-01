#include "backend/engine/coordinator/route_classifier.h"

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/node_dispositions.h"
#include "googlesql/public/function.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

namespace transpiler = ::bigquery_emulator::backend::engine::duckdb::transpiler;

// Priority lookup for the compositional fallback algorithm. Higher
// number = higher priority = wins over lower-priority dispositions
// when we walk the tree.
//
// `kControlOp` is in the middle deliberately: an inner `control_op`
// node (e.g. `ResolvedCatalogColumnRef`) does not promote a normal
// `SELECT`'s route up to `control_op`, because the caller checks
// the root-level `control_op` case before invoking the visitor.
// Inside the visitor, the priority value still matters only when
// the root is non-`control_op` AND an inner node carries a
// `control_op` disposition -- which is the
// `ResolvedCatalogColumnRef`-inside-SELECT case. We rank it above
// `kDuckdbUdf` (a fast-path concern) but below `kSemanticExecutor`
// (the local executor handles metadata expressions today) so the
// route ends up at the semantic executor when a SELECT mixes both.
int Priority(Disposition d) {
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
    case Disposition::kUnsupported:
      return 5;
  }
  return 0;
}

// Visitor that walks the resolved AST and tracks the highest-
// priority disposition seen so far. `DefaultVisit` consults the
// per-node-kind registry; the per-function-call overrides also
// consult the per-function registry.
class ClassifierVisitor : public ::googlesql::ResolvedASTVisitor {
 public:
  Disposition disposition() const {
    return disposition_;
  }
  const std::string& offending_node() const {
    return offending_node_;
  }

  absl::Status DefaultVisit(const ::googlesql::ResolvedNode* node) override {
    if (node != nullptr) {
      CheckNodeClass(node);
    }
    return ::googlesql::ResolvedASTVisitor::DefaultVisit(node);
  }

  absl::Status VisitResolvedFunctionCall(
      const ::googlesql::ResolvedFunctionCall* node) override {
    CheckFunction(node);
    return ::googlesql::ResolvedASTVisitor::VisitResolvedFunctionCall(node);
  }

  absl::Status VisitResolvedAggregateFunctionCall(
      const ::googlesql::ResolvedAggregateFunctionCall* node) override {
    CheckFunction(node);
    return ::googlesql::ResolvedASTVisitor::VisitResolvedAggregateFunctionCall(
        node);
  }

  absl::Status VisitResolvedAnalyticFunctionCall(
      const ::googlesql::ResolvedAnalyticFunctionCall* node) override {
    CheckFunction(node);
    return ::googlesql::ResolvedASTVisitor::VisitResolvedAnalyticFunctionCall(
        node);
  }

  // Property-based promotion for `ResolvedQueryStmt`: the wrapping
  // `is_value_table()` flag means `SELECT AS VALUE ...`, which has
  // no DuckDB analog (DuckDB has no value-table row shape). The
  // node-class disposition is `duckdb_native`, but the property
  // promotes to `semantic_executor` so the route reflects the
  // BigQuery-only contract at planning time. Plan ownership lives
  // with the semantic executor (`semantic-executor-core.plan.md`);
  // the stub `SemanticExecutor` returns `UNIMPLEMENTED` until the
  // owning plan ships, which is the same end-user-visible outcome
  // the transpiler's empty-string gate produced.
  absl::Status VisitResolvedQueryStmt(
      const ::googlesql::ResolvedQueryStmt* node) override {
    if (node != nullptr && node->is_value_table()) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedQueryStmt(is_value_table=true)");
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedQueryStmt(node);
  }

  // Property-based promotion for `ResolvedJoinScan`: `is_lateral()`
  // marks the lateral / correlated join shape. BigQuery's lateral
  // evaluation order does not map cleanly onto DuckDB's
  // `LATERAL` / `unnest(...)` model, so the semantic executor owns
  // the shape (`array-struct-semantic-path.plan.md`).
  // `has_using()` / lateral `parameter_list_size > 0` stay in the
  // transpiler's empty-string defense-in-depth gate today; they
  // are picked up by `array-struct-semantic-path.plan.md` /
  // `cte-subquery-routing.plan.md` when those plans ship.
  absl::Status VisitResolvedJoinScan(
      const ::googlesql::ResolvedJoinScan* node) override {
    if (node != nullptr && node->is_lateral()) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedJoinScan(is_lateral=true)");
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedJoinScan(node);
  }

 private:
  // Look up `node`'s class disposition in
  // `node_dispositions.yaml`. The YAML keys are the full class
  // names (e.g. `"ResolvedTableScan"`); `node_kind_string()`
  // returns the un-prefixed form (`"TableScan"`), so we prepend
  // `"Resolved"` before the lookup.
  //
  // Rows with `status=planned` are deliberately NOT promoted (see
  // the upstream `execution-disposition-registry.plan.md` contract
  // -- planned rows surface their disposition for documentation /
  // ownership tracking, but the executor for that route is not
  // ready yet, so the classifier keeps them on the default
  // `duckdb_native` route until the owning plan lands the real
  // executor and removes the `planned` marker. The fast path will
  // surface UNIMPLEMENTED from the transpiler for shapes it cannot
  // lower; that is a fast-path bug, not a classifier bug).
  void CheckNodeClass(const ::googlesql::ResolvedNode* node) {
    std::string class_name = absl::StrCat("Resolved", node->node_kind_string());
    const auto* entry = transpiler::LookupNodeDisposition(class_name);
    if (entry == nullptr) return;
    if (entry->planned) return;
    MaybePromote(entry->disposition, std::move(class_name));
    // A node kind absent from the registry is silently treated as
    // `kDuckdbNative` (no promotion). The registry covers every
    // user-visible `ResolvedAST` class; an absent row is by
    // construction a registry bug that the parity checker
    // (`task lint:dispositions`) is supposed to catch -- we do not
    // re-surface it here as an unsupported route.
  }

  // Look up a function call's disposition. Both the function entry
  // and the surrounding node class disposition matter; the class
  // disposition is handled separately by `DefaultVisit`.
  void CheckFunction(const ::googlesql::ResolvedNode* node) {
    if (node == nullptr) return;
    const ::googlesql::Function* fn = nullptr;
    if (node->Is<::googlesql::ResolvedFunctionCall>()) {
      fn = node->GetAs<::googlesql::ResolvedFunctionCall>()->function();
    } else if (node->Is<::googlesql::ResolvedAggregateFunctionCall>()) {
      fn =
          node->GetAs<::googlesql::ResolvedAggregateFunctionCall>()->function();
    } else if (node->Is<::googlesql::ResolvedAnalyticFunctionCall>()) {
      fn = node->GetAs<::googlesql::ResolvedAnalyticFunctionCall>()->function();
    }
    if (fn == nullptr) return;
    absl::string_view name = fn->Name();
    const auto* entry = transpiler::LookupFunction(name);
    if (entry == nullptr) {
      // Unknown function -> default fast path (no promotion). The
      // `functions.yaml` registry covers the BigQuery scalar / aggregate
      // surface; built-in operators (`$equal`, `$add`, `$less`, `$and`,
      // ...) are deliberately absent because the transpiler emits them
      // through dedicated paths (or the DML/DDL executors hand the SQL
      // verbatim to DuckDB). Re-routing unknown functions to the
      // unsupported stub would violate "no silent approximation" --
      // the fast path can run today's MERGE / SELECT shapes that
      // contain comparison operators. If the fast path genuinely
      // cannot lower a call, the transpiler's empty-string contract
      // surfaces UNIMPLEMENTED from inside the fast path; the parity
      // checker (`task lint:dispositions`) is responsible for
      // catching genuinely missing rows.
      return;
    }
    // Same `planned`-row contract as `CheckNodeClass`: a function
    // row whose owning plan has not yet landed the executor must
    // not promote the route. The transpiler's existing
    // empty-string contract will surface UNIMPLEMENTED if DuckDB
    // cannot lower the call. When the owning plan ships its
    // executor it drops the `planned` marker and this branch
    // starts promoting again.
    if (entry->planned) return;
    MaybePromote(entry->disposition, absl::StrCat("function:", name));
  }

  void MaybePromote(Disposition d, std::string name) {
    if (Priority(d) > Priority(disposition_)) {
      disposition_ = d;
      offending_node_ = std::move(name);
    }
  }

  Disposition disposition_ = Disposition::kDuckdbNative;
  std::string offending_node_;
};

// Render a human-readable reason for a `RouteDecision`. Kept off
// the hot path so the visitor stays tight and the formatting lives
// in one place.
std::string ReasonFor(Disposition d,
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
    case Disposition::kUnsupported:
      return absl::StrCat(
          "query is unsupported (offending node: ", offending_node, ")");
  }
  return std::string("");
}

}  // namespace

RouteDecision RouteClassifier::Classify(
    const ::googlesql::ResolvedStatement& stmt) const {
  const std::string root_class =
      absl::StrCat("Resolved", stmt.node_kind_string());
  const auto* root_entry = transpiler::LookupNodeDisposition(root_class);

  // The root statement's disposition is authoritative when it is
  // `control_op` (DDL / metadata) or `unsupported`. We short-circuit
  // those two cases before walking the tree so inner shapes don't
  // accidentally override them (a CTAS's inner SELECT might
  // contain a `semantic_executor` function, but the CTAS as a
  // whole is still a control-op statement; the control-op
  // executor is the one that owns the statement-level execution
  // model).
  //
  // A `planned` root row is treated identically to a non-existing
  // row at the route level: the upstream registry uses `planned`
  // to mean "the executor for this disposition hasn't landed yet",
  // so the classifier must not route to a stub executor today
  // (which would break gateway/e2e tests that hit DDL / DML shapes
  // the registry promises to a future executor). The fast path
  // (DuckDB) keeps handling them via the visitor's default branch
  // until the owning plan lands and drops `planned` from the YAML
  // row.
  if (root_entry != nullptr && !root_entry->planned) {
    if (root_entry->disposition == Disposition::kControlOp) {
      return RouteDecision{
          Disposition::kControlOp,
          ReasonFor(Disposition::kControlOp, root_class, root_class),
          root_class,
      };
    }
    if (root_entry->disposition == Disposition::kUnsupported) {
      return RouteDecision{
          Disposition::kUnsupported,
          ReasonFor(Disposition::kUnsupported, root_class, root_class),
          root_class,
      };
    }
  }
  // A missing root row is treated identically to a `planned` row at
  // the route level: do NOT promote to a stub executor. The fast
  // path (DuckDB) already handles statement shapes like
  // `ResolvedAlterTableStmt` via `DuckDbExecutor::ExecuteDdl`; the
  // registry's parity checker (`task lint:dispositions`) is
  // responsible for catching missing rows and they get added by the
  // owning plans. Re-routing them to `kUnsupported` here would
  // break existing gateway/e2e tests (no silent approximation).
  // We fall through to the visitor walk; missing-from-registry
  // resolves to the default `kDuckdbNative` if no inner promotion
  // fires.

  // Walk the whole tree. The visitor sees the root statement too
  // (its disposition is `kDuckdbNative` or `kSemanticExecutor`
  // here -- the two `control_op` / `unsupported` cases above
  // short-circuited).
  ClassifierVisitor visitor;
  absl::Status walk = stmt.Accept(&visitor);
  if (!walk.ok()) {
    // Defense-in-depth: our visitor never returns a non-OK status
    // (the base recursion `ChildrenAccept` could in theory, but
    // the AST is already analyzed and well-formed at this point).
    // Fold an unexpected error to unsupported so the gateway
    // surfaces a clean code.
    return RouteDecision{
        Disposition::kUnsupported,
        absl::StrCat("classifier visitor failed: ", walk.message()),
        root_class,
    };
  }

  Disposition d = visitor.disposition();
  std::string offending = visitor.offending_node();
  return RouteDecision{
      d,
      ReasonFor(d, root_class, offending),
      std::move(offending),
  };
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
