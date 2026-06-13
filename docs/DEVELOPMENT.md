# Development setup

Most of the toolchain is pinned in [`mise.toml`](https://github.com/vantaboard/bigquery-emulator/blob/main/mise.toml) and fetched by a
single command:

```bash
mise install   # or: task tools:install
```

That covers Go, Bazel, `task`, `clang` (the build toolchain `mise` materializes
for editor / dev use; Bazel itself still pins the system `/usr/bin/clang` for
hermeticity), `buf`, `golangci-lint`, `shellcheck`, `goreleaser`, `gcloud`,
`hadolint`, the Maven / Java JDK, and the Python interpreter used by the
third-party harnesses.

A few C++ tooling packages are **not** distributed via `mise` and have to be
installed through the system package manager. On Ubuntu / Debian:

```bash
sudo apt install clang-format clang-tidy cppcheck
```

These are required for the C++ lint gates (`task lint:cpp:format`,
`task lint:cpp:tidy`, `task lint:cpp:cppcheck`) and for matching what runs in
[CI](https://github.com/vantaboard/bigquery-emulator/blob/main/.github/workflows/ci.yml). On macOS use Homebrew
(`brew install clang-format llvm cppcheck`); other distributions ship equivalent
packages under similar names.

For the full lint policy — commands, thresholds, baselines, and suppression
markers — see [`docs/dev/cpp-lint.md`](./dev/cpp-lint.md).

## Repo layout

```
bigquery-emulator/
  binaries/
    gateway_main/         # Go REST gateway entrypoint (cli.go, main.go)
    emulator_main/        # C++ engine entrypoint (main.cc + version glue)
  gateway/              # Go: HTTP server + subprocess manager
    gateway.go            # Lifecycle: spawn engine, run HTTP, shutdown
    server.go             # HTTP routing
    handlers/             # BigQuery REST handlers (one file per resource)
    middleware/           # Auth + request logging middleware
    bqtypes/              # Wire-compatible BigQuery REST types
    enginepb/             # Generated Go bindings for proto/*.proto
    engine/               # gRPC client wrapper for emulator_main
    jobs/                 # In-process job lifecycle (creation -> done)
    seed/, seedfile/      # Seed-data REST API + YAML applier
    e2e/                  # Integration tests against a real engine
  frontend/             # C++: gRPC server that fronts the engine
    server/                 # gRPC plumbing
    handlers/               # Catalog + Query + StorageRead services
  backend/              # C++: catalog / schema / storage / engine
    catalog/, schema/       # GoogleSQL catalog adapter + type mapping
    storage/duckdb/         # DuckDB-backed catalog.duckdb persistence
    engine/                 # Engine interface + local execution coordinator
    engine/duckdb/          # DuckDB fast path: AST -> DuckDB SQL transpiler
                            # (additional strategies — semantic executor,
                            # DuckDB UDF/polyfill library, control-op
                            # executor — live alongside duckdb/ as they
                            # land; see ROADMAP.md "Execution strategies")
  proto/                # Internal Go <-> C++ contract
    emulator.proto        # Catalog + Query services
    storage_read.proto    # bigquery_emulator.v1.StorageRead
  conformance/          # YAML fixture runner + diff harness
    cmd/runner/             # `go run ./conformance/cmd/runner`
    fixtures/               # YAML fixtures (SELECT / GROUP BY / JOIN / DDL / ...)
  third_party/          # Vendored upstream client-library sample suites
    golang-bigquery-tests/, node-bigquery-tests/,
    python-bigquery-tests/, python-bigquery-dataframes-tests/,
    java-bigquery-tests/, duckdb/  # DuckDB pin
  docs/                 # Documentation
    REST_API.md           # Endpoint -> handler mapping (read this when
                          # debugging a specific BigQuery REST call)
    ENGINE_POLICY.md      # Local-only execution policy + route catalog
    SEEDING.md            # Declarative + template + REST seeding
    dev/                  # C++ lint policy, prebuilt GoogleSQL playbooks
    bigquery/             # Vendored copy of the upstream BigQuery docs
                          # corpus, used as the source of truth when
                          # verifying request/response shapes
  taskfiles/            # Per-namespace Task includes (emulator:, lint:,
                        # test:, docker:, conformance:, thirdparty:, ...)
  Taskfile.yml          # Common dev commands (build, run, test, lint)
  Makefile              # Same as Taskfile, for users who prefer make
  BUILD.bazel           # Bazel root
  MODULE.bazel          # Bzlmod root (GoogleSQL via local_path_override,
                        # DuckDB via http_archive)
  Dockerfile            # Multi-stage build: engine-builder-bazel + runtime
  docker-compose.yml    # `docker compose up` runtime + smoke recipe
  go.mod / go.sum       # Go module
  ROADMAP.md            # Capability-area plan (read this first)
  README.md
  LICENSE               # MIT
```

Run `task --list` for the full set of namespaces (`emulator:`, `lint:`,
`test:`, `docker:`, `conformance:`, `thirdparty:`, `googlesql:`, `release:`,
`ci:`, `tools:`).

## Building the engine

The C++ engine is built with Bazel. Run `task emulator:build-engine:bazel`
(alias: `task emulator:build-engine`). This links GoogleSQL's analyzer with the
local execution coordinator (DuckDB fast path today, with the semantic executor
/ control-op executor / DuckDB UDF polyfills slotting in behind the same
`Engine` interface), the DuckDB storage backend, and gRPC. The output binary
serves `Query.DryRun` and `Query.ExecuteQuery` end-to-end. The integration tests
under `gateway/e2e/` drive this binary directly.

GoogleSQL is wired in via Bazel; DuckDB v1.5.3 is pulled in as a prebuilt
tarball through `http_archive` (see [`third_party/duckdb/`](https://github.com/vantaboard/bigquery-emulator/tree/main/third_party/duckdb)).

Linux/amd64 only today — the GoogleSQL hermetic LLVM toolchain does not
cross-build cleanly to linux/arm64 yet, so the engine binary ships only for
amd64. Non-amd64 hosts should use the published Docker image.

### GoogleSQL build mode (prebuilt by default)

`task emulator:build-engine:bazel` consumes a **prebuilt GoogleSQL artifact** by
default — it does not recompile GoogleSQL on every build. Cold builds drop from
the multi-hour source link to a few minutes once the cache is warm.

`GOOGLESQL_SOURCE` selects the mode (defaulted to `prebuilt`):

```bash
# Default — prebuilt GoogleSQL from .cache/googlesql-prebuilt/.
task emulator:build-engine:bazel

# Explicit source rebuild from a sibling ../googlesql/ checkout.
# Use this when iterating on a GoogleSQL upgrade or producer change.
GOOGLESQL_SOURCE=local task emulator:build-engine:bazel
```

There is **no silent fallback**: if `GOOGLESQL_SOURCE=prebuilt` is selected (or
defaulted) and the prebuilt cache is empty or stale, the build refuses to start
with an actionable diagnostic instead of silently doing the wrong thing.

Populate the cache once with either:

```bash
# Published artifact (URL + SHA-256 from the producer release notes).
task googlesql:fetch-prebuilt URL=<asset url> SHA256=<64 hex chars>

# Or a locally-built real artifact (uses your sibling ../googlesql/
# checkout; ~25-55 min cold cache).
task googlesql:stage-bazel
```

Inspect or revalidate the cache without rebuilding:

```bash
task googlesql:status    # what's currently staged
task googlesql:validate  # re-run the safety validator over the cache
```

Escape hatch for the validator only (cache contents are still expected to be
correct; **never** use this to mask a SHA mismatch):

```bash
BIGQUERY_EMULATOR_SKIP_PREBUILT_VALIDATE=1 \
    task emulator:build-engine:bazel
```

### Prebuilt GoogleSQL vs prebuilt emulator engine binary

Two unrelated "prebuilt" surfaces both ship with releases — don't confuse them:

| Asset | What it is | Who consumes it |
|-------|------------|-----------------|
| **Prebuilt GoogleSQL artifact** (`googlesql-prebuilt/v<...>+gs-<...>` GitHub release) | The `@googlesql//...` Bazel external repo packaged as a `.tar.gz` with `manifest.json`. Lets `task emulator:build-engine:bazel` link GoogleSQL without compiling its ~8K C++ TUs. | Engine **builders** (this repo's CI, Docker `engine-builder-bazel` stage, release.yml) — anyone who needs to link `emulator_main` from source. |
| **Prebuilt emulator engine binary** (`bin/emulator_main` + `bin/libduckdb.so` inside the release archives and Docker image) | The already-linked C++ engine binary itself. Skips the Bazel build entirely. | Engine **users** — anyone who just wants to run the emulator (`docker run ghcr.io/...:vX.Y.Z`, the goreleaser tarballs, the Docker `ENGINE_SOURCE=prebuilt` stage). |

If you are not running `bazel build` or `task emulator:build-engine:bazel`, you
are using the prebuilt engine binary and the GoogleSQL artifact is irrelevant to
you. The Docker image and the release archives both ship the prebuilt binary; the
GoogleSQL artifact surface only matters if you are building the engine yourself.

### When the prebuilt artifact fetch or validation fails

If the build refuses to start, the diagnostic line names the gate that tripped.
Common shapes:

- `GOOGLESQL_SOURCE=prebuilt (default) but the cache is empty at: <path>` — the cache has not been populated. Run `task googlesql:fetch-prebuilt URL=... SHA256=...` or fall back to source mode explicitly.
- `validate_artifact: FAIL_PAYLOAD_SHA` (or any other `FAIL_*` token) — the validator rejected the staged cache. Each token corresponds to a specific failure class (checksum, platform, missing wrapper, manifest schema, …). The [GoogleSQL prebuilt troubleshooting guide](https://github.com/vantaboard/bigquery-emulator/blob/main/docs/dev/googlesql-prebuilt/troubleshooting.md) maps every `FAIL_*` token to the likely owner and the next step; the [rollback playbook](https://github.com/vantaboard/bigquery-emulator/blob/main/docs/dev/googlesql-prebuilt/rollback.md) covers the matching repin / revert procedures.

For the full maintainer flow (publishing new artifacts, bumping the GoogleSQL
pin, releasing) start at the
[GoogleSQL prebuilt docs index](https://github.com/vantaboard/bigquery-emulator/blob/main/docs/dev/googlesql-prebuilt/README.md).

## Runtime configuration

`emulator_main` runs a single local emulator process with a single `Engine`
interface and a single persistent storage backend (`DuckDBStorage`, `catalog.duckdb`
under `--data_dir`). Query execution behind that interface is multi-strategy — the
local coordinator chooses DuckDB-native SQL, a DuckDB rewrite/UDF, the semantic
executor, or a catalog/control handler per query — but the runtime surface stays a
single emulator binary with no runtime backend selector. There are no `--engine` /
`--storage` / `--on_unknown_fn` flags (those were removed when the ReferenceImpl
engine and in-memory storage backends were deleted, and the new multi-strategy
coordinator intentionally keeps the same single-knob posture). The only flags are
`--host_port` and `--data_dir`:

```bash
# Default: bind to 127.0.0.1:9060, use $HOME/.bigquery-emulator as the
# DuckDB catalog root.
./bin/emulator_main

# Pick a port + hermetic data dir (used by tests, conformance,
# `docker compose up`).
./bin/emulator_main --host_port=127.0.0.1:9060 --data_dir=/tmp/bq-emu
```

The gateway forwards `--data_dir` (and any of its hyphen-aliased equivalents — see
[`docs/SEEDING.md`](./SEEDING.md)) to `emulator_main` on spawn, so all of the
following set the same DuckDB catalog root:

```bash
./bin/bigquery-emulator-gateway --data-dir /var/lib/bq-emu
docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:v0.0.1 \
    --data-dir /var/lib/bq-emu
```

See [`docs/ENGINE_POLICY.md`](./ENGINE_POLICY.md) for the local-only execution
policy, the per-shape route catalog (DuckDB fast path / DuckDB rewrite / DuckDB UDF
/ semantic executor / control op / unsupported-by-design), and which DML / DDL
shapes are still UNIMPLEMENTED, and the conformance harness
([`conformance/README.md`](https://github.com/vantaboard/bigquery-emulator/blob/main/conformance/README.md)) for the shape of the
per-fixture diff.
