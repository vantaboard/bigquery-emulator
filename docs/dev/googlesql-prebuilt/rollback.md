# GoogleSQL Prebuilt — Rollback Criteria

This document specifies **when** and **how** to revert the default GoogleSQL
prebuilt adoption, or to repin to a different artifact. The maintainer
runbook weaves this into the broader operator surface; the safety-gate
track owns the machine-checkable rules and the maintainer-facing escape
hatches captured here.

## Scope

In scope:

- Reverting `task emulator:build-engine:bazel` from prebuilt-default to
  source-default (the entrypoints / consumer-default flip).
- Repinning `env.RELEASE_GOOGLESQL_PREBUILT_URL` /
  `env.RELEASE_GOOGLESQL_PREBUILT_SHA256` in
  [`.github/workflows/release.yml`](../../.github/workflows/release.yml) and
  `vars.GOOGLESQL_PREBUILT_URL` / `vars.GOOGLESQL_PREBUILT_SHA256` at the
  repository level.
- Reacting to source-vs-prebuilt parity-job regressions
  ([`googlesql-parity.yml`](../../.github/workflows/googlesql-parity.yml)).

Out of scope (handled by [`upgrade-rules.md`](upgrade-rules.md)):

- Adding new platforms.
- Bumping manifest `schema_version`.
- Reshaping the static-vs-shared library decision.

## Triggers that warrant rollback

The rules below name the failure signal, who notices it, and what the
"correct" rollback action is. The default is **always** repinning to a
known-good artifact (cheap, reversible) before reverting prebuilt adoption
(expensive, regresses on the multi-hour source compile).

### 1. Parity job failures

| Signal | Where it surfaces | Default response |
|--------|-------------------|------------------|
| `googlesql-parity` (`tier: pr`) reports `DIVERGED` | PR checks; `compare prebuilt vs source` job summary lists divergent fixtures. | **Repin** to the previous known-good `(URL, SHA-256)` in repo vars; the parity-divergent artifact is suspect. Open a tracking issue with the parity-job summary linked. |
| `googlesql-parity` (`tier: scheduled`) reports `DIVERGED` overnight | `googlesql-parity` nightly run summary; cron failure notification. | Same as above. The nightly lane is the canary; treat a `DIVERGED` result as a wake-up signal even if PRs are still green. |
| `googlesql-parity` (`tier: release`) reports `DIVERGED` before a release | `gh workflow run googlesql-parity.yml -f tier=release` run summary. | **Block the release** until parity is green. Either repin to a prior artifact (preferred) or, if the regression is in the source leg, investigate before publishing. |

### 2. Conformance / runtime regressions traced to GoogleSQL

| Signal | Where it surfaces | Default response |
|--------|-------------------|------------------|
| Conformance lane (`.github/workflows/conformance.yml`) flips one or more fixtures from PASS to FAIL after a GoogleSQL artifact pin bump. | Conformance job; per-profile `conformance-<profile>.json` artifact. | **Repin** to the previous artifact. If the regression survives the repin, the change is in the emulator, not the artifact — diagnose normally. |
| Engine `--version` start fails or the engine crashes during gateway boot after a pin bump. | `task docker:smoke` / `ci.yml` build step / `release.yml` `docker-smoke` step. | Repin. Engine startup is the safety-gate smoke check — a regression here implies a libstdc++ / abseil / protobuf ABI drift in the new artifact and the prior artifact is the safe fallback. |

### 3. Link / runtime ABI mismatch

| Signal | Where it surfaces | Default response |
|--------|-------------------|------------------|
| `validate_artifact.py` reports `FAIL_PAYLOAD_SHA` against the staged cache after a `task googlesql:fetch-prebuilt` (or its CI/Docker equivalent). | Local task output, `.github/actions/setup-googlesql`, Dockerfile. | The download was tampered with or the producer republished under a stale tag. Refetch from a clean cache; if that still fails, treat the artifact as compromised and repin to the prior version. **Never** override the SHA pin to make the check pass. |
| `validate_artifact.py` reports `FAIL_PLATFORM` (libc / cxx_abi / compiler mismatch). | Same. | The consumer host's runtime no longer matches the artifact contract. Either rebuild the producer with the new toolchain (preferred) or temporarily flip the local build to source mode (`GOOGLESQL_SOURCE=local task emulator:build-engine:bazel`) while the producer catches up. |
| Multiple-definition link errors at `bazel build //binaries/emulator_main:emulator_main` time after pinning a new artifact. | Bazel link step in CI / local. | Almost always caused by a `bundled_thirdparty_deps` drift between the artifact and the consumer's resolved Bzlmod graph. Revert to the previous pin and open an issue against the producer to align the bundle. |

