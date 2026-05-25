// Direct (no gRPC socket) tests for `QueryService::DryRun`. The
// analyzer call is expensive (the whole catalog gets initialized
// per RPC) but the tests still finish in well under a second on a
// developer laptop because the catalog is per-request and we never
// touch GoogleSQL's builtin-function library lazily-init guts more
// than once per test.

#include "frontend/handlers/query.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/memory/in_memory_storage.h"
#include "backend/storage/storage.h"
#include "grpcpp/grpcpp.h"
#include "gtest/gtest.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

class QueryServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    storage_ = std::make_unique<backend::storage::memory::InMemoryStorage>();
    service_ = std::make_unique<QueryService>(storage_.get());
  }

  // Builds a request with a project pre-set; tests just fill in the
  // SQL and any default dataset they care about.
  v1::QueryRequest MakeRequest(absl::string_view sql) {
    v1::QueryRequest req;
    req.set_project_id("proj-test");
    req.set_sql(std::string(sql));
    return req;
  }

  // Materializes a tiny `proj-test.ds.t` table with columns
  //   id   INT64    REQUIRED
  //   name STRING   NULLABLE
  //   tags STRING   REPEATED
  // so name-resolution tests have something to point at.
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
    ASSERT_TRUE(storage_
                    ->CreateDataset({"proj-test", "ds"}, "US")
                    .ok());
    ASSERT_TRUE(storage_
                    ->CreateTable({"proj-test", "ds", "t"}, schema)
                    .ok());
  }

  std::unique_ptr<backend::storage::memory::InMemoryStorage> storage_;
  std::unique_ptr<QueryService> service_;
};

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

// ---------------------------------------------------------------------------
// StreamQueryResults / ExecuteQuery
//
// The gRPC handler is a one-line shim over `StreamQueryResults`, which
// takes a writer callback so the tests can collect messages into a
// vector without spinning up a real `grpc::ServerWriter`. The streaming
// contract is what these tests pin down:
//
//   * The very first emitted message is `QueryResultRow` with the
//     resolved output schema populated (cells empty).
//   * Each subsequent message is a `QueryResultRow` with cells set
//     (schema unset).
//   * Even a zero-row query (e.g. SELECT from an empty table) still
//     produces the schema message, so the gateway can synthesize the
//     BigQuery REST `schema` field without waiting for a row.
// ---------------------------------------------------------------------------

// Helper that pushes every emitted message into a vector. Returns the
// std::function the handler expects so tests can spell out the
// `StreamQueryResults` invocation in one line.
class MessageCollector {
 public:
  std::function<bool(const v1::QueryResultRow&)> Writer() {
    return [this](const v1::QueryResultRow& msg) {
      messages_.push_back(msg);
      return true;
    };
  }
  const std::vector<v1::QueryResultRow>& messages() const { return messages_; }

 private:
  std::vector<v1::QueryResultRow> messages_;
};

TEST_F(QueryServiceTest, ExecuteQuerySelect1StreamsSchemaThenRow) {
  v1::QueryRequest req = MakeRequest("SELECT 1 AS one");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  ASSERT_TRUE(status.ok()) << status.error_message();

  const auto& messages = collector.messages();
  ASSERT_EQ(messages.size(), 2u);

  EXPECT_TRUE(messages[0].has_schema());
  EXPECT_EQ(messages[0].cells_size(), 0);
  ASSERT_EQ(messages[0].schema().fields_size(), 1);
  EXPECT_EQ(messages[0].schema().fields(0).name(), "one");
  EXPECT_EQ(messages[0].schema().fields(0).type(), "INT64");

  EXPECT_FALSE(messages[1].has_schema());
  ASSERT_EQ(messages[1].cells_size(), 1);
  EXPECT_EQ(messages[1].cells(0).string_value(), "1");
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
  ASSERT_TRUE(storage_
                  ->AppendRows({"proj-test", "ds", "t"},
                               absl::MakeConstSpan(rows))
                  .ok());

  v1::QueryRequest req =
      MakeRequest("SELECT id, name FROM ds.t ORDER BY id");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  ASSERT_TRUE(status.ok()) << status.error_message();

  const auto& messages = collector.messages();
  ASSERT_EQ(messages.size(), 4u);

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
}

TEST_F(QueryServiceTest, ExecuteQueryEmptyTableEmitsSchemaOnly) {
  CreatePeopleTable();
  v1::QueryRequest req = MakeRequest("SELECT id, name FROM ds.t");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(collector.messages().size(), 1u);
  EXPECT_TRUE(collector.messages()[0].has_schema());
}

TEST_F(QueryServiceTest, ExecuteQuerySyntaxErrorIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("SELECT FROM");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_TRUE(collector.messages().empty());
}

TEST_F(QueryServiceTest, ExecuteQueryUseLegacySqlIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("SELECT 1");
  req.set_use_legacy_sql(true);
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
  EXPECT_TRUE(collector.messages().empty());
}

