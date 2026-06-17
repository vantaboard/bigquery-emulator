# syntax=docker/dockerfile:1.7
#
# Multi-stage build for the BigQuery emulator. Mirrors the
# cloud-spanner-emulator container layout: one image carries both the Go
# REST gateway (`gateway_main`) and the C++ engine (`emulator_main`).
#
# Stage 1 (`engine-builder`) builds the canonical Bazel `cc_binary`
# `//binaries/emulator_main:emulator_main`, which links the full
# GoogleSQL analyzer + DuckDB engine + DuckDB storage +
# gRPC. GoogleSQL is consumed via `MODULE.bazel`'s `local_path_override`
# at sibling path `../googlesql`, so the stage shallow-clones the
# tagged upstream tree at `/googlesql` and applies the same one-line
# semver patch the conformance / ci workflows apply (`MODULE.bazel`'s
# top-of-file note explains why). The resulting binary + its dynamic
# `libduckdb.so` are staged into `/out/` for the runtime stage.
#
# Stage 2 (`gateway-builder`) compiles the Go gateway with
# `CGO_ENABLED=0` so the binary is freely copyable into the runtime
# image. Today the gateway is a thin REST shim around the engine's gRPC
# surface; the binary is small (<30 MB) and the build is short.
#
# Stage 3 is the runtime image: `ubuntu:24.04` plus libstdc++6
# (the engine `cc_binary` link-options statically link libgcc but
# cannot fully avoid libstdc++.so.6 once GoogleSQL's transitive deps
# are linked in) and `wget` (used by the HEALTHCHECK directive). Ubuntu
# 24.04 ships GLIBC 2.39, which matches the
# `github-hosted ubuntu-latest` runner so an engine binary built on
# the host (via `ENGINE_SOURCE=prebuilt`) loads cleanly — bookworm's
# GLIBC 2.36 is too old. The
# engine binary and its sibling `libduckdb.so` live under
# `/opt/bigquery-emulator/`; the binary's `-Wl,-rpath,$ORIGIN`
# linkopt finds the `.so` next to it without `LD_LIBRARY_PATH`. A
# small shell shim at `/usr/local/bin/bigquery-emulator-gateway`
# defaults `--hostname=0.0.0.0` so the published REST port is
# reachable through `docker run -p 9050:9050` without forcing the
# caller to remember the flag.
#
# Bazel inside Docker:
#   * Pinned to `/usr/bin/clang` per `.cursor/rules/bazel-process-hygiene.mdc`.
#   * Throttled by `task emulator:build-engine:bazel`'s auto-detect:
#     jobs = nproc - 2 (min 2), memory = ~75% of MemTotal MB (min 4096).
#     Override per-build with `--build-arg BAZEL_JOBS=N` /
#     `--build-arg BAZEL_MEM_MB=N` if a runner needs a different shape.
#   * BuildKit cache mount on `/root/.cache/bazel` keeps warm rebuilds
#     under a couple of minutes; cold rebuilds dominate on a clean cache.

ARG GO_VERSION=1.26
ARG DEBIAN_VERSION=bookworm
# GoogleSQL upstream tag (used by the source fallback path). Must
# match the version pinned in MODULE.bazel. The patched value works
# around gazelle's strict semver parser; see the top-of-file note in
# MODULE.bazel and the same patch applied in `.github/workflows/ci.yml`
# and `.github/workflows/conformance.yml`. With the
# `GOOGLESQL_PREBUILT_URL` build arg set, this is unused.
ARG GOOGLESQL_VERSION=2026.01.1
ARG GOOGLESQL_VERSION_PATCHED=2026.1.1
ARG BAZELISK_VERSION=v1.27.0

