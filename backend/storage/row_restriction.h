#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_ROW_RESTRICTION_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_ROW_RESTRICTION_H_

// Row-restriction parser for the Storage Read API.
//
// Plan 39 narrows the BigQuery `ReadOptions.row_restriction` surface
// to a single `<column> = <literal>` predicate. Anything more complex
// (AND, OR, NOT, IN, range, NULL, function calls) is rejected at the
// gRPC boundary with INVALID_ARGUMENT so the gateway can surface the
// BigQuery REST 400 envelope. This intentional shave keeps the
// pushdown wiring small enough to share between the memory and DuckDB
// backends without re-implementing a parser per backend.
//
// The parsed shape (`EqualityPredicate`) is attached to
// `ReadFilter::equality_predicate`; both backends consume the typed
// form rather than re-parsing the raw string, so the parse happens
// exactly once per session and any rejection surfaces before the
// stream opens.

#include <cstdint>
#include <optional>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {

// EqualityPredicate captures `<column> = <literal>` parsed against a
// table schema. The literal is stored both as a typed value (so the
// memory backend can compare cells without re-parsing) and as a raw
// string (so the DuckDB backend can render a CAST-bound SQL literal
// without re-quoting). At most one of the typed slots is populated;
// `kind` discriminates which one is live (kNull is unused today
// because `column = NULL` is invalid in standard SQL — callers must
// use `IS NULL`, which the parser rejects).
struct EqualityPredicate {
  enum class Kind {
    kInt64,
    kBool,
    kString,
  };
  std::string column;
  Kind kind = Kind::kString;
  // Index of `column` in the table's top-level column list. Filled in
  // by `ParseRowRestriction` so the memory backend's row scan does
  // not have to re-look-up the name on every row.
  std::size_t column_index = 0;
  std::int64_t int64_value = 0;
  bool bool_value = false;
  std::string string_value;
};

// Parses `restriction` against `schema` and writes the typed
// predicate into `*out`. Empty `restriction` is OK and leaves `*out`
// untouched (the caller treats that as "no predicate" and serves
// every row). Returns INVALID_ARGUMENT for any unsupported shape;
// the message names the rejected fragment so the user knows which
// part of their restriction tripped the parser.
//
// Supported literals:
//   * INT64        - decimal integer, optional leading sign
//   * BOOL         - case-insensitive `true` / `false`
//   * STRING       - single-quoted, doubled-quote escape (`''` -> `'`)
// Anything else (FLOAT64, NUMERIC, BYTES, DATE, ARRAY, STRUCT, NULL)
// is rejected with a clear message; plan 39 ships with the same three
// literal types BigQuery's `simple_filter` accepts in the public
// Storage Read API.
absl::Status ParseRowRestriction(absl::string_view restriction,
                                 const schema::TableSchema& schema,
                                 EqualityPredicate* out);

}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_ROW_RESTRICTION_H_
