# GoogleSQL Prebuilt — Compatibility Surface (Phase 1)

Phase 1 of the [GoogleSQL prebuilt rollout](../../../README.md) freezes the
**compatibility contract** between the GoogleSQL prebuilt artifact and the rest
of the emulator's Bazel build. Nothing in this directory builds or publishes an
artifact — that work lives in Phase 2 (producer) and Phase 3 (consume wiring).

The contract documented here is the source of truth for the four downstream
phases. Any change that breaks one of these documents is a **breaking change**
for the rollout and must be called out as such in commit messages and CI
release notes.

## Scope

- Platform: `linux/amd64` only.
- Upstream GoogleSQL pin: module name `googlesql`, version `2026.1.1`
  (upstream tag `2026.01.1`, commit `36dd14aa0657ea299725504bc0f938732f58f380`
  — see `MODULE.bazel` in this repo for the gazelle leading-zero workaround).
- Surface: only the `@googlesql//` Bazel labels that the emulator's `BUILD.bazel`
  files reference directly today. Indirect / transitive labels are packaged but
  not exposed.

## Documents in this directory

1. [`label-inventory.md`](label-inventory.md) — Section 1.
   Every `@googlesql//` label the emulator references, with the owning emulator
   package, the public headers each label exposes, and a per-label
   prebuilt-disposition decision.
2. [`headers-and-libraries.md`](headers-and-libraries.md) — Section 2.
   The minimal header include-root layout, the library shape (single combined
   archive vs. per-label `cc_import`), and the static-vs-shared linking
   decision.
3. [`repo-layout.md`](repo-layout.md) — Section 3.
   The on-disk layout of the unpacked external repo
   (`@googlesql_prebuilt_linux_amd64//`), wrapper-target conventions, and the
   full source-label → prebuilt-wrapper mapping.
4. [`manifest.md`](manifest.md) — Section 4.
   Manifest field schema, an example `manifest.json`, the consumer-pinned
   field list, and the rule that checksum mismatches are hard errors.
5. [`upgrade-rules.md`](upgrade-rules.md) — Done criteria.
   How the frozen surface may evolve during a GoogleSQL upgrade and which
   changes cascade as breaking work into later phases.
6. [`rollback.md`](rollback.md) — Phase 5 rollback playbook.
   When and how to repin or revert prebuilt adoption when the parity job,
   conformance lane, or validator surfaces a regression. Lists every
   `FAIL_*` token emitted by
   [`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py)
   with the recommended response.

## What this phase does NOT do

- It does not build or publish any prebuilt artifact (Phase 2).
- It does not change emulator `BUILD.bazel` files or `MODULE.bazel` (Phase 3).
- It does not wire CI, Docker, or release pipelines (Phase 4).
- It does not add manifest verification or source-vs-prebuilt parity checks
  (Phase 5).
- It does not update user/maintainer docs outside this directory (Phase 6).

## Verification (must remain green after every Phase 1 edit)

The bullets below are the Phase 1 plan's verification checklist. Re-run after
every edit to these docs.

- `rg '@googlesql//' /home/brighten-tompkins/Code/bigquery-emulator --glob '*.bazel' --glob 'BUILD' --glob 'BUILD.bazel'`
  must contain only labels listed in [`label-inventory.md`](label-inventory.md).
  A new label appearing here is a breaking change that must update the
  inventory **and** the wrapper mapping in [`repo-layout.md`](repo-layout.md).
- The label mapping table in [`repo-layout.md`](repo-layout.md) must cover
  **every** label in [`label-inventory.md`](label-inventory.md).
- The example `manifest.json` in [`manifest.md`](manifest.md) must include
  enough information for a Phase 3 / Phase 5 consumer to verify source identity,
  platform compatibility, and payload integrity without running a network
  query.

## How Phase 1 was executed

- All Bazel queries against the source GoogleSQL tree were resolved by reading
  upstream `BUILD` files directly (the package's own `hdrs = [...]` and
  `srcs = [...]` lists are the authoritative per-label header surface). No
  `bazel cquery` / `aquery` was needed; no Bazel daemon was warmed.
- The `bazel:status` audit before and after this phase reports `(clean)`
  (modulo the user-managed Docker `emulator_main` / `gateway_main` containers
  on ports 9050 / 9060, which the bazel-process-hygiene rule explicitly
  ignores).
