// Direct (no gRPC socket) tests for `StorageReadService::CreateReadSession`
// and the helper parsers that gate the RPC. Mirrors the shape of
// `query_test.cc` -- an in-memory storage instance is fabricated under
// `SetUp`, the service is constructed against it, and each test pokes
// the handler with a hand-built request proto. ServerContext is always
// nullptr because the handler never inspects it on the plan-37 path.
//
// Plan 38 (`storage-read-rows`) will extend the suite to cover the
// streaming `ReadRows` reply; today the only thing we pin for ReadRows
// is the UNIMPLEMENTED contract so the FallbackEngine-style call sites
// can branch on it.

#include "frontend/handlers/storage_read.h"

#include <memory>
#include <string>

#include "backend/schema/schema.h"
#include "backend/storage/memory/in_memory_storage.h"
#include "backend/storage/storage.h"
#include "grpcpp/grpcpp.h"
#include "gtest/gtest.h"
#include "proto/storage_read.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

class StorageReadServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    storage_ = std::make_unique<backend::storage::memory::InMemoryStorage>();
    service_ = std::make_unique<StorageReadService>(storage_.get());
  }

  // Materializes a tiny `proj-test.ds.t` table with columns
  //   id   INT64    REQUIRED
  //   name STRING   NULLABLE
  //   tags STRING   REPEATED
  // so the create-read-session happy path has something to bind to.
  // Matching shape to `query_test.cc::CreatePeopleTable` so plan 38
  // can share a fixture if it wants.
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
    backend::schema::ColumnSchema tags;
    tags.name = "tags";
    tags.type = backend::schema::ColumnType::kString;
    tags.mode = backend::schema::ColumnMode::kRepeated;
    schema.columns.push_back(tags);
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
    ASSERT_TRUE(
        storage_->CreateTable({"proj-test", "ds", "t"}, schema).ok());
  }

  // Builds a well-formed `CreateReadSessionRequest` that points at
  // the people table; individual tests mutate the pieces they want
  // to break.
  v1::CreateReadSessionRequest MakePeopleRequest() {
    v1::CreateReadSessionRequest req;
    req.set_parent("projects/proj-test");
    req.mutable_read_session()->set_table(
        "projects/proj-test/datasets/ds/tables/t");
    return req;
  }

  std::unique_ptr<backend::storage::memory::InMemoryStorage> storage_;
  std::unique_ptr<StorageReadService> service_;
};

// ---------------------------------------------------------------------------
// CreateReadSession: happy path
// ---------------------------------------------------------------------------

TEST_F(StorageReadServiceTest, CreateReadSessionReturnsSessionStreamAndSchema) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();

  // Session name uses the canonical
  // `projects/{p}/locations/-/sessions/s{N}` shape so the gateway can
  // synthesize matching BigQuery REST IDs.
  EXPECT_EQ(resp.name(), "projects/proj-test/locations/-/sessions/s1");

  // Table is echoed verbatim so the caller does not need to re-derive it.
  EXPECT_EQ(resp.table(), "projects/proj-test/datasets/ds/tables/t");

  // Schema is attached so a follow-up ReadRows does not have to
  // round-trip back through DescribeTable. Plan 37 sends every column
  // in declaration order.
  ASSERT_EQ(resp.schema().fields_size(), 3);
  EXPECT_EQ(resp.schema().fields(0).name(), "id");
  EXPECT_EQ(resp.schema().fields(0).type(), "INT64");
  EXPECT_EQ(resp.schema().fields(0).mode(), "REQUIRED");
  EXPECT_EQ(resp.schema().fields(1).name(), "name");
  EXPECT_EQ(resp.schema().fields(1).type(), "STRING");
  EXPECT_EQ(resp.schema().fields(2).name(), "tags");
  EXPECT_EQ(resp.schema().fields(2).type(), "STRING");
  EXPECT_EQ(resp.schema().fields(2).mode(), "REPEATED");

  // Exactly one stream today; plan 39+ will parallelize per
  // `max_stream_count`. The stream id nests under the session name.
  ASSERT_EQ(resp.streams_size(), 1);
  EXPECT_EQ(resp.streams(0).name(),
            "projects/proj-test/locations/-/sessions/s1/streams/0");

  // SessionsForTesting reflects the mint we just did (and only that
  // one). This is the test hook plan 38 will use to assert the
  // ReadRows lookup actually consults the session map.
  EXPECT_EQ(service_->SessionsForTesting(), 1u);
}

