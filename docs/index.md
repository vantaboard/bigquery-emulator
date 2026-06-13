# BigQuery Emulator

A locally-runnable emulator of Google Cloud BigQuery for development and
integration testing. The Go REST gateway implements the BigQuery v2 API;
the C++ engine links [GoogleSQL](https://github.com/google/googlesql)
directly and routes each query shape to DuckDB, semantic evaluation, or
catalog operations.

> **Status:** preview (`v0.x`). See the
> [capability roadmap](https://github.com/vantaboard/bigquery-emulator/blob/main/ROADMAP.md)
> for milestone tracking.

## Install

=== "Docker (recommended)"

    The fastest path — no Bazel build, no GoogleSQL toolchain:

    ```bash
    docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:latest

    curl -fsS http://localhost:9050/healthz
    ```

    See [Docker](DOCKER.md) for `docker compose`, volume mounts, and build notes.

=== "Release archive"

    Download a pre-built gateway + engine tarball from
    [GitHub Releases](https://github.com/vantaboard/bigquery-emulator/releases):

    ```bash
    curl -fL https://github.com/vantaboard/bigquery-emulator/releases/download/v0.0.1/bigquery-emulator_0.0.1_linux_amd64.tar.gz \
        | tar xz
    ./bigquery-emulator-gateway --help
    ```

    See [Releases & install](RELEASES.md) for GHCR image tags and versioning.

=== "Build from source"

    ```bash
    mise install                              # or: task tools:install
    task emulator:build-engine:bazel          # stages bin/emulator_main
    task emulator:run-full                    # gateway :9050, engine :9060
    ```

    See [Development setup](DEVELOPMENT.md) for the full toolchain and repo layout.

## Connect a client

Point any official BigQuery client at `http://localhost:9050` (or set
`BIGQUERY_EMULATOR_HOST=localhost:9050`). Authentication is accepted but
never validated — identical to `cloud-spanner-emulator`.

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
    ```

=== "Python"

    ```python
    from google.auth.credentials import AnonymousCredentials
    from google.cloud import bigquery

    client = bigquery.Client(
        project="test-project",
        client_options={"api_endpoint": "http://localhost:9050"},
        credentials=AnonymousCredentials(),
    )
    ```

=== "Java"

    ```java
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryOptions;

    BigQuery bigquery = BigQueryOptions.newBuilder()
        .setProjectId("test-project")
        .setHost("http://localhost:9050")
        .setCredentials(NoCredentials.getInstance())
        .build()
        .getService();
    ```

=== "Node.js"

    ```javascript
    const {BigQuery} = require('@google-cloud/bigquery');

    const bigquery = new BigQuery({
      projectId: 'test-project',
      apiEndpoint: 'http://localhost:9050',
    });
    ```

=== "bq CLI"

    ```bash
    bq --api http://localhost:9050 query --use_legacy_sql=false 'SELECT 1'
    ```

    The gateway also mounts routes under bare paths (without the
    `/bigquery/v2` prefix) for `gcloud` / `bq` compatibility.

See [Languages & tools](guides/multi-language.md) for environment variables,
SQL dialect notes, and per-language test lanes.

## Run your first query

```bash
curl -fsS -X POST http://localhost:9050/bigquery/v2/projects/test/queries \
    -H 'Content-Type: application/json' \
    -d '{"query":"SELECT 1 AS n","useLegacySql":false}'
```

## Next steps

| Topic | Guide |
|-------|-------|
| Seed datasets at startup | [Seeding data](SEEDING.md) |
| REST endpoint coverage | [REST API](REST_API.md) |
| SQL execution routes | [Engine policy](ENGINE_POLICY.md) |
| Feature & compatibility overview | [Feature support](features/support.md) |
| Client library wiring | [Client libraries](CLIENTS.md) |
