# C++ lint and formatting policy

This guide is the developer-facing reference for the C++ lint stack
that ships with the BigQuery emulator. It covers commands,
thresholds, and the suppression / baseline policy. The runner
implementations live in [`tools/lint/cpp/`](../../tools/lint/cpp/);
the rollout history and rationale live alongside the relevant plan
under `.cursor/plans/`.

## Architecture at a glance

```
                   .clang-format
                      │
+-----------------+   │   +-----------------------+
| tools/lint/cpp  | <-+   | .clang-tidy           |
|   (Go binary)   |       |   (compile-aware)     |
|                 |       +-----------+-----------+
|  list           |                   │
|  check          |       +-----------v-----------+
|  baseline       |       | clang-tidy            |
+--------+--------+       +-----------+-----------+
         │                            │
         │  canonical                 │
         │  first-party file list     │
         │                            │
         v                            v
   clang-format           clang-tidy / cppcheck

                fast lane (task lint:run)
                slow lane (task ci:cpp-analysis,
                          task lint:cpp:tidy,
                          task lint:cpp:cppcheck)
```

The Go binary is the spine: every C++ tool consumes the
first-party file list it produces, so vendored / generated /
cached trees can never sneak into a lint run by accident. The
ownership boundary is pinned by
[`tools/lint/cpp/sources_test.go`](../../tools/lint/cpp/sources_test.go)'s
`TestFilterFirstParty` table.

## Commands

| Use case | Command | Notes |
|---|---|---|
| Fast pre-commit gate | `task lint:run` | Go vet, gofmt, clang-format, source-only C++ checks. |
| Apply autofixes | `task lint:fix` | gofmt, clang-format, go vet. |
| Format C++ only | `task lint:cpp:format` / `task lint:cpp:format-fix` | Reads from `tools/lint/cpp list`. |
| Source-only C++ checks | `task lint:cpp:source` | File length, banned logging, status anti-patterns. |
| Regenerate baseline | `task lint:cpp:baseline` | Use after a refactor that splits an oversized file. |
| clang-tidy (slow) | `task lint:cpp:tidy` | Requires `compile_commands.json`. |
| Generate compile DB | `task lint:cpp:compile-commands` | Uses `bazel aquery`; takes minutes on a cold tree. |
| cppcheck (slow) | `task lint:cpp:cppcheck` | Secondary-analysis lane. |
| First-party `cc_test` | `task lint:cpp:test` | Discovers via `bazel query`; reuses `task bazel:test`. |
| Match CI locally | `task ci:run` / `task ci:cpp-analysis` | Both lanes — fast (blocking) and slow (warning-only). |

## What each tool covers

### `clang-format` (`task lint:cpp:format`)

Profile: [`.clang-format`](../../.clang-format) — Google style,
80-column limit, two-space indent, paren-aligned continuation
arguments, includes regrouped, namespace closers required. Editor
format-on-save picks it up via clangd's integration; CI runs the
dry-run / `--Werror` variant on every PR.

### `clang-tidy` (`task lint:cpp:tidy`)

Profile: [`.clang-tidy`](../../.clang-tidy). Checks pulled from
four buckets:

| Bucket | Examples |
|---|---|
| Correctness / runtime risk | `bugprone-*`, `clang-analyzer-*`, `cppcoreguidelines-narrowing-conversions`, `cppcoreguidelines-init-variables` |
| Complexity | `readability-function-size` (line/statement/branch/nesting/parameter caps), `readability-function-cognitive-complexity` |
| Modernization | `modernize-*`, `performance-*`, `portability-*` |
| Style maintainability | `readability-*` (with the noisy ones turned off) |

Header filter: `^(backend|binaries|frontend|tools/googlesql-prebuilt/smoke)/.*\.(h|hpp|hh)$`.
Without it, every Abseil / GoogleSQL / gRPC header pulled in by an
include would emit findings the rule set was never tuned for.

Posture: warning-only today. The CI job
([`.github/workflows/ci.yml`](../../.github/workflows/ci.yml))
runs the lane with `continue-on-error: true`; the rollout plan
flips the gate to required once the first sweep is clean and any
intentional findings have been baselined.

