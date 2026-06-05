#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

// Stable string representation of a `storage::Value` used as a
// primary-key lookup key when diffing the pre- and post-MERGE row
// sets.
std::string SerializeForPkLookup(const storage::Value& value) {
  using Kind = storage::Value::Kind;
  switch (value.kind()) {
    case Kind::kNull:
      return "n:";
    case Kind::kBool:
      return std::string(value.bool_value() ? "b:1" : "b:0");
    case Kind::kInt64:
      return absl::StrCat("i:", value.int64_value());
    case Kind::kFloat64:
      return absl::StrCat("f:", value.float64_value());
    case Kind::kString:
      return absl::StrCat("s:", value.string_value());
    case Kind::kBytes:
      return absl::StrCat("y:", value.string_value());
    case Kind::kArray: {
      std::string out = "a:[";
      for (const auto& e : value.array_value()) {
        absl::StrAppend(&out, SerializeForPkLookup(e), ",");
      }
      absl::StrAppend(&out, "]");
      return out;
    }
    case Kind::kStruct: {
      std::string out = "t:{";
      for (const auto& f : value.struct_value()) {
        absl::StrAppend(&out, SerializeForPkLookup(f), ",");
      }
      absl::StrAppend(&out, "}");
      return out;
    }
  }
  return "?:";
}

// Deep-equal comparison for two storage::Value cells. We do not
// reuse `SerializeForPkLookup` because it conflates a `Bool(true)`
// with a `String("true")` for PK collapsing purposes; the diff
// classification path needs an exact-shape compare so a MATCHED row
// whose only change is a string-to-bool coercion still surfaces
// as an UPDATE.
bool ValuesEqual(const storage::Value& a, const storage::Value& b) {
  if (a.kind() != b.kind()) return false;
  using Kind = storage::Value::Kind;
  switch (a.kind()) {
    case Kind::kNull:
      return true;
    case Kind::kBool:
      return a.bool_value() == b.bool_value();
    case Kind::kInt64:
      return a.int64_value() == b.int64_value();
    case Kind::kFloat64:
      return a.float64_value() == b.float64_value();
    case Kind::kString:
    case Kind::kBytes:
      return a.string_value() == b.string_value();
    case Kind::kArray: {
      const auto& av = a.array_value();
      const auto& bv = b.array_value();
      if (av.size() != bv.size()) return false;
      for (size_t i = 0; i < av.size(); ++i) {
        if (!ValuesEqual(av[i], bv[i])) return false;
      }
      return true;
    }
    case Kind::kStruct: {
      const auto& af = a.struct_value();
      const auto& bf = b.struct_value();
      if (af.size() != bf.size()) return false;
      for (size_t i = 0; i < af.size(); ++i) {
        if (!ValuesEqual(af[i], bf[i])) return false;
      }
      return true;
    }
  }
  return false;
}

// Drains every row out of a DuckDB result through the same chunked
// `arrow_to_bq::ChunkRowToCells` path the SELECT row source uses, so
// the cells the MERGE diff sees match what `ExecuteQuery` would
// surface for the same table. The result is destroyed in-place; the
// caller owns the lifetime up to the call.
absl::StatusOr<std::vector<storage::Row>> DrainResultToRows(
    ::duckdb_result* result, const schema::TableSchema& schema) {
  std::vector<storage::Row> rows;
  while (true) {
    ::duckdb_data_chunk chunk = ::duckdb_fetch_chunk(*result);
    if (chunk == nullptr) break;
    const ::idx_t n = ::duckdb_data_chunk_get_size(chunk);
    for (::idx_t i = 0; i < n; ++i) {
      absl::StatusOr<storage::Row> rendered =
          arrow_to_bq::ChunkRowToCells(chunk, i, schema);
      if (!rendered.ok()) {
        ::duckdb_destroy_data_chunk(&chunk);
        return rendered.status();
      }
      rows.push_back(std::move(rendered).value());
    }
    ::duckdb_destroy_data_chunk(&chunk);
  }
  return rows;
}

// Read every row out of the DuckDB-side table at `quoted_table_name`,
// converted onto the engine-agnostic `storage::Row` shape that
// matches `bq_schema`. Used to capture the post-MERGE state of the
// target table so we can diff against the pre-MERGE snapshot and
// classify each PK delta as inserted / updated / deleted.
absl::StatusOr<std::vector<storage::Row>> ReadBackTable(
    ::duckdb_connection conn,
    absl::string_view quoted_table_name,
    const schema::TableSchema& bq_schema) {
  const std::string sql = absl::StrCat("SELECT * FROM ", quoted_table_name);
  ::duckdb_result result;
  if (::duckdb_query(conn, sql.c_str(), &result) != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat(
        "DuckDBEngine: failed to read back target table ", sql, ": ", detail));
  }
  absl::StatusOr<std::vector<storage::Row>> rows =
      DrainResultToRows(&result, bq_schema);
  ::duckdb_destroy_result(&result);
  return rows;
}

