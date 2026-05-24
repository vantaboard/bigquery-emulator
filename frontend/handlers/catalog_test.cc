// Direct (no gRPC socket) tests for `CatalogService`. We exercise each
// RPC by calling the service method with a stack-allocated request /
// response pair; the gRPC `ServerContext*` is unused on the server side
// of every catalog RPC, so passing nullptr is safe and matches what
// the framework would do for an in-process call. The store under test
// is `InMemoryStorage` because it is the canonical, dependency-free
// `Storage` implementation; DuckDB-storage parity is covered by its
// own round-trip test.

#include "frontend/handlers/catalog.h"

#include <memory>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "backend/storage/memory/in_memory_storage.h"
#include "backend/storage/storage.h"
#include "proto/emulator.pb.h"
#include "gmock/gmock.h"
#include "grpcpp/grpcpp.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

using ::testing::HasSubstr;

// Reusable three-column proto schema used by the table RPC tests:
//   id   INT64    REQUIRED
//   name STRING   NULLABLE
//   tags STRING   REPEATED  (an ARRAY<STRING> on the BigQuery wire)
void FillPeopleSchema(v1::TableSchema* schema) {
  schema->Clear();
  auto* id = schema->add_fields();
  id->set_name("id");
  id->set_type("INT64");
  id->set_mode("REQUIRED");
  auto* name = schema->add_fields();
  name->set_name("name");
  name->set_type("STRING");
  name->set_mode("NULLABLE");
  auto* tags = schema->add_fields();
  tags->set_name("tags");
  tags->set_type("STRING");
  tags->set_mode("REPEATED");
}

class CatalogServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    storage_ = std::make_unique<backend::storage::memory::InMemoryStorage>();
    service_ = std::make_unique<CatalogService>(storage_.get());
  }

  std::unique_ptr<backend::storage::memory::InMemoryStorage> storage_;
  std::unique_ptr<CatalogService> service_;
};

TEST_F(CatalogServiceTest, RegisterAndDropDatasetHappyPath) {
  v1::RegisterDatasetRequest req;
  req.mutable_dataset()->set_project_id("proj-1");
  req.mutable_dataset()->set_dataset_id("ds_1");
  req.set_location("US");
  v1::RegisterDatasetResponse resp;
  auto status = service_->RegisterDataset(nullptr, &req, &resp);
  EXPECT_TRUE(status.ok()) << status.error_message();

  v1::DropDatasetRequest drop_req;
  drop_req.mutable_dataset()->set_project_id("proj-1");
  drop_req.mutable_dataset()->set_dataset_id("ds_1");
  v1::DropDatasetResponse drop_resp;
  auto drop_status = service_->DropDataset(nullptr, &drop_req, &drop_resp);
  EXPECT_TRUE(drop_status.ok()) << drop_status.error_message();
}

TEST_F(CatalogServiceTest, RegisterDatasetDuplicateIsAlreadyExists) {
  v1::RegisterDatasetRequest req;
  req.mutable_dataset()->set_project_id("proj-1");
  req.mutable_dataset()->set_dataset_id("ds_1");
  v1::RegisterDatasetResponse resp;
  ASSERT_TRUE(service_->RegisterDataset(nullptr, &req, &resp).ok());

  auto status = service_->RegisterDataset(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::ALREADY_EXISTS);
}

