# BigQuery Emulator

[![CI](https://github.com/vantaboard/bigquery-emulator/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/vantaboard/bigquery-emulator/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/vantaboard/bigquery-emulator/branch/main/graph/badge.svg)](https://codecov.io/gh/vantaboard/bigquery-emulator)
[![Go version](https://img.shields.io/github/go-mod/go-version/vantaboard/bigquery-emulator)](go.mod)
[![Top language](https://img.shields.io/github/languages/top/vantaboard/bigquery-emulator)](https://github.com/vantaboard/bigquery-emulator/search?l=go)
[![Languages](https://img.shields.io/github/languages/count/vantaboard/bigquery-emulator)](https://github.com/vantaboard/bigquery-emulator)
[![License](https://img.shields.io/github/license/vantaboard/bigquery-emulator)](LICENSE)

A locally-runnable emulator of Google Cloud BigQuery, intended for local
development and integration testing of applications that target the
BigQuery REST API.

> **Status:** very early scaffold. The Go REST gateway boots, answers a
> health probe, and registers every documented BigQuery v2 REST endpoint
> as a 501 stub. The C++ engine is a placeholder. See
> [`ROADMAP.md`](./ROADMAP.md) for the phased plan and
> [`docs/REST_API.md`](./docs/REST_API.md) for the per-endpoint mapping
> and current status.

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
  BUILD.bazel           # Bazel root (planned)
  MODULE.bazel          # Bzlmod root (planned)
  go.mod / go.sum       # Go module
  docs/                 # Documentation
    REST_API.md           # Endpoint -> handler mapping (read this when
                          # debugging a specific BigQuery REST call)
    bigquery/             # Vendored copy of the upstream BigQuery docs
                          # corpus, used as the source of truth when
                          # verifying request/response shapes
  ROADMAP.md            # Phased plan (read this first)
  README.md
  LICENSE               # MIT
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
task emulator:build-all   # build both gateway_main and emulator_main
task emulator:run-full    # run gateway, which spawns the engine
```

Run `task --list` for the full set of namespaces (`emulator:`, `lint:`,
`test:`, `docker:`, `ci:`, `tools:`).

### Building the engine

The C++ engine is built with Bazel. Run
`task emulator:build-engine:bazel` (alias: `task emulator:build-engine`).
This links GoogleSQL's analyzer + reference-impl evaluator along with the
DuckDB storage backend and gRPC, producing a binary that serves
`Query.DryRun` and `Query.ExecuteQuery` end-to-end. The integration
tests under `gateway/e2e/` drive this binary directly.

GoogleSQL is wired in via Bazel; DuckDB v1.5.3 is pulled in as a
prebuilt tarball through `http_archive` (see
[`third_party/duckdb/`](./third_party/duckdb/)).

Linux/amd64 only today — the GoogleSQL hermetic LLVM toolchain does
not cross-build cleanly to linux/arm64 yet, so the engine binary
ships only for amd64. Non-amd64 hosts should use the published
Docker image.

#### GoogleSQL build mode (prebuilt by default)

`task emulator:build-engine:bazel` consumes a **prebuilt GoogleSQL
artifact** by default — it does not recompile GoogleSQL on every
build. Cold builds drop from the multi-hour source link to a few
minutes once the cache is warm.

`GOOGLESQL_SOURCE` selects the mode (defaulted to `prebuilt`):

```bash
# Default — prebuilt GoogleSQL from .cache/googlesql-prebuilt/.
task emulator:build-engine:bazel

# Explicit source rebuild from a sibling ../googlesql/ checkout.
# Use this when iterating on a GoogleSQL upgrade or producer change.
GOOGLESQL_SOURCE=local task emulator:build-engine:bazel
```

There is **no silent fallback**: if `GOOGLESQL_SOURCE=prebuilt` is
selected (or defaulted) and the prebuilt cache is empty or stale,
the build refuses to start with an actionable diagnostic instead
of silently doing the wrong thing.

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

Escape hatch for the validator only (cache contents are still
expected to be correct; **never** use this to mask a SHA mismatch):

```bash
BIGQUERY_EMULATOR_SKIP_PREBUILT_VALIDATE=1 \
    task emulator:build-engine:bazel
```

#### Prebuilt GoogleSQL vs prebuilt emulator engine binary

Two unrelated "prebuilt" surfaces both ship with releases — don't
confuse them:

| Asset | What it is | Who consumes it |
|-------|------------|-----------------|
| **Prebuilt GoogleSQL artifact** (`googlesql-prebuilt/v<...>+gs-<...>` GitHub release) | The `@googlesql//...` Bazel external repo packaged as a `.tar.gz` with `manifest.json`. Lets `task emulator:build-engine:bazel` link GoogleSQL without compiling its ~8K C++ TUs. | Engine **builders** (this repo's CI, Docker `engine-builder-bazel` stage, release.yml) — anyone who needs to link `emulator_main` from source. |
| **Prebuilt emulator engine binary** (`bin/emulator_main` + `bin/libduckdb.so` inside the release archives and Docker image) | The already-linked C++ engine binary itself. Skips the Bazel build entirely. | Engine **users** — anyone who just wants to run the emulator (`docker run ghcr.io/...:vX.Y.Z`, the goreleaser tarballs, the Docker `ENGINE_SOURCE=prebuilt` stage). |

If you are not running `bazel build` or `task emulator:build-engine:bazel`,
you are using the prebuilt engine binary and the GoogleSQL artifact
is irrelevant to you. The Docker image and the release archives both
ship the prebuilt binary; the GoogleSQL artifact surface only matters
if you are building the engine yourself.

#### When the prebuilt artifact fetch or validation fails

If the build refuses to start, the diagnostic line names the gate
that tripped. Common shapes:

- `GOOGLESQL_SOURCE=prebuilt (Phase 4 default) but the cache is empty at: <path>` — the cache has not been populated. Run `task googlesql:fetch-prebuilt URL=... SHA256=...` or fall back to source mode explicitly.
- `validate_artifact: FAIL_PAYLOAD_SHA` (or any other `FAIL_*` token) — the validator rejected the staged cache. Each token corresponds to a specific failure class (checksum, platform, missing wrapper, manifest schema, …). The [GoogleSQL prebuilt troubleshooting guide](./docs/dev/googlesql-prebuilt/troubleshooting.md) maps every `FAIL_*` token to the likely owner and the next step; the [rollback playbook](./docs/dev/googlesql-prebuilt/rollback.md) covers the matching repin / revert procedures.

For the full maintainer flow (publishing new artifacts, bumping the
GoogleSQL pin, releasing) start at the
[GoogleSQL prebuilt docs index](./docs/dev/googlesql-prebuilt/README.md).

### Docker

The repo ships a multi-stage [`Dockerfile`](./Dockerfile) that builds
both the Go gateway and the C++ engine (the canonical Bazel
`//binaries/emulator_main:emulator_main` target, which links the full
GoogleSQL analyzer + reference-impl evaluator + DuckDB) and packages
them into a single runtime image. The layout mirrors the
`gcr.io/cloud-spanner-emulator/emulator` image. A
`docker/gateway_main.sh` shim injects `--hostname=0.0.0.0` inside the
container so the published port is reachable from the host without
forcing every caller to remember the flag.

> **Cold-cache build is slow.** The Bazel engine link pulls in
> GoogleSQL's source tree (~8K C++ TUs); a first-time `docker build`
> on a fresh runner can run 25–55 minutes. The `engine-builder` stage
> uses a BuildKit cache mount on `/root/.cache/bazel`, so warm
> rebuilds typically land in well under two minutes. Set
> `DOCKER_BUILDKIT=1` (default on Docker Desktop / recent Engine) and
> let the cache do its job.

#### Quickstart with `docker compose`

The fastest path is the top-level [`docker-compose.yml`](./docker-compose.yml):

```bash
docker compose up -d --build

# Liveness:
#   {"service":"bigquery-emulator","status":"ok"}
curl -fsS http://localhost:9050/healthz

# Synchronous SELECT 1 round-trip. Returns:
#   {"kind":"bigquery#queryResponse","jobReference":{...},
#    "schema":{"fields":[{"name":"n","type":"INTEGER",...}]},
#    "rows":[{"f":[{"v":"1"}]}],
#    "totalRows":"1","jobComplete":true}
curl -fsS -X POST http://localhost:9050/bigquery/v2/projects/test/queries \
    -H 'Content-Type: application/json' \
    -d '{"query":"SELECT 1 AS n","useLegacySql":false}'

# Tear down + drop the persistent volume:
docker compose down -v
```

The same recipe is wired up as `task docker:smoke` (see
[`taskfiles/docker.yml`](./taskfiles/docker.yml)) and runs in CI via
[`.github/workflows/docker-smoke.yml`](./.github/workflows/docker-smoke.yml).

#### Plain `docker run`

```bash
# Build the image. Tag whatever you like; `bigquery-emulator:dev` here.
docker build -t bigquery-emulator:dev .

# Run it. Publish the REST gateway (9050) and, optionally, the internal
# engine gRPC port (9060) for debugging.
docker run --rm -p 9050:9050 -p 9060:9060 bigquery-emulator:dev

# In another shell, hit the REST surface on the host:
curl -sS http://localhost:9050/healthz
curl -sS http://localhost:9050/bigquery/v2/projects/test/datasets
```

To override container defaults, pass extra flags after the image name —
they are forwarded to `gateway_main`:

```bash
docker run --rm -p 9050:9050 bigquery-emulator:dev \
    --log_requests --hostname=0.0.0.0 --http_port=9050
```

## Profiles

`emulator_main`'s `--engine` / `--storage` / `--on_unknown_fn` flags
form a small product space. Three combinations are named, documented,
and exercised in conformance:

| Profile  | `--engine`        | `--storage` | `--on_unknown_fn`   | Use case                                                                 |
|----------|-------------------|-------------|---------------------|--------------------------------------------------------------------------|
| `ci`     | `reference_impl`  | `memory`    | `unimplemented`     | Hermetic, no on-disk state. The conformance `memory` profile and the CI smoke lanes. Slow but the source of truth for semantics (`docs/ENGINE_POLICY.md`). |
| `duckdb` | `duckdb`          | `duckdb`    | `fallback`          | Persistent, GoogleSQL-via-DuckDB. The canonical `docker compose up` configuration; the conformance `duckdb` profile; the recommended day-to-day shape. |
| `dev`    | `duckdb`          | `duckdb`    | `unimplemented`     | DuckDB analyzer + DuckDB persistence with the fallback bridge OFF, so transpiler-uncovered shapes surface as `UNIMPLEMENTED` instead of silently routing to the reference impl. Use when you're working on the transpiler itself and want every gap to fail loud. |

Each profile is a thin label on top of the underlying flags; nothing in
the gateway or the conformance harness changes behavior on the label,
only on the flag combination. The two equivalent ways to select a
profile:

```bash
# 1. Use the engine's --profile shorthand (currently `ci` / `dev`;
#    see `emulator_main --help`). `--profile` defaults engine + storage
#    in one knob; explicit `--engine` / `--storage` flags always win:
./bin/emulator_main --profile=ci
./bin/emulator_main --profile=dev

# 2. Set the underlying flags explicitly. Identical to --profile but
#    surfaces the engine / storage decision in the command line:
./bin/emulator_main --engine=reference_impl --storage=memory                    # ci
./bin/emulator_main --engine=duckdb --storage=duckdb --on_unknown_fn=fallback   # duckdb (canonical)
./bin/emulator_main --engine=duckdb --storage=duckdb                            # dev
```

When the gateway spawns the engine (`task emulator:run-full`,
`docker compose up`, the published Docker image, the goreleaser
archive), it forwards every flag after the gateway-recognized set to
the engine. So the same profile knobs work end-to-end:

```bash
./bin/bigquery-emulator-gateway --engine=duckdb --storage=duckdb --on_unknown_fn=fallback
```

For Docker, append the flags after the image name (the
`docker/gateway_main.sh` shim forwards them):

```bash
docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:v0.0.1 \
    --engine=duckdb --storage=duckdb --on_unknown_fn=fallback
```

The `duckdb` profile is the recommended day-to-day shape: it has the
analyzer parity of the reference impl wherever the DuckDB transpiler
has coverage, the persistent on-disk catalog under `--data_dir`
(default `$HOME/.bigquery-emulator`), and the
`--on_unknown_fn=fallback` bridge to the reference impl for shapes the
transpiler does not yet cover. See [`docs/ENGINE_POLICY.md`](./docs/ENGINE_POLICY.md)
for the engine-asymmetry rationale (DuckDB is the active development
surface; the reference impl is maintenance-mode), and the conformance
harness ([`conformance/README.md`](./conformance/README.md)) for how
the same profile labels drive fixture selection.

## Seeding & CLI compatibility

The gateway accepts both the legacy underscore flag names this
repository started with (`--http_port`) and the hyphen-separated
names that `go-googlesql`'s `bq-emulator` exposes (`--http-port`),
so existing scripts keep working and operators can lift invocation
snippets from the upstream docs unchanged. The full alias table
and the seeding workflows live in
[`docs/SEEDING.md`](./docs/SEEDING.md):

1. **Declarative YAML seed files** via `--seed-data-file FILE.yaml`
   (repeatable). Loaded after the engine reports SERVING but before
   the gateway accepts public traffic, so any client that hits the
   REST API sees the seeded datasets/tables/rows from request one.
2. **Initial-data template directory** via `--initial-data-dir DIR`.
   The gateway copies the tree into `--data-dir` on first boot
   (when no `catalog.duckdb` is present) and never on subsequent
   boots, so operator writes are protected.
3. **Production seed REST API** via `--enable-seed-api`. Hits
   `POST /api/emulator/seed` to mirror live BigQuery metadata + rows
   into the local emulator; the polling endpoint is
   `GET /api/emulator/seed/operations/{id}`. Off by default;
   loopback-only by default; optional shared-secret header for
   defense in depth. The production reader is opt-in via the
   `seed_production_live` build tag so the default gateway build
   stays free of the cloud BigQuery client deps.

## Pointing client libraries at the emulator

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

Bearer tokens in `Authorization` headers are accepted but never
validated, identical to `cloud-spanner-emulator`'s posture. The full
upstream auth model (ADC, service-account keys, OAuth scopes) documented
under [`docs/bigquery/docs/authentication.md`](./docs/bigquery/docs/authentication.md)
is intentionally **not** modeled.

### SQL dialect

BigQuery's `useLegacySql` field defaults to `true` on the wire (older
clients still rely on this). The emulator only supports GoogleSQL,
because the engine is GoogleSQL's analyzer + reference impl. The query
handlers will:

- Treat `useLegacySql` unset or `false` as GoogleSQL.
- Reject `useLegacySql=true` with HTTP 400 + `reason: invalidQuery`.

If you're using the official Go client, explicitly set
`Query.UseLegacySQL = false` to be safe.

Python (`google-cloud-bigquery`), Java, and Node.js clients all support
the analogous endpoint override. We document each one as the relevant
smoke tests pass in Phase 8.

### Test lanes

The repository runs two parallel conformance lanes against the same
gateway:

1. **Fixture conformance** — `task conformance:*` drives YAML fixtures
   through the in-repo runner and pins SQL semantics for both the
   `memory` and `duckdb` profiles. See
   [`conformance/README.md`](./conformance/README.md) for the fixture
   schema, profile matrix, and authoring guide.
2. **Third-party client conformance** — `task thirdparty:*` runs the
   imported BigQuery client-library sample suites (Go, Node.js, Python,
   BigQuery DataFrames) end-to-end against the gateway's REST + gRPC
   surface and (optionally) `fake-gcs-server`. See
   [`third_party/README.md`](./third_party/README.md) for the
   per-language wiring contract, env-var matrix, and skip rules.

## Releases

> **Preview-grade.** The `v0.x` series is an explicit preview: the REST
> surface, gRPC contract, and on-disk format may break across releases.
> Stable promises arrive at `v1.0.0`. See [`HANDOFF.md`](./HANDOFF.md)
> §6 and [`ROADMAP.md`](./ROADMAP.md) for the active plan.

Releases are cut by tag push today. Tag the commit you want to release
with `vX.Y.Z` and push the tag; that triggers
[`.github/workflows/release.yml`](./.github/workflows/release.yml),
which builds the engine via Bazel, publishes the runtime Docker image
to [GHCR](https://github.com/users/vantaboard/packages/container/package/bigquery-emulator),
and uses [goreleaser](https://goreleaser.com) (config:
[`.goreleaser.yml`](./.goreleaser.yml)) to upload the gateway archives
+ SHA-256 checksums to the GitHub release page.

```bash
# Cut the very first release (preview).
git tag -a v0.0.1 -m 'release: v0.0.1 (preview)'
git push origin v0.0.1
```

`task release:tag VERSION=v0.0.1` is a foot-gun guard around the
above: it prints the exact `git tag` + `git push` lines and only
executes them when `CONFIRM=yes` is set
(`task release:tag VERSION=v0.0.1 CONFIRM=yes`).

The semantic-release config at [`.releaserc.yml`](./.releaserc.yml)
is parked for the eventual switch to auto-release on push to `main`.
It is not currently driving any GitHub Actions job; the file exists so
the conventional-commits format documented in
`.cursor/rules/auto-commit.mdc` has a target for the future flip.

### Install via release archive

```bash
# Pick the right tarball for your OS/arch from the releases page:
# https://github.com/vantaboard/bigquery-emulator/releases
curl -fL https://github.com/vantaboard/bigquery-emulator/releases/download/v0.0.1/bigquery-emulator_0.0.1_linux_amd64.tar.gz \
    | tar xz
./bigquery-emulator-gateway --help
```

Each archive bundles `bigquery-emulator-gateway` (the Go REST gateway)
plus `bin/emulator_main` and `bin/libduckdb.so` (the C++ engine). The
gateway's `--engine_binary` flag defaults to discovering the engine
beside the gateway binary, so `./bigquery-emulator-gateway` works
out of the tarball without extra flags.

> **Engine binary is linux/amd64 only.** Upstream GoogleSQL's
> hermetic LLVM toolchain does not yet cross-build cleanly to
> linux/arm64, and macOS engine builds are out of scope for the
> preview series. The macOS + linux/arm64 archives still bundle the
> linux/amd64 engine binary so the layout stays uniform, but you
> cannot run those engine binaries on a non-linux/amd64 host. The
> recommended path on macOS or linux/arm64 is the published Docker
> image (next section).

### Install via Docker

```bash
docker pull ghcr.io/vantaboard/bigquery-emulator:v0.0.1
docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:v0.0.1
```

Each release publishes four tags to GHCR:

- `vX.Y.Z` — exact version (immutable).
- `vX.Y` — minor track (moves on patch releases).
- `vX` — major track (moves on minor + patch releases).
- `latest` — newest non-pre-release.

Pre-release tags (`v0.0.1-rc1`) skip the `latest` tag promotion so
`docker pull ...:latest` always lands on a non-pre-release version.
The Docker image is `linux/amd64` only for the same engine-binary
reason as above.

## Why C++ for the engine

See the rationale captured at the top of [`ROADMAP.md`](./ROADMAP.md). The
short version: GoogleSQL is a 700+-file, monthly-releasing C++ library with
no idiomatic Go equivalent. `cloud-spanner-emulator` and Google's own
non-C++ GoogleSQL bindings (Java) all wrap the C++ implementation rather
than porting it. We do the same and put our effort into the
BigQuery-specific surface.

## License

MIT. See [`LICENSE`](./LICENSE).
