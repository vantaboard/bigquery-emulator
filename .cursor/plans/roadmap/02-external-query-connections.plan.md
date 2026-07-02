---
name: EXTERNAL_QUERY and connections
overview: Land fixture-backed EXTERNAL_QUERY as a TVF in the analyzer catalog, deepen bqconnection CRUD against the source-config model, and pin the posture for BigLake / object tables / Spanner external datasets.
todos:
  - id: fixture-contract
    content: "Define the $data_dir/external/connections/ fixture contract: per-connection result files (schema + rows) keyed by query hash or alias"
    status: completed
  - id: analyzer-tvf
    content: Register EXTERNAL_QUERY as a TVF in the per-query analyzer catalog so analysis succeeds and routes to the semantic executor
    status: completed
  - id: engine-eval
    content: Evaluate EXTERNAL_QUERY on the semantic executor by materializing the fixture result as a row source
    status: completed
  - id: connection-crud
    content: Deepen bqconnection CRUD (update/delete, properties round-trip) against sourceconfig, persisted under $data_dir
    status: completed
  - id: posture-rows
    content: Pin BigLake / object tables / Spanner external datasets as explicit unsupported-with-envelope rows in ENGINE_POLICY
    status: completed
isProject: false
---

# 02 — `EXTERNAL_QUERY` + cloud-resource connections (fixture-backed real)

- **Roadmap row:** §External data sources — ⏳ "Cloud-resource connections and
  federated / external-dataset query paths … bqconnection CRUD wired to config
  model; `EXTERNAL_QUERY` engine stub still pending"
- **Local-only posture:** per ROADMAP §Non-goals, query execution never
  proxies through real cloud services. `EXTERNAL_QUERY` therefore lands as
  **fixture mode** (snapshot results from `$data_dir`), matching the Sheets
  precedent (`fixture | local | live` resolution in
  `gateway/external/sourceconfig/`). Live Cloud SQL / Spanner federation is
  out of scope.

## Current state at HEAD (grounded)

- `gateway/handlers/bqconnection/` implements shallow
  `google.cloud.bigquery.connection.v1.ConnectionService` list/create/get
  stubs on the gRPC shim (`gateway/grpcserver/register.go`); enough for Java
  client startup probes (ENGINE_POLICY ~285).
- `Config.ConnectionFixtureRoot()`
  (`gateway/external/sourceconfig/config.go` ~220) already reserves
  `$data_dir/external/connections/` "fixture SQL result files for
  EXTERNAL_QUERY", and `connectionID()` (~237) parses the
  `region.connection_id` argument form — the config model anticipated this
  plan; nothing consumes the root yet.
- There is no engine-side `EXTERNAL_QUERY` entry: not in
  `functions.yaml`, no TVF registration, so analysis fails with "function not
  found" rather than a policy-shaped envelope.

## Done-criteria

1. `SELECT * FROM EXTERNAL_QUERY('us.my_conn', 'SELECT ...')` analyzes and
   returns the fixture snapshot rows for that connection + query.
2. A missing fixture surfaces a BigQuery-shaped error naming the connection
   and the fixture path to add (developer-actionable, not a bare 500).
3. Connection CRUD (create/get/list/delete) round-trips and persists across
   gateway restarts under `$data_dir`.
4. BigLake tables, object tables, and Spanner external datasets surface
   explicit `UNIMPLEMENTED` envelopes linking to ENGINE_POLICY (not analyzer
   "not found" noise).

## Implementation steps

### Step 1 — fixture contract

Define the on-disk shape under `$data_dir/external/connections/<conn_id>/`:
a `queries.yaml` manifest mapping either an exact query string or a named
alias to a result file (`schema` + `rows`, same typed-cell format the
conformance runner uses). Document in `docs/ENGINE_POLICY.md` and a new
`docs/guides/external-query.md`.

### Step 2 — analyzer TVF registration

`EXTERNAL_QUERY(connection STRING, query STRING)` returns a table whose
schema is not statically known — resolve the output schema at analysis time
by looking up the fixture manifest (gateway passes the connection fixture
root to the engine, or the engine reads `$data_dir` directly since it already
owns it). Register the TVF in the per-query catalog next to the existing SQL
TVF machinery; route the resulting `ResolvedTVFScan` to the semantic
executor.

### Step 3 — semantic evaluation

Materialize the fixture rows as a `RowSource` (reuse the TVF relation
materialization path — `ResolvedRelationArgumentScan` handling already
exists). Missing manifest/entry → `NOT_FOUND` with the expected path in the
message.

### Step 4 — connection CRUD depth

Extend `gateway/handlers/bqconnection/` with update/delete and property
round-trip (`cloudSql`, `spanner` property blocks stored as opaque config),
persisting through `sourceconfig` so restarts keep connections. Keep it
metadata-only — no live credentials handling.

### Step 5 — posture rows

Add explicit `unsupported` rows + sharp envelopes for BigLake
(`biglake_configuration`), object tables, and external (Spanner/Cloud SQL)
datasets in `docs/ENGINE_POLICY.md`, and matching ROADMAP wording, so the
remaining federated surface is deliberately scoped rather than silent.

## Tests

- Conformance: `conformance/fixtures/external/external_query_fixture.yaml`
  (seed fixture manifest via session setup, query, diff rows) +
  `external_query_missing_fixture.yaml` (error envelope).
- Go: `gateway/handlers/bqconnection/` CRUD round-trip + restart persistence.
- Java thirdparty lane: revisit the Connection IT allowlist
  (`third_party/README.md`) once CRUD deepens.

## Out of scope

- Live federation to Cloud SQL / Spanner / AlloyDB (never planned).
- `EXTERNAL_QUERY` result-schema inference without a fixture manifest.
- BigLake / object-table data access (posture rows only).

## Touch list

`gateway/handlers/bqconnection/`, `gateway/external/sourceconfig/`,
`backend/engine/semantic/` (TVF eval), per-query catalog registration,
`functions.yaml` / `node_dispositions.yaml`, `conformance/fixtures/external/`,
`docs/ENGINE_POLICY.md`, `docs/guides/`, `ROADMAP.md`.
