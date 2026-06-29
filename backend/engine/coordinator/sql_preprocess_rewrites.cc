#include <cctype>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/coordinator/sql_preprocess_internal.h"
#include "backend/engine/coordinator/sql_preprocess_rewrites_helpers.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

namespace {

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
    if (in_single_quoted) {
      size_t next = i;
      if (AdvanceSingleQuoted(sql, i, &out, &in_single_quoted, &next)) {
        i = next;
        continue;
      }
      out.push_back(sql[i]);
      continue;
    }
    if (sql[i] == '\'') {
      in_single_quoted = true;
      out.push_back(sql[i]);
      continue;
    }
    size_t next = i;
    if (TryAppendAnonymousStructOffsetRewrite(sql, i, &out, &next)) {
      i = next;
      continue;
    }
    out.push_back(sql[i]);
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
    if (in_single) {
      size_t next = i;
      if (AdvanceSingleQuoted(sql, i, &out, &in_single, &next)) {
        i = next;
        continue;
      }
      out.push_back(sql[i]);
      continue;
    }
    if (in_double) {
      size_t next = i;
      if (AdvanceDoubleQuoted(sql, i, &out, &in_double, &next)) {
        i = next;
        continue;
      }
      out.push_back(sql[i]);
      continue;
    }
    if (sql[i] == '\'') {
      in_single = true;
      out.push_back(sql[i]);
      continue;
    }
    if (sql[i] == '"') {
      in_double = true;
      out.push_back(sql[i]);
      continue;
    }
    size_t next = i;
    if (TryAppendFormatPercentTRewrite(sql, i, &out, &next)) {
      i = next;
      continue;
    }
    out.push_back(sql[i]);
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
    if (in_single) {
      size_t next = i;
      if (AdvanceSingleQuoted(sql, i, &out, &in_single, &next)) {
        i = next;
        continue;
      }
      out.push_back(sql[i]);
      continue;
    }
    if (in_double) {
      size_t next = i;
      if (AdvanceDoubleQuoted(sql, i, &out, &in_double, &next)) {
        i = next;
        continue;
      }
      out.push_back(sql[i]);
      continue;
    }
    if (sql[i] == '\'') {
      in_single = true;
      out.push_back(sql[i]);
      continue;
    }
    if (sql[i] == '"') {
      in_double = true;
      out.push_back(sql[i]);
      continue;
    }
    size_t next = i;
    if (TryAppendIntegerAliasRewrite(sql, i, &out, &next)) {
      i = next;
      continue;
    }
    out.push_back(sql[i]);
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
  return LowerTableDecorators(RewriteIntegerTypeAlias(
      RewriteStructInt64LiteralCasts(RewriteFormatTypeLiteral(
          RewriteAnonymousStructFieldAccess(StripBlockComments(normalized))))));
}

}  // namespace sql_preprocess_internal
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