# Engine source selector. Two valid values:
#   * `bazel` (default): run the canonical `cc_binary` build inside the
#     image. By default this consumes a published GoogleSQL prebuilt
#     artifact via the `GOOGLESQL_PREBUILT_URL` /
#     `GOOGLESQL_PREBUILT_SHA256` build args; if those are unset, falls
#     back to a sibling `google/googlesql` clone (slow first cut,
#     ~25-55 min cold cache). What `docker build .` does without any
#     extra setup.
#   * `prebuilt`: skip the in-container Bazel build and COPY the engine
#     binary + libduckdb.so from the host build context (under `bin/`).
#     Used by `.github/workflows/release.yml` so the release runner does
#     not rebuild the engine twice (once on the host, once inside the
#     container) and blow past the job timeout. `bin/emulator_main` +
#     `bin/libduckdb.so` must be staged before `docker build` runs;
#     `task emulator:build-engine:bazel` produces both.
ARG ENGINE_SOURCE=bazel

# CI/Docker/release prebuilt-mode wiring: when both args are set,
# the `engine-builder-bazel` stage downloads the published GoogleSQL
# prebuilt artifact, verifies its SHA-256, and unpacks it into the
# `.cache/googlesql-prebuilt/` cache the `--config=googlesql-prebuilt`
# group reads from (see `.bazelrc`). This skips the multi-GB GoogleSQL
# source compile inside the container.
#
# When either arg is empty, the stage falls back to the legacy sibling
# `google/googlesql` clone + `--config=googlesql-source` build. The
# fallback exists so `docker build .` keeps working before maintainers
# wire the prebuilt URL/SHA into their build invocations or CI vars.
ARG GOOGLESQL_PREBUILT_URL=""
ARG GOOGLESQL_PREBUILT_SHA256=""

###############################################################################
# Stage 1a: C++ engine (canonical Bazel build, full GoogleSQL + DuckDB)
#
# Selected when ENGINE_SOURCE=bazel (the default). Self-contained: no
# pre-staged host binaries required.
###############################################################################
FROM debian:${DEBIAN_VERSION}-slim AS engine-builder-bazel

