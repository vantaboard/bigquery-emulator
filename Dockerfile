# syntax=docker/dockerfile:1.7
# Override when validating a local toolchain, e.g.:
#   docker build --build-arg GO_ZETASQL_BASE=go-zetasql:dev -t bigquery-emulator .
# (build ../go-zetasql with tag go-zetasql:dev first — see that repo's Makefile.)
ARG GO_ZETASQL_BASE=ghcr.io/recidiviz/go-zetasql:0.5.5-recidiviz.3
FROM ${GO_ZETASQL_BASE} AS build

ARG VERSION

WORKDIR /work/bigquery-emulator

COPY go.mod go.sum ./

RUN --mount=type=cache,target=/go/pkg/mod \
    --mount=type=cache,target=/root/.cache/go-build \
    go mod download

COPY . ./

RUN --mount=type=cache,target=/go/pkg/mod \
    --mount=type=cache,target=/root/.cache/go-build \
    make emulator/build

# Since the binary uses dynamic linking we must use the same base image as the build runtime
ARG GO_ZETASQL_BASE=ghcr.io/recidiviz/go-zetasql:0.5.5-recidiviz.3
FROM ${GO_ZETASQL_BASE} AS emulator

COPY --from=build /work/bigquery-emulator/bigquery-emulator /bin/bigquery-emulator

WORKDIR /work

ENTRYPOINT ["/bin/bigquery-emulator"]
