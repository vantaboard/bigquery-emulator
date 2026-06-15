#include "backend/engine/semantic/explain_stmt.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/coordinator/route_classifier.h"
#include "backend/engine/disposition.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

schema::TableSchema MakeExplainOutputSchema() {
  schema::TableSchema schema;
  schema::ColumnSchema plan;
  plan.name = "plan";
  plan.type = schema::ColumnType::kString;
  plan.mode = schema::ColumnMode::kRequired;
  schema.columns.push_back(plan);
  return schema;
}

const char* DispositionName(Disposition d) {
  switch (d) {
    case Disposition::kDuckdbNative:
      return "duckdb_native";
    case Disposition::kDuckdbRewrite:
      return "duckdb_rewrite";
    case Disposition::kDuckdbUdf:
      return "duckdb_udf";
    case Disposition::kSemanticExecutor:
      return "semantic_executor";
    case Disposition::kLocalStub:
      return "local_stub";
    case Disposition::kControlOp:
      return "control_op";
    case Disposition::kUnsupported:
      return "unsupported";
  }
  return "unknown";
}

std::string ResolvedNodeLabel(const ::googlesql::ResolvedStatement& stmt) {
  std::string kind = stmt.node_kind_string();
  if (!kind.empty() && !absl::StartsWith(kind, "Resolved")) {
    return absl::StrCat("Resolved", kind);
  }
  return kind;
}

}  // namespace

absl::StatusOr<std::unique_ptr<RowSource>> ExecuteExplainStmt(
    const ::googlesql::ResolvedExplainStmt& explain_stmt) {
  const ::googlesql::ResolvedStatement* inner = explain_stmt.statement();
  if (inner == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: EXPLAIN inner statement is null");
  }
  coordinator::RouteClassifier classifier;
  coordinator::RouteDecision decision = classifier.Classify(*inner);
  const std::string plan_text =
      absl::StrCat(DispositionName(decision.disposition),
                   ":",
                   decision.offending_node.empty() ? ResolvedNodeLabel(*inner)
                                                   : decision.offending_node);

  schema::TableSchema output_schema = MakeExplainOutputSchema();
  storage::Row row;
  row.cells.push_back(storage::Value::String(plan_text));
  std::vector<storage::Row> rows;
  rows.push_back(std::move(row));
  return std::unique_ptr<RowSource>(
      new MaterializedRowSource(std::move(output_schema), std::move(rows)));
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
