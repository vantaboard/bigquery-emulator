# dbt-bigquery emulator feasibility (plan 10, phase 1)

Verified **2026-06-06** against the docker-compose gateway on `http://localhost:9050`.

## Client-level `api_endpoint` probe

`google-cloud-bigquery.Client` accepts `ClientOptions(api_endpoint=...)` plus
`AnonymousCredentials()` and successfully exercised:

| Surface | Result |
|---------|--------|
| `list_datasets` | OK |
| `create_dataset` | OK |
| `query` job + `job.result()` | OK |
| `delete_dataset` | OK |

Command shape (via `uvx --with 'google-cloud-bigquery>=3.0'`):

```python
from google.api_core.client_options import ClientOptions
from google.auth.credentials import AnonymousCredentials
from google.cloud.bigquery import Client

client = Client(
    "dev",
    AnonymousCredentials(),
    client_options=ClientOptions(api_endpoint="http://localhost:9050"),
)
```

## dbt-bigquery adapter facts

- `api_endpoint` is a first-class credential field (`credentials.py`) and is
  passed through to `BigQueryClient` (`clients.py`).
- Upstream functional `tests/conftest.py` only defines `oauth` and
  `service_account` profiles — no emulator target today.
- dbt-bigquery always builds real `google.auth` credentials (`create_google_credentials`);
  emulator wiring requires patching to `AnonymousCredentials` when
  `BIGQUERY_EMULATOR_HOST` is set (see `emulator_bootstrap.py`).

## Expected missing surfaces (first triage wave)

Functional tests subclass shared bases from the installed `dbt-tests-adapter`
package and hit BigQuery-specific paths beyond bare REST jobs:

- **Dataproc / python models** — `compute_region`, `gcs_bucket`, batch submission
- **GCS upload_file** materializations — needs `STORAGE_EMULATOR_HOST` + fake-gcs
- **INFORMATION_SCHEMA / catalogs** — `docs generate`, column metadata, grants
- **Materialized views** — `simple_bigquery_view`, changing relation types
- **Policy tags / row access** — column policy tests
- **Wildcards / external tables** — public GCS URIs

These are sketched in `emulator_pytest_skip.py`; refine during triage.

## dbt functional test probe (2026-06-06)

`DBT_BIGQUERY_RUN_TRIAGE=1` + `TestSimpleMaterializationsBigQuery::test_base`:

| Stage | Result |
|-------|--------|
| uvx install dbt-core + dbt-bigquery @ pinned SHA | OK |
| Emulator profile + anonymous creds patch | OK (after fixing duplicate `project`/`database` and `dataset`/`schema` aliases) |
| dbt adapter connects via `api_endpoint` | OK |
| First SQL (dataset/schema DDL during setup) | **FAIL** — engine `Unimplemented`: `ExecuteDdl only handles ALTER TABLE today; got CreateSchemaStmt` |

First-wave engine gap: dbt issues `CREATE SCHEMA` (dataset) through the query
job path; the DuckDB engine routes it to `ExecuteDdl` instead of
`ControlOpExecutor`. Until dataset DDL via jobs works, `BaseSimpleMaterializations`
cannot progress past setup.

## Next step

```bash
./scripts/sync_dbt_bigquery_tests.sh
DBT_BIGQUERY_RUN_TRIAGE=1 \
  DBT_BIGQUERY_PYTEST_ARGS='tests/functional/adapter/test_basic.py::TestSimpleMaterializationsBigQuery::test_base -n0' \
  task thirdparty:dbt-bigquery-tests
```
