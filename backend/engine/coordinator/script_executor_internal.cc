#include "backend/engine/coordinator/script_executor_internal.h"

#include <memory>
#include <string>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/strip.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/semantic/frame_stack.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/simple_catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

bool ScriptKeywordPresent(absl::string_view sql, absl::string_view keyword) {
  std::string upper = std::string(sql);
  for (char& c : upper) {
    c = static_cast<char>(absl::ascii_toupper(static_cast<unsigned char>(c)));
  }
  return upper.find(keyword) != std::string::npos;
}

}  // namespace

std::string LeadingScriptStatement(absl::string_view sql) {
  bool in_quote = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (c == '\'') {
      in_quote = !in_quote;
      continue;
    }
    if (c == ';' && !in_quote) {
      return std::string(absl::StripAsciiWhitespace(sql.substr(0, i)));
    }
  }
  return std::string(absl::StripAsciiWhitespace(sql));
}

bool ScriptUsesCreateConstantLowering(absl::string_view sql) {
  return ScriptKeywordPresent(sql, "CREATE CONSTANT");
}

std::string StripBeginEnd(absl::string_view sql) {
  std::string trimmed = std::string(absl::StripAsciiWhitespace(sql));
  if (absl::StartsWithIgnoreCase(trimmed, "BEGIN")) {
    trimmed = trimmed.substr(5);
    trimmed = std::string(absl::StripAsciiWhitespace(trimmed));
  }
  if (absl::EndsWithIgnoreCase(trimmed, "END")) {
    size_t pos = trimmed.size();
    while (pos > 0 && (trimmed[pos - 1] == ';' || trimmed[pos - 1] == ' ')) {
      --pos;
    }
    if (pos >= 3 && absl::EqualsIgnoreCase(trimmed.substr(pos - 3, 3), "END")) {
      trimmed.resize(pos - 3);
      trimmed = std::string(absl::StripAsciiWhitespace(trimmed));
    }
  }
  return trimmed;
}

absl::Status RegisterScriptVariablesOnCatalog(
    catalog::GoogleSqlCatalog* catalog,
    const semantic::FrameStack& variables,
    absl::flat_hash_set<std::string>* registered) {
  for (const auto& [name, value] : variables.VisibleBindings()) {
    if (registered->contains(name)) continue;
    const ::googlesql::Constant* existing = nullptr;
    absl::Status found =
        catalog->FindConstant({name}, &existing, /*options=*/{});
    if (found.ok() && existing != nullptr) {
      registered->insert(name);
      continue;
    }
    std::unique_ptr<::googlesql::SimpleConstant> constant;
    absl::Status created =
        ::googlesql::SimpleConstant::Create({name}, value, &constant);
    if (!created.ok()) return created;
    catalog->AddOwnedConstant(constant.release());
    registered->insert(name);
  }
  return absl::OkStatus();
}

absl::Status RegisterScriptVariablesOnCatalogFromDriver(
    catalog::GoogleSqlCatalog* catalog,
    const semantic::script::ScriptDriver& driver,
    absl::flat_hash_set<std::string>* registered) {
  return RegisterScriptVariablesOnCatalog(
      catalog, driver.variables(), registered);
}

bool ScriptNeedsGoogleSqlExecutor(absl::string_view sql) {
  const std::string trimmed = std::string(absl::StripAsciiWhitespace(sql));
  // Leading control-flow / dynamic-SQL statements (after the hybrid prefix
  // loop positions resume at IF/WHILE/... without a leading space).
  if (absl::StartsWithIgnoreCase(trimmed, "IF ") ||
      absl::StartsWithIgnoreCase(trimmed, "WHILE ") ||
      absl::StartsWithIgnoreCase(trimmed, "LOOP ") ||
      absl::StartsWithIgnoreCase(trimmed, "REPEAT") ||
      absl::StartsWithIgnoreCase(trimmed, "FOR ") ||
      absl::StartsWithIgnoreCase(trimmed, "RAISE ") ||
      absl::StartsWithIgnoreCase(trimmed, "EXECUTE IMMEDIATE") ||
      absl::StartsWithIgnoreCase(trimmed, "EXCEPTION")) {
    return true;
  }
  // Do not match BEGIN (all multi-statement scripts use it) or bare FOR
  // (false positives inside identifiers). FOR...IN loops include both tokens.
  return ScriptKeywordPresent(sql, " IF ") ||
         ScriptKeywordPresent(sql, "\nIF ") ||
         ScriptKeywordPresent(sql, " WHILE ") ||
         ScriptKeywordPresent(sql, "\nWHILE ") ||
         ScriptKeywordPresent(sql, " LOOP ") ||
         ScriptKeywordPresent(sql, "\nLOOP ") ||
         ScriptKeywordPresent(sql, " REPEAT") ||
         ScriptKeywordPresent(sql, "\nREPEAT") ||
         (ScriptKeywordPresent(sql, " FOR ") &&
          ScriptKeywordPresent(sql, " IN ")) ||
         (ScriptKeywordPresent(sql, "\nFOR ") &&
          ScriptKeywordPresent(sql, " IN ")) ||
         ScriptKeywordPresent(sql, "EXCEPTION") ||
         ScriptKeywordPresent(sql, " RAISE ") ||
         ScriptKeywordPresent(sql, "\nRAISE ") ||
         ScriptKeywordPresent(sql, "EXECUTE IMMEDIATE");
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
