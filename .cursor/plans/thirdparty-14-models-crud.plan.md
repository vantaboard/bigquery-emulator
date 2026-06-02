---
name: models (BQML) CRUD
overview: "Design an engine model registry, then implement the models.* CRUD surface against it. No current plan covers BQML model state — this is greenfield design + handler implementation. Models execution (training, eval, prediction) is a follow-on; this plan only covers the metadata + CRUD round-trip."
todos:
  - id: tp14_design
    content: "Design the engine model registry: minimum shape to round-trip ModelGet / ModelInsert / ModelPatch / ModelDelete. Per-row columns mirror BigQuery's Model resource — modelType, featureColumns, labelColumns, hyperparameters, trainingRuns[], etag."
    status: pending
  - id: tp14_registry
    content: "Implement the registry in backend/storage/ as a persistent table; cascade-delete with parent dataset (mirror Tables and Routines)."
    status: pending
  - id: tp14_get
    content: "ModelGet: lookup by modelReference; 404 envelope when missing."
    status: pending
  - id: tp14_list
    content: "ModelList: paginate by modelId; honor `filter`."
    status: pending
  - id: tp14_insert
    content: "ModelInsert: route any `CREATE MODEL` DDL through this surface; also accept direct REST inserts (the latter is rare but BigQuery exposes the route)."
    status: pending
  - id: tp14_patch_delete
    content: "ModelPatch + ModelDelete: standard CRUD; If-Match etag."
    status: pending
  - id: tp14_handler_wire
    content: "Replace the NotImplemented shim entries in gateway/handlers/models.go with the registry-backed implementations."
    status: pending
  - id: tp14_tests
    content: "Unit-test the registry + handler set; integration-test via the node Models block and a CREATE MODEL DDL round-trip."
    status: pending
  - id: tp14_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp14` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 14 — models (BQML) CRUD

## Source

- `not-implemented-routes-catalog.plan.md` Group 3 (now deleted).
- Failing node tests (before-all):
  - `Models > should retrieve a model if it exists`
  - `Create/Delete Model > should create a model`
  - `Create/Delete Model > should delete a model`
  - (cascade) `ModelPatch`
- Stubs: [`gateway/handlers/models.go`](../../gateway/handlers/models.go)
  plus the missing `ModelInsert` route currently handled by the
  `NotImplemented` shim.

## Prerequisites

None for the CRUD-only scope. Execution / training / prediction is
a separate plan family.

## Scope

This plan covers **only** the metadata round-trip — enough that the
node Models block can create, read, update, and delete a model
resource and observe consistent metadata. It does not cover:

- Model training execution.
- Model evaluation (`ML.EVALUATE`).
- Model prediction (`ML.PREDICT`).
- Any of the BQML ML.* table-valued functions.

Those land separately under
[`specialized-feature-policy.plan.md`](./specialized-feature-policy.plan.md)
when BQML moves out of `unsupported`. The metadata surface here is
deliberately ahead of execution so the node sample can finish its
fixture setup.

## Implementation

### Registry shape

Minimum columns to round-trip the BigQuery `Model` resource:

- `modelReference = {projectId, datasetId, modelId}` (primary key).
- `modelType` — string enum (LINEAR_REGRESSION, LOGISTIC_REGRESSION,
  KMEANS, RANDOM_FOREST_*, BOOSTED_TREE_*, AUTOENCODER, ARIMA, DNN_*,
  TENSORFLOW, ...).
- `featureColumns[]` — column name + type.
- `labelColumns[]` — same.
- `hyperparameters` — opaque JSON blob (round-tripped as-is).
- `trainingRuns[]` — opaque list (round-tripped as-is for now).
- `etag`, `creationTime`, `lastModifiedTime`, `expirationTime`,
  `friendlyName`, `description`, `location`, `labels`.

Persist alongside the dataset rows. Cascade-delete with parent
dataset.

### Handlers

1. **ModelGet** — by modelReference; 404 envelope.
2. **ModelList** — paginate by modelId; honor `filter`.
3. **ModelInsert** — register a model. Two entry points:
   - Direct REST insert (rare, but BigQuery exposes the route).
   - `CREATE MODEL` DDL routed through the control-op executor.
     Implement both. The DDL path is the one BQML samples use.
4. **ModelPatch** — PATCH with `etag` enforcement.
5. **ModelDelete** — remove from registry.

### DDL plumbing

`CREATE MODEL` DDL needs to route through the control-op executor
to `ModelInsert`. Confirm against
[`control-op-executor.plan.md`](./control-op-executor.plan.md) — if
the executor already exposes a `CreateModel` hook, wire to it; if
not, add the hook here.

### Non-goals + soft errors

- `MODEL.PREDICT` and other ML.* TVFs return a documented
  bigquery-shaped `UNIMPLEMENTED` (per
  [`specialized-feature-policy.plan.md`](./specialized-feature-policy.plan.md)).
- Training jobs (`CREATE MODEL ... AS SELECT ...`) write a model
  resource with empty `trainingRuns[]` and a documented note that
  no training occurred — never silently approximate training.

## Tests

- Backend unit tests for the model registry CRUD.
- Handler tests under
  `gateway/handlers/models_test.go` for each handler.
- Integration test: CREATE MODEL DDL via JobInsert from plan 08,
  observe via models.get, delete via models.delete.
- `task thirdparty:node-bigquery-tests` — Models block goes green
  for the CRUD surface (the prediction-using samples stay red
  until BQML execution lands).

## Done criteria

- Engine model registry exists and persists across restarts.
- The five handlers are wired.
- CREATE MODEL DDL round-trips through the registry.
- Node Models CRUD rows go green.
- `thirdparty-00-completion-index.plan.md` todo `tp14` flipped to
  `completed`.
