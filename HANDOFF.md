# Handoff — bigquery-emulator roadmap orchestration

**Audience:** a fresh Cursor (or other agentic IDE) instance with **no prior conversation history**. Read this top-to-bottom; it is self-contained and points you at every other artifact you need.

**Snapshot date:** 2026-05-25
**Last commit on `main`:** see `git log -1 --oneline` (this doc was committed alongside the WIP checkpoint described in §4).

---

## 1. What this project is

`bigquery-emulator` is a locally-runnable emulator of Google's BigQuery REST API, modeled on `cloud-spanner-emulator`'s split:

- A **Go REST gateway** (`gateway_main`, package layout under `gateway/`) implementing the BigQuery public surface (projects/datasets/tables/jobs/queries/insertAll/...).
- A **C++ engine subprocess** (`emulator_main`, layout under `frontend/` + `backend/`) that links GoogleSQL directly and exposes Catalog + Query gRPC services to the gateway.
- **DuckDB engine** (lowers GoogleSQL-analyzed AST to DuckDB SQL) backed by **DuckDB+Parquet storage**, behind two narrow C++ interfaces (`backend/engine/engine.h`, `backend/storage/storage.h`). The original ReferenceImpl engine and in-memory storage were removed once DuckDB reached parity for the supported surface; see `docs/ENGINE_POLICY.md`.

Authoritative project intro: [`ROADMAP.md`](ROADMAP.md). Read its first ~100 lines before doing anything else.

Build systems:

- **Bazel** is the sole build system for the C++ engine (the analyzer + DuckDB engine + the production `emulator_main`). GoogleSQL is Bazel-only and is consumed via `local_path_override` to a sibling checkout at `/home/brighten-tompkins/Code/go-googlesql` (see `MODULE.bazel`).
- **Go modules** for everything in `gateway/`.

Tooling pins live in `mise.toml` (with `direnv` driving `.envrc`); `Taskfile.yml` exposes `task lint:*`, `task ci:run`, `task emulator:build-engine:bazel`, etc.

### GoogleSQL build mode (operator-visible default)

`task emulator:build-engine:bazel` defaults to **prebuilt GoogleSQL**: it consumes a published artifact from `.cache/googlesql-prebuilt/` and skips the ~8K C++ TU GoogleSQL compile. The source build is one explicit env var away — `GOOGLESQL_SOURCE=local task emulator:build-engine:bazel` — and is reserved for upstream upgrades or producer debugging. There is no silent fallback when the prebuilt cache is empty; the pre-flight gate refuses the build with an actionable message.

Maintainer surface (commands, workflows, runbooks):

| Surface | Where |
|---|---|
| User-facing build flow + escape hatch | [`README.md`](README.md) §"Building the engine" |
| Compatibility surface (label inventory, manifest schema, layout, upgrade rules) | [`docs/dev/googlesql-prebuilt/`](docs/dev/googlesql-prebuilt/) (start at the README) |
| Maintainer artifact runbook (publish, pin, verify, roll back) | [`docs/dev/googlesql-prebuilt/maintainer-runbook.md`](docs/dev/googlesql-prebuilt/maintainer-runbook.md) |
| GoogleSQL upgrade procedure (ordered checklist) | [`docs/dev/googlesql-prebuilt/upgrade-procedure.md`](docs/dev/googlesql-prebuilt/upgrade-procedure.md) |
| Cache + performance expectations | [`docs/dev/googlesql-prebuilt/performance.md`](docs/dev/googlesql-prebuilt/performance.md) |
| `FAIL_*` validator-token troubleshooting | [`docs/dev/googlesql-prebuilt/troubleshooting.md`](docs/dev/googlesql-prebuilt/troubleshooting.md) |
| Rollback playbook (parity-failure / regression response) | [`docs/dev/googlesql-prebuilt/rollback.md`](docs/dev/googlesql-prebuilt/rollback.md) |
| Producer workflow (manual dispatch) | [`.github/workflows/googlesql-prebuilt.yml`](.github/workflows/googlesql-prebuilt.yml) |
| Source-vs-prebuilt parity workflow (PR / scheduled / release tiers) | [`.github/workflows/googlesql-parity.yml`](.github/workflows/googlesql-parity.yml) |
| Release pin (`env.RELEASE_GOOGLESQL_PREBUILT_URL/_SHA256`) | [`.github/workflows/release.yml`](.github/workflows/release.yml) |
| Repo-vars pin (consumer pin for all other CI lanes) | `vars.GOOGLESQL_PREBUILT_URL` / `vars.GOOGLESQL_PREBUILT_SHA256` |

