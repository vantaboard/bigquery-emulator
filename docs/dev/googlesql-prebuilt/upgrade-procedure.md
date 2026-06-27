# GoogleSQL Prebuilt — Upgrade Procedure

This is the ordered checklist for bumping the upstream GoogleSQL pin
end to end — from picking a source SHA candidate through landing the
new release pin. Stay on the checklist; each step exists because the
compatibility-surface, artifact-producer, consumer-wiring,
CI/Docker/release, and safety-gate tracks front-loaded specific gates
that the upgrade has to clear.

For day-to-day publish / pin / verify (no upstream bump), use the
[maintainer runbook](maintainer-runbook.md) instead.

For "what kinds of change count as breaking" and which footer they
need in the commit, read [`upgrade-rules.md`](upgrade-rules.md) once
**before** you start the upgrade. Sections of this checklist
explicitly defer to that doc for the breaking-change classification.

## 1. Pick an upstream GoogleSQL source SHA candidate

- Identify the upstream tag or commit you want to land on. The
  current pin lives in [`README.md`](README.md) ("Upstream GoogleSQL
  pin") and is mirrored by:
  - `MODULE.bazel` (`bazel_dep(name = "googlesql", version = "…")`)
  - the producer workflow's `googlesql_ref` default
    ([`.github/workflows/googlesql-prebuilt.yml`](../../../.github/workflows/googlesql-prebuilt.yml))
  - the `setup-googlesql` composite's `googlesql-ref` default
    ([`.github/actions/setup-googlesql/action.yml`](../../../.github/actions/setup-googlesql/action.yml))
- Resolve the ref to a full 40-hex commit. The producer records the
  resolved commit in `manifest.json` `googlesql.commit`; consumers
  pin against that field.
- If the candidate is a **commit between tags** (debug / unreleased),
  call this out in the commit message — it's allowed but it makes
  bisecting harder later.

## 2. Re-run the label inventory

The [compatibility surface](README.md) freezes which `@googlesql//...`
Bazel labels the emulator depends on. A GoogleSQL upstream change can
remove, rename, or split labels — silently if nothing forces a check.
Re-run the inventory verification:

```bash
# Every @googlesql// label currently referenced by emulator BUILD files
# must be listed in label-inventory.md.
rg '@googlesql//' /home/brighten-tompkins/Code/bigquery-emulator \
    --glob '*.bazel' --glob 'BUILD' --glob 'BUILD.bazel'
```

Cross-reference the output against [`label-inventory.md`](label-inventory.md).
Treat any divergence as a compatibility-surface change and read
[`upgrade-rules.md`](upgrade-rules.md) "Change taxonomy" to classify it:

- **Added** emulator reference to a new `@googlesql//some/new:label` →
  Breaking for the artifact contract. Add the inventory row **and**
  the wrapper mapping in [`repo-layout.md`](repo-layout.md). The
  producer run in step 4 must add the matching wrapper `cc_library`
  and bump `manifest.json` `compat.labels`.
- **Removed** emulator reference to an existing label → Non-breaking
  for the artifact; breaking for the contract. Remove the inventory
  row. The wrapper may stay one release cycle as deprecation cushion.
- **Renamed** upstream label (e.g. `:simple_catalog` → `:catalog_v2`)
  → Breaking. Update the emulator `BUILD.bazel` files **and** the
  inventory **and** the wrapper mapping in one commit; document in
  the upgrade commit body.

If the upstream tree gained a new **transitively-used** header under
an existing label, see step 3.

## 3. Update wrapper layout or manifest schema (only if needed)

If step 2 found a compatibility-surface change OR the upstream bump
introduces any of:

- New required libraries (e.g. a new `lib/libgooglesql_*.a` split)
- New required header paths
- A toolchain / platform change (compiler, libc, OS, arch)
- A new `bundled_thirdparty_deps` member (something promoted from
  Bzlmod resolution to bundled-in-`libgooglesql.a`)
- A manifest field addition / rename / removal

… then update **before** you produce. The compatibility-surface docs
that need edits:

| Change kind | Files |
|---|---|
| New / removed / renamed label | [`label-inventory.md`](label-inventory.md), [`repo-layout.md`](repo-layout.md) |
| New / removed header closure | [`headers-and-libraries.md`](headers-and-libraries.md) |
| New / removed / resized library | [`headers-and-libraries.md`](headers-and-libraries.md), [`repo-layout.md`](repo-layout.md) |
| Manifest field add/remove/rename | [`manifest.md`](manifest.md) + the closed-schema validator in [`tools/googlesql-prebuilt/manifest_writer.py`](../../../tools/googlesql-prebuilt/manifest_writer.py) |
| Toolchain / platform change | [`manifest.md`](manifest.md) (`platform.*`, `toolchain.*`), [`repo-layout.md`](repo-layout.md) |
| Bundled-thirdparty change | [`manifest.md`](manifest.md) (`bundled_thirdparty_deps`), the wrapper repo's `MODULE.bazel.tmpl` under `tools/googlesql-prebuilt/templates/` |

All such commits carry a `BREAKING CHANGE` footer per
[`upgrade-rules.md`](upgrade-rules.md). Don't try to slip a manifest
field rename in under a non-breaking commit — the validator will
reject artifacts produced under the new schema and the consumer side
won't deserialise them either.

When the manifest schema bumps, the closed-schema validator in
[`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py)
needs the matching update too (the validator delegates to
`manifest_writer.validate_manifest`, which carries the field
schema). Safety-gate logic tests live next to the validator.

## 4. Produce the new artifact

Dispatch the producer workflow with the new SHA:

```bash
gh workflow run googlesql-prebuilt.yml \
    -f googlesql_ref=<new-tag-or-sha> \
    -f artifact_version=<new-x.y.z> \
    -f publish=true
```

- Pick `artifact_version` per [`upgrade-rules.md`](upgrade-rules.md)'s
  guidance — major/minor/patch bump rules vary by change class.
- Watch the run for failures. The producer's two smoke modes
  (`verify.sh --smoke-mode=link` and `--smoke-mode=bazel`) catch
  most "labels moved / headers missing" regressions before publish.
- The pre-publish overwrite gate refuses to start if a release
  asset for the same `(artifact_version, googlesql_short_sha)`
  already exists — bump `artifact_version` to recover.

A successful publish writes three assets under the release tag
`googlesql-prebuilt/v<artifact_version>+gs-<short>` (`.tar.gz`,
`.tar.gz.sha256`, `.manifest.json`). See the
[runbook §3](maintainer-runbook.md#3-where-artifacts-live-publish-targets)
for full URLs.

## 5. Verify the package locally

Re-run the consumer-side gates from the maintainer machine —
catches `gh release upload` mistakes and verifies the cache path
the engine build takes:

```bash
# Pull + SHA-check + validator gate the artifact end to end.
TAG=googlesql-prebuilt/v<new-x.y.z>+gs-<short>
ASSET=googlesql-prebuilt-linux-amd64-clang18-<short>-v<new-x.y.z>.tar.gz
SHA=$(gh release download "$TAG" --pattern "$ASSET.sha256" --dir /tmp \
      && awk '{print $1}' "/tmp/$ASSET.sha256")
URL="https://github.com/<owner>/<repo>/releases/download/$TAG/$ASSET"

task googlesql:clean   # start from an empty cache (no leftover prior pin)
task googlesql:fetch-prebuilt URL="$URL" SHA256="$SHA"

# Explicit identity pins so the manifest's googlesql.commit and
# artifact_version must match what you produced.
task googlesql:validate \
    EXPECTED_ARTIFACT_VERSION=<new-x.y.z> \
    EXPECTED_GOOGLESQL_SHA=<resolved 40-hex>

# Engine build smoke (the canonical consumer of the artifact).
task emulator:build-engine:bazel
./bin/emulator_main --version
```

Any `FAIL_*` token at this stage is a producer / packaging bug — do
not pin the artifact. [`troubleshooting.md`](troubleshooting.md)
routes tokens to the owner; [`rollback.md`](rollback.md) has the
"discard this artifact" procedure.

## 6. Pin the artifact in consumer wiring

Two pin surfaces. Update both atomically (one commit per surface
per the auto-commit rule):

### 6.1 Repo-level Actions variables (per-PR CI lanes)

```bash
gh variable set GOOGLESQL_PREBUILT_URL --body "$URL" --repo <owner>/<repo>
gh variable set GOOGLESQL_PREBUILT_SHA256 --body "$SHA" --repo <owner>/<repo>
gh variable list --repo <owner>/<repo>
```

These cover every lane that calls `setup-googlesql` without an
explicit override: build-engine.yml, conformance.yml,
thirdparty-samples.yml, docker-smoke.yml, and googlesql-parity.yml's
prebuilt leg.

### 6.2 In-tree release pin

Edit [`.github/workflows/release.yml`](../../../.github/workflows/release.yml):

```yaml
env:
  RELEASE_GOOGLESQL_PREBUILT_URL: "<url>"
  RELEASE_GOOGLESQL_PREBUILT_SHA256: "<sha>"
```

Commit with `chore(release): pin GoogleSQL prebuilt v<new-x.y.z>+gs-<short>`
so future rollbacks can walk back via
`git log --oneline --grep 'pin GoogleSQL prebuilt'`.

If the upgrade is **breaking** (per [`upgrade-rules.md`](upgrade-rules.md))
include the breaking-change footer in this commit's body — that's the
commit that flips production lanes onto the new pin.

## 7. Run the parity job

The parity job ([`googlesql-parity.yml`](../../../.github/workflows/googlesql-parity.yml))
builds the same emulator commit twice — once against the pinned
prebuilt, once against an upstream source checkout — and diffs the
engine startup smoke, the smoke query set, and (scheduled / release
tier) the full conformance lanes.

After pinning:

```bash
# PR tier (fast): link + startup + smoke query set.
gh workflow run googlesql-parity.yml -f tier=pr -f googlesql_ref=<new>

# Scheduled tier (adds memory conformance lane against both modes).
gh workflow run googlesql-parity.yml -f tier=scheduled -f googlesql_ref=<new>

# Release tier (adds the duckdb conformance lane on top of scheduled).
gh workflow run googlesql-parity.yml -f tier=release -f googlesql_ref=<new>
```

Read the comparator job's "compare prebuilt vs source" workflow
summary. `matched` on every stage is required to move on. A
`DIVERGED` result is a safety-gate hard rollback signal — see
[`rollback.md`](rollback.md#1-parity-job-failures) for the response.

The scheduled tier also runs nightly on cron; if you have time
before the release, let one nightly run complete on the new pin as
extra confidence.

## 8. Roll through the release pin

Once the parity job is green on the release tier:

1. Confirm `.github/workflows/release.yml`'s `env.RELEASE_GOOGLESQL_PREBUILT_*` reflect the new artifact (step 6.2).
2. Tag the emulator release as usual (`task release:tag VERSION=vX.Y.Z CONFIRM=yes`).
3. The release workflow's "Validate GoogleSQL prebuilt pin" step gates on both env values being non-empty + the SHA being valid hex; the build then resolves them via `setup-googlesql` and the cold-cache cost drops to whatever the prebuilt artifact saves (see [`performance.md`](performance.md)).
4. After publish, the release notes carry the GoogleSQL provenance block (URL, SHA-256, upstream tag, manifest schema version, libc, compiler) so consumers can reproduce the build from the same artifact.

## Breaking-case quick reference

If the upgrade touches any of these surfaces, the commit body MUST
carry a `BREAKING CHANGE` footer per
[`upgrade-rules.md`](upgrade-rules.md) and `.cursor/rules/auto-commit.mdc`.
The matching follow-up work is the producer (step 4) + the consumer
pin (step 6) + the parity job (step 7) — none can be skipped.

| Breaking change | Required follow-up |
|---|---|
| Removed or renamed `@googlesql//...` label | Update [`label-inventory.md`](label-inventory.md) + [`repo-layout.md`](repo-layout.md) + every emulator `BUILD.bazel` that references the old label in **one atomic commit**, then produce + repin. |
| New required library or header | Update [`headers-and-libraries.md`](headers-and-libraries.md). Producer's `package.sh` must pick up the new label/header set; verify with `verify.sh --smoke-mode=bazel`. |
| Toolchain (compiler / libc / Bazel) change | Update [`manifest.md`](manifest.md) `platform.libc` / `toolchain.*`. Re-evaluate `setup-googlesql`'s `expected-*` defaults. Document the runtime impact in the release notes (especially for libc — a glibc bump can lock out older base images). |
| Platform tuple (linux/arm64, darwin/...) added or dropped | Sibling `@googlesql_prebuilt_<os>_<arch>` repo. Producer workflow becomes a matrix. Consumer-wiring wrapper `select()` lights up. |
| `manifest.json` schema change (field add/rename/remove, `schema_version` bump) | Update [`manifest.md`](manifest.md), `tools/googlesql-prebuilt/manifest_writer.py` (closed-schema validator), and [`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py). Producer regenerates artifacts under the new schema. Existing pins keep using the old schema until they expire. |

## See also

- [`upgrade-rules.md`](upgrade-rules.md) — formal change taxonomy + breaking-change-footer requirement.
- [`maintainer-runbook.md`](maintainer-runbook.md) — steady-state publish / pin / verify / roll-back flow without an upstream bump.
- [`performance.md`](performance.md) — what improves and what doesn't after the bump lands.
- [`troubleshooting.md`](troubleshooting.md) — `FAIL_*` validator-token map for unblocking step-5 / step-7 failures.
- [`rollback.md`](rollback.md) — safety-gate rollback playbook (parity-failure response).
