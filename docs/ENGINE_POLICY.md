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
| **DuckDB rewrites + UDFs** | `backend/engine/duckdb/` | DuckDB SQL expressed via rewrites or DuckDB UDFs/macros to make a BigQuery function correct locally. |
| **Local semantic executor** | `backend/engine/semantic/` | A local row/array/value interpreter for shapes that demand exact BigQuery semantics. |
| **Control-op executor** | `backend/engine/control/` | DDL / metadata / catalog ops routed straight through the storage layer instead of through query execution. |
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
| `unsupported`        | Deliberately out of scope locally.                                                                                   | Returns a BigQuery-shaped `UNIMPLEMENTED` whose message names the offending family (e.g. `family: function:keys.encrypt`) and links to this document. |

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
   assert which route served a query (see [`conformance/README.md`](../conformance/README.md)),
   so a passing fixture cannot silently drift from `duckdb_native` to
   `semantic_executor`.
4. **`unsupported` is intentional.** Promoting a row out of
   `unsupported` requires a planned route, a landing implementation,
   and conformance coverage. We never silently approximate.

## Specialized features

A handful of BigQuery feature families are intentionally NOT modeled
end-to-end by this emulator. The table below is the user-facing
summary the unsupported error envelope points at. Per-family posture
(`local_impl` vs `local_stub` vs `unsupported`) is recorded in
`functions.yaml` / `node_dispositions.yaml` and mirrored in
`SHAPE_TRACKER.md`.

