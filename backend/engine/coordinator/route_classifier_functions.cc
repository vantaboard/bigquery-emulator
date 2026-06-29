#include <string>
#include <utility>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/coordinator/route_classifier_visitor.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

namespace transpiler = ::bigquery_emulator::backend::engine::duckdb::transpiler;

constexpr absl::string_view kTemplatedSqlFunctionGroup =
    "Templated_SQL_Function";

// True for the BigQuery exact-decimal arithmetic shapes DuckDB cannot
// reproduce, given the operator `name` and the result `type`:
//
//   * `$divide` over NUMERIC/BIGNUMERIC -- DuckDB widens DECIMAL
//     division to DOUBLE (the result column comes back type_id=DOUBLE),
//     which both loses precision and changes the type.
//   * `$add`/`$subtract`/`$multiply` over BIGNUMERIC -- BIGNUMERIC is
//     persisted as VARCHAR (DuckDB's max DECIMAL precision is 38, which
//     cannot even hold DECIMAL(38,38) >= 1.0), so DuckDB cannot do the
//     arithmetic at all (`+(VARCHAR, VARCHAR)` binder error).
//
// NUMERIC `+`/`-`/`*` stay duckdb_native: DuckDB keeps them DECIMAL and
// the int128 arrow_to_bq reader renders the result exactly.
bool DecimalArithmeticNeedsSemantic(absl::string_view name,
                                    const ::googlesql::Type* type) {
  if (type == nullptr) return false;
  if (name == "$divide") {
    return type->IsNumericType() || type->IsBigNumericType();
  }
  if (name == "$add" || name == "$subtract" || name == "$multiply") {
    return type->IsBigNumericType();
  }
  return false;
}

}  // namespace

void RouteClassifierVisitor::CheckFunction(
    const ::googlesql::ResolvedNode* node) {
  if (node == nullptr) return;
  const ::googlesql::Function* fn = nullptr;
  const ::googlesql::Type* result_type = nullptr;
  if (node->Is<::googlesql::ResolvedFunctionCall>()) {
    const auto* call = node->GetAs<::googlesql::ResolvedFunctionCall>();
    fn = call->function();
    result_type = call->type();
  } else if (node->Is<::googlesql::ResolvedAggregateFunctionCall>()) {
    fn = node->GetAs<::googlesql::ResolvedAggregateFunctionCall>()->function();
  } else if (node->Is<::googlesql::ResolvedAnalyticFunctionCall>()) {
    fn = node->GetAs<::googlesql::ResolvedAnalyticFunctionCall>()->function();
  }
  if (fn == nullptr) return;
  if (fn->GetGroup() == kTemplatedSqlFunctionGroup ||
      fn->GetGroup() == ::googlesql::SQLFunction::kSQLFunctionGroup) {
    MaybePromote(Disposition::kSemanticExecutor,
                 absl::StrCat("sql_udf:", fn->Name()));
    return;
  }
  const std::string name = fn->FullName(/*include_group=*/false);
  const auto* entry = transpiler::LookupFunction(name);
  if (entry == nullptr) {
    return;
  }
  if (entry->planned) return;
  // AVG(NUMERIC/BIGNUMERIC) is exact in BigQuery, but DuckDB's avg()
  // widens a DECIMAL input to DOUBLE (wrong type + lost precision), so
  // route it to the semantic executor's exact-decimal AVG instead of
  // approximating. SUM/MIN/MAX keep DuckDB's DECIMAL output.
  if (absl::AsciiStrToLower(name) == "avg" &&
      node->Is<::googlesql::ResolvedAggregateFunctionCall>()) {
    const auto* agg = node->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
    for (int i = 0; i < agg->argument_list_size(); ++i) {
      const ::googlesql::ResolvedExpr* arg = agg->argument_list(i);
      if (arg == nullptr || arg->type() == nullptr) continue;
      if (arg->type()->IsBigNumericType()) {
        MaybePromote(Disposition::kSemanticExecutor,
                     "function:avg(bignumeric)");
        return;
      }
      if (arg->type()->IsNumericType()) {
        MaybePromote(Disposition::kSemanticExecutor, "function:avg(numeric)");
        return;
      }
    }
  }
  // Exact-decimal arithmetic DuckDB cannot reproduce routes to the
  // semantic executor's NumericValue/BigNumericValue path.
  if (DecimalArithmeticNeedsSemantic(name, result_type)) {
    MaybePromote(Disposition::kSemanticExecutor,
                 absl::StrCat("function:", name, "(decimal)"));
    return;
  }
  MaybePromote(entry->disposition,
               absl::StrCat("function:", absl::AsciiStrToLower(name)));
}

void RouteClassifierVisitor::CheckTvf(
    const ::googlesql::ResolvedTVFScan* node) {
  if (node == nullptr || node->tvf() == nullptr) return;
  const std::string name = absl::AsciiStrToLower(node->tvf()->SQLName());
  const auto* entry = transpiler::LookupFunction(name);
  if (entry == nullptr || entry->planned) return;
  MaybePromote(entry->disposition, absl::StrCat("function:", name));
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
