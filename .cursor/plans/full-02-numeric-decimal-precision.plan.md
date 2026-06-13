---
name: Full 02 — NUMERIC / BIGNUMERIC precision
overview: Close the exact-decimal gaps that make NUMERIC/BIGNUMERIC results wrong or rejected - aggregate results (SUM/AVG over NUMERIC), arithmetic precision/scale propagation, and the Storage Write proto-append full type matrix (extreme BIGNUMERIC) - so the Go skip rows SkipEmulatorNumericAggregateQuery and SkipEmulatorManagedWriterDefaultStream can be removed.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: reproduce-gaps
    content: "Reproduce the failures the Go skip helpers guard: SUM/AVG/aggregate over a NUMERIC column, and a ManagedWriter DefaultStream append with the full type matrix (extreme BIGNUMERIC). Capture the exact wrong/erroring behavior as failing fixtures + a gateway/e2e repro before changing code."
    status: pending
  - id: numeric-aggregate
    content: "Fix NUMERIC/BIGNUMERIC aggregate results: ensure SUM/AVG/MIN/MAX over DECIMAL columns preserve BigQuery's precision/scale rules and round-trip through Arrow -> REST as decimal strings, not float. Audit whether DuckDB's DECIMAL aggregate output is being lossily cast anywhere in the marshaling path."
    status: pending
  - id: arithmetic-precision
    content: "Audit NUMERIC arithmetic precision/scale propagation (+ - * /) against BigQuery's documented result-type rules; route the shapes DuckDB cannot match exactly to the semantic executor's decimal path rather than letting DuckDB silently widen/narrow."
    status: pending
  - id: storage-append-matrix
    content: "Storage Write proto append: extend the proto-row decode in gateway/handlers/bqstorage/ + backend/storage/duckdb append path to handle the full NUMERIC/BIGNUMERIC value range (the commit that stores BIGNUMERIC as VARCHAR is the anchor); pin extreme-magnitude values."
    status: pending
  - id: result-encoding
    content: "Verify result wire encoding matches StandardSqlDataType: INT64/NUMERIC/BIGNUMERIC as decimal strings with correct scale, no scientific notation, no float artifacts. Cross-check gateway/handlers result marshaling and the Storage Read Arrow path."
    status: pending
  - id: fixtures-skips
    content: "Add conformance/fixtures/scalar|aggregate/ NUMERIC precision fixtures; remove SkipEmulatorNumericAggregateQuery + SkipEmulatorManagedWriterDefaultStream from third_party/golang-bigquery-tests/.../emulator_skip.go and re-run task thirdparty:golang; update ROADMAP Storage Write bullet."
    status: pending
---

# Full 02 — NUMERIC / BIGNUMERIC precision

## Why

Two Go skip helpers in
`third_party/golang-bigquery-tests/bqtestutil/emulator_skip.go` exist
solely because decimals are not fully right yet:

```41:48:third_party/golang-bigquery-tests/bqtestutil/emulator_skip.go
// SkipEmulatorNumericAggregateQuery skips queries whose result includes
// SUM/aggregates over NUMERIC columns when the emulator host is set.
func SkipEmulatorNumericAggregateQuery(t *testing.T) {
	t.Helper()
	if strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_HOST")) != "" {
		t.Skip("NUMERIC aggregate query results are not yet fully supported by the emulator")
	}
}
```

NUMERIC/BIGNUMERIC are the financial/exact-decimal types; getting
aggregates or extreme magnitudes wrong is a correctness bug, not a
missing feature. The storage side already had to special-case
BIGNUMERIC (commit `f1f3d91` "store BIGNUMERIC append values as
VARCHAR"), which is a hint the decimal path is fragile.

## Key files

- [`backend/storage/duckdb/`](../../backend/storage/duckdb/) — append/scan decimal handling (BIGNUMERIC-as-VARCHAR anchor)
- [`gateway/handlers/bqstorage/`](../../gateway/handlers/bqstorage/) — proto-row decode for Storage Write
- `gateway/handlers/` result marshaling — Arrow `RecordBatch` -> REST `f`/`v` decimal-string encoding
- [`backend/engine/semantic/`](../../backend/engine/semantic/) — decimal arithmetic path for shapes DuckDB can't match
- `third_party/golang-bigquery-tests/.../emulator_skip.go` — skip rows to remove

## Steps

1. **Repro first.** Write the failing fixtures/e2e before touching code
   so the fix is pinned. A NUMERIC SUM that comes back as a float string
   or with the wrong scale is the canonical case.
2. Trace the value through the whole pipe: DuckDB DECIMAL aggregate →
   Arrow batch → REST row JSON. Find where precision is lost (likely a
   `double` hop or a default-scale cast).
3. For arithmetic shapes BigQuery defines exactly but DuckDB widens
   differently, route to the semantic executor's value path rather than
   approximating in DuckDB SQL (no-silent-approximation rule).
4. Extend the Storage Write proto decode to the full decimal range.
5. Fixtures + remove skip rows + re-run the Go lane; flip the ROADMAP
   Storage Write 🟡 bullet where the type matrix now passes.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task thirdparty:golang        # NUMERIC aggregate + ManagedWriter DefaultStream now unskipped
task bazel:shutdown && task bazel:status
```

## Out of scope

- Non-decimal type fidelity (covered elsewhere); this plan is decimals.
- Performance of decimal aggregates (correctness only).
