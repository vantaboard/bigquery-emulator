#include "backend/storage/row_restriction.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {

namespace {

// Strips ASCII whitespace from both ends of `s`. We deliberately do
// not pull in absl::StripAsciiWhitespace because we need string_view
// semantics and want a stable predictable result without allocations.
absl::string_view Trim(absl::string_view s) {
  while (!s.empty() && absl::ascii_isspace(s.front()))
    s.remove_prefix(1);
  while (!s.empty() && absl::ascii_isspace(s.back()))
    s.remove_suffix(1);
  return s;
}

// Splits `s` on the first '=' that does not sit inside a single-quote
// string literal. Returns false (and leaves `lhs`/`rhs` untouched) if
// no such '=' exists or if the '=' is at the start / end. The split
// honors the SQL escape sequence `''` so `name = 'O''Reilly'` parses
// as `name` / `'O''Reilly'`.
bool SplitOnTopLevelEquals(absl::string_view s,
                           absl::string_view* lhs,
                           absl::string_view* rhs) {
  bool in_string = false;
  for (std::size_t i = 0; i < s.size(); ++i) {
    const char c = s[i];
    if (in_string) {
      if (c == '\'') {
        if (i + 1 < s.size() && s[i + 1] == '\'') {
          ++i;
          continue;
        }
        in_string = false;
      }
      continue;
    }
    if (c == '\'') {
      in_string = true;
      continue;
    }
    if (c == '=') {
      if (i == 0 || i + 1 == s.size()) return false;
      *lhs = s.substr(0, i);
      *rhs = s.substr(i + 1);
      return true;
    }
  }
  return false;
}

// IsBareIdentifierChar accepts the subset of column-name characters
// the parser is willing to read unquoted. BigQuery permits hyphens
// in identifiers but the row_restriction shape is small and we keep
// to the ASCII letter / digit / underscore set so we don't have to
// hand-roll backtick-quoted identifier parsing for the v1 surface.
// A column with a hyphen or other punctuation is reachable via a
// backtick-quoted form which we accept below.
bool IsBareIdentifierChar(char c) {
  return absl::ascii_isalnum(c) || c == '_';
}

// ExtractIdentifier reads a column name from `s` and returns the
// trimmed name plus a status indicating whether the remaining
// characters are all whitespace. Two shapes are accepted:
//
//   * Backtick-quoted: ``\`my-col\` =`` — anything inside the
//     backticks is taken verbatim. Embedded backticks are not
//     supported (BigQuery doubles them as `\`\``; rare enough that
//     plan 39 punts on it).
//   * Bare:           `my_col =` — limited to
//     `[A-Za-z_][A-Za-z0-9_]*` (a leading digit would be ambiguous
//     with an INT64 literal on the LHS, which the parser also
//     rejects).
absl::Status ExtractIdentifier(absl::string_view s, std::string* name) {
  s = Trim(s);
  if (s.empty()) {
    return absl::InvalidArgumentError(
        "row_restriction: column name is empty (expected `col = literal`)");
  }
  if (s.front() == '`') {
    const auto close = s.find('`', 1);
    if (close == absl::string_view::npos) {
      return absl::InvalidArgumentError(
          "row_restriction: unterminated backtick-quoted column name");
    }
    *name = std::string(s.substr(1, close - 1));
    const absl::string_view rest = Trim(s.substr(close + 1));
    if (!rest.empty()) {
      return absl::InvalidArgumentError(absl::StrCat(
          "row_restriction: unexpected trailing input after column name: ",
          rest));
    }
    return absl::OkStatus();
  }
  if (!absl::ascii_isalpha(s.front()) && s.front() != '_') {
    return absl::InvalidArgumentError(
        absl::StrCat("row_restriction: column name must start with a letter or "
                     "underscore (got: ",
                     s,
                     ")"));
  }
  std::size_t i = 0;
  while (i < s.size() && IsBareIdentifierChar(s[i]))
    ++i;
  *name = std::string(s.substr(0, i));
  const absl::string_view rest = Trim(s.substr(i));
  if (!rest.empty()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "row_restriction: unexpected trailing input after column name: ",
        rest));
  }
  return absl::OkStatus();
}

// Parses a single-quoted string literal, honoring the SQL `''` escape
// for an embedded apostrophe. Rejects anything that does not look
// like a complete `'...'` token.
absl::Status ParseStringLiteral(absl::string_view s, std::string* out) {
  if (s.size() < 2 || s.front() != '\'' || s.back() != '\'') {
    return absl::InvalidArgumentError(absl::StrCat(
        "row_restriction: malformed string literal (expected '...'): ", s));
  }
  std::string acc;
  acc.reserve(s.size());
  for (std::size_t i = 1; i + 1 < s.size(); ++i) {
    const char c = s[i];
    if (c == '\'') {
      if (i + 2 < s.size() && s[i + 1] == '\'') {
        acc.push_back('\'');
        ++i;
        continue;
      }
      return absl::InvalidArgumentError(
          "row_restriction: unescaped apostrophe inside string literal");
    }
    acc.push_back(c);
  }
  *out = std::move(acc);
  return absl::OkStatus();
}

// Parses a bool literal (case-insensitive `true` / `false`).
absl::Status ParseBoolLiteral(absl::string_view s, bool* out) {
  if (absl::EqualsIgnoreCase(s, "true")) {
    *out = true;
    return absl::OkStatus();
  }
  if (absl::EqualsIgnoreCase(s, "false")) {
    *out = false;
    return absl::OkStatus();
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "row_restriction: expected BOOL literal (true / false), got: ", s));
}

