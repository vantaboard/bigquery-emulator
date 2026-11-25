#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_WRITER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_WRITER_H_

// Avro Object Container File writer behind `EXPORT DATA ... format =
// 'AVRO'`. DuckDB has no Avro writer, so the export runs the SELECT and
// streams the rendered rows through this writer instead of `COPY TO`.
//
// The schema follows BigQuery's Avro export shape (see "Avro export
// details" in docs/bigquery/docs/exporting-data.md): a top-level record
// named `Root`, NULLABLE fields as `["null", T]` unions, REPEATED fields
// as non-nullable arrays, STRUCTs as nested records named after the
// field (namespace `root[.<parent>...]`), NUMERIC / BIGNUMERIC as the
// `decimal` logical type, and DATE / TIME / DATETIME / TIMESTAMP as
// logical types unless `use_avro_logical_types = false`.

#include <cstdint>
#include <fstream>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

enum class AvroCodec { kNull, kDeflate, kSnappy };

/**
 * Maps the EXPORT DATA `compression` option onto an Avro codec. An
 * empty value means no compression. GZIP and unknown values return
 * INVALID_ARGUMENT, as BigQuery only supports DEFLATE and SNAPPY for
 * Avro.
 */
absl::StatusOr<AvroCodec> ParseAvroCompression(absl::string_view compression);

class AvroFileWriter {
 public:
  struct Options {
    AvroCodec codec = AvroCodec::kNull;
    bool use_logical_types = true;
  };

  /**
   * Creates `path` and writes the container header. Rows are buffered
   * into blocks and flushed as they fill; `Finish` flushes the last
   * block and closes the file.
   */
  static absl::StatusOr<std::unique_ptr<AvroFileWriter>> Create(
      const std::string& path,
      const schema::TableSchema& columns,
      Options options);

  absl::Status Write(const storage::Row& row);
  absl::Status Finish();

 private:
  AvroFileWriter(schema::TableSchema columns, Options options);

  absl::Status FlushBlock();

  schema::TableSchema columns_;
  Options options_;
  std::ofstream out_;
  std::string sync_;
  std::string block_;
  int64_t block_rows_ = 0;
};

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_WRITER_H_