// Classify the per-row delta between the pre-MERGE snapshot
// (`before`) and the post-MERGE state (`after`) of the same target
// table by walking both row sets keyed on the synthetic primary key
// (column 0, see `backend/catalog/storage_table.cc`'s constructor).
//
//   * PK present in `after` but not in `before` -> inserted
//   * PK present in both but row contents differ -> updated
//   * PK present in `before` but not in `after` -> deleted
//
// MATCHED-then-no-op rows (every column unchanged) do not count
// toward `updated_row_count`, mirroring BigQuery's behavior of only
// surfacing actually-modified rows in `dmlStats`.
DmlStats DiffByPrimaryKey(absl::Span<const storage::Row> before,
                          absl::Span<const storage::Row> after) {
  DmlStats stats;
  absl::flat_hash_map<std::string, const storage::Row*> before_by_pk;
  before_by_pk.reserve(before.size());
  for (const storage::Row& row : before) {
    if (row.cells.empty()) continue;
    before_by_pk[SerializeForPkLookup(row.cells.front())] = &row;
  }
  absl::flat_hash_map<std::string, const storage::Row*> after_by_pk;
  after_by_pk.reserve(after.size());
  for (const storage::Row& row : after) {
    if (row.cells.empty()) continue;
    after_by_pk[SerializeForPkLookup(row.cells.front())] = &row;
  }
  for (const auto& [pk, after_row] : after_by_pk) {
    auto it = before_by_pk.find(pk);
    if (it == before_by_pk.end()) {
      ++stats.inserted_row_count;
      continue;
    }
    const storage::Row* before_row = it->second;
    if (after_row->cells.size() != before_row->cells.size()) {
      ++stats.updated_row_count;
      continue;
    }
    bool changed = false;
    for (size_t c = 0; c < after_row->cells.size(); ++c) {
      if (!ValuesEqual(after_row->cells[c], before_row->cells[c])) {
        changed = true;
        break;
      }
    }
    if (changed) ++stats.updated_row_count;
  }
  for (const auto& [pk, _] : before_by_pk) {
    if (!after_by_pk.contains(pk)) ++stats.deleted_row_count;
  }
  return stats;
}
}  // namespace internal

