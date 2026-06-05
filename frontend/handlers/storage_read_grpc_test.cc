// In-process gRPC tests for `StorageReadService::ReadRows`.

#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "frontend/handlers/storage_read_test_fixture.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/credentials.h"
#include "grpcpp/security/server_credentials.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"
#include "proto/emulator.pb.h"
#include "proto/storage_read.grpc.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

namespace fs = std::filesystem;

class StorageReadGrpcTest : public ::testing::Test {
 protected:
  void SetUp() override {
    data_dir_ = MakeTempDataDir("storage-read-grpc-test");
    auto opened =
        backend::storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    service_ = std::make_unique<StorageReadService>(storage_.get());

    // Bind to localhost:0 so the OS picks a free port. Mirrors the
    // shape that `frontend/server/server.cc` uses for the production
    // engine.
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
    stub_ = v1::StorageRead::NewStub(channel_);
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
    ASSERT_TRUE(storage_->CreateTable({"proj-test", "ds", "t"}, schema).ok());
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
      r.cells.push_back(backend::storage::Value::Array(std::move(tag_values)));
      rows.push_back(std::move(r));
    }
    ASSERT_TRUE(
        storage_
            ->AppendRows({"proj-test", "ds", "t"}, absl::MakeConstSpan(rows))
            .ok());
  }

  fs::path data_dir_{};
  std::unique_ptr<backend::storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<StorageReadService> service_{};
  std::unique_ptr<::grpc::Server> server_{};
  std::shared_ptr<::grpc::Channel> channel_{};
  std::unique_ptr<v1::StorageRead::Stub> stub_{};
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
  ASSERT_TRUE(stub_->CreateReadSession(&create_ctx, create_req, &session).ok());
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
  ASSERT_TRUE(stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

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
    for (const auto& row : page.rows())
      rows.push_back(row);
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
  ASSERT_TRUE(stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  read_req.set_read_stream(session.streams(0).name());
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  ASSERT_NE(reader, nullptr);

  v1::ReadRowsResponse page;
  int pages = 0;
  while (reader->Read(&page))
    ++pages;
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
  while (reader->Read(&page)) {}
  ::grpc::Status status = reader->Finish();
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND)
      << status.error_message();
}

// Plan 15 (storage-read-write): selected_fields projection
// end-to-end. The session is minted with `selected_fields = [id]`,
// so the response schema lists only `id` and ReadRows yields one
// cell per row instead of three. The projection survives a
// round-trip through the in-process gRPC server, which is the same
// codec path the production engine uses.
TEST_F(StorageReadGrpcTest, ReadRowsProjectsToSelectedFields) {
  CreatePeopleTable();
  AppendPeople(/*n=*/4);

  ::grpc::ClientContext create_ctx;
  v1::CreateReadSessionRequest create_req;
  create_req.set_parent("projects/proj-test");
  create_req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/tables/t");
  create_req.mutable_read_session()
      ->mutable_read_options()
      ->add_selected_fields("name");
  create_req.mutable_read_session()
      ->mutable_read_options()
      ->add_selected_fields("id");
  v1::ReadSession session;
  ASSERT_TRUE(stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

  // Schema reflects the caller-supplied projection order, not the
  // table's declared column order.
  ASSERT_EQ(session.schema().fields_size(), 2);
  EXPECT_EQ(session.schema().fields(0).name(), "name");
  EXPECT_EQ(session.schema().fields(1).name(), "id");

  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  read_req.set_read_stream(session.streams(0).name());
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  ASSERT_NE(reader, nullptr);
  std::vector<v1::DataRow> rows;
  v1::ReadRowsResponse page;
  while (reader->Read(&page)) {
    for (const auto& row : page.rows())
      rows.push_back(row);
  }
  ::grpc::Status status = reader->Finish();
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(rows.size(), 4u);
  for (size_t i = 0; i < rows.size(); ++i) {
    // Two cells per row, in projected order: name then id.
    ASSERT_EQ(rows[i].cells_size(), 2);
    EXPECT_EQ(rows[i].cells(0).string_value(), absl::StrCat("person-", i));
    EXPECT_EQ(rows[i].cells(1).string_value(), absl::StrCat(i));
  }
}

// Plan 39: row_restriction pushdown end-to-end. CreateReadSession
// parses the predicate, stashes it on the SessionState, and ReadRows
// hands it to `CreateReadStream` which filters before paginating.
TEST_F(StorageReadGrpcTest, ReadRowsHonorsRowRestriction) {
  CreatePeopleTable();
  AppendPeople(/*n=*/10);

  ::grpc::ClientContext create_ctx;
  v1::CreateReadSessionRequest create_req;
  create_req.set_parent("projects/proj-test");
  create_req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/tables/t");
  create_req.mutable_read_session()
      ->mutable_read_options()
      ->set_row_restriction("id = 5");
  v1::ReadSession session;
  ASSERT_TRUE(stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

  ::grpc::ClientContext read_ctx;
  v1::ReadRowsRequest read_req;
  read_req.set_read_stream(session.streams(0).name());
  auto reader = stub_->ReadRows(&read_ctx, read_req);
  ASSERT_NE(reader, nullptr);
  std::vector<v1::DataRow> rows;
  v1::ReadRowsResponse page;
  while (reader->Read(&page)) {
    for (const auto& row : page.rows())
      rows.push_back(row);
  }
  ::grpc::Status status = reader->Finish();
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(rows.size(), 1u);
  EXPECT_EQ(rows[0].cells(0).string_value(), "5");
  EXPECT_EQ(rows[0].cells(1).string_value(), "person-5");
}

TEST_F(StorageReadGrpcTest, CreateReadSessionRejectsMalformedRowRestriction) {
  CreatePeopleTable();

  ::grpc::ClientContext create_ctx;
  v1::CreateReadSessionRequest create_req;
  create_req.set_parent("projects/proj-test");
  create_req.mutable_read_session()->set_table(
      "projects/proj-test/datasets/ds/tables/t");
  // `id > 0` is a range predicate — the parser only accepts `=`.
  create_req.mutable_read_session()
      ->mutable_read_options()
      ->set_row_restriction("id > 0");
  v1::ReadSession session;
  ::grpc::Status status =
      stub_->CreateReadSession(&create_ctx, create_req, &session);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
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
  ASSERT_TRUE(stub_->CreateReadSession(&create_ctx, create_req, &session).ok());

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
  while (reader->Read(&page)) {}
  ::grpc::Status status = reader->Finish();
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::FAILED_PRECONDITION)
      << status.error_message();
}
}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
