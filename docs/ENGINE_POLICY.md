# Execution policy

This emulator ships **one local emulator process** with **one C++
`Engine` interface** (`backend/engine/engine.h`) and **one persistent
storage implementation** (`DuckDBStorage`). Behind that single
`Engine`, query execution is **multi-strategy**: a local execution
router classifies each resolved GoogleSQL AST shape and dispatches it
to the strategy that fits, all inside the same process.

> **TL;DR**: there is no second engine binary, no runtime backend
> selector, and no fallback to a cloud BigQuery service. There is also
> no claim that the DuckDB transpiler is the universal lowering
> target. Inside `emulator_main`, a route classifier picks one of
> seven local dispositions (DuckDB fast path, DuckDB rewrite, DuckDB
> UDF/polyfill, local semantic executor, control-op handler,
> local-stub, or the deliberate `unsupported` envelope) per query,
> and shapes that have no planned local route surface a BigQuery-
> shaped `UNIMPLEMENTED` whose message names the offending family
> and links back here. See "History" below for the runtimes this
> layout replaces.

## The runtime

| Component | Source | Notes |
|---|---|---|
| **`Engine` interface** | `backend/engine/engine.h` | Single public interface seen by the gRPC layer. Implementation behind it is the local execution coordinator. |
| **Local execution coordinator** | `backend/engine/coordinator/` (`LocalCoordinatorEngine`) | Owns the route classifier and dispatches each query to one of the routes documented below. The classifier consumes the `node_dispositions.yaml` / `functions.yaml` registries (built into `backend/engine/duckdb/transpiler/`) at build time and returns a `RouteDecision{disposition; reason; offending_node}` per resolved statement. |
| **Route classifier** | `backend/engine/coordinator/route_classifier.{h,cc}` | Walks the resolved AST, consults the disposition registries, and picks one of `kDuckdbNative / kDuckdbRewrite / kDuckdbUdf / kSemanticExecutor / kControlOp / kLocalStub / kUnsupported`. Compositional fallback at planning time: any leaf with a higher-priority disposition promotes the whole query; planned rows do NOT promote (the fast path handles them via the transpiler's empty-string contract). Priority order is `unsupported > local_stub > semantic_executor > control_op > duckdb_udf > duckdb_rewrite > duckdb_native`, so a SELECT mixing a local-stub function (e.g. `KEYS.NEW_KEYSET`) and an unsupported function (`APPROX_QUANTILES`) surfaces `UNIMPLEMENTED` rather than a stubbed answer. |
| **DuckDB fast path** | `backend/engine/duckdb/` (`DuckDbExecutor`) | The transpiler + DuckDB execution surface. Produces DuckDB SQL via a `googlesql::ResolvedASTVisitor` and runs it through DuckDB's C++ client. Implements the `coordinator::Executor` interface (`ExecuteQuery` / `ExecuteDml` / `ExecuteDdl`); the coordinator hands it a pre-analyzed `ResolvedStatement`. |
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
| `local_stub`         | Deliberate BigQuery-shaped placeholder for a specialized family the emulator does not model end-to-end.              | Function-level stubs (e.g. `KEYS.NEW_KEYSET`) dispatch through the semantic executor's stub table (`backend/engine/semantic/stubs/`) and return a fixed-shape sentinel of the documented BigQuery return type. Statement-level stubs (e.g. `CREATE MODEL`) are pre-dispatched from the coordinator's `ExecuteDdl` to `backend/engine/control/stubs/` and return OK without persisting anything; the matching evaluator (`ML.PREDICT`, ...) stays on `unsupported` so a downstream call surfaces `UNIMPLEMENTED`. The two halves together preserve client-library startup-probe compatibility without silently approximating any downstream semantics. |
| `unsupported`        | Deliberately out of scope locally.                                                                                   | Returns a BigQuery-shaped `UNIMPLEMENTED` whose message names the offending family (e.g. `family: function:keys.encrypt`) and links to this document and `googlesqlite-15-specialized-stubs.plan.md`. |

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

## Specialized features

A handful of BigQuery feature families are intentionally NOT modeled
end-to-end by this emulator. The full posture decision -- per-family
choice between `local_impl`, `local_stub`, and `unsupported`, plus the
rationale -- lives in
[`.cursor/plans/googlesqlite-15-specialized-stubs.plan.md`](../.cursor/plans/googlesqlite-15-specialized-stubs.plan.md).
The plan file's "Per-family postures" table is the source of truth; this
section is the user-facing summary the unsupported error envelope
points at.

| Family                                                                                                       | Posture     | What happens                                                                                                                                                                                                |
|---|---|---|
| BigQuery ML (`ML.PREDICT`, `ML.FORECAST`, `ML.EVALUATE`)                                                     | `unsupported` | `UNIMPLEMENTED` with `family: function:ml.<name>` and a link here.                                                                                                                                          |
| BigQuery ML `CREATE MODEL`                                                                                   | `local_stub`  | Accepted as metadata-only; returns OK without registering a model. A subsequent `ML.PREDICT` over the named model still surfaces `UNIMPLEMENTED` (the family stays unsupported).                            |
| Geography / GIS (`ST_*`)                                                                                     | `unsupported` | `UNIMPLEMENTED` with `family: function:st_<name>`.                                                                                                                                                          |
| Differential privacy / anonymized aggregation (`AnonymizedAggregate*`, `DifferentialPrivacyAggregate*`, ...) | `unsupported` | `UNIMPLEMENTED`. The DP guarantee depends on noise calibration the emulator cannot honor.                                                                                                                   |
| Networking (`NET.*`)                                                                                         | `local_impl`  | Routes to the semantic executor; implementation deferred to `googlesqlite-09-date-time.plan.md`. Surfaces UNIMPLEMENTED until that plan lands the body.                                                |
| Key management (`KEYS.NEW_KEYSET`, `KEYS.KEYSET_LENGTH`)                                                     | `local_stub`  | Returns a deterministic BigQuery-shaped sentinel (`KEYS.NEW_KEYSET` -> a fixed `BYTES` envelope; `KEYS.KEYSET_LENGTH` -> `1`). NOT a real Tink keyset.                                                      |
| Key management (`KEYS.ENCRYPT`, `KEYS.DECRYPT_BYTES`)                                                        | `unsupported` | Encryption-bearing entries deliberately fail loudly so a downstream consumer cannot round-trip the stub sentinel into an actual AEAD operation.                                                            |
| HLL (`HLL_COUNT.*`)                                                                                          | `local_impl`  | Routes to the semantic executor; implementation deferred to `googlesqlite-09-date-time.plan.md`.                                                                                                       |
| Protobuf shapes (`ResolvedMakeProto`, `ResolvedGetProtoField`, ...)                                          | `unsupported` | The emulator does not model the GoogleSQL proto type surface end-to-end.                                                                                                                                    |
| MEASURE / measure functions (`AGGREGATE(<measure>)`)                                                         | `unsupported` | Measure types require BigQuery's analytical layer the emulator does not model.                                                                                                                              |
| Graph (`GRAPH_TABLE`, GQL subqueries, `ResolvedGraph*Scan`)                                                  | `unsupported` | The Spanner-Graph SQL surface is not part of this emulator's scope.                                                                                                                                         |
| Sequences (`ResolvedSequence`, `NEXT VALUE FOR`)                                                             | `unsupported` | BigQuery does not ship general SQL sequences; the analyzer-visible surface is anchored on `unsupported` to make the gap explicit.                                                                          |
| JavaScript UDFs (`CREATE FUNCTION ... LANGUAGE js`)                                                          | `local_stub` (deferred) | Posture is `local_stub` (metadata-only registration; call site returns `UNIMPLEMENTED` naming JavaScript). Implementation is BLOCKED on `googlesqlite-15-specialized-stubs.plan.md` family 4 (cross-request UDF body storage through `DuckDBStorage`'s catalog -- deferred at the end of plan 13). Until both land, `CREATE FUNCTION ... LANGUAGE js` surfaces `UNIMPLEMENTED` from the control-op executor and the body is NOT persisted (registering a body that does not persist would silently approximate the BigQuery contract). |
| `LOAD DATA LOCAL <local-uri>`                                                                                | `control_op` (deferred) | Belongs on the control-op route; the local-filesystem reader family is tracked by `googlesqlite-01-ddl-catalog.plan.md`'s follow-up "add LOAD DATA reader family." Currently surfaces `UNIMPLEMENTED`.             |
| `LOAD DATA <gs://...>` (cloud storage)                                                                       | `unsupported` | The emulator does not model BigQuery's cloud-storage ingest surface.                                                                                                                                        |

The two halves of the `local_stub` posture are load-bearing:

1. **BigQuery-shaped placeholder** -- client-library startup probes
   (e.g. a connector that issues `SELECT KEYS.NEW_KEYSET(...)` to
   confirm the dialect understands the namespace) succeed against the
   emulator without forcing the user to disable their probe.
2. **No silent approximation downstream** -- the call site that would
   actually consume the stub value (`KEYS.ENCRYPT(...)`, `ML.PREDICT(
   MODEL <id>, ...)`) stays on `unsupported` so a misuse fails loudly
   rather than emitting a plausible-looking-but-fake answer.

When the unsupported route surfaces an envelope, the message names the
offending family (e.g. `family: function:keys.encrypt`, `family:
function:ml.predict`, `family: ResolvedSequenceStmt`) so a user landing
on this document can find the matching row in the table above without
running the route classifier in their head.

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
   `googlesqlite-14-dml-system.plan.md` on the `semantic_executor` route.

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
  the semantic executor when `googlesqlite-14-dml-system.plan.md` lands).
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

- [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](../backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) — per-node route dispositions (`duckdb_native`, `duckdb_rewrite`, `duckdb_udf`, `semantic_executor`, `control_op`, `local_stub`, `unsupported`).
- [`.cursor/plans/local-execution-roadmap-index.plan.md`](../.cursor/plans/local-execution-roadmap-index.plan.md) — route vocabulary, foundation plans, and link to the googlesqlite index.
- [`.cursor/plans/googlesqlite-00-index.plan.md`](../.cursor/plans/googlesqlite-00-index.plan.md) — **active work**: sequential plans 01→16 for the ported googlesqlite query suite.
- [`.cursor/plans/googlesqlite-15-specialized-stubs.plan.md`](../.cursor/plans/googlesqlite-15-specialized-stubs.plan.md) — per-family posture decisions (`local_impl` vs `local_stub` vs `unsupported`); source of truth for the "Specialized features" section above.
- [`conformance/README.md`](../conformance/README.md) — fixture authoring guide; references this document from its "Contributing a new fixture" section.
- [`README.md` "Runtime configuration"](../README.md#runtime-configuration) — the user-facing version of the flag surface this document governs.