absl::StatusOr<DmlStats> DuckDbExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis has already happened on the coordinator.
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDbExecutor::ExecuteDml: storage backend is not configured");
  }

  if (stmt.node_kind() != ::googlesql::RESOLVED_MERGE_STMT) {
    // DML ENGINE POLICY (see `docs/ENGINE_POLICY.md`): the DuckDB
    // engine only implements MERGE today. INSERT / UPDATE / DELETE
    // route to the local DML executor in
    // `backend/engine/semantic/dml/`; reaching this branch means the
    // route classifier dispatched the wrong executor (an internal bug,
    // not a user-facing UNIMPLEMENTED).
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: ExecuteDml only implements MERGE today (family: "
        "node:",
        stmt.node_kind_string(),
        ", route: semantic_executor); see docs/ENGINE_POLICY.md and plan "
        "docs/ENGINE_POLICY.md"));
  }
  const auto* merge_stmt = stmt.GetAs<::googlesql::ResolvedMergeStmt>();
  if (merge_stmt->table_scan() == nullptr ||
      merge_stmt->table_scan()->table() == nullptr) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: MERGE statement has no resolved "
        "target table scan");
  }

  // 2. Collect every referenced storage table. The target appears
  // once (from `table_scan()`); the source side (`from_scan()`) may
  // mention zero or more tables depending on the USING clause.
  internal::TableScanCollector collector;
  absl::Status visit_status = stmt.Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  const auto* target_table = dynamic_cast<const catalog::StorageTable*>(
      merge_stmt->table_scan()->table());
  if (target_table == nullptr) {
    return absl::FailedPreconditionError(
        absl::StrCat("DuckDbExecutor::ExecuteDml: MERGE target '",
                     merge_stmt->table_scan()->table()->FullName(),
                     "' is not backed by a StorageTable; cannot apply DML"));
  }
  const storage::TableId target_id = target_table->storage_table_id();

  // 3. Snapshot the target rows so we can diff post-MERGE to derive
  // per-branch DmlStats counts (DuckDB's MERGE returns a single
  // total via `duckdb_rows_changed`, but BigQuery's wire envelope
  // distinguishes insertedRowCount / updatedRowCount /
  // deletedRowCount).
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> before_iter =
      storage_->ScanRows(target_id);
  if (!before_iter.ok()) return before_iter.status();
  std::vector<storage::Row> before_rows;
  {
    std::unique_ptr<storage::RowIterator> iter = std::move(before_iter).value();
    storage::Row row;
    while (true) {
      absl::StatusOr<bool> has = iter->Next(&row);
      if (!has.ok()) return has.status();
      if (!*has) break;
      before_rows.push_back(row);
    }
  }

  // 4. Open a fresh in-memory DuckDB. Per-query lifetime matches the
  // SELECT path; the connection is torn down before we return.
  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: duckdb_connect failed");
  }
  // Same polyfill registration as the SELECT path: the MERGE
  // statement can carry BigQuery scalar / aggregate calls that
  // route through `duckdb_udf`, so the UDFs must be installed
  // before DuckDB sees the user-submitted SQL.
  if (auto reg = udf::RegisterAll(conn); !reg.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return reg;
  }

  // 5. Materialize each referenced storage table inside the DuckDB
  // connection under its schema-qualified `"dataset"."table"` name so
  // the user-submitted MERGE SQL (which typically writes
  // `MERGE INTO ds.people ...`) resolves end-to-end. The target
  // table's qualified name is captured so step 7 can read it back.
  std::string quoted_target;
  for (const ::googlesql::Table* tbl : collector.tables()) {
    const auto* storage_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (storage_table == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::FailedPreconditionError(absl::StrCat(
          "DuckDbExecutor::ExecuteDml: cannot attach non-StorageTable '",
          tbl->Name(),
          "' for MERGE; rebuild against a "
          "GoogleSqlCatalog-backed analyzer"));
    }
    const storage::TableId& id = storage_table->storage_table_id();
    const std::string create_schema = absl::StrCat(
        "CREATE SCHEMA IF NOT EXISTS ", internal::QuoteIdent(id.dataset_id));
    absl::Status schema_status = internal::RunSqlNoResult(conn, create_schema);
    if (!schema_status.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return schema_status;
    }
    const std::string qualified =
        absl::StrCat(internal::QuoteIdent(id.dataset_id),
                     ".",
                     internal::QuoteIdent(id.table_id));
    absl::Status attach = internal::AttachStorageTableAt(
        conn, storage_, *storage_table, qualified);
    if (!attach.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return attach;
    }
    if (id == target_id) quoted_target = qualified;
  }
  if (quoted_target.empty()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: target table was not in the resolved "
        "table-scan set");
  }

  // 6. Execute the MERGE. We pass the user-submitted SQL verbatim:
  // DuckDB v1.2+ supports `MERGE INTO ... WHEN MATCHED / WHEN NOT
  // MATCHED ...` with the same statement shape BigQuery exposes, so
  // for the simple cases the conformance harness will seed in plans
  // 40-42 we do not need a transpiler. Cases DuckDB rejects fold to
  // INTERNAL (rather than UNIMPLEMENTED) because the DuckDB engine
  // is the only path for MERGE today.
  ::duckdb_result merge_result;
  if (::duckdb_query(conn, request.sql.c_str(), &merge_result) !=
      ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&merge_result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&merge_result);
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine: DuckDB rejected MERGE: ",
                     detail,
                     " (sql=",
                     request.sql,
                     ")"));
  }
  ::duckdb_destroy_result(&merge_result);

  // 7. Read back the post-MERGE target rows so we can (a) ship them
  // back into the storage backend via `OverwriteRows` and (b)
  // classify the per-branch DmlStats counts by diffing against the
  // pre-MERGE snapshot.
  absl::StatusOr<std::vector<storage::Row>> after_rows =
      internal::ReadBackTable(conn, quoted_target, target_table->bq_schema());
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!after_rows.ok()) return after_rows.status();

  absl::Status applied =
      storage_->OverwriteRows(target_id, absl::MakeConstSpan(*after_rows));
  if (!applied.ok()) return applied;

  return internal::DiffByPrimaryKey(absl::MakeConstSpan(before_rows),
                                    absl::MakeConstSpan(*after_rows));
}

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
