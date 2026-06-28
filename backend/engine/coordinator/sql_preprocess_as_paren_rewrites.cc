#include <cctype>
#include <string>

#include "absl/strings/string_view.h"
#include "backend/engine/coordinator/sql_preprocess_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

namespace {

bool IsIdentChar(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool MatchKeywordAt(absl::string_view sql, size_t pos, absl::string_view lit) {
  if (pos + lit.size() > sql.size()) return false;
  for (size_t k = 0; k < lit.size(); ++k) {
    if (std::tolower(static_cast<unsigned char>(sql[pos + k])) != lit[k]) {
      return false;
    }
  }
  if (pos + lit.size() < sql.size() && IsIdentChar(sql[pos + lit.size()])) {
    return false;
  }
  return true;
}

bool IsSubqueryBodyStart(absl::string_view sql, size_t pos) {
  return MatchKeywordAt(sql, pos, "select") || MatchKeywordAt(sql, pos, "with");
}

// Collapse `AS ( (WITH` to `AS (( WITH` and `AS ( (SELECT` to `AS ((SELECT`.
void AppendOpenedSubqueryParens(std::string* out,
                                absl::string_view sql,
                                size_t body) {
  if (MatchKeywordAt(sql, body, "with")) {
    out->append("(( ");
  } else {
    out->append("((");
  }
}

size_t SkipWhitespace(absl::string_view sql, size_t pos) {
  while (pos < sql.size() &&
         std::isspace(static_cast<unsigned char>(sql[pos])) != 0) {
    ++pos;
  }
  return pos;
}

bool MatchCiPrefix(absl::string_view sql, size_t pos, absl::string_view lit) {
  if (pos + lit.size() > sql.size()) return false;
  for (size_t k = 0; k < lit.size(); ++k) {
    if (std::tolower(static_cast<unsigned char>(sql[pos + k])) != lit[k]) {
      return false;
    }
  }
  return true;
}

struct AsParenRewriteState {
  bool in_single = false;
  bool in_double = false;
  bool opened_as_double = false;
};

bool HandleQuotedLiteralChar(char c,
                             size_t& i,
                             absl::string_view sql,
                             AsParenRewriteState& state,
                             std::string& out) {
  if (state.in_single) {
    out.push_back(c);
    if (c == '\'') {
      if (i + 1 < sql.size() && sql[i + 1] == '\'') {
        out.push_back(sql[++i]);
      } else {
        state.in_single = false;
      }
    }
    return true;
  }
  if (state.in_double) {
    out.push_back(c);
    if (c == '"') {
      if (i + 1 < sql.size() && sql[i + 1] == '"') {
        out.push_back(sql[++i]);
      } else {
        state.in_double = false;
      }
    }
    return true;
  }
  if (c == '\'') {
    state.in_single = true;
    out.push_back(c);
    return true;
  }
  if (c == '"') {
    state.in_double = true;
    out.push_back(c);
    return true;
  }
  return false;
}

bool TryRewriteLeadingOpenParen(size_t i,
                                absl::string_view sql,
                                std::string& out,
                                AsParenRewriteState& state,
                                size_t& next_i) {
  if (!out.empty() || sql[i] != '(') return false;
  size_t j = i + 1;
  bool had_ws_between = false;
  while (j < sql.size() &&
         std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
    had_ws_between = true;
    ++j;
  }
  if (!had_ws_between || j >= sql.size() || sql[j] != '(') return false;
  const size_t body = SkipWhitespace(sql, j + 1);
  if (!IsSubqueryBodyStart(sql, body)) return false;
  AppendOpenedSubqueryParens(&out, sql, body);
  state.opened_as_double = true;
  next_i = j;
  return true;
}

bool TryRewriteAsOpenParen(size_t i,
                           absl::string_view sql,
                           std::string& out,
                           AsParenRewriteState& state,
                           size_t& next_i) {
  if (!MatchCiPrefix(sql, i, "as")) return false;
  size_t j = SkipWhitespace(sql, i + 2);
  if (j >= sql.size() || sql[j] != '(' ||
      (j + 1 < sql.size() && sql[j + 1] == '(')) {
    return false;
  }
  const size_t k = SkipWhitespace(sql, j + 1);
  if (k >= sql.size() || sql[k] != '(') return false;
  const size_t body = SkipWhitespace(sql, k + 1);
  if (!IsSubqueryBodyStart(sql, body)) return false;
  out.append("AS ");
  AppendOpenedSubqueryParens(&out, sql, body);
  state.opened_as_double = true;
  next_i = k;
  return true;
}

bool TryRewriteCloseDoubleParen(size_t i,
                                absl::string_view sql,
                                std::string& out,
                                AsParenRewriteState& state,
                                size_t& next_i) {
  if (sql[i] != ')') return false;
  const size_t j = SkipWhitespace(sql, i + 1);
  if (j < sql.size() && sql[j] == ')') {
    const size_t k = SkipWhitespace(sql, j + 1);
    if (k < sql.size() && sql[k] == ';' && state.opened_as_double) {
      out.append("));");
      state.opened_as_double = false;
      next_i = k + 1;
      return true;
    }
  }
  if (state.opened_as_double && j < sql.size() && sql[j] == ';') {
    out.append("));");
    state.opened_as_double = false;
    next_i = j + 1;
    return true;
  }
  return false;
}

// BigQuery-utils fixtures often use nested parens on separate lines:
//   CREATE FUNCTION ... AS ( ( SELECT ... ) );
// or, in templated bodies extracted by the analyzer:
//   ( ( SELECT ... ) );
// Collapse to `AS ((` / `((` and `));` closers. Only rewrite subquery
// bodies starting with `SELECT` / `WITH`, not scalar expr groups like
// `(bits & (1 << index))`.
std::string NormalizeCreateFunctionAsParensImpl(absl::string_view sql) {
  std::string out;
  out.reserve(sql.size());
  AsParenRewriteState state;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (HandleQuotedLiteralChar(c, i, sql, state, out)) {
      continue;
    }
    size_t next_i = i;
    if (TryRewriteLeadingOpenParen(i, sql, out, state, next_i)) {
      i = next_i;
      continue;
    }
    if ((c == 'A' || c == 'a') && i + 1 < sql.size() &&
        TryRewriteAsOpenParen(i, sql, out, state, next_i)) {
      i = next_i;
      continue;
    }
    if (TryRewriteCloseDoubleParen(i, sql, out, state, next_i)) {
      i = next_i;
      continue;
    }
    out.push_back(c);
  }
  return out;
}

}  // namespace

std::string NormalizeCreateFunctionAsParens(absl::string_view sql) {
  return NormalizeCreateFunctionAsParensImpl(sql);
}

}  // namespace sql_preprocess_internal
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
