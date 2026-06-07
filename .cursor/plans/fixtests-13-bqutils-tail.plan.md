---
name: FixTests 13 — bqutils tail (geography, XML, analytics, migration)
overview: Close the remaining ~11 known_failing fixtures that do not fit the major themes - GEOGRAPHY, SQL XML builders, nested-struct analytics UDFs, the migration vendor compat set, and the one Python-language UDF (tracked separately as an external-language gap).
depends_on: [fixtests-12-bqutils-bignumeric]
est_effort: 2+ weeks
isProject: true
todos:
  - id: geography
    content: Implement GEOGRAPHY + ST_GeogPOINT + spherical math for azimuth_to_geog_point.
    status: pending
  - id: xml-builders
    content: Implement the SQL XML string builders cw_xml_element and cw_xml_element_with_attributes.
    status: pending
  - id: analytics-udfs
    content: Close nested-STRUCT analytics UDFs t_test and linear_regression (community scalar variant; stored-proc linear_regression already passes separately).
    status: pending
  - id: migration-vendor
    content: Close migration vendor compat - vertica (upperb/lowerb/substrb), sqlserver (convert_*), redshift, oracle (round_datetime) where not already absorbed by plans 09/11/12.
    status: pending
  - id: python-udf
    content: Decide and document the path for cw_xml_extract (LANGUAGE python) - implement Python UDF support or keep it as a documented known_failing external-language gap.
    status: pending
  - id: triage
    content: Re-sync + triage; aim for known_failing -> 0 (minus any explicitly deferred external-language fixture); update index count and docs.
    status: pending
---

# FixTests 13 — bqutils tail (geography, XML, analytics, migration)

## Why

The long tail (~11 files) is a grab-bag of distinct, lower-frequency surfaces. Grouped here so the major-theme plans (09–12) stay focused, and so the final "known_failing -> 0" push is a single accountable plan.

## Fixtures

| Bucket | Fixtures |
|--------|----------|
| Geography | `azimuth_to_geog_point` (`ST_GeogPOINT`, spherical math) |
| SQL XML builders | `cw_xml_element`, `cw_xml_element_with_attributes` (pure SQL) |
| Analytics UDFs | `t_test`, `linear_regression` (community scalar; stored-proc variant already passes) |
| Migration vendor | `migration/vertica/{upperb,lowerb,substrb}`, `migration/sqlserver/{convert_string_bytes,convert_datetime_string,convert_bytes_string}`, `migration/redshift/*`, `migration/oracle/round_datetime` — minus any absorbed by plans 09/11/12 |
| External language | `cw_xml_extract` (**LANGUAGE python**, lxml xpath) |

> Reconcile the migration set with plans 09 (bytes/substrb), 11 (redshift initcap/translate), and 12 (oracle round_datetime) using the plan-01 triage output so no fixture is owned twice.

## Decision: Python UDF (`cw_xml_extract`)

This is an external-language UDF (like the JS UDFs in [bqutils-04-js-udfs.plan.md](bqutils-04-js-udfs.plan.md)). Two acceptable outcomes — pick one and document it:

1. Implement Python UDF execution (large, likely its own follow-up plan), or
2. Keep it in `known_failing/` as a documented external-language gap (mirroring the JS UDF disposition), so "known_failing -> 0" is "0 except documented external-language UDFs".

## Key files

- Geography: new `ST_*` function impls + GEOGRAPHY type plumbing ([`backend/catalog/`](backend/catalog/), [`backend/engine/semantic/`](backend/engine/semantic/), [`gateway/bqtypes/wire.go`](gateway/bqtypes/wire.go)).
- XML/string builders + migration compat: semantic function impls under [`backend/engine/semantic/functions/`](backend/engine/semantic/functions/).

## Steps

1. Repro each; confirm ownership vs plans 09/11/12.
2. Implement geography, XML builders, analytics UDFs, remaining migration compat.
3. Resolve the Python UDF decision and document it.
4. Final re-sync + triage:

```bash
task emulator:build-engine:bazel
task conformance:bqutils-sync
./scripts/triage_bqutils_fixtures.sh
task conformance:bqutils
```

5. Update [bqutils-00-index.plan.md](bqutils-00-index.plan.md) (final passing count, ideally ~117), [`third_party/README.md`](third_party/README.md), `docs/ENGINE_POLICY.md`, `ROADMAP.md`.

## Out of scope

- The major themes (plans 09–12); first-party conformance (Track A).
