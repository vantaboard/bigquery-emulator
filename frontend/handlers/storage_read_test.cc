// Direct (no gRPC socket) tests for `StorageReadService::CreateReadSession`
// and the helper parsers that gate the RPC. Mirrors the shape of
// `query_test.cc` -- an in-memory storage instance is fabricated under
// `SetUp`, the service is constructed against it, and each test pokes
// the handler with a hand-built request proto. ServerContext is always
// nullptr because the handler never inspects it on the plan-37 path.
//
// Plan 38 (`storage-read-rows`) adds a second fixture
// (`StorageReadGrpcTest`) that stands up an in-process gRPC server and
// client channel so the streaming `ReadRows` reply can be exercised
// end-to-end without inventing a ServerWriter mock. The in-process
// transport is the same one `grpc::testing` uses; it bypasses the
// network stack but otherwise runs the full
// codec/dispatch/serialization path.

#include "frontend/handlers/storage_read.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "backend/schema/schema.h"
#include "backend/storage/memory/in_memory_storage.h"
#include "backend/storage/storage.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/credentials.h"
#include "grpcpp/security/server_credentials.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"
#include "gtest/gtest.h"
#include "proto/emulator.pb.h"
#include "proto/storage_read.grpc.pb.h"
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
// ReadRows: validation-only tests that do not require a real
// ServerWriter. Happy-path tests live in the StorageReadGrpcTest
// fixture below; they need an in-process server because
// ::grpc::ServerWriter is concrete and not designed to be mocked.
// ---------------------------------------------------------------------------

