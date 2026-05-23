# BigQuery Emulator

A locally-runnable emulator of Google Cloud BigQuery, intended for local
development and integration testing of applications that target the
BigQuery REST API.

> **Status:** very early scaffold. The Go REST gateway boots and answers a
> health probe. The C++ engine is a placeholder. See [`ROADMAP.md`](./ROADMAP.md)
> for what is implemented and what is planned.

## Architecture

This emulator is modeled directly on Google's
[`cloud-spanner-emulator`](https://github.com/GoogleCloudPlatform/cloud-spanner-emulator):

```
+-------------------------------+        +--------------------------------+
|  gateway_main (Go)            |  gRPC  |  emulator_main (C++)           |
|                               | <----> |                                |
|  - Implements BigQuery REST   |        |  - Links GoogleSQL directly    |
|    (projects/datasets/tables/ |        |  - Analyzer + reference_impl   |
|     jobs/queries/insertAll)   |        |  - In-memory catalog/storage   |
|  - Spawns engine as subproc   |        |                                |
+-------------------------------+        +--------------------------------+
```

- The **engine is C++** so it can link [GoogleSQL](https://github.com/google/googlesql)
  directly. SQL parsing, name resolution, type inference, and reference-impl
  execution come from upstream. We do **not** re-port that surface to Go.
- The **REST gateway is Go** because that is where the BigQuery-specific
  value lives: REST routes, jobs lifecycle, datasets/tables/projects model,
  streaming inserts, error envelope, discovery doc.
- The Go gateway spawns the C++ engine as a subprocess on startup and
  shuts it down cleanly on exit, identical to how `gateway_main` spawns
  `emulator_main` in the Spanner emulator.

This split is the same one Google's own emulator team picked for Spanner,
and the same one [`goccy/bigquery-emulator`][goccy] has converged on (via
WASM) for similar reasons. See [`ROADMAP.md`](./ROADMAP.md) for the design
rationale.

[goccy]: https://github.com/goccy/bigquery-emulator

## Repo layout

```
bigquery-emulator/
  binaries/
    gateway_main/main.go    # Go REST gateway entrypoint
    emulator_main/main.cc   # C++ engine entrypoint (links GoogleSQL)
  gateway/              # Go: HTTP server + subprocess manager
    gateway.go            # Lifecycle: spawn engine, run HTTP, shutdown
    server.go             # HTTP routing
    handlers/             # BigQuery REST handlers (one file per resource)
    bqtypes/              # Wire-compatible BigQuery REST types
    enginepb/             # Generated Go bindings for proto/emulator.proto
  frontend/             # C++: gRPC server that fronts the engine
    server/                 # gRPC plumbing
    handlers/               # Implements proto/emulator.proto services
  backend/              # C++: in-memory storage / schema / catalog
  proto/                # Internal Go <-> C++ contract
    emulator.proto
  build/                # Bazel + Docker glue (planned)
  Taskfile.yml          # Common dev commands (build, run, test, lint)
  Makefile              # Same as Taskfile, for users who prefer make
  CMakeLists.txt        # CMake build for the C++ side (alt. to Bazel)
  BUILD.bazel           # Bazel root (planned)
  MODULE.bazel          # Bzlmod root (planned)
  go.mod / go.sum       # Go module
  ROADMAP.md            # Phased plan (read this first)
  README.md
  LICENSE               # Apache-2.0
```

## Quickstart

> Right now only the Go side builds and runs. Engine wiring is Phase 2 in
> [`ROADMAP.md`](./ROADMAP.md).

```bash
# Build the gateway.
go build -o bin/gateway_main ./binaries/gateway_main

# Run the gateway. It will try to spawn the engine; for now you can disable
# the engine subprocess with --engine_binary="" while we are still scaffolding.
./bin/gateway_main --engine_binary="" --http_port=9050

# In another shell:
curl -sS http://localhost:9050/        # health check
curl -sS http://localhost:9050/bigquery/v2/projects/test/datasets
```

When the C++ side starts being implemented (Phase 2 onward), the default
flow becomes:

```bash
task build         # build both gateway_main and emulator_main
task run           # run gateway, which spawns the engine
```

## Pointing client libraries at the emulator

Once Phase 1 is complete, BigQuery client libraries can be pointed at the
emulator using the standard endpoint override. For the Go client:

```go
client, err := bigquery.NewClient(ctx, "test-project",
    option.WithEndpoint("http://localhost:9050"),
    option.WithoutAuthentication(),
)
```

Python (`google-cloud-bigquery`), Java, and Node.js clients all support the
analogous endpoint override. We will document each one as we get the
relevant smoke tests passing in Phase 8.

## Why C++ for the engine

See the rationale captured at the top of [`ROADMAP.md`](./ROADMAP.md). The
short version: GoogleSQL is a 700+-file, monthly-releasing C++ library with
no idiomatic Go equivalent. `cloud-spanner-emulator` and Google's own
non-C++ GoogleSQL bindings (Java) all wrap the C++ implementation rather
than porting it. We do the same and put our effort into the
BigQuery-specific surface.

## License

MIT. See [`LICENSE`](./LICENSE).
