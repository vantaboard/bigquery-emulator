#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_NODE_DISPOSITIONS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_NODE_DISPOSITIONS_H_

// Per-`ResolvedAST` node-kind disposition table.
//
// The canonical list lives in `node_dispositions.yaml` next to this
// header; a Bazel `genrule` (see `BUILD.bazel`,
// `:node_dispositions_table_inc`) turns the YAML into
// `node_dispositions_table.inc`, which `node_dispositions.cc`
// `#include`s inside its
// `absl::flat_hash_map<std::string, NodeDispositionEntry>`
// initializer. The runtime API is one lookup function -- callers
// stay decoupled from the YAML shape.
//
// The `engine-router-foundation.plan.md` follow-up is the first
// caller; this plan ships the registry, not the router itself.
//
// See `backend/engine/disposition.h` for the canonical
// `Disposition` enum and the route semantics. Every YAML row maps
// to one enum value, plus optional `plan=` and `status=planned`
// metadata so the engine can log "row owned by <plan>, planned
// implementation not landed yet" when it surfaces `UNIMPLEMENTED`
// for a node kind whose disposition is a not-yet-implementable
// route.

#include <vector>

#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

struct NodeDispositionEntry {
  Disposition disposition;
  // Owning `.cursor/plans/*.plan.md` file name, or empty when this
  // row is documentation-only with no specific owning plan. For
  // `Disposition::kUnsupported` rows this is always non-empty (the
  // YAML generator rejects an unsupported row without a plan
  // pointer at build time); the convention is to point at
  // `specialized-feature-policy.plan.md`.
  absl::string_view plan;
  // True when the owning plan has not yet landed an implementation
  // (the YAML row carried `status=planned`). The engine surfaces
  // `UNIMPLEMENTED` for these rows; the flag exists so the log
  // message can distinguish "planned, waiting for plan X" from "no
  // disposition row".
  bool planned;
};

// Returns the disposition for `node_kind` (the GoogleSQL resolved
// AST class name, e.g. `"ResolvedQueryStmt"`), or nullptr when the
// kind is not in the registry. Names are case-sensitive (the
// registry mirrors `googlesql/resolved_ast/resolved_ast.h`'s class
// names verbatim).
const NodeDispositionEntry* LookupNodeDisposition(absl::string_view node_kind);

namespace internal {

// View over every (name, entry) pair in the registry. Stable pointer
// stability: the underlying string + entry references live in the
// static map; the view is rebuilt on the first call to
// `AllNodeDispositions`. Used by the unit test to walk the table
// without taking a dependency on the map type.
struct NodeDispositionView {
  absl::string_view name;
  const NodeDispositionEntry* entry;
};

const std::vector<NodeDispositionView>& AllNodeDispositions();

}  // namespace internal

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_NODE_DISPOSITIONS_H_
