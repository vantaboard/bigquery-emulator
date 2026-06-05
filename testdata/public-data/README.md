# Public dataset seed fixtures

Minimal `bigquery-public-data` catalog tables for thirdparty client sample
tests. The gateway loads [`bigquery-public-data.yaml`](bigquery-public-data.yaml)
at startup via `--seed-data-file`.

## Seeded tables

| Table | Used by |
|-------|---------|
| `usa_names.usa_1910_2013` | Node/Python query samples (TX names, pagination) |
| `samples.shakespeare` | Cache-disabled / parameterized query samples |
| `stackoverflow.posts_questions` | Stack Overflow simple-app sample |

## Local gateway

```bash
task emulator:run-seeded SEED_FILE=testdata/public-data/bigquery-public-data.yaml
```

Docker Compose (`task thirdparty:emulator-up`) passes the same file via the
image `CMD` (see `Dockerfile`).