# Stage-wide SHELL switch to bash with `pipefail`. The very next RUN
# pipes `curl ... | gpg --dearmor` to register the LLVM apt key; without
# `pipefail`, dash would only report `gpg`'s exit status and a curl
# failure (404, network blip, MITM rejection by `--fail`) would silently
# install an empty/garbage keyring. The later RUN at the GoogleSQL
# fetch step also needs bash for `<(...)` process substitution, so this
# directive doubles as that stage's shell baseline.
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Build deps: clang-18 (from apt.llvm.org) + libstdc++ headers + JDK
# (Bazel needs Java) + git/curl/unzip for fetching bazelisk and the
# googlesql sibling. Python 3 + zip are required by some of the Bazel
# rule sets the GoogleSQL build pulls in; pkg-config keeps the
# configure step quiet.
#
# Why clang-18 specifically: GoogleSQL's
# `googlesql/common/match_recognize/nfa_builder.h` instantiates
# `std::function<void(...)>` in a way that trips a
# clang-14 + libstdc++-12 bug on `_Decay_t<void&>` substitution
# (reproducible on debian:bookworm-slim's default `clang` package).
# The project's bazel-process-hygiene rule pins `/usr/bin/clang` ->
# `clang-18`, so we mirror that pin inside the build stage. The
# runtime stage stays on bookworm-slim with libstdc++6 (gcc-12), which
# matches the libstdc++ headers used at build time.
#
# DL3008 (pin versions in apt-get install) is intentionally ignored:
# debian:bookworm-slim is a rolling base whose package versions rotate
# with security updates, and old versions are quickly purged from the
# Debian mirrors. Pinning exact versions would make this layer break
# nondeterministically on rebuild. The base image tag itself is the
# version pin.
# hadolint ignore=DL3008
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        g++ \
        git \
        gnupg \
        libstdc++-12-dev \
        libssl-dev \
        openjdk-17-jdk-headless \
        pkg-config \
        python3 \
        unzip \
        zip \
    && curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key \
        | gpg --dearmor -o /usr/share/keyrings/llvm.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] http://apt.llvm.org/bookworm/ llvm-toolchain-bookworm-18 main" \
        > /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends clang-18 \
    && ln -sf /usr/bin/clang-18 /usr/bin/clang \
    && ln -sf /usr/bin/clang++-18 /usr/bin/clang++ \
    && rm -rf /var/lib/apt/lists/*

ARG BAZELISK_VERSION
RUN curl -fsSL -o /usr/local/bin/bazel \
        "https://github.com/bazelbuild/bazelisk/releases/download/${BAZELISK_VERSION}/bazelisk-linux-amd64" \
    && chmod +x /usr/local/bin/bazel

WORKDIR /src

# Bazel inputs only: the stage skips `gateway/`, `docs/`, etc. so a
# touched-Go-only change does not invalidate this expensive layer.
COPY .bazelrc .bazelversion BUILD.bazel googlesql_deps.bzl MODULE.bazel MODULE.bazel.lock ./
COPY backend ./backend
COPY binaries/emulator_main ./binaries/emulator_main
COPY frontend ./frontend
COPY proto ./proto
COPY third_party ./third_party

# Centralized safety-gate validator. Copied in BEFORE the GoogleSQL
# fetch step so the in-container fetch can run the same
# Schema/identity/payload gates the host-side
# `task googlesql:fetch-prebuilt` runs. Touching any of these tools
# busts only the validate step's layer — the (expensive) bazel build
# layer below stays cached.
COPY tools/googlesql-prebuilt ./tools/googlesql-prebuilt
COPY scripts/patch_googlesql_source_checkout.sh ./scripts/patch_googlesql_source_checkout.sh

# Resolve a GoogleSQL build mode (the CI/Docker/release track of the
# `googlesql-prebuilt` rollout — see
# `docs/dev/googlesql-prebuilt/README.md`):
#
#   * If `GOOGLESQL_PREBUILT_URL` + `GOOGLESQL_PREBUILT_SHA256` are
#     set, fetch + verify (SHA gate) + unpack into
#     `/src/.cache/googlesql-prebuilt/googlesql_prebuilt_linux_amd64/`,
#     where `--override_module=googlesql=.cache/googlesql-prebuilt/...`
#     from `.bazelrc` resolves under `--config=googlesql-prebuilt`
#     (the path is workspace-root-relative; Bazel resolves it against
#     the workspace root at invocation time).
#   * Otherwise clone `google/googlesql@${GOOGLESQL_VERSION}` at
#     `/googlesql` (so `MODULE.bazel`'s
#     `local_path_override(path = "../googlesql")` resolves) and apply
#     the gazelle leading-zero patch.
#
# The fetch path mirrors the SHA-gate logic in
# `taskfiles/googlesql.yml::fetch-prebuilt` so Bazel only sees the
# cache after the checksum + module-name + manifest gates have all
# passed.
ARG GOOGLESQL_VERSION
ARG GOOGLESQL_VERSION_PATCHED
ARG GOOGLESQL_PREBUILT_URL
ARG GOOGLESQL_PREBUILT_SHA256
# Bash + pipefail is already set at the top of this stage (see the
# stage-level SHELL directive). The fetch RUN below relies on both:
# `<(...)` process substitution for the tarball-entry validation loop,
# and pipefail for the `sha256sum | awk` and `grep | head | sed`
# pipelines.
RUN set -eux; \
    if [ -n "${GOOGLESQL_PREBUILT_URL}" ] && [ -n "${GOOGLESQL_PREBUILT_SHA256}" ]; then \
        if ! [[ "${GOOGLESQL_PREBUILT_SHA256}" =~ ^[0-9a-f]{64}$ ]]; then \
            echo "GOOGLESQL_PREBUILT_SHA256 must be 64 lowercase hex chars; got '${GOOGLESQL_PREBUILT_SHA256}'" >&2; \
            exit 2; \
        fi; \
        cache_root=/src/.cache/googlesql-prebuilt; \
        mkdir -p "$cache_root"; \
        tmp="$(mktemp -d -p "$cache_root" .fetch-XXXXXX)"; \
        tarball="$tmp/artifact.tar.gz"; \
        echo "fetching ${GOOGLESQL_PREBUILT_URL} -> $tarball"; \
        curl --fail --location --show-error --silent \
            --output "$tarball" "${GOOGLESQL_PREBUILT_URL}"; \
        actual_sha="$(sha256sum "$tarball" | awk '{print $1}')"; \
        if [ "$actual_sha" != "${GOOGLESQL_PREBUILT_SHA256}" ]; then \
            echo "GoogleSQL prebuilt SHA mismatch (expected ${GOOGLESQL_PREBUILT_SHA256}, got $actual_sha)" >&2; \
            rm -rf "$tmp"; \
            exit 1; \
        fi; \
        while IFS= read -r path; do \
            case "$path" in \
              /*|*..*) echo "tarball path-escape entry: $path" >&2; exit 1 ;; \
            esac; \
        done < <(tar -tzf "$tarball"); \
        unpack="$tmp/unpack"; \
        mkdir -p "$unpack"; \
        tar -xzf "$tarball" -C "$unpack"; \
        if [ ! -d "$unpack/googlesql_prebuilt_linux_amd64" ]; then \
            echo "tarball missing top-level googlesql_prebuilt_linux_amd64/" >&2; \
            ls -la "$unpack" >&2; \
            exit 1; \
        fi; \
        mod_name="$(grep -E '^[[:space:]]*name = ' "$unpack/googlesql_prebuilt_linux_amd64/MODULE.bazel" | head -1 | sed -E 's/.*"([^"]+)".*/\1/')"; \
        if [ "$mod_name" != "googlesql" ]; then \
            echo "prebuilt MODULE.bazel declares module(name = \"$mod_name\"); expected \"googlesql\"" >&2; \
            exit 1; \
        fi; \
        rm -rf "$cache_root/googlesql_prebuilt_linux_amd64"; \
        mv "$unpack/googlesql_prebuilt_linux_amd64" "$cache_root/googlesql_prebuilt_linux_amd64"; \
        echo "${GOOGLESQL_PREBUILT_SHA256}  artifact.tar.gz" > "$cache_root/googlesql_prebuilt_linux_amd64.tarball.sha256"; \
        rm -rf "$tmp"; \
        # Centralized safety-gate validator — schema, identity,
        # platform, payload SHA, wrapper-presence gates. Anything
        # failing here is a hard error: the Dockerfile refuses to
        # proceed to the bazel build with a partially-valid cache.
        python3 tools/googlesql-prebuilt/validate_artifact.py \
            --repo-root "$cache_root/googlesql_prebuilt_linux_amd64" \
            --summary-line \
            --summary-json /tmp/googlesql-validate.json; \
        artifact_version="$(python3 -c 'import json; print(json.load(open("'"$cache_root"'/googlesql_prebuilt_linux_amd64/manifest.json")).get("artifact_version","?"))' 2>/dev/null || echo '?')"; \
        gs_commit="$(python3 -c 'import json; print(json.load(open("'"$cache_root"'/googlesql_prebuilt_linux_amd64/manifest.json")).get("googlesql",{}).get("commit","?")[:12])' 2>/dev/null || echo '?')"; \
        echo "GoogleSQL prebuilt artifact_version=$artifact_version googlesql=$gs_commit"; \
        echo prebuilt > /tmp/googlesql-mode; \
    else \
        echo "GOOGLESQL_PREBUILT_URL/_SHA256 not set; falling back to source GoogleSQL build" >&2; \
        git clone --depth=1 --branch="${GOOGLESQL_VERSION}" \
            https://github.com/google/googlesql.git /googlesql; \
        bash scripts/patch_googlesql_source_checkout.sh /googlesql; \
        echo source > /tmp/googlesql-mode; \
    fi

