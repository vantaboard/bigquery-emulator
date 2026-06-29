

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

// Build the route-specific UNIMPLEMENTED message. Centralized so
// each executor renders an identical envelope; conformance fixtures
// may assert on the route name. For `unsupported` routes the message
// also names the offending family so the operator can map the failure
// back to the exact `functions.yaml` / `node_dispositions.yaml` row.
std::string MakeUnimplementedMessage(absl::string_view route,
                                     absl::string_view operation,
                                     absl::string_view stmt_kind,
                                     absl::string_view offending_family) {
  std::string family_segment;
  if (!offending_family.empty() && offending_family != stmt_kind) {
    family_segment = absl::StrCat(", family: ", offending_family);
  }
  return absl::StrCat("local coordinator: ",
                      operation,
                      " on route '",
                      route,
                      "' is not implemented (statement kind: ",
                      stmt_kind,
                      family_segment,
                      "); see docs/ENGINE_POLICY.md");
}

std::string OffendingFamilyFor(const ::googlesql::ResolvedStatement& stmt) {
  RouteClassifier classifier;
  RouteDecision decision = classifier.Classify(stmt);
  (void)decision;
  return decision.offending_node;
}

}  // namespace

UnsupportedExecutor::~UnsupportedExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> UnsupportedExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(
      MakeUnimplementedMessage("unsupported",
                               "ExecuteQuery",
                               stmt.node_kind_string(),
                               OffendingFamilyFor(stmt)));
}

absl::StatusOr<DmlResult> UnsupportedExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(
      MakeUnimplementedMessage("unsupported",
                               "ExecuteDml",
                               stmt.node_kind_string(),
                               OffendingFamilyFor(stmt)));
}

absl::Status UnsupportedExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(
      MakeUnimplementedMessage("unsupported",
                               "ExecuteDdl",
                               stmt.node_kind_string(),
                               OffendingFamilyFor(stmt)));
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
