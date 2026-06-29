#include "backend/sqltools/sql_tools_complete_helpers.h"

#include <cctype>
#include <cstddef>
#include <string>

#include "absl/strings/string_view.h"
#include "googlesql/public/parse_tokens.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace sql_tools_complete_internal {
namespace {

bool IsTableContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{"FROM",
                                           "JOIN",
                                           "INNER",
                                           "LEFT",
                                           "RIGHT",
                                           "FULL",
                                           "CROSS",
                                           "INTO",
                                           "UPDATE",
                                           "TABLE",
                                           "MERGE",
                                           "USING",
                                           "DELETE",
                                           "TRUNCATE"};
  return kKeywords->contains(std::string(keyword));
}

bool IsColumnContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{"SELECT",
                                           "WHERE",
                                           "ON",
                                           "BY",
                                           "HAVING",
                                           "QUALIFY",
                                           "SET",
                                           "AND",
                                           "OR",
                                           "NOT",
                                           "WHEN",
                                           "THEN",
                                           "ELSE",
                                           "CASE",
                                           "GROUP",
                                           "ORDER",
                                           "PARTITION"};
  return kKeywords->contains(std::string(keyword));
}

bool IsRoutineContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{"FUNCTION", "PROCEDURE", "ROUTINE"};
  return kKeywords->contains(std::string(keyword));
}

std::string TokenText(const ::googlesql::ParseToken& token) {
  if (token.IsIdentifier()) return token.GetIdentifier();
  if (token.IsKeyword()) return token.GetKeyword();
  return std::string(token.GetImage());
}

CompletionContext InferDotCompletionContext(
    const std::vector<::googlesql::ParseToken>& tokens, int idx) {
  CompletionContext ctx;
  bool after_table_intro = false;
  for (int i = idx - 2; i >= 0; --i) {
    if (!tokens[i].IsKeyword()) continue;
    const std::string kw = tokens[i].GetKeyword();
    if (IsTableContextKeyword(kw)) {
      after_table_intro = true;
      break;
    }
    if (IsColumnContextKeyword(kw)) {
      break;
    }
  }
  if (!after_table_intro) {
    ctx.kind = CompletionContextKind::kMember;
    ctx.qualifier = TokenText(tokens[idx - 2]);
    return ctx;
  }
  ctx.kind = CompletionContextKind::kTable;
  return ctx;
}

CompletionContext InferKeywordCompletionContext(
    const std::vector<::googlesql::ParseToken>& tokens, int idx) {
  CompletionContext ctx;
  const std::string last_keyword = tokens[idx].GetKeyword();
  if (IsTableContextKeyword(last_keyword)) {
    ctx.kind = CompletionContextKind::kTable;
    return ctx;
  }
  for (int i = idx; i >= 0; --i) {
    if (!tokens[i].IsKeyword()) continue;
    const std::string kw = tokens[i].GetKeyword();
    if (IsRoutineContextKeyword(kw)) {
      ctx.kind = CompletionContextKind::kRoutine;
      return ctx;
    }
    if (IsTableContextKeyword(kw)) {
      ctx.kind = CompletionContextKind::kTable;
      return ctx;
    }
    if (IsColumnContextKeyword(kw)) {
      ctx.kind = CompletionContextKind::kColumn;
      return ctx;
    }
  }
  return ctx;
}

}  // namespace

CompletionContext InferCompletionContext(
    const std::vector<::googlesql::ParseToken>& tokens) {
  CompletionContext ctx;
  if (tokens.empty()) return ctx;

  int idx = static_cast<int>(tokens.size()) - 1;
  while (idx >= 0 && tokens[idx].IsEndOfInput()) {
    --idx;
  }
  if (idx < 0) return ctx;

  if (idx > 0 && tokens[idx - 1].GetKeyword() == ".") {
    return InferDotCompletionContext(tokens, idx);
  }

  return InferKeywordCompletionContext(tokens, idx);
}

void FindReplacementSpan(absl::string_view sql,
                         size_t cursor,
                         CompletionContextKind context_kind,
                         int* replacement_start,
                         int* replacement_end,
                         std::string* prefix) {
  *replacement_start = static_cast<int>(cursor);
  *replacement_end = static_cast<int>(cursor);
  prefix->clear();
  if (cursor > sql.size()) cursor = sql.size();
  size_t start = cursor;
  while (start > 0) {
    const char c = sql[start - 1];
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') break;
    --start;
  }
  if (context_kind == CompletionContextKind::kTable) {
    size_t qual_start = start;
    while (qual_start > 0 && sql[qual_start - 1] == '.') {
      --qual_start;
      while (qual_start > 0) {
        const char c = sql[qual_start - 1];
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') break;
        --qual_start;
      }
    }
    start = qual_start;
  }
  *replacement_start = static_cast<int>(start);
  *replacement_end = static_cast<int>(cursor);
  prefix->assign(sql.substr(start, cursor - start));
}

}  // namespace sql_tools_complete_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