# Auto-detect throttling matches taskfiles/bazel.yml + emulator.yml so a
# constrained CI runner picks the same shape as a local invocation. Per
# `.cursor/rules/bazel-process-hygiene.mdc`, do NOT lower these; let the
# runner's nproc / MemTotal drive the picks.
ARG BAZEL_JOBS=
ARG BAZEL_MEM_MB=

# Build with a BuildKit cache mount on Bazel's user-root so subsequent
# `docker build` invocations skip re-downloading and re-compiling
# GoogleSQL. The first cold build dominates: figure ~25-55 min in
# source mode; the prebuilt path drops that to ~5-10 min on a cold
# tree because GoogleSQL's archive is already linked.
RUN --mount=type=cache,target=/root/.cache/bazel,id=bigquery-emulator-bazel-clang18 \
    set -eux; \
    jobs="${BAZEL_JOBS}"; \
    if [ -z "$jobs" ]; then \
        n=$(nproc 2>/dev/null || echo 4); \
        if [ "$n" -gt 4 ]; then jobs=$((n - 2)); else jobs=2; fi; \
    fi; \
    mem="${BAZEL_MEM_MB}"; \
    if [ -z "$mem" ] && [ -r /proc/meminfo ]; then \
        mem=$(awk '/^MemTotal:/ { mb = int($2 * 3 / 4 / 1024); if (mb < 4096) mb = 4096; print mb }' /proc/meminfo); \
    fi; \
    : "${mem:=8192}"; \
    mode="$(cat /tmp/googlesql-mode)"; \
    case "$mode" in \
      prebuilt) gsq_cfg=googlesql-prebuilt ;; \
      source)   gsq_cfg=googlesql-source ;; \
      *)        echo "unknown googlesql mode '$mode'" >&2; exit 1 ;; \
    esac; \
    echo "engine-builder-bazel: --config=$gsq_cfg (jobs=$jobs mem=${mem})"; \
    CC=/usr/bin/clang CXX=/usr/bin/clang++ PATH=/usr/bin:$PATH \
        bazel build \
            --config="$gsq_cfg" \
            --jobs="${jobs}" \
            --local_resources=cpu="${jobs}" \
            --local_resources=memory="${mem}" \
            --action_env=CC=/usr/bin/clang \
            --action_env=CXX=/usr/bin/clang++ \
            //binaries/emulator_main:emulator_main; \
    mkdir -p /out; \
    cp -L bazel-bin/binaries/emulator_main/emulator_main /out/emulator_main; \
    chmod +w /out/emulator_main; \
    duckdb_so="$(find -L bazel-bin/binaries/emulator_main/emulator_main.runfiles -name libduckdb.so -print -quit)"; \
    if [ -z "$duckdb_so" ]; then \
        echo 'engine-builder: libduckdb.so not found in runfiles' >&2; \
        exit 1; \
    fi; \
    cp -L "$duckdb_so" /out/libduckdb.so; \
    bazel shutdown || true

