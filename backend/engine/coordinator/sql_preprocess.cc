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

}  // namespace

std::string PreprocessSqlForAnalyzer(absl::string_view sql) {
  return RewriteAnonymousStructFieldAccess(sql);
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
