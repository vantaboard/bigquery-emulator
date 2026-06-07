---
name: FixTests 03 — C++ cc_test green (exit 37)
overview: Get task lint:cpp:test back to exit 0 in CI. The June-3 CI run died with Bazel exit 37 (internal crash) in ~4s; determine whether that is an environment/Bazel crash or a masked real test failure (exit 3), then fix the actual cause.
depends_on: [fixtests-01-foundation]
est_effort: 1-3 days
isProject: true
todos:
  - id: classify
    content: From plan 01's repro, classify the failure as Bazel internal-error (37) vs tests-failed (3). If 37, treat as environment/Bazel; if 3, bisect targets.
    status: pending
  - id: env-fix
    content: If exit 37 - inspect bazel info server_log, retry with -s --verbose_failures, check Bazelisk version + setup-bazel flags (duplicate --bes_header, PATH issues), and ensure CI runs the current lint:cpp:test that does not swallow query stderr.
    status: pending
  - id: bisect
    content: If exit 3 - bisect the 44 first-party cc_test targets (start with backend/engine/duckdb/transpiler and backend/engine/coordinator where recent work landed) and fix the failing assertions.
    status: pending
  - id: harden
    content: Harden CI logging so a future query/test failure is visible (no 2>/dev/null on bazel query; --test_output=errors retained).
    status: pending
  - id: verify
    content: GOOGLESQL_SOURCE=prebuilt task lint:cpp:test passes locally; re-run CI lane green.
    status: pending
---

# FixTests 03 — C++ cc_test green (exit 37)

## Why

CI run [26860630245](https://github.com/vantaboard/bigquery-emulator/actions/runs/26860630245) failed `Run first-party cc_test targets via Bazel` with `task: Failed to run task "lint:cpp:test": exit status 37`. Exit **37** is Bazel `INTERNAL_ERROR` (a crash), not a test assertion failure (exit **3**). The step died in ~4 seconds with almost no Bazel output, which points at a crash or an environment problem rather than 44 slow tests.

## Decision tree

```mermaid
flowchart TD
  Start[Reproduce in CI mode] --> Q{exit code}
  Q -->|37| Env[Bazel internal crash]
  Q -->|3| Bisect[Real test failure]
  Q -->|0| Done[Was stale revision; harden logging]
  Env --> ServerLog[Read bazel info server_log]
  Env --> Flags[Check Bazelisk + setup-bazel flags, PATH]
  Bisect --> Targets[Bisect 44 cc_test targets]
```

## Key facts

- Chain: `task lint:cpp:test` -> `bazel query` (discover cc_test) -> `task bazel:test` -> `googlesql:preflight` -> `bazel test --config=googlesql-prebuilt`.
- Current discovery query: `kind(cc_test, //backend/... + //binaries/... + //frontend/... + //tools/googlesql-prebuilt/smoke/...)` -> **44 targets**.
- No first-party `cc_test` is tagged flaky. Recent `backend/engine` work (UDAF wiring, routing regression tests, transpiler/disposition changes, bqutils gaps) would deterministically produce **exit 3**, not 37.
- The June-3 CI used an older `lint:cpp:test` that swallowed `bazel query` stderr (`2>/dev/null`); current [`taskfiles/lint.yml`](taskfiles/lint.yml) passes `--config={{.GOOGLESQL_BAZEL_CONFIG}}` on the query.

## Steps

1. Reproduce: `GOOGLESQL_SOURCE=prebuilt task lint:cpp:test` (see plan 01).
2. If **37**: capture `bazel info server_log`; retry `-s --verbose_failures`; verify Bazelisk version and `setup-bazel` action flags; check for duplicate `--bes_header` / bad `PATH` entries (known 37 triggers).
3. If **3**: bisect via `task bazel:test TARGETS='//backend/engine/duckdb/transpiler:...' TEST_FLAGS='--test_output=errors'`, then widen; fix the failing target(s).
4. Harden CI logging so the next failure surfaces the underlying error.

## Verify

```bash
GOOGLESQL_SOURCE=prebuilt task lint:cpp:test    # exit 0
task bazel:status                                # (clean) after; task bazel:shutdown at end
```

## Out of scope

- Conformance fixtures (plans 02, 05–07) and parity workflow (plan 04). This plan is only the cc_test lane.
