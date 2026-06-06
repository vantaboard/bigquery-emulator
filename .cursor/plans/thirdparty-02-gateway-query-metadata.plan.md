---
name: Thirdparty 02 — Gateway query & metadata
overview: Fix gateway-level query parameter JSON, dry-run stats, result pagination, dataset/table PATCH fields, and view metadata — clearing ~15 python/node failures without tp08.
depends_on: [thirdparty-01-harness]
blocks: [thirdparty-08-advanced-query-params, thirdparty-11-bigframes-gate]
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 3-5 days
isProject: true
todos:
  - id: query-param-json
    content: Flexible QueryParameterValue UnmarshalJSON (numbers/bools → string for engine)
    status: pending
  - id: dry-run-bytes
    content: Populate statistics.totalBytesProcessed on dry-run responses
    status: pending
  - id: query-pagination
    content: Implement pageToken cursoring in jobs.getQueryResults over cached QueryResult.Rows
    status: pending
  - id: metadata-patch
    content: Persist requirePartitionFilter, defaultPartitionExpirationMs, numRows/schema on get
    status: pending
  - id: view-metadata
    content: tables.get returns view.query and type=VIEW for DDL-created views
    status: pending
  - id: gateway-tests
    content: Add/extend gateway unit tests and conformance fixtures for each fix
    status: pending
---

# Thirdparty 02 — Gateway query & metadata

## Goal

Clear failures that are **gateway bugs**, not tp08 or engine semantic gaps.

## Failures addressed (from baseline log)

### Query parameters (1a)

| Suite | Test / area |
|-------|-------------|
| Node | `should run a query with named/positional params` (items 18–21) — `cannot unmarshal number into QueryParameterValue.value of type string` |
| Python | `test_client_query_w_struct_params` (partial — INT64 numeric params) |

**Files:**
- [`gateway/bqtypes/types.go`](../../gateway/bqtypes/types.go) — `QueryParameterValue` custom `UnmarshalJSON`
- [`gateway/handlers/queries.go`](../../gateway/handlers/queries.go) — `parametersToEngineMap`

**Approach:** Accept JSON number/bool/string in `parameterValue.value`; normalize to decimal string before engine RPC.

### Dry-run + pagination (1b)

| Suite | Test |
|-------|------|
| Node | `should run a query as a dry run` — missing `totalBytesProcessed` |
| Node | `jobs.test.js` `should auto-paginate through query result rows` — `expected 'Query results:' to match /name/` |

**Files:**
- [`gateway/handlers/queries.go`](../../gateway/handlers/queries.go) — dry-run `statistics`
- [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) — `jobs.getQueryResults` pageToken
- [`gateway/jobs/registry.go`](../../gateway/jobs/registry.go) — `QueryResult.Rows` already cached in full

**Approach:**
- Dry-run: emit `totalBytesProcessed: "0"` minimum; wire engine `DryRun` bytes when available
- Pagination: slice `Rows` by `maxResults` + opaque `pageToken` (start index)

### Metadata PATCH (1c)

| Suite | Test |
|-------|------|
| Python | `test_update_table_require_partition_filter` — returns `None` not `True` |
| Python | `test_update_dataset_default_partition_expiration` |
| Python | `test_get_table` — wrong field count |
| Python | `test_dataset_exists`, `test_dataset_label_samples` |

**Files:**
- [`gateway/bqtypes/`](../../gateway/bqtypes/) — Table/Dataset struct fields
- [`gateway/handlers/tables.go`](../../gateway/handlers/tables.go) — patch/update handlers
- Dataset handlers under [`gateway/handlers/`](../../gateway/handlers/)
- Engine catalog if metadata persists in C++ layer

**Fields to round-trip:**
- `requirePartitionFilter` (table)
- `defaultPartitionExpirationMs` (dataset)
- `numRows`, accurate `schema.fields` length on `tables.get`

### View metadata (1d)

| Suite | Test |
|-------|------|
| Node | `should get a view` — `Cannot read properties of undefined (reading 'query')` |

**Fix:** When a view is created via DDL query job, `tables.get` must include `view: { query: "..." }` and `type: "VIEW"`.

---

## Verification

```bash
# Fast: gateway tests
go test ./gateway/... -count=1 -run 'QueryParameter|GetQueryResults|RequirePartition|DryRun'

# Targeted thirdparty (after docker image rebuild if gateway changed)
task thirdparty:node-bigquery-tests
task thirdparty:python-bigquery-tests
```

**Success metrics (approximate):**
- Node query-suite named/positional param tests pass
- Node dry-run + auto-paginate pass
- Python partition-filter + dataset expiration tests pass

## Out of scope

- TIMESTAMP / STRUCT parameter **engine binding** (plan 08)
- tp08 load jobs
- `bigquery-public-data` tables (plan 09)
- Query-job `ALTER TABLE` schema evolution (plan 08)

## Done when

- [ ] All gateway unit tests green for new behavior
- [ ] Listed python/node tests pass on re-run
- [ ] `docs/REST_API.md` updated for any changed wire fields
