#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;

absl::StatusOr<std::vector<ColumnBindings>> MaterializeRelationArgumentScan(
    const ::googlesql::ResolvedRelationArgumentScan& ref, EvalContext& ctx) {
  if (ctx.relation_arguments == nullptr) {
    return absl::InternalError(
        "semantic: RelationArgumentScan without active TVF relation bindings");
  }
  const std::string key = absl::AsciiStrToLower(ref.name());
  auto it = ctx.relation_arguments->find(key);
  if (it == ctx.relation_arguments->end()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat(
            "semantic: unknown TVF relation argument '", ref.name(), "'"));
  }
  const CteTable& bound = it->second;
  if (static_cast<int>(bound.column_ids.size()) != ref.column_list_size()) {
    return absl::InternalError(
        "semantic: RelationArgumentScan column count does not match bound "
        "relation");
  }
  std::vector<ColumnBindings> out;
  out.reserve(bound.rows.size());
  for (const ColumnBindings& bound_row : bound.rows) {
    ColumnBindings row;
    row.reserve(ref.column_list_size());
    for (int i = 0; i < ref.column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& dst = ref.column_list(i);
      const int src_id = bound.column_ids[i];
      auto cit = bound_row.find(src_id);
      if (cit == bound_row.end()) {
        return absl::InternalError(absl::StrCat(
            "semantic: TVF relation row missing column_id=", src_id));
      }
      row.emplace(dst.column_id(), cit->second);
    }
    out.push_back(std::move(row));
  }
  return out;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
