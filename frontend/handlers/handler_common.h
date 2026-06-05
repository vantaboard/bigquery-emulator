#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_HANDLER_COMMON_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_HANDLER_COMMON_H_

#include "absl/status/status.h"
#include "backend/storage/storage.h"
#include "grpcpp/support/status.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

// Translates an `absl::Status` from the storage layer into the closest
// matching gRPC status the gateway / external clients can route on.
::grpc::Status AbslToGrpcStatus(const absl::Status& status);

// Marshals an engine-agnostic `backend::storage::Value` onto the
// `v1::Cell` oneof. Primitives land on `string_value` and NULLs on
// `null_value`, matching the BigQuery REST `f`/`v` wire shape used by
// catalog ListRows and StorageRead ReadRows.
void ValueToCell(const backend::storage::Value& value, v1::Cell* out);

// Inverse of `ValueToCell`. Lower-level `Cell` shapes from insert /
// append RPCs round-trip through the same conversion.
backend::storage::Value CellToValue(const v1::Cell& cell);

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_HANDLER_COMMON_H_
