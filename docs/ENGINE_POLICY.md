# Execution policy

This emulator ships **one local emulator process** with **one C++
`Engine` interface** (`backend/engine/engine.h`) and **one persistent
storage implementation** (`DuckDBStorage`). Behind that single
`Engine`, query execution is **multi-strategy**: a local execution
router classifies each resolved GoogleSQL AST shape and dispatches it
to the strategy that fits, all inside the same process.

> **TL;DR**: there is no second engine binary, no runtime backend
> selector, and no fallback to a cloud BigQuery service. There is also
> no claim that "everything becomes DuckDB SQL." Inside `emulator_main`,
> a route classifier picks one of five local strategies (DuckDB fast
> path, DuckDB rewrite, DuckDB UDF/polyfill, local semantic executor,
> control-op handler) per query, and shapes that have no planned local
> route surface a deliberate, BigQuery-shaped `UNIMPLEMENTED`. See
> "History" below for the runtimes this layout replaces.

## The runtime

| Component | Source | Notes |
|---|---|---|
| **`Engine` interface** | `backend/engine/engine.h` | Single public interface seen by the gRPC layer. Implementation behind it is the local execution coordinator. |
| **Local execution coordinator** | `backend/engine/` | Owns the route classifier and dispatches each query to one of the routes documented below. Today the coordinator degenerates to "always DuckDB fast path"; the additional routes land via the local-execution roadmap (`.cursor/plans/local-execution-roadmap-index.plan.md`). |
| **DuckDB fast path** | `backend/engine/duckdb/` | The transpiler + DuckDB execution surface. Produces DuckDB SQL via a `googlesql::ResolvedASTVisitor` and runs it through DuckDB's C++ client. |
| **DuckDB rewrites + UDFs** | (planned, `backend/engine/duckdb/`) | DuckDB SQL expressed via rewrites or DuckDB UDFs/macros to make a BigQuery function correct locally. |
| **Local semantic executor** | (planned, `backend/engine/semantic/`) | A local row/array/value interpreter for shapes that demand exact BigQuery semantics. |
| **Control-op executor** | (planned, `backend/engine/control/`) | DDL / metadata / catalog ops routed straight through the storage layer instead of through query execution. |
| **`Storage` interface** | `backend/storage/storage.h` | Single public interface. |
| **DuckDB storage** | `backend/storage/duckdb/` | Sole storage implementation. Catalog + table rows persist to a `catalog.duckdb` file under `--data_dir`. |

`emulator_main` does not accept `--engine`, `--storage`, or
`--on_unknown_fn`; those flags were removed when the ReferenceImpl
engine and in-memory storage were deleted, and the new multi-strategy
coordinator intentionally keeps the same single-knob posture (route
selection happens internally at AST-classification time, not via a
runtime flag). The only knobs are `--host_port` (default
`localhost:9060`) and `--data_dir` (default `$HOME/.bigquery-emulator`,
with `./.bigquery-emulator` as the fallback when `HOME` is unset).

## Routes

The route classifier maps every `ResolvedAST` shape (and every
built-in function) to exactly one route. The same vocabulary appears
in [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](../backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)
on a per-node basis.

| Route                | Meaning                                                                                                              | What it does at runtime                                                                                |
|----------------------|----------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------|
| `duckdb_native`      | Lowers cleanly to DuckDB SQL with semantics that already match BigQuery's exactly.                                   | Transpiles to DuckDB SQL and runs it through DuckDB. Today's primary path.                             |
| `duckdb_rewrite`     | Lowers to DuckDB SQL with a deliberate structural rewrite (e.g. struct/array shape rewrites, JSON operator mapping). | Same execution path as `duckdb_native`; the rewrite lives in the transpiler.                           |
| `duckdb_udf`         | Lowers to DuckDB SQL that calls a DuckDB UDF / macro we register at engine startup.                                  | Same DuckDB connection runs the UDF locally; the UDF body owns the BigQuery-specific semantics.        |
| `semantic_executor`  | Runs on a local row/value interpreter that owns exact BigQuery semantics.                                            | Uses DuckDB only as a row source; expression evaluation, type coercion, and error surfaces are local.  |
| `control_op`         | DDL / metadata / catalog op.                                                                                         | Bypasses query execution; the storage layer applies the change and emits the BigQuery-shaped response. |
| `unsupported`        | Deliberately out of scope locally.                                                                                   | Returns a BigQuery-shaped `UNIMPLEMENTED` (or, when called for, an `INVALID_ARGUMENT`).                |

A few rules the router obeys:

1. **One route per shape.** A `ResolvedAST` node kind picks exactly
   one route. We do not catch DuckDB errors at runtime and retry on a
   semantic executor — drift between strategies is hidden by silent
   fallback, and we removed the old `FallbackEngine` precisely
   because of that.
2. **Compositional fallback at lowering time, not runtime.** When the
   DuckDB fast path can lower most of a query but a leaf node is
   `semantic_executor`, the router promotes the surrounding shape to
   the semantic executor at planning time instead of mixing strategies
   mid-execution.
3. **Route labels are observable in tests.** Conformance fixtures may
   assert which route served a query
   (`conformance-routing-matrix.plan.md`), so a passing fixture cannot
   silently drift from `duckdb_native` to `semantic_executor`.