| Family                                                                                                       | Posture     | What happens                                                                                                                                                                                                |
|---|---|---|
| BigQuery ML (`ML.PREDICT`, `ML.FORECAST`, `ML.EVALUATE`)                                                     | `unsupported` | `UNIMPLEMENTED` with `family: function:ml.<name>` and a link here.                                                                                                                                          |
| BigQuery ML `CREATE MODEL`                                                                                   | `local_stub`  | Accepted as metadata-only; returns OK without registering a model. A subsequent `ML.PREDICT` over the named model still surfaces `UNIMPLEMENTED` (the family stays unsupported).                            |
| Geography / GIS (`ST_*`)                                                                                     | `unsupported` (partial) | The `ST_GEOGPOINT` constructor and `GEOGRAPHY` value plumbing are partially landed on the semantic executor (`backend/engine/semantic/functions/geog_funcs.cc`); the broader `ST_*` surface (`ST_DISTANCE`, `ST_WITHIN`, `ST_CONTAINS`, `ST_INTERSECTS`, WKT/WKB constructors, ...) still surfaces `UNIMPLEMENTED` with `family: function:st_<name>` pending a DuckDB `spatial`-extension pass. |
| Differential privacy / anonymized aggregation (`AnonymizedAggregate*`, `DifferentialPrivacyAggregate*`, ...) | `unsupported` | `UNIMPLEMENTED`. The DP guarantee depends on noise calibration the emulator cannot honor.                                                                                                                   |
| Approximate aggregation (`APPROX_QUANTILES`, `APPROX_COUNT_DISTINCT`, `APPROX_TOP_COUNT`, `APPROX_TOP_SUM`)   | `local_impl`  | Routes to the semantic executor (`backend/engine/semantic/functions/aggregate_specialized.cc`). Results are computed exactly (not sketch-approximated); not differential-privacy aggregation (see the DP row above). |
| Networking (`NET.*`)                                                                                         | `local_impl`  | Routes to the semantic executor; implemented in `net_funcs.cc` (IP parse/mask/trunc, HOST, PUBLIC_SUFFIX, REG_DOMAIN). Pinned by `conformance/fixtures/specialized/net_host_reg_domain.yaml`. |
| Key management (`KEYS.NEW_KEYSET`, `KEYS.KEYSET_LENGTH`)                                                     | `local_stub`  | Returns a deterministic BigQuery-shaped sentinel (`KEYS.NEW_KEYSET` -> a fixed `BYTES` envelope; `KEYS.KEYSET_LENGTH` -> `1`). NOT a real Tink keyset.                                                      |
| Key management (`KEYS.ENCRYPT`, `KEYS.DECRYPT_BYTES`)                                                        | `unsupported` | Encryption-bearing entries deliberately fail loudly so a downstream consumer cannot round-trip the stub sentinel into an actual AEAD operation.                                                            |
| HLL (`HLL_COUNT.*`)                                                                                          | `local_impl`  | Routes to the semantic executor; `hll_funcs.cc` implements INIT/MERGE/MERGE_PARTIAL/EXTRACT with a local sketch wire format (not byte-compatible with cloud BigQuery). Pinned by `conformance/fixtures/specialized/hll_count_round_trip.yaml`. |
| Protobuf shapes (`ResolvedMakeProto`, `ResolvedGetProtoField`, ...)                                          | `unsupported` | The emulator does not model the GoogleSQL proto type surface end-to-end.                                                                                                                                    |
| MEASURE / measure functions (`AGGREGATE(<measure>)`)                                                         | `unsupported` | Measure types require BigQuery's analytical layer the emulator does not model.                                                                                                                              |
| Graph (`GRAPH_TABLE`, GQL subqueries, `ResolvedGraph*Scan`)                                                  | `unsupported` | The Spanner-Graph SQL surface is not part of this emulator's scope.                                                                                                                                         |
| Sequences (`ResolvedSequence`, `NEXT VALUE FOR`)                                                             | `unsupported` | BigQuery does not ship general SQL sequences; the analyzer-visible surface is anchored on `unsupported` to make the gap explicit.                                                                          |
| JavaScript UDFs (`CREATE FUNCTION ... LANGUAGE js`)                                                          | `local_stub`  | `CREATE FUNCTION ... LANGUAGE js` registers metadata-only `External_function` entries and persists the DDL in `DuckDBStorage` so `routines.get` round-trips; call-time evaluation surfaces `UNIMPLEMENTED` until an embedded JS runtime lands (the two-halves stub contract). |
| Python UDFs (`CREATE FUNCTION ... LANGUAGE python`)                                                        | `unsupported` | No Python UDF runtime; `cw_xml_extract` stays in bqutils `known_failing/` as a documented external-language gap (same disposition as JS UDFs). |
| SQL scalar UDFs (`CREATE FUNCTION ... AS (...)`), including `ANY TYPE` templated parameters                  | `semantic_executor`     | `CREATE FUNCTION` registers through the per-project UDF registry (`backend/catalog/udf_registry.cc`), writes through to `DuckDBStorage` (`__bqemu_routines`), and replays into each query's `GoogleSqlCatalog` (including after engine restart via `RehydrateRoutinesFromStorage`), shadowing a built-in when the names collide. Templated bodies evaluate on the semantic executor via `EvalSqlUdfBody`; SQL UDAFs evaluate via `EvalSqlUdafBody`. SQL TVFs and procedures follow the same persistence path through `tvf_registry` / `procedure_registry`. `DROP FUNCTION` deletes registry + storage rows. Conformance fixtures under `conformance/fixtures/udf/`. |
| Scripting (`DECLARE`, `SET`, `CALL`, `BEGIN…END` multi-stmt blocks)                                          | `semantic_executor`     | Gateway sends DECLARE/SET/CALL scripts in one engine `ExecuteQuery` round-trip (`script_runner_engine.go`); control-flow scripts register a single child job for the final SELECT result. Simple statements (`CREATE CONSTANT`/`SET`/`CALL`/`ASSERT`/`EXECUTE IMMEDIATE` literals) run via `ExecuteScriptViaAnalyzeNext`; scripts with structured control flow (`IF`/`WHILE`/`LOOP`/`FOR`/`EXECUTE IMMEDIATE`/`EXCEPTION`/`RAISE` in blocks) delegate to `googlesql::ScriptExecutor` through `EmulatorStatementEvaluator` when `BIGQUERY_EMULATOR_HAS_GOOGLESQL_SCRIPTING=1` (always set once the prebuilt artifact ships `//googlesql/scripting:script_executor`). `@@error.*` reads resolve through `EvalContext::script_system_variables` (populated by the ScriptExecutor during `EXCEPTION` handlers). Conformance fixtures under `conformance/fixtures/scripting/`. |
| `LOAD DATA LOCAL <local-uri>` / `LOAD DATA ... FROM FILES (uris = ['file://...'])`                           | `control_op`  | Local CSV/JSON/Parquet readers via DuckDB (`RunLoadData`); `gs://` URIs surface unsupported. Pinned by `conformance/fixtures/ddl/export_load_round_trip.yaml`. |
| `LOAD DATA <gs://...>` (cloud storage)                                                                       | `unsupported` | The emulator does not model BigQuery's cloud-storage ingest surface.                                                                                                                                        |
| `EXPORT DATA` (local `file://` URI)                                                                          | `control_op`  | DuckDB `COPY (SELECT ...) TO` for CSV/JSON/Parquet; `gs://` URIs surface unsupported. Pinned by `conformance/fixtures/ddl/export_load_round_trip.yaml`. |
| `CREATE MATERIALIZED VIEW`                                                                                   | `control_op`  | Full-refresh materialization at creation only (no incremental refresh). Stored as a regular table; reads use the existing table-scan path. Pinned by `conformance/fixtures/ddl/materialized_view_query.yaml`. |
| `INFORMATION_SCHEMA.*` reflection views                                                                      | `duckdb_native` | `<dataset>.INFORMATION_SCHEMA.<VIEW>` and region-qualified `` `region-<r>`.INFORMATION_SCHEMA.<VIEW> `` resolve at analyze time through `backend/catalog/info_schema_table.{h,cc}` (a `VirtualCatalogTable`) and materialize from `Storage` + the routine/view registries: `TABLES`, `COLUMNS`, `SCHEMATA`, `VIEWS` (view registry), `ROUTINES` (`__bqemu_routines`), `COLUMN_FIELD_PATHS` (recursed STRUCT/ARRAY paths), `PARTITIONS` / `TABLE_STORAGE` (`Storage::CountRows`, single unpartitioned partition; byte columns NULL per the persistence non-goal), and `TABLE_OPTIONS` / `KEY_COLUMN_USAGE` (empty — options/constraints are not modeled). `JOBS` / `JOBS_BY_PROJECT` stay unresolved (NOT_FOUND): job state lives in the Go gateway, not the engine `Storage`, so faking it would violate the no-approximation rule (see `.cursor/plans/full-01-information-schema.plan.md`). Conformance fixtures under `conformance/fixtures/info_schema/`. |

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
   UDF, semantic executor, control op), land the implementation, and
   update the shape tracker row in the same commit.

