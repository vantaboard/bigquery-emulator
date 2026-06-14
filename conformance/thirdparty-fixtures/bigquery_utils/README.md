# bigquery-utils conformance fixtures

Generated from [GoogleCloudPlatform/bigquery-utils](https://github.com/GoogleCloudPlatform/bigquery-utils)
(Apache-2.0) via `task conformance:bqutils-sync`.

- `known_failing/` — default codegen output (all extracted pure-SQL scalar UDFs).
- `passing/` — fixtures promoted there manually after engine triage.

Do not edit generated YAML by hand; regenerate with `task conformance:bqutils-sync`.

**Promoted external-language fixtures** (moved from `known_failing/` to `passing/`):

- `community/cw_xml_extract.yaml` — scalar `LANGUAGE python` (lxml xpath); evaluated via `python_udf_runtime.cc` (`docs/ENGINE_POLICY.md`).
