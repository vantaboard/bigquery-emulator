// DryRun and validation tests for `QueryService`.

#include "frontend/handlers/query_test_fixture.h"

namespace bigquery_emulator {
namespace frontend {
namespace {
TEST_F(QueryServiceTest, DryRunSelect1ReturnsSingleInt64Column) {
  v1::QueryRequest req = MakeRequest("SELECT 1");
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(resp.schema().fields_size(), 1);
  EXPECT_EQ(resp.schema().fields(0).type(), "INT64");
  EXPECT_EQ(resp.estimated_bytes_processed(), 0);
}

TEST_F(QueryServiceTest, DryRunSelectMultipleConstantsReturnsAllColumns) {
  v1::QueryRequest req = MakeRequest(
      "SELECT 1 AS one, 'hello' AS greeting, CAST(3.14 AS FLOAT64) AS pi");
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(resp.schema().fields_size(), 3);
  EXPECT_EQ(resp.schema().fields(0).name(), "one");
  EXPECT_EQ(resp.schema().fields(0).type(), "INT64");
  EXPECT_EQ(resp.schema().fields(1).name(), "greeting");
  EXPECT_EQ(resp.schema().fields(1).type(), "STRING");
  EXPECT_EQ(resp.schema().fields(2).name(), "pi");
  EXPECT_EQ(resp.schema().fields(2).type(), "FLOAT64");
}

TEST_F(QueryServiceTest, DryRunSelectFromTableReturnsTableSchema) {
  CreatePeopleTable();
  v1::QueryRequest req = MakeRequest("SELECT id, name, tags FROM ds.t");
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(resp.schema().fields_size(), 3);
  EXPECT_EQ(resp.schema().fields(0).name(), "id");
  EXPECT_EQ(resp.schema().fields(0).type(), "INT64");
  EXPECT_EQ(resp.schema().fields(1).name(), "name");
  EXPECT_EQ(resp.schema().fields(1).type(), "STRING");
  EXPECT_EQ(resp.schema().fields(2).name(), "tags");
  EXPECT_EQ(resp.schema().fields(2).type(), "STRING");
  EXPECT_EQ(resp.schema().fields(2).mode(), "REPEATED");
}

TEST_F(QueryServiceTest, DryRunSyntaxErrorIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("SELECT FROM");
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_FALSE(status.error_message().empty());
  // Error message must carry a `line:column:` prefix so the gateway
  // can extract it for the BigQuery REST error envelope.
  EXPECT_NE(status.error_message().find(':'), std::string::npos)
      << status.error_message();
}

TEST_F(QueryServiceTest, DryRunUnknownTableIsInvalidArgument) {
  // Name-resolution errors surface as `INVALID_ARGUMENT` from
  // GoogleSQL even though the catalog status was `NOT_FOUND`. The
  // analyzer wraps it: `Table not found: ds.missing`. The gateway
  // maps both to BigQuery's `invalidQuery` reason.
  v1::QueryRequest req = MakeRequest("SELECT * FROM ds.missing");
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  EXPECT_NE(status.error_code(), ::grpc::StatusCode::OK)
      << status.error_message();
  EXPECT_NE(status.error_message().find("missing"), std::string::npos)
      << status.error_message();
}

TEST_F(QueryServiceTest, DryRunEmptySqlIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("");
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(QueryServiceTest, DryRunUseLegacySqlIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("SELECT 1");
  req.set_use_legacy_sql(true);
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(QueryServiceTest, DryRunMissingProjectIsInvalidArgument) {
  v1::QueryRequest req;
  req.set_sql("SELECT 1");
  v1::DryRunResponse resp;
  auto status = service_->DryRun(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST(QueryServiceWithoutStorageTest, DryRunReturnsFailedPrecondition) {
  QueryService service(/*storage=*/nullptr);
  v1::QueryRequest req;
  req.set_project_id("proj-test");
  req.set_sql("SELECT 1");
  v1::DryRunResponse resp;
  auto status = service.DryRun(nullptr, &req, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::FAILED_PRECONDITION)
      << status.error_message();
}
}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
