// Direct (no gRPC socket) tests for `StorageReadService::CreateReadSession`.

#include "frontend/handlers/storage_read_internal.h"
#include "frontend/handlers/storage_read_test_fixture.h"
#include "grpcpp/grpcpp.h"

namespace bigquery_emulator {
namespace frontend {
namespace {
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
  // Plan 39: the handler parses the row_restriction at session-mint
  // time. `id = 0` is the canonical happy-path shape (single
  // `<column> = <literal>` equality); range / connective forms now
  // surface INVALID_ARGUMENT from CreateReadSession, which has its
  // own dedicated test below.
  options->set_row_restriction("id = 0");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();

  // The proto contract says read_options round-trips on the reply so
  // the gateway can show the caller what shape the session was minted
  // with. Plan 15 (storage-read-write) honors selected_fields, so the
  // response *schema* now reflects the projection (id, name only)
  // while the read_options round-trip still echoes the request.
  ASSERT_EQ(resp.read_options().selected_fields_size(), 2);
  EXPECT_EQ(resp.read_options().selected_fields(0), "id");
  EXPECT_EQ(resp.read_options().selected_fields(1), "name");
  EXPECT_EQ(resp.read_options().row_restriction(), "id = 0");

  // Plan 15: the response schema is now projected to the caller's
  // selected_fields list, in caller-supplied order. The full table
  // has three columns; pinning "id" + "name" must drop "tags".
  ASSERT_EQ(resp.schema().fields_size(), 2);
  EXPECT_EQ(resp.schema().fields(0).name(), "id");
  EXPECT_EQ(resp.schema().fields(1).name(), "name");
}

TEST_F(StorageReadServiceTest, CreateReadSessionRejectsUnknownSelectedField) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  // `phone` is not a column on the people table; plan 15 rejects
  // this at session-mint time so the streaming RPC never starts
  // against an invalid projection.
  req.mutable_read_session()->mutable_read_options()->add_selected_fields(
      "phone");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_EQ(service_->SessionsForTesting(), 0u);
}

TEST_F(StorageReadServiceTest, CreateReadSessionRejectsEmptySelectedField) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  // An empty entry is a wire-level oddity but the handler refuses
  // it explicitly so a buggy client cannot smuggle a no-match
  // projection that would silently approximate "all columns".
  req.mutable_read_session()->mutable_read_options()->add_selected_fields("");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(StorageReadServiceTest, CreateReadSessionRejectsRangeRestriction) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  // `>` is outside the plan-39 surface; the parser bails with a
  // clear "only `<column> = <literal>` supported" message, which the
  // handler maps onto gRPC INVALID_ARGUMENT.
  req.mutable_read_session()->mutable_read_options()->set_row_restriction(
      "id > 0");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(StorageReadServiceTest,
       CreateReadSessionRejectsUnknownColumnRestriction) {
  CreatePeopleTable();
  v1::CreateReadSessionRequest req = MakePeopleRequest();
  req.mutable_read_session()->mutable_read_options()->set_row_restriction(
      "missing = 1");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
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

TEST_F(StorageReadServiceTest, CreateReadSessionEmptyTableIsInvalidArgument) {
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
  req.mutable_read_session()->set_table("projects/proj-test/datasets/ds/t");
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

TEST_F(StorageReadServiceTest,
       CreateReadSessionAllowsPublicDataTableWithCallerParent) {
  backend::schema::TableSchema schema;
  backend::schema::ColumnSchema name;
  name.name = "name";
  name.type = backend::schema::ColumnType::kString;
  schema.columns.push_back(name);
  ASSERT_TRUE(
      storage_->CreateDataset({internal::kPublicDataProject, "usa_names"}, "US")
          .ok());
  ASSERT_TRUE(
      storage_
          ->CreateTable({internal::kPublicDataProject, "usa_names", "names"},
                        schema)
          .ok());

  v1::CreateReadSessionRequest req;
  req.set_parent("projects/dev");
  req.mutable_read_session()->set_table(
      "projects/bigquery-public-data/datasets/usa_names/tables/names");
  v1::ReadSession resp;
  ::grpc::Status status = service_->CreateReadSession(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(service_->SessionsForTesting(), 1u);
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
}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
