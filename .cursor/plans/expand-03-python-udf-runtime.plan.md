---
name: Expand 03 — Python UDF runtime
overview: Promote CREATE FUNCTION ... LANGUAGE python off unsupported by adding a local Python UDF runtime that mirrors the landed LANGUAGE js path (register through a UDF registry, persist the DDL through DuckDBStorage, evaluate bodies at call time on the semantic executor). cw_xml_extract from the bqutils known_failing/ corpus is the representative parity target.
est_effort: ~3 weeks
isProject: true
todos:
  - id: runtime-choice
    content: "Choose the embedded Python runtime + isolation story. Options: a vendored interpreter (CPython embed / a sandboxed subprocess) invoked per call, parallel to how js_udf_runtime.cc embeds Duktape. Decide on the dependency-availability contract (BigQuery Python UDFs allow a packages list) and what stays unsupported (network, arbitrary pip installs)."
    status: pending
  - id: registry-persistence
    content: "Python UDF registration + persistence: extend the routine registry path (backend/catalog/js_udf_registry.{h,cc} is the JS analog; add a python_udf_registry or generalize) so CREATE FUNCTION ... LANGUAGE python registers, writes through to DuckDBStorage (__bqemu_routines), rehydrates across restart, and round-trips through routines.get / INFORMATION_SCHEMA.ROUTINES."
    status: pending
  - id: call-eval
    content: "Call-time evaluation on the semantic executor: marshal BigQuery arg values into Python, run the body via the chosen runtime (mirror backend/engine/semantic/js_udf_runtime.{h,cc}), marshal the result back with correct type coercion + NULL handling. Scalar UDFs first."
    status: pending
  - id: tvf-aggregate
    content: "Decide scope for table-valued / aggregate Python UDFs. Scalar is the must-have; TVF/aggregate may stay unsupported initially (document), matching the JS UDF posture where non-scalar shapes are deferred."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures under conformance/fixtures/udf/ (python_scalar_*). Flip the Python UDF row in ENGINE_POLICY (unsupported -> local_impl), update ROADMAP §Python UDFs."
    status: pending
  - id: skip-audit
    content: "Third-party + conformance skip audit (run before declaring done). Re-run each suite and unskip / promote what now passes: promote cw_xml_extract out of bqutils known_failing/ (conformance/thirdparty-fixtures/bigquery_utils/known_failing/community/cw_xml_extract.yaml) into passing/ if the Python runtime evaluates it; drop python LANGUAGE-python UDF skip rows from the client-lane skip matrices (third_party/python-bigquery-tests/emulator_pytest_skip.py and others) that now pass. Keep skips for non-scalar (TVF/aggregate) Python UDFs if those stay deferred + note why. Update third_party/README.md."
    status: pending
---

# Expand 03 — Python UDF runtime

## Why

[ROADMAP.md §Python UDFs](../../ROADMAP.md) tracks
`CREATE FUNCTION ... LANGUAGE python` as ⏳ planned. The `LANGUAGE js`
scalar path already exists end-to-end — registration
(`backend/catalog/js_udf_registry.{h,cc}`), persistence in
`DuckDBStorage`, and call-time evaluation via embedded Duktape
(`backend/engine/semantic/js_udf_runtime.{h,cc}`). Python UDFs follow the
same shape but need a Python runtime. `cw_xml_extract` sits in bqutils
`known_failing/` as the documented external-language gap and is the
parity target.

## The hard part

Embedding Python safely. Unlike Duktape (a small self-contained JS
engine), a Python runtime is heavier and the BigQuery contract allows a
declared `packages` list. The plan must pick an isolation model
(embedded CPython vs sandboxed subprocess), define the
dependency-availability contract, and keep anything it can't honor
(network, arbitrary installs) `unsupported` with a clear envelope.

## Key files

- [`backend/engine/semantic/js_udf_runtime.{h,cc}`](../../backend/engine/semantic/) — JS runtime analog to mirror
- [`backend/catalog/js_udf_registry.{h,cc}`](../../backend/catalog/) — JS registration analog
- [`backend/catalog/routine_persistence.{h,cc}`](../../backend/catalog/) — `__bqemu_routines` write-through
- [`backend/engine/coordinator/routine_rehydrate.{h,cc}`](../../backend/engine/coordinator/) — restart rehydration
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) / [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Python UDF row
- `third_party/*/emulator_*skip*`, [`third_party/README.md`](../../third_party/README.md) — skip matrices

## Steps

1. Choose + vendor the Python runtime + isolation model.
2. Registration + persistence + rehydration (mirror the JS path).
3. Call-time scalar evaluation with type marshaling.
4. Decide TVF / aggregate scope (defer if needed, documented).
5. Fixtures + posture flip + skip-matrix removal.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task thirdparty:python
task bazel:shutdown && task bazel:status
```

## Third-party / conformance to revisit

When the Python runtime lands, **audit the skip surfaces** — the
representative parity target and the client-lane Python-UDF skips should
flip. Re-run to prove it; keep skips for deferred shapes + note why.

- **bqutils corpus** — promote
  `conformance/thirdparty-fixtures/bigquery_utils/known_failing/community/cw_xml_extract.yaml`
  into `passing/` if it now evaluates.
- **python lane** — drop `LANGUAGE python` UDF skip rows from
  `third_party/python-bigquery-tests/emulator_pytest_skip.py`.
- Keep skips for non-scalar (TVF / aggregate) Python UDFs if deferred;
  update `third_party/README.md`.

## Out of scope

- Arbitrary pip installs / network access from UDF bodies.
- Non-scalar (TVF / aggregate) Python UDFs if deferred in `tvf-aggregate`.
- Performance parity — correctness + determinism only.
