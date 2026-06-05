#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_WRITE_INTERNAL_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_WRITE_INTERNAL_H_

#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/storage/storage.h"
#include "grpcpp/support/status.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

::grpc::Status AbslToGrpcStatus(const absl::Status& status);

backend::storage::Value CellToValue(const v1::Cell& cell);

bool SplitTablePath(absl::string_view path, backend::storage::TableId* out);

std::string TablePathFor(const backend::storage::TableId& id);

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_WRITE_INTERNAL_H_
