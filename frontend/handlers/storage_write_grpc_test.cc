// In-process gRPC tests for `StorageWriteService` streaming RPCs.

#include <grpcpp/support/status.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/credentials.h"
#include "grpcpp/security/server_credentials.h"
#include "grpcpp/server_builder.h"
#include "gtest/gtest.h"
#include "proto/storage_write.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

namespace fs = std::filesystem;

fs::path MakeTempDataDir(absl::string_view prefix) {
  const char* tmpdir_env = std::getenv("TMPDIR");
  const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
  std::random_device rd;
  std::seed_seq seed{rd(), rd()};
  std::mt19937_64 rng(seed);
  fs::path out = fs::path(tmpdir) /
                 absl::StrCat("bqemu-", std::string(prefix), "-", rng());
  std::error_code ec;
  fs::remove_all(out, ec);
  return out;
}

class StorageWriteGrpcTest : public ::testing::Test {
 protected:
  void SetUp() override {
    data_dir_ = MakeTempDataDir("storage-write-grpc-test");
    auto opened =
        backend::storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    service_ = std::make_unique<StorageWriteService>(storage_.get());

    int bound_port = 0;
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(
        "127.0.0.1:0", ::grpc::InsecureServerCredentials(), &bound_port);
    builder.RegisterService(service_.get());
    server_ = builder.BuildAndStart();
    ASSERT_NE(server_, nullptr);
    ASSERT_NE(bound_port, 0);

    const std::string target = absl::StrCat("127.0.0.1:", bound_port);
    channel_ =
        ::grpc::CreateChannel(target, ::grpc::InsecureChannelCredentials());
    ASSERT_NE(channel_, nullptr);
    stub_ = v1::StorageWrite::NewStub(channel_);
    ASSERT_NE(stub_, nullptr);
  }

  void TearDown() override {
    if (server_ != nullptr) server_->Shutdown();
    stub_.reset();
    channel_.reset();
    server_.reset();
    service_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  void CreatePeopleTable() {
    backend::schema::TableSchema schema;
    backend::schema::ColumnSchema id;
    id.name = "id";
    id.type = backend::schema::ColumnType::kInt64;
    id.mode = backend::schema::ColumnMode::kRequired;
    schema.columns.push_back(id);
    backend::schema::ColumnSchema name;
    name.name = "name";
    name.type = backend::schema::ColumnType::kString;
    name.mode = backend::schema::ColumnMode::kNullable;
    schema.columns.push_back(name);
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
    ASSERT_TRUE(storage_->CreateTable({"proj-test", "ds", "t"}, schema).ok());
  }

  v1::AppendRowsRequest BuildAppendRequest(const std::string& stream_name,
                                           int64_t first_id,
                                           int64_t count) {
    v1::AppendRowsRequest req;
    req.set_write_stream(stream_name);
    auto* proto_rows = req.mutable_proto_rows();
    for (int64_t i = 0; i < count; ++i) {
      auto* row = proto_rows->add_rows();
      row->add_cells()->set_string_value(absl::StrCat(first_id + i));
      row->add_cells()->set_string_value(absl::StrCat("person-", first_id + i));
    }
    return req;
  }

  fs::path data_dir_{};
  std::unique_ptr<backend::storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<StorageWriteService> service_{};
  std::unique_ptr<::grpc::Server> server_{};
  std::shared_ptr<::grpc::Channel> channel_{};
  std::unique_ptr<v1::StorageWrite::Stub> stub_{};
};

TEST_F(StorageWriteGrpcTest, AppendRowsCommitsToCommittedStream) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest create_req;
  create_req.set_parent("projects/proj-test/datasets/ds/tables/t");
  create_req.mutable_write_stream()->set_type(v1::WriteStream::COMMITTED);
  v1::WriteStream stream;
  ::grpc::ClientContext create_ctx;
  ASSERT_TRUE(stub_->CreateWriteStream(&create_ctx, create_req, &stream).ok());

  ::grpc::ClientContext append_ctx;
  auto stream_rw = stub_->AppendRows(&append_ctx);
  ASSERT_NE(stream_rw, nullptr);

  v1::AppendRowsRequest req1 = BuildAppendRequest(stream.name(), 0, 2);
  req1.set_trace_id("trace-1");
  ASSERT_TRUE(stream_rw->Write(req1));