TEST_F(StorageReadServiceTest, ReadRowsRejectsMalformedReadStream) {
  v1::ReadRowsRequest req;
  // Missing the `/streams/0` suffix entirely. Plan 37 only mints
  // streams/0; the handler refuses every other shape.
  req.set_read_stream("projects/proj-test/locations/-/sessions/s1");
  ::grpc::Status status = service_->ReadRows(nullptr, &req, nullptr);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(StorageReadServiceTest, ReadRowsRejectsEmptyReadStream) {
  v1::ReadRowsRequest req;
  ::grpc::Status status = service_->ReadRows(nullptr, &req, nullptr);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

// ---------------------------------------------------------------------------
// StorageReadGrpcTest: in-process gRPC server + client. This is the
// C++-side end-to-end harness for ReadRows; it stands in for the
// `grpcurl` smoke the plan calls for and unblocks plan 39 from
// having to wait on a Go integration test to validate the streaming
// contract.
// ---------------------------------------------------------------------------

class StorageReadGrpcTest : public ::testing::Test {
 protected:
  void SetUp() override {
    storage_ = std::make_unique<backend::storage::memory::InMemoryStorage>();
    service_ = std::make_unique<StorageReadService>(storage_.get());

    // Bind to localhost:0 so the OS picks a free port. Mirrors the
    // shape that `frontend/server/server.cc` uses for the production
    // engine.
    int bound_port = 0;
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort("127.0.0.1:0",
                              ::grpc::InsecureServerCredentials(),
                              &bound_port);
    builder.RegisterService(service_.get());
    server_ = builder.BuildAndStart();
    ASSERT_NE(server_, nullptr);
    ASSERT_NE(bound_port, 0);

    const std::string target =
        absl::StrCat("127.0.0.1:", bound_port);
    channel_ = ::grpc::CreateChannel(target,
                                      ::grpc::InsecureChannelCredentials());
    ASSERT_NE(channel_, nullptr);
    stub_ = v1::StorageRead::NewStub(channel_);
    ASSERT_NE(stub_, nullptr);
  }

  void TearDown() override {
    if (server_ != nullptr) server_->Shutdown();
  }

  // Creates a fixed `proj-test.ds.t` table with the three-column
  // people schema (id, name, tags). Tests append rows to it and
  // round-trip them through ReadRows.
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

  // Appends `n` rows with id=0..n-1 and a derived name/tags shape.
  // Mirrors the StorageReadServiceTest schema so the two suites can
  // share assertions.
  void AppendPeople(int64_t n) {
    std::vector<backend::storage::Row> rows;
    rows.reserve(n);
    for (int64_t i = 0; i < n; ++i) {
      backend::storage::Row r;
      r.cells.push_back(backend::storage::Value::Int64(i));
      r.cells.push_back(
          backend::storage::Value::String(absl::StrCat("person-", i)));
      std::vector<backend::storage::Value> tag_values;
      tag_values.push_back(
          backend::storage::Value::String(absl::StrCat("t", i)));
      r.cells.push_back(
          backend::storage::Value::Array(std::move(tag_values)));
      rows.push_back(std::move(r));
    }
    ASSERT_TRUE(storage_
                     ->AppendRows({"proj-test", "ds", "t"},
                                   absl::MakeConstSpan(rows))
                     .ok());
  }

  std::unique_ptr<backend::storage::memory::InMemoryStorage> storage_;
  std::unique_ptr<StorageReadService> service_;
  std::unique_ptr<::grpc::Server> server_;
  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<v1::StorageRead::Stub> stub_;
};

TEST_F(StorageReadGrpcTest, ReadRowsRoundTripsRowsInOrder) {
  CreatePeopleTable();
  AppendPeople(/*n=*/250);

  // Step 1: mint a session via the wire.
  ::grpc::ClientContext create_ctx;
  v1::CreateReadSessionRequest create_req;
  create_req.set_parent("projects/proj-test");
  create_req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/tables/t");
  v1::ReadSession session;
  ASSERT_TRUE(
      stub_->CreateReadSession(&create_ctx, create_req, &session).ok());
  ASSERT_EQ(session.streams_size(), 1);
  const std::string stream_name = session.streams(0).name();

  // Step 2: drain the stream. With 250 rows and a page size of 100
  // we expect three pages: 100, 100, 50.
  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  read_req.set_read_stream(stream_name);
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  ASSERT_NE(reader, nullptr);

  std::vector<v1::DataRow> rows;
  std::vector<int64_t> page_sizes;
  v1::ReadRowsResponse page;
  while (reader->Read(&page)) {
    page_sizes.push_back(page.row_count());
    for (const auto& row : page.rows()) {
      rows.push_back(row);
    }
  }
  ::grpc::Status status = reader->Finish();
  ASSERT_TRUE(status.ok()) << status.error_message();

  // Paging: 100 + 100 + 50.
  ASSERT_EQ(page_sizes.size(), 3u);
  EXPECT_EQ(page_sizes[0], 100);
  EXPECT_EQ(page_sizes[1], 100);
  EXPECT_EQ(page_sizes[2], 50);

  // Row contents: 250 rows in id order with the expected name/tags.
  ASSERT_EQ(rows.size(), 250u);
  for (int64_t i = 0; i < 250; ++i) {
    ASSERT_EQ(rows[i].cells_size(), 3);
    // INT64 cells ride on Cell.string_value as the decimal repr;
    // mirrors the catalog handler's wire shape so a single gateway
    // decoder works for both tabledata.list and ReadRows.
    EXPECT_EQ(rows[i].cells(0).string_value(), absl::StrCat(i));
    EXPECT_EQ(rows[i].cells(1).string_value(), absl::StrCat("person-", i));
    ASSERT_EQ(rows[i].cells(2).array().elements_size(), 1);
    EXPECT_EQ(rows[i].cells(2).array().elements(0).string_value(),
              absl::StrCat("t", i));
  }
}

TEST_F(StorageReadGrpcTest, ReadRowsHonorsOffset) {
  CreatePeopleTable();
  AppendPeople(/*n=*/20);

  ::grpc::ClientContext create_ctx;
  v1::CreateReadSessionRequest create_req;
  create_req.set_parent("projects/proj-test");
  create_req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/tables/t");
  v1::ReadSession session;
  ASSERT_TRUE(
      stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  read_req.set_read_stream(session.streams(0).name());
  // Skip the first 15 rows; remaining 5 land in a single page.
  read_req.set_offset(15);
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  ASSERT_NE(reader, nullptr);

  std::vector<v1::DataRow> rows;
  v1::ReadRowsResponse page;
  while (reader->Read(&page)) {
    for (const auto& row : page.rows()) rows.push_back(row);
  }
  ::grpc::Status status = reader->Finish();
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(rows.size(), 5u);
  for (size_t i = 0; i < rows.size(); ++i) {
    EXPECT_EQ(rows[i].cells(0).string_value(), absl::StrCat(15 + i));
  }
}

TEST_F(StorageReadGrpcTest, ReadRowsEmptyTableYieldsZeroPages) {
  // An empty table still mints a valid session; ReadRows must
  // succeed with no pages emitted (the client loop reads 0 messages
  // then sees Finish() return OK).
  CreatePeopleTable();
  ::grpc::ClientContext create_ctx;
  v1::CreateReadSessionRequest create_req;
  create_req.set_parent("projects/proj-test");
  create_req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/tables/t");
  v1::ReadSession session;
  ASSERT_TRUE(
      stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  read_req.set_read_stream(session.streams(0).name());
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  ASSERT_NE(reader, nullptr);

  v1::ReadRowsResponse page;
  int pages = 0;
  while (reader->Read(&page)) ++pages;
  ::grpc::Status status = reader->Finish();
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(pages, 0);
}

TEST_F(StorageReadGrpcTest, ReadRowsUnknownStreamIsNotFound) {
  CreatePeopleTable();
  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  // Plan 37 mints session names starting at s1; an `s99` session was
  // never created so the handler MUST surface NOT_FOUND.
  read_req.set_read_stream(
      "projects/proj-test/locations/-/sessions/s99/streams/0");
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  ASSERT_NE(reader, nullptr);

  v1::ReadRowsResponse page;
  while (reader->Read(&page)) {
  }
  ::grpc::Status status = reader->Finish();
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND)
      << status.error_message();
}

TEST_F(StorageReadGrpcTest, ReadRowsSchemaDriftIsFailedPrecondition) {
  // Mint a session against the people table, then drop+recreate the
  // table with a different schema. The follow-up ReadRows must
  // detect the drift and return FAILED_PRECONDITION; the gateway
  // (plan 39) translates that to BigQuery's "schema changed" error.
  CreatePeopleTable();
  AppendPeople(5);

  ::grpc::ClientContext create_ctx;
  v1::CreateReadSessionRequest create_req;
  create_req.set_parent("projects/proj-test");
  create_req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/tables/t");
  v1::ReadSession session;
  ASSERT_TRUE(
      stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

  // Drop and recreate with an incompatible shape (1 column instead
  // of 3). The session's stashed schema is stale; ReadRows trips
  // the drift check.
  const backend::storage::TableId tbl{"proj-test", "ds", "t"};
  ASSERT_TRUE(storage_->DropTable(tbl).ok());
  backend::schema::TableSchema other;
  backend::schema::ColumnSchema only;
  only.name = "only";
  only.type = backend::schema::ColumnType::kInt64;
  only.mode = backend::schema::ColumnMode::kNullable;
  other.columns.push_back(only);
  ASSERT_TRUE(storage_->CreateTable(tbl, other).ok());

  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  read_req.set_read_stream(session.streams(0).name());
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  v1::ReadRowsResponse page;
  while (reader->Read(&page)) {
  }
  ::grpc::Status status = reader->Finish();
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::FAILED_PRECONDITION)
      << status.error_message();
}

}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
