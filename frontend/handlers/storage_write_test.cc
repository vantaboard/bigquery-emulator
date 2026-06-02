// Plan 15 (storage-read-write-api-plan): unit + in-process gRPC tests
// for `StorageWriteService`.
//
// The shape mirrors `storage_read_test.cc`: an in-memory DuckDB
// storage instance is fabricated under SetUp, the service is
// constructed against it, and each test pokes the handler with a
// hand-built request proto. The bidirectional `AppendRows` RPC needs
// a real `ServerReaderWriter`, so the streaming tests stand up an
// in-process gRPC server (same `grpc::testing` transport
// `storage_read_test.cc` uses for `ReadRows`) and drive the streaming
// reply through a client stub.

#include "frontend/handlers/storage_write.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/credentials.h"
#include "grpcpp/security/server_credentials.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"
#include "gtest/gtest.h"
#include "proto/emulator.pb.h"
#include "proto/storage_write.grpc.pb.h"
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

class StorageWriteServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    data_dir_ = MakeTempDataDir("storage-write-test");
    auto opened =
        backend::storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    service_ = std::make_unique<StorageWriteService>(storage_.get());
  }

  void TearDown() override {
    service_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  // Two-column toy schema (id + name). Mirrors the plan-15 happy
  // path; tests append rows against it and round-trip through
  // `Storage::ScanRows` to confirm the rows landed.
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

  fs::path data_dir_{};
  std::unique_ptr<backend::storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<StorageWriteService> service_{};
};

// ---------------------------------------------------------------------------
// CreateWriteStream: validation + happy path
// ---------------------------------------------------------------------------

