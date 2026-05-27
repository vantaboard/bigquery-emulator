# syntax=docker/dockerfile:1.7
#
# Multi-stage build for the BigQuery emulator. Mirrors the
# cloud-spanner-emulator container layout: one image carries both the Go
# REST gateway (`gateway_main`) and the C++ engine (`emulator_main`).
#
# Stage 1 (`engine-builder`) builds the canonical Bazel `cc_binary`
# `//binaries/emulator_main:emulator_main`, which links the full
# GoogleSQL analyzer + reference-impl evaluator + DuckDB storage +
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
# GoogleSQL upstream tag. Must match the version pinned in MODULE.bazel.
# The patched value works around gazelle's strict semver parser; see the
# top-of-file note in MODULE.bazel and the same patch applied in
# `.github/workflows/ci.yml` and `.github/workflows/conformance.yml`.
ARG GOOGLESQL_VERSION=2026.01.1
ARG GOOGLESQL_VERSION_PATCHED=2026.1.1
ARG BAZELISK_VERSION=v1.27.0

# Engine source selector. Two valid values:
#   * `bazel` (default): run the canonical `cc_binary` build inside the
#     image; self-contained, slow first cut (~25-55 min cold cache).
#     What `docker build .` does without any extra setup.
#   * `prebuilt`: skip the in-container Bazel build and COPY the engine
#     binary + libduckdb.so from the host build context (under `bin/`).
#     Used by `.github/workflows/release.yml` so the release runner does
#     not rebuild the engine twice (once on the host, once inside the
#     container) and blow past the job timeout. `bin/emulator_main` +
#     `bin/libduckdb.so` must be staged before `docker build` runs;
#     `task emulator:build-engine:bazel` produces both.
ARG ENGINE_SOURCE=bazel

###############################################################################
# Stage 1a: C++ engine (canonical Bazel build, full GoogleSQL + DuckDB)
#
# Selected when ENGINE_SOURCE=bazel (the default). Self-contained: no
# pre-staged host binaries required.
###############################################################################
FROM debian:${DEBIAN_VERSION}-slim AS engine-builder-bazel

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
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        g++ \
        git \
        gnupg \
        libstdc++-12-dev \
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

# Stage googlesql sibling at /googlesql so MODULE.bazel's
# `local_path_override(path = "../googlesql")` resolves cleanly when
# the workspace lives at /src.
ARG GOOGLESQL_VERSION
ARG GOOGLESQL_VERSION_PATCHED
RUN git clone --depth=1 --branch=${GOOGLESQL_VERSION} \
        https://github.com/google/googlesql.git /googlesql \
    && sed -i "s/version = \"${GOOGLESQL_VERSION}\"/version = \"${GOOGLESQL_VERSION_PATCHED}\"/" \
        /googlesql/MODULE.bazel

WORKDIR /src

# Bazel inputs only: the stage skips `gateway/`, `docs/`, etc. so a
# touched-Go-only change does not invalidate this expensive layer.
COPY .bazelrc .bazelversion BUILD.bazel MODULE.bazel MODULE.bazel.lock ./
COPY backend ./backend
COPY binaries/emulator_main ./binaries/emulator_main
COPY frontend ./frontend
COPY proto ./proto
COPY third_party ./third_party

# Auto-detect throttling matches taskfiles/bazel.yml + emulator.yml so a
# constrained CI runner picks the same shape as a local invocation. Per
# `.cursor/rules/bazel-process-hygiene.mdc`, do NOT lower these; let the
# runner's nproc / MemTotal drive the picks.
ARG BAZEL_JOBS=
ARG BAZEL_MEM_MB=

# Build with a BuildKit cache mount on Bazel's user-root so subsequent
# `docker build` invocations skip re-downloading and re-compiling
# GoogleSQL. The first cold build dominates: figure ~25-55 min depending
# on the runner; warm rebuilds typically land under two minutes.
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
    CC=/usr/bin/clang CXX=/usr/bin/clang++ PATH=/usr/bin:$PATH \
        bazel build \
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
###############################################################################
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
# locally and is overridden by the release pipeline (plan 44).
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
#   * wget — used by the HEALTHCHECK directive below.
#   * ca-certificates — keeps the gateway HTTP client honest if the
#     image is repurposed to talk to upstream BigQuery in the future.
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libstdc++6 \
        wget \
    && rm -rf /var/lib/apt/lists/*

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

# Persistent data dir consumed by `--storage=duckdb` (the default is
# `--storage=memory`, so this directory only matters when the operator
# explicitly opts into the persistent storage backend). WORKDIR + VOLUME
# guarantee the data persists across `docker run` cycles when the
# operator mounts the volume.
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
# Default profile: gateway + engine (the engine defaults to
# `--profile=ci` shape: reference_impl + memory). `--copy_engine_stderr`
# surfaces engine logs on the container's stderr so a `docker logs`
# diagnoses the engine subprocess too.
CMD ["--http_port=9050", "--grpc_port=9060", "--copy_engine_stderr"]
