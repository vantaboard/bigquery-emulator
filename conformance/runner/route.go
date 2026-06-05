package runner

import "slices"

// Go-side mirror of the C++ canonical `Disposition` vocabulary
// `backend/engine/disposition.cc::DispositionToString` produces.
// The two sides MUST agree letter-for-letter: a fixture writer
// pinning `expected.route: duckdb_native` only PASSes when the
// engine trails `emulator_route=duckdb_native`. Keep this list
// sorted by `node_dispositions.yaml`'s priority order (low ->
// high) so a reviewer can eyeball the relative weight if it ever
// comes up.
//
// Plan ownership: `docs/ENGINE_POLICY.md`
// (this file) and `docs/ENGINE_POLICY.md`
// (the C++ source of truth). A new disposition value lands on BOTH
// sides at once.
//
// We do NOT generate this from the C++ header at build time: the
// 7-entry list churns rarely (each new entry is a multi-plan
// rollout) and the parity check in `tools/check_disposition_parity`
// already catches a C++/YAML/Go drift before it ships.

// Canonical lowercase-snake disposition names. Must mirror
// `Disposition::k*` in `backend/engine/disposition.h`.
const (
	// RouteDuckDBNative lowers to DuckDB SQL whose semantics
	// already match BigQuery exactly.
	RouteDuckDBNative = "duckdb_native"

	// RouteDuckDBRewrite lowers to DuckDB SQL via a deliberate
	// structural rewrite (struct/array shape rewrites, JSON
	// operator mapping, ...). Same executor as duckdb_native.
	RouteDuckDBRewrite = "duckdb_rewrite"

	// RouteDuckDBUDF lowers to DuckDB SQL that calls one of the
	// polyfill UDFs/macros registered at engine startup.
	RouteDuckDBUDF = "duckdb_udf"

	// RouteSemanticExecutor runs on the local row/value semantic
	// executor instead of DuckDB SQL evaluation.
	RouteSemanticExecutor = "semantic_executor"

	// RouteControlOp is the DDL / metadata / catalog op route
	// (CREATE TABLE / DROP TABLE / ALTER / pipe-DDL).
	RouteControlOp = "control_op"

	// RouteLocalStub is the deterministic BigQuery-shaped stub
	// route for specialized features (KEYS.NEW_KEYSET, CREATE
	// MODEL, ...). See `docs/ENGINE_POLICY.md`.
	RouteLocalStub = "local_stub"

	// RouteUnsupported surfaces a BigQuery-shaped `UNIMPLEMENTED`.
	// See `docs/ENGINE_POLICY.md` for
	// the unsupported families list.
	RouteUnsupported = "unsupported"
)

// knownRoutes pins the closed set in priority order (matches the
// C++ enum declaration in `disposition.h`). Iterating this slice
// gives a stable, reviewable ordering in user-facing error
// messages.
var knownRoutes = []string{
	RouteDuckDBNative,
	RouteDuckDBRewrite,
	RouteDuckDBUDF,
	RouteSemanticExecutor,
	RouteControlOp,
	RouteLocalStub,
	RouteUnsupported,
}

// KnownRouteNames returns a copy of the canonical disposition
// names in priority order. Used by validation messages and the
// matrix walker so they share one source of truth.
func KnownRouteNames() []string {
	out := make([]string, len(knownRoutes))
	copy(out, knownRoutes)
	return out
}

// isKnownRouteName reports whether `s` is one of the canonical
// disposition names.
func isKnownRouteName(s string) bool {
	return slices.Contains(knownRoutes, s)
}