2. **Some DML / DDL shapes are still deferred today.** Remaining DML
   gaps (pipe-operator forms, `ResolvedPipeInsertScan`, ...) land on the
   `semantic_executor` route as handlers are added. Landed today on the local DML executor
   (`backend/engine/semantic/dml/`): `INSERT VALUES`, `INSERT ...
   SELECT`, scalar and deep-STRUCT `UPDATE`, nested `(DELETE arr WITH OFFSET ...)`
   in `UPDATE SET`, `UPDATE ... FROM`,
   `DELETE`, `THEN RETURN` on INSERT/UPDATE/DELETE/MERGE,
   `ASSERT_ROWS_MODIFIED`, and the harder MERGE matrix (`WHEN NOT
   MATCHED BY SOURCE`, multi-action sequences via `dml_merge.cc`).
   Simple `MERGE` (`WHEN MATCHED` + single `WHEN NOT MATCHED BY
   TARGET`) stays on the DuckDB fast path. Use `tabledata.insertAll`
   to seed rows for tests and fixtures while gaps are closed.
   `CREATE TABLE`, `CREATE TABLE AS SELECT`, `DROP TABLE`, `ALTER TABLE`,
   `CREATE MATERIALIZED VIEW` (full refresh), `EXPORT DATA`, and
   `LOAD DATA` (local files) are implemented today on the control-op
   route.

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
- **Seed with `sql:` `INSERT VALUES` or `rows:` setup steps** —
  `INSERT` executes through the local DML executor, and `rows:`
  steps (which call `tabledata.insertAll`) remain available for
  fixtures that want to bypass DML entirely.
- **Document `unsupported` gaps loudly.** If a fixture is blocked on
  an `unsupported` shape, leave it out of the suite rather than
  `t.Skip()`-ing it; the conformance harness's purpose is to pin what
  works.
- **Route labels are optional.** Fixtures may assert which route
  served a query alongside the row output (`expected.route`), and
  `task conformance:routing-matrix` reports the observed route for
  every fixture.

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

## BigQuery Storage gRPC (engine `bigquery_emulator.v1.*`)

