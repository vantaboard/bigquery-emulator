// StreamQueryResults / ExecuteQuery tests for `QueryService`.

#include "frontend/handlers/query_test_fixture.h"

namespace bigquery_emulator {
namespace frontend {
namespace {
TEST_F(QueryServiceTest, ExecuteQuerySelect1StreamsSchemaThenRow) {
  v1::QueryRequest req = MakeRequest("SELECT 1 AS one");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  ASSERT_TRUE(status.ok()) << status.error_message();

  const auto& messages = collector.messages();
  // Four messages: schema, one row, then the trailing
  // `statement_type` + `emulator_route` pair the gateway folds into
  // `Job.statistics.query.{statementType,emulatorRoute}`. See
  // `docs/ENGINE_POLICY.md` Item 5 and
  // `docs/ENGINE_POLICY.md`.
  ASSERT_EQ(messages.size(), 4u);

  EXPECT_TRUE(messages[0].has_schema());
  EXPECT_EQ(messages[0].cells_size(), 0);
  ASSERT_EQ(messages[0].schema().fields_size(), 1);
  EXPECT_EQ(messages[0].schema().fields(0).name(), "one");
  EXPECT_EQ(messages[0].schema().fields(0).type(), "INT64");

  EXPECT_FALSE(messages[1].has_schema());
  ASSERT_EQ(messages[1].cells_size(), 1);
  EXPECT_EQ(messages[1].cells(0).string_value(), "1");

  EXPECT_FALSE(messages[2].has_schema());
  EXPECT_EQ(messages[2].cells_size(), 0);
  EXPECT_EQ(messages[2].statement_type(), "SELECT");
  // `SELECT 1` has no FROM clause -> semantic executor (see
  // `docs/ENGINE_POLICY.md`).
  EXPECT_EQ(messages[3].emulator_route(), "semantic_executor");
}

TEST_F(QueryServiceTest, ExecuteQuerySelectFromTableStreamsAllRows) {
  CreatePeopleTable();
  std::vector<backend::storage::Row> rows;
  auto append = [&](int64_t id, std::string name) {
    backend::storage::Row r;
    r.cells = {
        backend::storage::Value::Int64(id),
        backend::storage::Value::String(std::move(name)),
        backend::storage::Value::Array({}),
    };
    rows.push_back(std::move(r));
  };
  append(1, "ada");
  append(2, "linus");
  append(3, "grace");
  ASSERT_TRUE(
      storage_->AppendRows({"proj-test", "ds", "t"}, absl::MakeConstSpan(rows))
          .ok());

  v1::QueryRequest req = MakeRequest("SELECT id, name FROM ds.t ORDER BY id");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  ASSERT_TRUE(status.ok()) << status.error_message();

  const auto& messages = collector.messages();
  // Six messages: schema, three rows, plus the trailing
  // `statement_type` + `emulator_route` pair.
  ASSERT_EQ(messages.size(), 6u);

  ASSERT_TRUE(messages[0].has_schema());
  ASSERT_EQ(messages[0].schema().fields_size(), 2);
  EXPECT_EQ(messages[0].schema().fields(0).name(), "id");
  EXPECT_EQ(messages[0].schema().fields(0).type(), "INT64");
  EXPECT_EQ(messages[0].schema().fields(1).name(), "name");
  EXPECT_EQ(messages[0].schema().fields(1).type(), "STRING");

  ASSERT_EQ(messages[1].cells_size(), 2);
  EXPECT_EQ(messages[1].cells(0).string_value(), "1");
  EXPECT_EQ(messages[1].cells(1).string_value(), "ada");
  ASSERT_EQ(messages[2].cells_size(), 2);
  EXPECT_EQ(messages[2].cells(0).string_value(), "2");
  EXPECT_EQ(messages[2].cells(1).string_value(), "linus");
  ASSERT_EQ(messages[3].cells_size(), 2);
  EXPECT_EQ(messages[3].cells(0).string_value(), "3");
  EXPECT_EQ(messages[3].cells(1).string_value(), "grace");

  EXPECT_EQ(messages[4].statement_type(), "SELECT");
  EXPECT_EQ(messages[5].emulator_route(), "duckdb_native");
}

TEST_F(QueryServiceTest, ExecuteQueryEmptyTableEmitsSchemaThenStatementType) {
  CreatePeopleTable();
  v1::QueryRequest req = MakeRequest("SELECT id, name FROM ds.t");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  ASSERT_TRUE(status.ok()) << status.error_message();
  // Three messages: the schema (always emitted, even for zero-row
  // results) and the trailing `statement_type` + `emulator_route`
  // pair.
  ASSERT_EQ(collector.messages().size(), 3u);
  EXPECT_TRUE(collector.messages()[0].has_schema());
  EXPECT_EQ(collector.messages()[1].statement_type(), "SELECT");
  EXPECT_EQ(collector.messages()[2].emulator_route(), "duckdb_native");
}

TEST_F(QueryServiceTest, ExecuteQuerySyntaxErrorIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("SELECT FROM");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_TRUE(collector.messages().empty());
}

TEST_F(QueryServiceTest, ExecuteQueryUseLegacySqlIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("SELECT 1");
  req.set_use_legacy_sql(true);
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_TRUE(collector.messages().empty());
}

TEST_F(QueryServiceTest, ExecuteQueryMissingProjectIsInvalidArgument) {
  v1::QueryRequest req;
  req.set_sql("SELECT 1");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(QueryServiceTest, ExecuteQueryEmptySqlIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST(QueryServiceWithoutStorageTest, ExecuteQueryReturnsFailedPrecondition) {
  v1::QueryRequest req;
  req.set_project_id("proj-test");
  req.set_sql("SELECT 1");
  std::vector<v1::QueryResultRow> messages;
  ::grpc::Status status = StreamQueryResults(
      /*storage=*/nullptr, req, [&](const v1::QueryResultRow& m) {
        messages.push_back(m);
        return true;
      });
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::FAILED_PRECONDITION)
      << status.error_message();
  EXPECT_TRUE(messages.empty());
}

TEST_F(QueryServiceTest, ExecuteQueryCancelledWriterReturnsCancelled) {
  v1::QueryRequest req = MakeRequest("SELECT 1 AS one");
  ::grpc::Status status = StreamQueryResults(
      storage_.get(),
      req,
      [](const v1::QueryResultRow&) { return false; },
      engine_.get());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::CANCELLED)
      << status.error_message();
}

// ---------------------------------------------------------------------------
// Statement classification
//
// `StreamQueryResults` analyzes the statement once up front so it can
// pick the right engine entry point: SELECT keeps the existing
// schema+rows protocol, INSERT/UPDATE/DELETE/MERGE routes through
// ExecuteDml and emits a final dml_stats summary, and DDL is rejected
// with UNIMPLEMENTED until CREATE/DROP/ALTER lands.
// ---------------------------------------------------------------------------

TEST_F(QueryServiceTest, ExecuteQueryInsertEmitsDmlStats) {
  CreatePeopleTable();
  v1::QueryRequest req = MakeRequest(
      "INSERT INTO ds.t (id, name, tags) "
      "VALUES (1, 'ada', ['math']), (2, 'linus', [])");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  ASSERT_TRUE(status.ok()) << status.error_message();

  // DML response shape: dml_stats followed by the trailing
  // statement_type + emulator_route pair.
  const auto& messages = collector.messages();
  ASSERT_EQ(messages.size(), 3u);
  EXPECT_FALSE(messages[0].has_schema());
  EXPECT_EQ(messages[0].cells_size(), 0);
  ASSERT_TRUE(messages[0].has_dml_stats());
  EXPECT_EQ(messages[0].dml_stats().inserted_row_count(), 2);
  EXPECT_EQ(messages[0].dml_stats().updated_row_count(), 0);
  EXPECT_EQ(messages[0].dml_stats().deleted_row_count(), 0);
  EXPECT_EQ(messages[1].statement_type(), "INSERT");
  // INSERT against a user table routes through the semantic
  // executor's DML path (/ `docs/ENGINE_POLICY.md`).
  EXPECT_EQ(messages[2].emulator_route(), "semantic_executor");

  // Storage round-trip: the rows actually landed.
  auto scan = storage_->ScanRows({"proj-test", "ds", "t"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  int rows_seen = 0;
  backend::storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ++rows_seen;
  }
  EXPECT_EQ(rows_seen, 2);
}

TEST_F(QueryServiceTest, ExecuteQueryDdlEmitsStatementType) {
  // CREATE TABLE routes through the coordinator's `kControlOp`
  // route to `backend/engine/control/control_op_executor`. The
  // reply stream carries exactly two messages: the
  // `statement_type` trailer the gateway folds into
  // `Job.statistics.query.statementType` and the `emulator_route`
  // trailer the gateway folds into
  // `Job.statistics.query.emulatorRoute` (loopback-only). See
  // `docs/ENGINE_POLICY.md` Item 5 and
  // `docs/ENGINE_POLICY.md`.
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  v1::QueryRequest req =
      MakeRequest("CREATE TABLE ds.new_table (id INT64, name STRING)");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(collector.messages().size(), 2u);
  EXPECT_EQ(collector.messages()[0].statement_type(), "CREATE_TABLE");
  EXPECT_EQ(collector.messages()[1].emulator_route(), "control_op");

  auto sch = storage_->GetSchema({"proj-test", "ds", "new_table"});
  ASSERT_TRUE(sch.ok()) << sch.status();
  ASSERT_EQ(sch->columns.size(), 2u);
  EXPECT_EQ(sch->columns[0].name, "id");
  EXPECT_EQ(sch->columns[1].name, "name");
}

TEST_F(QueryServiceTest, ExecuteQuerySqlUdfCreateThenCall) {
  const char* kCreate = R"(CREATE FUNCTION customfunc(
  arr ARRAY<STRUCT<name STRING, val INT64>>
) AS (
  (SELECT SUM(IF(elem.name = "foo",elem.val,null)) FROM UNNEST(arr) AS elem)
))";
  MessageCollector create_collector;
  ::grpc::Status create_status = StreamQueryResults(storage_.get(),
                                                    MakeRequest(kCreate),
                                                    create_collector.Writer(),
                                                    engine_.get());
  ASSERT_TRUE(create_status.ok()) << create_status.error_message();

