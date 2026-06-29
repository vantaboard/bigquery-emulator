#include "backend/catalog/routine_persistence.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_enums.pb.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

bool RoutineIdIsComplete(const storage::RoutineId& id) {
  return !id.project_id.empty() && !id.dataset_id.empty() &&
         !id.routine_id.empty();
}

storage::RoutineKind KindForCreateFunction(
    const ::googlesql::ResolvedCreateFunctionStmt& create_fn) {
  return create_fn.is_aggregate() ? storage::RoutineKind::kAggregateFunction
                                  : storage::RoutineKind::kScalarFunction;
}

storage::RoutineRecord BuildRecordFromStmt(
    const engine::QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt) {
  storage::RoutineRecord rec;
  rec.id.project_id = request.project_id;
  rec.id.dataset_id = request.default_dataset_id;
  rec.ddl_sql = request.sql;
  rec.language = "SQL";

  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT: {
      const auto* create_fn =
          stmt.GetAs<::googlesql::ResolvedCreateFunctionStmt>();
      rec.id = RoutineIdFromNamePath(create_fn->name_path(),
                                     request.project_id,
                                     request.default_dataset_id);
      rec.kind = KindForCreateFunction(*create_fn);
      rec.language =
          create_fn->language().empty() ? "SQL" : create_fn->language();
      rec.is_temp = create_fn->create_scope() ==
                    ::googlesql::ResolvedCreateStatementEnums::CREATE_TEMP;
      rec.signature_json = SerializeFunctionSignatureJson(*create_fn);
      break;
    }
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT: {
      const auto* create_tvf =
          stmt.GetAs<::googlesql::ResolvedCreateTableFunctionStmt>();
      rec.id = RoutineIdFromNamePath(create_tvf->name_path(),
                                     request.project_id,
                                     request.default_dataset_id);
      rec.kind = storage::RoutineKind::kTableValuedFunction;
      break;
    }
    case ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT: {
      const auto* create_proc =
          stmt.GetAs<::googlesql::ResolvedCreateProcedureStmt>();
      rec.id = RoutineIdFromNamePath(create_proc->name_path(),
                                     request.project_id,
                                     request.default_dataset_id);
      rec.kind = storage::RoutineKind::kProcedure;
      break;
    }
    default:
      break;
  }
  return rec;
}

}  // namespace

storage::RoutineId RoutineIdFromNamePath(
    const std::vector<std::string>& name_path,
    absl::string_view project_id,
    absl::string_view default_dataset_id) {
  storage::RoutineId id;
  switch (name_path.size()) {
    case 1:
      id.project_id = std::string(project_id);
      id.dataset_id = std::string(default_dataset_id);
      id.routine_id = name_path[0];
      break;
    case 2:
      id.project_id = std::string(project_id);
      id.dataset_id = name_path[0];
      id.routine_id = name_path[1];
      break;
    default:
      if (name_path.size() >= 3) {
        id.project_id = name_path[0];
        id.dataset_id = name_path[1];
        id.routine_id = name_path.back();
      }
      break;
  }
  return id;
}

std::string SerializeFunctionSignatureJson(
    const ::googlesql::ResolvedCreateFunctionStmt& create_fn) {
  // Minimal JSON: argument names in declaration order. ANY TYPE and
  // concrete types are recovered when the stored DDL is re-analyzed;
  // this blob exists for REST metadata round-trip.
  std::vector<std::string> quoted;
  quoted.reserve(create_fn.argument_name_list_size());
  for (const std::string& name : create_fn.argument_name_list()) {
    quoted.push_back(absl::StrCat("\"", name, "\""));
  }
  return absl::StrCat(
      "{\"argument_names\":[", absl::StrJoin(quoted, ","), "]}");
}

absl::Status PersistRoutineDdl(storage::Storage* storage,
                               const engine::QueryRequest& request,
                               const ::googlesql::ResolvedStatement& stmt) {
  if (storage == nullptr) return absl::OkStatus();
  storage::RoutineRecord rec = BuildRecordFromStmt(request, stmt);
  if (rec.is_temp || rec.ddl_sql.empty()) return absl::OkStatus();
  // Unqualified routines without a default dataset cannot be keyed in
  // storage; registration still succeeded in the in-memory registry.
  if (!RoutineIdIsComplete(rec.id)) return absl::OkStatus();
  return storage->UpsertRoutine(rec);
}

absl::Status DeletePersistedRoutine(storage::Storage* storage,
                                    const storage::RoutineId& id) {
  if (storage == nullptr) return absl::OkStatus();
  absl::Status deleted = storage->DeleteRoutine(id);
  if (deleted.ok() || deleted.code() == absl::StatusCode::kNotFound) {
    return absl::OkStatus();
  }
  return deleted;
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
