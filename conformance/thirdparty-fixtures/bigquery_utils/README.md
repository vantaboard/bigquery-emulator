# bigquery-utils conformance fixtures

Generated from [GoogleCloudPlatform/bigquery-utils](https://github.com/GoogleCloudPlatform/bigquery-utils)
(Apache-2.0) via `task conformance:bqutils-sync`.

- `known_failing/` — default codegen output (all extracted scalar UDFs, including those that fail triage).
- `passing/` — fixtures promoted there after engine triage (`./scripts/triage_bqutils_fixtures.sh`).

Do not edit generated YAML by hand; regenerate with `task conformance:bqutils-sync`.

**Promoted external-language fixtures** (moved from `known_failing/` to `passing/`):

- `community/cw_xml_extract.yaml` — scalar `LANGUAGE python` (lxml xpath); evaluated via `python_udf_runtime.cc` (`docs/ENGINE_POLICY.md`).
- Scalar `LANGUAGE js` UDFs (e.g. `cw_editdistance`, `cw_json_array_*`, `cw_url_*`, `url_decode`) — extracted from upstream and evaluated via `js_udf_runtime.cc` (Duktape). JS aggregate/table UDFs remain skipped at extraction (`LANGUAGE js (non-scalar UDF unsupported)`).
