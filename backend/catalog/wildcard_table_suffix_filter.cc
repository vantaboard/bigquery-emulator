

#include "googlesql/public/type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

using ::googlesql::RESOLVED_COLUMN_REF;
using ::googlesql::RESOLVED_FILTER_SCAN;
using ::googlesql::RESOLVED_FUNCTION_CALL;
using ::googlesql::RESOLVED_LITERAL;
using ::googlesql::RESOLVED_PROJECT_SCAN;
using ::googlesql::RESOLVED_TABLE_SCAN;

bool IsTableSuffixColumnRef(const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr || expr->node_kind() != RESOLVED_COLUMN_REF) {
    return false;
  }
  const auto* ref = expr->GetAs<::googlesql::ResolvedColumnRef>();
  return ref != nullptr && ref->column().name() == kTableSuffixColumnName;
}

std::optional<std::string> StringFromLiteral(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr || expr->node_kind() != RESOLVED_LITERAL) {
    return std::nullopt;
  }
  const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
  if (lit == nullptr) return std::nullopt;
  const ::googlesql::Value& v = lit->value();
  if (!v.is_valid() || v.is_null()) return std::nullopt;
  if (v.type()->IsString()) return v.string_value();
  return std::nullopt;
}

std::optional<std::vector<std::string>> IntersectAllowLists(
    const std::optional<std::vector<std::string>>& a,
    const std::optional<std::vector<std::string>>& b) {
  if (!a.has_value() || !b.has_value()) return std::nullopt;
  std::vector<std::string> out;
  for (const std::string& s : *a) {
    if (std::find(b->begin(), b->end(), s) != b->end()) {
      out.push_back(s);
    }
  }
  return out;
}

std::optional<std::vector<std::string>> ExtractFromInCall(
    const ::googlesql::ResolvedFunctionCall* call) {
  if (call == nullptr || call->argument_list_size() != 2) {
    return std::nullopt;
  }
  const ::googlesql::ResolvedExpr* left = call->argument_list(0);
  const ::googlesql::ResolvedExpr* right = call->argument_list(1);
  if (!IsTableSuffixColumnRef(left)) return std::nullopt;
  if (right == nullptr || right->node_kind() != RESOLVED_LITERAL) {
    return std::nullopt;
  }
  const auto* lit = right->GetAs<::googlesql::ResolvedLiteral>();
  if (lit == nullptr) return std::nullopt;
  const ::googlesql::Value& v = lit->value();
  if (!v.is_valid() || v.is_null() || !v.type()->IsArray()) {
    return std::nullopt;
  }
  std::vector<std::string> out;
  for (int i = 0; i < v.num_elements(); ++i) {
    const ::googlesql::Value& elem = v.element(i);
    if (!elem.is_valid() || elem.is_null() || !elem.type()->IsString()) {
      return std::nullopt;
    }
    out.push_back(elem.string_value());
  }
  return out;
}

std::optional<std::vector<std::string>> ExtractFromBetweenCall(
    const ::googlesql::ResolvedFunctionCall* call) {
  if (call == nullptr || call->argument_list_size() != 3) {
    return std::nullopt;
  }
  if (!IsTableSuffixColumnRef(call->argument_list(0))) {
    return std::nullopt;
  }
  auto low = StringFromLiteral(call->argument_list(1));
  auto high = StringFromLiteral(call->argument_list(2));
  if (!low.has_value() || !high.has_value()) return std::nullopt;
  return std::vector<std::string>{*low, *high};
}

std::optional<std::vector<std::string>> ExtractFromEqualCall(
    const ::googlesql::ResolvedFunctionCall* call) {
  if (call == nullptr || call->argument_list_size() != 2) {
    return std::nullopt;
  }
  const ::googlesql::ResolvedExpr* left = call->argument_list(0);
  const ::googlesql::ResolvedExpr* right = call->argument_list(1);
  const ::googlesql::ResolvedExpr* lit_side = nullptr;
  if (IsTableSuffixColumnRef(left)) {
    lit_side = right;
  } else if (IsTableSuffixColumnRef(right)) {
    lit_side = left;
  } else {
    return std::nullopt;
  }
  auto s = StringFromLiteral(lit_side);
  if (!s.has_value()) return std::nullopt;
  return std::vector<std::string>{*s};
}

std::optional<std::vector<std::string>> ExtractFromExpr(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) return std::nullopt;
  if (expr->node_kind() != RESOLVED_FUNCTION_CALL) return std::nullopt;
  const auto* call = expr->GetAs<::googlesql::ResolvedFunctionCall>();
  if (call == nullptr || call->function() == nullptr) return std::nullopt;
  const std::string fn = absl::AsciiStrToLower(call->function()->Name());
  if (fn == "$equal") return ExtractFromEqualCall(call);
  if (fn == "$in") return ExtractFromInCall(call);
  if (fn == "$between") return ExtractFromBetweenCall(call);
  if (fn == "$and") {
    std::optional<std::vector<std::string>> acc;
    for (int i = 0; i < call->argument_list_size(); ++i) {
      auto part = ExtractFromExpr(call->argument_list(i));
      if (!part.has_value()) return std::nullopt;
      if (!acc.has_value()) {
        acc = *part;
        continue;
      }
      acc = IntersectAllowLists(acc, part);
      if (acc.has_value() && acc->empty()) return acc;
    }
    return acc;
  }
  return std::nullopt;
}

const ::googlesql::ResolvedTableScan* UnwrapToTableScan(
    const ::googlesql::ResolvedScan* scan) {
  while (scan != nullptr) {
    switch (scan->node_kind()) {
      case RESOLVED_PROJECT_SCAN:
        scan = scan->GetAs<::googlesql::ResolvedProjectScan>()->input_scan();
        break;
      case RESOLVED_FILTER_SCAN:
        scan = scan->GetAs<::googlesql::ResolvedFilterScan>()->input_scan();
        break;
      default:
        if (scan->node_kind() == RESOLVED_TABLE_SCAN) {
          return scan->GetAs<::googlesql::ResolvedTableScan>();
        }
        return nullptr;
    }
  }
  return nullptr;
}

}  // namespace

std::optional<std::vector<std::string>> ExtractTableSuffixAllowList(
    const ::googlesql::ResolvedExpr* filter_expr) {
  return ExtractFromExpr(filter_expr);
}

std::optional<std::vector<std::string>> FindTableSuffixAllowListForWildcardScan(
    const ::googlesql::ResolvedScan* scan,
    absl::string_view wildcard_table_id) {
  if (scan == nullptr) return std::nullopt;
  if (scan->node_kind() != RESOLVED_FILTER_SCAN) return std::nullopt;
  const auto* filter_scan = scan->GetAs<::googlesql::ResolvedFilterScan>();
  const ::googlesql::ResolvedTableScan* table_scan =
      UnwrapToTableScan(filter_scan->input_scan());
  if (table_scan == nullptr || table_scan->table() == nullptr) {
    return std::nullopt;
  }
  if (table_scan->table()->Name() != wildcard_table_id) {
    return std::nullopt;
  }
  return ExtractTableSuffixAllowList(filter_scan->filter_expr());
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