TEST_F(QueryServiceTest, ExecuteQueryMissingProjectIsInvalidArgument) {
  v1::QueryRequest req;
  req.set_sql("SELECT 1");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST_F(QueryServiceTest, ExecuteQueryEmptySqlIsInvalidArgument) {
  v1::QueryRequest req = MakeRequest("");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::INVALID_ARGUMENT)
      << status.error_message();
}

TEST(QueryServiceWithoutStorageTest, ExecuteQueryReturnsFailedPrecondition) {
  v1::QueryRequest req;
  req.set_project_id("proj-test");
  req.set_sql("SELECT 1");
  std::vector<v1::QueryResultRow> messages;
  ::grpc::Status status = StreamQueryResults(
      /*storage=*/nullptr, req,
      [&](const v1::QueryResultRow& m) {
        messages.push_back(m);
        return true;
      });
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::FAILED_PRECONDITION)
      << status.error_message();
  EXPECT_TRUE(messages.empty());
}

TEST_F(QueryServiceTest, ExecuteQueryCancelledWriterReturnsCancelled) {
  v1::QueryRequest req = MakeRequest("SELECT 1 AS one");
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req,
                          [](const v1::QueryResultRow&) { return false; });
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::CANCELLED)
      << status.error_message();
}

// ---------------------------------------------------------------------------
// Statement classification (Phase 6a)
//
// `StreamQueryResults` analyzes the statement once up front so it can
// pick the right engine entry point: SELECT keeps the existing
// schema+rows protocol, INSERT/UPDATE/DELETE/MERGE routes through
// ExecuteDml and emits a final dml_stats summary, and DDL is rejected
// with UNIMPLEMENTED until Phase 6b implements CREATE/DROP/ALTER.
// ---------------------------------------------------------------------------

TEST_F(QueryServiceTest, ExecuteQueryInsertEmitsDmlStats) {
  CreatePeopleTable();
  v1::QueryRequest req = MakeRequest(
      "INSERT INTO ds.t (id, name, tags) "
      "VALUES (1, 'ada', ['math']), (2, 'linus', [])");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  ASSERT_TRUE(status.ok()) << status.error_message();

  // DML response shape: one message, no schema / cells, just stats.
  const auto& messages = collector.messages();
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_FALSE(messages[0].has_schema());
  EXPECT_EQ(messages[0].cells_size(), 0);
  ASSERT_TRUE(messages[0].has_dml_stats());
  EXPECT_EQ(messages[0].dml_stats().inserted_row_count(), 2);
  EXPECT_EQ(messages[0].dml_stats().updated_row_count(), 0);
  EXPECT_EQ(messages[0].dml_stats().deleted_row_count(), 0);

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

TEST_F(QueryServiceTest, ExecuteQueryDdlIsUnimplemented) {
  v1::QueryRequest req =
      MakeRequest("CREATE TABLE ds.new_table (id INT64) OPTIONS()");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED)
      << status.error_message();
  EXPECT_TRUE(collector.messages().empty());
}

TEST_F(QueryServiceTest, ExecuteQueryDeleteEmitsDmlStats) {
  CreatePeopleTable();
  // Phase 6b: DELETE now runs end-to-end against the reference-impl
  // engine; the handler streams a single dml_stats message with the
  // matching deletedRowCount.
  v1::QueryRequest req = MakeRequest("DELETE FROM ds.t WHERE FALSE");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  ASSERT_TRUE(status.ok()) << status.error_message();
  const auto& messages = collector.messages();
  ASSERT_EQ(messages.size(), 1u);
  ASSERT_TRUE(messages[0].has_dml_stats());
  EXPECT_EQ(messages[0].dml_stats().deleted_row_count(), 0);
}

TEST_F(QueryServiceTest, ExecuteQueryMergeIsUnimplementedFromEngine) {
  CreatePeopleTable();
  // Phase 6b: MERGE on the reference-impl engine returns UNIMPLEMENTED
  // because the reference-impl algebrizer does not yet support
  // ResolvedMergeStmt at the statement root. Plan 34's engine-policy
  // decision (HANDOFF.md §4.3 path 3, "DuckDB-only MERGE") landed MERGE
  // on the DuckDB engine and left this path as the documented
  // asymmetry; the handler propagates the engine's UNIMPLEMENTED as
  // gRPC UNIMPLEMENTED. The empty array literal is explicitly typed
  // (`CAST([] AS ARRAY<STRING>)`) because the analyzer cannot
  // otherwise infer an element type for `[]` against the
  // `ARRAY<STRING>` `tags` column.
  v1::QueryRequest req = MakeRequest(
      "MERGE INTO ds.t T USING (SELECT 99 AS id, 'mira' AS name, "
      "CAST([] AS ARRAY<STRING>) AS tags) S ON T.id = S.id "
      "WHEN NOT MATCHED THEN INSERT (id, name, tags) "
      "VALUES (S.id, S.name, S.tags)");
  MessageCollector collector;
  ::grpc::Status status =
      StreamQueryResults(storage_.get(), req, collector.Writer());
  EXPECT_EQ(status.error_code(), ::grpc::StatusCode::UNIMPLEMENTED)
      << status.error_message();
  EXPECT_TRUE(collector.messages().empty());
}

}  // namespace
}  // namespace frontend
}  // namespace bigquery_emulator