4. **`unsupported` is intentional.** Promoting a row out of
   `unsupported` requires a planned route, a landing implementation,
   and conformance coverage. We never silently approximate.

## What this means in practice

1. **New feature work has a route, not "a transpiler row."** Identify
   the right local strategy (DuckDB native, DuckDB rewrite, DuckDB
   UDF, semantic executor, control op) and put the work in the
   matching plan from
   [`.cursor/plans/local-execution-roadmap-index.plan.md`](../.cursor/plans/local-execution-roadmap-index.plan.md).
   Update the shape tracker row in the same commit.

2. **Some DML / DDL shapes are still on `unsupported` today.** As of
   this revision the coordinator returns `UNIMPLEMENTED` for:
   - `INSERT VALUES` / `INSERT ... SELECT`
   - `UPDATE`
   - `DELETE`
   - Scalar-only `SELECT` (no `FROM` clause)

   Use `tabledata.insertAll` to seed rows for tests and fixtures
   while these gaps are closed. `MERGE`, `CREATE TABLE`,
   `CREATE TABLE AS SELECT`, and `DROP TABLE` are all implemented
   today; the harder DML branches will land via
   `dml-local-executor.plan.md` on the `semantic_executor` route.

3. **Storage follows the same single-implementation rule.** The
   in-memory storage backend is gone; every persistent state path
   goes through `DuckDBStorage`. Tests that previously used a
   volatile in-memory store now allocate a temp `--data_dir` and
   rely on DuckDB instead.

4. **There is no cloud passthrough.** This emulator never forwards
   query work to the real BigQuery service. Local coverage is the
   responsibility of the multi-strategy coordinator inside this
   process, full stop.

## Implication for conformance fixtures

The conformance harness (`conformance/cmd/runner`,
`conformance/fixtures/*.yaml`) runs a single profile today:

| Profile | Engine                      | Storage |
|---------|-----------------------------|---------|
| `local` | local execution coordinator | duckdb  |

The legacy profile name `duckdb` refers to the same coordinator
binary and is accepted by the runner for backwards compatibility
during the rename. New fixtures should use `local`.

Per the policy above:

- **Leave `profiles:` unset** in new fixtures unless you are
  intentionally targeting a future second profile. Today the default
  profile set is `[local]` and the harness runs every fixture against
  it.
- **Use `rows:` setup steps** (which call `tabledata.insertAll`)
  instead of `sql:` `INSERT VALUES` for seeding, since INSERT is on
  the `unsupported` route on the coordinator today (it will move to
  the semantic executor when `dml-local-executor.plan.md` lands).
- **Document `unsupported` gaps loudly.** If a fixture is blocked on
  an `unsupported` shape, leave it out of the suite rather than
  `t.Skip()`-ing it; the conformance harness's purpose is to pin what
  works.
- **Route labels are optional today, asserted soon.** Once
  `conformance-routing-matrix.plan.md` lands, fixtures will be able
  to assert "this query ran on the DuckDB fast path" or "this query
  ran on the semantic executor" alongside the row output.

## History

A previous iteration of the emulator carried two engines
(ReferenceImpl + DuckDB) bridged by a `FallbackEngine` wrapper that
routed DuckDB-uncovered constructs to ReferenceImpl at **runtime**,
plus an in-memory storage backend for hermetic tests. That layout was
removed because:

- ReferenceImpl coverage was incomplete (no DDL, partial DML,
  missing analytics functions) and not actively maintained.
- The runtime fallback bridge made it ambiguous which engine produced
  any given result; production users reported subtle divergences when
  fixtures ran on `duckdb` but tests ran on `memory`.
- Maintaining two storage backends doubled the test-fixture cost
  for no production benefit, since `docker compose up` always
  used the persistent DuckDB store anyway.

The removal commit landed with a `BREAKING CHANGE:` footer noting
that `--engine=reference_impl`, `--storage=memory`, and
`--on_unknown_fn=fallback` are gone.

After that removal, the project briefly described itself as a
"DuckDB-only" emulator and tried to extend DuckDB coverage by
promoting every remaining shape to a transpiler row. That framing
underestimated the BigQuery vs. DuckDB semantic gap and would have
forced the transpiler to absorb work it is the wrong tool for. The
current policy — local multi-strategy execution behind a single
`Engine` — is the deliberate replacement: DuckDB stays as the fast
analytical path, and the BigQuery-specific work moves to whichever
local strategy actually owns it. Route selection happens at
AST-classification time rather than at runtime error-catch time, so
it never reintroduces the silent-drift hazard the old
`FallbackEngine` had.

## Cross-references

- [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](../backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) — per-node route dispositions (`duckdb_native`, `duckdb_rewrite`, `duckdb_udf`, `semantic_executor`, `control_op`, `unsupported`).
- [`.cursor/plans/local-execution-roadmap-index.plan.md`](../.cursor/plans/local-execution-roadmap-index.plan.md) — execution order, terminology, and per-route plans.
- [`conformance/README.md`](../conformance/README.md) — fixture authoring guide; references this document from its "Contributing a new fixture" section.
- [`README.md` "Runtime configuration"](../README.md#runtime-configuration) — the user-facing version of the flag surface this document governs.
