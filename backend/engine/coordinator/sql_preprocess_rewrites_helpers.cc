#include "backend/engine/coordinator/sql_preprocess_rewrites_helpers.h"

#include <cctype>
#include <cstddef>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

bool IsIdentChar(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool MatchCaseInsensitiveAt(absl::string_view sql,
                            size_t pos,
                            absl::string_view literal) {
  if (pos + literal.size() > sql.size()) return false;
  for (size_t k = 0; k < literal.size(); ++k) {
    if (std::tolower(static_cast<unsigned char>(sql[pos + k])) != literal[k]) {
      return false;
    }
  }
  return true;
}

bool IsWordBoundaryBefore(absl::string_view sql, size_t pos) {
  return pos == 0 || !IsIdentChar(sql[pos - 1]);
}

bool IsWordBoundaryAfter(absl::string_view sql, size_t pos, size_t lit_size) {
  return pos + lit_size >= sql.size() || !IsIdentChar(sql[pos + lit_size]);
}

bool AdvanceSingleQuoted(absl::string_view sql,
                         size_t i,
                         std::string* out,
                         bool* in_single_quoted,
                         size_t* next_i) {
  const char c = sql[i];
  if (c != '\'') return false;
  out->push_back(c);
  if (i + 1 < sql.size() && sql[i + 1] == '\'') {
    out->push_back(sql[++i]);
    *next_i = i;
    return true;
  }
  *in_single_quoted = false;
  *next_i = i;
  return true;
}

bool AdvanceDoubleQuoted(absl::string_view sql,
                         size_t i,
                         std::string* out,
                         bool* in_double_quoted,
                         size_t* next_i) {
  const char c = sql[i];
  if (c != '"') return false;
  out->push_back(c);
  if (i + 1 < sql.size() && sql[i + 1] == '"') {
    out->push_back(sql[++i]);
    *next_i = i;
    return true;
  }
  *in_double_quoted = false;
  *next_i = i;
  return true;
}

bool TryAppendAnonymousStructOffsetRewrite(absl::string_view sql,
                                           size_t i,
                                           std::string* out,
                                           size_t* next_i) {
  const char c = sql[i];
  if (c != '.' || i + 2 >= sql.size() || sql[i + 1] != '_') {
    return false;
  }
  size_t j = i + 2;
  if (std::isdigit(static_cast<unsigned char>(sql[j])) == 0) {
    return false;
  }
  const size_t start = j;
  while (j < sql.size() &&
         std::isdigit(static_cast<unsigned char>(sql[j])) != 0) {
    ++j;
  }
  if (j < sql.size() && IsIdentChar(sql[j])) {
    return false;
  }
  absl::StrAppend(out, "[OFFSET(", sql.substr(start, j - start), ")]");
  *next_i = j - 1;
  return true;
}

bool TryAppendFormatPercentTRewrite(absl::string_view sql,
                                    size_t i,
                                    std::string* out,
                                    size_t* next_i) {
  const char c = sql[i];
  if ((c != 'F' && c != 'f') || i + 6 >= sql.size() ||
      !MatchCaseInsensitiveAt(sql, i, "format(")) {
    return false;
  }
  size_t j = i + 7;
  while (j < sql.size() &&
         std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
    ++j;
  }
  if (j >= sql.size() || sql[j] != '\'') {
    return false;
  }
  ++j;
  if (j + 2 >= sql.size() || sql[j] != '%' || sql[j + 1] != 'T' ||
      sql[j + 2] != '\'') {
    return false;
  }
  j += 3;
  while (j < sql.size() &&
         std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
    ++j;
  }
  if (j >= sql.size() || sql[j] != ',') {
    return false;
  }
  ++j;
  while (j < sql.size() &&
         std::isspace(static_cast<unsigned char>(sql[j])) != 0) {
    ++j;
  }
  const size_t expr_start = j;
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
  if (j <= expr_start) {
    return false;
  }
  out->append("emu_format_t(");
  out->append(sql.substr(expr_start, j - expr_start));
  out->push_back(')');
  *next_i = j;
  return true;
}

bool TryAppendIntegerAliasRewrite(absl::string_view sql,
                                  size_t i,
                                  std::string* out,
                                  size_t* next_i) {
  static constexpr absl::string_view kIntegerAlias = "integer";
  const char c = sql[i];
  if ((c != 'I' && c != 'i') || i + kIntegerAlias.size() > sql.size() ||
      !MatchCaseInsensitiveAt(sql, i, kIntegerAlias) ||
      !IsWordBoundaryBefore(sql, i) ||
      !IsWordBoundaryAfter(sql, i, kIntegerAlias.size())) {
    return false;
  }
  out->append("INT64");
  *next_i = i + kIntegerAlias.size() - 1;
  return true;
}

}  // namespace sql_preprocess_internal
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