  const char* kSelect = R"(SELECT customfunc([
  STRUCT<name STRING, val INT64>("foo", 10),
  STRUCT<name STRING, val INT64>("bar", 40),
  STRUCT<name STRING, val INT64>("foo", 20)
]))";
  MessageCollector select_collector;
  ::grpc::Status select_status = StreamQueryResults(storage_.get(),
                                                    MakeRequest(kSelect),
                                                    select_collector.Writer(),
                                                    engine_.get());
  ASSERT_TRUE(select_status.ok()) << select_status.error_message();
  const auto& messages = select_collector.messages();
  ASSERT_GE(messages.size(), 2u);
  ASSERT_TRUE(messages[0].has_schema());
  ASSERT_FALSE(messages[1].has_schema());
  ASSERT_EQ(messages[1].cells_size(), 1);
  EXPECT_EQ(messages[1].cells(0).string_value(), "30");
}

TEST_F(QueryServiceTest, ExecuteQueryDeleteEmitsDmlStats) {
  CreatePeopleTable();
  // DELETE runs end-to-end against the DuckDB engine; the handler
  // streams a single dml_stats message with the matching
  // deletedRowCount.
  v1::QueryRequest req = MakeRequest("DELETE FROM ds.t WHERE FALSE");
  MessageCollector collector;
  ::grpc::Status status = StreamQueryResults(
      storage_.get(), req, collector.Writer(), engine_.get());
  ASSERT_TRUE(status.ok()) << status.error_message();
  const auto& messages = collector.messages();
  // dml_stats followed by the trailing statement_type +
  // emulator_route pair.
  ASSERT_EQ(messages.size(), 3u);
  ASSERT_TRUE(messages[0].has_dml_stats());
  EXPECT_EQ(messages[0].dml_stats().deleted_row_count(), 0);
  EXPECT_EQ(messages[1].statement_type(), "DELETE");
  EXPECT_EQ(messages[2].emulator_route(), "semantic_executor");
}
}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
