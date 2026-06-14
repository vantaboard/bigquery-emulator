---
name: Expand 02 — BigQuery ML (deterministic stubs)
overview: BigQuery ML is not useful in a local emulator (no Vertex AI, no real training/serving), so the goal here is NOT a real implementation - it is to stop ML.* from failing a query. CREATE MODEL already routes local_stub; extend the local_stub posture to ML.PREDICT / ML.FORECAST / ML.EVALUATE so a query that references them returns a deterministic BigQuery-shaped placeholder result instead of UNIMPLEMENTED. No model is trained or stored.
est_effort: ~1 week
isProject: true
todos:
  - id: stub-semantics
    content: "Decide the deterministic placeholder each ML.* call returns so a query does not fail: ML.PREDICT -> input rows passed through with the documented predicted_<label> output column(s) filled with a fixed sentinel/NULL; ML.EVALUATE -> a single metrics row of fixed/NULL values with the documented schema; ML.FORECAST -> the documented forecast schema with sentinel rows. Shapes must match BigQuery's output schema so client libraries parse the result; values are explicitly placeholders, not predictions."
    status: pending
  - id: stub-handlers
    content: "Implement the ML.* stubs through the existing function-stub mechanism (backend/engine/semantic/stubs/, the same lane KEYS.NEW_KEYSET uses) and confirm CREATE MODEL stays the metadata-only control stub (backend/engine/control/stubs/create_model.{h,cc}). A predict over an unregistered model name must still produce a clean (non-crashing) result, not a hard error."
    status: pending
  - id: routing
    content: "Route classifier: ensure ML.* model-bearing calls dispatch to the stub lane rather than surfacing UNIMPLEMENTED, while keeping the priority order intact (a query mixing a stubbed ML.* with a genuinely unsupported shape still surfaces the unsupported error)."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: CREATE MODEL + ML.PREDICT / ML.EVALUATE return a schema-correct placeholder result (schema_only matching mode) without erroring. Flip ml.predict / ml.forecast / ml.evaluate from unsupported -> local_stub in functions.yaml + SHAPE_TRACKER; update the ENGINE_POLICY ML rows + ROADMAP §BigQuery ML to describe the stub posture."
    status: pending
  - id: skip-audit
    content: "Third-party skip audit (run before declaring done). The ML stub means ML.* queries no longer error, so several currently-skipped ML tests may run as schema_only checks. Re-run each suite and unskip what passes: golang `bqtestutil.SkipEmulatorBQML` call sites (third_party/golang-bigquery-tests/bqtestutil/emulator_skip.go); python `model` / `bqml` skip substrings (third_party/python-bigquery-tests/emulator_pytest_skip.py + _SKIP_FIXTURES model_id); node `models.test.js` (third_party/node-bigquery-tests/test/setup.js + EMULATOR.md); java BQML ITs. Keep skips where the test asserts real prediction values (the stub returns placeholders, not predictions) and note why. Update third_party/README.md."
    status: pending
---

# Expand 02 — BigQuery ML (deterministic stubs)

## Why

BigQuery ML depends on Vertex AI / real model training + serving that a
local emulator cannot meaningfully provide. The product decision
(ROADMAP §BigQuery ML) is therefore **stub, do not implement**: the only
goal is that a query containing `ML.PREDICT` / `ML.FORECAST` /
`ML.EVALUATE` (over a `CREATE MODEL` that already returns a metadata-only
stub) **does not fail**. It returns a deterministic, schema-correct
placeholder instead of `UNIMPLEMENTED`.

## The hard part

Honesty + schema fidelity. The stub must return the **documented output
schema** so client libraries parse the result, while making clear (docs
+ placeholder values) that nothing was predicted. This deliberately
relaxes the older ENGINE_POLICY stance ("`ML.PREDICT` stays unsupported
so misuse fails loudly") — the new intent is no-fail, not loud-fail, so
the policy text must be updated alongside.

## Key files

- [`backend/engine/control/stubs/create_model.{h,cc}`](../../backend/engine/control/stubs/) — CREATE MODEL stub (stays as-is)
- [`backend/engine/semantic/stubs/`](../../backend/engine/semantic/stubs/) — function-stub lane (add ML.* placeholders, mirror `keys.cc`)
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — ML.* dispatch to the stub lane
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) — `ml.*` rows (`unsupported` -> `local_stub`)
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — BigQuery ML rows + the `local_stub` "no silent approximation" note

## Steps

1. Decide the placeholder result schema/values per ML.* function.
2. Implement the stubs in the function-stub lane; keep CREATE MODEL stub.
3. Route ML.* to the stub lane (no `UNIMPLEMENTED`).
4. Fixtures (schema_only, no-error) + flip `ml.*` to `local_stub` + doc updates.

## Third-party / conformance to revisit

The stub makes ML.* queries succeed (schema-correct placeholder), so
**audit the ML skip rows** — tests that only need the query to *run*
(not to return real predictions) can be unskipped. Re-run the suite to
prove it; keep skips that assert prediction *values* and note why.

- **golang** — `bqtestutil.SkipEmulatorBQML` call sites
  (`third_party/golang-bigquery-tests/bqtestutil/emulator_skip.go`).
- **python** — `model` / `bqml` skip substrings + `model_id` fixture
  (`third_party/python-bigquery-tests/emulator_pytest_skip.py`).
- **node** — `models.test.js` (`third_party/node-bigquery-tests/test/setup.js`).
- **java** — BQML ITs; update `third_party/README.md` matrices.

## Out of scope

- Any real training, model storage, or inference math.
- Remote / Vertex AI models, TENSORFLOW / ONNX imports, generative-AI
  functions (`AI.GENERATE_*`, `ML.GENERATE_TEXT`) — keep `unsupported`.
- Numeric/result parity with production BigQuery (values are placeholders).