TEST_F(StorageWriteServiceTest, CreateWriteStreamMintsCommittedStream) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/t");
  req.mutable_write_stream()->set_type(v1::WriteStream::COMMITTED);
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();

  // Stream id nests under the table path with a server-assigned
  // suffix; the helper mints `s1` for the first call.
  EXPECT_EQ(resp.name(),
            "projects/proj-test/datasets/ds/tables/t/streams/s1");
  EXPECT_EQ(resp.type(), v1::WriteStream::COMMITTED);
  ASSERT_EQ(resp.schema().fields_size(), 2);
  EXPECT_EQ(resp.schema().fields(0).name(), "id");
  EXPECT_EQ(resp.schema().fields(1).name(), "name");
  EXPECT_FALSE(resp.create_time().empty());
  EXPECT_EQ(service_->StreamsForTesting(), 1u);
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamDefaultsToCommitted) {
  // BigQuery's documented default for an unspecified stream type is
  // COMMITTED. Plan 15 follows that.
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/t");
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(resp.type(), v1::WriteStream::COMMITTED);
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamRejectsPending) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/t");
  req.mutable_write_stream()->set_type(v1::WriteStream::PENDING);
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED)
      << status.error_message();
  EXPECT_EQ(service_->StreamsForTesting(), 0u);
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamRejectsBuffered) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/t");
  req.mutable_write_stream()->set_type(v1::WriteStream::BUFFERED);
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED)
      << status.error_message();
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamRejectsMalformedParent) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  // Drops the `tables/` segment; the parser refuses anything that
  // does not match `projects/{p}/datasets/{d}/tables/{t}`.
  req.set_parent("projects/proj-test/datasets/ds/t");
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT);
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamMissingTableIsNotFound) {
  // Dataset exists but the table does not. Storage::GetSchema returns
  // NotFound and the handler maps that onto gRPC NOT_FOUND so a
  // BigQuery REST 404 envelope can be synthesized at the gateway.
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/missing");
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

// ---------------------------------------------------------------------------
// Deferred RPCs return UNIMPLEMENTED (plan 15: BUFFERED/PENDING land
// in the deferred follow-up subagent).
// ---------------------------------------------------------------------------

TEST_F(StorageWriteServiceTest, FinalizeWriteStreamUnimplemented) {
  v1::FinalizeWriteStreamRequest req;
  req.set_name("projects/p/datasets/d/tables/t/streams/s1");
  v1::FinalizeWriteStreamResponse resp;
  ::grpc::Status status = service_->FinalizeWriteStream(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED);
}

TEST_F(StorageWriteServiceTest, BatchCommitWriteStreamsUnimplemented) {
  v1::BatchCommitWriteStreamsRequest req;
  req.set_parent("projects/p/datasets/d/tables/t");
  v1::BatchCommitWriteStreamsResponse resp;
  ::grpc::Status status =
      service_->BatchCommitWriteStreams(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED);
}

TEST_F(StorageWriteServiceTest, FlushRowsUnimplemented) {
  v1::FlushRowsRequest req;
  req.set_write_stream("projects/p/datasets/d/tables/t/streams/s1");
  v1::FlushRowsResponse resp;
  ::grpc::Status status = service_->FlushRows(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED);
}

// ---------------------------------------------------------------------------
// AppendRows: in-process gRPC fixture so the bidi-streaming RPC can
// run end-to-end without inventing a ServerReaderWriter mock.
// ---------------------------------------------------------------------------

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

  // Builds an `AppendRowsRequest` for the people table where
  // each row is `(id, "person-id")`. Mirrors the AppendPeople
  // helper in storage_read_test.cc so the two suites can share
  // assertions if they want.
  v1::AppendRowsRequest BuildAppendRequest(const std::string& stream_name,
                                           int64_t first_id,
                                           int64_t count) {
    v1::AppendRowsRequest req;
    req.set_write_stream(stream_name);
    auto* proto_rows = req.mutable_proto_rows();
    for (int64_t i = 0; i < count; ++i) {
      auto* row = proto_rows->add_rows();
      row->add_cells()->set_string_value(absl::StrCat(first_id + i));
      row->add_cells()->set_string_value(
          absl::StrCat("person-", first_id + i));
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

  // First batch: two rows.
  v1::AppendRowsRequest req1 = BuildAppendRequest(stream.name(), 0, 2);
  req1.set_trace_id("trace-1");
  ASSERT_TRUE(stream_rw->Write(req1));

  v1::AppendRowsResponse resp1;
  ASSERT_TRUE(stream_rw->Read(&resp1));
  ASSERT_EQ(resp1.response_case(), v1::AppendRowsResponse::kAppendResult);
  EXPECT_EQ(resp1.append_result().offset(), 0);
  EXPECT_EQ(resp1.row_count(), 2);
  EXPECT_EQ(resp1.trace_id(), "trace-1");

  // Second batch: three rows; offset bumps by the prior commit count.
  // Subsequent messages may leave write_stream empty (the binding
  // sticks); the test exercises that contract explicitly.
  v1::AppendRowsRequest req2 = BuildAppendRequest("", 2, 3);
  ASSERT_TRUE(stream_rw->Write(req2));
  v1::AppendRowsResponse resp2;
  ASSERT_TRUE(stream_rw->Read(&resp2));
  ASSERT_EQ(resp2.response_case(), v1::AppendRowsResponse::kAppendResult);
  EXPECT_EQ(resp2.append_result().offset(), 2);
  EXPECT_EQ(resp2.row_count(), 3);

  ASSERT_TRUE(stream_rw->WritesDone());
  ::grpc::Status status = stream_rw->Finish();
  ASSERT_TRUE(status.ok()) << status.error_message();

  // Round-trip through Storage::ScanRows to confirm the rows landed.
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
  for (size_t i = 0; i < rows.size(); ++i) {
    EXPECT_EQ(rows[i].cells[0].int64_value(), static_cast<int64_t>(i));
    EXPECT_EQ(rows[i].cells[1].string_value(),
              absl::StrCat("person-", i));
  }
}

TEST_F(StorageWriteGrpcTest, AppendRowsToDefaultStreamMintsLazily) {
  CreatePeopleTable();
  // Plan 15: AppendRows against the reserved `_default` stream id
  // does not require a prior CreateWriteStream call. The handler
  // mints the StreamState lazily on the first request.
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
  ::grpc::Status status = stream_rw->Finish();
  ASSERT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(service_->StreamsForTesting(), 1u);
}

TEST_F(StorageWriteGrpcTest, AppendRowsShapeMismatchSurfaceOnEnvelope) {
  // Plan 15: a recoverable shape error (cell count != table column
  // count) lands on the AppendRowsResponse.error_message envelope
  // without tearing the stream down. The producer can fix the
  // shape and retry on the next message.
  CreatePeopleTable();
  v1::CreateWriteStreamRequest create_req;
  create_req.set_parent("projects/proj-test/datasets/ds/tables/t");
  v1::WriteStream stream;
  ::grpc::ClientContext create_ctx;
  ASSERT_TRUE(stub_->CreateWriteStream(&create_ctx, create_req, &stream).ok());

  ::grpc::ClientContext append_ctx;
  auto stream_rw = stub_->AppendRows(&append_ctx);
  ASSERT_NE(stream_rw, nullptr);

  // Bad batch: only one cell where two are required.
  v1::AppendRowsRequest bad;
  bad.set_write_stream(stream.name());
  bad.mutable_proto_rows()->add_rows()->add_cells()->set_string_value("0");
  ASSERT_TRUE(stream_rw->Write(bad));
  v1::AppendRowsResponse resp_bad;
  ASSERT_TRUE(stream_rw->Read(&resp_bad));
  ASSERT_EQ(resp_bad.response_case(),
            v1::AppendRowsResponse::kErrorMessage);
  EXPECT_FALSE(resp_bad.error_message().empty());

  // Recoverable: stream still alive; a correct batch lands.
  v1::AppendRowsRequest good = BuildAppendRequest("", 0, 1);
  ASSERT_TRUE(stream_rw->Write(good));
  v1::AppendRowsResponse resp_good;
  ASSERT_TRUE(stream_rw->Read(&resp_good));
  EXPECT_EQ(resp_good.response_case(),
            v1::AppendRowsResponse::kAppendResult);

  ASSERT_TRUE(stream_rw->WritesDone());
  ::grpc::Status status = stream_rw->Finish();
  ASSERT_TRUE(status.ok()) << status.error_message();
}

TEST_F(StorageWriteGrpcTest, AppendRowsUnknownStreamIsNotFound) {
  // Plan 15: a write to an unminted explicit stream id fails with
  // NOT_FOUND so a producer that mistyped the name sees a clear
  // error rather than silently routing to the default stream.
  CreatePeopleTable();
  ::grpc::ClientContext append_ctx;
  auto stream_rw = stub_->AppendRows(&append_ctx);
  ASSERT_NE(stream_rw, nullptr);
  v1::AppendRowsRequest req = BuildAppendRequest(
      "projects/proj-test/datasets/ds/tables/t/streams/s99", 0, 1);
  ASSERT_TRUE(stream_rw->Write(req));
  v1::AppendRowsResponse resp;
  // The stream is unknown; the handler returns NOT_FOUND on the
  // RPC. The reader's first Read() call may surface the close so
  // we tolerate either path.
  while (stream_rw->Read(&resp)) {}
  ::grpc::Status status = stream_rw->Finish();
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND)
      << status.error_message();
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
  EXPECT_EQ(got.create_time(), stream.create_time());
}

TEST_F(StorageWriteGrpcTest, GetWriteStreamUnknownIsNotFound) {
  ::grpc::ClientContext get_ctx;
  v1::GetWriteStreamRequest get_req;
  get_req.set_name("projects/proj-test/datasets/ds/tables/t/streams/ghost");
  v1::WriteStream got;
  ::grpc::Status status = stub_->GetWriteStream(&get_ctx, get_req, &got);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
