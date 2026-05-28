# GoogleSQL Prebuilt — Troubleshooting

Short troubleshooting reference for the GoogleSQL prebuilt path.
Routes the safety-gate validator's `FAIL_*` tokens (emitted by
[`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py))
and the most common operator-visible failure shapes to the **likely
owner** so the right person picks it up. This page intentionally
**does not** restate the rollback procedure — for the "what do I
actually do about it" step, follow the links into
[`rollback.md`](rollback.md).

## Owners at a glance

| Owner | What they own | When to ping |
|---|---|---|
| **Artifact producer** | The `.tar.gz` + `manifest.json` published under `googlesql-prebuilt/v<...>+gs-<...>` GitHub Release. | When the artifact itself is corrupt, drifted from its manifest, or mis-claimed its platform / identity / schema. |
| **Consumer pin** | `vars.GOOGLESQL_PREBUILT_URL` / `_SHA256` (CI) and `env.RELEASE_GOOGLESQL_PREBUILT_URL` / `_SHA256` (release.yml). | When the pin references the wrong artifact (URL points at a stale release, SHA mismatches the asset, or no pin is set on a `fail-on-source-fallback: true` lane). |
| **Local environment** | The developer machine. The prebuilt cache at `.cache/googlesql-prebuilt/`, the sibling `../googlesql/` checkout, the host's libc / compiler / `nproc` / `MemTotal`. | When the failure reproduces only on one host, or the cache state is the suspect. |
| **Compatibility surface** | The compatibility-surface docs under this directory. | When the artifact is structurally fine but the consumer expects more labels / headers / manifest fields than the artifact carries (or vice versa). |

## Validator FAIL_* tokens

Every validator failure prints a stable token on stderr. The list
below maps token → likely owner → next step. The validator's diagnostic
also names the failing field, expected value, and actual value — read
those first. The full per-token rollback response lives in
[`rollback.md`](rollback.md#failure-tokens-for-runbook-lookup); this
page narrows that to the "who picks it up" half so triage is fast.

### Payload integrity tokens (almost always the artifact producer)

| Token | Likely owner | Next step |
|---|---|---|
| `FAIL_PAYLOAD_SHA` | **Artifact producer.** A library or header on disk does not match the manifest's recorded SHA-256. The artifact was tampered with mid-flight, the producer republished under a stale tag, or the upload corrupted the asset. | Refetch from a clean cache (`task googlesql:clean && task googlesql:fetch-prebuilt URL=... SHA256=...`). If still failing, the artifact is compromised — **never** override the SHA. [`rollback.md`](rollback.md#3-link--runtime-abi-mismatch) → repin to the prior known-good artifact and open a producer issue. |
| `FAIL_PAYLOAD_MISSING` | **Artifact producer.** A manifest entry has no corresponding file on disk. The unpacked artifact is incomplete (partial extract, truncated upload, missing file in the producer's `package.sh` output set). | Repin / refetch. Same rollback procedure as `FAIL_PAYLOAD_SHA`. |
| `FAIL_PAYLOAD_SIZE` | **Artifact producer.** A library's `size_bytes` disagrees with the on-disk file size. Same root-cause class as `FAIL_PAYLOAD_SHA` — artifact was tampered or republished. | Repin to the prior artifact; open a producer issue. |
| `FAIL_PAYLOAD_ESCAPE` | **Artifact producer.** A payload path tries to escape the artifact root (absolute path, or contains `..`). This is either a producer bug (`package.sh` emitted a bad relative path) or active tampering. **Do not consume.** | Refuse the pin, open a producer issue immediately. |
| `FAIL_PAYLOAD_UNACCOUNTED` | **Artifact producer.** A file lives on disk under the artifact root but no manifest entry references it. The manifest is closed by contract — strays are a producer drift bug. | Republish the artifact (the producer must regenerate `manifest.json` to reference the new file, or remove the file from the tarball). Repin to the new artifact. |
| `FAIL_WRAPPER_MISSING` | **Artifact producer.** A required wrapper file is absent (`MODULE.bazel`, `BUILD.bazel`, `googlesql/public/BUILD.bazel`, `googlesql/resolved_ast/BUILD.bazel`). The artifact is corrupt or stale. | Repin / refetch; if reproducible across pulls, open a producer issue against the asset URL. |

### Schema / identity tokens (artifact producer **and** compatibility surface)

| Token | Likely owner | Next step |
|---|---|---|
| `FAIL_SCHEMA` | **Artifact producer** or **compatibility surface** — depends on what changed. The closed-schema validator rejected the manifest. Two cases: (a) the producer added an optional field the validator's allowlist doesn't include yet; (b) the producer bumped `schema_version` without a matching consumer update. | (a) is a [`manifest.md`](manifest.md) docs + `tools/googlesql-prebuilt/manifest_writer.py` validator update (compatibility surface); (b) requires updating the validator atomically with the producer per [`upgrade-rules.md`](upgrade-rules.md). [`rollback.md`](rollback.md#5-manifest-validation-false-positives-caused-by-schema-drift) covers both branches. **Do not disable the validator.** |
| `FAIL_IDENTITY_PIN` | **Consumer pin.** Caller passed `--expected-googlesql-sha` / `--expected-artifact-version` (or the equivalent `setup-googlesql` inputs) that does not match the manifest. Either the wrong artifact landed in the cache or the pin is wrong. | Confirm both: read the asset's `manifest.json` (the [`maintainer-runbook.md`](maintainer-runbook.md#4-read-the-manifest-digest--checksum) shows the `gh release download` recipe), then update either the pin or refetch. |
| `FAIL_MANIFEST_MISSING` | **Artifact producer** or **local environment.** `manifest.json` is absent under the artifact root. Either the upload was incomplete, the cache was hand-edited, or an unrelated tool deleted the file. | `task googlesql:clean && task googlesql:fetch-prebuilt URL=... SHA256=...` (consumer / local env). If the freshly-fetched cache still misses `manifest.json`, the artifact is corrupt — repin. |
| `FAIL_MANIFEST_PARSE` | **Artifact producer.** `manifest.json` exists but does not parse as JSON. The artifact is corrupt mid-write or the upload truncated it. | Repin / refetch. |

### Platform / toolchain tokens (consumer pin or local environment)

| Token | Likely owner | Next step |
|---|---|---|
| `FAIL_PLATFORM` | **Local environment** if the consumer host drifted (e.g. you moved from a glibc-2.31 box to glibc-2.18) or **artifact producer** if the artifact was built on the wrong toolchain. The validator gates `platform.os`, `platform.arch`, `platform.libc`, `platform.cxx_abi`, `toolchain.compiler`, `toolchain.compiler_version`, and `toolchain.bazel_version`. | If the artifact is for a platform you don't target, repin to the right artifact (likely a sibling `@googlesql_prebuilt_<os>_<arch>` once that exists). If the artifact targets your platform but the host moved underneath you, either rebuild the producer with the new toolchain (preferred) or fall back to source mode (`GOOGLESQL_SOURCE=local task emulator:build-engine:bazel`). [`rollback.md`](rollback.md#3-link--runtime-abi-mismatch) covers the ABI-drift case. |
| `FAIL_REPO_ROOT` | **Local environment** (consumer-side bug). The validator was pointed at a path that is not a directory. | Re-stage via `task googlesql:fetch-prebuilt`. If you invoked the validator directly, check `--repo-root` — it must point at the unpacked artifact root (the `googlesql_prebuilt_linux_amd64/` directory). |

## Operator-visible failure shapes (no `FAIL_*` prefix)

Beyond the validator, a few non-token failures recur. Owners follow
the same routing logic.

### "GOOGLESQL_SOURCE=prebuilt … but the cache is empty"

**Likely owner: local environment.** The pre-flight gate in
[`taskfiles/emulator.yml`](../../../taskfiles/emulator.yml)
refuses the build because `.cache/googlesql-prebuilt/googlesql_prebuilt_linux_amd64/`
does not contain `MODULE.bazel`. Populate the cache (one of):

```bash
task googlesql:fetch-prebuilt URL=<asset url> SHA256=<64 hex>
task googlesql:stage-bazel       # local build (~25-55 min cold)
task googlesql:stage-fixture     # stub (smoke only; cannot link emulator_main)
```

Or fall back to source mode explicitly:

```bash
GOOGLESQL_SOURCE=local task emulator:build-engine:bazel
```

### CI workflow warning: "vars.GOOGLESQL_PREBUILT_URL/_SHA256 not set; falling back to source GoogleSQL build (slower)"

**Likely owner: consumer pin.** The repo-level Actions variables are
not populated, so `setup-googlesql` is falling back to a sibling
source checkout. On `fail-on-source-fallback: 'false'` lanes (ci.yml,
conformance.yml, etc.) the build still passes — slowly. On
`fail-on-source-fallback: 'true'` lanes (release.yml, the parity
job's prebuilt leg) the lane fails fast. Set the repo vars per
[`maintainer-runbook.md`](maintainer-runbook.md#61-repo-level-actions-variables-the-per-pr--per-push-ci-lanes).

### "release.yml has no GoogleSQL prebuilt pin set"

**Likely owner: consumer pin.** The release workflow's "Validate
GoogleSQL prebuilt pin" step refuses to run a tag-push release when
`env.RELEASE_GOOGLESQL_PREBUILT_URL` / `_SHA256` are empty. Either
update the in-tree pin per
[`maintainer-runbook.md`](maintainer-runbook.md#62-release-pin-the-releaseyml-workflow)
**or** use the workflow-dispatch debug knob
(`gh workflow run release.yml -f ref=vX.Y.Z -f googlesql_source=true`)
when the release explicitly needs a source build (e.g. the prebuilt
producer is also broken).

### Multiple-definition linker errors at engine link time

**Likely owner: compatibility surface (and possibly the artifact producer).**
This is almost always a `bundled_thirdparty_deps` drift between the
artifact and the consumer's resolved Bzlmod graph — for example, the
artifact bundles Abseil but the consumer's `MODULE.bazel` resolves
Abseil to a different version. Read
[`rollback.md`](rollback.md#3-link--runtime-abi-mismatch); revert to
the previous pin, then open a producer issue against the bundle.

### Parity job reports `DIVERGED` after a pin bump

**Likely owner: artifact producer + compatibility surface.** The
source leg and the prebuilt leg disagree on engine startup smoke,
the smoke query set, or a conformance result. This is exactly the
class of regression the parity job exists to catch. The default
response is to **repin to the prior known-good artifact** while the
producer side is diagnosed. Full procedure:
[`rollback.md`](rollback.md#1-parity-job-failures).

If the parity job has been green and then divergence appears after
an emulator commit (not a pin bump), the regression is on the
consumer side — diagnose normally; the prebuilt pin is fine.

### Engine boot fails (`emulator_main --version` crashes) after a pin bump

**Likely owner: artifact producer (ABI drift).** Engine startup is
the safety-gate smoke check. A regression here strongly implies a
libstdc++ / abseil / protobuf ABI drift between the new artifact and
the runtime environment. Repin to the prior artifact; if the
regression persists, the change is in the emulator and the prebuilt
pin is fine. See
[`rollback.md`](rollback.md#2-conformance--runtime-regressions-traced-to-googlesql).

### `setup-googlesql` warning about "module name is 'X', not 'googlesql'"

**Likely owner: artifact producer.** The cache contains a directory
whose `MODULE.bazel` declares the wrong module name, so
`--override_module=googlesql=<cache>` cannot resolve. This can only
happen if the producer published an artifact built from
[`tools/googlesql-prebuilt/templates/MODULE.bazel.tmpl`](../../../tools/googlesql-prebuilt/templates/)
without the correct substitution. Repin to a known-good artifact
and open a producer issue.

## Local escape hatches recap

When you need to keep working while triage happens, the validator
itself names the supported escape hatches in every failure block:

- `GOOGLESQL_SOURCE=local task emulator:build-engine:bazel` — rebuild
  from the sibling `../googlesql/` checkout. Slow but always works
  when the sibling tree is present.
- `BIGQUERY_EMULATOR_SKIP_PREBUILT_VALIDATE=1 task emulator:build-engine:bazel`
  — skip the validator pre-flight only. The cache contents are still
  expected to be correct (`MODULE.bazel` must exist with the right
  module name). **Never** use this to mask a `FAIL_PAYLOAD_SHA` or a
  `FAIL_PAYLOAD_UNACCOUNTED`; only to debug the validator itself.
- `gh workflow run release.yml -f ref=<tag> -f googlesql_source=true`
  — release-grade source rebuild. The documented escape hatch when a
  release must ship and the prebuilt path is broken.
- `task googlesql:validate` — re-run the validator over the currently
  staged cache without rebuilding. Useful when triaging a `FAIL_*`
  token to confirm it reproduces and to capture a `--summary-json`
  for an issue.

## See also

- [`rollback.md`](rollback.md) — safety-gate rollback playbook (full per-token rollback procedure, parity-failure response, revert-prebuilt-adoption procedure).
- [`maintainer-runbook.md`](maintainer-runbook.md) — publish / pin / verify (the "how do I get a clean known-good artifact" flow).
- [`upgrade-procedure.md`](upgrade-procedure.md) — full upgrade flow (when a `FAIL_SCHEMA` or `FAIL_PLATFORM` traces back to an upstream bump that broke the surface).
- [`performance.md`](performance.md) — for the "build is slow" symptoms that are **not** the prebuilt path's fault.
- [`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py) — the validator whose tokens this doc maps.
