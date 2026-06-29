#include "backend/engine/duckdb/arrow_to_bq.h"

#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/arrow_to_bq_internal.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace arrow_to_bq {

absl::StatusOr<storage::Value> ReadCellFromVector(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column) {
  return internal::DispatchReadCellFromVector(vector, row, column);
}

absl::StatusOr<storage::Row> ChunkRowToCells(
    ::duckdb_data_chunk chunk,
    ::idx_t row,
    const schema::TableSchema& output_schema) {
  if (chunk == nullptr) {
    return absl::InvalidArgumentError("arrow_to_bq: chunk is null");
  }
  const ::idx_t ncols = ::duckdb_data_chunk_get_column_count(chunk);
  if (ncols != static_cast<::idx_t>(output_schema.columns.size())) {
    return absl::InvalidArgumentError(
        absl::StrCat("arrow_to_bq: chunk has ",
                     ncols,
                     " columns but analyzer output schema has ",
                     output_schema.columns.size()));
  }
  storage::Row out;
  out.cells.reserve(ncols);
  for (::idx_t c = 0; c < ncols; ++c) {
    ::duckdb_vector vector = ::duckdb_data_chunk_get_vector(chunk, c);
    auto cell = ReadCellFromVector(vector, row, output_schema.columns[c]);
    if (!cell.ok()) return cell.status();
    out.cells.push_back(std::move(cell).value());
  }
  return out;
}

}  // namespace arrow_to_bq
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
