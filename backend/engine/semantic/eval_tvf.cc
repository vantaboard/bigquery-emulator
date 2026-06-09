#include "backend/engine/semantic/eval_tvf.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/sql_tvf.h"
#include "googlesql/public/templated_sql_tvf.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

absl::Status BindTvfArguments(const ::googlesql::ResolvedTVFScan& scan,
                              const std::vector<std::string>& arg_names,
                              const EvalContext& ctx,
                              FrameStack& frames) {
  if (arg_names.size() != static_cast<size_t>(scan.argument_list_size())) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: TVF argument count mismatch (expected ",
                     arg_names.size(),
                     ", got ",
                     scan.argument_list_size(),
                     ")"));
  }
  frames.PushFrame();
  for (int i = 0; i < scan.argument_list_size(); ++i) {
    const ::googlesql::ResolvedFunctionArgument* arg = scan.argument_list(i);
    if (arg == nullptr || arg->expr() == nullptr) {
      return absl::InvalidArgumentError(
          "semantic: TVF call has null argument expression");
    }
    auto value = EvalExpr(*arg->expr(), ctx);
    if (!value.ok()) return value.status();
    absl::Status declared = frames.Declare(arg_names[i], *std::move(value));
    if (!declared.ok()) return declared;
  }
  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeTvfScan(
    const ::googlesql::ResolvedTVFScan& scan, EvalContext& ctx) {
  if (scan.tvf() == nullptr) {
    return absl::InvalidArgumentError("semantic: TVFScan has null tvf");
  }
  if (scan.signature() != nullptr) {
    const auto* templated_sig =
        dynamic_cast<const ::googlesql::TemplatedSQLTVFSignature*>(
            scan.signature().get());
    if (templated_sig != nullptr &&
        templated_sig->resolved_templated_query() != nullptr &&
        templated_sig->resolved_templated_query()->query() != nullptr) {
      return scan_eval_internal::MaterializeScanImpl(
          templated_sig->resolved_templated_query()->query(), ctx);
    }
  }

  const auto* sql_tvf =
      dynamic_cast<const ::googlesql::SQLTableValuedFunction*>(scan.tvf());
  if (sql_tvf == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             absl::StrCat("semantic: TVF '",
                                          scan.tvf()->SQLName(),
                                          "' is not a supported SQL TVF"));
  }
  if (sql_tvf->query() == nullptr) {
    return absl::InternalError(
        "semantic: SQLTableValuedFunction has null query scan");
  }
  FrameStack frames;
  absl::Status bound =
      BindTvfArguments(scan, sql_tvf->GetArgumentNames(), ctx, frames);
  if (!bound.ok()) return bound;
  EvalContext inner = ctx;
  inner.arguments = &frames;
  return scan_eval_internal::MaterializeScanImpl(sql_tvf->query(), inner);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