### `cppcheck` (`task lint:cpp:cppcheck`)

Profile in [`taskfiles/lint.yml`](../../taskfiles/lint.yml):
`--enable=warning,style,performance,portability` with
`--inline-suppr` and `--error-exitcode=1`. Runs in the same slow
CI lane as clang-tidy. cppcheck is well-positioned to find
uninitialised state, bounds issues, resource leaks, and
null-handling mistakes that the templated abstractions in
clang-tidy can occasionally miss.

### Source-only checker (`task lint:cpp:source`)

Implemented in [`tools/lint/cpp/`](../../tools/lint/cpp/). Three
rules:

1. **`file-length`** — first-party `.cc`/`.h` files over 500 lines
   fail unless they are baselined in
   [`tools/lint/cpp/baseline.txt`](../../tools/lint/cpp/baseline.txt).
2. **`banned-logging`** — `std::cout`, `std::cerr`, `std::clog`,
   `std::printf`, `std::fprintf`, `printf`, `fprintf` are banned in
   production C++. Tests, the `tools/googlesql-prebuilt/smoke/`
   binaries, and `binaries/emulator_main/main.cc` are exempt; one
   pre-gRPC bootstrap call inside `frontend/server/server.cc` is
   suppressed inline (see "Suppressions" below).
3. **`status-discarded`** / **`statusor-unchecked-value`** —
   common engine APIs that return `absl::Status` (`ExecuteDdl`,
   `AppendRows`, `OverwriteRows`, `DropTable`) must capture their
   result. `.value()` on a `StatusOr<T>` without a nearby `.ok()`
   guard is reported.

The same rules are reinforced at the compiler boundary by the
`[[nodiscard]]` attributes on the abstract
[`Storage`](../../backend/storage/storage.h) and
[`Engine`](../../backend/engine/engine.h) interfaces, and by the
scoped `-Wall -Wextra` profile in
[`.bazelrc`](../../.bazelrc) (`--per_file_copt`).

## Thresholds

The thresholds below are pinned in code and configuration; bump
them only with a recorded rationale in the commit message.

| Threshold | Value | Source | Rationale |
|---|---|---|---|
| Whole-file lines (`.cc`/`.h`) | 500 | `tools/lint/cpp` source-only checker (`-max-lines`) | Mirrors the Go `revive` `file-length-limit` and the rollout plan's bar for "split before it gets unscanable". |
| Function lines | 80 | `readability-function-size.LineThreshold` | Roughly two screens; matches the Go cyclop rule of thumb. |
| Function statements | 60 | `readability-function-size.StatementThreshold` | |
| Function branches | 12 | `readability-function-size.BranchThreshold` | |
| Function nesting | 4 | `readability-function-size.NestingThreshold` | Caps the `if / for / if / for / if` cascade. |
| Function parameters | 8 | `readability-function-size.ParameterThreshold` | |
| Cognitive complexity | 25 | `readability-function-cognitive-complexity.Threshold` | Matches `cyclop`'s `max-complexity: 25` in `.golangci.yml`. |

The whole-file cap is the only rule with a baseline ratchet today.
Per-function complexity findings are surfaced by clang-tidy's
warning-only lane and are expected to be addressed inline.

## Baselines

[`tools/lint/cpp/baseline.txt`](../../tools/lint/cpp/baseline.txt)
grandfathers files that already exceed the 500-line cap. New
oversized files always fail; the baseline only protects existing
code so the lint gate could land without a sweeping refactor.

Every entry should map to a follow-up issue tracking the eventual
split. Removing an entry is a one-liner once the file is below the
cap; regenerate via `task lint:cpp:baseline` after the refactor.

Do **not** add a file to the baseline as a workaround for a new
violation. The whole point of the ratchet is that the threshold
gets tighter over time, not looser.

## Suppressions

C++ rules that allow inline suppressions (the source-only
checker's `banned-logging`, `status-discarded`, and
`statusor-unchecked-value` rules) accept a marker of the form:

```cpp
// cpp-lint:allow(rule-name) -- one-line reason
```

Format requirements:

- The marker names at least one rule. A bare
  `// cpp-lint:allow` (no rule list) does not suppress.
- A `--reason` body is mandatory. Suppressions without a reason
  silently fail to apply, surfacing the original finding.
- The marker may sit on the same line as the offending
  construct or on the line immediately above it. The above-line
  variant is preferred because clang-format may wrap a long
  trailing comment over multiple lines (which would split the
  marker text).

Examples:

```cpp
// Same-line trailing marker (works when the marker fits within
// the column limit):
absl::Status ignore_me = backend.AppendRows(id, rows);  // cpp-lint:allow(status-discarded) -- demo only

// Above-line marker (preferred for production code; clang-format
// cannot wrap the comment off the marker line):
// cpp-lint:allow(banned-logging) -- pre-gRPC bootstrap diagnostic; Server::Create has no Status return today
std::fprintf(stderr, "[frontend::Server] failed to bind\n");
```

The marker contract is enforced by the source-only checker's
test suite
([`tools/lint/cpp/checks_test.go`](../../tools/lint/cpp/checks_test.go)).

`clang-tidy` and `cppcheck` keep their own suppression mechanisms
(`// NOLINT(check-name)` and `// cppcheck-suppress`, respectively).
Mirror the same "name a rule + give a reason" discipline when
using either: a bare `NOLINT` is rarely the right answer.

## CI surface

| Job | Workflow | Posture |
|---|---|---|
| `build-and-test` | [`ci.yml`](../../.github/workflows/ci.yml) | Required. Includes `task lint:run` (gofmt, vet, clang-format, source-only C++) and `task lint:cpp:test`. |
| `cpp-analysis` | [`ci.yml`](../../.github/workflows/ci.yml) | `continue-on-error: true`. Runs `task lint:cpp:cppcheck` today; the rollout plan adds `task lint:cpp:tidy` once the compile-commands generator is settled and the first sweep is clean. |

The local mirror is `task ci:run` (required-equivalent) and
`task ci:cpp-analysis` (warning-only).

## Adding a new first-party C++ tree

1. Add the new top-level directory to
   `firstPartyIncludeRoots` in
   [`tools/lint/cpp/sources.go`](../../tools/lint/cpp/sources.go).
2. Pin the new path in `TestFilterFirstParty` so the ownership
   boundary stays auditable.
3. Mirror the path in
   [`.bazelrc`](../../.bazelrc)'s `--per_file_copt` block so
   `-Wall -Wextra` apply to the new tree.
4. Mirror the path in
   [`.clang-tidy`](../../.clang-tidy)'s `HeaderFilterRegex`.
5. Mirror the path in
   [`tools/lint/cpp/compile_db.py`](../../tools/lint/cpp/compile_db.py)'s
   `FIRST_PARTY_PREFIXES`.

The five places must stay in sync; a missing edit usually
manifests as either silent skipping (lint never sees the new
files) or a wall of upstream-tree findings (the new files are
treated as first-party but their includes drag in upstream
warnings).

## Tooling versions

| Tool | Version | Source |
|---|---|---|
| clang-format | 18.1.x | `apt install clang-format` (Ubuntu) or `brew install clang-format`; pinned by the matching clang-18 toolchain Bazel uses for builds. |
| clang-tidy | 18.1.x | `apt install clang-tidy` (Ubuntu) or `brew install llvm`. |
| cppcheck | 2.13.x | `apt install cppcheck` (Ubuntu) or `brew install cppcheck`. |
| Bazel | per `mise.toml` | `mise install`. |
| Go | per `mise.toml` | `mise install`. |

`mise.toml` does not pin the clang-* lint tools today because
they ship outside `mise`'s tool index. The README's "Local
development setup" section calls out the manual install step.
Upgrading the apt-shipped clang-format / clang-tidy / cppcheck
versions in CI requires bumping the same versions locally;
otherwise contributors and CI will disagree on formatting output.
