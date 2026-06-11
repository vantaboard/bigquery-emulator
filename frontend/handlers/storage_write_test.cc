// Unit + in-process gRPC tests for `StorageWriteService`.
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
#include "grpcpp/grpcpp.h"
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

  // Two-column toy schema (id + name). Mirrors the StorageWrite
  // happy path; tests append rows against it and round-trip through
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
  EXPECT_EQ(resp.name(), "projects/proj-test/datasets/ds/tables/t/streams/s1");
  EXPECT_EQ(resp.type(), v1::WriteStream::COMMITTED);
  ASSERT_EQ(resp.schema().fields_size(), 2);
  EXPECT_EQ(resp.schema().fields(0).name(), "id");
  EXPECT_EQ(resp.schema().fields(1).name(), "name");
  EXPECT_FALSE(resp.create_time().empty());
  EXPECT_EQ(service_->StreamsForTesting(), 1u);
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamDefaultsToCommitted) {
  // BigQuery's documented default for an unspecified stream type is
  // COMMITTED. The emulator follows that.
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/t");
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(resp.type(), v1::WriteStream::COMMITTED);
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamMintsPendingStream) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/t");
  req.mutable_write_stream()->set_type(v1::WriteStream::PENDING);
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(resp.type(), v1::WriteStream::PENDING);
  EXPECT_EQ(service_->StreamsForTesting(), 1u);
}

TEST_F(StorageWriteServiceTest, CreateWriteStreamMintsBufferedStream) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest req;
  req.set_parent("projects/proj-test/datasets/ds/tables/t");
  req.mutable_write_stream()->set_type(v1::WriteStream::BUFFERED);
  v1::WriteStream resp;
  ::grpc::Status status = service_->CreateWriteStream(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(resp.type(), v1::WriteStream::BUFFERED);
  EXPECT_FALSE(resp.name().empty());
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

TEST_F(StorageWriteServiceTest, BatchCommitRequiresFinalizedPendingStream) {
  CreatePeopleTable();
  v1::CreateWriteStreamRequest create_req;
  create_req.set_parent("projects/proj-test/datasets/ds/tables/t");
  create_req.mutable_write_stream()->set_type(v1::WriteStream::PENDING);
  v1::WriteStream stream;
  ASSERT_TRUE(service_->CreateWriteStream(nullptr, &create_req, &stream).ok());

  v1::BatchCommitWriteStreamsRequest commit_req;
  commit_req.set_parent("projects/proj-test/datasets/ds/tables/t");
  commit_req.add_write_streams(stream.name());
  v1::BatchCommitWriteStreamsResponse commit_resp;
  ::grpc::Status status = service_->BatchCommitWriteStreams(
      nullptr, &commit_req, &commit_resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::FAILED_PRECONDITION);
}

}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
