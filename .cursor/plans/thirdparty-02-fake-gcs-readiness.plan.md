---
name: fake-gcs readiness wait
overview: "Add a `wait_for_healthz` poll to `fake-gcs-up:` in `taskfiles/thirdparty.yml` so node-bigquery-tests and python-bigquery-dataframes-tests stop racing the docker-proxy listener. Reuses `scripts/wait_for_healthz.sh` against a stable 2xx URL exposed by fake-gcs-server."
todos:
  - id: tp02_edit
    content: "After `docker compose --profile thirdparty up -d fake-gcs-server`, add a wait_for_healthz call against http://localhost:${FAKE_GCS_PORT:-4443}/storage/v1/b/cloud-samples-data."
    status: pending
  - id: tp02_verify
    content: "Run node-bigquery-tests preflight (and dataframes) twice in a row from a cold compose volume; both should stop emitting curl (56) `Recv failure: Connection reset by peer`."
    status: pending
  - id: tp02_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp02` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 02 — `fake-gcs-up` readiness wait

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 1.2 (now deleted).
- Failing log lines: `.logs/thirdparty-20260602-134739.log:2932-2958`
  — `Container ... fake-gcs-server-1 Started` followed immediately by
  `preflight: HTTP request to http://127.0.0.1:4443/... failed (curl rc=56).
  curl: (56) Recv failure: Connection reset by peer`. Same
  docker-proxy race already documented for the emulator gateway in
  [`scripts/wait_for_healthz.sh`](../../scripts/wait_for_healthz.sh).

## Prerequisites

Plan [`thirdparty-01-dataframes-env-var.plan.md`](./thirdparty-01-dataframes-env-var.plan.md)
recommended but not required; both can land in either order.

## Scope

`fake-gcs-up` (see [`taskfiles/thirdparty.yml:283`](../../taskfiles/thirdparty.yml))
currently brings up the container and returns. The docker-proxy that
listens on `4443` is not yet established when the next dependent task
fires its preflight curl, so the very first request sees the listener
reset the connection.

## Implementation

In the `fake-gcs-up:` recipe, after the existing
`docker compose --profile thirdparty up -d fake-gcs-server`, add:

```bash
port="${FAKE_GCS_PORT:-4443}"
bash "${root}/scripts/wait_for_healthz.sh" \
  "http://localhost:${port}/storage/v1/b/cloud-samples-data" 60 fake-gcs-server
```

Pick a URL that returns 2xx once the bind-mounted bucket is loaded:
`/storage/v1/b/cloud-samples-data` returns 200 because the
`cloud-samples-data` bucket is preloaded from `testdata/fake-gcs/`
(verify against the live container before committing). Any other
stable 2xx URL on the configured port is fine if the bucket path
changes.

## Tests

- `task thirdparty:node-bigquery-tests` and
  `task thirdparty:python-bigquery-dataframes-tests` both stop
  emitting the curl (56) reset on a fresh `THIRDPARTY_FRESH_VOLUME=1`
  run.
- The added healthz call costs at most one round-trip past readiness
  (the script sleeps + retries until 200).
- Repeat the run twice back-to-back without `THIRDPARTY_FRESH_VOLUME=1`
  to confirm the healthz call is idempotent and does not regress the
  warm-volume happy path.

## Done criteria

- `fake-gcs-up:` invokes `wait_for_healthz.sh` after `docker compose`.
- A cold-volume run of node-bigquery-tests and dataframes preflight
  no longer races the proxy.
- `thirdparty-00-completion-index.plan.md` todo `tp02` flipped to
  `completed`.
