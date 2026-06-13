# Languages & tools

Two equivalent ways to redirect a BigQuery client at the emulator:

1. **Endpoint override** — works in every official client library.
2. **`BIGQUERY_EMULATOR_HOST`** — mirrors the `STORAGE_EMULATOR_HOST` and
   `SPANNER_EMULATOR_HOST` conventions used by other Google emulators.

Bearer tokens in `Authorization` headers are accepted but never validated.
The full upstream auth model (ADC, service-account keys, OAuth scopes) is
intentionally **not** modeled.

## Environment variable

=== "Shell"

    ```bash
    export BIGQUERY_EMULATOR_HOST=localhost:9050
    ```

    Most official client libraries read this variable and redirect API
    calls to the emulator without code changes.

=== "Docker Compose"

    ```yaml
    services:
      app:
        environment:
          BIGQUERY_EMULATOR_HOST: bigquery-emulator:9050
    ```

=== "CI / test runner"

    ```bash
    BIGQUERY_EMULATOR_HOST=localhost:9050 task thirdparty:python-bigquery-tests
    ```

## Per-language setup

=== "Go"

    ```go
    import (
        "cloud.google.com/go/bigquery"
        "google.golang.org/api/option"
    )

    client, err := bigquery.NewClient(ctx, "test-project",
        option.WithEndpoint("http://localhost:9050"),
        option.WithoutAuthentication(),
    )
    if err != nil {
        log.Fatal(err)
    }
    defer client.Close()

    q := client.Query("SELECT 1 AS n")
    q.UseLegacySQL = false
    it, err := q.Read(ctx)
    ```

    **Test lane:** `task thirdparty:golang-bigquery-tests`

=== "Python"

    ```python
    from google.auth.credentials import AnonymousCredentials
    from google.cloud import bigquery

    client = bigquery.Client(
        project="test-project",
        client_options={"api_endpoint": "http://localhost:9050"},
        credentials=AnonymousCredentials(),
    )

    rows = client.query("SELECT 1 AS n").result()
    for row in rows:
        print(row.n)
    ```

    **Test lanes:**

    - `task thirdparty:python-bigquery-tests`
    - `task thirdparty:python-bigquery-dataframes-tests`
    - `task thirdparty:dbt-bigquery-tests`

=== "Java"

    ```java
    import com.google.auth.oauth2.GoogleCredentials;
    import com.google.cloud.NoCredentials;
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryOptions;
    import com.google.cloud.bigquery.QueryJobConfiguration;

    BigQuery bigquery = BigQueryOptions.newBuilder()
        .setProjectId("test-project")
        .setHost("http://localhost:9050")
        .setCredentials(NoCredentials.getInstance())
        .build()
        .getService();

    QueryJobConfiguration config = QueryJobConfiguration
        .newBuilder("SELECT 1 AS n")
        .setUseLegacySql(false)
        .build();
    ```

    **Test lane:** `task thirdparty:java-bigquery-tests`

=== "Node.js"

    ```javascript
    const {BigQuery} = require('@google-cloud/bigquery');

    const bigquery = new BigQuery({
      projectId: 'test-project',
      apiEndpoint: 'http://localhost:9050',
    });

    const [rows] = await bigquery.query({
      query: 'SELECT 1 AS n',
      useLegacySql: false,
    });
    ```

    **Test lane:** `task thirdparty:node-bigquery-tests`

=== "bq CLI"

    ```bash
    # Synchronous query
    bq --api http://localhost:9050 query --use_legacy_sql=false 'SELECT 1'

    # List datasets
    bq --api http://localhost:9050 ls --project_id=test-project
    ```

    The gateway registers routes under both `/bigquery/v2/...` and bare
    paths so `gcloud` / `bq` tools work without extra configuration.

## SQL dialect

BigQuery's `useLegacySql` field defaults to `true` on the wire. The
emulator only supports **GoogleSQL** — the engine is GoogleSQL's analyzer
feeding the local execution coordinator.

- Treat `useLegacySql` unset or `false` as GoogleSQL.
- Reject `useLegacySql=true` (except a narrow bracket-table-ref
  translation for third-party samples) with HTTP 400 + `reason: invalidQuery`.

Always set `useLegacySql=false` / `use_legacy_sql=false` in new code.

## Storage Read API (gRPC)

Arrow/Avro bulk reads use the internal gRPC Storage Read API on the
engine port (`:9060` by default), not the REST gateway. Set:

```bash
export BIGQUERY_STORAGE_GRPC_ENDPOINT=localhost:9060
```

See [REST API — Storage Read API](../REST_API.md#storage-read-api) for
transport details.

## Seeding before clients connect

Load fixture data at gateway startup so the first client request sees
populated tables:

```bash
task emulator:run-seeded SEED_FILE=testdata/fixtures/demo.yaml
```

Or pass `--seed-data-file` directly. See [Seeding data](../SEEDING.md).

## Related guides

- [Client libraries](../CLIENTS.md) — conformance lanes and CLI flag aliases
- [Development setup](../DEVELOPMENT.md) — building and running locally
- [Third-party harnesses](https://github.com/vantaboard/bigquery-emulator/blob/main/third_party/README.md) —
  per-language wiring contract and skip rules