  v1::AppendRowsResponse resp1;
  ASSERT_TRUE(stream_rw->Read(&resp1));
  ASSERT_EQ(resp1.response_case(), v1::AppendRowsResponse::kAppendResult);
  EXPECT_EQ(resp1.append_result().offset(), 0);
  EXPECT_EQ(resp1.row_count(), 2);
  EXPECT_EQ(resp1.trace_id(), "trace-1");

  v1::AppendRowsRequest req2 = BuildAppendRequest("", 2, 3);
  ASSERT_TRUE(stream_rw->Write(req2));
  v1::AppendRowsResponse resp2;
  ASSERT_TRUE(stream_rw->Read(&resp2));
  ASSERT_EQ(resp2.response_case(), v1::AppendRowsResponse::kAppendResult);
  EXPECT_EQ(resp2.append_result().offset(), 2);
  EXPECT_EQ(resp2.row_count(), 3);

  ASSERT_TRUE(stream_rw->WritesDone());
  ASSERT_TRUE(stream_rw->Finish().ok());

  auto iter_or = storage_->ScanRows({"proj-test", "ds", "t"});
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::unique_ptr<backend::storage::RowIterator> iter = std::move(*iter_or);
  std::vector<backend::storage::Row> rows;
  backend::storage::Row row;
  while (true) {
    auto has = iter->Next(&row);
    ASSERT_TRUE(has.ok());
    if (!*has) break;
    rows.push_back(row);
  }
  ASSERT_EQ(rows.size(), 5u);
}

TEST_F(StorageWriteGrpcTest, AppendRowsToDefaultStreamMintsLazily) {
  CreatePeopleTable();
  ::grpc::ClientContext append_ctx;
  auto stream_rw = stub_->AppendRows(&append_ctx);
  ASSERT_NE(stream_rw, nullptr);
  const std::string default_stream =
      "projects/proj-test/datasets/ds/tables/t/streams/_default";
  v1::AppendRowsRequest req = BuildAppendRequest(default_stream, 0, 4);
  ASSERT_TRUE(stream_rw->Write(req));
  v1::AppendRowsResponse resp;
  ASSERT_TRUE(stream_rw->Read(&resp));
  ASSERT_EQ(resp.response_case(), v1::AppendRowsResponse::kAppendResult);
  EXPECT_EQ(resp.row_count(), 4);
  ASSERT_TRUE(stream_rw->WritesDone());
  ASSERT_TRUE(stream_rw->Finish().ok());
  EXPECT_EQ(service_->StreamsForTesting(), 1u);
}

TEST_F(StorageWriteGrpcTest, AppendRowsShapeMismatchSurfaceOnEnvelope) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest create_req;
  create_req.set_parent("projects/proj-test/datasets/ds/tables/t");
  v1::WriteStream stream;
  ::grpc::ClientContext create_ctx;
  ASSERT_TRUE(stub_->CreateWriteStream(&create_ctx, create_req, &stream).ok());

  ::grpc::ClientContext append_ctx;
  auto stream_rw = stub_->AppendRows(&append_ctx);
  ASSERT_NE(stream_rw, nullptr);

  v1::AppendRowsRequest bad;
  bad.set_write_stream(stream.name());
  bad.mutable_proto_rows()->add_rows()->add_cells()->set_string_value("0");
  ASSERT_TRUE(stream_rw->Write(bad));
  v1::AppendRowsResponse resp_bad;
  ASSERT_TRUE(stream_rw->Read(&resp_bad));
  ASSERT_EQ(resp_bad.response_case(), v1::AppendRowsResponse::kErrorMessage);

  v1::AppendRowsRequest good = BuildAppendRequest("", 0, 1);
  ASSERT_TRUE(stream_rw->Write(good));
  v1::AppendRowsResponse resp_good;
  ASSERT_TRUE(stream_rw->Read(&resp_good));
  EXPECT_EQ(resp_good.response_case(), v1::AppendRowsResponse::kAppendResult);

  ASSERT_TRUE(stream_rw->WritesDone());
  ASSERT_TRUE(stream_rw->Finish().ok());
}

