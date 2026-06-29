#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

absl::Status DuckDbExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  (void)storage_;
  // All catalog/storage DDL shapes route to ControlOpExecutor via
  // `node_dispositions.yaml`. If one reaches the DuckDB executor,
  // the classifier or YAML row drifted out of sync.
  return absl::UnimplementedError(
      absl::StrCat("duckdb engine: ExecuteDdl does not implement ",
                   stmt.node_kind_string(),
                   "; DDL is owned by ControlOpExecutor "
                   "(see backend/engine/control/control_op_executor.cc)"));
}

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
