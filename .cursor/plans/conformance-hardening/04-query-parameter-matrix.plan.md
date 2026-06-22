# 04 — Query-parameter wire-format matrix (driven through real clients)

- **Series:** conformance-hardening (plans 01–08). Run sequentially.
- **Sequencing:** fourth. Reuses the differential oracle from plan 01 to pin
  "what BigQuery actually accepts." Do plan 01 first.
- **Priority:** P2.

## Why this exists (origin)

A dashboard query broke with:

```
400 ... InvalidArgument ... semantic: invalid TIMESTAMP parameter value '2026-06-22T10:00:00'
```

The user passed a naive ISO-8601 string (no `Z`/offset) as a TIMESTAMP
`ScalarQueryParameter`. Production BigQuery accepts that; the emulator rejected
it. The user only got past it by hand-appending `Z`. Notably, a sample test for
timestamp params **already exists**
(`third_party/python-bigquery-tests/samples/tests/test_client_query_w_timestamp_params.py`)
— but it passes a **timezone-aware `datetime`**, which the client serializes with
an offset, so the real failure mode (a naive string) was never exercised. The
lesson: parameter parsing diverges from BigQuery across the *input space*, and
the happy-path sample didn't cover it.

## Current state (grounded)

- Engine-side parameter parsing: `backend/engine/semantic/value_parameters.cc`
  (+ `backend/engine/semantic/value.h`).
- Gateway-side parameter wire decoding: `gateway/bqtypes/query_parameter.go`,
  `gateway/bqtypes/query_wire_types.go`, with tests in
  `gateway/bqtypes/query_parameter_test.go` and
  `gateway/handlers/queries_parameters_test.go`.
- Sample coverage exists per type
  (`client_query_w_{timestamp,named,positional,array,struct}_params.py`) but
  asserts a single happy value each, not the accepted-format space.

## Goal / done-criteria

1. A **parameter format matrix** documenting, per BigQuery scalar type, every
   string form the API accepts and the canonical parsed value — pinned from
   production BigQuery (via plan 01's oracle or `bq`).
2. Emulator accepts every BigQuery-accepted form and rejects every
   BigQuery-rejected form, **at both layers** (gateway decode + engine parse),
   for at least: TIMESTAMP (naive `YYYY-MM-DD HH:MM:SS`, ISO with `T`, with `Z`,
   with `±HH:MM`, fractional seconds, date-only), DATETIME, DATE, TIME, NUMERIC,
   BIGNUMERIC, BOOL, BYTES (base64), plus array/struct of these.
3. The naive-TIMESTAMP case specifically passes end-to-end through the Python
   client.
4. Coverage lives in three places: an engine unit test (value_parameters), a
   gateway unit test (wire decode), and a client-driven case (real serialization).

## Implementation steps

### Step 1 — Pin the matrix from BigQuery
- Add the parameter cases to plan 01's differential corpus
  (`conformance/differential/corpus/params_*`) — e.g.
  `SELECT @p AS v` with `@p` bound to each candidate form — and record the
  oracle (accepted value or the exact 400 message). This is the source of truth.

### Step 2 — Fix engine parsing
- In `backend/engine/semantic/value_parameters.cc`, make TIMESTAMP accept the
  naive ISO form (`2026-06-22T10:00:00`) and the space form, defaulting to UTC
  exactly as BigQuery does; align DATETIME/DATE/TIME acceptance. Add a focused
  unit test parameterized over the matrix.

### Step 3 — Fix/verify gateway decode
- Ensure `gateway/bqtypes/query_parameter.go` forwards the raw string forms
  without lossy normalization, and extend `query_parameter_test.go` /
  `queries_parameters_test.go` with the matrix.

### Step 4 — Strengthen the client-driven samples
- Add a first-party scenario (under the plan 05 client-scenario tree, or a new
  `gateway/e2e` case) that binds a **naive** datetime/string TIMESTAMP param via
  the Python/Go client and asserts success — covering the exact reported failure
  the upstream sample missed. Do **not** weaken the upstream sample; add
  alongside.

### Step 5 — Wire into the differential lane
- The param corpus cases run in `task conformance:differential` (plan 01) so any
  future divergence is caught automatically.

## Tests
- `backend/engine/semantic/value_parameters_test.cc` (matrix-parameterized).
- `gateway/bqtypes/query_parameter_test.go` + `queries_parameters_test.go`
  (matrix).
- Client-driven naive-TIMESTAMP scenario (plan 05 tree or `gateway/e2e`).
- Differential corpus `params_*` cases green against the recorded oracle.

## Process hygiene (repo rules)
Engine unit tests build the C++ engine — follow the bazel + process hygiene rules
(single invocation, throttled, cleanup + `task bazel:status` clean at the end).

## Out of scope
- Query parameters in DDL/scripting contexts beyond scalar SELECT binding.
- Locale/timezone-name parsing beyond what BigQuery accepts for these types.

## Touch list
`backend/engine/semantic/value_parameters.cc` (+ test + `BUILD.bazel`),
`gateway/bqtypes/query_parameter.go`, `gateway/bqtypes/query_parameter_test.go`,
`gateway/handlers/queries_parameters_test.go`,
`conformance/differential/corpus/params_*`, a client-driven scenario file.