###############################################################################
# Stage 1b: Pre-built engine pass-through
#
# Selected when ENGINE_SOURCE=prebuilt. Skips the full Bazel toolchain
# install + compile and simply lifts `bin/emulator_main` +
# `bin/libduckdb.so` from the build context into `/out/` so the runtime
# stage's `COPY --from=engine-builder` keeps working unchanged. The
# release workflow stages these files via
# `task emulator:build-engine:bazel` on the host before invoking
# `docker build`; .dockerignore allows the two specific files through.
###############################################################################
FROM debian:${DEBIAN_VERSION}-slim AS engine-builder-prebuilt

WORKDIR /out
COPY bin/emulator_main /out/emulator_main
COPY bin/libduckdb.so /out/libduckdb.so

###############################################################################
# Stage 1: engine-builder selector
#
# `FROM ${ENGINE_SOURCE}` chooses between the bazel and prebuilt stages
# above. The runtime stage's `COPY --from=engine-builder` indirection
# isolates the rest of the Dockerfile from the choice.
# `engine-builder-${ENGINE_SOURCE}` resolves to a sibling build stage
# (`engine-builder-bazel` or `engine-builder-prebuilt`), not an external
# image, so DL3006's tag-pinning advice does not apply; hadolint cannot
# statically resolve the ARG substitution and treats it as a remote ref.
###############################################################################
# hadolint ignore=DL3006
FROM engine-builder-${ENGINE_SOURCE} AS engine-builder

