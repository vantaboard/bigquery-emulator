---
name: Expand 08 — Scalar + statement long tail
overview: Land the small, independent deferred items from ROADMAP §Planned work that don't need a plan of their own - KEYS.ENCRYPT / KEYS.DECRYPT_BYTES (real local AEAD over the existing keyset stub), ST_GEOGFROMWKB (WKB -> GEOGRAPHY on the semantic GIS path), SESSION_USER (session principal), and ResolvedExplainStmt (EXPLAIN plan introspection). Each is a self-contained promotion off unsupported.
est_effort: ~2 weeks
isProject: true
todos:
  - id: keys-aead
    content: "KEYS.ENCRYPT / KEYS.DECRYPT_BYTES: implement real local AEAD over the keyset representation that KEYS.NEW_KEYSET / KEYS.KEYSET_LENGTH already stub (backend/engine/semantic/stubs/keys.{h,cc}). Promote NEW_KEYSET/KEYSET_LENGTH from local_stub to a real (local, non-Tink-wire-compatible) keyset if needed so encrypt/decrypt round-trip. Preserve the 'fail loudly on misuse' contract for any unsupported keyset shape. Flip keys.encrypt / keys.decrypt_bytes off unsupported."
    status: pending
  - id: st-geogfromwkb
    content: "ST_GEOGFROMWKB: parse WKB bytes into the GEOGRAPHY representation used by geog_funcs.cc (which already does ST_GEOGFROMTEXT / ST_GEOGPOINT / ST_ASTEXT). Reuse the existing GIS value plumbing; flip st_geogfromwkb off unsupported in functions.yaml and update the ENGINE_POLICY Geography row (remove ST_GEOGFROMWKB from the unsupported list)."
    status: pending
  - id: session-user
    content: "SESSION_USER: return the session principal identifier. Decide the source (a configurable session identity, default to a deterministic emulator principal) and surface it for row/column-policy + audit queries. Flip session_user off unsupported."
    status: pending
  - id: explain-stmt
    content: "ResolvedExplainStmt: EXPLAIN plan introspection. Emit a BigQuery-shaped plan/explain result for the analyzed query (the route classifier + transpiler already produce a plan shape). Flip ResolvedExplainStmt off unsupported in node_dispositions.yaml."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: KEYS encrypt/decrypt round-trip, ST_GEOGFROMWKB constructor (WKB -> WKT), SESSION_USER, and an EXPLAIN smoke. Flip the four families in functions.yaml / node_dispositions.yaml + SHAPE_TRACKER; update ENGINE_POLICY (Key management, Geography, + add EXPLAIN/SESSION_USER notes) and ROADMAP §Deferred built-in functions + §Statements."
    status: pending
---

# Expand 08 — Scalar + statement long tail

## Why

[ROADMAP.md §Deferred built-in functions](../../ROADMAP.md) and
§Statements list four self-contained ⏳ items that each promote off
`unsupported` without needing a dedicated plan:

- `KEYS.ENCRYPT` / `KEYS.DECRYPT_BYTES` — today `unsupported` so a
  consumer can't round-trip the `KEYS.NEW_KEYSET` / `KEYS.KEYSET_LENGTH`
  `local_stub` sentinels into a real AEAD op.
- `ST_GEOGFROMWKB` — the one remaining constructor gap in the landed GIS
  MVP (`geog_funcs.cc` already does WKT + point constructors).
- `SESSION_USER` — session principal for row/column-policy + audit SQL.
- `ResolvedExplainStmt` — `EXPLAIN` plan introspection.

## The hard part

`KEYS.*` is the one with a contract subtlety: the existing keyset stubs
deliberately fail loudly so a fake sentinel can't be encrypted/decrypted.
Promoting encrypt/decrypt means making `NEW_KEYSET` produce a **real
local keyset** (not necessarily Tink-wire-compatible) so the round-trip
is honest, while still rejecting genuinely unsupported keyset shapes.
The other three are mechanical.

## Key files

- [`backend/engine/semantic/stubs/keys.{h,cc}`](../../backend/engine/semantic/stubs/) — keyset stubs (promote)
- [`backend/engine/semantic/functions/geog_funcs.{h,cc}`](../../backend/engine/semantic/functions/) — GIS value plumbing (add WKB)
- [`backend/engine/semantic/functions/dispatch.cc`](../../backend/engine/semantic/functions/dispatch.cc) — function dispatch
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — `ResolvedExplainStmt` dispatch
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) — `keys.*`, `st_geogfromwkb`, `session_user`
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — `ResolvedExplainStmt`
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Key management + Geography rows

## Steps

1. `KEYS.ENCRYPT` / `KEYS.DECRYPT_BYTES` + real local keyset.
2. `ST_GEOGFROMWKB` on the GIS path.
3. `SESSION_USER` session principal.
4. `ResolvedExplainStmt` plan introspection.
5. Fixtures + tracker/posture flips.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Tink wire-format compatibility for keysets (local representation only).
- The broader `ST_*` GIS long tail (aggregates, buffer/simplify) — those
  stay tracked separately; this plan only adds the WKB constructor.
- Real authenticated session identity / IAM (SESSION_USER returns a
  configurable/deterministic principal, not an authenticated user).
