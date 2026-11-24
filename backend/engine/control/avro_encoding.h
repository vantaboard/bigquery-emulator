#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_ENCODING_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_ENCODING_H_

// Avro schema rendering and binary datum encoding for the Avro writer
// behind `EXPORT DATA` (see `avro_writer.h` for the container format).

#include <cstdint>
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

/**
 * Renders the Avro schema JSON for an export of `columns`. Returns
 * INVALID_ARGUMENT for column types Avro export does not support.
 */
absl::StatusOr<std::string> BuildAvroSchemaJson(
    const schema::TableSchema& columns, bool use_logical_types);

/** Appends `v` as an Avro zig-zag varint `long`. */
void AppendAvroLong(std::string* out, int64_t v);

/** Appends `bytes` as an Avro `bytes` / `string` datum. */
void AppendAvroBytes(std::string* out, absl::string_view bytes);

/**
 * Appends `value` encoded against the Avro type that
 * `BuildAvroSchemaJson` renders for `column`.
 */
absl::Status AppendAvroField(std::string* out,
                             const schema::ColumnSchema& column,
                             const storage::Value& value,
                             bool use_logical_types);

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_ENCODING_H_
