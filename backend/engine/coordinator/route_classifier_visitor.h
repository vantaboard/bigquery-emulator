#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTE_CLASSIFIER_VISITOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTE_CLASSIFIER_VISITOR_H_

// Internal AST visitor for `RouteClassifier`. Split out of
// `route_classifier.cc` to keep the coordinator lint file-length cap.

#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

// Priority lookup for the compositional fallback algorithm.
int RouteClassifierPriority(Disposition d);

// Visitor that walks the resolved AST and tracks the highest-priority
// disposition seen so far.
class RouteClassifierVisitor : public ::googlesql::ResolvedASTVisitor {
 public:
  Disposition disposition() const { return disposition_; }
  const std::string& offending_node() const { return offending_node_; }

  absl::Status DefaultVisit(const ::googlesql::ResolvedNode* node) override;
  absl::Status VisitResolvedFunctionCall(
      const ::googlesql::ResolvedFunctionCall* node) override;
  absl::Status VisitResolvedAggregateFunctionCall(
      const ::googlesql::ResolvedAggregateFunctionCall* node) override;
  absl::Status VisitResolvedAnalyticFunctionCall(
      const ::googlesql::ResolvedAnalyticFunctionCall* node) override;
  absl::Status VisitResolvedQueryStmt(
      const ::googlesql::ResolvedQueryStmt* node) override;
  absl::Status VisitResolvedJoinScan(
      const ::googlesql::ResolvedJoinScan* node) override;
  absl::Status VisitResolvedSubqueryExpr(
      const ::googlesql::ResolvedSubqueryExpr* node) override;
  absl::Status VisitResolvedArrayScan(
      const ::googlesql::ResolvedArrayScan* node) override;

  static bool IsScalarOnlySelect(const ::googlesql::ResolvedQueryStmt* node);

 private:
  void CheckNodeClass(const ::googlesql::ResolvedNode* node);
  void CheckFunction(const ::googlesql::ResolvedNode* node);
  void MaybePromote(Disposition d, std::string name);

  Disposition disposition_ = Disposition::kDuckdbNative;
  std::string offending_node_;
};

std::string RouteClassifierReasonFor(Disposition d,
                                     absl::string_view root_class,
                                     absl::string_view offending_node);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTE_CLASSIFIER_VISITOR_H_
