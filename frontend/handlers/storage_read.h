#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_H_

#include <grpcpp/grpcpp.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/storage.h"
#include "proto/storage_read.grpc.pb.h"
#include "proto/storage_read.pb.h"

namespace bigquery_emulator {
namespace frontend {

class StorageReadService final : public v1::StorageRead::Service {
 public:
  explicit StorageReadService(backend::storage::Storage* storage);

  ::grpc::Status CreateReadSession(::grpc::ServerContext* context,
                                   const v1::CreateReadSessionRequest* request,
                                   v1::ReadSession* response) override;

  ::grpc::Status ReadRows(
      ::grpc::ServerContext* context,
      const v1::ReadRowsRequest* request,
      ::grpc::ServerWriter<v1::ReadRowsResponse>* writer) override;

  ::grpc::Status SplitReadStream(::grpc::ServerContext* context,
                                 const v1::SplitReadStreamRequest* request,
                                 v1::SplitReadStreamResponse* response) override;

  static constexpr int kReadRowsBatchSize = 100;

  std::size_t SessionsForTesting() const;

 private:
  struct StreamPartition {
    std::string name;
    std::int64_t row_start = 0;
    std::int64_t row_end = -1;
  };

  struct SessionState {
    backend::storage::TableId table;
    backend::schema::TableSchema schema;
    std::optional<std::string> where_sql;
    std::vector<std::string> selected_fields;
    std::int64_t total_rows = 0;
    std::map<std::string, StreamPartition> streams;
    std::int64_t next_split_id = 0;
  };

  ::grpc::Status ParseParent(const std::string& parent,
                             std::string* project_id) const;

  ::grpc::Status ParseTablePath(const std::string& table_path,
                                backend::storage::TableId* out) const;

  std::string NewSessionId(const std::string& project_id)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  std::string StreamIdForSession(const std::string& session_name,
                                 std::int64_t stream_index) const;

  ::grpc::Status ParseReadStreamName(const std::string& read_stream,
                                     std::string* session_name,
                                     std::string* stream_key) const;

  absl::StatusOr<std::int64_t> CountTableRows(
      const backend::storage::TableId& table) const;

  backend::storage::Storage* storage_;
  mutable absl::Mutex mu_;
  std::int64_t next_session_id_ ABSL_GUARDED_BY(mu_) = 1;
  std::map<std::string, SessionState> sessions_ ABSL_GUARDED_BY(mu_){};
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_H_