###############################################################################
# Stage 2: Go gateway
###############################################################################
FROM golang:${GO_VERSION}-${DEBIAN_VERSION} AS gateway-builder

ENV CGO_ENABLED=0 \
    GOFLAGS=-trimpath

WORKDIR /src

COPY go.mod go.sum ./
RUN --mount=type=cache,target=/root/.cache/go-build \
    --mount=type=cache,target=/go/pkg/mod \
    go mod download

COPY binaries ./binaries
COPY gateway ./gateway
COPY proto ./proto

RUN --mount=type=cache,target=/root/.cache/go-build \
    --mount=type=cache,target=/go/pkg/mod \
    go build -ldflags='-s -w' -o /out/gateway_main ./binaries/gateway_main

###############################################################################
# Stage 3: runtime image
#
# `ubuntu:24.04` is pinned (not `${DEBIAN_VERSION}-slim`) because
# `ENGINE_SOURCE=prebuilt` lifts a binary built on the GitHub
# `ubuntu-latest` runner (Ubuntu 24.04, GLIBC 2.39); a Debian
# bookworm runtime (GLIBC 2.36) refuses to load it. Ubuntu 24.04
# matches the build host and stays GLIBC-compatible with binaries
# built in the `engine-builder-bazel` stage too (which targets
# bookworm's 2.36 — forward-compatible on 2.39).
###############################################################################
FROM ubuntu:24.04 AS runtime

# Image labels are filled at build time. `VERSION` defaults to `dev`
# locally and is overridden by the release pipeline
# (`.github/workflows/release.yml`).
ARG VERSION=dev
ARG SOURCE_REPO=https://github.com/vantaboard/bigquery-emulator

LABEL org.opencontainers.image.source="${SOURCE_REPO}" \
      org.opencontainers.image.licenses="MIT" \
      org.opencontainers.image.version="${VERSION}" \
      org.opencontainers.image.title="bigquery-emulator" \
      org.opencontainers.image.description="Locally-runnable BigQuery REST API emulator (Go gateway + GoogleSQL/DuckDB engine)"