TEST_F(CatalogServiceTest, DropDatasetMissingIsNotFound) {
  v1::DropDatasetRequest req;
  req.mutable_dataset()->set_project_id("proj-1");
  req.mutable_dataset()->set_dataset_id("ghost");
  v1::DropDatasetResponse resp;
  auto status = service_->DropDataset(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

TEST_F(CatalogServiceTest, RegisterDatasetMissingFieldsIsInvalidArgument) {
  v1::RegisterDatasetRequest req;
  req.mutable_dataset()->set_project_id("proj-1");
  // dataset_id intentionally left empty.
  v1::RegisterDatasetResponse resp;
  auto status = service_->RegisterDataset(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT);
  EXPECT_THAT(status.error_message(), HasSubstr("dataset_id"));
}

TEST_F(CatalogServiceTest, RegisterTableHappyPathAndDescribe) {
  v1::RegisterDatasetRequest ds_req;
  ds_req.mutable_dataset()->set_project_id("proj-1");
  ds_req.mutable_dataset()->set_dataset_id("ds_1");
  v1::RegisterDatasetResponse ds_resp;
  ASSERT_TRUE(service_->RegisterDataset(nullptr, &ds_req, &ds_resp).ok());

  v1::RegisterTableRequest req;
  req.mutable_table()->set_project_id("proj-1");
  req.mutable_table()->set_dataset_id("ds_1");
  req.mutable_table()->set_table_id("people");
  FillPeopleSchema(req.mutable_schema());
  v1::RegisterTableResponse resp;
  auto status = service_->RegisterTable(nullptr, &req, &resp);
  EXPECT_TRUE(status.ok()) << status.error_message();

  v1::DescribeTableRequest desc_req;
  desc_req.mutable_table()->set_project_id("proj-1");
  desc_req.mutable_table()->set_dataset_id("ds_1");
  desc_req.mutable_table()->set_table_id("people");
  v1::DescribeTableResponse desc_resp;
  auto desc_status = service_->DescribeTable(nullptr, &desc_req, &desc_resp);
  ASSERT_TRUE(desc_status.ok()) << desc_status.error_message();
  ASSERT_EQ(desc_resp.schema().fields_size(), 3);
  EXPECT_EQ(desc_resp.schema().fields(0).name(), "id");
  EXPECT_EQ(desc_resp.schema().fields(0).type(), "INT64");
  EXPECT_EQ(desc_resp.schema().fields(0).mode(), "REQUIRED");
  EXPECT_EQ(desc_resp.schema().fields(2).name(), "tags");
  EXPECT_EQ(desc_resp.schema().fields(2).mode(), "REPEATED");
}

TEST_F(CatalogServiceTest, RegisterTableDuplicateIsAlreadyExists) {
  v1::RegisterDatasetRequest ds_req;
  ds_req.mutable_dataset()->set_project_id("proj-1");
  ds_req.mutable_dataset()->set_dataset_id("ds_1");
  v1::RegisterDatasetResponse ds_resp;
  ASSERT_TRUE(service_->RegisterDataset(nullptr, &ds_req, &ds_resp).ok());

  v1::RegisterTableRequest req;
  req.mutable_table()->set_project_id("proj-1");
  req.mutable_table()->set_dataset_id("ds_1");
  req.mutable_table()->set_table_id("people");
  FillPeopleSchema(req.mutable_schema());
  v1::RegisterTableResponse resp;
  ASSERT_TRUE(service_->RegisterTable(nullptr, &req, &resp).ok());

  auto status = service_->RegisterTable(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::ALREADY_EXISTS);
}

TEST_F(CatalogServiceTest, RegisterTableWithoutDatasetIsNotFound) {
  v1::RegisterTableRequest req;
  req.mutable_table()->set_project_id("proj-1");
  req.mutable_table()->set_dataset_id("missing_ds");
  req.mutable_table()->set_table_id("people");
  FillPeopleSchema(req.mutable_schema());
  v1::RegisterTableResponse resp;
  auto status = service_->RegisterTable(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

TEST_F(CatalogServiceTest, DropTableMissingIsNotFound) {
  v1::RegisterDatasetRequest ds_req;
  ds_req.mutable_dataset()->set_project_id("proj-1");
  ds_req.mutable_dataset()->set_dataset_id("ds_1");
  v1::RegisterDatasetResponse ds_resp;
  ASSERT_TRUE(service_->RegisterDataset(nullptr, &ds_req, &ds_resp).ok());

  v1::DropTableRequest req;
  req.mutable_table()->set_project_id("proj-1");
  req.mutable_table()->set_dataset_id("ds_1");
  req.mutable_table()->set_table_id("ghost");
  v1::DropTableResponse resp;
  auto status = service_->DropTable(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

TEST_F(CatalogServiceTest, DescribeTableMissingIsNotFound) {
  v1::DescribeTableRequest req;
  req.mutable_table()->set_project_id("proj-1");
  req.mutable_table()->set_dataset_id("missing_ds");
  req.mutable_table()->set_table_id("ghost");
  v1::DescribeTableResponse resp;
  auto status = service_->DescribeTable(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

TEST_F(CatalogServiceTest, DropDatasetNonEmptyIsFailedPrecondition) {
  v1::RegisterDatasetRequest ds_req;
  ds_req.mutable_dataset()->set_project_id("proj-1");
  ds_req.mutable_dataset()->set_dataset_id("ds_1");
  v1::RegisterDatasetResponse ds_resp;
  ASSERT_TRUE(service_->RegisterDataset(nullptr, &ds_req, &ds_resp).ok());

  v1::RegisterTableRequest tbl_req;
  tbl_req.mutable_table()->set_project_id("proj-1");
  tbl_req.mutable_table()->set_dataset_id("ds_1");
  tbl_req.mutable_table()->set_table_id("people");
  FillPeopleSchema(tbl_req.mutable_schema());
  v1::RegisterTableResponse tbl_resp;
  ASSERT_TRUE(service_->RegisterTable(nullptr, &tbl_req, &tbl_resp).ok());

  v1::DropDatasetRequest drop_req;
  drop_req.mutable_dataset()->set_project_id("proj-1");
  drop_req.mutable_dataset()->set_dataset_id("ds_1");
  drop_req.set_delete_contents(false);
  v1::DropDatasetResponse drop_resp;
  auto status = service_->DropDataset(nullptr, &drop_req, &drop_resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::FAILED_PRECONDITION);

  // With delete_contents=true the dataset and its tables go away.
  drop_req.set_delete_contents(true);
  auto cascade_status = service_->DropDataset(nullptr, &drop_req, &drop_resp);
  EXPECT_TRUE(cascade_status.ok()) << cascade_status.error_message();
}

TEST_F(CatalogServiceTest, RegisterTableWithMissingFieldNameIsInvalidArgument) {
  v1::RegisterDatasetRequest ds_req;
  ds_req.mutable_dataset()->set_project_id("proj-1");
  ds_req.mutable_dataset()->set_dataset_id("ds_1");
  v1::RegisterDatasetResponse ds_resp;
  ASSERT_TRUE(service_->RegisterDataset(nullptr, &ds_req, &ds_resp).ok());

  v1::RegisterTableRequest req;
  req.mutable_table()->set_project_id("proj-1");
  req.mutable_table()->set_dataset_id("ds_1");
  req.mutable_table()->set_table_id("people");
  // Add a field with no `name` — TableSchemaFromProto must reject it.
  auto* field = req.mutable_schema()->add_fields();
  field->set_type("INT64");
  v1::RegisterTableResponse resp;
  auto status = service_->RegisterTable(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT);
  EXPECT_THAT(status.error_message(), HasSubstr("name"));
}

// Helper: register `proj-1.ds_1.people` with the three-column
// schema used by the row-level RPC tests.
void RegisterPeople(CatalogService* service) {
  v1::RegisterDatasetRequest ds_req;
  ds_req.mutable_dataset()->set_project_id("proj-1");
  ds_req.mutable_dataset()->set_dataset_id("ds_1");
  v1::RegisterDatasetResponse ds_resp;
  ASSERT_TRUE(service->RegisterDataset(nullptr, &ds_req, &ds_resp).ok());

  v1::RegisterTableRequest tbl_req;
  tbl_req.mutable_table()->set_project_id("proj-1");
  tbl_req.mutable_table()->set_dataset_id("ds_1");
  tbl_req.mutable_table()->set_table_id("people");
  FillPeopleSchema(tbl_req.mutable_schema());
  v1::RegisterTableResponse tbl_resp;
  ASSERT_TRUE(service->RegisterTable(nullptr, &tbl_req, &tbl_resp).ok());
}

// Helper: append a string cell to `row` with the given value.
void AddStringCell(v1::DataRow* row, const std::string& value) {
  row->add_cells()->set_string_value(value);
}

// Helper: append a null cell to `row`.
void AddNullCell(v1::DataRow* row) {
  row->add_cells()->set_null_value(true);
}

TEST_F(CatalogServiceTest, InsertAndListRowsHappyPath) {
  RegisterPeople(service_.get());

  v1::InsertRowsRequest ins_req;
  ins_req.mutable_table()->set_project_id("proj-1");
  ins_req.mutable_table()->set_dataset_id("ds_1");
  ins_req.mutable_table()->set_table_id("people");
  auto* row1 = ins_req.add_rows();
  AddStringCell(row1, "1");
  AddStringCell(row1, "alice");
  // tags is REPEATED -> empty ARRAY cell.
  row1->add_cells()->mutable_array();
  auto* row2 = ins_req.add_rows();
  AddStringCell(row2, "2");
  AddNullCell(row2);
  row2->add_cells()->mutable_array();
  v1::InsertRowsResponse ins_resp;
  auto ins_status = service_->InsertRows(nullptr, &ins_req, &ins_resp);
  EXPECT_TRUE(ins_status.ok()) << ins_status.error_message();

  v1::ListRowsRequest ls_req;
  ls_req.mutable_table()->set_project_id("proj-1");
  ls_req.mutable_table()->set_dataset_id("ds_1");
  ls_req.mutable_table()->set_table_id("people");
  v1::ListRowsResponse ls_resp;
  auto ls_status = service_->ListRows(nullptr, &ls_req, &ls_resp);
  ASSERT_TRUE(ls_status.ok()) << ls_status.error_message();
  EXPECT_EQ(ls_resp.total_rows(), 2);
  EXPECT_EQ(ls_resp.next_start_index(), 2);
  ASSERT_EQ(ls_resp.rows_size(), 2);
  ASSERT_EQ(ls_resp.rows(0).cells_size(), 3);
  EXPECT_EQ(ls_resp.rows(0).cells(0).string_value(), "1");
  EXPECT_EQ(ls_resp.rows(0).cells(1).string_value(), "alice");
  EXPECT_TRUE(ls_resp.rows(0).cells(2).has_array());
  EXPECT_EQ(ls_resp.rows(1).cells(0).string_value(), "2");
  EXPECT_TRUE(ls_resp.rows(1).cells(1).null_value());
}

TEST_F(CatalogServiceTest, ListRowsPagination) {
  RegisterPeople(service_.get());
  v1::InsertRowsRequest ins_req;
  ins_req.mutable_table()->set_project_id("proj-1");
  ins_req.mutable_table()->set_dataset_id("ds_1");
  ins_req.mutable_table()->set_table_id("people");
  for (int i = 0; i < 5; ++i) {
    auto* row = ins_req.add_rows();
    AddStringCell(row, std::to_string(i));
    AddStringCell(row, "n" + std::to_string(i));
    row->add_cells()->mutable_array();
  }
  v1::InsertRowsResponse ins_resp;
  ASSERT_TRUE(service_->InsertRows(nullptr, &ins_req, &ins_resp).ok());

  v1::ListRowsRequest ls_req;
  ls_req.mutable_table()->set_project_id("proj-1");
  ls_req.mutable_table()->set_dataset_id("ds_1");
  ls_req.mutable_table()->set_table_id("people");
  ls_req.set_start_index(1);
  ls_req.set_max_results(2);
  v1::ListRowsResponse ls_resp;
  ASSERT_TRUE(service_->ListRows(nullptr, &ls_req, &ls_resp).ok());
  EXPECT_EQ(ls_resp.total_rows(), 5);
  EXPECT_EQ(ls_resp.next_start_index(), 3);
  ASSERT_EQ(ls_resp.rows_size(), 2);
  EXPECT_EQ(ls_resp.rows(0).cells(0).string_value(), "1");
  EXPECT_EQ(ls_resp.rows(1).cells(0).string_value(), "2");
}

TEST_F(CatalogServiceTest, InsertRowsMissingTableIsNotFound) {
  v1::InsertRowsRequest ins_req;
  ins_req.mutable_table()->set_project_id("proj-1");
  ins_req.mutable_table()->set_dataset_id("ds_1");
  ins_req.mutable_table()->set_table_id("ghost");
  auto* row = ins_req.add_rows();
  AddStringCell(row, "1");
  v1::InsertRowsResponse ins_resp;
  auto status = service_->InsertRows(nullptr, &ins_req, &ins_resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

TEST_F(CatalogServiceTest, ListRowsMissingTableIsNotFound) {
  v1::ListRowsRequest ls_req;
  ls_req.mutable_table()->set_project_id("proj-1");
  ls_req.mutable_table()->set_dataset_id("ds_1");
  ls_req.mutable_table()->set_table_id("ghost");
  v1::ListRowsResponse ls_resp;
  auto status = service_->ListRows(nullptr, &ls_req, &ls_resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::NOT_FOUND);
}

}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
