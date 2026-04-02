# syntax=docker/dockerfile:1.7
FROM ghcr.io/recidiviz/go-zetasql:0.5.5-recidiviz.3 AS build

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
FROM ghcr.io/recidiviz/go-zetasql:0.5.5-recidiviz.3 AS emulator

COPY --from=build /work/bigquery-emulator/bigquery-emulator /bin/bigquery-emulator

WORKDIR /work

ENTRYPOINT ["/bin/bigquery-emulator"]