### 4. Missing labels

| Signal | Where it surfaces | Default response |
|--------|-------------------|------------------|
| `bazel build` reports a missing `@googlesql//some/new:label`. | Local / CI Bazel step. | Two cases. (a) The new label is the emulator's fault (added by a commit to this repo); follow [`upgrade-rules.md`](upgrade-rules.md) — add the row to the inventory, repackage, repin. (b) The label was already in the inventory but the artifact is older than the inventory bump — repin to a newer artifact that includes the wrapper. |
| `validate_artifact.py` reports `FAIL_WRAPPER_MISSING` for one of the BUILD files. | Validator output (see Section "Failure tokens"). | The artifact is corrupt or stale. Repin / refetch. |

### 5. Manifest validation false positives caused by schema drift

| Signal | Where it surfaces | Default response |
|--------|-------------------|------------------|
| `validate_artifact.py` reports `FAIL_SCHEMA` but a manual inspection of `manifest.json` shows the producer simply added an optional field. | Validator output. | The validator is too strict — the optional-field allowlist in `manifest_writer.validate_manifest` needs the new key. Update `TOP_LEVEL_FIELDS` (and the field schema), regenerate, repin. **Do not** disable the validator. |
| `validate_artifact.py` reports `FAIL_SCHEMA` because of a `schema_version` bump (e.g. `1` → `2`). | Validator output. | The producer published an artifact with a newer schema than the consumer understands. Either (a) revert the producer to the prior schema and republish, or (b) land the matching validator + manifest_writer change in the emulator and only then repin. See [`upgrade-rules.md`](upgrade-rules.md) for the breaking-change-footer requirement. |

## How to roll back

### Repin (cheap; preferred response)

1. Find the previous-known-good pin. The release-asset URL + SHA-256 lives in
   the `chore(release): pin GoogleSQL prebuilt v<...>+gs-<...>` commit history
   on `main`. `git log --oneline --grep 'pin GoogleSQL prebuilt'` walks them.
2. Update the values:
   - **Repo vars (most consumers):** `gh variable set GOOGLESQL_PREBUILT_URL …`
     and `gh variable set GOOGLESQL_PREBUILT_SHA256 …` against the repo.
   - **Release pipeline:** edit
     `env.RELEASE_GOOGLESQL_PREBUILT_URL` /
     `env.RELEASE_GOOGLESQL_PREBUILT_SHA256` at the top of
     `.github/workflows/release.yml` and commit with the conventional message
     above.
3. Re-run any failing parity / conformance / release lane against the same
   ref. Confirm the lane goes green.
4. Open an issue against the producer with the bad artifact's `(URL, SHA-256)`
   + the parity / conformance diff so the producer side can fix or yank it.

### Revert prebuilt adoption (heavy; last resort)

Only do this when **all** of these are true:

- Repinning to **any** prior artifact does not restore the green lane.
- The local-source build (`GOOGLESQL_SOURCE=local task emulator:build-engine:bazel`)
  passes — i.e. the regression is in the prebuilt path itself, not in the
  emulator's consumption of GoogleSQL.
- A repin would block urgent release work (otherwise the right answer is
  "wait for the producer to publish a clean artifact").

Procedure:

1. In `taskfiles/emulator.yml::build-engine-bazel`, flip
   `_GOOGLESQL_SOURCE_RAW`'s default back to `local`. The relevant block is
   the `vars:` section under `build-engine-bazel`:
   ```yaml
   _GOOGLESQL_SOURCE_RAW:
     sh: echo "${GOOGLESQL_SOURCE:-prebuilt}"   # change to ${GOOGLESQL_SOURCE:-local}
   ```
   Same change to the `GOOGLESQL_SOURCE` canonicaliser default a few lines
   below.
2. In every CI workflow that calls `setup-googlesql` with
   `fail-on-source-fallback: 'true'`, change to `'false'`. (Today this is
   the `release.yml` job and the `googlesql-parity.yml` `build-prebuilt`
   job.) Tag the change as a temporary revert in the commit message and
   the workflow comment.
3. Commit with `feat(googlesql): revert default to source-mode build` and a
   `BREAKING CHANGE` footer (see `auto-commit.mdc`) naming the artifact that
   was bad and the producer issue tracking the fix.
4. Open a follow-up tracking issue that lists the criteria for re-flipping
   back to prebuilt-default (typically: producer ships a clean artifact +
   parity lane stays green for N consecutive runs).
5. When re-flipping back, drop the temporary `'false'` values, restore the
   `'prebuilt'` defaults, and re-test the parity job.

### Local escape hatches (no rollback needed)