// Parses an INT64 literal. Accepts a leading sign; rejects anything
// containing a decimal point or exponent because plan 39 limits the
// FLOAT64 axis (BigQuery's `simple_filter` does not support equality
// on FLOAT64 anyway — the wire shape is lossy enough that "=" rarely
// does what the caller meant).
absl::Status ParseInt64Literal(absl::string_view s, std::int64_t* out) {
  for (char c : s) {
    if (c == '.' || c == 'e' || c == 'E') {
      return absl::InvalidArgumentError(absl::StrCat(
          "row_restriction: FLOAT64 literals are not supported by the "
          "row_restriction parser (got: ",
          s,
          ")"));
    }
  }
  if (!absl::SimpleAtoi(s, out)) {
    return absl::InvalidArgumentError(
        absl::StrCat("row_restriction: expected INT64 literal, got: ", s));
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status ParseRowRestriction(absl::string_view restriction,
                                 const schema::TableSchema& schema,
                                 EqualityPredicate* out) {
  if (out == nullptr) {
    return absl::InternalError(
        "row_restriction: out parameter must be non-null");
  }
  const absl::string_view trimmed = Trim(restriction);
  if (trimmed.empty()) {
    return absl::OkStatus();
  }

  // Bail early on connectives so the error message names the
  // unsupported feature instead of the parser-internal step that
  // happened to trip on it. We look for the keywords surrounded by
  // whitespace so a column named `andrew` does not match. We also
  // reject the SQL relational operators we do not implement here.
  static constexpr absl::string_view kBanned[] = {
      " AND ",
      " and ",
      " OR ",
      " or ",
      " NOT ",
      " not ",
      " IN ",
      " in ",
      " IS ",
      " is ",
      " LIKE ",
      " like ",
      " BETWEEN ",
      " between ",
  };
  for (absl::string_view kw : kBanned) {
    if (absl::StrContains(trimmed, kw)) {
      return absl::InvalidArgumentError(absl::StrCat(
          "row_restriction: only `<column> = <literal>` is supported in "
          "this emulator profile; got: ",
          restriction));
    }
  }
  // The same intent for the operator forms (`<`, `>`, `<=`, `>=`,
  // `!=`, `<>`). Anything but a single `=` is a no-go.
  static constexpr absl::string_view kBannedOps[] = {
      "<=",
      ">=",
      "!=",
      "<>",
      "<",
      ">",
  };
  for (absl::string_view op : kBannedOps) {
    if (absl::StrContains(trimmed, op)) {
      return absl::InvalidArgumentError(absl::StrCat(
          "row_restriction: only `<column> = <literal>` is supported in "
          "this emulator profile; got: ",
          restriction));
    }
  }

  absl::string_view lhs;
  absl::string_view rhs;
  if (!SplitOnTopLevelEquals(trimmed, &lhs, &rhs)) {
    return absl::InvalidArgumentError(absl::StrCat(
        "row_restriction: missing `=` (expected `<column> = <literal>`): ",
        restriction));
  }

  std::string column_name;
  if (auto s = ExtractIdentifier(lhs, &column_name); !s.ok()) return s;

  std::size_t column_index = 0;
  const schema::ColumnSchema* column = nullptr;
  for (std::size_t i = 0; i < schema.columns.size(); ++i) {
    if (schema.columns[i].name == column_name) {
      column_index = i;
      column = &schema.columns[i];
      break;
    }
  }
  if (column == nullptr) {
    return absl::InvalidArgumentError(
        absl::StrCat("row_restriction: unknown column `",
                     column_name,
                     "` (table has no top-level column with that name)"));
  }
  if (column->mode == schema::ColumnMode::kRepeated) {
    return absl::InvalidArgumentError(
        absl::StrCat("row_restriction: equality on REPEATED column `",
                     column_name,
                     "` is not supported in this emulator profile"));
  }
  if (column->type == schema::ColumnType::kArray ||
      column->type == schema::ColumnType::kStruct) {
    return absl::InvalidArgumentError(
        absl::StrCat("row_restriction: equality on ARRAY/STRUCT column `",
                     column_name,
                     "` is not supported in this emulator profile"));
  }

  const absl::string_view lit = Trim(rhs);
  if (lit.empty()) {
    return absl::InvalidArgumentError(
        absl::StrCat("row_restriction: missing literal after `=` (expected "
                     "`<column> = <literal>`): ",
                     restriction));
  }

  EqualityPredicate pred;
  pred.column = column_name;
  pred.column_index = column_index;

  // Lower the literal according to the column type so the wire shape
  // round-trips through the same accessor the storage backends use
  // when materializing the cell.
  switch (column->type) {
    case schema::ColumnType::kInt64: {
      pred.kind = EqualityPredicate::Kind::kInt64;
      if (auto s = ParseInt64Literal(lit, &pred.int64_value); !s.ok()) {
        return s;
      }
      break;
    }
    case schema::ColumnType::kBool: {
      pred.kind = EqualityPredicate::Kind::kBool;
      if (auto s = ParseBoolLiteral(lit, &pred.bool_value); !s.ok()) {
        return s;
      }
      break;
    }
    case schema::ColumnType::kString: {
      pred.kind = EqualityPredicate::Kind::kString;
      if (auto s = ParseStringLiteral(lit, &pred.string_value); !s.ok()) {
        return s;
      }
      break;
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "row_restriction: column `",
          column_name,
          "` has type ",
          schema::ColumnTypeName(column->type),
          "; only INT64, BOOL, and STRING columns are supported by the "
          "row_restriction parser in this emulator profile"));
  }

  *out = std::move(pred);
  return absl::OkStatus();
}

}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
