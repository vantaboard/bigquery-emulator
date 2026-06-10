#include "backend/engine/coordinator/sql_preprocess_internal.h"

#include <cctype>
#include <string>

#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

namespace {

bool IsIdentChar(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

// Removes /* ... */ block comments. License headers in bqutils fixtures
// never embed */ inside the comment body, so a quote-unaware scan is safe
// and avoids mis-parsing (the "License") / "AS IS" text in headers.
std::string StripBlockComments(absl::string_view sql) {
  std::string out;
  out.reserve(sql.size());
  for (size_t i = 0; i < sql.size();) {
    if (i + 1 < sql.size() && sql[i] == '/' && sql[i + 1] == '*') {
      i += 2;
      while (i + 1 < sql.size() && !(sql[i] == '*' && sql[i + 1] == '/')) {
        ++i;
      }
      if (i + 1 < sql.size()) {
        i += 2;
      }
      continue;
    }
    out.push_back(sql[i++]);
  }
  return out;
}

// Rewrites `.<_digits>` struct field access outside of single-quoted
// string literals to `[OFFSET(<digits>)]`.
std::string RewriteAnonymousStructFieldAccess(absl::string_view sql) {
  std::string out;
  out.reserve(sql.size());
  bool in_single_quoted = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (in_single_quoted) {
      out.push_back(c);
      if (c == '\'') {
        if (i + 1 < sql.size() && sql[i + 1] == '\'') {
          out.push_back(sql[++i]);
          continue;
        }
        in_single_quoted = false;
      }
      continue;
    }
    if (c == '\'') {
      in_single_quoted = true;
      out.push_back(c);
      continue;
    }
    if (c == '.' && i + 2 < sql.size() && sql[i + 1] == '_') {
      size_t j = i + 2;
      if (std::isdigit(static_cast<unsigned char>(sql[j])) != 0) {
        size_t start = j;
        while (j < sql.size() &&
               std::isdigit(static_cast<unsigned char>(sql[j])) != 0) {
          ++j;
        }
        if (j == sql.size() || !IsIdentChar(sql[j])) {
          absl::StrAppend(&out, "[OFFSET(", sql.substr(start, j - start), ")]");
          i = j - 1;
          continue;
        }
      }
    }
    out.push_back(c);
  }
  return out;
}

// typeof() UDF bodies call FORMAT('%T', input) inside CASE patterns that
// also use double-quoted LIKE strings. Rewrite FORMAT('%T', expr) to
// emu_format_t(expr) outside of quoted literals.
std::string RewriteFormatTypeLiteral(absl::string_view sql) {
  std::string out;
  out.reserve(sql.size());
  bool in_single = false;
  bool in_double = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (in_single) {
      out.push_back(c);
      if (c == '\'') {
        if (i + 1 < sql.size() && sql[i + 1] == '\'') {
          out.push_back(sql[++i]);
          continue;
        }
        in_single = false;
      }
      continue;
    }
    if (in_double) {
      out.push_back(c);
      if (c == '"') {
        if (i + 1 < sql.size() && sql[i + 1] == '"') {
          out.push_back(sql[++i]);
          continue;
        }
        in_double = false;
      }
      continue;
    }
    if (c == '\'') {
      in_single = true;
      out.push_back(c);
      continue;
    }
    if (c == '"') {
      in_double = true;
      out.push_back(c);
      continue;
    }
    if ((c == 'F' || c == 'f') && i + 6 < sql.size()) {
      const auto match_ci = [&](absl::string_view lit) {
        if (i + lit.size() > sql.size()) return false;
        for (size_t k = 0; k < lit.size(); ++k) {
          if (std::tolower(static_cast<unsigned char>(sql[i + k])) != lit[k]) {
            return false;
          }
        }
        return true;
      };
      if (match_ci("format(")) {
        size_t j = i + 7;
        while (j < sql.size() &&
               std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
          ++j;
        }
        if (j < sql.size() && sql[j] == '\'') {
          ++j;
          if (j + 2 < sql.size() && sql[j] == '%' && sql[j + 1] == 'T' &&
              sql[j + 2] == '\'') {
            j += 3;
            while (j < sql.size() &&
                   std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
              ++j;
            }
            if (j < sql.size() && sql[j] == ',') {
              ++j;
              while (j < sql.size() &&
                     std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
                ++j;
              }
              size_t expr_start = j;
              int depth = 0;
              while (j < sql.size()) {
                const char ch = sql[j];
                if (ch == '(') {
                  ++depth;
                } else if (ch == ')') {
                  if (depth == 0) break;
                  --depth;
                } else if (ch == ',' && depth == 0) {
                  break;
                }
                ++j;
              }
              if (j > expr_start) {
                out.append("emu_format_t(");
                out.append(sql.substr(expr_start, j - expr_start));
                out.push_back(')');
                i = j;
                continue;
              }
            }
          }
        }
      }
    }
    out.push_back(c);
  }
  return out;
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
  bool in_single = false;
  bool in_double = false;
  bool opened_as_double = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (in_single) {
      out.push_back(c);
      if (c == '\'') {
        if (i + 1 < sql.size() && sql[i + 1] == '\'') {
          out.push_back(sql[++i]);
          continue;
        }
        in_single = false;
      }
      continue;
    }
    if (in_double) {
      out.push_back(c);
      if (c == '"') {
        if (i + 1 < sql.size() && sql[i + 1] == '"') {
          out.push_back(sql[++i]);
          continue;
        }
        in_double = false;
      }
      continue;
    }
    if (c == '\'') {
      in_single = true;
      out.push_back(c);
      continue;
    }
    if (c == '"') {
      in_double = true;
      out.push_back(c);
      continue;
    }
    if (out.empty() && c == '(') {
      size_t j = i + 1;
      bool had_ws_between = false;
      while (j < sql.size() &&
             std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
        had_ws_between = true;
        ++j;
      }
      if (had_ws_between && j < sql.size() && sql[j] == '(') {
        size_t body = j + 1;
        while (body < sql.size() &&
               std::isspace(static_cast<unsigned char>(sql[body])) != 0) {
          ++body;
        }
        if (IsSubqueryBodyStart(sql, body)) {
          AppendOpenedSubqueryParens(&out, sql, body);
          opened_as_double = true;
          i = j;
          continue;
        }
      }
    }
    if ((c == 'A' || c == 'a') && i + 1 < sql.size()) {
      const auto match_ci = [&](absl::string_view lit) {
        if (i + lit.size() > sql.size()) return false;
        for (size_t k = 0; k < lit.size(); ++k) {
          if (std::tolower(static_cast<unsigned char>(sql[i + k])) != lit[k]) {
            return false;
          }
        }
        return true;
      };
      if (match_ci("as")) {
        size_t j = i + 2;
        while (j < sql.size() &&
               std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
          ++j;
        }
        if (j < sql.size() && sql[j] == '(' &&
            !(j + 1 < sql.size() && sql[j + 1] == '(')) {
          size_t k = j + 1;
          while (k < sql.size() &&
                 std::isspace(static_cast<unsigned char>(sql[k])) != 0) {
            ++k;
          }
          if (k < sql.size() && sql[k] == '(') {
            size_t body = k + 1;
            while (body < sql.size() &&
                   std::isspace(static_cast<unsigned char>(sql[body])) != 0) {
              ++body;
            }
            if (IsSubqueryBodyStart(sql, body)) {
              out.append("AS ");
              AppendOpenedSubqueryParens(&out, sql, body);
              opened_as_double = true;
              i = k;
              continue;
            }
          }
        }
      }
    }
    if (c == ')') {
      size_t j = i + 1;
      while (j < sql.size() &&
             std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
        ++j;
      }
      if (j < sql.size() && sql[j] == ')') {
        size_t k = j + 1;
        while (k < sql.size() &&
               std::isspace(static_cast<unsigned char>(sql[k])) != 0) {
          ++k;
        }
        if (k < sql.size() && sql[k] == ';') {
          if (opened_as_double) {
            out.append("));");
            opened_as_double = false;
            i = k + 1;
            continue;
          }
        }
      }
      if (opened_as_double && j < sql.size() && sql[j] == ';') {
        out.append("));");
        opened_as_double = false;
        i = j + 1;
        continue;
      }
    }
    out.push_back(c);
  }
  return out;
}

