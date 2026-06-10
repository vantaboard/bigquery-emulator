---
name: Parity 13 — GoogleSQL .test corpus lane
overview: Vendor a curated subset of the GoogleSQL compliance .test corpus and run it against the engine via jobs.query as a third conformance lane, so GoogleSQL upgrades and every parity plan above get a semantic regression net.
est_effort: ~1 week
isProject: true
todos:
  - id: corpus-survey
    content: "Survey the upstream GoogleSQL .test corpus format (statements + expected results + feature flags) in the sibling googlesql checkout; pick a starter subset matching landed routes (scalar functions, aggregates, joins, arrays/structs)."
    status: pending
  - id: vendor-runner
    content: "Vendor the subset under conformance/googlesql-corpus/ (or third_party/) with provenance notes; write a Go runner that parses .test files, executes via jobs.query, and compares results with the typed-cell comparison the fixture lane already has."
    status: pending
  - id: triage-and-pin
    content: "First run triage: bucket failures into engine-bug / not-yet-landed-route / corpus-feature-out-of-scope; pin the passing set as the gate (the lane's purpose is 'a test that used to pass now fails', mirroring the third-party skip-matrix discipline)."
    status: pending
  - id: ci-wire
    content: "Wire into CI: extend .github/workflows/googlesql-parity.yml (ROADMAP names it the placeholder for this lane) with a corpus job gating on the pinned passing set; task conformance:googlesql-corpus task entry."
    status: pending
  - id: docs
    content: "Document the lane in conformance/README.md + flip the ROADMAP ⏳ bullet; describe the subset-refresh procedure for GoogleSQL upgrades."
    status: pending
---

# Parity 13 — GoogleSQL .test corpus lane

## Why

ROADMAP's last ⏳ conformance bullet: *"Vendor a subset of the
GoogleSQL `.test` corpus and run it against the engine via `jobs.query`
(catches semantic regressions whenever GoogleSQL is upgraded). The
GoogleSQL parity workflow is the placeholder for this lane today."*

This is infrastructure rather than feature work, but it multiplies the
value of every other parity plan: each landed route gains hundreds of
upstream-authored semantic assertions for free, and GoogleSQL prebuilt
upgrades stop being a leap of faith.

## Key files

- `.github/workflows/googlesql-parity.yml` — existing placeholder workflow
- [`conformance/`](../../conformance/) — fixture runner with typed-cell comparison to reuse
- `taskfiles/` — new `conformance:googlesql-corpus` task
- Sibling `../googlesql/` checkout (`GOOGLESQL_SOURCE=local` flow) — corpus source
- [`third_party/README.md`](../../third_party/README.md) — skip-matrix discipline to mirror

## Steps

1. Survey first: corpus files encode required feature options and
   prepared-table schemas; map what the runner must seed (the fixture
   lane's REST seeding paths cover table setup; `tabledata.insertAll`
   bypasses DML per ENGINE_POLICY).
2. Start narrow — one or two function-family files — and get the
   parse/execute/compare loop solid before widening.
3. Keep vendored files byte-identical to upstream with a provenance
   README (matching the repo's "honest about what is vendored" stance);
   the pinned-passing manifest lives next to the runner, not inside
   the vendored files.
4. CI: non-blocking artifact first run, then gate on the pinned set
   once stable (same maturation path the routing matrix took).

## Verify

```bash
task conformance:googlesql-corpus          # pinned set green
task conformance:run                       # existing lanes unaffected
rtk gh run list --workflow googlesql-parity.yml   # after push
```

## Out of scope

- Running the full corpus (most of it exercises features that are
  `unsupported` by design) — the pinned subset is the deliverable
- Changing the GoogleSQL prebuilt pipeline itself
