# First-party workflow scenarios (Python)

Repo-owned multi-step flows that mirror reported customer usage through the
real `google-cloud-bigquery` Python client — not upstream sample snippets.

Run against a live emulator:

```bash
export BIGQUERY_EMULATOR_HOST=http://localhost:9050
task thirdparty:scenarios
```

Expected values are production-validated via `conformance/differential/` oracles
(plan 01) and documented inline in each test module.
