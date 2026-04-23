# Profiling long-running DuckDB (read) queries

Use this to separate **Go-side SQL generation** (heap / CPU in `strings.Builder` and the engine) from **DuckDB execution** (wall time in `execute_pending` / CGO).

## 1. Get DuckDB `EXPLAIN ANALYZE` *before* execution

For long read queries, `after` only logs after the query finishes. Use `before` so the plan is emitted at the start.

```bash
export GOOGLESQL_ENGINE_DUCK_EXPLAIN_ANALYZE=before
# Optional: log physical SQL with a correlation id for one statement
export GOOGLESQL_ENGINE_LOG_SQL_CORRELATION=1
```

Or use the emulator flag (rebuild the binary if the flag is missing):

```text
--duck-explain-analyze=before
--log-sql-correlation
```

## 2. Pair with heap / CPU profiles

With the default pprof sidecar (see `cmd/bigquery-emulator` / `--pprof-addr`):

```bash
# while the query is running
curl -sS 'http://127.0.0.1:6060/debug/pprof/heap?debug=1' -o /tmp/heap.txt
curl -sS 'http://127.0.0.1:6060/debug/pprof/goroutine?debug=2' -o /tmp/goroutine.txt
```

To analyze offline (replace `/path/to/bigquery-emulator-duck` with your built binary if using profile proto):

```bash
go tool pprof -text -nodecount=30 /path/to/binary /tmp/heap.pb.gz
```

## 3. Match logs to one statement

Use `--log-file` (or your process manager) and find the first `correlation_id=...` for the heavy query in the physical SQL / explain lines.

## 4. Re-run the same query after code changes

Compare:

- Wall time and `content_query_phases` debug log fields (`pre_query_context_ms`, `query_context_ms`, `row_materialize_ms`) in `contentdata.Query`
- `alloc_space` hot spots in heap profile (`strings.Builder`, `WriteQuotedIdent`, `BinaryExpression`, etc.)
- DuckDB `EXPLAIN ANALYZE` plan (dominant operators)

## 5. Script

See `scripts/duckdb-long-query-profile-sampling.sh` for curl snippets you can copy.

## 6. Checklist: re-profile after a change (same query)

1. Rebuild the DuckDB emulator binary if you added flags (`task emulator:build-duck` or your air config).
2. Run with, for example:

```text
./bigquery-emulator-duck --database .bqdata --log-file ./bigquery-emulator-duck.log \
  --pprof-addr=127.0.0.1:6060 \
  --duck-explain-analyze=before \
  --log-sql-correlation
```

3. Note the `correlation_id` on the first physical SQL line for your statement, then take heap/goroutine samples while the query runs (see section 2).
4. Compare **debug** line `content_query_phases` (`pre_query_context_ms`, `query_context_ms`, `row_materialize_ms`) before vs after, plus `go tool pprof` alloc space and the `EXPLAIN ANALYZE` plan.

`task profile:duck-long-query` prints paths to this doc and the sampling script.