// BigQuery-utils fixtures use INTEGER as an alias for INT64 in CAST and
// STRUCT literals; GoogleSQL in this repo only recognizes INT64.
std::string RewriteIntegerTypeAlias(absl::string_view sql) {
  std::string out;
  out.reserve(sql.size());
  bool in_single = false;
  bool in_double = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (in_single) {
      out.push_back(c);
      if (c == '\'') {
        if (i + 1 < sql.size() && sql[i + 1] == '\'') {
          out.push_back(sql[++i]);
          continue;
        }
        in_single = false;
      }
      continue;
    }
    if (in_double) {
      out.push_back(c);
      if (c == '"') {
        if (i + 1 < sql.size() && sql[i + 1] == '"') {
          out.push_back(sql[++i]);
          continue;
        }
        in_double = false;
      }
      continue;
    }
    if (c == '\'') {
      in_single = true;
      out.push_back(c);
      continue;
    }
    if (c == '"') {
      in_double = true;
      out.push_back(c);
      continue;
    }
    if ((c == 'I' || c == 'i') && i + 7 <= sql.size()) {
      const auto match_ci = [&](absl::string_view lit) {
        if (i + lit.size() > sql.size()) return false;
        for (size_t k = 0; k < lit.size(); ++k) {
          if (std::tolower(static_cast<unsigned char>(sql[i + k])) != lit[k]) {
            return false;
          }
        }
        const bool before_ok = i == 0 || !IsIdentChar(sql[i - 1]);
        const bool after_ok =
            i + lit.size() >= sql.size() || !IsIdentChar(sql[i + lit.size()]);
        return before_ok && after_ok;
      };
      static constexpr absl::string_view kIntegerAlias = "integer";
      if (match_ci(kIntegerAlias)) {
        out.append("INT64");
        // for-loop ++i must land on the first char after the matched alias.
        i += kIntegerAlias.size() - 1;
        continue;
      }
    }
    out.push_back(c);
  }
  return out;
}

