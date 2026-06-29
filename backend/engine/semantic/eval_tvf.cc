#include "backend/engine/semantic/eval_tvf.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/stubs/ml.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/sql_tvf.h"
#include "googlesql/public/templated_sql_tvf.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

absl::StatusOr<std::vector<ColumnBindings>> RemapScanOutputColumns(
    const std::vector<ColumnBindings>& rows,
    const ::googlesql::ResolvedScan* src_scan,
    const ::googlesql::ResolvedTVFScan& tvf_scan) {
  if (src_scan == nullptr) {
    return rows;
  }
  if (tvf_scan.column_list_size() != src_scan->column_list_size()) {
    return absl::InternalError(
        "semantic: TVFScan output column count does not match inner query");
  }
  if (tvf_scan.column_list_size() == 0) {
    return rows;
  }
  std::vector<ColumnBindings> out;
  out.reserve(rows.size());
  for (const ColumnBindings& in_row : rows) {
    ColumnBindings row;
    row.reserve(tvf_scan.column_list_size());
    for (int i = 0; i < tvf_scan.column_list_size(); ++i) {
      const int src_id = src_scan->column_list(i).column_id();
      const ::googlesql::ResolvedColumn& dst = tvf_scan.column_list(i);
      auto it = in_row.find(src_id);
      if (it == in_row.end()) {
        return absl::InternalError(
            absl::StrCat("semantic: TVF inner row missing column_id=", src_id));
      }
      row.emplace(dst.column_id(), it->second);
    }
    out.push_back(std::move(row));
  }
  return out;
}

absl::Status BindTvfArguments(
    const ::googlesql::ResolvedTVFScan& scan,
    const std::vector<std::string>& arg_names,
    EvalContext& ctx,
    FrameStack& frames,
    absl::flat_hash_map<std::string, CteTable>& relations) {
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
    if (arg == nullptr) {
      return absl::InvalidArgumentError("semantic: TVF call has null argument");
    }
    const std::string arg_key = absl::AsciiStrToLower(arg_names[i]);
    if (arg->scan() != nullptr) {
      auto rows = scan_eval_internal::MaterializeScanImpl(arg->scan(), ctx);
      if (!rows.ok()) return rows.status();
      CteTable table;
      table.rows = *std::move(rows);
      table.column_ids.reserve(arg->scan()->column_list_size());
      for (int j = 0; j < arg->scan()->column_list_size(); ++j) {
        table.column_ids.push_back(arg->scan()->column_list(j).column_id());
      }
      relations.emplace(arg_key, std::move(table));
      continue;
    }
    if (arg->expr() == nullptr) {
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

absl::StatusOr<std::vector<ColumnBindings>> MaterializeMlPredictTvf(
    const ::googlesql::ResolvedTVFScan& scan, EvalContext& ctx) {
  for (int i = 0; i < scan.argument_list_size(); ++i) {
    const ::googlesql::ResolvedFunctionArgument* arg = scan.argument_list(i);
    if (arg == nullptr || arg->scan() == nullptr) {
      continue;
    }
    const ::googlesql::ResolvedScan* input_scan = arg->scan();
    auto input_rows = scan_eval_internal::MaterializeScanImpl(input_scan, ctx);
    if (!input_rows.ok()) {
      return input_rows.status();
    }
    return stubs::MlPredictStub(scan, *input_rows, input_scan);
  }
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      "semantic stub: ML.PREDICT requires a TABLE input argument");
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeMlEvaluateTvf(
    const ::googlesql::ResolvedTVFScan& scan, EvalContext& ctx) {
  for (int i = 0; i < scan.argument_list_size(); ++i) {
    const ::googlesql::ResolvedFunctionArgument* arg = scan.argument_list(i);
    if (arg != nullptr && arg->scan() != nullptr) {
      auto input_rows =
          scan_eval_internal::MaterializeScanImpl(arg->scan(), ctx);
      if (!input_rows.ok()) {
        return input_rows.status();
      }
      break;
    }
  }
  return stubs::MlEvaluateStub(scan);
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeTemplatedSqlTvf(
    const ::googlesql::ResolvedTVFScan& scan, EvalContext& ctx) {
  const auto* templated_sig =
      dynamic_cast<const ::googlesql::TemplatedSQLTVFSignature*>(
          scan.signature().get());
  const ::googlesql::ResolvedScan* inner =
      templated_sig->resolved_templated_query()->query();
  auto rows = scan_eval_internal::MaterializeScanImpl(inner, ctx);
  if (!rows.ok()) return rows.status();
  return RemapScanOutputColumns(*rows, inner, scan);
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeRegisteredSqlTvf(
    const ::googlesql::ResolvedTVFScan& scan, EvalContext& ctx) {
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
  absl::flat_hash_map<std::string, CteTable> relations;
  absl::Status bound = BindTvfArguments(
      scan, sql_tvf->GetArgumentNames(), ctx, frames, relations);
  if (!bound.ok()) return bound;
  EvalContext inner = ctx;
  inner.arguments = &frames;
  inner.relation_arguments = &relations;
  const ::googlesql::ResolvedScan* inner_query = sql_tvf->query();
  auto rows = scan_eval_internal::MaterializeScanImpl(inner_query, inner);
  if (!rows.ok()) return rows.status();
  return RemapScanOutputColumns(*rows, inner_query, scan);
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeTvfScan(
    const ::googlesql::ResolvedTVFScan& scan, EvalContext& ctx) {
  if (scan.tvf() == nullptr) {
    return absl::InvalidArgumentError("semantic: TVFScan has null tvf");
  }
  const std::string tvf_name = absl::AsciiStrToLower(scan.tvf()->SQLName());
  if (tvf_name == "ml.predict") {
    return MaterializeMlPredictTvf(scan, ctx);
  }
  if (tvf_name == "ml.evaluate") {
    return MaterializeMlEvaluateTvf(scan, ctx);
  }
  if (tvf_name == "ml.forecast") {
    return stubs::MlForecastStub(scan);
  }
  if (scan.signature() != nullptr) {
    const auto* templated_sig =
        dynamic_cast<const ::googlesql::TemplatedSQLTVFSignature*>(
            scan.signature().get());
    if (templated_sig != nullptr &&
        templated_sig->resolved_templated_query() != nullptr &&
        templated_sig->resolved_templated_query()->query() != nullptr) {
      return MaterializeTemplatedSqlTvf(scan, ctx);
    }
  }
  return MaterializeRegisteredSqlTvf(scan, ctx);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
