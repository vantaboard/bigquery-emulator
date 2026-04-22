# BigQuery Emulator

The BigQuery emulator provides a way to launch a BigQuery server on your local machine for testing and development.

The Vantaboard fork has many features, performance improvements, and bugfixes that are missing from the upstream repository.

# Features

- If you use the Go BigQuery client, you can launch the emulator within the testing process by [httptest](https://pkg.go.dev/net/http/httptest) .
- BigQuery emulator can be launched as a standalone process. So, you can use the BigQuery emulator from programs written in non-Go languages or the [bq](https://cloud.google.com/bigquery/docs/bq-command-line-tool) command, by specifying the address of the launched BigQuery emulator.
- BigQuery emulator utilizes SQLite for storage. You can select either memory or file as the data storage destination at startup, and if you set it to file, data can be persisted.
- You can load seed data from a JSON or YAML file on startup

## BigQuery API

We've implemented all the [BigQuery APIs](https://cloud.google.com/bigquery/docs/reference/rest) except the API to manipulate IAM resources. It is possible that some options are not supported, in which case please report them in an Issue.

## Google Cloud Storage

BigQuery emulator supports loading data from Google Cloud Storage and exporting table data. Currently, only CSV and JSON data types can be used for export.
If you use Google Cloud Storage emulator, please set `STORAGE_EMULATOR_HOST` environment variable.

## BigQuery Storage API

Supports gRPC-based read/write using [BigQuery Storage API](https://cloud.google.com/bigquery/docs/reference/storage).
Supports both Apache `Avro` and `Arrow` formats.

## Google Standard SQL

BigQuery emulator supports many of the specifications present in [Google Standard SQL](https://cloud.google.com/bigquery/docs/reference/standard-sql/introduction).
For example, it has the following features.

- 200+ standard functions
- Wildcard table
- Templated Argument Function
- JavaScript UDF

If you want to know which specific features are supported, please see [here](https://github.com/vantaboard/go-googlesqlite#status)

## DuckDB execution backend (optional)

By default the emulator uses **`googlesqlite`** (SQLite). You can select **`googlesqlduck`** (DuckDB via [go-googlesqlite](https://github.com/vantaboard/go-googlesqlite)) with:

- **CLI:** `--execution-backend=duckdb`
- **Env:** `BQ_EMULATOR_EXECUTION_BACKEND=duckdb` (also bound to the flag)

The DuckDB driver is only registered when the binary is built with **`-tags duckdb`** (and usually **`duckdb_use_lib`** plus a pinned **`libduckdb`**; see [go-googlesqlite `docs/duckdb-parity-gates.md`](https://github.com/vantaboard/go-googlesqlite/blob/main/docs/duckdb-parity-gates.md)). Use **`task emulator:build-duck`** and **`task test:duckdb-backend`** in this repo after setting **`DUCKDB_LIB_DIR`**.

**Running `bigquery-emulator-duck`:** `task emulator:build-duck` copies **`libduckdb.so`** (Linux) or **`libduckdb.dylib`** (macOS) into the same directory as the binary and links with a runpath (`$ORIGIN` / `@executable_path`), so **`./bigquery-emulator-duck`** usually works without **`LD_LIBRARY_PATH`**. If you built the binary yourself with plain `go build`, set **`LD_LIBRARY_PATH`** (Linux) or **`DYLD_LIBRARY_PATH`** (macOS) to the directory that contains the DuckDB shared library, or keep that `.so` / `.dylib` next to the executable.

SQLite-specific URI pragmas ([`storageWithSQLiteDefaults`](server/server.go)) are **not** applied for DuckDB. With `--execution-backend=duckdb`, **temporary database files** are created by deleting a reserved path first so DuckDB can initialize a valid file (unlike SQLite empty `CreateTemp` files). TEMP tables, MERGE scratch tables, and pooling interact differently with DuckDB; see [go-googlesqlite `docs/duckdb-phase3-phase4-followon.md`](https://github.com/vantaboard/go-googlesqlite/blob/main/docs/duckdb-phase3-phase4-followon.md). Raw metadata SQL uses the same go-googlesqlite pipeline as queries (including `@` named parameters rewritten for DuckDB when formatting is disabled).

# Sponsor 

If this project is of useful to you or your team, consider sponsoring the original creator [@goccy](https://github.com/goccy)

## Explorer HTTP API (`/api` for bigquery-emulator-ui)

The [bigquery-emulator-ui](https://github.com/filipecaixeta/bigquery-emulator-ui) React app calls JSON endpoints under **`/api/*`** on the **same HTTP port** as the BigQuery REST API (the **`--port`** listener, default **9050**). They are implemented in **`internal/explorerapi`** and registered by the main **`bigquery-emulator`** binary—no separate sidecar process.

If **`BIGQUERY_EMULATOR_HOST`** is not set, the emulator sets it to the REST listen address on loopback (e.g. `127.0.0.1:9050`) so the embedded explorer client can reach this server via the Go BigQuery client. Point **`BIGQUERY_EMULATOR_HOST`** at another host if you want the explorer UI to use a different BigQuery endpoint.

See the UI repository’s `docs/api-contract.md` for route shapes.

# Installation

**Prebuilt-first installs:** this emulator depends on [`go-googlesql`](https://github.com/vantaboard/go-googlesql), whose supported default path is **`googlesql` + `googlesql_unified_prebuilt`** with release prebuilts and the shared stack bootstrap env. For local source builds, prefer sibling checkouts of `bigquery-emulator`, `go-googlesql`, and `go-googlesqlite`, then follow the **Development build modes** below instead of a blind `go install`.

You can download the Docker image with:

```console
$ docker pull ghcr.io/vantaboard/bigquery-emulator:latest
```

You can also download release binaries directly from [releases](https://github.com/vantaboard/bigquery-emulator/releases).

For local source builds, set up sibling checkouts plus the shared bootstrap env and build with:

```console
$ direnv allow
$ task emulator:build
```

## Development build modes

**Default: unified prebuilt stack:** follow the same default upstream tag set as **`go-googlesql`** (**`googlesql,googlesql_unified_prebuilt,googlesql_prebuilts_mod,googlesql_prebuilts_platform_pkg`**). Native archives, release tarball **`go-googlesql-prebuilts-default-linux_amd64-<tag>.tar.gz`**, and the downstream checklist live in [`go-googlesql` `docs/prebuilt-cgo.md`](https://github.com/vantaboard/go-googlesql/blob/main/docs/prebuilt-cgo.md). When you bump `github.com/vantaboard/go-googlesql`, use the **same** Git tag for the Go module, any prebuilt tarball you unpack, and [`docs/stack-release-policy.md`](https://github.com/vantaboard/go-googlesql/blob/main/docs/stack-release-policy.md).

**Host linker env:** [direnv](https://direnv.net/) with this repo’s [`.envrc`](.envrc), or [`go-googlesql/scripts/go-googlesql-stack-bootstrap.sh`](https://github.com/vantaboard/go-googlesql/blob/main/scripts/go-googlesql-stack-bootstrap.sh), so **`CGO_LDFLAGS_ALLOW`** / **`CGO_LDFLAGS`** match [`go-googlesql` `Taskfile.yml`](https://github.com/vantaboard/go-googlesql/blob/main/Taskfile.yml).

For normal `bigquery-emulator` work with sibling checkouts (use [`go.work.dev`](go.work.dev) via `go.work` or `GOWORK`):

```console
$ task emulator:build
$ task docker:build
```

`emulator:build-linked` uses `go.work.dev` via `GOWORK` (same bootstrap and tags). `docker:build-linked` is an alias for `docker:build`, and [`Dockerfile.linked`](Dockerfile.linked) is the primary Docker path for CI/release and local sibling workspaces.

For **repeat** host builds, use **`CC="ccache clang"`** and **`CXX="ccache clang++"`** (and on **Linux**, **`mold`** on **`PATH`**), or **`task test:linux`** for CI-parity tests inside **`go-googlesql:dev`**.

**CI:** [`.github/workflows/test.yml`](.github/workflows/test.yml) checks out **`vantaboard/go-googlesql`** and **`vantaboard/go-googlesqlite`** at the pinned **`go.mod`** versions beside this repo, runs **`ci-download-or-build-default-prebuilts.sh`** on **`go-googlesql`**, then **`task emulator:build`** and **`go test`** with [`go-googlesql-stack-bootstrap.sh`](https://github.com/vantaboard/go-googlesql/blob/main/scripts/go-googlesql-stack-bootstrap.sh) so the default upstream **mod_platform** prebuilt path matches local sibling development.

### Local `go-googlesql` base image (upgrade / CGO cache)

Docker builds use a **pinned Go+clang** base (`GO_GOOGLESQL_BASE`, default `ghcr.io/vantaboard/go-googlesql:v0.5.12`). To validate against a **local** toolchain image you built from the `go-googlesql` repo (for example tag `go-googlesql:dev`), pass env when invoking Task:

```console
$ GO_GOOGLESQL_BASE=go-googlesql:dev task docker:build
$ GO_GOOGLESQL_BASE=go-googlesql:dev task docker:build-linked
```

Build the `go-googlesql:dev` image first (`task docker:build-dev` in `go-googlesql`). The runtime stage must stay compatible with the linked binary (same glibc/toolchain expectations as the chosen base).

### Sequential test runs and shared caches

When testing the full stack locally, run heavy **`go test` / Docker builds sequentially** across `go-googlesql`, `go-googlesqlite`, and `bigquery-emulator` so parallel CGO compiles do not exhaust memory. Reuse a shared **`GO_CACHE_ROOT`** (which backs **`GOCACHE`**, **`GOMODCACHE`**, and **`ccache`**) across the sibling checkouts for faster host-native runs.

**`GO_CACHE_ROOT`:** The [Taskfile](Taskfile.yml) **`task test:linux`** target bind-mounts **`GO_CACHE_ROOT`** (default **`$HOME/.cache/go-googlesql`**) into **`gocache`**, **`gomodcache`**, and **`ccache`** in the container—the same convention as **`go-googlesql`** and **`go-googlesqlite`**. Set **`GO_CACHE_ROOT`** consistently across sibling checkouts so one warm cache serves all three repos.

**Optional warm-up:** Run **`task -d ../go-googlesql docker:warm-cache`** once after a cold cache or toolchain change so the next **`task test:linux`** here pays less compile cost (pre-builds the **`-race`** graph without running tests).

# How to start the standalone server

If you can install the `bigquery-emulator` CLI, you can start the server using the following options.

```console
$ ./bigquery-emulator -h
Usage:
  bigquery-emulator [OPTIONS]

Application Options:
      --project=        [deprecated: use POST /emulator/v1/projects to create projects] optional seed project at startup
      --dataset=        optional seed dataset (only with --project)
      --port=           specify the http port number. this port used by bigquery api (default: 9050)
      --grpc-port=      specify the grpc port number. this port used by bigquery storage api (default: 9060)
      --log-level=      specify the log level (debug/info/warn/error) (default: error)
      --log-format=     specify the log format (console/json) (default: console)
      --log-file=       append structured logs to this file (still logs to stderr); env BIGQUERY_EMULATOR_LOG_FILE
      --database=       specify the database file if required. if not specified, it will be on memory
      --data-from-yaml= specify the path to the YAML file that contains the initial data
  -v, --version         print version

Help Options:
  -h, --help            Show this help message
```

Projects are created through the emulator-only HTTP API (not part of the BigQuery REST surface): **`POST /emulator/v1/projects`** with JSON body `{"id":"<project-id>"}`. The **`--project`** flag and **`BIGQUERY_EMULATOR_PROJECT`** environment variable are **deprecated** (they only seed an empty project at startup); prefer the API.

Start the server with no initial project:

```console
$ ./bigquery-emulator
[bigquery-emulator] REST server listening at 0.0.0.0:9050
[bigquery-emulator] gRPC server listening at 0.0.0.0:9060
```

Create a project (example uses [curl](https://curl.se/)):

```console
$ curl -sS -X POST http://127.0.0.1:9050/emulator/v1/projects -H 'Content-Type: application/json' -d '{"id":"test"}'
{"id":"test"}
```

If you want to use docker image to start emulator:

```console
$ docker run -it -p 9050:9050 -p 9060:9060 ghcr.io/vantaboard/bigquery-emulator:latest
```

## How to use from bq client

### 1. Start the standalone server

```console
$ ./bigquery-emulator --data-from-yaml=./server/testdata/data.yaml
[bigquery-emulator] REST server listening at 0.0.0.0:9050
[bigquery-emulator] gRPC server listening at 0.0.0.0:9060
```

The YAML defines projects (see `projects:` in the file), so **`--project`** is not needed.

* `server/testdata/data.yaml` is [here](https://github.com/vantaboard/bigquery-emulator/blob/main/server/testdata/data.yaml)

### 2. Call endpoint from bq client

```console
$ bq --api http://0.0.0.0:9050 query --project_id=test "SELECT * FROM dataset1.table_a WHERE id = 1"

+----+-------+---------------------------------------------+------------+----------+---------------------+
| id | name  |                  structarr                  |  birthday  | skillNum |     created_at      |
+----+-------+---------------------------------------------+------------+----------+---------------------+
|  1 | alice | [{"key":"profile","value":"{\"age\": 10}"}] | 2012-01-01 |        3 | 2022-01-01 12:00:00 |
+----+-------+---------------------------------------------+------------+----------+---------------------+
```

## How to use from python client

> **For Python unit testing**: See the comprehensive [Python Testing Guide](test/python/README.md) for using the emulator with testcontainers, pytest fixtures, and `unittest.TestCase`.

### 1. Start the standalone server

```console
$ ./bigquery-emulator
# Then create project `test` and dataset `dataset1` via the API / client libraries (see below).
[bigquery-emulator] REST server listening at 0.0.0.0:9050
[bigquery-emulator] gRPC server listening at 0.0.0.0:9060
```

Alternatively, the deprecated **`--project=test --dataset=dataset1`** still seeds an empty project and dataset at startup.

### 2. Call endpoint from python client

Create ClientOptions with api_endpoint option and use AnonymousCredentials to disable authentication.

```python
from google.api_core.client_options import ClientOptions
from google.auth.credentials import AnonymousCredentials
from google.cloud import bigquery
from google.cloud.bigquery import QueryJobConfig

client_options = ClientOptions(api_endpoint="http://0.0.0.0:9050")
client = bigquery.Client(
  "test",
  client_options=client_options,
  credentials=AnonymousCredentials(),
)
client.query(query="...", job_config=QueryJobConfig())
```

If you use a DataFrame as the download destination for the query results,
You must either disable the BigQueryStorage client with `create_bqstorage_client=False` or
create a BigQueryStorage client that references the local grpc port (default 9060).

https://cloud.google.com/bigquery/docs/samples/bigquery-query-results-dataframe?hl=en

```python
result = client.query(sql).to_dataframe(create_bqstorage_client=False)
```

or

```python
from google.cloud import bigquery_storage

client_options = ClientOptions(api_endpoint="0.0.0.0:9060")
read_client = bigquery_storage.BigQueryReadClient(client_options=client_options)
result = client.query(sql).to_dataframe(bqstorage_client=read_client)
``` 

# Synopsis

If you use the Go language as a BigQuery client, you can launch the BigQuery emulator on the same process as the testing process.  
Import `github.com/vantaboard/bigquery-emulator/server` (and `github.com/vantaboard/bigquery-emulator/types`) and you can use `server.New` API to create the emulator server instance.

See the API reference for more information: https://pkg.go.dev/github.com/vantaboard/bigquery-emulator

```go
package main

import (
  "context"
  "fmt"

  "cloud.google.com/go/bigquery"
  "github.com/vantaboard/bigquery-emulator/server"
  "github.com/vantaboard/bigquery-emulator/types"
  "google.golang.org/api/iterator"
  "google.golang.org/api/option"
)

func main() {
  ctx := context.Background()
  const (
    projectID = "test"
    datasetID = "dataset1"
    routineID = "routine1"
  )
  bqServer, err := server.New(server.TempStorage)
  if err != nil {
    panic(err)
  }
  if err := bqServer.Load(
    server.StructSource(
      types.NewProject(
        projectID,
        types.NewDataset(
          datasetID,
        ),
      ),
    ),
  ); err != nil {
    panic(err)
  }
  if err := bqServer.SetProject(projectID); err != nil {
    panic(err)
  }
  testServer := bqServer.TestServer()
  defer testServer.Close()

  client, err := bigquery.NewClient(
    ctx,
    projectID,
    option.WithEndpoint(testServer.URL),
    option.WithoutAuthentication(),
  )
  if err != nil {
    panic(err)
  }
  defer client.Close()
}
```

# Debugging

If you have specified a database file when starting `bigquery-emulator`, you can check the status of the database by using the `googlesqlite-cli` tool. See [here](https://github.com/vantaboard/go-googlesqlite/tree/main/cmd/googlesqlite-cli#readme) for details.

# How it works

## BigQuery Emulator Architecture Overview

After receiving a query, `go-googlesqlite` parses and analyzes the input query using `google/googlesql`. 
Query metadata objects are extracted from the AST, then transformed into a SQLite-compatible query.
The [modernc.org/sqlite](https://modernc.org/sqlite) driver is then used to access the SQLite Database.

```mermaid
---
config:
  theme: base
  themeVariables:
    background: '#ffffff'
    mainBkg: '#ffffff'
    clusterBkg: '#ffffff'
    lineColor: '#1f2328'
    arrowheadColor: '#1f2328'
    primaryTextColor: '#1f2328'
    primaryBorderColor: '#d0d7de'
    clusterBorder: '#d0d7de'
    edgeLabelBackground: '#ffffff'
    edgeLabelColor: '#1f2328'
    secondaryBkg: '#f6f8fa'
    tertiaryColor: '#f6f8fa'
    darkMode: false
---
flowchart TD
  subgraph diagramRoot [" "]
    direction TB
    subgraph clientLayer [Clients]
      direction TB
      bqCli[bq CLI]
      sdks["BigQuery client SDKs (Go, Python, Java, …)"]
    end
    bigqueryEmu["bigquery-emulator<br/>BigQuery REST API"]
    googlesqlite["go-googlesqlite<br/>• Parses and analyzes GoogleSQL with go-googlesql<br/>• Generates and runs SQLite via modernc.org/sqlite (database/sql)"]
    sqliteDb[(SQLite<br/>storage or :memory:)]

    clientLayer -->|"HTTP (BigQuery API)"| bigqueryEmu
    bigqueryEmu -->|"Jobs / queries as GoogleSQL strings"| googlesqlite
    googlesqlite -->|"Execute"| sqliteDb
  end
```


## Type Conversion Flow

BigQuery has a number of types that do not exist in SQLite (e.g. ARRAY and STRUCT).
In order to handle them in SQLite, `go-googlesqlite` encodes all types except `INT64` / `FLOAT64` / `BOOL` with the type information and data combination.
When using the encoded data, the data is decoded via a custom function registered with driver before use.

```mermaid
---
config:
  theme: base
  themeVariables:
    background: '#ffffff'
    mainBkg: '#ffffff'
    clusterBkg: '#ffffff'
    lineColor: '#1f2328'
    arrowheadColor: '#1f2328'
    primaryTextColor: '#1f2328'
    primaryBorderColor: '#d0d7de'
    clusterBorder: '#d0d7de'
    edgeLabelBackground: '#ffffff'
    edgeLabelColor: '#1f2328'
    secondaryBkg: '#f6f8fa'
    tertiaryColor: '#f6f8fa'
    darkMode: false
---
flowchart TD
  subgraph diagramRoot [" "]
    direction TB
    subgraph inputs [Application]
      direction TB
      lit[Literal values in SQL]
      par[Bound parameters]
    end

    subgraph ggl [go-googlesqlite]
      direction TB
      enc[Encode with type metadata]
      dec[Decode driver.Rows rows]
    end

    subgraph mod [modernc.org/sqlite]
      direction TB
      subgraph udf [Custom function]
        direction TB
        da[Decode SQL arguments]
        udfLogic[Logic]
        er[Encode return value]
      end
    end

    sq[(SQLite)]
    clientOut[Decoded values to client]

    lit --> enc
    par --> enc
    enc -->|store| sq
    sq -->|load rows| dec
    dec --> clientOut

    sq -->|load| da
    da --> udfLogic --> er
    er -->|store| sq
  end
```


# Reference

Regarding the story of bigquery-emulator, there are the following articles.
- [How to create a BigQuery Emulator](https://docs.google.com/presentation/d/1j5TPCpXiE9CvBjq78W8BWz-cGxU8djW1qy9Y6eBHso8/edit?usp=sharing) ( Japanese )


# License

MIT
