# GoogleSQL Prebuilt — Upgrade Rules (Phase 1 — Done Criteria)

This file freezes the rules for how the Phase 1 compatibility surface may
change after Phase 1 lands. It exists because the entire 6-phase rollout
treats Phase 1 artifacts (label inventory, repo layout, manifest schema) as
load-bearing: a silent surface change between phases would let producer and
consumer drift apart with no diagnostic.

## Change taxonomy

Each row identifies a class of change, who is allowed to make it, and what
ripple it produces in later phases. The **Severity** column maps to the
[`auto-commit.mdc`](../../../.cursor/rules/auto-commit.mdc) breaking-change
guidance.

| Change                                                                         | Severity      | Required follow-ups                                                                                                                                                                                                       |
|--------------------------------------------------------------------------------|---------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Bump the **upstream GoogleSQL commit** (same minor module version)             | Non-breaking. | Re-run the producer (Phase 2), republish, update `manifest.json` `googlesql.commit` + `producer.*`. No consumer changes if the wrapper labels and header set are unchanged. Phase 5 parity job must re-run on the new SHA. |
| Bump the **upstream GoogleSQL module version** (`MAJOR.MINOR.PATCH`)           | Breaking.     | All of the above, **plus** bump `manifest.json` `googlesql.module_version` and re-evaluate the patch list. Bump `artifact_version` MAJOR or MINOR. Phase 5's parity gate must run before this artifact can be made the default. |
| Add a new emulator `BUILD.bazel` line `@googlesql//some/new:label`             | Breaking for the artifact contract. | Add the row to [`label-inventory.md`](label-inventory.md) **and** [`repo-layout.md`](repo-layout.md). The next producer run must add the matching wrapper `cc_library` and bump `manifest.json` `compat.labels`. Phase 3 consumer code does not change (labels mirror upstream paths). |
| Remove an emulator `BUILD.bazel` reference to an existing label                | Non-breaking for the artifact, breaking for the contract. | Remove the row from the inventory. The artifact may keep the wrapper for one release cycle as a deprecation cushion; the next minor `artifact_version` bump can drop it. Phase 5's "unused-wrapper" lint flags it.       |
| Add a new `#include "googlesql/..."` line in emulator C++ source under an existing label | Possibly breaking. | The header must already be in the closure listed in [`headers-and-libraries.md`](headers-and-libraries.md). If it is, no surface change. If it isn't, add it to the closure, bump `payload.headers`, and re-run the producer. |
| Change the static-vs-shared decision (e.g. ship `libgooglesql.so` instead of `libgooglesql.a`) | Breaking.     | New manifest field, new wrapper `cc_import` shape, new RPATH considerations. Phase 1 docs must be updated **before** Phase 2 produces a new-shape artifact. Phase 3 wrappers must be updated atomically. Phase 5 parity gate must re-run.    |
| Change the **bundled** third-party set (e.g. start bundling Abseil into `libgooglesql.a`) | Breaking.     | Bump `manifest.json` `bundled_thirdparty_deps` from `[]` to the new list. Phase 3 wrappers must drop the corresponding Bzlmod `deps`. Phase 5 must add a duplicate-symbol guard so the source-mode build doesn't end up with two Abseils. |
| Add a new platform tuple (e.g. `linux/arm64`)                                  | Non-breaking. | Publish a sibling repo (`@googlesql_prebuilt_linux_arm64`). Phase 1 docs gain platform-specific sections. Phase 3's `select()` lights up. `artifact_version` of the existing `linux_amd64` artifact does NOT change.                       |
| Drop a platform tuple                                                          | Breaking.     | Same workflow as adding, but consumers pinned to that platform will fail to build until they migrate. Announce in changelog at least one minor release ahead.                                                                                |
| Bump the **manifest** `schema_version`                                         | Breaking.     | Phase 5's validator changes to match. Phase 3's wrapper-repo loader gates on the new version. Old artifacts continue to validate against the old schema until the producer drops the older format.                                          |
| Add a new optional manifest field                                              | Non-breaking. | Document the field in [`manifest.md`](manifest.md). The validator must treat the field as optional and tolerate older artifacts that omit it.                                                                                                |
| Remove or rename a manifest field                                              | Breaking.     | Bump `schema_version`. Phase 5 validator gains a transitional read path if backwards compatibility is required.                                                                                                                              |
| Repo rename (`@googlesql_prebuilt_linux_amd64` -> something else)              | Breaking.     | Every Phase 3+ consumer's `MODULE.bazel` updates atomically with the producer's published repo name. Do not do this casually; the suffix `linux_amd64` is the platform discriminator, not the version discriminator.                          |

## What "breaking" obligates in commit messages

Any commit that lands a change marked **Breaking** above MUST include one of
the breaking-change footers recognised by `.releaserc.yml` (`BREAKING
CHANGE`, `BREAKING CHANGES`, or `BREAKING`) per
[`auto-commit.mdc`](../../../.cursor/rules/auto-commit.mdc). The body of the
footer must:

1. Name the field / label / file that broke compatibility.
2. State the new contract.
3. Link the relevant Phase 2 / 3 / 4 / 5 phase plan, so the reviewer can see
   that downstream phases will follow.

Example commit body (illustrative — do not commit verbatim):

> ```
> feat(googlesql): bump prebuilt manifest schema_version to "2"
>
> BREAKING CHANGE: manifest.json now requires `toolchain.target_triple`. Consumers
> built against schema_version "1" must update to the Phase 3 loader at commit
> >= <SHA> before this artifact can be consumed.
> ```

## Phase ripple checklist

When a Phase-1 surface change lands, the author is responsible for adding (or
updating) the corresponding follow-up commits in the affected phase plan
files **before** the changed artifact is published. The check is:

1. **Phase 1 docs** (this directory) — updated to describe the new surface.
2. **Phase 2 producer plan** (`googlesql-prebuilt-producer_2a3b4c5d.plan.md`)
   — updated to describe the producer changes needed to emit the new shape.
3. **Phase 3 consume-wiring plan**
   (`googlesql-prebuilt-consume-wiring_3b4c5d6e.plan.md`) — updated if
   wrapper labels / aliases / MODULE.bazel wiring change.
4. **Phase 4 entrypoints plan**
   (`googlesql-prebuilt-entrypoints_4c5d6e7f.plan.md`) — updated if local /
   CI / Docker / release plumbing must change.
5. **Phase 5 safety-gates plan**
   (`googlesql-prebuilt-safety-gates_5d6e7f80.plan.md`) — updated for any
   change to manifest fields, checksum scope, or parity behaviour.
6. **Phase 6 docs / ops plan**
   (`googlesql-prebuilt-docs-ops_6e7f8091.plan.md`) — updated for user-facing
   diagnostics, troubleshooting steps, or version-bump procedures.

A change that does not touch the surface (e.g. compiler version inside a
non-breaking commit window) needs only Phase 1 + Phase 2 updates.

## How to ratify a change against these rules

Workflow when a contributor proposes a Phase-1-surface change:

1. Reclassify the change against the table above. If unsure, ask the
   maintainer who owns the affected phase plan.
2. Land the docs update (this directory) **first**, in its own commit, using
   the conventional-commits scope `docs(googlesql)`.
3. Land the producer change next, scoped `feat(googlesql)` or
   `fix(googlesql)` depending on the situation.
4. Land the consumer wiring last, scoped per the affected package (e.g.
   `feat(bazel)`).

The auto-commit rule does not allow grouping these across phases — each
phase commits separately. Breaking changes still must carry their footer.
