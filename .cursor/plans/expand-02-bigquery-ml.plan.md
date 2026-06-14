---
name: Expand 02 — BigQuery ML (CREATE MODEL + inference)
overview: Promote BigQuery ML from a CREATE MODEL local_stub + unsupported inference to a real local implementation. Materialize and register models in storage (off local_stub), then evaluate ML.PREDICT / ML.FORECAST / ML.EVALUATE locally over the registered model + input rows on the semantic executor. No Vertex AI / remote model passthrough.
est_effort: ~4 weeks
isProject: true
todos:
  - id: model-registry
    content: "Model registry + persistence: promote CREATE MODEL off local_stub (backend/engine/control/stubs/create_model.{h,cc}) to a real control-op that parses model type + OPTIONS + the training query, trains/fits a local model for the supported model families, and persists model metadata + parameters through DuckDBStorage (mirror the __bqemu_routines pattern). Rehydrate across engine restarts. REST models.* (gateway/handlers/models.go) serves real metadata instead of stubs."
    status: pending
  - id: model-families
    content: "Pick the initial supported model families and implement local training: start with the deterministic, closed-form ones (LINEAR_REG, LOGISTIC_REG, KMEANS) so results are reproducible for conformance. Document which families stay unsupported (DNN/BOOSTED_TREE/remote/TENSORFLOW/ONNX) and keep them surfacing UNIMPLEMENTED."
    status: pending
  - id: ml-predict
    content: "ML.PREDICT(MODEL <id>, <input>): resolve the named model from the registry, apply the fitted parameters to each input row on the semantic executor, emit the documented output schema (predicted_<label> + probabilities for classifiers). Flip ml.predict off unsupported in functions.yaml."
    status: pending
  - id: ml-evaluate-forecast
    content: "ML.EVALUATE (metrics over an eval set for the supported families) and ML.FORECAST (for the time-series families if in scope, else keep unsupported + documented). Flip ml.evaluate / ml.forecast rows per what actually lands."
    status: pending
  - id: routing
    content: "Route classifier: ML.* are table-valued/analytic shapes — confirm the coordinator dispatches model-bearing calls to the semantic executor and that a call over an unregistered/unsupported-family model still surfaces a BigQuery-shaped UNIMPLEMENTED (no fake answer)."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: CREATE MODEL + ML.PREDICT round-trip for each supported family; ML.EVALUATE metrics; restart-persistence proof. Flip CREATE MODEL (local_stub -> local_impl) and ml.predict/evaluate(/forecast) rows in ENGINE_POLICY + functions.yaml + SHAPE_TRACKER; update ROADMAP §BigQuery ML (⏳ -> ✅ for landed families). Drop any client-lane ML skip rows that pass."
    status: pending
---

# Expand 02 — BigQuery ML

## Why

[ROADMAP.md §BigQuery ML](../../ROADMAP.md) tracks the full ML surface as
⏳ planned. Today `CREATE MODEL` is a metadata-only `local_stub`
(`backend/engine/control/stubs/create_model.{h,cc}`) that returns OK
without registering anything, and `ml.predict` / `ml.forecast` /
`ml.evaluate` are `unsupported` (functions.yaml) so a downstream call
surfaces `UNIMPLEMENTED`. Client libraries that probe ML availability
succeed at startup but cannot run inference.

## The hard part

BigQuery ML spans dozens of model types, most backed by Vertex AI /
remote models the emulator must **not** call. The plan deliberately
scopes to the **deterministic, closed-form** families
(`LINEAR_REG`, `LOGISTIC_REG`, `KMEANS`) so training + inference are
reproducible for conformance. Everything else stays `unsupported` with a
clear envelope. The `local_stub` -> `local_impl` promotion must preserve
the "no fake answer" contract: an unsupported family or unregistered
model still fails loudly.

## Key files

- [`backend/engine/control/stubs/create_model.{h,cc}`](../../backend/engine/control/stubs/) — current CREATE MODEL stub (promote)
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — ML.* dispatch
- [`backend/engine/semantic/functions/`](../../backend/engine/semantic/functions/) — ML evaluation home (new model_funcs unit)
- [`backend/storage/duckdb/`](../../backend/storage/duckdb/) — model persistence (mirror `__bqemu_routines`)
- [`gateway/handlers/models.go`](../../gateway/handlers/models.go) — REST `models.*` metadata
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) — `ml.*` rows
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — BigQuery ML rows

## Steps

1. Model registry + persistence; promote `CREATE MODEL` to a real control-op.
2. Local training for the closed-form families; document the unsupported set.
3. `ML.PREDICT` evaluation on the semantic executor.
4. `ML.EVALUATE` (+ `ML.FORECAST` if in scope).
5. Routing + unregistered/unsupported-model `UNIMPLEMENTED` guard.
6. Fixtures + posture flips + ROADMAP/ENGINE_POLICY updates.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Remote / Vertex AI models, TENSORFLOW / ONNX / TFLite imports, DNN /
  boosted-tree / matrix-factorization families — stay `unsupported`.
- Generative-AI functions (`AI.GENERATE_*`, `ML.GENERATE_TEXT`).
- Training-quality parity with production (we fit deterministically for
  reproducibility, not to match BigQuery's optimizer numerically).
