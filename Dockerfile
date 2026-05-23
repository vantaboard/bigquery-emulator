# syntax=docker/dockerfile:1.7
#
# Multi-stage build for the BigQuery emulator. Mirrors the
# cloud-spanner-emulator container layout: one image carries both the Go
# REST gateway (`gateway_main`) and the C++ engine (`emulator_main`).
#
# Stage 1 (`go-builder`) compiles the Go gateway with CGO disabled, so
# the resulting binary is statically linked against musl-equivalent
# glibc-static and trivially copyable into a minimal runtime image.
#
# Stage 2 (`cpp-builder`) builds the C++ engine via CMake + clang.
# Today the engine is the placeholder StubServer from
# `frontend/server/server.cc`; once Phase 2 lands gRPC + GoogleSQL,
# this stage will grow build-time deps. The interface stays the same.
#
# Stage 3 is the runtime image: `debian:bookworm-slim` plus libstdc++6
# (for the C++ engine) and ca-certificates. We ship both binaries plus
# the LICENSE file at the image root and a tiny shell shim that defaults
# `--hostname=0.0.0.0` for the gateway, so the published REST port is
# reachable through `docker run -p 9050:9050` without forcing every
# caller to remember the flag.

ARG GO_VERSION=1.26
ARG DEBIAN_VERSION=bookworm

###############################################################################
# Stage 1: Go gateway
###############################################################################
FROM golang:${GO_VERSION}-${DEBIAN_VERSION} AS go-builder

ENV CGO_ENABLED=0 \
    GOFLAGS=-trimpath

WORKDIR /src

# Module graph first so subsequent layers cache cleanly.
COPY go.mod ./
RUN go mod download

COPY binaries ./binaries
COPY gateway ./gateway
COPY proto ./proto

RUN go build \
        -ldflags="-s -w" \
        -o /out/gateway_main \
        ./binaries/gateway_main

###############################################################################
# Stage 2: C++ engine
###############################################################################
FROM debian:${DEBIAN_VERSION}-slim AS cpp-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        clang \
        cmake \
        make \
        ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY CMakeLists.txt ./
COPY backend ./backend
COPY binaries ./binaries
COPY frontend ./frontend
COPY proto ./proto

RUN cmake -S . -B build-out \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
    && cmake --build build-out --target emulator_main -j

###############################################################################
# Stage 3: runtime image
###############################################################################
FROM debian:${DEBIAN_VERSION}-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Real binaries live under /opt; the shim at /gateway_main wraps the
# Go binary so the in-container default binds to 0.0.0.0.
COPY --from=go-builder /out/gateway_main /opt/bigquery-emulator/gateway_main
COPY --from=cpp-builder /src/build-out/emulator_main /opt/bigquery-emulator/emulator_main
COPY LICENSE /LICENSE
COPY docker/gateway_main.sh /gateway_main
COPY docker/gateway_main.sh /opt/bigquery-emulator/gateway_main.sh
RUN ln -s /opt/bigquery-emulator/emulator_main /emulator_main \
    && chmod +x /gateway_main /opt/bigquery-emulator/gateway_main.sh

EXPOSE 9050 9060

CMD ["/gateway_main", \
     "--engine_binary=/emulator_main", \
     "--http_port=9050", \
     "--grpc_port=9060"]
