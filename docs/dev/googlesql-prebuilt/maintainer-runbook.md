# GoogleSQL Prebuilt — Maintainer Artifact Runbook (Phase 6)

This runbook is the operational counterpart to the Phase 1
[compatibility surface](README.md) and the Phase 5
[rollback playbook](rollback.md). It covers the steady-state
"publish a new artifact, pin it, verify it, roll back if needed"
workflow that maintainers run between upgrades (the upgrade flow
itself lives in [`upgrade-procedure.md`](upgrade-procedure.md)).

Every command and workflow name here matches what is wired in the
repo today; no placeholders. Cross-reference the source if in doubt:

- Producer: [`.github/workflows/googlesql-prebuilt.yml`](../../../.github/workflows/googlesql-prebuilt.yml)
- Producer scripts: [`tools/googlesql-prebuilt/package.sh`](../../../tools/googlesql-prebuilt/package.sh) and [`verify.sh`](../../../tools/googlesql-prebuilt/verify.sh)
- Validator (Phase 5): [`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py)
- Consumer setup composite: [`.github/actions/setup-googlesql/action.yml`](../../../.github/actions/setup-googlesql/action.yml)
- Local cache helpers: [`taskfiles/googlesql.yml`](../../../taskfiles/googlesql.yml)
- Release pin: [`.github/workflows/release.yml`](../../../.github/workflows/release.yml) (`env.RELEASE_GOOGLESQL_PREBUILT_URL` / `_SHA256`)
- Parity job (Phase 5): [`.github/workflows/googlesql-parity.yml`](../../../.github/workflows/googlesql-parity.yml)

## Roles at a glance

| Role | Surface they own |
|---|---|
| **Artifact producer** | The producer workflow + scripts, the `.tar.gz` + `manifest.json` + `.tar.gz.sha256` triple under the `googlesql-prebuilt/v<...>+gs-<...>` GitHub Release. |
| **Consumer-pin owner** | `vars.GOOGLESQL_PREBUILT_URL` + `vars.GOOGLESQL_PREBUILT_SHA256` (every CI workflow's pin), and `env.RELEASE_GOOGLESQL_PREBUILT_URL` / `_SHA256` in `release.yml` (the release pin). |
| **Local environment** | The developer's machine — `.cache/googlesql-prebuilt/` and the sibling `../googlesql/` checkout. |
| **Compatibility surface** | The Phase 1 docs under this directory: label inventory, manifest schema, repo layout, upgrade rules. Bumping this surface is a breaking change (see [`upgrade-rules.md`](upgrade-rules.md)). |

The [troubleshooting guide](troubleshooting.md) routes every validator
`FAIL_*` token to the owner above so the right person is on point.

## 1. Run the producer workflow

Producer entrypoint: [`googlesql-prebuilt` workflow](../../../.github/workflows/googlesql-prebuilt.yml).
Manual dispatch only — there is no automatic trigger today.

```bash
# Build and publish a new artifact:
gh workflow run googlesql-prebuilt.yml \
    -f googlesql_ref=2026.01.1 \
    -f artifact_version=0.1.0 \
    -f publish=true
```

Inputs:

| Input | Required | Meaning |
|---|---|---|
| `googlesql_ref` | yes | Upstream GoogleSQL git ref (tag or commit). The producer shallow-clones `google/googlesql` at this ref. Defaults to `2026.01.1` (the Phase 1-frozen pin). |
| `artifact_version` | yes | Strict-semver string (e.g. `0.1.0`). Bumped whenever the artifact's payload, header set, wrapper layout, or compatibility contract changes. See [`upgrade-rules.md`](upgrade-rules.md) for what kinds of change require which kind of bump. |
| `publish` | no (default `false`) | If `true`, the workflow uploads the verified artifact to a GitHub Release. Leave it `false` for trial / verification-only runs; the workflow's "Upload workflow artifacts" step always preserves the tarball + manifest as a workflow artifact for 30 days regardless. |

What the workflow does, in order (the comments in the workflow file
are the source of truth — this is a summary):

1. Checks out the emulator at the branch the workflow is dispatched from.
2. Checks out `google/googlesql` at `googlesql_ref` and applies the gazelle leading-zero patch.
3. Refuses to start if `publish=true` and a release asset for the same `(artifact_version, googlesql_short_sha)` pair already exists. The producer's contract is immutable artifacts — there is no `--clobber`.
4. Runs `tools/googlesql-prebuilt/package.sh --mode=bazel` to produce the `.tar.gz` + sidecar `manifest.json` + `.sha256`.
5. Runs `verify.sh --smoke-mode=link` (clang link smoke) then `--smoke-mode=bazel` (full wrapper-target build) against the unpacked artifact. **Both must pass before publish.** Phase 5 hooked these into `validate_artifact.py` so the same gates the consumer applies are enforced here.
6. Uploads the tarball + sidecar manifest + summary as a 30-day workflow artifact (success or failure — failed runs still leave a forensic trail).
7. If `publish=true`: creates the release `googlesql-prebuilt/v<artifact_version>+gs-<googlesql_short_sha>` and uploads three assets:
   - `googlesql-prebuilt-linux-amd64-clang18-<short>-v<version>.tar.gz`
   - `<asset>.tar.gz.sha256`
   - `<asset>.manifest.json`
8. Shuts down the host-side bazel daemons.

## 2. Choose the GoogleSQL source SHA

Decide what upstream commit the artifact will be pinned to **before**
dispatching the workflow:

- **Tag pin (default).** Reuse `2026.01.1` (or whichever tag is currently frozen in [`docs/dev/googlesql-prebuilt/README.md`](README.md)). The producer resolves it to a concrete 40-hex commit during the run and writes it into `manifest.json` `googlesql.commit`.
- **Commit pin (debug / between-tag).** Pass a 40-hex commit as `googlesql_ref` when you need to pin to something that isn't tagged yet. The producer will still write the resolved commit to `manifest.json`.

Either way, the artifact carries enough provenance (`googlesql.upstream_tag`,
`googlesql.commit`, `googlesql.repo_url`) for a consumer to walk back to
the exact source tree without needing the producer's local environment.

Sanity-check the source SHA before publishing:

```bash
git ls-remote --tags https://github.com/google/googlesql
git ls-remote https://github.com/google/googlesql <ref>
```

If you are bumping the upstream pin (vs. republishing the current pin
for an unrelated reason), follow the
[GoogleSQL upgrade procedure](upgrade-procedure.md) first — it covers
the label-inventory review and the wrapper-layout impact that a SHA
bump can hide.

## 3. Where artifacts live (publish targets)

| Surface | Location |
|---|---|
| GitHub Release | `https://github.com/<owner>/<repo>/releases/tag/googlesql-prebuilt/v<artifact_version>+gs-<googlesql_short_sha>` |
| Tarball asset URL | `https://github.com/<owner>/<repo>/releases/download/googlesql-prebuilt/v<artifact_version>+gs-<short>/googlesql-prebuilt-linux-amd64-clang18-<short>-v<artifact_version>.tar.gz` |
| Sidecar manifest | Same release, `<asset>.manifest.json` |
| Tarball SHA-256 (sidecar) | Same release, `<asset>.tar.gz.sha256` |
| Workflow artifact (success or fail) | `googlesql-prebuilt-<short>-v<version>` under the workflow run, 30-day retention |

`gh release list --json tagName | jq '.[] | select(.tagName | startswith("googlesql-prebuilt/"))'`
walks every published artifact tag. The
[rollback playbook](rollback.md#repin-cheap-preferred-response) uses the
same listing to find a known-good prior pin.

## 4. Read the manifest digest / checksum

After a `publish=true` run:

```bash
TAG=googlesql-prebuilt/v0.1.0+gs-<short>
gh release view "$TAG" --json assets --jq '.assets[].name'
# Asset URL:
gh release view "$TAG" --json assets --jq '.assets[] | {name, url}'

# Tarball SHA-256 (the value you will paste into pins below):
ASSET=googlesql-prebuilt-linux-amd64-clang18-<short>-v0.1.0.tar.gz
gh release download "$TAG" --pattern "$ASSET.sha256" --dir /tmp
awk '{print $1}' "/tmp/$ASSET.sha256"

# Manifest fields the validator and the parity job key on:
gh release download "$TAG" --pattern "$ASSET.manifest.json" --dir /tmp
jq '{schema_version, artifact_version, googlesql, platform, toolchain}' \
    "/tmp/$ASSET.manifest.json"
```

The producer also surfaces the asset URL and SHA in the workflow's
"Publish to GitHub Release" step output (`ARTIFACT_URL` /
`ARTIFACT_SHA256` notice and `$GITHUB_STEP_SUMMARY` block) so you can
grab them straight from the run page if you don't want to shell out
to `gh`.

## 5. Verify the artifact after publishing

The producer workflow already runs the verifier internally. After
publish, re-run the same gates from your own environment to catch
"the wrong asset got uploaded" mistakes:

```bash
# Pull the just-published asset into the local cache and run every
# gate the build path will run later. fetch-prebuilt re-runs the
# centralized validator (schema, identity, platform, payload SHA,
# wrapper presence, no-unaccounted-file) BEFORE committing to the
# cache.
task googlesql:fetch-prebuilt \
    URL=https://github.com/<owner>/<repo>/releases/download/$TAG/$ASSET \
    SHA256=$(awk '{print $1}' "/tmp/$ASSET.sha256")

# Re-validate explicitly with identity pins so artifact_version and
# googlesql.commit must match what you expect (defensive — catches
# a republish-under-the-same-URL incident).
task googlesql:validate \
    EXPECTED_ARTIFACT_VERSION=0.1.0 \
    EXPECTED_GOOGLESQL_SHA=<full 40-hex commit>

# Status of what is staged in the cache:
task googlesql:status
```

If `task googlesql:fetch-prebuilt` or `task googlesql:validate`
prints any `FAIL_*` token, **do not pin** that artifact. See
[`troubleshooting.md`](troubleshooting.md) for token → owner mapping
and [`rollback.md`](rollback.md) for response procedure.

You can also run the producer's own verifier against the downloaded
asset to repeat the full link-smoke + bazel-smoke gate locally:

```bash
./tools/googlesql-prebuilt/verify.sh \
    --tarball /tmp/$ASSET \
    --tarball-sha256 $(awk '{print $1}' "/tmp/$ASSET.sha256") \
    --expected-googlesql-sha <full 40-hex> \
    --expected-artifact-version 0.1.0 \
    --smoke-mode=link
```

## 6. Update the consumer pin

Two distinct pin surfaces — bump both whenever you adopt a new
artifact:

### 6.1 Repo-level Actions variables (the per-PR / per-push CI lanes)

`vars.GOOGLESQL_PREBUILT_URL` and `vars.GOOGLESQL_PREBUILT_SHA256`
are read by every lane that calls `setup-googlesql` without an
explicit override (ci.yml, conformance.yml, coverage-bazel.yml,
thirdparty-samples.yml, docker-smoke.yml, googlesql-parity.yml's
prebuilt leg). Update them with `gh`:

```bash
gh variable set GOOGLESQL_PREBUILT_URL \
    --body "https://github.com/<owner>/<repo>/releases/download/$TAG/$ASSET" \
    --repo <owner>/<repo>
gh variable set GOOGLESQL_PREBUILT_SHA256 \
    --body "$(awk '{print $1}' "/tmp/$ASSET.sha256")" \
    --repo <owner>/<repo>
gh variable list --repo <owner>/<repo>
```

When these are unset, `setup-googlesql` emits a workflow warning
and falls back to a source checkout — except on lanes that pass
`fail-on-source-fallback: 'true'` (release.yml and the parity job's
prebuilt leg), which refuse to run.

### 6.2 Release pin (the release.yml workflow)

The release pipeline does **not** read repo vars — it reads
`env.RELEASE_GOOGLESQL_PREBUILT_URL` / `_SHA256` at the top of
[`.github/workflows/release.yml`](../../../.github/workflows/release.yml).
Edit them in-tree:

```yaml
# .github/workflows/release.yml
env:
  RELEASE_GOOGLESQL_PREBUILT_URL: "https://.../googlesql-prebuilt-...-v<...>.tar.gz"
  RELEASE_GOOGLESQL_PREBUILT_SHA256: "<64 hex>"
```

Commit with `chore(release): pin GoogleSQL prebuilt v<version>+gs-<short>`
(the existing `git log --oneline --grep 'pin GoogleSQL prebuilt'`
trail makes prior pins easy to find when rolling back).

Tag-push releases (`on: push: tags`) refuse to start when either
value is empty; the "Validate GoogleSQL prebuilt pin" step is the
hard gate. The only way to release with empty pins is the explicit
debug knob `gh workflow run release.yml -f ref=vX.Y.Z -f googlesql_source=true`.

## 7. Roll out + verify the new pin

After bumping the pins above:

```bash
# Trigger the PR-tier parity job against the same commit:
gh workflow run googlesql-parity.yml -f tier=pr

# Watch the comparator's "compare prebuilt vs source" summary block:
gh run watch
```

Before cutting a release with the new pin, run the **release tier**:

```bash
gh workflow run googlesql-parity.yml -f tier=release
```

The release-tier comparator adds the duckdb conformance lane on top
of the scheduled-tier memory lane. A `matched` result on every stage
is the gate. A `DIVERGED` result is a Phase 5 hard rollback signal
([`rollback.md`](rollback.md#1-parity-job-failures)).

## 8. Roll back

The full triggers + procedures live in [`rollback.md`](rollback.md).
The repin path (preferred) is just: bump the pins above to a prior
known-good `(URL, SHA-256)` pair from the
`chore(release): pin GoogleSQL prebuilt` commit history, then re-run
the failing lane. The revert-prebuilt-adoption path is reserved for
the case where **no** prior artifact restores green and the
local-source build still works.

## 9. Maintainer cheat-sheet

The single-command shapes worth memorising:

```bash
# Publish:
gh workflow run googlesql-prebuilt.yml \
    -f googlesql_ref=<tag-or-sha> \
    -f artifact_version=<x.y.z> \
    -f publish=true

# Find the new asset URL + SHA:
gh release view "googlesql-prebuilt/v<x.y.z>+gs-<short>" --json assets

# Pull, re-stage, and re-validate locally:
task googlesql:fetch-prebuilt URL=<url> SHA256=<sha>
task googlesql:validate \
    EXPECTED_ARTIFACT_VERSION=<x.y.z> \
    EXPECTED_GOOGLESQL_SHA=<40-hex>

# Bump the consumer pins:
gh variable set GOOGLESQL_PREBUILT_URL --body "<url>"
gh variable set GOOGLESQL_PREBUILT_SHA256 --body "<sha>"
# (and edit env.RELEASE_GOOGLESQL_PREBUILT_URL / _SHA256 in release.yml)

# Sanity-test the new pin:
gh workflow run googlesql-parity.yml -f tier=pr
# Before release:
gh workflow run googlesql-parity.yml -f tier=release

# Process hygiene — always finish with:
task bazel:status
```

## See also

- [`README.md`](README.md) — Phase 1 compatibility-surface index.
- [`upgrade-procedure.md`](upgrade-procedure.md) — full GoogleSQL upgrade flow (compatibility-surface review through release pin).
- [`performance.md`](performance.md) — cache and build-time expectations.
- [`troubleshooting.md`](troubleshooting.md) — `FAIL_*` validator-token map.
- [`rollback.md`](rollback.md) — Phase 5 rollback playbook.
- [`manifest.md`](manifest.md) — manifest schema reference.
