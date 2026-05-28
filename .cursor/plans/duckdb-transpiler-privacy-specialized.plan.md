# DuckDB Transpiler Privacy And Specialized Functions

## Goal

Promote specialized BigQuery-only or security-sensitive families only after the
emulator has explicit semantic support for them.

## Tracker Rows

- `ResolvedAnonymizedAggregateScan` from `skiplist` to `done`.
- `ResolvedDifferentialPrivacyAggregateScan` from `skiplist` to `done`.
- `ResolvedAggregationThresholdAggregateScan` from `skiplist` to `done`.
- `functions.yaml` skiplist entries for approximate aggregates, HLL,
  BigQuery ML, networking, key-management, geography, `BIT_COUNT`,
  `GENERATE_UUID`, `SESSION_USER`, and `ERROR`.

## Implementation Plan

1. Define an emulator policy for privacy-sensitive aggregate behavior. Do not
   emit DuckDB SQL for differential privacy until noise, thresholds, privacy
   units, and error cases are intentionally modeled.
2. Implement approximate aggregate functions either through DuckDB equivalents
   with verified error bounds or through emulator-owned aggregate functions.
3. Implement HLL functions as typed byte/struct operations with compatibility
   tests against BigQuery examples.
4. Implement geography functions only after choosing a DuckDB spatial extension
   strategy and adding binary availability to local builds.
5. Implement networking and key-management functions with deterministic emulator
   behavior; avoid real cloud KMS semantics unless explicitly supported.
6. Implement session/user functions from emulator request context rather than
   process-global state.

## Tests

- Differential privacy and aggregation-threshold tests with deterministic seeds
  or golden statistical bounds.
- Approximate aggregate accuracy and null-handling tests.
- Geography function tests gated on the spatial extension strategy.
- Security-sensitive function tests that prove emulator-local behavior is
  documented and deterministic.

## Done Criteria

- Specialized rows/functions move out of `skiplist` only with explicit product
  semantics.
- No privacy or security function silently delegates to an unrelated DuckDB
  default.
- `functions.yaml` and `SHAPE_TRACKER.md` agree on every promoted family.
