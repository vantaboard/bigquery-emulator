---
name: FixTests 12 — bqutils BIGNUMERIC + intervals
overview: Promote the 7 high-precision numeric and interval fixtures - 128-bit signed shifts, banker's rounding (round-half-even), MONTHS_BETWEEN, TIMESTAMP_BUCKET, and interval stringification.
depends_on: [fixtests-11-bqutils-regexp]
est_effort: ~1 week
isProject: true
todos:
  - id: repro
    content: Run the 7 fixtures; classify each as 128-bit numeric, rounding, or interval/datetime.
    status: pending
  - id: bignumeric-shift
    content: Implement 128-bit signed left/right shift semantics for cw_signed_leftshift_128bit and cw_signed_rightshift_128bit over BIGNUMERIC.
    status: pending
  - id: round-half-even
    content: Implement round-half-even (banker's rounding) for cw_round_half_even and cw_round_half_even_bignumeric.
    status: pending
  - id: interval-datetime
    content: Implement MONTHS_BETWEEN (cw_months_between), TIMESTAMP_BUCKET (ts_tumble), and interval stringification (cw_stringify_interval).
    status: pending
  - id: triage
    content: Re-sync + triage; move newly-passing fixtures to passing/; update index count and docs.
    status: pending
---

# FixTests 12 — bqutils BIGNUMERIC + intervals

## Why

Seven fixtures need high-precision numeric semantics and interval/datetime math. The `interval_*` helper fixtures already pass, so interval formatting has a partial base; BIGNUMERIC 128-bit shifts and banker's rounding are the substantive new work.

## Fixtures (community + migration)

- 128-bit numeric: `cw_signed_leftshift_128bit`, `cw_signed_rightshift_128bit`
- Rounding: `cw_round_half_even`, `cw_round_half_even_bignumeric`
- Interval / datetime: `cw_months_between`, `ts_tumble` (`TIMESTAMP_BUCKET`), `cw_stringify_interval`
- (`migration/oracle/round_datetime` may also land here — confirm via triage.)

## Likely gaps

- BIGNUMERIC 128-bit arithmetic + signed shift semantics.
- `ROUND(..., mode => 'ROUND_HALF_EVEN')` banker's rounding for NUMERIC/BIGNUMERIC.
- `TIMESTAMP_BUCKET(ts, INTERVAL n SECOND, origin)`; `MONTHS_BETWEEN`; interval-to-string formatting.

## Key files

- Numeric/decimal handling in semantic eval + [`backend/engine/semantic/functions/`](backend/engine/semantic/functions/) (math funcs).
- Datetime/interval function impls; wire encoding for BIGNUMERIC in [`gateway/bqtypes/wire.go`](gateway/bqtypes/wire.go).

## Steps

1. Repro + classify each fixture.
2. Implement 128-bit shift + round-half-even, then the interval/datetime functions.
3. Re-sync + triage:

```bash
task emulator:build-engine:bazel
task conformance:bqutils-sync
./scripts/triage_bqutils_fixtures.sh
task conformance:bqutils
```

4. Update [bqutils-00-index.plan.md](bqutils-00-index.plan.md), [`third_party/README.md`](third_party/README.md), `docs/ENGINE_POLICY.md`.

## Out of scope

- INT64 bitwise (plan 09 — distinct from 128-bit BIGNUMERIC shifts), regexp/string (plan 11), geography/XML/migration tail (plan 13).
