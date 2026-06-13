---
name: Full 05 — JavaScript UDF call-time runtime
overview: Promote LANGUAGE js UDFs from the metadata-only local_stub posture to a real call-time implementation by embedding a sandboxed JS engine, mapping BigQuery <-> JS type marshaling, and evaluating CREATE FUNCTION ... LANGUAGE js bodies on the semantic executor. Keep Python UDFs unsupported.
est_effort: ~2-3 weeks
isProject: true
todos:
  - id: runtime-choice
    content: "Choose an embeddable JS engine that links cleanly into the Bazel C++ build (QuickJS is the leading candidate: small, no JIT, easy to vendor via http_archive; V8 is heavier and a toolchain risk). Vendor it under third_party/ with a BUILD file; document the determinism/sandbox posture."
    status: pending
  - id: type-marshaling
    content: "Implement BigQuery <-> JS value marshaling per the UDF reference: INT64->Number/String, FLOAT64->Number, STRING->string, BYTES->Uint8Array, BOOL->boolean, ARRAY->Array, STRUCT->Object, JSON->parsed, NUMERIC/BIGNUMERIC/DATE/TIMESTAMP per the documented conversions. Honor the determinism requirement."
    status: pending
  - id: call-eval
    content: "Evaluate registered LANGUAGE js scalar UDFs at call time on the semantic executor: build the JS function from the routine body + arg names, bind marshaled args, run, marshal the return. Wire through the existing routine registry (udf_registry.cc) and EvalContext so persistence/rehydrate already work."
    status: pending
  - id: options-libraries
    content: "Support the OPTIONS(library=[...]) gs:// vs file:// posture: file:// libraries load locally; gs:// stays unsupported (consistent with EXPORT/LOAD). Honor OPTIONS for determinism / max bytes where feasible."
    status: pending
  - id: errors
    content: "Map JS runtime errors / timeouts to BigQuery-shaped error envelopes (the documented `User-defined function` failure surface); enforce a wall-clock cap so a runaway UDF can't hang the engine."
    status: pending
  - id: fixtures-trackers
    content: "conformance/fixtures/udf/ js fixtures (scalar js UDF, array/struct round-trip, error surface); flip the LANGUAGE js row in functions.yaml/node_dispositions + ENGINE_POLICY from local_stub to local_impl/semantic_executor; update SHAPE_TRACKER ResolvedCreateFunctionStmt note; drop dbt functions/test_js + Node createModel-adjacent skip rows that now pass."
    status: pending
---

# Full 05 — JavaScript UDF call-time runtime

## Why

`CREATE FUNCTION ... LANGUAGE js` currently registers metadata only and
surfaces `UNIMPLEMENTED` at call time (the documented two-halves stub
contract):

```105:105:docs/ENGINE_POLICY.md
| JavaScript UDFs (`CREATE FUNCTION ... LANGUAGE js`)                                                          | `local_stub`  | `CREATE FUNCTION ... LANGUAGE js` registers metadata-only `External_function` entries and persists the DDL in `DuckDBStorage` so `routines.get` round-trips; call-time evaluation surfaces `UNIMPLEMENTED` until an embedded JS runtime lands (the two-halves stub contract). |
```

JS UDFs are common in real BigQuery codebases (custom parsing, hashing,
formatting). The registration/persistence half already works from
parity-08, so this plan is purely the call-time runtime.

## Key files

- [`backend/catalog/udf_registry.{h,cc}`](../../backend/catalog/) — routine registry (registration half already done)
- `gateway/routines/ddl.go` — DDL parsing accepts `LANGUAGE js`
- [`backend/engine/semantic/`](../../backend/engine/semantic/) — call-time eval (`EvalSqlUdfBody` is the SQL analog to mirror)
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — JS UDF row to flip
- `third_party/` — JS engine vendoring + BUILD

## Steps

1. Pick + vendor the JS engine (QuickJS recommended for build sanity).
2. Type marshaling layer with the documented BigQuery↔JS conversions.
3. Call-time eval wired through the existing registry/EvalContext.
4. `file://` library loading; `gs://` stays unsupported.
5. Error + timeout mapping.
6. Fixtures + flip the `local_stub` → real route + doc updates + drop
   the `functions/test_js` dbt skip row.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task thirdparty:dbt-bigquery-tests   # functions/test_js
task thirdparty:node                 # JS UDF samples
task bazel:shutdown && task bazel:status
```

## Out of scope

- Python UDFs (`LANGUAGE python`) — stays `unsupported` (no runtime).
- `gs://`-hosted JS libraries — stays `unsupported` (cloud-storage gap).
- BigQuery ML / remote functions.
