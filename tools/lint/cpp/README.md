# `tools/lint/cpp`

Source-only C++ lint runner for the BigQuery emulator. The binary
backs the fast `task lint:cpp:source` gate and the source-list helper
that the heavier `task lint:cpp:format`, `task lint:cpp:tidy`, and
`task lint:cpp:cppcheck` lanes consume.

## What it does

Three classes of rule that the standard C++ tooling does not cover
well in this repo:

1. **First-party source list.** `cpp-lint list` prints the canonical
   set of repo-owned C++ files (one path per line). Every C++ lint
   task pipes that list into `clang-format`, `clang-tidy`, etc., so
   GoogleSQL, generated proto code, vendored trees, Bazel outputs,
   and the `.cache/` artifacts can never sneak into a lint run. The
   include / exclude rules are pinned in `sources.go` and tested in
   `sources_test.go` — that test is the "tested invariant of the
   ownership boundary" the rollout plan calls out.
2. **Whole-file line cap.** `cpp-lint check` rejects first-party
   `.cc`/`.h` files over 500 lines. `clang-tidy`'s
   `readability-function-size` does not enforce a per-file cap, so
   this rule lives here.
3. **Repo-specific anti-patterns.**
   - `banned-logging` — production C++ may not call `std::cout`,
     `std::cerr`, `std::clog`, `std::printf`, `std::fprintf`,
     `printf`, or `fprintf`. Tests, the `tools/googlesql-prebuilt/smoke`
     binaries, and `binaries/emulator_main/main.cc` are exempt
     (they have legitimate startup-error reporting paths today).
   - `status-discarded` — common engine APIs that return
     `absl::Status` (`ExecuteDdl`, `AppendRows`, `OverwriteRows`,
     `DropTable`) must capture their result. Drops the result
     when called as a statement.
   - `statusor-unchecked-value` — `.value()` on a
     `absl::StatusOr<T>` without a nearby `.ok()` / status guard
     is reported. The look-back window is five preceding lines so
     the canonical pattern (`if (!s.ok()) return s.status();
     auto v = std::move(s).value();`) does not generate noise.

These are deliberately conservative checks. The compile-aware
mechanism for the same concerns is `clang-tidy`'s
`bugprone-unused-return-value`, `bugprone-unchecked-optional-access`,
and the matching `[[nodiscard]]` annotations on first-party headers
(both gating in the slow CI lane). The Go binary keeps
`task lint:run` snappy and the rules language-agnostic-friendly,
the way `tools/coverage` does for the coverage pipeline.

## Subcommands

```bash
go run ./tools/lint/cpp list                        # print first-party C++ paths
go run ./tools/lint/cpp list -tests=false           # skip *_test.cc entries
go run ./tools/lint/cpp check                       # run source-only checks
```

The corresponding Task targets (`task lint:cpp:source`) wrap these
so contributors do not have to remember the binary path;
`task lint:run` includes `task lint:cpp:source` in the fast
pre-commit gate.

## Inline suppressions

When a rule is wrong for a specific line — for example a startup
diagnostic that has no `absl::Status` return path — annotate the
line with a marker that names the rule and a one-line reason:

```cpp
std::fprintf(stderr,  // cpp-lint:allow(banned-logging) -- pre-gRPC bootstrap diagnostic
             "[frontend::Server] failed to bind gRPC server on %s\n",
             options.server_address.c_str());
```

The format is intentionally strict, mirroring the Go `nolintlint`
posture in [`.golangci.yml`](../../../.golangci.yml):

- The marker must name at least one rule.
- A reason after `--` is required.

A bare `// cpp-lint:allow` without a reason is rejected so audit
trails stay readable.

## Adding a new check

1. Add the rule helper to `checks.go` (named `checkXxx`) and a
   matching `ruleXxx` constant. Write the helper to take a
   pre-split `[]string` of lines so the suppression collector can
   reuse the buffer.
2. Wire the helper into `runOnce`.
3. Add table-driven tests to `checks_test.go`. The minimum bar is
   one positive case, one negative case (a literal that should NOT
   match), and one inline-suppression case.
4. Document the rule name and rationale in this README so authors
   can grep for it from their editor diagnostics.

## Adding a new first-party C++ tree

Edit `firstPartyIncludeRoots` in `sources.go` and pin the new path
in the `TestFilterFirstParty` table. The tests fail loudly if a
new tree is added without a corresponding fixture entry, which
keeps the ownership boundary auditable.

## Why a Go binary instead of a shell script

The repo already ships a `tools/coverage` Go binary for the same
"small, dependency-free, easy to test" reasons; following the same
pattern means contributors recognise the layout and the test
harness (`go test ./tools/lint/cpp/...`) is the same one they use
for every other Go change.
