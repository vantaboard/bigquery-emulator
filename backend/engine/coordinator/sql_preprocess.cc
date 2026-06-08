#include "backend/engine/coordinator/sql_preprocess.h"

#include <cctype>
#include <string>

#include "absl/strings/str_cat.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

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
        size_t j = i + 6;
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
                i = j - 1;
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

// BigQuery-utils fixtures often use `AS ( (` on separate lines after a
// license block comment. The analyzer rejects that shape when the body
// contains `SELECT ... FROM UNNEST(...)`; collapse to `AS ((` / `));`.
std::string NormalizeCreateFunctionAsParens(absl::string_view sql) {
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
        if (j < sql.size() && sql[j] == '(') {
          size_t k = j + 1;
          while (k < sql.size() &&
                 std::isspace(static_cast<unsigned char>(sql[k])) != 0) {
            ++k;
          }
          if (k < sql.size() && sql[k] == '(') {
            out.append("AS ((");
            i = k;
            continue;
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
          out.append("));");
          i = k;
          continue;
        }
      }
    }
    out.push_back(c);
  }
  return out;
}

}  // namespace

std::string PreprocessSqlForAnalyzer(absl::string_view sql) {
  return NormalizeCreateFunctionAsParens(RewriteFormatTypeLiteral(
      RewriteAnonymousStructFieldAccess(StripBlockComments(sql))));
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
