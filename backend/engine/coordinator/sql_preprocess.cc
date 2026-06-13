#include "backend/engine/coordinator/sql_preprocess.h"

#include <cctype>
#include <string>

#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "backend/engine/coordinator/sql_preprocess_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

std::string PreprocessFunctionBodyForAnalyzer(absl::string_view sql) {
  std::string normalized =
      sql_preprocess_internal::NormalizeCreateFunctionAsParens(
          sql_preprocess_internal::PreprocessFunctionBodyBase(sql));
  while (!normalized.empty() &&
         std::isspace(static_cast<unsigned char>(normalized.back())) != 0) {
    normalized.pop_back();
  }
  // Templated SQL UDF bodies are parsed via ParseResumeLocation as
  // expressions, not semicolon-terminated statements.
  if (!normalized.empty() && normalized.back() == ';') {
    normalized.pop_back();
  }
  return normalized;
}

std::string PreprocessSqlForAnalyzer(absl::string_view sql) {
  if (sql_preprocess_internal::HasDecoratorAndSystemTimeConflict(sql)) {
    return std::string(sql);
  }
  const std::string base =
      sql_preprocess_internal::PreprocessFunctionBodyBase(sql);
  if (absl::StrContains(absl::AsciiStrToLower(base), "create function")) {
    return sql_preprocess_internal::NormalizeCreateFunctionAsParens(base);
  }
  return base;
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
