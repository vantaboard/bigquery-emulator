# bigquery-utils conformance fixtures

Generated from [GoogleCloudPlatform/bigquery-utils](https://github.com/GoogleCloudPlatform/bigquery-utils)
(Apache-2.0) via `task conformance:bqutils-sync`.

- `known_failing/` — default codegen output (all extracted pure-SQL scalar UDFs).
- `passing/` — fixtures promoted there manually after engine triage.

Do not edit generated YAML by hand; regenerate with `task conformance:bqutils-sync`.

**Deferred external-language fixtures** (intentionally kept in `known_failing/`):

- `community/cw_xml_extract.yaml` — `LANGUAGE python` (lxml xpath); JS/Python UDFs are unsupported (`LANGUAGE js` surfaces UNIMPLEMENTED per `docs/ENGINE_POLICY.md`), so this stays deferred until Python UDF execution lands.