// STRUCT(..., CAST(21 AS INT64) AS field, ...) is rejected by the parser
// (nested AS). BigQuery-utils expected rows use INTEGER literals in STRUCT.
std::string RewriteStructInt64LiteralCasts(absl::string_view sql) {
  std::string out(sql);
  const std::string prefix = "CAST(";
  size_t pos = 0;
  while ((pos = out.find(prefix, pos)) != std::string::npos) {
    size_t lit_start = pos + prefix.size();
    size_t lit_end = lit_start;
    while (lit_end < out.size()) {
      const char ch = out[lit_end];
      if (std::isdigit(static_cast<unsigned char>(ch)) || ch == '-' ||
          ch == '+') {
        ++lit_end;
        continue;
      }
      break;
    }
    if (lit_end == lit_start) {
      ++pos;
      continue;
    }
    const std::string lit = out.substr(lit_start, lit_end - lit_start);
    bool rewritten = false;
    for (absl::string_view type_suffix :
         {absl::string_view("INT64"), absl::string_view("INTEGER")}) {
      const std::string pattern =
          absl::StrCat(prefix, lit, " AS ", type_suffix, ") AS ");
      if (out.compare(pos, pattern.size(), pattern) == 0) {
        out.replace(pos, pattern.size(), absl::StrCat(lit, " AS "));
        pos += lit.size() + 4;
        rewritten = true;
        break;
      }
    }
    if (rewritten) {
      continue;
    }
    ++pos;
  }
  return out;
}

}  // namespace

std::string PreprocessFunctionBodyBase(absl::string_view sql) {
  std::string normalized;
  normalized.reserve(sql.size());
  for (char c : sql) {
    if (c == '\t') {
      normalized.push_back(' ');
    } else {
      normalized.push_back(c);
    }
  }
  // Rewrite STRUCT literal casts before INTEGER→INT64 alias so patterns still
  // match `CAST(lit AS INTEGER) AS field` from bigquery-utils fixtures.
  return RewriteIntegerTypeAlias(
      RewriteStructInt64LiteralCasts(RewriteFormatTypeLiteral(
          RewriteAnonymousStructFieldAccess(StripBlockComments(normalized)))));
}

std::string NormalizeCreateFunctionAsParens(absl::string_view sql) {
  return NormalizeCreateFunctionAsParensImpl(sql);
}

}  // namespace sql_preprocess_internal
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