The C++ engine on `:9060` implements an **internal** Storage Read/Write
contract (`proto/storage_read.proto`, `proto/storage_write.proto`).
Official client libraries dial the **public** service names
(`google.cloud.bigquery.storage.v1.BigQueryRead` /
`BigQueryWrite`) and wire formats (Arrow/Avro read pages, proto-descriptor
row encoding for `JsonStreamWriter`). The Go gateway shim in
`gateway/handlers/bqstorage/` registers those public services on
`:9060` and adapts to the engine's internal contract. Java
`WriteBufferedStreamIT` and `StorageArrowSampleIT` pass against the
local emulator; Connection + DataTransfer ITs remain allowlisted.

| Surface | Posture |
|---|---|
| Storage Write `COMMITTED` / `_default` | Engine `AppendRows` commit path; shim decodes public proto rows |
| Storage Write `BUFFERED` | Engine buffered hold + `FlushRows` / `FinalizeWriteStream`; shim caches proto descriptors and reuses the emulator `BigQueryWriteClient` in samples |
| Storage Write `PENDING` | Engine buffered hold + `FinalizeWriteStream` + `BatchCommitWriteStreams` through `DuckDBStorage::AppendRows`; gateway shim forwards public RPCs |
| Storage Read sessions | `CreateReadSession` with projection + analyzer-transpiled `row_restriction`, multi-stream `max_stream_count`, and `SplitReadStream`; public-data tables readable with a caller-scoped parent project |
| Storage Read Arrow | Arrow schema + IPC record batches via gateway shim |
| Storage Read Avro | Arrow read from engine, Avro OCF encoding in gateway shim (`gateway/handlers/bqstorage/avro.go`) |

Set `BIGQUERY_STORAGE_GRPC_ENDPOINT` (default `localhost:9060` in
`task thirdparty:*`) to reach the gateway gRPC listener. ManagedWriter /
Storage Read subtests skip when it is unset.

The same `:9060` listener multiplexes additional public BigQuery gRPC
services the Go sample suites exercise:

| Public service | Posture |
|---|---|
| `google.cloud.bigquery.storage.v1.BigQueryRead` / `BigQueryWrite` | Gateway shim → engine internal Storage Read/Write |
| `google.cloud.bigquery.connection.v1.ConnectionService` | Shallow list/create/get stubs (`gateway/handlers/bqconnection/`) |
| `google.cloud.bigquery.reservation.v1.ReservationService` | Empty list stubs (`gateway/handlers/bqreservation/`) |
| `google.cloud.bigquery.analyticshub.v1.AnalyticsHubService` | In-memory exchange/listing CRUD (`gateway/handlers/bqanalyticshub/`) |
| `google.cloud.bigquery.v2.*` (Dataset/Table/Job/Project/Routine) | Thin gRPC adapters over existing REST/catalog handlers (`gateway/handlers/bqv2grpc/`) |

Set `BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT` to the same host:port when
samples dial Analytics Hub separately. BigQuery v2 preview clients reuse
`BIGQUERY_STORAGE_GRPC_ENDPOINT` via `bqopts.BigQueryV2GRPCClientOptions()`.

## Google Sheets external tables

The emulator does not call the Google Sheets API. Python thirdparty
samples `test_query_external_sheets_*` are skipped via
`third_party/python-bigquery-tests/emulator_pytest_skip.py` when
`BIGQUERY_EMULATOR_HOST` is set. GCS-backed external tables remain in
scope.

## Relational UNNEST / lateral / correlated subqueries

These shapes share the semantic executor's per-outer-row evaluation
frame (`backend/engine/semantic/outer_row_eval.{h,cc}`). The
classifier promotes divergent subsets to `semantic_executor`; the
fast path keeps the standalone cases on `duckdb_native`.

| Family | Representative SQL | Route | Conformance |
|---|---|---|---|
| Standalone UNNEST | `SELECT n FROM UNNEST([1,2,3]) AS n` | `duckdb_native` | fast-path fixtures |
| UNNEST WITH OFFSET | `... UNNEST(arr) AS n WITH OFFSET AS idx` | `semantic_executor` | `array_struct/unnest_with_offset.yaml` |
| Multi-array zip | `FROM UNNEST(a, b)` (`array_zip_mode`) | `semantic_executor` | `array_struct/multi_array_unnest_pad.yaml` |
| Cross-join UNNEST | `FROM t, UNNEST(t.arr) AS n` | `semantic_executor` | `array_struct/cross_join_unnest.yaml` |
| LEFT JOIN UNNEST | `LEFT JOIN UNNEST(t.arr) AS n ON TRUE` | `semantic_executor` | `array_struct/left_join_unnest.yaml` |
| JOIN USING | `INNER JOIN t2 USING (id)` | `duckdb_native` | `fastpath/join_using_inner.yaml` |
| Lateral join scan | `... JOIN LATERAL (...)` (`is_lateral`) | `semantic_executor` | unit tests (`MaterializeJoinScan`) |
| Correlated subquery | `EXISTS (SELECT 1 FROM r WHERE r.k = o.k)` | `semantic_executor` | `cte_subquery/subquery_expr_correlated_exists.yaml` |

