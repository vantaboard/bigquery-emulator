#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/semantic/dml/dml_executor.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

absl::StatusOr<ParameterBindings> BuildParameterBindings(
    const QueryRequest& request) {
  ParameterBindings bindings;
  bool any_positional = false;
  bool any_named = false;
  for (const QueryParameter& p : request.parameters) {
    auto value = ParseParameterValue(p.value_json, p.type_kind, p.type_json);
    if (!value.ok()) return value.status();
    if (IsPositionalParameterName(p.name)) {
      if (p.name.empty()) {
        bindings.by_position.push_back(*std::move(value));
      } else {
        const int idx = SyntheticPositionalParameterIndex(p.name);
        if (idx < 0) {
          return absl::InvalidArgumentError(absl::StrCat(
              "semantic/dml: invalid synthetic positional parameter '",
              p.name,
              "'"));
        }
        if (bindings.by_position.size() <= static_cast<size_t>(idx)) {
          bindings.by_position.resize(static_cast<size_t>(idx) + 1);
        }
        bindings.by_position[static_cast<size_t>(idx)] = *std::move(value);
      }
      any_positional = true;
    } else {
      bindings.by_name[absl::AsciiStrToLower(p.name)] = *std::move(value);
      any_named = true;
    }
  }
  if (any_positional && any_named) {
    return absl::InvalidArgumentError(
        "semantic/dml: request mixes named and positional parameters");
  }
  return bindings;
}

// Resolve `stmt`'s `table_scan()` -> `table()` to the matching
// `backend::catalog::StorageTable` (which carries the `TableId` /
// `bq_schema()` the storage layer indexes by). DML can only
// proceed against a `StorageTable`-backed analyzer table; bare
// `SimpleTable` instances (e.g. fixtures the analyzer's built-in
// catalog ships) raise FAILED_PRECONDITION so the gateway can
// surface a clean error. Returns the storage table pointer (not
// owned).
// Find the index of the column named `name` in `schema.columns`.
// Lookup is case-insensitive to match BigQuery column-name
// resolution semantics. Returns -1 when the column is absent.
int IndexOfColumn(const schema::TableSchema& schema, absl::string_view name) {
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (absl::EqualsIgnoreCase(schema.columns[i].name, name)) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

// Build a `column_id -> table_column_index` map for the columns
// the `ResolvedTableScan` exposes. The analyzer issues fresh
// `ResolvedColumn` ids per query; the engine resolves them by
// lining up the table-scan's `column_list` against the storage
// table's schema by NAME (case-insensitive) so column_id-to-cell
// mapping stays stable across DML statements that omit some
// columns from the projection. Columns in the table schema not
// referenced by the scan are simply absent from the map; callers
// fall back to NULL / pass-through on lookup misses.
absl::StatusOr<absl::flat_hash_map<int, int>> BuildColumnIndexByColumnId(
    const ::googlesql::ResolvedTableScan& scan,
    const schema::TableSchema& schema) {
  absl::flat_hash_map<int, int> by_id;
  by_id.reserve(scan.column_list_size());
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    const int idx = IndexOfColumn(schema, col.name());
    if (idx < 0) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: ResolvedTableScan column '",
                       col.name(),
                       "' not found in storage table schema"));
    }
    by_id[col.column_id()] = idx;
  }
  return by_id;
}

// Drain every row of `id` out of `storage` into a vector. Used by
// UPDATE / DELETE to materialize the pre-mutation snapshot before
// applying the predicate / SET list. The storage layer does not
// expose a streaming partial-mutate API today (`OverwriteRows`
// replaces the entire row vector), so DML reads everything,
// computes the post-mutation shape in memory, and writes it back
// in one shot. This matches the DuckDB executor's MERGE strategy.
absl::StatusOr<std::vector<storage::Row>> ScanAllRows(
    storage::Storage& storage, const storage::TableId& id) {
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter_or =
      storage.ScanRows(id);
  if (!iter_or.ok()) return iter_or.status();
  std::unique_ptr<storage::RowIterator> iter = std::move(iter_or).value();
  std::vector<storage::Row> rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  return rows;
}

