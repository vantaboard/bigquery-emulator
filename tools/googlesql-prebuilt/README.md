# `tools/googlesql-prebuilt/` — artifact producer

This directory holds the producer-side tooling for the
[GoogleSQL prebuilt rollout](../../docs/dev/googlesql-prebuilt/README.md).
It builds the `@googlesql_prebuilt_linux_amd64` external repo described in
[`repo-layout.md`](../../docs/dev/googlesql-prebuilt/repo-layout.md),
verifies it independently of the source checkout, and (via the
matching workflow) publishes the artifact as a GitHub Release asset.

The artifact-producer pipeline does **not** flip normal emulator builds
onto the prebuilt artifact — that lives in the consumer-wiring track. The
producer only delivers the producer scripts + verifier.

## Publication medium

**GitHub Release assets**, attached to a dedicated
`googlesql-prebuilt/vX.Y.Z+gs-<short_sha>` tag per `(artifact_version,
googlesql_sha)` pair. Rationale:

1. **Fits `http_archive` natively.** The compatibility-surface manifest
   contract (`manifest.md`) names `.tar.gz` as the distribution format and
   tells the consumer to pin via
   `http_archive(sha256 = ..., strip_prefix = ...)`. GitHub Release
   assets have stable, immutable URLs of the form
   `https://github.com/<owner>/<repo>/releases/download/<tag>/<asset>`,
   which is exactly the shape `http_archive` expects.
2. **Bisecting-friendly.** Each tag carries `(artifact_version,
   googlesql_sha)` in its name, so `gh release list --json tagName`
   lets a maintainer walk back through historical artifacts when
   triaging a parity-CI failure.
3. **No extra OCI tooling on the consumer side.** GHCR OCI artifacts
   would be attractive for digest-pinning, but consumers would need
   to wire an OCI puller (`oras`, `crane`, or Bazel's experimental
   `oci` rule) into their `MODULE.bazel` extension. The consumer-wiring
   track is already non-trivial; introducing a second medium at the
   same time is unnecessary risk.

GHCR / OCI may be layered ON TOP of releases later if a measured
benefit appears (per-digest immutability guarantees, multi-arch fanout,
etc.). The producer's contract — manifest schema, tarball shape,
checksum semantics — is medium-agnostic.

## Tools

| File                                          | Role                                                                  |
|-----------------------------------------------|----------------------------------------------------------------------|
| [`package.sh`](package.sh)                    | Orchestrator. Stages the repo layout, runs `bazel build` (or stages a stub fixture), generates wrapper BUILDs + manifest, tarballs the staged tree. |
| [`verify.sh`](verify.sh)                      | Refusal-grade verifier. Unpacks the tarball into a clean tmpdir, validates manifest schema, re-checksums every payload entry, and runs a smoke binary against the packaged wrappers. |
| [`manifest_writer.py`](manifest_writer.py)    | Emits `manifest.json` per the closed compatibility-surface schema. Also exposes `--validate-only PATH` for the verifier. |
| [`wrapper_writer.py`](wrapper_writer.py)      | Emits `googlesql/public/BUILD.bazel` and `googlesql/resolved_ast/BUILD.bazel` for the staged artifact (the `hdrs` for `:type` and `:resolved_ast` are dynamic, so the producer materialises them at package time). |
| [`templates/`](templates/)                    | Static template files (root `BUILD.bazel`, `MODULE.bazel.tmpl`). Shipped verbatim into the artifact. |
| [`smoke/`](smoke/)                            | Source for the two smoke variants the verifier runs (portable clang link + bazel-wrappers).            |

## Layout shipped in the artifact

Matches [`repo-layout.md`](../../docs/dev/googlesql-prebuilt/repo-layout.md)
exactly. Validated by `verify.sh` after each package run.

```text
googlesql_prebuilt_linux_amd64/
├── BUILD.bazel               # :_archive private cc_imports + exports_files
├── MODULE.bazel              # version pins (artifact + absl/protobuf/grpc)
├── manifest.json             # compatibility-surface closed schema
├── LICENSES/
│   ├── googlesql-LICENSE     # upstream license verbatim
│   └── thirdparty-NOTICE     # consumer-resolved dep notice
├── include/
│   └── googlesql/...         # header tree (mirror of upstream source paths)
├── lib/
│   ├── libgooglesql.a        # combined static archive
│   └── libgooglesql_protos.a # generated .pb.cc objects
├── googlesql/public/
│   └── BUILD.bazel           # 16 wrapper cc_library targets
└── googlesql/resolved_ast/
    └── BUILD.bazel           # 2 wrapper cc_library targets
```

