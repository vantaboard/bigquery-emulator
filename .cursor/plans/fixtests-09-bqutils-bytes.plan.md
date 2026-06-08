---
name: FixTests 09 — bqutils bytes / binary / bitwise
overview: Promote the 8 bytes/binary/bitwise/base-conversion fixtures from known_failing - to_hex, from_hex, to_binary, from_binary, cw_getbit, cw_getbit_binary, cw_to_base, cw_from_base - by closing INT64 bit-shift, FORMAT('%02x'), and BYTES cast + SUBSTR gaps.
depends_on: [fixtests-08-bqutils-any-type]
est_effort: ~1 week
isProject: true
todos:
  - id: repro
    content: Run the 8 bytes/bitwise known_failing fixtures; diff each against the already-passing getbit fixture to isolate the missing primitive.
    status: completed
  - id: bitwise
    content: Implement/repair INT64 bit shifts (<<, >>) and bit extraction used by cw_getbit / cw_getbit_binary / to_binary / from_binary.
    status: completed
  - id: format-hex
    content: Support FORMAT('%02x', ...) and GENERATE_ARRAY byte walks for to_hex / from_hex / cw_to_base / cw_from_base.
    status: completed
  - id: bytes-substr
    content: Support CAST(str AS BYTES) + SUBSTR over bytes (overlaps migration/vertica substrb) for multibyte handling.
    status: completed
  - id: triage
    content: Re-sync + triage; move newly-passing fixtures to passing/; update index count and docs.
    status: completed
---

# FixTests 09 — bqutils bytes / binary / bitwise

## Why

Eight fixtures depend on byte/bit primitives. `cw_getbit` already passes, so the gap is a small set of missing builtins rather than a whole subsystem — use the passing fixture as the reference oracle.

Fixtures (community): `to_hex`, `from_hex`, `to_binary`, `from_binary`, `cw_getbit`, `cw_getbit_binary`, `cw_to_base`, `cw_from_base`. Thematically overlaps the migration bytes vendors (`migration/vertica/substrb`, `migration/sqlserver/convert_bytes_string`, `convert_string_bytes`) handled in plan 13 — share the BYTES/SUBSTR work.

## Likely gaps

- INT64 bit shifts (`<<`, `>>`) and bit extraction.
- `FORMAT('%02x', ...)` hex formatting; `GENERATE_ARRAY` byte walks.
- `CAST(str AS BYTES)` + `SUBSTR` on bytes for multibyte strings.

## Key files

- Semantic function impls for bitwise / format / bytes ops under [`backend/engine/semantic/functions/`](backend/engine/semantic/functions/).
- DuckDB transpiler function emit for any of these routed to `duckdb_udf` / `duckdb_native`.

## Steps

1. Repro each; diff SQL bodies against `cw_getbit` (passing) to find the one missing primitive per fixture.
2. Implement the bitwise / format / bytes builtins.
3. Re-sync + triage:

```bash
task emulator:build-engine:bazel
task conformance:bqutils-sync
./scripts/triage_bqutils_fixtures.sh
task conformance:bqutils
```

4. Update [bqutils-00-index.plan.md](bqutils-00-index.plan.md), [`third_party/README.md`](third_party/README.md), `docs/ENGINE_POLICY.md`.

## Out of scope

- RANGE (plan 10), regexp (plan 11), BIGNUMERIC 128-bit shifts (plan 12 — note the overlap with `cw_signed_*_128bit`). Keep this plan on INT64/BYTES-width work.
