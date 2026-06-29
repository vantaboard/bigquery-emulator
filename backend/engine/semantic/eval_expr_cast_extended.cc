#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/cast.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using functions::datetime_internal::DefaultTimeZone;

::googlesql::LanguageOptions ProductExternalLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

absl::StatusOr<::googlesql::ExtendedCompositeCastEvaluator>
BuildExtendedCastEvaluator(const ::googlesql::ResolvedExtendedCast& ext) {
  std::vector<::googlesql::ConversionEvaluator> evaluators;
  for (int i = 0; i < ext.element_list_size(); ++i) {
    const ::googlesql::ResolvedExtendedCastElement* elem = ext.element_list(i);
    if (elem == nullptr) {
      return absl::InvalidArgumentError(
          "semantic: extended_cast element is null");
    }
    auto evaluator_or = ::googlesql::ConversionEvaluator::Create(
        elem->from_type(), elem->to_type(), elem->function());
    if (!evaluator_or.ok()) return evaluator_or.status();
    evaluators.push_back(*std::move(evaluator_or));
  }
  if (evaluators.empty()) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: extended_cast has no elements");
  }
  return ::googlesql::ExtendedCompositeCastEvaluator(std::move(evaluators));
}

bool IsUnsupportedExtendedCastTarget(const ::googlesql::Type* type) {
  if (type == nullptr) return true;
  switch (type->kind()) {
    case ::googlesql::TYPE_PROTO:
    case ::googlesql::TYPE_ENUM:
    case ::googlesql::TYPE_RANGE:
    case ::googlesql::TYPE_GRAPH_ELEMENT:
    case ::googlesql::TYPE_GRAPH_PATH:
    case ::googlesql::TYPE_MEASURE:
    case ::googlesql::TYPE_TOKENLIST:
      return true;
    default:
      return false;
  }
}

}  // namespace

absl::StatusOr<Value> EvalExtendedCast(const ::googlesql::ResolvedCast& cast,
                                       Value inner,
                                       const ::googlesql::Type* source) {
  const ::googlesql::ResolvedExtendedCast* ext = cast.extended_cast();
  if (ext == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: EvalExtendedCast requires extended_cast");
  }
  const ::googlesql::Type* target = cast.type();
  if (target == nullptr) {
    return absl::InvalidArgumentError("semantic: ResolvedCast has null type");
  }
  if (IsUnsupportedExtendedCastTarget(target) ||
      (source != nullptr && IsUnsupportedExtendedCastTarget(source))) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             absl::StrCat("semantic: CAST extended_cast to ",
                                          target->DebugString(),
                                          " is unsupported locally"));
  }

  auto evaluator_or = BuildExtendedCastEvaluator(*ext);
  if (!evaluator_or.ok()) return evaluator_or.status();

  std::optional<std::string> format;
  if (cast.format() != nullptr &&
      cast.format()->node_kind() == ::googlesql::RESOLVED_LITERAL &&
      cast.format()->type()->kind() == ::googlesql::TYPE_STRING) {
    format = cast.format()
                 ->GetAs<::googlesql::ResolvedLiteral>()
                 ->value()
                 .string_value();
  }
  std::optional<std::string> time_zone;
  if (cast.time_zone() != nullptr &&
      cast.time_zone()->node_kind() == ::googlesql::RESOLVED_LITERAL &&
      cast.time_zone()->type()->kind() == ::googlesql::TYPE_STRING) {
    time_zone = cast.time_zone()
                    ->GetAs<::googlesql::ResolvedLiteral>()
                    ->value()
                    .string_value();
  }

  auto casted = ::googlesql::internal::CastValueWithoutTypeValidation(
      inner,
      DefaultTimeZone(),
      absl::Now(),
      ProductExternalLanguageOptions(),
      target,
      format,
      time_zone,
      &(*evaluator_or),
      /*canonicalize_zero=*/false);
  if (!casted.ok()) {
    if (cast.return_null_on_error()) return NullOfType(target);
    return casted.status();
  }
  return *std::move(casted);
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