## Compatibility-surface clarification: wrapper BUILD files live in subpackages

The compatibility-surface layout doc states "The top-level `BUILD.bazel`
is the only `BUILD` file in the prebuilt repo." That's slightly inaccurate
given the label space frozen in
[`manifest.md`](../../docs/dev/googlesql-prebuilt/manifest.md)'s
`compat.labels` (which uses `//googlesql/public:analyzer` and
`//googlesql/resolved_ast:resolved_ast`). Those labels require BUILD
files at the corresponding subpackage paths — Bazel labels do not
support targets defined in a parent package's BUILD claiming
ownership of a sub-package label space.

This producer therefore emits a small `BUILD.bazel` under
`googlesql/public/` and `googlesql/resolved_ast/` in addition to the
root. The label space (`//googlesql/public:analyzer` etc.), the
strict-deps narrowing semantics, and the consumer-side wiring all
remain identical to the compatibility-surface intent. The producer's
hdrs / deps lists for each wrapper match
[`label-inventory.md`](../../docs/dev/googlesql-prebuilt/label-inventory.md)
row-for-row. The doc text in
[`repo-layout.md`](../../docs/dev/googlesql-prebuilt/repo-layout.md)
should be updated to reflect this when the compatibility-surface docs
next see an intentional change (the producer hasn't touched the docs
because the contract — what `compat.labels` says — is what matters).

## Compatibility-surface clarification: artifact MODULE.bazel identity is `googlesql`

[`repo-layout.md`](../../docs/dev/googlesql-prebuilt/repo-layout.md)
says "The repo is named `@googlesql_prebuilt_linux_amd64` in the
consumer's Bzlmod graph" and goes on to claim a consumer "can swap
`@googlesql//` for `@googlesql_prebuilt_linux_amd64//` in a single
`bazel mod` extension and everything resolves." That swap is not how
the consumer-wiring track wires consumption.

The compatibility-surface
[`label-inventory.md`](../../docs/dev/googlesql-prebuilt/label-inventory.md)
froze 18 emulator-side `@googlesql//...` references and explicitly
disallowed renaming them: rewriting every emulator `BUILD.bazel` to
the prebuilt repo's name would be label churn the rollout was
designed to avoid. The clean two-mode story (consumer-wiring plan
[`googlesql-prebuilt-consume-wiring_3b4c5d6e.plan.md`](../../docs/dev/googlesql-prebuilt/upgrade-rules.md))
needs **the same** `@googlesql//` label space in both modes so the
emulator BUILDs do not change. That is only possible if the prebuilt
artifact's `MODULE.bazel` advertises module identity `googlesql`,
not `googlesql_prebuilt_linux_amd64`.

This producer therefore emits a `module(name = "googlesql", ...)`
declaration in the artifact's `MODULE.bazel`. The other three names
the rollout pins are unchanged:

- Tarball top-level directory  : `googlesql_prebuilt_linux_amd64/` (compatibility-surface freeze).
- GitHub Release asset prefix  : `googlesql-prebuilt-linux-amd64-...` (artifact-producer contract).
- Tarball/Release tag           : `googlesql-prebuilt/v<version>+gs-<short_sha>` (artifact-producer contract).

Only the in-Bzlmod identity (the `module(name = ...)` field that
`bazel_dep` and `--override_module` resolve against) is `googlesql`.
The platform discriminator stays the tarball directory name + asset
name suffix, so a future `linux/arm64` artifact would publish a
sibling tarball `googlesql_prebuilt_linux_arm64/` whose
`MODULE.bazel` also says `module(name = "googlesql", ...)` — the
selection between platforms still happens at the consumer's
`select()` (or via a per-platform `archive_override`).

The
[compatibility-surface upgrade rules](../../docs/dev/googlesql-prebuilt/upgrade-rules.md)
do not classify this clarification as breaking: no exposed wrapper
label changed, no payload bytes changed, no manifest field changed.
What changed is which name the consumer's `bazel_dep` resolves to,
and that is documented here so a future producer change cannot drift
from this commit's intent.

