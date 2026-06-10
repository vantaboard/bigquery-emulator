# GoogleSQL `.test` corpus conformance lane

Vendored subset of the upstream GoogleSQL compliance corpus
([`googlesql/compliance/testdata/`](https://github.com/google/googlesql/tree/main/googlesql/compliance/testdata)),
executed through `jobs.query` and compared with the YAML fixture lane's
typed-cell diff engine (`conformance/runner.CompareRows`).

## Provenance

| Field | Value |
|-------|-------|
| Upstream repo | `github.com/google/googlesql` |
| Upstream tag | `2026.1.1` (see `MODULE.bazel`) |
| Vendored path | `corpus/*.test` (byte-identical copies) |
| Survey notes | [`SURVEY.md`](./SURVEY.md) |

Do not edit vendored `.test` files. Refresh by copying from the sibling
checkout:

```bash
cp ../googlesql/googlesql/compliance/testdata/<file>.test conformance/googlesql-corpus/corpus/
```

Re-triage after refresh (see below).

## Layout

- `corpus/` — vendored `.test` files
- `manifest/pinned.json` — pinned-passing case IDs (`file::name`) + triage buckets
- `../googlesqlcorpus/` — Go parser + runner library
- `../cmd/googlesql-corpus/` — CLI

## Commands

```bash
# Pinned gate (CI + local pre-push)
task conformance:googlesql-corpus

# First-run / refresh triage (runs all runnable cases, rewrites manifest)
go run ./conformance/cmd/googlesql-corpus --triage --gate-pinned=false
```

## Pin discipline

Mirrors the third-party skip-matrix model (`third_party/README.md`):

- The gate runs **only** cases listed in `manifest/pinned.json`.
- A CI regression is "a pinned case that used to pass now fails".
- Failures triaged as `engine-bug`, `not-yet-landed-route`, or
  `corpus-feature-out-of-scope` stay out of `pinned` until fixed or
  the route lands.

## Subset refresh procedure (GoogleSQL upgrades)

1. Bump `MODULE.bazel` / prebuilt pin per `docs/dev/googlesql-prebuilt/upgrade-procedure.md`.
2. Copy new/changed `.test` files from the matching upstream tag into `corpus/`.
3. Run `go run ./conformance/cmd/googlesql-corpus --triage --gate-pinned=false`.
4. Review `manifest/pinned.json` diff; commit vendored bytes + manifest together.
5. Confirm `task conformance:googlesql-corpus` is green and `task conformance:run` unchanged.
