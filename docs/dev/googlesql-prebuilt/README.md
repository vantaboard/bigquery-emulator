# GoogleSQL Prebuilt — Compatibility Surface

The [GoogleSQL prebuilt rollout](../../../README.md) freezes the
**compatibility contract** between the GoogleSQL prebuilt artifact and the rest
of the emulator's Bazel build. Nothing in this directory builds or publishes an
artifact — that work lives in the artifact-producer pipeline and the
consumer-wiring tracks.

The contract documented here is the source of truth for the four downstream
tracks (producer, consumer wiring, CI/Docker/release, safety gates). Any change
that breaks one of these documents is a **breaking change** for the rollout and
must be called out as such in commit messages and CI release notes.

## Scope

- Platforms: `linux/amd64` (shipping) and `linux/arm64` (producer + CI spike in
  flight — see [`arm64-feasibility.md`](./arm64-feasibility.md)).
- Upstream GoogleSQL pin: module name `googlesql`, version `2026.1.1`
  (upstream tag `2026.01.1`, commit `36dd14aa0657ea299725504bc0f938732f58f380`
  — see `MODULE.bazel` in this repo for the gazelle leading-zero workaround).
- Surface: only the `@googlesql//` Bazel labels that the emulator's `BUILD.bazel`
  files reference directly today. Indirect / transitive labels are packaged but
  not exposed.

## Documents in this directory

Compatibility surface:

1. [`label-inventory.md`](label-inventory.md) — label inventory.
   Every `@googlesql//` label the emulator references, with the owning emulator
   package, the public headers each label exposes, and a per-label
   prebuilt-disposition decision.
2. [`headers-and-libraries.md`](headers-and-libraries.md) — headers and library
   shape. The minimal header include-root layout, the library shape (single
   combined archive vs. per-label `cc_import`), and the static-vs-shared linking
   decision.
3. [`repo-layout.md`](repo-layout.md) — repo layout.
   The on-disk layout of the unpacked external repo
   (`@googlesql_prebuilt_linux_amd64//`), wrapper-target conventions, and the
   full source-label → prebuilt-wrapper mapping.
4. [`manifest.md`](manifest.md) — manifest schema.
   Manifest field schema, an example `manifest.json`, the consumer-pinned
   field list, and the rule that checksum mismatches are hard errors.
5. [`upgrade-rules.md`](upgrade-rules.md) — done criteria.
   How the frozen surface may evolve during a GoogleSQL upgrade and which
   changes cascade as breaking work into the producer, consumer-wiring,
   CI/Docker/release, or safety-gate tracks.

Safety gates:

6. [`rollback.md`](rollback.md) — rollback playbook.
   When and how to repin or revert prebuilt adoption when the parity job,
   conformance lane, or validator surfaces a regression. Lists every
   `FAIL_*` token emitted by
   [`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py)
   with the recommended response.

Docs and operations:

7. [`maintainer-runbook.md`](maintainer-runbook.md) — maintainer artifact runbook.
   The steady-state publish / pin / verify / roll-back flow: how to run the
   producer workflow, find the asset URL + SHA, update the consumer pins
   (`vars.GOOGLESQL_PREBUILT_*` + `env.RELEASE_GOOGLESQL_PREBUILT_*`), and
   roll back via repin.
8. [`upgrade-procedure.md`](upgrade-procedure.md) — upgrade procedure.
   Ordered checklist for bumping the upstream GoogleSQL pin end to end:
   source SHA candidate → label inventory → wrapper/manifest changes if needed
   → produce → verify → pin → parity → release pin. Calls out the breaking
   cases (label renames, schema bumps, toolchain changes).
9. [`performance.md`](performance.md) — cache and performance expectations.
   What improves (local rebuilds, PR CI, Docker `ENGINE_SOURCE=bazel`,
   release determinism) and what does not (source mode still slow, first-time
   download cost, non-GoogleSQL C++ still compiles, prebuilt-engine Docker
   image stays fastest).
10. [`troubleshooting.md`](troubleshooting.md) — validator troubleshooting.
    Maps every safety-gate `FAIL_*` token to the likely owner (artifact producer,
    consumer pin, local environment, compatibility surface) and points back
    into `rollback.md` for the rollback procedure.

## What the compatibility surface does NOT do

- It does not build or publish any prebuilt artifact (that is the
  artifact-producer track).
- It does not change emulator `BUILD.bazel` files or `MODULE.bazel` (that is
  the consumer-wiring track).
- It does not wire CI, Docker, or release pipelines (that is the
  CI/Docker/release track).
- It does not add manifest verification or source-vs-prebuilt parity checks
  (that is the safety-gates track).
- It does not update user/maintainer docs outside this directory (that is the
  docs-and-operations track).

## Verification (must remain green after every edit to the compatibility surface)

The bullets below are the verification checklist for the compatibility-surface
contract. Re-run after every edit to these docs.

- `rg '@googlesql//' /home/brighten-tompkins/Code/bigquery-emulator --glob '*.bazel' --glob 'BUILD' --glob 'BUILD.bazel'`
  must contain only labels listed in [`label-inventory.md`](label-inventory.md).
  A new label appearing here is a breaking change that must update the
  inventory **and** the wrapper mapping in [`repo-layout.md`](repo-layout.md).
- The label mapping table in [`repo-layout.md`](repo-layout.md) must cover
  **every** label in [`label-inventory.md`](label-inventory.md).
- The example `manifest.json` in [`manifest.md`](manifest.md) must include
  enough information for a consumer-wiring or safety-gate consumer to verify
  source identity, platform compatibility, and payload integrity without
  running a network query.

## How the compatibility surface was established

- All Bazel queries against the source GoogleSQL tree were resolved by reading
  upstream `BUILD` files directly (the package's own `hdrs = [...]` and
  `srcs = [...]` lists are the authoritative per-label header surface). No
  `bazel cquery` / `aquery` was needed; no Bazel daemon was warmed.
- The `bazel:status` audit before and after the inventory pass reports
  `(clean)` (modulo the user-managed Docker `emulator_main` / `gateway_main`
  containers on ports 9050 / 9060, which the bazel-process-hygiene rule
  explicitly ignores).
