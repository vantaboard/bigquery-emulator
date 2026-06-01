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
  Disposition disposition() const { return disposition_; }
  const std::string& offending_node() const { return offending_node_; }

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

 private:
  // Look up `node`'s class disposition in
  // `node_dispositions.yaml`. The YAML keys are the full class
  // names (e.g. `"ResolvedTableScan"`); `node_kind_string()`
  // returns the un-prefixed form (`"TableScan"`), so we prepend
  // `"Resolved"` before the lookup.
  void CheckNodeClass(const ::googlesql::ResolvedNode* node) {
    std::string class_name =
        absl::StrCat("Resolved", node->node_kind_string());
    const auto* entry = transpiler::LookupNodeDisposition(class_name);
    if (entry != nullptr) {
      MaybePromote(entry->disposition, std::move(class_name));
    }
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
      fn = node->GetAs<::googlesql::ResolvedAggregateFunctionCall>()->function();
    } else if (node->Is<::googlesql::ResolvedAnalyticFunctionCall>()) {
      fn = node->GetAs<::googlesql::ResolvedAnalyticFunctionCall>()->function();
    }
    if (fn == nullptr) return;
    absl::string_view name = fn->Name();
    const auto* entry = transpiler::LookupFunction(name);
    if (entry == nullptr) {
      // Unknown function -> unsupported. Carry the function name
      // so the gateway can surface a useful error message.
      MaybePromote(Disposition::kUnsupported,
                   absl::StrCat("function:", name));
      return;
    }
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
      return absl::StrCat("query lowers via duckdb_rewrite (promoted by ",
                          offending_node, ")");
    case Disposition::kDuckdbUdf:
      return absl::StrCat("query lowers via duckdb_udf (promoted by ",
                          offending_node, ")");
    case Disposition::kSemanticExecutor:
      return absl::StrCat("query requires the semantic executor (promoted by ",
                          offending_node, ")");
    case Disposition::kControlOp:
      return absl::StrCat("statement ", root_class,
                          " routes to the control-op executor");
    case Disposition::kUnsupported:
      return absl::StrCat("query is unsupported (offending node: ",
                          offending_node, ")");
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
  if (root_entry != nullptr) {
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
  } else {
    // Root not in the registry -- by construction a registry bug
    // (parity checker should catch this), but fold to unsupported
    // so the gateway has a stable error code while the parity
    // failure gets fixed.
    return RouteDecision{
        Disposition::kUnsupported,
        absl::StrCat("statement ", root_class,
                     " has no entry in node_dispositions.yaml"),
        root_class,
    };
  }

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