## Cast extensions, COLLATE, value tables, set-op CORRESPONDING

These expression / projection edges promote divergent subsets off the
DuckDB fast path; the semantic executor owns exact BigQuery semantics
where noted.

| Family | Representative SQL | Route | Conformance |
|---|---|---|---|
| CAST FORMAT | `CAST(DATE '2018-01-30' AS STRING FORMAT 'YYYY')` | `semantic_executor` | `scalar/cast_format_date_to_string.yaml`, `cast_format_parse_date.yaml` |
| CAST AT TIME ZONE | `CAST(TIMESTAMP ... AS STRING AT TIME ZONE 'UTC')` | `semantic_executor` | `scalar/cast_timestamp_at_timezone.yaml` |
| COLLATE in ORDER BY | `ORDER BY name COLLATE 'und:ci'` | `semantic_executor` | `scalar/order_by_collate_und_ci.yaml` |
| SELECT AS VALUE | `SELECT AS VALUE 42 AS n` | `semantic_executor` | `scalar/select_as_value_scalar.yaml` |
| Set-op CORRESPONDING | `UNION ALL CORRESPONDING` | `duckdb_native` (subset) | `setops/set_op_corresponding_union_all.yaml` |

Extended-cast / type-modifier casts (`STRING(n)`, `NUMERIC(p,s)`, …)
still surface `UNIMPLEMENTED` on the semantic path until a follow-up
plan lands them.

## Exact-decimal (`NUMERIC` / `BIGNUMERIC`)

BigQuery's `NUMERIC` (`DECIMAL(38,9)`) and `BIGNUMERIC`
(`DECIMAL(38,38)`) are exact decimals. DuckDB can store and compare
`NUMERIC` natively, but it widens some operations to `DOUBLE` and
cannot store `BIGNUMERIC` as a `DECIMAL` at all (its max precision is
38, which cannot represent `DECIMAL(38,38) >= 1.0`), so `BIGNUMERIC`
is persisted as `VARCHAR`. Following the no-silent-approximation rule,
shapes DuckDB would widen or reject reroute to the semantic executor's
decimal path (`googlesql::NumericValue` / `BigNumericValue`) rather
than approximating in `DOUBLE`.

| Family | Representative SQL | Route | Conformance |
|---|---|---|---|
| `SUM`/`MIN`/`MAX`/`COUNT` over `NUMERIC` | `SUM(amount)` | `duckdb_native` | `aggregate/aggregate_numeric_sum.yaml` |
| `AVG` over `NUMERIC`/`BIGNUMERIC` | `AVG(amount)` | `semantic_executor` | `aggregate/aggregate_numeric_avg.yaml` |
| `NUMERIC`/`BIGNUMERIC` division | `a / b` | `semantic_executor` | `scalar/numeric_division.yaml` |
| `+`/`-`/`*` over `BIGNUMERIC` | `a + b` | `semantic_executor` | `scalar/bignumeric_arithmetic.yaml` |

Results encode as exact decimal strings on the wire (no float
artifacts); `arrow_to_bq` renders `HUGEINT`-backed and `VARCHAR`-backed
decimals exactly (`backend/engine/duckdb/arrow_to_bq_*.cc`).

## Cross-references

- [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](../backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) — per-node route dispositions (`duckdb_native`, `duckdb_rewrite`, `duckdb_udf`, `semantic_executor`, `control_op`, `local_stub`, `unsupported`).
- [`ROADMAP.md`](../ROADMAP.md) — work tracking and high-level milestone status.
- [`conformance/README.md`](../conformance/README.md) — fixture authoring guide; references this document from its "Contributing a new fixture" section.
- [`DEVELOPMENT.md` "Runtime configuration"](./DEVELOPMENT.md#runtime-configuration) — the user-facing version of the flag surface this document governs.
