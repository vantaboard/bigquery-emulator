# Seeding the BigQuery emulator

The emulator has three complementary ways to pre-populate the engine
catalog at startup. Pick the one that matches your workflow:

| Mechanism                                                                      | Best for                                                                 | Persistence                                                        |
|--------------------------------------------------------------------------------|--------------------------------------------------------------------------|--------------------------------------------------------------------|
| [Declarative YAML seed files](#1-declarative-yaml-seed-files-recommended)      | Local dev fixtures, CI test data, repeatable demos                       | Re-applied at every gateway start                                  |
| [Initial-data template directory](#2-initial-data-template-directory)          | Shipping a pre-built DuckDB catalog with a container image               | Materialized once into `--data-dir`; subsequent boots are no-ops   |
| [Production seed REST API](#3-production-seed-rest-api-opt-in-build)           | One-off "mirror this real BigQuery project into my laptop" sessions      | Persists in the DuckDB catalog under `--data-dir`                  |

All three end up writing through the same engine `CatalogClient` /
`InsertRows` surface the REST handlers use, so seeded data is
indistinguishable from data created via `bigquery.datasets.insert`,
`bigquery.tables.insert`, and `bigquery.tabledata.insertAll`.

---

## 1. Declarative YAML seed files (recommended)

Pass one or more `--seed-data-file FILE.yaml` flags (also accepted as
`--seed-yaml`) and the gateway loads them after the engine reports
`SERVING` but **before** it starts accepting public HTTP traffic. Files
are applied in the order they appear on the command line, datasets
within a file are applied in declaration order, and rows are inserted
in declaration order.

### Schema

```yaml
project_id: dev            # default project for datasets that omit project_id
location: US               # default location for datasets that omit location
datasets:
  - id: ds                 # required
    project_id: dev        # optional override of the file-level default
    location: US           # optional override
    tables:
      - id: people         # required
        schema:
          - {name: id,   type: INT64,  mode: REQUIRED}
          - {name: name, type: STRING}
          - {name: addr, type: STRUCT, fields: [
              {name: city, type: STRING},
              {name: zip,  type: STRING},
            ]}
        rows:
          - {id: 1, name: ada, addr: {city: London, zip: NW1}}
          - {id: 2, name: bob, addr: {city: Paris,  zip: 75001}}
```

- `project_id` precedence: dataset entry > file `project_id` >
  `--project-id` gateway flag. If all three are empty, gateway startup
  fails with a clear error pointing at the offending dataset.
- `location` precedence: dataset entry > file `location` >
  `--default-dataset-location`. Empty is permitted; the engine will
  accept the dataset without a location.
- Schema is required for tables that include `rows`. Tables with no
  rows are allowed (useful when you want the catalog entry to exist
  but data lands later via REST).
- Rows are key/value maps. Missing keys become `NULL` cells; extra
  keys not in the schema are silently dropped, matching the
  `tabledata.insertAll` REST handler.

### Quickstart

```bash
cat > /tmp/seed.yaml <<'YAML'
project_id: dev
datasets:
  - id: ds
    tables:
      - id: people
        schema:
          - {name: id,   type: INT64}
          - {name: name, type: STRING}
        rows:
          - {id: 1, name: ada}
          - {id: 2, name: bob}
YAML

task emulator:build-all
./bin/gateway_main \
    --engine_binary=./bin/emulator_main \
    --seed-data-file=/tmp/seed.yaml
```

A subsequent `curl -s http://localhost:9050/bigquery/v2/projects/dev/datasets/ds/tables/people/data | jq .`
returns the seeded rows.

### Repeatable across boots

YAML seeds are re-applied every time the gateway boots. The applier
treats `ALREADY_EXISTS` from the engine as a successful no-op for
datasets and tables, so reruns succeed against the persistent
DuckDB-backed catalog. Rows are appended on every boot; if you
need deterministic per-boot state, point `--data-dir` at a fresh
temp directory each run (e.g. `--data-dir "$(mktemp -d)"`) so the
catalog starts empty.

---

## 2. Initial-data template directory

`--initial-data-dir DIR` materializes a pre-built catalog tree into
`--data-dir` on first boot. The gateway treats the destination as
"initialized" when `catalog.duckdb` is present in `--data-dir`; if
it is, the template is left alone so operator-written data is
never clobbered.

Use this when you ship a container image that should boot with a
fully-populated catalog (e.g. a tutorial image with a worked
example), or when you've snapshotted a long-lived dev catalog and
want to restore it across machines.

```bash
# One-time: prepare a template you can hand around.
mkdir -p /tmp/seed_template
docker run --rm -v /tmp/seed_template:/data ghcr.io/vantaboard/bigquery-emulator:vX.Y.Z \
    --data-dir=/data
# ... use the live emulator to register datasets/tables/rows ...
# When you're done the on-disk catalog under /tmp/seed_template
# can be tarballed and shipped.

# Later: ship that template into a fresh data-dir.
./bin/gateway_main \
    --engine_binary=./bin/emulator_main \
    --initial-data-dir=/tmp/seed_template \
    --data-dir=$HOME/.bigquery-emulator
```

Environment-variable fallbacks for the template path:

- `BIGQUERY_EMULATOR_INITIAL_DATA_DIR`
- `EMULATOR_INITIAL_DATA_DIR` (legacy, accepted for compatibility)

---

## 3. Production seed REST API (opt-in build)

The gateway exposes a `POST /api/emulator/seed` endpoint that, when
the build includes a production client, copies live BigQuery
metadata + rows into the local emulator. The request and response
shape is documented below so existing seed tooling can reuse it.

### Posture

The seed API is **off** by default. Enable it explicitly with
`--enable-seed-api`. The default safe profile:

- Only loopback (`127.0.0.0/8`, `::1`) callers are accepted.
- An optional shared-secret header `X-BigQuery-Emulator-Seed-Token`
  can be required via `--seed-api-seed-token` (or the
  `BIGQUERY_EMULATOR_SEED_TOKEN` env var).
- For CI runners reaching the gateway across a LAN,
  `--seed-api-allow-remote` removes the loopback gate; combine with
  `--seed-api-seed-token` for defense in depth.

### Build matrix

The default `go build ./binaries/gateway_main` ships **without** the
production reader (it would pull `cloud.google.com/go/bigquery`
into the gateway, which most users don't need). When the runner is
missing, the POST handler returns `501 notImplemented` with a
message pointing operators at `--seed-data-file`.

Building with `-tags=seed_production_live` adds the production
reader (planned; see the orchestrator in `gateway/seed/orchestrator.go`).

### Request shape

```json
{
  "source": { "project": "my-prod-project", "dataset": "events" },
  "destination": { "dataset": "events_local" },
  "maxRowsPerTable": 1000,
  "billingProject": "my-billing-project"
}
```

Source scopes (pick one):

- `{ "project": "p" }` — every dataset in `p`.
- `{ "project": "p", "dataset": "d" }` — every table in `p.d`.
- `{ "project": "p", "dataset": "d", "table": "t" }` — one table.

Destination is optional; omit it to mirror the source ids 1:1.
When set, only the supplied fields override the corresponding source
field. Mixing a project-scope source with a dataset-scope destination
is rejected at validation time.

### Polling

`POST /api/emulator/seed` returns `202 Accepted` with an operation
envelope:

```json
{
  "id": "op-2a3b4c5d6e7f8091",
  "state": "PENDING",
  "started": "2026-05-27T18:00:00Z",
  "request": { ... }
}
```

Poll the operation with `GET /api/emulator/seed/operations/{id}`.
The response shape stays the same; `state` moves
`PENDING → RUNNING → DONE | FAILED`, and the final `result`
breakdown follows the stable seed API contract:

```json
{
  "id": "op-2a3b4c5d6e7f8091",
  "state": "DONE",
  "result": {
    "started": "2026-05-27T18:00:00Z",
    "finished": "2026-05-27T18:00:43Z",
    "datasetsCreated": 1,
    "datasetsSkipped": 0,
    "tablesCreated": 12,
    "tablesSkipped": 0,
    "rowsCopied": 9871,
    "resourceErrors": [
      { "resource": "table:my-prod-project.events.daily_v",
        "kind": "unsupported",
        "error": "type \"VIEW\" is not yet supported by the BigQuery emulator; only physical TABLE entries are seeded" }
    ]
  }
}
```

### Billing-project resolution

The `billingProject` field of the request is the first preference.
If it is empty, the seed orchestrator walks:

1. `--project-id` (gateway flag).
2. `GOOGLE_CLOUD_QUOTA_PROJECT`.
3. `GOOGLE_CLOUD_PROJECT`.
4. `GCLOUD_PROJECT`.
5. The source project itself.

This mirrors the chain `gcloud`'s tooling uses, so the same ADC
setup that lets you `bq ls` locally lets you seed without extra
configuration.

### Supported resource types

Production seeding lands physical tables only in the initial
integration. Views, materialized views, models, external tables,
and routines surface in `result.resourceErrors` with
`kind="unsupported"`, and the rest of the seed proceeds. Expanding
coverage is gated on the engine's storage / proto adding
persistence for those resource types.

---

## CLI flag compatibility

Every operator-facing gateway flag accepts both the legacy
underscore form (`--http_port`) and the hyphen form
(`--http-port`) documented for `gateway_main`. The
canonical reference:

| Flag                           | Aliases                       | Purpose                                                                          |
|--------------------------------|-------------------------------|----------------------------------------------------------------------------------|
| `--listen-host`                | `--hostname`                  | Hostname the REST gateway binds to (default `localhost`).                        |
| `--http-port`                  | `--http_port`                 | BigQuery REST listener port (default `9050`).                                    |
| `--grpc-port`                  | `--grpc_port`                 | Internal engine gRPC port (default `9060`).                                      |
| `--engine-binary`              | `--engine_binary`             | Path to the C++ engine subprocess; empty disables it.                            |
| `--data-dir`                   | `--data_dir`                  | Persistent storage root. Falls back to `$BIGQUERY_EMULATOR_DATA_DIR`.            |
| `--initial-data-dir`           |                               | Template copied into `--data-dir` on first boot (see §2).                        |
| `--copy-engine-stdout`         | `--copy_engine_stdout`        | Forward engine stdout.                                                           |
| `--copy-engine-stderr`         | `--copy_engine_stderr`        | Forward engine stderr (default `true`).                                          |
| `--log-requests`               | `--log_requests`              | Log every REST request and response.                                             |
| `--debug`                      |                               | Verbose lifecycle logging.                                                       |
| `--project-id`                 | `--project_id`                | Default project for seeding and YAML loader fallbacks.                           |
| `--default-dataset-location`   |                               | Default location stamped on datasets without an explicit one.                    |
| `--enable-seed-api`            |                               | Register the production seed REST endpoints (see §3).                            |
| `--seed-api-allow-remote`      |                               | Allow non-loopback callers to hit the seed API.                                  |
| `--seed-api-seed-token`        |                               | Required value of `X-BigQuery-Emulator-Seed-Token` on every seed request.        |
| `--seed-data-file`             | `--seed-yaml`                 | YAML seed file (repeatable) (see §1).                                            |
| `--version`                    |                               | Print version metadata and exit.                                                 |

Environment-variable fallbacks (consulted only when the matching
flag is empty):

| Env var                                | Maps to                          |
|----------------------------------------|----------------------------------|
| `BIGQUERY_EMULATOR_DATA_DIR`           | `--data-dir`                     |
| `BIGQUERY_EMULATOR_INITIAL_DATA_DIR`   | `--initial-data-dir`             |
| `EMULATOR_INITIAL_DATA_DIR`            | `--initial-data-dir` (legacy)    |
| `BIGQUERY_EMULATOR_SEED_TOKEN`         | `--seed-api-seed-token`          |
