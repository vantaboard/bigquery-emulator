#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_SQLTOOLS_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_SQLTOOLS_H_

#include <grpcpp/grpcpp.h>

#include "backend/storage/storage.h"
#include "proto/emulator.grpc.pb.h"

namespace bigquery_emulator {
namespace frontend {

class SqlToolsService final : public v1::SqlTools::Service {
 public:
  explicit SqlToolsService(backend::storage::Storage* storage);

  ::grpc::Status Format(::grpc::ServerContext* context,
                        const v1::FormatSqlRequest* request,
                        v1::FormatSqlResponse* response) override;

  ::grpc::Status Parse(::grpc::ServerContext* context,
                       const v1::ParseSqlRequest* request,
                       v1::ParseSqlResponse* response) override;

  ::grpc::Status Tokenize(::grpc::ServerContext* context,
                          const v1::TokenizeSqlRequest* request,
                          v1::TokenizeSqlResponse* response) override;

  ::grpc::Status Complete(::grpc::ServerContext* context,
                          const v1::CompleteSqlRequest* request,
                          v1::CompleteSqlResponse* response) override;

 private:
  backend::storage::Storage* storage_;
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_SQLTOOLS_H_
