FROM ghcr.io/recidiviz/go-zetasql:0.5.5-recidiviz.3

ARG VERSION

WORKDIR /work

COPY --from=go_zetasql . ./go-zetasql
COPY --from=go_zetasqlite . ./go-zetasqlite
COPY . ./bigquery-emulator

WORKDIR /work/bigquery-emulator

RUN go mod download

RUN make emulator/build

# Since the binary uses dynamic linking we must use the same base image as the build runtime
FROM ghcr.io/recidiviz/go-zetasql:0.5.5-recidiviz.3 AS emulator

COPY --from=0 /work/bigquery-emulator/bigquery-emulator /bin/bigquery-emulator

WORKDIR /work

ENTRYPOINT ["/bin/bigquery-emulator"]
