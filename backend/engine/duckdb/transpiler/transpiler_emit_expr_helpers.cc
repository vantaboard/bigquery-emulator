#include "backend/engine/duckdb/transpiler/transpiler_emit_expr_helpers.h"

#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

bool CastHasUnsupportedFeatures(const ::googlesql::ResolvedCast* node) {
  if (node == nullptr || node->expr() == nullptr) return true;
  if (node->format() != nullptr || node->time_zone() != nullptr) return true;
  if (node->extended_cast() != nullptr) return true;
  if (!node->type_modifiers().IsEmpty()) return true;
  const ::googlesql::Type* target = node->type();
  if (target == nullptr) return true;
  return !internal::IsCastTargetSupported(target->kind());
}

std::string NormalizeTimestampCastInner(absl::string_view inner,
                                        const ::googlesql::ResolvedExpr* expr) {
  if (auto lit = internal::TryLiteralString(expr)) {
    if (lit->find('+') == std::string::npos &&
        lit->find('Z') == std::string::npos &&
        lit->find("UTC") == std::string::npos &&
        (lit->size() < 6 || lit->compare(lit->size() - 6, 6, "+00:00") != 0)) {
      return internal::QuoteString(absl::StrCat(*lit, "+00"));
    }
  }
  return std::string(inner);
}

std::string TryEmitStructTypeCast(const std::string& inner,
                                  const ::googlesql::Type* target,
                                  const ::googlesql::ResolvedExpr* expr) {
  if (target == nullptr || target->kind() != ::googlesql::TYPE_STRUCT) {
    return "";
  }
  const ::googlesql::Type* source_type = expr->type();
  if (source_type == nullptr || !source_type->IsStruct()) {
    return "";
  }
  const ::googlesql::StructType* target_st = target->AsStruct();
  const ::googlesql::StructType* source_st = source_type->AsStruct();
  if (target_st == nullptr || source_st == nullptr) {
    return "";
  }
  return internal::EmitStructPositionalCastRemap(inner, *source_st, *target_st);
}

std::string FormatCastExpression(absl::string_view inner,
                                 absl::string_view type_sql,
                                 bool return_null_on_error) {
  const char* op = return_null_on_error ? "TRY_CAST" : "CAST";
  return absl::StrCat(op, "(", inner, " AS ", type_sql, ")");
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
