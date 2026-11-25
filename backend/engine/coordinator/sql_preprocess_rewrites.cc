#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/coordinator/sql_preprocess_internal.h"
#include "backend/engine/coordinator/sql_preprocess_rewrites_helpers.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

namespace {

// Returns the position after the quoted literal or identifier that starts at
// `start`: '...', "...", their triple-quoted forms, or `...`.
size_t QuotedEnd(absl::string_view sql, size_t start) {
  const char quote = sql[start];
  const size_t width =
      quote != '`' && sql.substr(start, 3) == std::string(3, quote) ? 3 : 1;
  const absl::string_view close = sql.substr(start, width);
  for (size_t i = start + width; i < sql.size(); ++i) {
    if (sql[i] == '\\') {
      ++i;
    } else if (sql.substr(i, width) == close) {
      return i + width;
    }
  }
  return sql.size();
}

// Removes /* ... */ block comments. String literals, quoted identifiers and
// line comments are copied as they are, so a `/*` inside them (a
// `gs://bucket/*.csv` URI, a `-- see /*` note) starts no comment. Quotes
// inside a block comment open no literal, so license headers that quote
// "License" and "AS IS" are removed whole.
std::string StripBlockComments(absl::string_view sql) {
  std::string out;
  out.reserve(sql.size());
  for (size_t i = 0; i < sql.size();) {
    const char c = sql[i];
    const char next = i + 1 < sql.size() ? sql[i + 1] : '\0';
    if (c == '/' && next == '*') {
      const size_t close = sql.find("*/", i + 2);
      i = close == absl::string_view::npos ? sql.size() : close + 2;
      continue;
    }
    size_t end = i + 1;
    if (c == '\'' || c == '"' || c == '`') {
      end = QuotedEnd(sql, i);
    } else if (c == '#' || (c == '-' && next == '-')) {
      end = std::min(sql.find('\n', i), sql.size());
    }
    out.append(sql.substr(i, end - i));
    i = end;
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
