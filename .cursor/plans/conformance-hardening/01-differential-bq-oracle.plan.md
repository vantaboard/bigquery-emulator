# 01 — Differential testing lane against production BigQuery (recorded oracle)

- **Series:** conformance-hardening (plans 01–08). Run sequentially; this is the
  highest-leverage plan and lands the mechanism the later plans reuse.
- **Sequencing:** first. No prerequisites. Plans 04, 06, 07 build on the
  recorded-oracle format and the typed-cell comparator wiring introduced here.
- **Priority:** P1.

## Why this exists (origin)

A real user (recidiviz-fork migrant) reported a stream of divergences over email
that the in-repo suites did not catch: `UNION DISTINCT` unimplemented, a
TIMESTAMP query parameter rejected because it lacked a `Z`, a CTE referenced from
a subquery failing, and a transpiler column-binding loss on an anti-join over a
deduped view. The common thread: **nothing automatically compares emulator
output against what production BigQuery actually returns.** The repo's
`bq`-validation step is *manual* and only invoked "when a fixture misbehaves"
(`.cursor/rules/conformance-bq-validation.mdc`). This plan promotes that into an
automated differential lane with a recorded oracle so feature gaps and semantic
divergences surface in CI instead of in a customer's inbox.

## Current state (grounded)

- `conformance/cmd/runner/main.go` + `conformance/runner/` already drive the
  emulator over REST and diff rows/errors with a **typed-cell comparator**
  (`conformance/README.md` "Typed cell comparison" table; `diff_scalars.go`).
  Expected values are pinned **by hand** in each YAML `expected:` block.
- `conformance/fixtures/core_usage/` fixtures require production-validated
  expectations and refuse `--update-baselines` rewrites
  (`conformance/fixtures/core_usage/README.md`, `verified_production: true`).
- There is **no** harness that (a) records prod BigQuery output for an arbitrary
  query corpus and (b) replays the same corpus against the emulator and diffs.

## Goal / done-criteria

1. A new lane `task conformance:differential` that, given a **corpus** of
   `(setup, query)` cases and a **recorded oracle** (rows/schema/error captured
   once from production BigQuery), runs each case against the local emulator and
   diffs using the existing typed-cell comparator. Exit non-zero on any
   divergence.
2. A recorder `task conformance:differential-record` that, when GCP creds +
   `BIGQUERY_DIFFERENTIAL_PROJECT` are present, executes the corpus on real
   BigQuery via the `bq` CLI / Go client and writes oracle files. Without creds
   it is a no-op that prints how to record.
3. Oracle files are committed (so CI needs no GCP access) and carry provenance:
   the prod `jobReference`, the capture timestamp, and the BigQuery
   `schema.fields`.
4. The lane classifies each divergence as one of: `feature_gap` (emulator
   returns Unimplemented), `semantic_divergence` (both succeed, rows differ),
   `error_divergence` (one errors, one succeeds), `crash` (engine aborts — hand
   off to plan 03).
5. Seeded with the four email-thread queries (UNION DISTINCT CTE rollup, naive
   TIMESTAMP param dashboard, CTE-in-subquery, orphan-orders anti-join over a
   QUALIFY view) plus 10–20 everyday shapes. CI runs the committed-oracle replay;
   recording stays manual/opt-in.

## Implementation steps

### Step 1 — Corpus + oracle format
- New dir `conformance/differential/corpus/` — one YAML per case reusing the
  existing fixture `setup:`/`query:` schema (so the loader is shared), plus an
  `oracle_ref:` pointer.
- New dir `conformance/differential/oracle/` — one JSON per case:
  `{ "captured_at", "project", "job_id", "schema": [...], "rows": [...] }` or
  `{ "error": { "code", "message" } }`. Mirror the wire shape the runner already
  parses from `QueryResponse` so the comparator is reused verbatim.

### Step 2 — Recorder (`conformance/cmd/differential-record`)
- Reuse the Go BigQuery client wiring from
  `third_party/golang-bigquery-tests/.../bqopts` but pointed at **real**
  BigQuery (ADC, no emulator endpoint).
- For each corpus case: create ephemeral dataset, apply `setup:`, run `query:`,
  capture schema+rows (or the error envelope), tear down, write the oracle JSON.
- Gate behind `BIGQUERY_DIFFERENTIAL_PROJECT`; print a clear skip message when
  unset. Honor `.cursor/rules/conformance-bq-validation.mdc`.

### Step 3 — Replay runner (`conformance/cmd/differential` or a `--mode=oracle`
flag on the existing runner)
- Spawn `./bin/emulator_main` per case (reuse the runner's process lifecycle and
  signal cleanup), apply `setup:`, run `query:`, diff actual vs the oracle JSON
  with the existing typed-cell comparator (`conformance/runner/diff_scalars.go`).
- Emit the divergence classification (Step 4 of done-criteria) in both text and
  JSON output modes.

### Step 4 — Task + CI wiring
- Add `conformance:differential` and `conformance:differential-record` to
  `taskfiles/conformance.yml`.
- Add a CI job in `.github/workflows/conformance.yml` (after `build-engine`,
  mirroring the `bqutils` job) that runs the committed-oracle replay only.
- Document the lane in `conformance/README.md` (new "Differential lane" section)
  and link it from `third_party/README.md`'s lane table.

### Step 5 — Seed the corpus
Add cases for the four reported divergences and ~15 everyday shapes (joins,
window+QUALIFY, set ops, CTEs, scalar subqueries, JSON_EXTRACT_SCALAR, COUNTIF,
TIMESTAMP_SUB). Record their oracles (or, if no GCP access, pin expectations from
`bq` output by hand per the validation rule and mark them `oracle_source: bq-cli`).

## Tests
- Unit: oracle loader + classifier (golden divergence cases: feature gap,
  semantic, error, match).
- A self-test case whose oracle matches emulator output (PASS) and one
  intentionally mismatched (FAIL) to prove the lane fails loudly.
- `go test ./conformance/...` stays green; lane is additive.

## Process hygiene (repo rules)
Building/running the engine here is a **heavy command**: follow
`.cursor/rules/process-hygiene.mdc` and `.cursor/rules/bazel-process-hygiene.mdc`
— pre-spawn audit, single bazel invocation, and the cleanup block
(`task bazel:shutdown` + emulator/runner kill) after any engine run.

## Out of scope
- Live per-PR calls to real BigQuery (cost/flakiness). CI replays committed
  oracles only; recording is manual.
- Non-determinism handling beyond the existing `schema_only`/`unordered` modes —
  reuse them for timestamp/generated-id columns.

## Touch list
`conformance/differential/corpus/*`, `conformance/differential/oracle/*`,
`conformance/cmd/differential-record/main.go`,
`conformance/cmd/differential/main.go` (or `runner` `--mode=oracle`),
`conformance/runner/` (share loader + comparator), `taskfiles/conformance.yml`,
`.github/workflows/conformance.yml`, `conformance/README.md`,
`third_party/README.md`.
