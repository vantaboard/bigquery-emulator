---
name: Unblock 09 — Test isolation sweep
overview: Fix THIRDPARTY_FRESH_VOLUME propagation, node UUID dataset collisions, and cascade Not Found failures after upstream feature plans land.
depends_on: [unblock-04-gateway-wire-shapes, unblock-05-query-params, unblock-06-load-avro-orc]
blocks: [unblock-10-final-aggregator]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: ~1 day
isProject: true
todos:
  - id: audit-fresh-volume
    content: Confirm THIRDPARTY_FRESH_VOLUME=1 propagates from task thirdparty aggregator to child suite tasks
    status: pending
  - id: audit-node-setup
    content: Review third_party/node-bigquery-tests/test/setup.js UUID dataset naming vs gateway persistence
    status: pending
  - id: fix-already-exists
    content: Fix remaining Already Exists on supposedly fresh volume (3 baseline failures)
    status: pending
  - id: fix-cascade-not-found
    content: Fix upstream failures causing Views update Not Found (#45) and dataset Not Found (#5)
    status: pending
  - id: gate-node-spot
    content: Re-run node-bigquery-tests; target 0 failing or document DEFERRED with per-test list
    status: pending
  - id: status-commit
    content: Update orchestration-status.md; lint + commit
    status: pending
---

# Unblock 09 — Test isolation sweep

## Goal

Clear **residual node failures** that are test-harness / catalog isolation issues rather than missing features — typically **3+2 cascade** failures at baseline after feature work from plans 01–07.

## Baseline patterns

| Pattern | Count | Example |
|---------|-------|---------|
| `Already Exists: Dataset/Table dev:nodejs_samples_tests_...` | 3 | Fresh volume should not collide |
| Cascade `Not found: Dataset dev:...` | 1 | Downstream of failed create (#5) |
| Cascade `Not found: Table dev:...view` | 1 | Downstream of failed view create (#43–45) |

From [`.logs/thirdparty-20260605-134407.log`](../../.logs/thirdparty-20260605-134407.log) node section.

## Prerequisites

Run **after** plans 04–06 so wire shapes, query params, and load jobs are not masquerading as isolation failures.

Plans 01–02 fix seed-related View failures; re-verify before blaming isolation.

## Investigation checklist

### 1. THIRDPARTY_FRESH_VOLUME

[`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) default aggregator sets `THIRDPARTY_FRESH_VOLUME=1` (L153).

Verify child `task thirdparty:node-bigquery-tests` inherits env:

```bash
THIRDPARTY_FRESH_VOLUME=1 task thirdparty:node-bigquery-tests 2>&1 | head -20
# Should log emulator-up wipe of bq-emulator-data
```

If child tasks spawn without inherited export, fix task YAML to pass env explicitly.

### 2. Node setup.js

[`third_party/node-bigquery-tests/test/setup.js`](../../third_party/node-bigquery-tests/test/setup.js):

- UUID suffix on dataset IDs
- `before` hook create vs emulator stale catalog
- Whether tests assume delete-on-teardown that emulator doesn't implement

### 3. Gateway metadata

[`gateway/handlers/metadata_store.go`](../../gateway/handlers/metadata_store.go) (or equivalent):

- Dataset delete actually removes id from store
- List vs get consistency on empty project
- 404 vs empty list for missing dataset (wrong behavior causes cascade)

### 4. Cascade triage

For each `Not Found` failure:

1. Identify preceding test in same `describe` block that failed first
2. Fix root test; cascade often clears without isolation fix

## Fast gate

```bash
go test ./gateway/handlers/... -count=1 -run 'Dataset|Delete|Metadata'
```

## Slow gate

```bash
THIRDPARTY_FRESH_VOLUME=1 THIRDPARTY_REBUILD=1 task thirdparty:node-bigquery-tests
```

Capture failing count vs baseline 45. Target: **0** or honest DEFERRED list per test name.

## Out of scope

- New feature work (belongs in plans 04–08)
- Changing upstream node test sources in `third_party/` unless harness contract bug is proven

## Done criteria

- [ ] Two consecutive node runs with `THIRDPARTY_FRESH_VOLUME=1` show no `Already Exists` on first dataset create
- [ ] Cascade `Not Found` failures eliminated or traced to specific DEFERRED feature gaps
- [ ] Node failure count documented in orchestration-status
