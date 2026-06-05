#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTE_CLASSIFIER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTE_CLASSIFIER_H_

// Route classifier.
//
// `RouteClassifier::Classify` walks a resolved GoogleSQL AST and
// returns the planned execution-route `Disposition` for the whole
// query. The decision is *compositional at planning time*: the
// classifier picks one route per query, not per node, so we never
// mix executors mid-execution. The chosen route plus a human-
// readable reason and the first node that promoted it travel
// together in a `RouteDecision`, so the gateway-side error message
// can attribute the decision and so future route-label tests can
// assert which executor served a fixture.
//
// Algorithm (see `docs/ENGINE_POLICY.md`):
//
//   1. Look up the root statement's disposition in
//      `node_dispositions.yaml`.
//        * `kControlOp` at the root (DDL / metadata) -> control-op
//          route directly. Inner shapes don't override this -- DDL
//          is handled as a whole unit by the control-op executor.
//        * `kUnsupported` at the root (e.g. `EXPLAIN`) -> short-
//          circuit to the unsupported route.
//        * Anything else -> proceed to step 2.
//   2. Walk the whole resolved AST. For each node, look up its
//      class disposition; for each function call, look up the
//      function disposition. Track the highest-priority
//      disposition seen so far. Priority order (high -> low):
//
//          kUnsupported > kSemanticExecutor > kControlOp >
//          kDuckdbUdf > kDuckdbRewrite > kDuckdbNative.
//
//      A leaf `kSemanticExecutor` promotes the whole query to the
//      semantic executor; a leaf `kUnsupported` promotes to the
//      unsupported executor with the offending node's name carried
//      in `RouteDecision`.
//   3. Return the final `RouteDecision`.
//
// The classifier never re-routes at runtime. The DuckDB fast path
// is allowed to surface `UNIMPLEMENTED` from inside its transpiler
// (that is a fast-path bug, not a classifier bug); we deliberately
// do not catch and retry on another executor -- the old
// `FallbackEngine`'s silent-drift hazard is exactly what this
// classifier exists to prevent.

#include <string>

#include "backend/engine/disposition.h"

namespace googlesql {
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

// Outcome of `RouteClassifier::Classify`. The chosen `disposition`
// drives the coordinator's executor dispatch; `reason` and
// `offending_node` are observable surfaces a future
// `docs/ENGINE_POLICY.md` fixture (or a gateway error
// message) can read off the engine.
struct RouteDecision {
  // The route the coordinator dispatches on.
  Disposition disposition = Disposition::kDuckdbNative;

  // Human-readable explanation. Empty when the route is trivially
  // `kDuckdbNative` (the default, no promotion happened). Otherwise
  // explains why the route was chosen.
  std::string reason;

  // The first node (resolved AST class name) or function name that
  // promoted the route above `kDuckdbNative`. Empty when the route
  // is `kDuckdbNative`. Function names are recorded verbatim
  // (lowercased the same way `LookupFunction` lowercases them).
  std::string offending_node;
};

class RouteClassifier {
 public:
  RouteClassifier() = default;
  ~RouteClassifier() = default;

  RouteClassifier(const RouteClassifier&) = delete;
  RouteClassifier& operator=(const RouteClassifier&) = delete;

  // Classify the resolved statement and return the planned route.
  // Thread-safe; the classifier holds no per-call state.
  RouteDecision Classify(const ::googlesql::ResolvedStatement& stmt) const;
};

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTE_CLASSIFIER_H_
