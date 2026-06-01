---
name: ""
overview: ""
todos: []
isProject: false
---

# Specialized Feature Policy

## Goal

Decide the local-only posture for the BigQuery feature families that
cannot be implemented as a thin DuckDB rewrite or a small UDF: ML,
geography / GIS, differential privacy, anonymized aggregation,
networking, key-management, HLL, proto, MEASURE / measure functions,
graph (GQL), and the JavaScript UDF surface. For each family, pick
one of three postures and document the choice.

## Background

The emulator does not forward query work to BigQuery. Any specialized
feature is therefore "local implementation," "deterministic stub
with BigQuery-shaped error," or "unsupported by design." We need a
single source of truth for the per-family decision so future
contributors are not tempted to silently approximate.

The three postures:

- **`local_impl`** — we ship a local implementation that runs in
  the semantic executor (or, occasionally, as a DuckDB UDF when the
  semantics fit). Behavior matches BigQuery on the documented
  surface; numerical / approximate algorithms may have
  documented epsilon differences.
- **`local_stub`** — the function / statement is accepted at
  parse / analyzer time and the engine returns a deterministic
  BigQuery-shaped stub (typically an `OK` response with a sentinel
  value, or an `UNIMPLEMENTED` with a clear message). The stub is
  there so client libraries' startup probes succeed; it does not
  pretend to compute the real BigQuery result.
- **`unsupported`** — the function / statement returns a
  `UNIMPLEMENTED` error whose message names the family and links to
  this document.

## Dependencies

- `execution-disposition-registry.plan.md` (the YAML rows that this
  plan annotates).
- `semantic-functions-compliance.plan.md` (when a family graduates
  to `local_impl`, this plan defers the implementation there).

## Scope

Family-by-family decisions this plan publishes:

| Family                            | Default posture | Notes                                                                                  |
|-----------------------------------|-----------------|----------------------------------------------------------------------------------------|
| BigQuery ML (`ML.*`)              | `unsupported`   | No local model training / serving. `CREATE MODEL` accepted as `local_stub` metadata; inference returns `UNIMPLEMENTED`. |
| Geography / GIS (`ST_*`)          | `unsupported`   | Could graduate to `local_impl` via DuckDB's `spatial` extension; deferred until demand is clear. |
| Differential privacy (`AnonymizedAggregate*`, `DifferentialPrivacyAggregate*`, `AggregationThresholdAggregate*`) | `unsupported` | DP semantics are too easy to get wrong by accident; no stub. |
| Anonymized aggregation (other)    | `unsupported`   | Same posture as DP family above.                                                                  |
| Networking (`NET.*`)              | `local_impl`    | Small surface; deterministic; lands in `semantic-functions-compliance.plan.md`.                   |
| Key management (`KEYS.*`)         | `local_stub`    | Returns deterministic placeholder values so test data does not surprise client libraries.         |
| HLL (`HLL_COUNT.*`)               | `local_impl`    | Local approximate counter; deterministic seed. `semantic-functions-compliance.plan.md`.           |
| Proto (`ResolvedMakeProto`, ...)  | `unsupported`   | Proto type system is a large surface; no demand today.                                            |
| MEASURE / measure functions       | `unsupported`   | Tracked by `ResolvedGetRowField`; no demand today.                                                |
| Graph (`GQL`, `ResolvedGraph*Scan`) | `unsupported` | Pinned `unsupported`; revisit when a client library starts probing it.                            |
| JavaScript UDFs                   | `unsupported`   | `CREATE FUNCTION ... LANGUAGE js` accepted as `local_stub` metadata; invocation returns `UNIMPLEMENTED` until a sandboxed runtime ships. |
| Sequences (`ResolvedSequence`)    | `unsupported`   | No demand today.                                                                                  |
| `LOAD DATA` from non-local sources | `unsupported`  | `LOAD DATA LOCAL` lands via the control-op executor; cloud-storage sources stay `unsupported`.    |

## Implementation Plan

1. For each family in the table above, mark the matching
   `functions.yaml` / `node_dispositions.yaml` rows with the
   correct disposition (`unsupported` or `semantic_executor` for
   `local_impl` rows that graduate).
2. For `local_stub` families, add a small handler under
   `backend/engine/control/stubs/` (for metadata-only stubs like
   `CREATE MODEL`) or `backend/engine/semantic/stubs/` (for
   function-call stubs like `KEYS.NEW_KEYSET`). Each stub returns
   a deterministic BigQuery-shaped value.
3. For `unsupported` families, ensure the error message names the
   family and links to this document so users know what to do
   instead.
4. For families that graduate to `local_impl` later, leave a
   prominent TODO in this document plus a one-line stub in the
   matching `semantic-functions-compliance.plan.md` row.
5. Cross-link this document from `docs/ENGINE_POLICY.md`'s
   `unsupported` row so the public docs name the source of truth
   for these decisions.

## Tests

- Per-family unit tests that the disposition is set correctly in
  the registry.
- Engine-level integration tests in `gateway/e2e/specialized/`
  asserting:
  - `local_stub` families return the documented stub response.
  - `unsupported` families return `UNIMPLEMENTED` with the
    documented error message and the link to this doc.
  - `local_impl` families (after they graduate via
    `semantic-functions-compliance.plan.md`) compute the right
    result.
- Conformance fixtures under
  `conformance/fixtures/specialized/` pinning the disposition.

## Done Criteria

- Every specialized-family row in `functions.yaml` /
  `node_dispositions.yaml` has a non-`(planned)` disposition that
  matches this document's table.
- Every `unsupported` error names the family and links here.
- `local_stub` families return their deterministic stub responses
  without surprising client libraries.
- `docs/ENGINE_POLICY.md` cross-links this document from its
  `unsupported` route description.
