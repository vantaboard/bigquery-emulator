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
// See `backend/engine/disposition.h` for the canonical
// `Disposition` enum and the route semantics. Every YAML row maps
// to one enum value, plus optional `status=planned` metadata so the
// engine can surface `UNIMPLEMENTED` for a node kind whose runtime
// emit is not yet implemented.

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
  // True when the YAML row carried `status=planned`. The engine
  // surfaces `UNIMPLEMENTED` for these rows until the transpiler or
  // executor implements the route for this node kind.
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
