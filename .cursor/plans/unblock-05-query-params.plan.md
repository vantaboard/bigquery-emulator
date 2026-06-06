---
name: Unblock 05 — Query parameters
overview: ARRAY parameter support in coordinator plus scalar/STRUCT/TIMESTAMP result wire fixes for ~7 node Queries failures.
depends_on: [unblock-04-gateway-wire-shapes]
blocks: [unblock-09-test-isolation]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: 3-5 days
isProject: true
todos:
  - id: array-coordinator
    content: Extend local_coordinator_analyze.cc to bind ARRAY<T> parameters (not rejected at L246)
    status: pending
  - id: array-engine
    content: Mirror ARRAY binding in value_parameters.cc if coordinator delegates there
    status: pending
  - id: scalar-struct-timestamp-wire
    content: Fix parametersToEngineMap + wire.go result cells for parameterized SELECT fixtures
    status: pending
  - id: handler-tests
    content: Gateway handler tests mirroring node query-param samples
    status: pending
  - id: gate-node-queries
    content: Spot-check node Queries suite items 6–15 or filtered mocha grep Query
    status: pending
  - id: status-commit
    content: Update orchestration-status.md; lint + commit (Go + C++)
    status: pending
---

# Unblock 05 — Query parameters

## Goal

Clear **~7 node Queries failures** for ARRAY, scalar, STRUCT, and TIMESTAMP parameterized queries. Builds on partial plan 08 work (commit `5e582b1`).

## Log signatures

```
LocalCoordinatorEngine: parameter type kind 'ARRAY' is not supported by the coordinator's analyzer plumbing
expected 'Rows:' to match /word_count/
expected 'Rows:\nundefined' to match /foo/ or /BigQueryTimestamp/
```

## Architecture

```mermaid
sequenceDiagram
  participant Node as node_client
  participant GW as gateway_handlers
  participant BQ as bqtypes_query_parameter
  participant Coord as local_coordinator_analyze
  participant Engine as value_parameters
  Node->>GW: jobs.query queryParameters
  GW->>BQ: UnmarshalJSON + ValueJSON
  GW->>Coord: Analyze with parameters
  Coord->>Engine: Bind @name literals
  Engine->>GW: Row cells
  GW->>Node: JSON rows + schema
```

## Implementation

### 1. ARRAY (coordinator)

[`backend/engine/coordinator/local_coordinator_analyze.cc`](../../backend/engine/coordinator/local_coordinator_analyze.cc) ~L246 explicitly rejects ARRAY.

- Extend parameter type mapping to `ARRAY<element>` (gateway already marshals in [`gateway/bqtypes/query_parameter.go`](../../gateway/bqtypes/query_parameter.go))
- Follow STRUCT path patterns in same file / [`value_parameters.cc`](../../backend/engine/value_parameters.cc)

### 2. Scalar / STRUCT / TIMESTAMP results

- [`gateway/handlers/queries.go`](../../gateway/handlers/queries.go) `parametersToEngineMap`
- [`gateway/bqtypes/wire.go`](../../gateway/bqtypes/wire.go) — TIMESTAMP micros, STRUCT nested fields
- Ensure `assembleQueryResponse` serializes non-null cells (not `undefined` in node assertions)

### 3. Tests

```bash
go test ./gateway/handlers/... -count=1 -run 'QueryParameter|Timestamp|Struct|Array'
```

Add conformance fixture if TIMESTAMP param still gaps (optional from plan 08).

### Bazel (if engine touched)

```bash
task bazel:test -- //backend/engine/coordinator/...:*param*
```

## Node samples reference

`third_party/node-bigquery-tests/` Queries section — parameterized query tests (#6–15 in aggregator log).

## Out of scope

- Full python 34-test baseline (snippets session is mostly skipped)
- Named query parameters in DML only paths not covered by node samples

## Done criteria

- [ ] ARRAY UNIMPLEMENTED error gone for node ARRAY param tests
- [ ] Scalar/STRUCT/TIMESTAMP node query tests return matching row text
- [ ] Gateway + scoped engine tests green
