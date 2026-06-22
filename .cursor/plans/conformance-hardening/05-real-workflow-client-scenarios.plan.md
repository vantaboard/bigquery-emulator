# 05 — First-party "real workflow" client-library scenarios

- **Series:** conformance-hardening (plans 01–08). Run sequentially.
- **Sequencing:** fifth. Benefits from plan 02 (sessions) and plan 04 (params)
  but can be authored independently. Reuses the third_party client wiring.
- **Priority:** P2.

## Why this exists (origin)

Every reported bug came through the **real `google-cloud-bigquery` Python
client** doing ordinary multi-tenant ELT — not through hand-built REST or raw
SQL. The failures were in client-shaped flows the repo never exercised end to
end:

- `client.update_dataset(dataset, ["access"])` to authorize a view on a source
  dataset (→ the engine abort).
- `client.query("CREATE OR REPLACE TABLE ... AS SELECT ... ROW_NUMBER() ...")`
  dedup CTAS (→ the DDL single-segment-name error).
- A parameterized dashboard query (→ the naive-TIMESTAMP rejection).
- `client.query(...).result()` for an orphan-orders **anti-join over deduped
  views** (→ the transpiler binding loss).

The repo's `third_party/` lanes run **upstream sample/snippet** suites, which are
happy-path and heavily skipped (`third_party/README.md` skip matrices). They
assert "feature works," not "this realistic flow works." This plan adds a small,
**first-party** scenario suite that mirrors the customer's actual usage so these
flows are exercised on every run.

## Current state (grounded)

- Client wiring already exists and is CI-gated:
  `third_party/python-bigquery-tests/` (nox `snippets`),
  `third_party/golang-bigquery-tests/` (`bqopts`),
  `third_party/java-bigquery-tests/`, `third_party/node-bigquery-tests/`,
  driven by `taskfiles/thirdparty.yml` +
  `.github/workflows/thirdparty-samples.yml`.
- The emulator endpoint contract is documented in `third_party/README.md`
  ("Environment-variable contract"); `BIGQUERY_EMULATOR_HOST` etc.
- What's missing: a **repo-owned** scenario suite (not vendored upstream samples)
  that scripts realistic multi-step client flows.

## Goal / done-criteria

1. A new first-party suite (recommend Python first, since the reports were
   Python) under `third_party/scenarios/` (or `gateway/e2e/scenarios/` if Go is
   preferred for CI speed) that runs against the emulator endpoint.
2. Scenarios mirroring the email thread, each asserting concrete outcomes:
   - **authorize_view**: create source table + rows, create a view in a tenant
     dataset selecting from the source, `update_dataset(["access"])` to add the
     view AccessEntry, then `SELECT` through the view returns the rows; repeat
     across 3 tenants without the engine dying.
   - **dedup_ctas**: `CREATE OR REPLACE TABLE \`ds.t_dedup\` AS SELECT * EXCEPT(rn)
     FROM (... ROW_NUMBER() OVER (PARTITION BY id ORDER BY ts DESC) rn) WHERE
     rn=1`, then read back the deduped rows.
   - **dashboard_params**: a CTE rollup with `COUNTIF`, `JSON_EXTRACT_SCALAR`,
     `TIMESTAMP_SUB`, `UNION DISTINCT`, and a naive TIMESTAMP param; assert the
     scalar results.
   - **orphan_orders**: LEFT JOIN orders→profiles with `WHERE p.id IS NULL` over
     QUALIFY-deduped views; assert the orphan rows.
3. CI job (gated after `build-engine`, like the other thirdparty lanes) that runs
   the scenario suite; failures block.
4. Each scenario's expected values are production-validated (plan 01 oracle or
   `bq`), per `.cursor/rules/conformance-bq-validation.mdc`.

## Implementation steps

### Step 1 — Scenario harness
- `third_party/scenarios/python/` with a `conftest.py` reusing the emulator
  wiring pattern from `third_party/python-bigquery-tests` (AnonymousCredentials,
  `BIGQUERY_EMULATOR_HOST` → `http://host:port`). One module per scenario.
- Each scenario: set up datasets/tables/rows via the client, run the flow,
  assert rows/scalars (and that no exception/abort occurred).

### Step 2 — Author the four scenarios above
Use the actual SQL shapes from the email thread (obfuscated table/column names
are fine). Keep each scenario one realistic flow; assert post-state explicitly.

### Step 3 — Task + CI
- `task thirdparty:scenarios` in `taskfiles/thirdparty.yml`, added to the
  `thirdparty` aggregator.
- A `scenarios-live` job in `.github/workflows/thirdparty-samples.yml` using the
  shared `setup-thirdparty-docker-emulator` action.
- Document in `third_party/README.md` (new "First-party scenarios" lane row).

### Step 4 — (Optional) mirror critical scenarios in Go
For CI speed, port `authorize_view` and `orphan_orders` to
`third_party/golang-bigquery-tests` or `gateway/e2e` so a fast Go gate runs even
when the Python lane is skipped.

## Tests
- The four scenarios themselves are the tests; they must pass green (some may be
  red until plans 03/06 land the underlying engine/transpiler fixes — mark those
  `xfail` with a pointer to the owning plan, don't delete them).

## Process hygiene (repo rules)
Brings up the gateway + engine (`task emulator:run-full`) and possibly fake-gcs.
Follow `.cursor/rules/process-hygiene.mdc`: audit before bring-up, and run the
emulator/gateway kill + `docker compose down` cleanup block after the lane.

## Out of scope
- Full client API surface coverage (that's the upstream sample lanes' job).
- BQML/Storage gRPC/DataTransfer flows (tracked elsewhere; see
  `third_party/README.md` skip matrices).

## Touch list
`third_party/scenarios/python/*` (+ `conftest.py`), optional
`third_party/golang-bigquery-tests/` or `gateway/e2e/scenarios/*`,
`taskfiles/thirdparty.yml`, `.github/workflows/thirdparty-samples.yml`,
`third_party/README.md`.
