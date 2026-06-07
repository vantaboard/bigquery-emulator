---
name: FixTests 11 — bqutils regexp / unicode / string
overview: Promote the 16 string/regexp/NVP/URL-parsing fixtures - the single largest known_failing theme - by closing Unicode property-class (\p{}) support and the regexp/string builtins the bodies use. cw_regexp_instr_2 already passes and anchors the diff.
depends_on: [fixtests-10-bqutils-range]
est_effort: 1-2 weeks
isProject: true
todos:
  - id: repro-diff
    content: Run the 16 string/regexp fixtures; diff cw_regexp_instr_3 against the passing cw_regexp_instr_2 and group the rest by missing primitive (unicode class, translate, instr, split, NVP, URL).
    status: pending
  - id: unicode-classes
    content: Add Unicode property classes (\p{P}, \p{S}, etc.) in REGEXP_EXTRACT_ALL / regexp matching (RE2 config) for initcap-style fixtures.
    status: pending
  - id: string-builtins
    content: Close gaps in cw_otranslate, cw_initcap, cw_split_part_delimstr_idx, cw_instr4, cw_substrb, cw_find_in_list, cw_comparable_format_varchar_t, NVP (cw_td_nvp, cw_nvp2json4), and URL parsing (url_trim_query).
    status: pending
  - id: regexp-instr
    content: Fix cw_regexp_instr_3 and the cw_disjoint_*_regexp / cw_overlapping_partition_by_regexp family.
    status: pending
  - id: triage
    content: Re-sync + triage; move newly-passing fixtures to passing/; update index count and docs.
    status: pending
---

# FixTests 11 — bqutils regexp / unicode / string

## Why

Sixteen fixtures — the largest single theme — depend on regexp + string builtins. `cw_regexp_instr_2` passes while `_3` fails, so the gaps are specific (often Unicode property classes and a few string functions), not the whole regexp surface.

## Fixtures (community + migration)

`cw_regexp_instr_3`, `cw_disjoint_rangesessionize_regexp`-style and `cw_overlapping_partition_by_regexp` family, `cw_initcap`, `cw_otranslate`, `cw_split_part_delimstr_idx`, `cw_instr4`, `cw_substrb`, `cw_td_nvp`, `cw_nvp2json4`, `cw_find_in_list`, `cw_comparable_format_varchar_t`, `url_trim_query`, `migration/redshift/initcap`, `migration/redshift/translate`. (Confirm the exact 16 via the triage output from plan 01.)

## Likely gaps

- Unicode property classes `\p{P}`, `\p{S}` in `REGEXP_EXTRACT_ALL` / regexp matching (RE2 build/config flags).
- String builtins: `TRANSLATE`/`OTRANSLATE`, `INITCAP`, `SPLIT`/split-part by delimiter+index, `INSTR` variants, `SUBSTR` on bytes, NVP (name-value-pair) parsing, URL query trimming with `SAFE_OFFSET` / `STRING_AGG`.

## Key files

- Regexp + string function impls under [`backend/engine/semantic/functions/`](backend/engine/semantic/functions/) and the RE2 wiring.
- DuckDB transpiler emit for any routed to `duckdb_udf` / `duckdb_native`.

## Steps

1. Diff `cw_regexp_instr_3` vs `_2` (passing); group the remaining 15 by missing primitive.
2. Land Unicode-class support first (unblocks initcap family), then the per-function gaps.
3. Re-sync + triage:

```bash
task emulator:build-engine:bazel
task conformance:bqutils-sync
./scripts/triage_bqutils_fixtures.sh
task conformance:bqutils
```

4. Update [bqutils-00-index.plan.md](bqutils-00-index.plan.md), [`third_party/README.md`](third_party/README.md), `docs/ENGINE_POLICY.md`.

## Out of scope

- BIGNUMERIC (plan 12), bytes width work owned by plan 09 (share `cw_substrb` if needed), geography/XML/migration tail (plan 13).