---

## 2. Roadmap & orchestration model (already in motion)

The roadmap was decomposed into **46 small `.plan.md` files** under [`.cursor/plans/`](.cursor/plans/), each with ≤3 todos and one-subagent-session of work. The canonical index is:

[`.cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md`](.cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md)

It tracks per-plan completion status in YAML front-matter. **Only the orchestrator updates that index** — never let a plan-execution subagent edit it.

**Orchestration policy decisions already locked in** (don't re-litigate without the user):

| Decision | Choice |
|---|---|
| Driver | **In-session** — the parent agent serially launches one `generalPurpose` subagent per plan. |
| Failure policy | **Retry once, then stop and surface to the user.** Treat `BLOCKED` the same way: retry once. |
| Pause points | **Honor** them. Pause and ask the user before plans that need a directional decision. |
| Index updates | The orchestrator commits a `docs(plans): mark <plan> completed in index` after each successful plan. |

**Standing pause points still ahead** (see §6):

- **Before plan 44** (`goreleaser-release_p3a4b5c6`): confirm release tag scheme + GHCR image targets.

There is no remaining DuckDB- or GoogleSQL-vendoring pause point — those were already resolved in earlier sessions.

### 2.1 Subagent prompt template (use this for plans 35→45)

Put the plan-specific guidance inside; keep the structural scaffolding identical.

```
You are executing Plan <NN> (`<plan-slug>`) for the bigquery-emulator project.

## Workspace
- Repo: /home/brighten-tompkins/Code/bigquery-emulator
- Plan file: .cursor/plans/<plan-slug>.plan.md   <-- read it FIRST
- Index file (DO NOT EDIT): .cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md
- Auto-commit + lint rules: CLAUDE.md and .cursor/rules/{auto-commit,pre-commit-lint}.mdc

## Your sequence
1. Read the plan file. Identify the todos + verification commands + done criteria.
2. Read enough of the codebase to ground each todo. Prior plans set the precedent for naming, commit style, test layout.
3. Implement each todo. Stay strictly in scope; do NOT start the next plan.
4. Run the plan's verification commands. Fix failures.
5. Commit per CLAUDE.md (conventional commits, group by logical unit, never `git add .`).
6. End your final message with EXACTLY this on its own line at the bottom:

   PLAN_RESULT: <SUCCESS|FAILED|BLOCKED>
   PLAN_ID: <NN>
   COMMITS: <comma-separated short SHAs, oldest-first>
   NOTES: <one-sentence summary; for FAILED/BLOCKED include reason and what's needed>

Begin.
```

After `SUCCESS`, the orchestrator: (a) verifies the working tree is clean and the reported SHAs exist, (b) flips the matching `status: pending` → `status: completed` in the index, and (c) commits that index update with `docs(plans): mark <plan> completed in index`.

---

## 3. Where we are right now

**Completed:** plans 00 through 33 inclusive (34/46 = 73%).

The full per-plan status is the source of truth in the index file. Recent commit history snapshot at handoff time:

```
2df58c5 chore(mise): add clang 22.1.6
dc4e794 docs(plans): mark dml-insert completed in index
d886f3e fix(gateway/e2e): seed INSERT SELECT source via DML instead of insertAll
4aef4ce chore(plans): mark dml-insert-e2e todos completed
7b26087 feat(gateway): surface DmlStats on REST QueryResponse
7a20c4c feat(frontend): classify SELECT vs DML vs DDL and route INSERT to ExecuteDml
bb9374e feat(engine): implement INSERT DML on reference impl engine
3bbf674 feat(proto): add DmlStats message and QueryResultRow.dml_stats field
```

A retroactive interlude (between plans 24 and 25) built a canonical Bazel `emulator_main` that links GoogleSQL + reference impl + DuckDB + grpc, which retroactively unblocked the previously auto-skipped E2E tests for plans 17–24.

---

## 4. Plan 34 (`dml-update-delete`) — IN-FLIGHT, WIP-checkpointed

This is where you (the new Cursor instance) will resume.

### 4.1 Why we paused

The previous orchestrator session ran on **WSL2 with 9.7 GiB RAM and ~3.6 GiB swap already in use**. The plan-34 subagent froze trying to run `bazel test //...` against the GoogleSQL-linked tree at the default `--jobs=8`. The user is rebooting into native Linux on the same machine (which has more RAM) to continue. Memory is no longer expected to be a hard ceiling, but Bazel hygiene is still a hard requirement — see [`.cursor/rules/bazel-process-hygiene.mdc`](.cursor/rules/bazel-process-hygiene.mdc). Use the `task bazel:*` helpers (already wired in `taskfiles/bazel.yml`) and the throttled `task emulator:build-engine:bazel`; do not spawn raw `bazel build`/`bazel test` invocations in parallel and always finish with `task bazel:shutdown`.

If you hit OOM or thrashing again, fall back to:

```
BAZEL_JOBS=2 BAZEL_MEM_MB=4096 task bazel:build TARGETS=//path/to:target
BAZEL_JOBS=2 BAZEL_MEM_MB=4096 task bazel:test  TARGETS=//path/to:target
task bazel:shutdown   # between long invocations to release the persistent server's heap
```

Do **not** edit `.bazelrc` to add a low-memory config; keep flags on the CLI / `BAZEL_*` env vars.

### 4.2 What's already shipped in the WIP checkpoint commit

The WIP commit on `main` (subject: `chore(wip): checkpoint plan 34 dml-update-delete (untested)`) contains:

| File | Purpose |
|---|---|
| `backend/storage/storage.h` | Adds `Storage::OverwriteRows()` interface — atomic table-replace primitive used by UPDATE/DELETE/MERGE scan-and-rewrite. |
| `backend/storage/memory/in_memory_storage.{h,cc,test.cc}` | `OverwriteRows()` impl + unit tests. |
| `backend/storage/duckdb/duckdb_storage.{h,cc}` | `OverwriteRows()` impl (atomic Parquet rewrite). |
| `backend/catalog/storage_table.cc` | Catalog-side wiring needed by the engine's UPDATE/DELETE rewrites. |
| `backend/engine/reference_impl/reference_impl_engine.cc` (+395 lines) | UPDATE + DELETE statement support via `googlesql::PreparedModify`. INSERT path was already in plan 33. |
| `backend/engine/reference_impl/reference_impl_engine_test.cc` (+179 lines) | Unit tests for UPDATE + DELETE. |
| `backend/engine/reference_impl/BUILD.bazel` | New deps for the modify path. |
| `frontend/handlers/query_test.cc` | Routing test updates so UPDATE/DELETE land in `ExecuteDml`. |
| `gateway/e2e/dml_update_delete_test.go` (NEW, 271+ lines) | Four E2E scenarios: `TestDMLUpdateRoundTrip`, `TestDMLDeleteRoundTrip`, `TestDMLDeleteWhereTrueClearsTable`, `TestDMLMergeIsUnimplemented`. |

### 4.3 The MERGE decision (READ THIS — needs orchestrator+user input before plan 34 closes)

The previous subagent explicitly **deferred MERGE**. Its rationale, baked into a comment block in `reference_impl_engine.cc` and pinned by the `TestDMLMergeIsUnimplemented` E2E test:

> The GoogleSQL reference-impl algebrizer does not yet support `RESOLVED_MERGE_STMT` at the statement root (see `googlesql/reference_impl/algebrizer.cc::AlgebrizeStatement`'s `// TODO: Add MERGE support.`). `PreparedModify::Execute` therefore cannot drive MERGE end-to-end. The engine returns `UNIMPLEMENTED` with the standard prefix.

Plan 34's `merge-basic` to-do says: *"MERGE INTO WHEN MATCHED/NOT MATCHED single-table target; document BQ limitations."* The previous agent's interpretation: the "document limitations" half is fulfilled by the unimplemented-error message + the pinning E2E test, and the MERGE implementation work moves to a new "Phase 6c" plan.

**Three viable paths forward — pause and ask the user before picking one:**

1. **Accept the deferral.** Mark plan 34's `merge-basic` to-do as cancelled-and-deferred, complete plan 34, and insert a new `merge-scan-and-diff` plan slot before plan 35 in the index. (Smallest scope; honors the previous subagent's reasoning.)
2. **Implement MERGE via scan-and-diff in this same plan.** Bypass `PreparedModify` for MERGE: walk the source + target row sets in C++, compute the WHEN MATCHED / WHEN NOT MATCHED projections directly, and call `OverwriteRows()`. Keeps plan 34 honest to its original scope.
3. **DuckDB-only MERGE.** Implement MERGE on the DuckDB engine (which can transpile MERGE natively) and keep ReferenceImplEngine returning `UNIMPLEMENTED`. Document the asymmetry; the conformance harness later (plans 40–42) will surface any divergence.

The user's prior orchestration choices (small atomic plans, retry-once, honor pauses) suggest **option 1** is the closest fit, but they have not been asked yet.

### 4.4 What still needs to happen for plan 34 to close

1. **Verify the WIP checkpoint compiles and tests pass.** From the repo root:

   ```bash
   bazel shutdown
   bazel build --test_output=errors //backend/storage/... //backend/engine/reference_impl/... //frontend/...
   bazel test  --test_output=errors //backend/storage/... //backend/engine/reference_impl/... //frontend/handlers/...
   go test -tags=integration ./gateway/e2e/... -run DML
   ```

   If you're still memory-constrained, use the `--jobs=2 --local_resources=...` flags from §4.1.

2. **Resolve the MERGE decision** (§4.3). Make a single ask of the user. Then:
   - Path 1: split `merge-basic` into a new plan, update the index, ship plan 34 with UPDATE+DELETE only.
   - Path 2 or 3: implement MERGE, add unit + E2E tests, then ship.

3. **Verify lint:** `task lint:fix && task lint:run`. The pre-commit-lint gate (`.cursor/rules/pre-commit-lint.mdc`) was deliberately skipped on the WIP checkpoint commit; the *next* commit on this branch must pass it.

4. **Final commit(s) for plan 34:** one or more conventional-commit messages closing each todo. Then the orchestrator's index-update commit `docs(plans): mark dml-update-delete completed in index` flipping `run-34-dml-update-delete` to `completed`.

5. **Resume the orchestration loop** at plan 35 (`ddl-statements_g4d5e6f7`).

### 4.5 Untracked files at handoff time

- `.golangci.yml` — present in the working tree, never committed in this repo's history (`git log -- .golangci.yml` is empty). It is a fully-formed v2 config file consistent with the project's lint policy. **Origin uncertain**: it may be from an even-earlier session that never closed. Leave it untracked until you can confirm it belongs to a specific plan's deliverable. If it turns out to be plan-33 or plan-34 collateral, commit it as `chore(lint): add golangci-lint config` separately.

---

## 5. Recovery shortcut for plan 34 (drop-in subagent prompt)

If the user just says "continue plan 34", spawn a `generalPurpose` subagent with this prompt (after asking about the MERGE decision in §4.3):

```
You are recovering Plan 34 (`dml-update-delete`) for bigquery-emulator. The plan-34
WIP checkpoint commit (subject "chore(wip): checkpoint plan 34 dml-update-delete (untested)")
has UPDATE + DELETE shipped on the reference impl + both storage backends, with E2E tests.
MERGE is intentionally deferred behind an UNIMPLEMENTED + pinning test.

The orchestrator has decided MERGE-handling path: <PATH 1 / 2 / 3 from HANDOFF.md §4.3>.
Apply that decision, then verify with:

  bazel shutdown
  bazel build --test_output=errors //backend/... //frontend/...
  bazel test  --test_output=errors //backend/storage/... //backend/engine/reference_impl/... //frontend/handlers/...
  go test -tags=integration ./gateway/e2e/... -run DML
  task lint:fix && task lint:run

Use --jobs=2 --local_resources=cpu=2,memory=4096 if memory-constrained.

Commit per CLAUDE.md. Do NOT edit the index file. End with the standard
PLAN_RESULT/PLAN_ID/COMMITS/NOTES sentinel block.
```

---

## 6. Remaining roadmap (plans 34 → 45)

| # | Plan | Phase | Notes |
|---|------|-------|-------|
| 34 | [`dml-update-delete`](.cursor/plans/dml-update-delete_f3c4d5e6.plan.md) | 6b | **IN-FLIGHT** — see §4. |
| 35 | [`ddl-statements`](.cursor/plans/ddl-statements_g4d5e6f7.plan.md) | 6c | CREATE/DROP/ALTER TABLE on both engines. |
| 36 | [`dml-ddl-e2e`](.cursor/plans/dml-ddl-e2e_h5e6f7a8.plan.md) | 6d | E2E coverage of the combined DML+DDL surface. |
| 37 | [`storage-read-proto`](.cursor/plans/storage-read-proto_i6f7a8b9.plan.md) | 7a | Proto definitions for the BigQuery Storage Read API. |
| 38 | [`storage-read-rows`](.cursor/plans/storage-read-rows_j7a8b9c0.plan.md) | 7b | Row-stream impl. |
| 39 | [`storage-read-gateway-e2e`](.cursor/plans/storage-read-gateway_e2e_k8b9c0d1.plan.md) | 7c | Gateway+E2E for Storage Read API. |
| 40 | [`conformance-fixtures-runner`](.cursor/plans/conformance-fixtures-runner_l9c0d1e2.plan.md) | 8a | Conformance harness skeleton. |
| 41 | [`conformance-diff-ci`](.cursor/plans/conformance-diff-ci_m0d1e2f3.plan.md) | 8b | CI-side diffing of expected vs. actual fixtures. |
| 42 | [`conformance-seed-docs`](.cursor/plans/conformance-seed-docs_n1e2f3a4.plan.md) | 8c | Document fixture authoring + seed initial set. |
| 43 | [`docker-compose-smoke`](.cursor/plans/docker-compose-smoke_o2f3a4b5.plan.md) | 9a | docker-compose smoke test of the multi-stage image. |
| **PAUSE** | — | — | **Confirm with user**: release tag scheme + GHCR image targets. |
| 44 | [`goreleaser-release`](.cursor/plans/goreleaser-release_p3a4b5c6.plan.md) | 9b | goreleaser config + GHCR push. |
| 45 | [`profile-docs-version`](.cursor/plans/profile-docs-version_q4b5c6d7.plan.md) | 9c | Final docs/version polish. |

After plan 45 the orchestrator should print a summary (total commits, wall time, next-step pointers) and stop.

### 6.1 Release / distribution posture (set by plan 44)

The user-confirmed decisions for plan 44 are **frozen**; do not
re-litigate without explicit user input:

| Knob | Decision |
|------|----------|
| Tag scheme | **Manual seed.** `v0.0.1` is the first cut. Releases are triggered by `git tag -a vX.Y.Z && git push origin vX.Y.Z`; the tag push fires [`.github/workflows/release.yml`](./.github/workflows/release.yml). The semantic-release config at [`.releaserc.yml`](./.releaserc.yml) is **parked**, not active — it documents the conventional-commits format we already use (see `.cursor/rules/auto-commit.mdc`) and is a one-commit flip away from being wired to a `push: branches: [main]` workflow when the project chooses to auto-release. **Do not** wire that workflow before the user explicitly opts in. |
| GHCR target | `ghcr.io/vantaboard/bigquery-emulator`. The release workflow lowercases the owner via `tr '[:upper:]' '[:lower:]'` before passing it to `docker/metadata-action@v5`. |
| Docker tags | Full SemVer trinity + `latest` per release (`vX.Y.Z`, `vX.Y`, `vX`, `latest`). `latest` is suppressed for pre-release tags (`v0.0.1-rc1`) via the `!contains(github.ref, '-')` predicate. |
| Engine binary | **`linux/amd64` only.** The release workflow builds it once via `task emulator:build-engine:bazel` and goreleaser bundles `bin/emulator_main` + `bin/libduckdb.so` into all four gateway archives (linux/{amd64,arm64} + darwin/{amd64,arm64}). The release notes header + README §Releases call out that non-linux/amd64 callers must use the published Docker image (`ghcr.io/vantaboard/bigquery-emulator:vX.Y.Z`) or supply their own engine. |
| Trigger | `on: push: tags: ['v*']` plus `workflow_dispatch` with an `inputs.ref` to re-run a previously pushed tag without re-tagging. |

Helper task list:

- `task release:check` — `goreleaser check` (offline schema validation).
- `task release:snapshot` — `goreleaser release --snapshot --clean`. Requires `bin/emulator_main` + `bin/libduckdb.so`; the goreleaser `before.hooks` block on those existing.
- `task release:tag VERSION=v0.0.1` — print-only by default; pass `CONFIRM=yes` to actually run `git tag -a … && git push origin …`.

When plan 45 lands, the gateway will gain a real `--version` flag.
The `-X main.version=…` ldflags injected by `.goreleaser.yml` are
forward-compatible no-ops until that flag exists; nothing in plan 44
needs to change when plan 45 wires the variable.

#### v0.0.1 cut posture (after the manual ship)

The first cut of `v0.0.1` exposed two design gaps that left the
release in a **partially-shipped** state. Both are recorded here so the
next release does not relearn them.

| What shipped | Where |
|--------------|-------|
| 4 gateway archives (linux/{amd64,arm64} + darwin/{amd64,arm64}) bundling `bin/emulator_main` + `bin/libduckdb.so` + docs | <https://github.com/vantaboard/bigquery-emulator/releases/tag/v0.0.1> (5 assets, ~80 MB each + a 438-byte SHA256 checksums file) |
| The `v0.0.1` annotated tag | `git rev-list -n 1 v0.0.1` → `27e2010` |

| What did NOT ship | Why | Fix path |
|-------------------|-----|----------|
| `ghcr.io/vantaboard/bigquery-emulator:{v0.0.1,v0.0,v0,latest}` | First push to a user-namespace GHCR package by a workflow `GITHUB_TOKEN` is blocked with `denied: permission_denied: write_package` until the package exists. Login succeeded; create-and-push did not. The repo-level `default_workflow_permissions=write` flip (done during triage) was necessary but not sufficient. | Bootstrap once manually: `docker login ghcr.io -u vantaboard -p <PAT-with-write:packages>` then `docker push ghcr.io/vantaboard/bigquery-emulator:bootstrap`. Then on <https://github.com/users/vantaboard/packages/container/bigquery-emulator/settings> add the `vantaboard/bigquery-emulator` repo under "Manage Actions access" with `Write`. Subsequent tag pushes will succeed. |

The release workflow's Docker-push step is now wrapped in
`continue-on-error: true` so a GHCR denial does NOT block the
`goreleaser` step that ships the gateway archives. The next release
ships binaries even if GHCR is still broken; once GHCR is unblocked,
the same step starts publishing images without further workflow
changes.

Lessons baked into the workflow + Dockerfile by the v0.0.1 cut:

- **Bazel runs ONCE per release.** The Dockerfile's `engine-builder`
  stage now switches between `engine-builder-bazel` (default,
  self-contained) and `engine-builder-prebuilt` (lifts `bin/...` from
  the build context) via a global `ENGINE_SOURCE` build arg.
  `release.yml` passes `ENGINE_SOURCE=prebuilt` so the
  cold-cache GoogleSQL+gRPC link runs once on the host (~125 min) and
  the Docker push reuses the same binary in ~5 minutes — not the
  ~250 min the original "build twice" path required.
- **Runtime base flipped to `ubuntu:24.04`.** Bookworm's GLIBC 2.36 was
  too old to load a binary built on the GitHub `ubuntu-latest` runner
  (GLIBC 2.39). Ubuntu 24.04 matches the build host exactly and stays
  GLIBC-compatible with binaries from the bookworm-based `bazel`
  stage too (forward-compat on 2.39).
- **Release job `timeout-minutes: 180`.** First cut hit `120` mid-Bazel
  at `[5,120 / 5,389]` actions (~95% complete). Bumped to 180 so the
  next cold cut finishes and the `setup-bazel` post-step uploads the
  `disk-cache: release` for subsequent runs.

The four GitHub runs from the v0.0.1 cut, for posterity:

| Run | Outcome | Why |
|-----|---------|-----|
| `26480976029` | timed out at 120 min mid-Bazel | original timeout too tight; led to the 180-min bump |
| `26485361700` | cancelled at 162 min mid-Docker | Docker stage was about to re-do the cold Bazel build a second time; led to the `ENGINE_SOURCE=prebuilt` refactor |
| `26490598133` | failed at Docker push | GHCR `write_package` denial; led to the `default_workflow_permissions=write` flip |
| `26495700505` | failed at Docker push (same denial) | repo-level flip was not enough; led to the manual goreleaser ship + `continue-on-error` workflow patch |

---

## 7. Key reference files (skim these as needed)

- [`ROADMAP.md`](ROADMAP.md) — full architecture & phase narrative.
- [`CLAUDE.md`](CLAUDE.md) — auto-commit workflow (conventional commits, hunk-staging rules, breaking-change footers).
- [`.cursor/rules/pre-commit-lint.mdc`](.cursor/rules/pre-commit-lint.mdc) — when to gate commits on lint vs. when to skip.
- [`.cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md`](.cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md) — per-plan progress + dependency graph.
- [`MODULE.bazel`](MODULE.bazel), [`.bazelrc`](.bazelrc), [`.bazelversion`](.bazelversion) — Bazel + GoogleSQL toolchain wiring.
- [`Taskfile.yml`](Taskfile.yml) — `task lint:*`, `task ci:run`, `task emulator:build-engine:bazel`, `task release:*`.
- [`.goreleaser.yml`](.goreleaser.yml), [`.releaserc.yml`](.releaserc.yml), [`.github/workflows/release.yml`](.github/workflows/release.yml) — release path (plan 44 deliverables; see §6.1).
- [`mise.toml`](mise.toml), `.envrc` — toolchain pins (Go, golangci-lint, clang, bazel-via-bazelisk). `direnv` exposes them.
- `/home/brighten-tompkins/Code/go-googlesql/` — sibling GoogleSQL checkout; `MODULE.bazel` `local_path_override`s to it. Its `.envrc` was the reference for tooling setup.

---

## 8. House rules cheatsheet (so you don't have to relearn them)

- **Never** `git add .` or `git add -A`. Stage explicit paths or hunks.
- Conventional commits only. Allowed types: `feat`, `fix`, `refactor`, `style`, `docs`, `test`, `chore`. WIP checkpoints use `chore(wip)` or include `(WIP)` in the subject.
- Group commits by logical unit (Go file + its test together; handler + route together). Don't group unrelated bug fixes, deps + features, or different features.
- Pre-commit lint gate (`task lint:fix && task lint:run`) applies to any commit touching Go source or `.golangci.yml`. The gate is allowed to be **skipped** for explicit WIP/checkpoint commits and when the toolchain isn't available — see `.cursor/rules/pre-commit-lint.mdc` clauses 1 & 3.
- A subagent executing a single plan **must not** edit the index file. Only the orchestrator updates `.cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md`.
- Use the `PLAN_RESULT / PLAN_ID / COMMITS / NOTES` sentinel block at the end of every plan-execution subagent message so the orchestrator can parse it.