The defaults are designed to be overridable per-invocation; a one-off
debugging session does **not** need a repository-wide rollback.

- `GOOGLESQL_SOURCE=local task emulator:build-engine:bazel` — rebuild from
  the sibling `../googlesql/` checkout. The pre-flight refuses to touch the
  prebuilt cache.
- `BIGQUERY_EMULATOR_SKIP_PREBUILT_VALIDATE=1 task emulator:build-engine:bazel`
  — skip the safety-gate validator pre-flight (still requires the cache to
  contain `MODULE.bazel`). Use ONLY when debugging the validator itself; the
  cache contents are still expected to be correct.
- `gh workflow run release.yml -f ref=<tag> -f googlesql_source=true` —
  trigger a release-grade rebuild against the upstream source. This is the
  documented escape hatch for "I need a release binary right now and the
  prebuilt is bad."
- `task googlesql:validate` — re-run the centralized validator against the
  current staged cache. Useful when triaging a `FAIL_*` token to confirm the
  failure reproduces.

## Failure tokens (for runbook lookup)

The centralized validator
([`tools/googlesql-prebuilt/validate_artifact.py`](../../tools/googlesql-prebuilt/validate_artifact.py))
tags every diagnostic with a stable `FAIL_*` token so runbook entries can
key off them directly:

| Token | Meaning | Default response |
|-------|---------|------------------|
| `FAIL_REPO_ROOT` | Validator was pointed at a path that is not a directory. | Re-stage the artifact via `task googlesql:fetch-prebuilt` (consumer-side bug). |
| `FAIL_MANIFEST_MISSING` | `manifest.json` absent under the artifact root. | The unpacked artifact is incomplete — fetch failure or stale partial extract. Repin / refetch. |
| `FAIL_MANIFEST_PARSE` | `manifest.json` does not parse as JSON. | The artifact is corrupt. Repin / refetch. |
| `FAIL_SCHEMA` | The closed-schema validator rejected the manifest. | See "Manifest validation false positives" above. |
| `FAIL_IDENTITY_PIN` | Caller-supplied pin (commit / artifact_version) does not match the manifest. | Either the wrong artifact landed in the cache or the pin is wrong. Confirm both, repin if needed. |
| `FAIL_PLATFORM` | OS / arch / libc / compiler mismatch. | See "Link / runtime ABI mismatch" above. |
| `FAIL_WRAPPER_MISSING` | A required BUILD or MODULE wrapper file is absent. | The artifact is corrupt / stale. Repin / refetch. |
| `FAIL_PAYLOAD_MISSING` | Manifest entry has no on-disk file. | Same as `FAIL_WRAPPER_MISSING`. |
| `FAIL_PAYLOAD_SHA` | A payload file's SHA-256 disagrees with the manifest. | Treat the artifact as compromised. **Never override.** Repin to a known-good artifact and open a producer issue. |
| `FAIL_PAYLOAD_SIZE` | A library's `size_bytes` disagrees with the manifest. | Same as `FAIL_PAYLOAD_SHA`. |
| `FAIL_PAYLOAD_ESCAPE` | A payload path tries to escape the artifact root. | Producer bug or tampering — **do not** consume. Open a producer issue. |
| `FAIL_PAYLOAD_UNACCOUNTED` | A file is on disk under the artifact root but missing from the closed manifest. | Producer drift bug. Republish, repin. |

## What "rolled back" looks like

You are done rolling back when:

- The `googlesql-parity` workflow (PR tier minimum, scheduled tier when
  reachable) reports `matched` for every stage on the post-rollback ref.
- The conformance lane is green on a re-run of the post-rollback ref.
- The release workflow's "Validate GoogleSQL prebuilt pin" step accepts the
  new pin without an error and `setup-googlesql` produces a non-empty
  `mode=prebuilt` plus a `validate_artifact.py` `OK` summary.
- A maintainer-facing tracking issue (where applicable) lists the bad
  artifact's identity + the rollback's pin so future incidents can correlate.

## See also

- [`README.md`](README.md) — compatibility surface index.
- [`manifest.md`](manifest.md) — manifest schema this rollback workflow defends.
- [`upgrade-rules.md`](upgrade-rules.md) — when a contract change is allowed
  and what footer it requires.
- [`../../.github/workflows/googlesql-parity.yml`](../../.github/workflows/googlesql-parity.yml)
  — the source-vs-prebuilt parity workflow.
- [`../../.github/workflows/release.yml`](../../.github/workflows/release.yml)
  — where the release pin lives.
- [`../../tools/googlesql-prebuilt/validate_artifact.py`](../../tools/googlesql-prebuilt/validate_artifact.py)
  — the validator whose tokens this doc lists.