## Local exercise (no real bazel build)

```bash
# 1. Make sure you have a sibling googlesql/ checkout next to bigquery-emulator/.
ls ../googlesql/MODULE.bazel  # must exist

# 2. Stage a fixture artifact (stub libraries, hand-picked headers).
mkdir -p /tmp/gsq-prebuilt-out
tools/googlesql-prebuilt/package.sh \
    --mode=fixture \
    --googlesql-src ../googlesql \
    --emulator-src "$PWD" \
    --artifact-version 0.0.0 \
    --out-dir /tmp/gsq-prebuilt-out \
    --workflow-id manual \
    --run-id 0

# 3. Verify the produced tarball with the hash + clang link smoke.
sha256=$(awk '{print $1}' /tmp/gsq-prebuilt-out/googlesql-prebuilt-*.tar.gz.sha256)
tools/googlesql-prebuilt/verify.sh \
    --tarball /tmp/gsq-prebuilt-out/googlesql-prebuilt-*.tar.gz \
    --tarball-sha256 "$sha256" \
    --smoke-mode=link
```

Fixture mode emits **stub** static archives via `ar rcS` — empty `ar`
archives that satisfy the linker but contain zero objects. The
clang-link smoke is symbol-free, so the link succeeds. This
exercises packaging + verification end-to-end without the 25-55 min
GoogleSQL bazel build.

For a real publish, use the workflow (`--mode=bazel` is reserved for
the CI environment because the harvest logic assumes a complete
`bazel-bin/googlesql/` tree).

## Producing a real artifact (CI workflow dispatch)

Trigger
[`.github/workflows/googlesql-prebuilt.yml`](../../.github/workflows/googlesql-prebuilt.yml)
manually:

1. **Actions → googlesql-prebuilt → Run workflow.**
2. Inputs:
   - `googlesql_ref`: upstream tag or commit to pin (default `2026.01.1`).
   - `artifact_version`: strict semver (e.g. `0.1.0`).
   - `publish`: set `true` to upload as a Release asset; leave `false`
     to only run the build + verify gates.
3. The workflow:
   - Checks out the emulator + sibling googlesql.
   - Runs `package.sh --mode=bazel` (25-55 min cold cache).
   - Runs `verify.sh --smoke-mode=link` (portable clang).
   - Runs `verify.sh --smoke-mode=bazel` (real wrappers).
   - If `publish=true`, refuses to overwrite an existing asset for
     the same `(artifact_version, googlesql_sha)` pair, then uploads
     the tarball + sidecar manifest + `.sha256` to a Release tag
     named `googlesql-prebuilt/v<version>+gs-<short_sha>`.

The workflow ALSO uploads the tarball, manifest, and build log as a
plain workflow artifact (30-day retention) regardless of the publish
input — so a failed verify still leaves a forensic trail without
committing to a permanent Release.

## Refusal contracts

The producer + verifier collectively refuse to publish if **any** of
the following gates fail. Each refusal cites the offending field.

| Gate                                              | Refusal trigger                                                                                                            |
|---------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|
| Strict-semver `--artifact-version`                | Not matching `MAJOR.MINOR.PATCH[-prerelease]`.                                                                             |
| Manifest schema validation                        | Unknown top-level field, missing required field, wrong type, malformed SHA.                                                |
| `compat.labels` set                               | Not exactly the 18 labels frozen in [`label-inventory.md`](../../docs/dev/googlesql-prebuilt/label-inventory.md).         |
| Per-file SHA-256 verification                     | Any file under the unpacked tarball has a SHA-256 that doesn't match its manifest entry, or any manifest entry lacks a file. |
| Path-traversal check                              | Any tarball entry has an absolute path or a `..` segment.                                                                 |
| Tarball SHA-256                                   | The provided `--tarball-sha256` doesn't match the actual hash.                                                            |
| Smoke (link or bazel)                             | Compile, link, or smoke binary execution fails.                                                                            |
| Overwrite gate (publish only)                     | A Release asset for the same `(artifact_version, googlesql_sha)` already exists.                                          |

Once all gates pass, the workflow surfaces the artifact URL +
SHA-256 in the run's GitHub Step Summary so a consumer-wiring caller
can copy-paste it into a `MODULE.bazel`.
