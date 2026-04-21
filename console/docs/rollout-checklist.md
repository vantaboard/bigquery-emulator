# Rollout checklist (React UI + `bq-explorer-api`)

## Prerequisites

- BigQuery emulator reachable from the API process (`BIGQUERY_EMULATOR_HOST`, e.g. `localhost:9050` or `bigquery:9050` in Compose).
- Explorer API running (`bq-explorer-api` from the `bigquery-emulator` repo, default port **8000**).
- UI dev server or static build; browser calls **`/api/*`** (proxied in dev, or nginx in Docker).

## Smoke tests

1. **Projects**: sidebar lists projects (`GET /api/projects`).
2. **Tree**: expand project → datasets; expand dataset → tables; select a table → metadata loads.
3. **Query**: default SQL appears; **Run query** returns columns/rows; tab switches to **Results**.
4. **Format SQL** runs without throwing on valid BigQuery SQL.
5. **Share** copies a URL; paste in new tab restores selection and query (where encoded in URL).
6. **Optional admin**: with `ALLOW_EMULATOR_PROJECT_ADMIN=true` and emulator host, **Add emulator project** succeeds and list refreshes.

## Docker

- `docker compose up` exposes the UI on **8080** (nginx) and proxies **`/api`** to **`host.docker.internal:8000`** (run **`bq-explorer-api`** on the host; see the UI README).
- The BigQuery emulator container publishes **9050** to the host so the API can use **`BIGQUERY_EMULATOR_HOST=localhost:9050`**.

## Regression notes

- This is a full UI rewrite; behavior is intended to be equivalent for core flows, not line-for-line DOM parity.
