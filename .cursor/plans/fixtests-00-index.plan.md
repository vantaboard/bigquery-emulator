---
name: FixTests 00 — Index & dispatch
overview: Master index for closing every remaining test failure (first-party conformance, CI fastpath, cc_test, googlesql-parity, third-party) plus shrinking the bqutils known_failing corpus. Split into self-contained sub-plans so each can be handed to a separate subagent.
parent_plan: fix_remaining_test_failures_26f123d3
isProject: true
---

# Fix remaining test failures — subagent dispatch index

This index replaces the monolith at [fix_remaining_test_failures_26f123d3.plan.md](fix_remaining_test_failures_26f123d3.plan.md). Each linked plan is **self-contained**: scope, key files, steps, verification, dependencies, and out-of-scope boundaries. Run in number order within a track; the two tracks run in parallel.

Orchestration playbook: [fixtests-dispatch.plan.md](fixtests-dispatch.plan.md) (or `fixtests_subagent_dispatch_aa7f9f06.plan.md` in Cursor plans).

## Failure inventory (final — 2026-06-08)

| Lane | Status | Scope |
|------|--------|-------|
| Go packages (`task test:run`) | Green | — |
| **First-party conformance** (`task conformance:run`) | **Green** | **100/100** |
| **CI fastpath** (`task conformance:fastpath`) | **Green** | **20/20** |
| **C++ cc_test** (`task lint:cpp:test`) | **Green** | 44/44 exit 0 |
| **bqutils passing** (`task conformance:bqutils`) | **Mostly green** | **105/111** (6 legacy fails in passing/) |
| bqutils known_failing | **6 remain** | 5 engine blockers + 1 deferred Python UDF |
| googlesql-parity | Improved (plan 04) | artifact gating, smoke/full split |
| **Third-party** | **Partial** | node+java green; python 1 fail; golang 1 fail; bigframes 1 fail |

## Sub-plans

### Track A — CI green + first-party conformance

| # | Plan file | State | Notes |
|---|-----------|-------|-------|
| 01 | [fixtests-01-foundation.plan.md](fixtests-01-foundation.plan.md) | done | triage in docs/dev/fixtests-triage.md |
| 02 | [fixtests-02-quickwins.plan.md](fixtests-02-quickwins.plan.md) | done | 3d3e45a |
| 03 | [fixtests-03-cc-test.plan.md](fixtests-03-cc-test.plan.md) | done | bb52b45; 44/44 cc_test |
| 04 | [fixtests-04-ci-parity-infra.plan.md](fixtests-04-ci-parity-infra.plan.md) | done | c187084 |
| 05 | [fixtests-05-scalar-subquery.plan.md](fixtests-05-scalar-subquery.plan.md) | done | 1d466a4; 94/100 |
| 06 | [fixtests-06-fastpath-scans.plan.md](fixtests-06-fastpath-scans.plan.md) | done | bd8bbbf; fastpath 20/20 |
| 07 | [fixtests-07-advanced-relational.plan.md](fixtests-07-advanced-relational.plan.md) | done | 481adea; 100/100 |

### Track B — bqutils known_failing

| # | Plan file | State | Notes |
|---|-----------|-------|-------|
| 08 | [fixtests-08-bqutils-any-type.plan.md](fixtests-08-bqutils-any-type.plan.md) | done | a9843b5; 61→67 |
| 09 | [fixtests-09-bqutils-bytes.plan.md](fixtests-09-bqutils-bytes.plan.md) | done | e092f11; 67→75 |
| 10 | [fixtests-10-bqutils-range.plan.md](fixtests-10-bqutils-range.plan.md) | done | f5d5a90; 75→81 |
| 11 | [fixtests-11-bqutils-regexp.plan.md](fixtests-11-bqutils-regexp.plan.md) | done | 0e38872; 81→95 |
| 12 | [fixtests-12-bqutils-bignumeric.plan.md](fixtests-12-bqutils-bignumeric.plan.md) | done | c243c54; 95→102 |
| 13 | [fixtests-13-bqutils-tail.plan.md](fixtests-13-bqutils-tail.plan.md) | done | b8148bb+; 102→111; 6 known_failing remain |

### Convergence

| # | Plan file | State | Notes |
|---|-----------|-------|-------|
| 14 | [fixtests-14-thirdparty-full.plan.md](fixtests-14-thirdparty-full.plan.md) | done | c69ce7f; node+java green; CI python/node/bigframes jobs added |

## Track B triage loop

```bash
task emulator:build-engine:bazel
./scripts/triage_bqutils_fixtures.sh
task conformance:bqutils
```

## Verification matrix

| Check | Command | Target | Actual |
|-------|---------|--------|--------|
| Go | `task test:run` | all pass | green |
| CI fastpath | `task conformance:fastpath` | 20/20 | **20/20** |
| Full conformance | `task conformance:run` | 100/100 | **100/100** |
| bqutils gate | `task conformance:bqutils` | ~117/117 | 105/111 (6 legacy in passing/) |
| C++ tests | `GOOGLESQL_SOURCE=prebuilt task lint:cpp:test` | exit 0 | **exit 0** |
| Third-party | `task thirdparty` | all suites green | node+java green; python/golang/bigframes partial |
| Parity | scheduled `googlesql-parity` | smoke legs comparable | improved (plan 04) |

## Remaining follow-ups (outside fixtests dispatch)

- **bqutils:** 6 `known_failing/` (t_test, url_trim_query, cw_disjoint_partition_by_regexp, migration/redshift/translate, migration/sqlserver/convert_datetime_string, cw_xml_extract deferred Python)
- **Third-party:** thirdparty-00-index sub-plans for golang/python/bigframes/dbt gaps
- **Push:** branch ahead of origin; user approval needed for `git push` + CI watch
