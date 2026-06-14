---
name: Expand 08 — Scalar + statement long tail
overview: Land the small, independent deferred items from ROADMAP §Planned work. Two are real local implementations - ST_GEOGFROMWKB (WKB -> GEOGRAPHY on the semantic GIS path) and ResolvedExplainStmt (EXPLAIN plan introspection). Two are deterministic stubs that only need to stop failing - KEYS.ENCRYPT / KEYS.DECRYPT_BYTES and SESSION_USER. Each is self-contained.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: st-geogfromwkb
    content: "ST_GEOGFROMWKB (REAL implementation): parse WKB bytes into the GEOGRAPHY representation used by geog_funcs.cc (which already does ST_GEOGFROMTEXT / ST_GEOGPOINT / ST_ASTEXT). Reuse the existing GIS value plumbing; flip st_geogfromwkb off unsupported -> local_impl (semantic_executor) in functions.yaml and remove ST_GEOGFROMWKB from the unsupported list in the ENGINE_POLICY Geography row."
    status: pending
  - id: explain-stmt
    content: "ResolvedExplainStmt (REAL implementation): EXPLAIN plan introspection. Emit a BigQuery-shaped plan/explain result for the analyzed query (the route classifier + transpiler already produce a plan shape). Flip ResolvedExplainStmt off unsupported in node_dispositions.yaml."
    status: pending
  - id: keys-stub
    content: "KEYS.ENCRYPT / KEYS.DECRYPT_BYTES (STUB - not a real AEAD): these are not useful locally and should only stop failing. Return a deterministic BigQuery-shaped placeholder through the existing keys stub lane (backend/engine/semantic/stubs/keys.{h,cc}, the same lane as KEYS.NEW_KEYSET): KEYS.ENCRYPT -> a fixed BYTES envelope, KEYS.DECRYPT_BYTES -> a fixed/echoed BYTES value. NOT real encryption. Flip keys.encrypt / keys.decrypt_bytes from unsupported -> local_stub."
    status: pending
  - id: session-user-stub
    content: "SESSION_USER (STUB): return a deterministic placeholder principal identifier (a fixed emulator principal string, optionally configurable) so row/column-policy + audit queries do not fail. Flip session_user from unsupported -> local_stub."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: ST_GEOGFROMWKB constructor (WKB -> WKT, real), an EXPLAIN smoke (real), KEYS.ENCRYPT/DECRYPT_BYTES round-trip returns a placeholder without erroring (stub), and SESSION_USER returns the placeholder principal (stub). Flip the rows in functions.yaml / node_dispositions.yaml + SHAPE_TRACKER with the correct posture (local_impl for ST_GEOGFROMWKB/EXPLAIN, local_stub for KEYS/SESSION_USER); update ENGINE_POLICY (Key management, Geography, + EXPLAIN/SESSION_USER notes) and ROADMAP §Deferred built-in functions + §Statements."
    status: pending
---

# Expand 08 — Scalar + statement long tail

## Why

[ROADMAP.md §Deferred built-in functions](../../ROADMAP.md) and
§Statements list four self-contained ⏳ items. Two get **real** local
implementations; two are **stubs** that only need to stop failing:

- **Real:** `ST_GEOGFROMWKB` — the one remaining constructor gap in the
  landed GIS MVP (`geog_funcs.cc` already does WKT + point constructors).
- **Real:** `ResolvedExplainStmt` — `EXPLAIN` plan introspection.
- **Stub:** `KEYS.ENCRYPT` / `KEYS.DECRYPT_BYTES` — no real AEAD;
  encryption is not useful locally, so return a deterministic placeholder
  so the query does not fail.
- **Stub:** `SESSION_USER` — return a fixed placeholder principal.

## The hard part

Keeping the two postures straight. `ST_GEOGFROMWKB` and `EXPLAIN` are
genuine implementations that must produce correct results. `KEYS.*` and
`SESSION_USER` are placeholders — and the `KEYS.*` change deliberately
reverses the older ENGINE_POLICY "fail loudly" stance (the new intent is
no-fail), so the policy text must be updated alongside.

## Key files

- [`backend/engine/semantic/functions/geog_funcs.{h,cc}`](../../backend/engine/semantic/functions/) — GIS value plumbing (add WKB, real)
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — `ResolvedExplainStmt` dispatch (real)
- [`backend/engine/semantic/stubs/keys.{h,cc}`](../../backend/engine/semantic/stubs/) — KEYS stub lane (add encrypt/decrypt placeholders)
- [`backend/engine/semantic/stubs/`](../../backend/engine/semantic/stubs/) — SESSION_USER placeholder
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) — `keys.*`, `st_geogfromwkb`, `session_user`
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — `ResolvedExplainStmt`
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Key management + Geography rows

## Steps

1. `ST_GEOGFROMWKB` on the GIS path (real).
2. `ResolvedExplainStmt` plan introspection (real).
3. `KEYS.ENCRYPT` / `KEYS.DECRYPT_BYTES` placeholder (stub).
4. `SESSION_USER` placeholder principal (stub).
5. Fixtures + per-item posture flips (local_impl vs local_stub) + docs.

## Out of scope

- Real AEAD / Tink-compatible keysets — `KEYS.*` is a placeholder.
- Authenticated session identity / IAM — `SESSION_USER` is a placeholder.
- The broader `ST_*` GIS long tail (aggregates, buffer/simplify) — this
  plan only adds the WKB constructor.
