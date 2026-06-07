# Upstream functional tests (sync on demand)

`*.py` and fixtures under this directory are populated by:

```bash
./scripts/sync_dbt_bigquery_tests.sh
```

Do not commit synced sources; only curated emulator wiring lives in the parent directory.