// Convert one row of a `ResolvedTableScan` row stream onto the
// `ColumnBindings` shape `EvalExpr` consumes for `WHERE` /
// `SET = <expr>` evaluation. The analyzer's `column_list()` is
// the ordered set of columns the scan exposes (one
// `ResolvedColumn` per scan column); the storage row's cells line
// up with the table schema by index, which we resolved into
// `by_id` once outside the loop.
absl::StatusOr<ColumnBindings> BindRow(
    const storage::Row& row,
    const ::googlesql::ResolvedTableScan& scan,
    const absl::flat_hash_map<int, int>& by_id,
    const schema::TableSchema& schema) {
  ColumnBindings bindings;
  bindings.reserve(scan.column_list_size());
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    auto it = by_id.find(col.column_id());
    if (it == by_id.end()) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: column_id=",
                       col.column_id(),
                       " missing from storage column-index map"));
    }
    const int idx = it->second;
    if (idx < 0 || static_cast<size_t>(idx) >= row.cells.size()) {
      return absl::InternalError(absl::StrCat("semantic/dml: column index ",
                                              idx,
                                              " out of range for row with ",
                                              row.cells.size(),
                                              " cells"));
    }
    auto v = catalog::StorageValueToGoogleSqlValue(row.cells[idx], col.type());
    if (!v.ok()) return v.status();
    bindings[col.column_id()] = *std::move(v);
  }
  return bindings;
}

// INSERT VALUES helper: build one `storage::Row` from a
// `ResolvedInsertRow`'s `value_list()`. Each value evaluates
// against the same `EvalContext` (no per-row column bindings;
// VALUES expressions are scalar / parameter-bound). Columns of
// the destination table NOT named in `insert_column_list` are
// filled with NULL -- BigQuery's INSERT contract honors the
// table's NULLABLE / REQUIRED modes at the storage layer
// (REQUIRED columns rejected as NULL surface from
// `Storage::AppendRows`).
absl::StatusOr<storage::Row> BuildInsertRow(
    const ::googlesql::ResolvedInsertRow& row_node,
    const std::vector<int>& column_idx_for_insert_position,
    const schema::TableSchema& schema,
    EvalContext& ctx) {
  if (row_node.value_list_size() !=
      static_cast<int>(column_idx_for_insert_position.size())) {
    return absl::InternalError(
        absl::StrCat("semantic/dml: INSERT row has ",
                     row_node.value_list_size(),
                     " values but the column list named ",
                     column_idx_for_insert_position.size(),
                     " columns"));
  }
  storage::Row out;
  out.cells.assign(schema.columns.size(), storage::Value::Null());
  for (int i = 0; i < row_node.value_list_size(); ++i) {
    const ::googlesql::ResolvedDMLValue* dml = row_node.value_list(i);
    if (dml == nullptr || dml->value() == nullptr) {
      return absl::InternalError(
          "semantic/dml: ResolvedDMLValue carries a null inner expr");
    }
    auto v = EvalExpr(*dml->value(), ctx);
    if (!v.ok()) return v.status();
    auto cell = ToStorageValue(*v);
    if (!cell.ok()) return cell.status();
    out.cells[column_idx_for_insert_position[i]] = *std::move(cell);
  }
  return out;
}

// Reject DML shapes the executor does not yet handle. Surfaces
// `kNotImplemented` so the gateway envelope is the same as for
// any other planned-but-not-landed route. Specific deferred
// shapes: `RETURNING` clause (Family 7), `ASSERT_ROWS_MODIFIED`
// modifier (BigQuery-only, no DuckDB analog), generated columns
// (BigQuery doesn't expose CREATE TABLE generated columns
// today). `array_offset_column` is only set on UPDATE/DELETE
// when the target sits inside a `FROM t, UNNEST(t.arr)` shape,
// which Family 4 (correlated array scans, deferred from plan
// 8) is the right home for.
absl::Status RejectUnsupportedDmlFeatures(
    const ::googlesql::ResolvedReturningClause* returning,
    const ::googlesql::ResolvedAssertRowsModified* assert_rows,
    bool has_array_offset_column,
    int generated_column_count,
    absl::string_view kind) {
  if (returning != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: ",
                     kind,
                     " RETURNING clause is not yet supported; see "
                     "docs/ENGINE_POLICY.md"));
  }
  if (assert_rows != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: ",
                     kind,
                     " ASSERT_ROWS_MODIFIED modifier is not yet supported"));
  }
  if (has_array_offset_column) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat(
            "semantic/dml: ",
            kind,
            " WITH OFFSET requires correlated lateral evaluation owned by "
            "deferred work tracked in docs/ENGINE_POLICY.md"));
  }
  if (generated_column_count > 0) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: ",
                     kind,
                     " on tables with GENERATED columns is not yet supported"));
  }
  return absl::OkStatus();
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
