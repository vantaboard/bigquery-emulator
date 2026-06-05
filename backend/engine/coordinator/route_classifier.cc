#include "backend/engine/coordinator/route_classifier.h"

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/node_dispositions.h"
#include "googlesql/public/function.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

namespace transpiler = ::bigquery_emulator::backend::engine::duckdb::transpiler;

// GoogleSQL function-group names for user-defined SQL bodies. Kept as
// string literals so route classification does not depend on
// `@googlesql//googlesql/public:sql_function` / `:templated_sql_function`
// (those labels exist in source mode but are not part of the frozen
// prebuilt wrapper surface).
constexpr absl::string_view kSqlFunctionGroup = "Lazy_resolution_function";
constexpr absl::string_view kTemplatedSqlFunctionGroup =
    "Templated_SQL_Function";

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
//
// `kLocalStub` ranks just above `kSemanticExecutor`: a stub function
// call (e.g. `KEYS.NEW_KEYSET(...)`) inside an otherwise-semantic
// SELECT promotes the route to local-stub so the coordinator's
// stub-aware dispatch fires. `kUnsupported` still dominates -- an
// `unsupported` function inside a SELECT that also contains a stub
// function lands on `kUnsupported` (the unsupported route's "no
// silent approximation" rule outranks the stub's "BigQuery-shaped
// placeholder" contract).
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
    case Disposition::kLocalStub:
      return 5;
    case Disposition::kUnsupported:
      return 6;
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

  // Property-based promotion for `ResolvedQueryStmt`:
  //
  //   1. `is_value_table()` means `SELECT AS VALUE ...`, which has
  //      no DuckDB analog (DuckDB has no value-table row shape).
  //      The node-class disposition is `duckdb_native`, but the
  //      property promotes to `semantic_executor` so the route
  //      reflects the BigQuery-only contract at planning time.
  //   2. A scalar-only SELECT (no FROM, i.e. the inner query is a
  //      `ResolvedProjectScan` over a `ResolvedSingleRowScan`)
  //      routes to `semantic_executor` per
  //      `docs/ENGINE_POLICY.md`. DuckDB
  //      can render the SQL on its side, but the semantic
  //      executor's strict NULL / overflow / error contracts are
  //      what the gateway/e2e suite asserts on. Promoting at the
  //      planning layer avoids a runtime "fast path returned an
  //      approximate answer" fallback.
  absl::Status VisitResolvedQueryStmt(
      const ::googlesql::ResolvedQueryStmt* node) override {
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

  // Detect the scalar-only-SELECT shape:
  // `ResolvedQueryStmt(query=ResolvedProjectScan(input_scan=
  // ResolvedSingleRowScan))`. This matches every shape the
  // semantic executor's scalar-only path handles today; FROM-
  // clause shapes (TableScan / JoinScan / ArrayScan / ...)
  // bypass this branch and stay on the DuckDB fast path until the
  // downstream plans pick them up.
  static bool IsScalarOnlySelect(const ::googlesql::ResolvedQueryStmt* node) {
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

  // Property-based promotion for `ResolvedJoinScan`: `is_lateral()`
  // marks the lateral / correlated join shape. BigQuery's lateral
  // evaluation order does not map cleanly onto DuckDB's
  // `LATERAL` / `unnest(...)` model, so the semantic executor owns
  // the shape (`docs/ENGINE_POLICY.md`).
  // `has_using()` / lateral `parameter_list_size > 0` stay in the
  // transpiler's empty-string defense-in-depth gate today; they
  // are picked up by the semantic executor when those handlers ship.
  absl::Status VisitResolvedJoinScan(
      const ::googlesql::ResolvedJoinScan* node) override {
    if (node != nullptr && node->is_lateral()) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedJoinScan(is_lateral=true)");
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedJoinScan(node);
  }

  // Property-based promotion for `ResolvedArrayScan`: the YAML row is
  // `duckdb_native` because the standalone `UNNEST(<arr>) AS <col>`
  // shape lowers cleanly to DuckDB's `SELECT unnest(...)`. The
  // divergent subset listed in `docs/ENGINE_POLICY.md`
  // promotes to the semantic executor because DuckDB's `LIST` model
  // does not match BigQuery's `ARRAY` model in those cases:
  //
  //   * `array_offset_column != nullptr` -> `UNNEST(<arr>) WITH
  //     OFFSET AS <idx>`. BigQuery's ordinal column is load-bearing;
  //     DuckDB's lateral unnest doesn't expose it cleanly.
  //   * `is_outer == true` -> outer UNNEST. BigQuery emits a single
  //     all-NULL row when the array is empty; DuckDB drops the row.
  //   * `array_expr_list_size() > 1` (and the matching
  //     `array_zip_mode != nullptr`) -> multi-array zip. The
  //     PAD/STRICT/TRUNCATE modes are BigQuery-only.
  //   * `join_expr != nullptr` -> `LEFT JOIN UNNEST(<arr>) ON ...`,
  //     a correlated array scan owned by this plan.
  //   * `input_scan` is non-NULL and not a `ResolvedSingleRowScan`
  //     -> `FROM t, UNNEST(t.arr)`, a correlated lateral scan
  //     owned by Family 4.
  //
  // Families 4 (correlated) and 5 (FLATTEN) are deferred from this
  // subagent per the plan's pragmatic posture; the classifier still
  // promotes them so the user surfaces a clean
  // `semantic-executor not-implemented` envelope instead of the
  // transpiler's empty-string UNIMPLEMENTED. The policy rows
  // in `node_dispositions.yaml` continue to point at
  // `docs/ENGINE_POLICY.md` until a follow-up subagent
  // lands them.
  // Property-based promotion for `ResolvedSubqueryExpr`. The node
  // class disposition is `duckdb_native` (the transpiler's
  // `EmitSubqueryExpr` lowers non-correlated SCALAR / IN / EXISTS /
  // ARRAY directly to DuckDB), but a non-empty `parameter_list()`
  // marks a correlated subquery: BigQuery evaluates the inner
  // subquery once per outer row with the parameter columns
  // re-bound from the outer scan, and DuckDB's
  // correlated-subquery decorrelation does not guarantee BigQuery's
  // per-outer-row evaluation order on every shape. Promoting at
  // planning time hands the correlated shape to the semantic
  // executor (today's stub; the executor for correlated subqueries
  // Correlated subqueries defer to the semantic executor.
  // follow-up subagent) so we never silently approximate.
  //
  // LIKE ANY / LIKE ALL / NOT LIKE ANY / NOT LIKE ALL also live on
  // `ResolvedSubqueryExpr` (with a `subquery_type` outside the
  // four-type SCALAR/IN/EXISTS/ARRAY set). The transpiler's emit
  // falls back to "" for those, which surfaces UNIMPLEMENTED on
  // the fast path. We do NOT promote them here -- their
  // disposition row already says `duckdb_native`, and the
  // empty-string contract is the right end-user-visible answer
  // until a follow-up plan picks them up.
  absl::Status VisitResolvedSubqueryExpr(
      const ::googlesql::ResolvedSubqueryExpr* node) override {
    if (node != nullptr && node->parameter_list_size() > 0) {
      MaybePromote(Disposition::kSemanticExecutor,
                   "ResolvedSubqueryExpr(correlated)");
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedSubqueryExpr(node);
  }

  absl::Status VisitResolvedArrayScan(
      const ::googlesql::ResolvedArrayScan* node) override {
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
      }
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedArrayScan(node);
  }

 private:
  // Look up `node`'s class disposition in
  // `node_dispositions.yaml`. The YAML keys are the full class
  // names (e.g. `"ResolvedTableScan"`); `node_kind_string()`
  // returns the un-prefixed form (`"TableScan"`), so we prepend
  // `"Resolved"` before the lookup.
  //
  // Rows with `status=planned` are deliberately NOT promoted (see
  // the upstream `docs/ENGINE_POLICY.md` contract
  // -- planned rows surface their disposition for documentation /
  // ownership tracking, but the executor for that route is not
  // ready yet, so the classifier keeps them on the default
  // `duckdb_native` route until the policy lands the real
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
    // Project-scoped CREATE FUNCTION bodies are evaluated by the semantic
    // executor using the analyzer-resolved expression on the call.
    if (fn->GetGroup() == kTemplatedSqlFunctionGroup ||
        fn->GetGroup() == kSqlFunctionGroup) {
      MaybePromote(Disposition::kSemanticExecutor,
                   absl::StrCat("sql_udf:", fn->Name()));
      return;
    }
    // Use `FullName(include_group=false)` so namespaced families
    // like `KEYS.NEW_KEYSET` / `NET.HOST` / `HLL_COUNT.MERGE`
    // resolve to their dotted YAML key (`keys.new_keyset`,
    // `net.host`, `hll_count.merge`). `Function::Name()` only
    // returns the last path element (`new_keyset`), which would
    // miss the namespaced rows entirely and silently leave the
    // route at the surrounding default disposition. For
    // non-namespaced functions (`concat`, `abs`, `safe_divide`)
    // `FullName(false) == Name()`, so this is a no-op.
    const std::string name = fn->FullName(/*include_group=*/false);
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
    // row whose policy has not yet landed the executor must
    // not promote the route. The transpiler's existing
    // empty-string contract will surface UNIMPLEMENTED if DuckDB
    // cannot lower the call. When the policy ships its
    // executor it drops the `planned` marker and this branch
    // starts promoting again.
    if (entry->planned) return;
    MaybePromote(entry->disposition,
                 absl::StrCat("function:", absl::AsciiStrToLower(name)));
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
  // until the policy lands and drops `planned` from the YAML
  // row.
  if (root_entry != nullptr && !root_entry->planned) {
    if (root_entry->disposition == Disposition::kControlOp) {
      return RouteDecision{
          Disposition::kControlOp,
          ReasonFor(Disposition::kControlOp, root_class, root_class),
          root_class,
      };
    }
    if (root_entry->disposition == Disposition::kLocalStub) {
      // Statement-level `kLocalStub` (e.g. `ResolvedCreateModelStmt`)
      // short-circuits the visitor walk for the same reason
      // `kControlOp` does: the deliberate-stub posture is owned by
      // the per-statement handler under `backend/engine/control/
      // stubs/` or `backend/engine/semantic/stubs/`. Inner shapes
      // don't override it (a CREATE MODEL's transform list might
      // contain BigQuery-specific functions, but the statement as a
      // whole is still a metadata-only stub).
      return RouteDecision{
          Disposition::kLocalStub,
          ReasonFor(Disposition::kLocalStub, root_class, root_class),
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
  // policys. Re-routing them to `kUnsupported` here would
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
