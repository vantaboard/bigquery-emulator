# Rollout checklist (React UI + BigQuery REST)

## Prerequisites

- **`bigquery-emulator`** running with HTTP on **9050** (or your chosen port).
- UI dev server or static build; browser calls **`/bigquery/v2/*`** (Vite proxy or nginx in Docker).

## Smoke tests

1. **Projects**: sidebar lists projects (`GET /bigquery/v2/projects`).
2. **Tree**: expand project → datasets; expand dataset → tables; select a table → metadata loads.
3. **Query**: default SQL appears; **Run query** returns columns/rows; tab switches to **Results**.
4. **Format SQL** runs without throwing on valid BigQuery SQL.
5. **Share** copies a URL; paste in new tab restores selection and query (where encoded in URL).

## Docker

- `docker compose up` exposes the UI on **8080** (nginx) and proxies **`/bigquery`** to **`http://bigquery:9050`** inside the Compose network.
- Automated E2E: `npm run test:e2e` (see [README](../README.md#e2e-testing)).

## Regression notes

- This is a full UI rewrite; behavior is intended to be equivalent for core flows, not line-for-line DOM parity.
