---
name: dataframes GOOGLE_CLOUD_PROJECT default
overview: "Add a one-line `GOOGLE_CLOUD_PROJECT=\"${GOOGLE_CLOUD_PROJECT:-dev}\"` export to the `python-bigquery-dataframes-tests:` task in `taskfiles/thirdparty.yml`, matching the pattern that `python-bigquery-tests` and `java-bigquery-tests` already use. Trivial unblock for the dataframes suite."
todos:
  - id: tp01_edit
    content: "Add the env-var export in the third inline step of python-bigquery-dataframes-tests (around taskfiles/thirdparty.yml:889)."
    status: pending
  - id: tp01_verify
    content: "Run `task thirdparty:python-bigquery-dataframes-tests` and confirm the KeyError on GOOGLE_CLOUD_PROJECT no longer appears."
    status: pending
  - id: tp01_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp01` to `completed` in the same commit."
    status: pending
isProject: false
---

# Thirdparty 01 — dataframes `GOOGLE_CLOUD_PROJECT` default

## Source

- [`fix_thirdparty_failures_cbe91a41.plan.md` Tier 1.1](https://example.invalid)
  (now deleted; preserved here as the audit trail).
- Failing log line: `.logs/thirdparty-20260602-134739.log:4732`
  — `nox > Session py-3.12 raised exception KeyError('GOOGLE_CLOUD_PROJECT')`
  raised from
  `third_party/python-bigquery-dataframes-tests/samples/snippets/noxfile.py:82`.

## Prerequisites

None. This is the lowest-friction unblock in the sequence.

## Scope

`python-bigquery-tests` ([`taskfiles/thirdparty.yml:510`](../../taskfiles/thirdparty.yml))
and `java-bigquery-tests` ([`taskfiles/thirdparty.yml:700`](../../taskfiles/thirdparty.yml))
already default `GOOGLE_CLOUD_PROJECT` to `dev`. The dataframes task does not.
The noxfile fails fast on a missing env var, so adding the export
inline in the recipe is the minimum-surface fix.

## Implementation

Edit [`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) inside
the `python-bigquery-dataframes-tests:` recipe, in the third inline step
(around line 889 in the source log's snapshot). Add:

```yaml
export GOOGLE_CLOUD_PROJECT="${GOOGLE_CLOUD_PROJECT:-dev}"
```

Mirror the inline comment from the `python-bigquery-tests` block so a
future reader sees why the export exists.

## Tests

- `task thirdparty:python-bigquery-dataframes-tests` no longer raises
  `KeyError('GOOGLE_CLOUD_PROJECT')` during the noxfile session setup.
- The suite either runs to completion or fails on a downstream test
  body — the prerequisite-env failure mode is gone.

## Done criteria

- The one-line export is in place inside the recipe.
- The KeyError is gone from a fresh run of the task.
- `thirdparty-00-completion-index.plan.md` todo `tp01` is flipped to
  `completed` in the same commit as the recipe edit.
