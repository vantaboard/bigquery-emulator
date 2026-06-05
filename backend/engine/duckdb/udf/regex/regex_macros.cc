// BigQuery regex polyfill macros.
//
// Each macro installed here pins a specific BigQuery / DuckDB
// regex-function gap. The body documents the gap; the per-macro
// unit test (`regex_macros_test.cc`) exercises both the common
// path AND the BigQuery-specific edge case so a future DuckDB
// regression surfaces as a unit-test failure rather than as silent
// semantic drift.
//
// Family scope (this commit ships the two thin-macro cases):
//
//   * `bq_regexp_contains(value, regex)` -- thin alias for DuckDB
//     `regexp_matches`. BQ uses RE2; DuckDB's `regexp_matches`
//     also uses RE2 (DuckDB v1.5.3 vendors a copy of Google's
//     RE2), so the dialects agree on anchoring, character
//     classes, embedded `(?i)` / `(?s)` / `(?m)` flags, and
//     backreference numbering. The wrapper pins the contract
//     under our own name.
//
//   * `bq_regexp_replace(value, regex, replacement)` -- DuckDB's
//     `regexp_replace` defaults to replacing ONLY the first
//     occurrence; BigQuery replaces ALL non-overlapping
//     occurrences. The wrapper forwards the `'g'` global option
//     so a regression that dropped the `'g'` would surface as
//     stop-at-first-match output.
//
// Out of scope (re-pointed to `docs/ENGINE_POLICY.md`
// in the wrap-up commit): `regexp_extract`, `regexp_extract_all`.
// Both functions diverge from DuckDB on the capture-group
// semantic: BigQuery returns the FIRST capturing group when the
// regex contains one and falls back to the whole match otherwise.
// DuckDB always returns the whole match by default and exposes
// the group via a numeric `group` argument. A thin macro cannot
// introspect the regex pattern to decide which behavior applies,
// so the BQ contract requires either a C++ UDF that parses the
// pattern (out of scope for a thin polyfill) or the semantic
// executor (where the discrimination is a few lines of Go).

#include "absl/status/status.h"
#include "backend/engine/duckdb/udf/internal/run_macro.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

absl::Status RegisterRegex(::duckdb_connection conn) {
  // `bq_regexp_contains(value, regex)` --- BigQuery REGEXP_CONTAINS.
  //
  // BigQuery REGEXP_CONTAINS(value, regex) returns TRUE if `value`
  // contains any non-overlapping match for `regex`. The regex uses
  // RE2 syntax. DuckDB's `regexp_matches(value, regex)` agrees on
  // the contract (both vendor Google's RE2 library, both are
  // contains-style matchers rather than full-string match) so the
  // macro is a thin alias.
  //
  // Edge cases the unit test pins:
  //   * Anchored match. `REGEXP_CONTAINS('abc', '^b') == FALSE`
  //     and `REGEXP_CONTAINS('abc', '^a') == TRUE` -- `^` is the
  //     start-of-string anchor, not match-anywhere.
  //   * Embedded flag. `REGEXP_CONTAINS('ABC', '(?i)abc') == TRUE`
  //     -- the `(?i)` inline flag triggers case-insensitivity at
  //     the RE2 level.
  //   * Case-sensitive default. `REGEXP_CONTAINS('ABC', 'abc') ==
  //     FALSE` (no implicit case folding).
  //   * NULL propagation. Either argument NULL -> NULL.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_regexp_contains(value, regex) AS "
          "regexp_matches(value, regex)");
      !s.ok()) {
    return s;
  }

  // `bq_regexp_replace(value, regex, replacement)` --- BigQuery
  // REGEXP_REPLACE with the implicit "replace ALL" contract.
  //
  // BigQuery REGEXP_REPLACE(value, regex, replacement) replaces
  // every non-overlapping match of `regex` in `value` with
  // `replacement`. Capture groups can be referenced from the
  // replacement via `\1`, `\2`, ... (and `\0` for the full
  // match). DuckDB's `regexp_replace(value, regex, replacement)`
  // defaults to replacing ONLY the first match; passing `'g'` as
  // the fourth argument switches to global mode (every
  // non-overlapping match). Both engines use RE2 for the regex
  // and accept the same `\<n>` backreference syntax in the
  // replacement, so the only gap is the global-vs-first default.
  // The macro forwards `'g'` unconditionally so the BQ "replace
  // all" contract holds at the call site.
  //
  // Edge cases the unit test pins:
  //   * Global replacement. `REGEXP_REPLACE('aaaa', 'a', 'b') ==
  //     'bbbb'`, not `'baaa'` (the BQ contract is "all"; a
  //     regression that dropped the `'g'` flag would surface
  //     here).
  //   * Backreference in replacement. `REGEXP_REPLACE('John
  //     Doe', '(\w+) (\w+)', '\2 \1') == 'Doe John'`.
  //   * NULL propagation. Any argument NULL -> NULL.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_regexp_replace(value, regex, "
          "replacement) AS "
          "regexp_replace(value, regex, replacement, 'g')");
      !s.ok()) {
    return s;
  }

  return absl::OkStatus();
}

}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