# Runtime deps:
#   * libstdc++6 — the engine cc_binary statically links libgcc and
#     libstdc++ where it can, but GoogleSQL's transitive deps still
#     pull in libstdc++.so.6.
#   * tzdata — GoogleSQL's analyzer hardcodes `America/Los_Angeles` as
#     the default_timezone and resolves it via absl's
#     `FindTimeZoneByName` at engine init. Without IANA zoneinfo
#     installed, the lookup fails OUT_OF_RANGE and the absl CHECK
#     aborts the engine on its first query ("Did you need to install
#     the tzdata package?"). Pin TZ=Etc/UTC below so the container's
#     own clock stays deterministic regardless of the host.
#   * wget — used by the HEALTHCHECK directive below.
#   * ca-certificates — keeps the gateway HTTP client honest if the
#     image is repurposed to talk to upstream BigQuery in the future.
#
# hadolint DL3008 (pin apt versions): intentionally unpinned — these
# are core Ubuntu system libraries (libstdc++, tzdata, wget,
# ca-certificates) on the version-pinned `ubuntu:24.04` base. Ubuntu's
# security mirror rotates exact versions in-place, so a hard pin like
# `libstdc++6=14.2.0-...` breaks the build the next time a security
# update rolls out. The `ubuntu:24.04` tag already provides
# layer-level reproducibility for the runtime stage.
# hadolint ignore=DL3008
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libssl3t64 \
        libstdc++6 \
        python3 \
        python3-lxml \
        tzdata \
        wget \
    && rm -rf /var/lib/apt/lists/*
ENV TZ=Etc/UTC

# Engine + libduckdb.so co-located so `-Wl,-rpath,$ORIGIN` finds the
# dynamic dep without `LD_LIBRARY_PATH`. The Go gateway's default
# `--engine_binary=emulator_main` resolves the engine relative to the
# gateway binary's own directory (see `binaries/gateway_main/main.go`
# `resolveEngineBinary`), so installing both side-by-side under
# `/opt/bigquery-emulator/` keeps the default working without forcing
# the caller to pass an explicit `--engine_binary` flag.
COPY --from=engine-builder /out/emulator_main /opt/bigquery-emulator/emulator_main
COPY --from=engine-builder /out/libduckdb.so /opt/bigquery-emulator/libduckdb.so
COPY --from=gateway-builder /out/gateway_main /opt/bigquery-emulator/gateway_main
COPY LICENSE /LICENSE

# Shim defaults `--hostname=0.0.0.0` so the published REST port is
# reachable from outside the container. Caller-supplied `--hostname`
# wins.
COPY docker/gateway_main.sh /usr/local/bin/bigquery-emulator-gateway
RUN chmod +x /usr/local/bin/bigquery-emulator-gateway

# Bundled public-dataset seed for thirdparty sample tests.
COPY testdata/public-data/ /opt/bigquery-emulator/testdata/public-data/

# Persistent data dir consumed by the DuckDB storage backend (now the
# only backend). The emulator opens a `catalog.duckdb` file under this
# directory and persists the catalog + table rows across boots.
# WORKDIR + VOLUME guarantee the data survives `docker run` cycles
# when the operator mounts the volume.
WORKDIR /var/lib/bigquery-emulator
VOLUME ["/var/lib/bigquery-emulator"]

# `9050` is the REST gateway (per docs/REST_API.md and the
# `--http_port` default in `binaries/gateway_main/main.go`); `9060` is
# the internal engine gRPC port (`--grpc_port`), exposed for debugging.
EXPOSE 9050 9060

# `/healthz` is wired by `gateway/server.go` to `handlers.Health`,
# which returns 200 + `{"status":"ok",...}`. Wait 10s for the engine
# subprocess to flip to SERVING (see `engineReadyTimeout` in
# `gateway/gateway.go`).
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD wget -q -O - http://localhost:9050/healthz >/dev/null 2>&1 || exit 1

ENTRYPOINT ["/usr/local/bin/bigquery-emulator-gateway"]
# Operational defaults live in the entrypoint shim
# (`docker/gateway_main.sh`), NOT here. Docker REPLACES the whole `CMD`
# the moment a caller passes any argument to `docker run`, so baking
# `--data-dir` / `--seed-data-file` into `CMD` meant a single user flag
# (e.g. `--project-id=foo`) silently dropped the persistent data dir and
# the bundled seed — looking like "CLI args aren't respected". The shim
# now injects `--hostname=0.0.0.0`, `--data-dir`, and `--seed-data-file`
# only when the caller did not already supply them, so user flags
# augment the defaults instead of wiping them. The remaining engine
# defaults (`--http_port=9050`, `--grpc_port=9060`, `--copy_engine_stderr`)
# already match gateway_main's built-in defaults (see `defaultConfig` in
# binaries/gateway_main/cli.go), so an empty CMD reproduces the prior
# behavior while staying override-safe.
CMD []