TEST_F(StorageReadServiceTest, CreateReadSessionEchoesReadOptions) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  auto* options = req.mutable_read_session()->mutable_read_options();
  options->add_selected_fields("id");
  options->add_selected_fields("name");
  options->set_row_restriction("id > 0");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();

  // Plan 37 does not enforce read_options, but the proto contract says
  // they round-trip on the reply so the gateway can show the caller
  // what shape the session was minted with.
  ASSERT_EQ(resp.read_options().selected_fields_size(), 2);
  EXPECT_EQ(resp.read_options().selected_fields(0), "id");
  EXPECT_EQ(resp.read_options().selected_fields(1), "name");
  EXPECT_EQ(resp.read_options().row_restriction(), "id > 0");
}

TEST_F(StorageReadServiceTest, CreateReadSessionMintsUniqueSessionIds) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();

  v1::ReadSession first;
  v1::ReadSession second;
  ASSERT_TRUE(service_->CreateReadSession(nullptr, &req, &first).ok());
  ASSERT_TRUE(service_->CreateReadSession(nullptr, &req, &second).ok());
  EXPECT_NE(first.name(), second.name());
  EXPECT_EQ(service_->SessionsForTesting(), 2u);
}

// ---------------------------------------------------------------------------
// CreateReadSession: validation errors
// ---------------------------------------------------------------------------

TEST_F(StorageReadServiceTest, CreateReadSessionEmptyParentIsInvalidArgument) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  req.set_parent("");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_EQ(service_->SessionsForTesting(), 0u);
}

TEST_F(StorageReadServiceTest,
       CreateReadSessionMalformedParentIsInvalidArgument) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  // Missing the `projects/` prefix; matches the public BigQuery
  // surface, which rejects the same shape.
  req.set_parent("proj-test");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(StorageReadServiceTest,
       CreateReadSessionEmptyTableIsInvalidArgument) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  req.mutable_read_session()->set_table("");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(StorageReadServiceTest,
       CreateReadSessionMalformedTableIsInvalidArgument) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  // Drop the `tables/` segment so the path no longer has 6 parts.
  req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/t");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(StorageReadServiceTest,
       CreateReadSessionParentTableProjectMismatchIsInvalidArgument) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  req.set_parent("projects/other-proj");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_EQ(service_->SessionsForTesting(), 0u);
}

TEST_F(StorageReadServiceTest, CreateReadSessionMissingTableIsNotFound) {
  // Dataset + project exist but the table does not. Storage::GetSchema
  // returns NotFound and the handler maps that onto gRPC NOT_FOUND.
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND)
      << status.error_message();
}

TEST_F(StorageReadServiceTest, CreateReadSessionMissingDatasetIsNotFound) {
  // Neither dataset nor table exist; same NOT_FOUND mapping but
  // through a different storage path.
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND)
      << status.error_message();
}

// ---------------------------------------------------------------------------
// ReadRows: plan-37 stub
// ---------------------------------------------------------------------------

TEST_F(StorageReadServiceTest, ReadRowsIsUnimplemented) {
  // Plan 37 only ships CreateReadSession; ReadRows lands in plan 38.
  // Pin the UNIMPLEMENTED contract so the gateway's fallback path can
  // branch on it without parsing the message body.
  v1::ReadRowsRequest req;
  req.set_read_stream(
      "projects/proj-test/locations/-/sessions/s1/streams/0");
  ::grpc::Status status = service_->ReadRows(nullptr, &req, nullptr);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED)
      << status.error_message();
}

}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
