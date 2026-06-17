# Docker

The fastest way to get started is the published image on GHCR — no Bazel build,
no GoogleSQL toolchain. The sections below also cover building the image from
source when you need a local checkout or custom flags.

## Install via Docker

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
`docker pull ...:latest` always lands on a non-pre-release version. The Docker
image is `linux/amd64` only — upstream GoogleSQL's hermetic LLVM toolchain does
not yet cross-build cleanly to linux/arm64, and macOS engine builds are out of
scope for the preview series. See [Releases & install](./RELEASES.md) for
release archives when you need a native gateway binary without Docker.

Verify the emulator is up:

```bash
curl -fsS http://localhost:9050/healthz
curl -fsS -X POST http://localhost:9050/bigquery/v2/projects/test/queries \
    -H 'Content-Type: application/json' \
    -d '{"query":"SELECT 1 AS n","useLegacySql":false}'
```

## Build from source

The repo ships a multi-stage [`Dockerfile`](https://github.com/vantaboard/bigquery-emulator/blob/main/Dockerfile) that builds both the
Go gateway and the C++ engine (the canonical Bazel
`//binaries/emulator_main:emulator_main` target, which links the full GoogleSQL
analyzer + the local execution coordinator + DuckDB storage) and packages them
into a single runtime image. The layout mirrors the
`gcr.io/cloud-spanner-emulator/emulator` image. A `docker/gateway_main.sh` shim
injects `--hostname=0.0.0.0` inside the container so the published port is
reachable from the host without forcing every caller to remember the flag.

> **Cold-cache build is slow.** The Bazel engine link pulls in GoogleSQL's source
> tree (~8K C++ TUs); a first-time `docker build` on a fresh runner can run
> 25–55 minutes. The `engine-builder` stage uses a BuildKit cache mount on
> `/root/.cache/bazel`, so warm rebuilds typically land in well under two minutes.
> Set `DOCKER_BUILDKIT=1` (default on Docker Desktop / recent Engine) and let
> the cache do its job.

### Quickstart with `docker compose`

When working from a repo checkout, the top-level [`docker-compose.yml`](https://github.com/vantaboard/bigquery-emulator/blob/main/docker-compose.yml) builds and runs the stack locally:

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
[`taskfiles/docker.yml`](https://github.com/vantaboard/bigquery-emulator/blob/main/taskfiles/docker.yml)) and runs in CI via
[`.github/workflows/docker-smoke.yml`](https://github.com/vantaboard/bigquery-emulator/blob/main/.github/workflows/docker-smoke.yml).

### Plain `docker run`

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

### Passing CLI flags

Any flags you put after the image name are forwarded to `gateway_main`. This
works for both the locally-built and the published image:

```bash
docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:latest \
    --log_requests --http_port=9050 --project-id=my-project
```

The entrypoint shim (`docker/gateway_main.sh`) injects three operational
defaults — `--hostname=0.0.0.0`, `--data-dir=/var/lib/bigquery-emulator`, and
`--seed-data-file=<bundled public data>` — but **only when you don't pass that
flag yourself**. So your flags *augment* the defaults instead of replacing them;
pass `--data-dir=...` or `--seed-data-file=...` (or `--seed-yaml=...`) to
override, and the rest of the baked-in defaults stay intact.

> **Coming from `goccy/bigquery-emulator`?** The canonical flag names differ
> (`--project-id`, `--http_port` / `--http-port`, `--seed-data-file` /
> `--seed-yaml`), but the gateway also accepts the goccy spellings as aliases:
> `--project` → `--project-id`, `--port` → `--http-port`, and
> `--data-from-yaml` → `--seed-data-file`. Any *other* unknown flag still makes
> `gateway_main` exit on a parse error, so the container appears to ignore your
> arguments — run
> `docker run --rm ghcr.io/vantaboard/bigquery-emulator:latest --help` to see
> the full flag surface. See [Seeding & CLI flags](./SEEDING.md) for details.

For the published image without building locally, use the [Install via Docker](#install-via-docker)
commands above (`:latest` or a pinned `vX.Y.Z` tag).
