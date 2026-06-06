---
name: Unblock 10 — Final aggregator
overview: Run full task thirdparty with rebuild; publish PASS/DEFERRED summary vs post-dispatch baseline in orchestration-status.md.
depends_on: [unblock-03-bigframes-gate, unblock-08-storage-grpc, unblock-09-test-isolation]
blocks: []
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: ~0.5 day
isProject: true
todos:
  - id: pre-spawn-audit
    content: Process hygiene pre-spawn audit + free -h ≥4 GiB per process-hygiene.mdc
    status: pending
  - id: fake-gcs-sync
    content: task testdata:fake-gcs-sync if fixtures stale
    status: pending
  - id: run-aggregator
    content: THIRDPARTY_REBUILD=1 task thirdparty 2>&1 | tee .logs/thirdparty-$(date +%Y%m%d-%H%M%S).log
    status: pending
  - id: publish-status
    content: Update thirdparty-orchestration-status.md Final aggregator table vs baseline
    status: pending
  - id: cleanup
    content: task bazel:shutdown, kill emulator/gateway/docker per process-hygiene cleanup block
    status: pending
---

# Unblock 10 — Final aggregator

## Goal

Run the **full** thirdparty aggregator and publish honest results vs the post-dispatch baseline. Parent-controlled verification — do not trust prior subagent summaries alone.

## Prerequisites

All unblock plans **01–09** should be PASS or documented DEFERRED. Minimum for target zero:

| Plan | Required for |
|------|----------------|
| 01 | GCS-backed suites |
| 02 | python snippets |
| 03 | bigframes gate |
| 04–06 | node Queries + Jobs |
| 07 | java-bigquery/snippets |
| 08 | java-bigquerystorage + full java |
| 09 | node isolation remainder |

## Baseline (compare against)

From [thirdparty-orchestration-status.md](thirdparty-orchestration-status.md) / [`.logs/thirdparty-20260605-134407.log`](../../.logs/thirdparty-20260605-134407.log):

| Suite | Baseline |
|-------|----------|
| golang-bigquery-tests | OK |
| python-bigquery-tests | 1 failed |
| node-bigquery-tests | 45 failing |
| java-bigquery-tests | 1 module fail |
| dataframes-snippet-gate | 2 pass / 2 fail |

## Execution

### 1. Pre-spawn audit

```bash
ps -eo pid,ppid,etime,pcpu,pmem,rss,args \
 | grep -E 'bazel|clang\b|emulator_main|gateway_main|buildkitd' \
 | grep -vE '\bgrep\b' || echo '(clean)'
free -h | head -2
```

### 2. Run aggregator

```bash
mkdir -p .logs
THIRDPARTY_REBUILD=1 task thirdparty 2>&1 | tee .logs/thirdparty-$(date +%Y%m%d-%H%M%S).log
```

Suites run in order (from [`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml)):

1. golang-bigquery-tests
2. python-bigquery-tests
3. node-bigquery-tests
4. java-bigquery-tests
5. python-bigquery-dataframes-snippet-gate

`THIRDPARTY_FRESH_VOLUME=1` is default on aggregator.

### 3. Publish results

Update [thirdparty-orchestration-status.md](thirdparty-orchestration-status.md):

- New **Unblock lane** section or extend **Final aggregator** table
- Log path, exit code, per-suite pass/fail counts
- List any DEFERRED items with blockers

### 4. Target

| Suite | Target |
|-------|--------|
| golang | OK |
| python snippets | 0 failed |
| node | 0 failing |
| java | 0 (no unexpected ITs) |
| bigframes gate | 4 pass |

Exit **0** from `task thirdparty` is the success criterion.

### 5. Cleanup (mandatory)

```bash
task bazel:shutdown
task bazel:kill-strays
docker compose down -v --remove-orphans 2>/dev/null || true
pgrep -af 'emulator_main|gateway_main' | grep -v grep | awk '{print $1}' | xargs -r kill -TERM
```

## On failure

Do **not** restart the full aggregator in a loop. Instead:

1. Identify failing suite from summary
2. Map failures to unblock plan 01–09
3. Dispatch targeted subagent on that plan only
4. Re-run plan 10 after fix

## Out of scope

- New feature implementation
- Editing unblock-00-index or individual plan files (unless documenting DEFER)

## Done criteria

- [ ] Tee log under `.logs/thirdparty-*.log`
- [ ] orchestration-status.md updated with final table
- [ ] Process catalog clean after cleanup block