TEST_F(StorageWriteGrpcTest, AppendRowsUnknownStreamIsNotFound) {
  CreatePeopleTable();
  ::grpc::ClientContext append_ctx;
  auto stream_rw = stub_->AppendRows(&append_ctx);
  ASSERT_NE(stream_rw, nullptr);
  v1::AppendRowsRequest req = BuildAppendRequest(
      "projects/proj-test/datasets/ds/tables/t/streams/s99", 0, 1);
  ASSERT_TRUE(stream_rw->Write(req));
  v1::AppendRowsResponse resp;
  while (stream_rw->Read(&resp)) {}
  ::grpc::Status status = stream_rw->Finish();
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

TEST_F(StorageWriteGrpcTest, GetWriteStreamReturnsRecordedMetadata) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest create_req;
  create_req.set_parent("projects/proj-test/datasets/ds/tables/t");
  v1::WriteStream stream;
  ::grpc::ClientContext create_ctx;
  ASSERT_TRUE(stub_->CreateWriteStream(&create_ctx, create_req, &stream).ok());

  ::grpc::ClientContext get_ctx;
  v1::GetWriteStreamRequest get_req;
  get_req.set_name(stream.name());
  v1::WriteStream got;
  ASSERT_TRUE(stub_->GetWriteStream(&get_ctx, get_req, &got).ok());
  EXPECT_EQ(got.name(), stream.name());
  EXPECT_EQ(got.type(), v1::WriteStream::COMMITTED);
}

TEST_F(StorageWriteGrpcTest, GetWriteStreamUnknownIsNotFound) {
  ::grpc::ClientContext get_ctx;
  v1::GetWriteStreamRequest get_req;
  get_req.set_name("projects/proj-test/datasets/ds/tables/t/streams/ghost");
  v1::WriteStream got;
  ::grpc::Status status = stub_->GetWriteStream(&get_ctx, get_req, &got);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

TEST_F(StorageWriteGrpcTest, BufferedStreamFlushCommitsRows) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest create_req;
  create_req.set_parent("projects/proj-test/datasets/ds/tables/t");
  create_req.mutable_write_stream()->set_type(v1::WriteStream::BUFFERED);
  v1::WriteStream stream;
  ::grpc::ClientContext create_ctx;
  ASSERT_TRUE(stub_->CreateWriteStream(&create_ctx, create_req, &stream).ok());

  ::grpc::ClientContext append_ctx;
  auto stream_rw = stub_->AppendRows(&append_ctx);
  ASSERT_NE(stream_rw, nullptr);
  ASSERT_TRUE(stream_rw->Write(BuildAppendRequest(stream.name(), 0, 2)));
  v1::AppendRowsResponse resp1;
  ASSERT_TRUE(stream_rw->Read(&resp1));
  ASSERT_EQ(resp1.response_case(), v1::AppendRowsResponse::kAppendResult);
  ASSERT_TRUE(stream_rw->Write(BuildAppendRequest("", 2, 3)));
  v1::AppendRowsResponse resp2;
  ASSERT_TRUE(stream_rw->Read(&resp2));
  ASSERT_EQ(resp2.response_case(), v1::AppendRowsResponse::kAppendResult);
  ASSERT_TRUE(stream_rw->WritesDone());
  ASSERT_TRUE(stream_rw->Finish().ok());

  auto pre_flush = storage_->ScanRows({"proj-test", "ds", "t"});
  ASSERT_TRUE(pre_flush.ok());
  std::unique_ptr<backend::storage::RowIterator> pre_iter =
      std::move(*pre_flush);
  backend::storage::Row row;
  auto has = pre_iter->Next(&row);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);

  v1::FlushRowsRequest flush_req;
  flush_req.set_write_stream(stream.name());
  flush_req.set_offset(4);
  v1::FlushRowsResponse flush_resp;
  ::grpc::ClientContext flush_ctx;
  ASSERT_TRUE(stub_->FlushRows(&flush_ctx, flush_req, &flush_resp).ok());

  v1::FinalizeWriteStreamRequest finalize_req;
  finalize_req.set_name(stream.name());
  v1::FinalizeWriteStreamResponse finalize_resp;
  ::grpc::ClientContext finalize_ctx;
  ASSERT_TRUE(
      stub_->FinalizeWriteStream(&finalize_ctx, finalize_req, &finalize_resp)
          .ok());
  EXPECT_EQ(finalize_resp.row_count(), 5);

  auto iter_or = storage_->ScanRows({"proj-test", "ds", "t"});
  ASSERT_TRUE(iter_or.ok());
  std::unique_ptr<backend::storage::RowIterator> iter = std::move(*iter_or);
  std::size_t count = 0;
  while (true) {
    auto next = iter->Next(&row);
    ASSERT_TRUE(next.ok());
    if (!*next) break;
    ++count;
  }
  EXPECT_EQ(count, 5u);
}

}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
