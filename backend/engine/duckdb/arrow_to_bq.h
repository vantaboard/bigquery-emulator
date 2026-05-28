#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_ARROW_TO_BQ_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_ARROW_TO_BQ_H_

// arrow_to_bq lowers the columnar result format DuckDB hands back from
// `duckdb_fetch_chunk` into the engine-agnostic `backend::storage::Value`
// cells the rest of the emulator already speaks. A DuckDB `data_chunk`
// is the C-API equivalent of an Arrow `RecordBatch` (DuckDB ships a
// `duckdb_data_chunk_to_arrow` adapter on top of the same backing
// vectors), so this header is the "Arrow batch -> BigQuery cell"
// boundary the DuckDB engine relies on.
//
// The cell shape produced here matches what the DuckDB engine
// surfaces through `ValueToCell`, so the gateway's
// per-engine `frontend/handlers/query.cc::ValueToCell` -> proto Cell
// path renders the same wire JSON regardless of which engine ran the
// query. See `gateway/bqtypes/wire.go::ValueToCell` for the matching
// downstream conversion.
//
// The translation is schema-driven: each cell is rendered against the
// BigQuery-typed `ColumnSchema` from the analyzer's
// `ResolvedQueryStmt::output_column_list`. The DuckDB vector type may
// differ (e.g. an INT32 vector for a column the analyzer typed INT64
// because of an implicit cast), so we cross-check both sides and lean
// on the BigQuery schema when DuckDB's nominal type does not fully
// pin the wire encoding.

#include <cstdint>

#include "absl/status/statusor.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace arrow_to_bq {

// Reads a single cell out of `vector` at the given `row` index and
// renders it as a `storage::Value` shaped for the BigQuery column
// described by `column`. ARRAY (LIST) and STRUCT vectors recurse into
// their child vectors so nested BigQuery cells round-trip into nested
// `storage::Value::Array` / `storage::Value::Struct` containers.
//
// The function uses the vector's validity mask to detect NULL cells
// and never touches the underlying data pointer for a NULL row. Any
// cell whose vector type does not match `column.type` falls back to
// the vector's textual rendering via the duckdb_value-friendly
// helpers; this is the fallback path for types the engine has not
// yet specialized (`GoogleSqlValueToStorageValue`
// returns `Value::String(value.DebugString())`).
absl::StatusOr<storage::Value> ReadCellFromVector(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column);

// Reads one full row from a fetched `chunk`. The output row's cell
// count and per-cell type follow `output_schema`. The chunk's column
// count must match `output_schema.columns.size()`; otherwise the
// function returns INVALID_ARGUMENT to surface an analyzer / engine
// drift instead of silently mis-rendering rows.
absl::StatusOr<storage::Row> ChunkRowToCells(
    ::duckdb_data_chunk chunk,
    ::idx_t row,
    const schema::TableSchema& output_schema);

}  // namespace arrow_to_bq
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_ARROW_TO_BQ_H_
