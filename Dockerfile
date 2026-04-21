# syntax=docker/dockerfile:1.6
#
# Secondary local-only Docker build: use this when you intentionally build from a
# parent directory containing all three sibling repos. CI/release should prefer
# Dockerfile.linked, which matches the stack bootstrap and prebuilt contract directly.
#
# BigQuery emulator needs CGO (go-googlesql) and must be built against the same
# go-googlesql / go-googlesqlite sources as this repo's go.work.dev sibling replaces.
#
# Build from the parent directory that contains all three repos side by side:
#   go-googlesql/
#   go-googlesqlite/
#   bigquery-emulator/
#
#   cd /path/to/parent
#   docker build -f bigquery-emulator/Dockerfile \
#     --ignorefile bigquery-emulator/docker/parent.dockerignore \
#     -t bigquery-emulator:local .

FROM golang:1.26-bookworm AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
		clang=1:14.0-55.7~deb12u1 \
		mold=1.10.1+dfsg-1 \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY go-googlesql /src/go-googlesql
COPY go-googlesqlite /src/go-googlesqlite
COPY bigquery-emulator /src/bigquery-emulator

WORKDIR /src/bigquery-emulator

# Align with go-googlesql Taskfile / scripts/go-googlesql-env.sh (default unified prebuilt path).
ENV CGO_ENABLED=1
ENV CC=clang
ENV CXX=clang++
ENV CGO_CXXFLAGS=-stdlib=libc++
ENV CGO_LDFLAGS_ALLOW="-Wl,--no-gc-sections|-Wl,--allow-multiple-definition|-fuse-ld=mold|-Wl,--whole-archive|-Wl,--no-whole-archive|-Wl,--start-group|-Wl,--end-group|-stdlib=libc\+\+"
# Do not use mold: unified prebuilt CGO passes -l:libcxx_prebuilt.a; mold mis-parses it (see Dockerfile.linked).
ENV CGO_LDFLAGS="-Wl,--no-gc-sections -Wl,--allow-multiple-definition -stdlib=libc++"

RUN --mount=type=cache,target=/go/pkg/mod \
	--mount=type=cache,target=/root/.cache/go-build \
	go build -tags googlesql,googlesql_unified_prebuilt -trimpath -ldflags="-s -w" \
		-o /out/bigquery-emulator \
		./cmd/bigquery-emulator

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
		ca-certificates=20230311+deb12u1 \
		libstdc++6=12.2.0-14+deb12u1 \
	&& rm -rf /var/lib/apt/lists/*

COPY --from=builder /out/bigquery-emulator /usr/local/bin/bigquery-emulator

EXPOSE 9050 9060

ENTRYPOINT ["/usr/local/bin/bigquery-emulator"]

# Create projects at runtime: POST /emulator/v1/projects with body {"id":"<project-id>"}.
CMD ["--host=0.0.0.0", "--port=9050", "--grpc-port=9060", "--log-level=info"]
