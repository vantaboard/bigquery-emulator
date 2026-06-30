#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

using merge_internal::ApplyMergeWhenClauses;
using merge_internal::BuildMergeFinalRows;
using merge_internal::CheckMatchedTargetMultiMatch;
using merge_internal::CorrelateMergeTargetsAndSources;
using merge_internal::LoadMergeParticipants;

absl::StatusOr<DmlStats> ExecuteMerge(
    const ::googlesql::ResolvedMergeStmt& merge,
    storage::Storage& storage,
    EvalContext& ctx) {
  auto target_or = StorageTargetFor(merge, "MERGE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  auto target_by_id = BuildColumnIndexByColumnId(*merge.table_scan(), schema);
  if (!target_by_id.ok()) return target_by_id.status();

  auto participants_or =
      LoadMergeParticipants(merge, storage, schema, *target_by_id, target, ctx);
  if (!participants_or.ok()) return participants_or.status();

  CorrelateMergeTargetsAndSources(
      merge, participants_or->targets, participants_or->sources, ctx);

  absl::Status multi_match =
      CheckMatchedTargetMultiMatch(merge, participants_or->targets);
  if (!multi_match.ok()) return multi_match;

  DmlStats stats;
  std::vector<storage::Row> inserts;
  absl::Status applied = ApplyMergeWhenClauses(
      merge, schema, *participants_or, stats, inserts, ctx);
  if (!applied.ok()) return applied;

  std::vector<storage::Row> final_rows =
      BuildMergeFinalRows(participants_or->targets, inserts);

  absl::Status overwrote =
      // cpp-lint:allow(status-discarded) -- captured into overwrote
      storage.OverwriteRows(target->storage_table_id(), final_rows);
  if (!overwrote.ok()) return overwrote;
  return stats;
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
