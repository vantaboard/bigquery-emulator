# Client libraries

Two equivalent ways to redirect a BigQuery client at the emulator:

1. **Endpoint override** (works in every official client). In Go:

   ```go
   client, err := bigquery.NewClient(ctx, "test-project",
       option.WithEndpoint("http://localhost:9050"),
       option.WithoutAuthentication(),
   )
   ```

2. **`BIGQUERY_EMULATOR_HOST` environment variable** (mirrors the
   `STORAGE_EMULATOR_HOST` and `SPANNER_EMULATOR_HOST` conventions used by
   other Google emulators):

   ```bash
   export BIGQUERY_EMULATOR_HOST=localhost:9050
   ```

Bearer tokens in `Authorization` headers are accepted but never validated,
identical to `cloud-spanner-emulator`'s posture. The full upstream auth model
(ADC, service-account keys, OAuth scopes) documented under
[`docs/bigquery/docs/authentication.md`](./bigquery/docs/authentication.md) is
intentionally **not** modeled.

## SQL dialect

BigQuery's `useLegacySql` field defaults to `true` on the wire (older clients
still rely on this). The emulator only supports GoogleSQL, because the engine is
GoogleSQL's analyzer feeding the local execution coordinator. The query handlers
will:

- Treat `useLegacySql` unset or `false` as GoogleSQL.
- Reject `useLegacySql=true` with HTTP 400 + `reason: invalidQuery`.

If you're using the official Go client, explicitly set `Query.UseLegacySQL =
false` to be safe.

Python (`google-cloud-bigquery`), Java, and Node.js clients all support the
analogous endpoint override. We document each one as the relevant smoke tests
pass under the conformance harness.

## Seeding & CLI compatibility

The gateway accepts both the legacy underscore flag names this repository started
with (`--http_port`) and the hyphen-separated names documented for
`gateway_main` (`--http-port`), so existing scripts keep working and operators
can lift invocation snippets from the upstream docs unchanged. The full alias
table and the seeding workflows live in [`docs/SEEDING.md`](./SEEDING.md):

1. **Declarative YAML seed files** via `--seed-data-file FILE.yaml` (repeatable).
   Loaded after the engine reports SERVING but before the gateway accepts public
   traffic, so any client that hits the REST API sees the seeded
   datasets/tables/rows from request one.
2. **Initial-data template directory** via `--initial-data-dir DIR`. The gateway
   copies the tree into `--data-dir` on first boot (when no `catalog.duckdb` is
   present) and never on subsequent boots, so operator writes are protected.
3. **Production seed REST API** via `--enable-seed-api`. Hits
   `POST /api/emulator/seed` to mirror live BigQuery metadata + rows into the
   local emulator; the polling endpoint is `GET /api/emulator/seed/operations/{id}`.
   Off by default; loopback-only by default; optional shared-secret header for
   defense in depth. The production reader is opt-in via the `seed_production_live`
   build tag so the default gateway build stays free of the cloud BigQuery client
   deps.

## Test lanes

The repository runs two parallel conformance lanes against the same gateway:

1. **Fixture conformance** — `task conformance:*` drives YAML fixtures through
   the in-repo runner and pins SQL semantics against the local execution
   coordinator (the single `local` profile today, which covers every routed
   strategy: DuckDB fast path, DuckDB rewrites, DuckDB UDFs, semantic executor,
   and control ops). See [`docs/ENGINE_POLICY.md`](./ENGINE_POLICY.md) for the
   route catalog and [`conformance/README.md`](../conformance/README.md) for the
   fixture schema, profile matrix, and authoring guide.
2. **Third-party client conformance** — `task thirdparty:*` runs the imported
   BigQuery client-library sample suites (Go, Node.js, Python, BigQuery
   DataFrames) end-to-end against the gateway's REST + gRPC surface and
   (optionally) `fake-gcs-server`. See
   [`third_party/README.md`](../third_party/README.md) for the per-language
   wiring contract, env-var matrix, and skip rules.
