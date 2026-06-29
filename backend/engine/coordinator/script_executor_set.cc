#include "backend/engine/coordinator/script_executor_set.h"

#include <cctype>
#include <memory>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/strip.h"
#include "backend/engine/coordinator/local_coordinator_analyze.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/expression_column_bindings.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_output.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {

struct StatementChunk {
  std::string sql;
  int consumed_bytes = 0;
};

absl::string_view SkipLeadingLineComments(absl::string_view sql) {
  while (true) {
    sql = absl::StripAsciiWhitespace(sql);
    if (absl::StartsWith(sql, "--")) {
      const size_t newline = sql.find('\n');
      if (newline == absl::string_view::npos) {
        return absl::string_view();
      }
      sql = sql.substr(newline + 1);
      continue;
    }
    break;
  }
  return sql;
}

StatementChunk ExtractFirstStatementChunk(absl::string_view sql) {
  size_t start = 0;
  while (start < sql.size() &&
         std::isspace(static_cast<unsigned char>(sql[start]))) {
    ++start;
  }
  bool in_quote = false;
  for (size_t i = start; i < sql.size(); ++i) {
    const char c = sql[i];
    if (c == '\'') {
      in_quote = !in_quote;
      continue;
    }
    if (c == ';' && !in_quote) {
      return StatementChunk{
          std::string(absl::StripAsciiWhitespace(sql.substr(start, i - start))),
          static_cast<int>(i + 1),
      };
    }
  }
  return StatementChunk{
      std::string(absl::StripAsciiWhitespace(sql.substr(start))),
      static_cast<int>(sql.size() - start),
  };
}

absl::StatusOr<semantic::Value> EvalExpressionScalar(
    const QueryRequest& request,
    absl::string_view expr,
    catalog::GoogleSqlCatalog* bq_catalog,
    ::googlesql::Catalog* catalog,
    const semantic::FrameStack& variables) {
  absl::StatusOr<::googlesql::AnalyzerOptions> options =
      BuildAnalyzerOptionsForRequest(request,
                                     bq_catalog,
                                     /*all_statements=*/false);
  if (!options.ok()) return options.status();
  absl::Status registered =
      semantic::RegisterExpressionColumnsOnAnalyzerOptions(variables, *options);
  if (!registered.ok()) return registered;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed =
      ::googlesql::AnalyzeExpression(std::string(expr),
                                     *options,
                                     catalog,
                                     bq_catalog->type_factory(),
                                     &output);
  if (!analyzed.ok()) return analyzed;
  if (output == nullptr || output->resolved_expr() == nullptr) {
    return absl::InternalError(
        "script::EvalExpressionScalar: analyzer returned null expression");
  }
  semantic::EvalContext ctx;
  ctx.project_id = request.project_id;
  ctx.script_variables = &variables;
  ctx.arguments = &variables;
  absl::flat_hash_map<std::string, ::googlesql::Value> columns_by_name;
  semantic::PopulateEvalContextExpressionColumns(
      variables, ctx, &columns_by_name);
  return semantic::EvalExpr(*output->resolved_expr(), ctx);
}

}  // namespace

absl::Status ExecuteProcedureSet(const QueryRequest& request,
                                 absl::string_view target_name,
                                 absl::string_view expr,
                                 catalog::GoogleSqlCatalog* bq_catalog,
                                 ::googlesql::Catalog* catalog,
                                 semantic::script::ScriptDriver& driver) {
  absl::StatusOr<semantic::Value> value = EvalExpressionScalar(
      request, expr, bq_catalog, catalog, driver.variables());
  if (!value.ok()) return value.status();
  absl::Status set = driver.variables().Set(target_name, *std::move(value));
  if (!set.ok()) {
    return semantic::MakeSemanticError(
        semantic::SemanticErrorReason::kInvalidArgument,
        absl::StrCat("script::ExecuteProcedureSet: cannot assign to '",
                     target_name,
                     "': ",
                     set.message()));
  }
  return absl::OkStatus();
}

absl::Status TryExecuteLeadingSetStatement(
    const QueryRequest& request,
    ::googlesql::ParseResumeLocation& resume,
    catalog::GoogleSqlCatalog* bq_catalog,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver& driver,
    bool* handled) {
  *handled = false;
  const absl::string_view remaining =
      resume.input().substr(resume.byte_position());
  const absl::string_view trimmed = SkipLeadingLineComments(remaining);
  if (trimmed.empty() || !absl::StartsWithIgnoreCase(trimmed, "SET ")) {
    return absl::OkStatus();
  }

  const StatementChunk chunk = ExtractFirstStatementChunk(remaining);
  if (chunk.sql.empty()) {
    return absl::InvalidArgumentError("script: empty SET statement");
  }
  std::string rest = chunk.sql.substr(4);
  const size_t eq = rest.find('=');
  if (eq == std::string::npos) {
    return absl::InvalidArgumentError("script: malformed SET statement");
  }
  const std::string target =
      std::string(absl::StripAsciiWhitespace(rest.substr(0, eq)));
  const std::string expr =
      std::string(absl::StripAsciiWhitespace(rest.substr(eq + 1)));
  absl::Status set_status =
      ExecuteProcedureSet(request, target, expr, bq_catalog, catalog, driver);
  if (!set_status.ok()) {
    return set_status;
  }
  resume.set_byte_position(resume.byte_position() + chunk.consumed_bytes);
  *handled = true;
  return absl::OkStatus();
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
