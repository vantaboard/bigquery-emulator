# Long DuckDB queries: wall clock vs heap profiling

## Wall clock (A/B)

For steady-state **execution time** on DuckDB reads, set **`--duck-explain-analyze=off`** (or leave unset / empty). Modes **`before`** and **`after`** set `GOOGLESQL_ENGINE_DUCK_EXPLAIN_ANALYZE` in go-googlesql-engine and run **extra** `EXPLAIN` / `EXPLAIN ANALYZE` around the main query, which can roughly **double** DuckDB work when you are trying to measure wall time.

Use **`before`** or **`after`** when you need an execution plan or analyze timing for its own sake, not as a baseline for raw query speed.

## Heap / SQL size (pprof)

1. Start the emulator with a pprof listen address, e.g. **`--pprof-addr=127.0.0.1:6060`**.
2. While a long query runs, poll heap profiles with [`scripts/pprof-heap-poll.sh`](../scripts/pprof-heap-poll.sh) (see script header for `PPROF`, `INTERVAL`, `OUT`).
3. Compare **before/after** codegen or engine changes, for the **same** workload and env:

   ```bash
   go tool pprof -top -inuse_space /path/to/heap-*.pb.gz
   ```

   Inspect hot frames such as `strings.(*Builder).WriteString` and paths through `formatSQLFragment` / `WriteSql` if physical SQL string building dominates.

4. Optional: set **`--log-sql-correlation`** so physical SQL logs include a **correlation_id** that lines up with pprof samples for a given read.

## Verification checklist (Springs-style jobs)

- Same query, data volume, and emulator flags except the variable under test.
- **Flat** in-use `WriteString` or total in-use heap lower on mid-query heap samples after UNNEST/struct codegen fixes.
- Shorter or bounded **`googlesqlengine physical sql`** line length in logs when `GOOGLESQL_ENGINE_LOG_PHYSICAL_SQL=1` (truncated; use correlation + DuckDB for full SQL if needed).
