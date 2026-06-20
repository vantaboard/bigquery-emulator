// CREATE TABLE AS SELECT tests for the control-op executor. Kept in a
// separate file so `control_op_executor_test.cc` stays under the
// cpp-lint file-length cap.

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/control/control_op_executor.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace {

namespace fs = std::filesystem;

::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::AnalyzerOptions options(MakeLanguageOptions());
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

class ControlOpExecutorCtasTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-control-op-ctas-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    executor_ = std::make_unique<ControlOpExecutor>(storage_.get());
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  }

  void TearDown() override {
    executor_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  QueryRequest MakeRequest(absl::string_view sql) {
    QueryRequest req;
    req.project_id = "proj-test";
    req.sql = std::string(sql);
    return req;
  }

  void CreatePeopleTable() {
    schema::TableSchema bq_schema;
    schema::ColumnSchema id;
    id.name = "id";
    id.type = schema::ColumnType::kInt64;
    id.mode = schema::ColumnMode::kRequired;
    bq_schema.columns.push_back(id);
    schema::ColumnSchema name;
    name.name = "name";
    name.type = schema::ColumnType::kString;
    name.mode = schema::ColumnMode::kNullable;
    bq_schema.columns.push_back(name);
    ASSERT_TRUE(
        storage_->CreateTable({"proj-test", "ds", "people"}, bq_schema).ok());

    auto make_row = [](int64_t id_val, std::string name_val) {
      storage::Row r;
      r.cells = {
          storage::Value::Int64(id_val),
          storage::Value::String(std::move(name_val)),
      };
      return r;
    };
    std::vector<storage::Row> rows = {
        make_row(1, "ada"),
        make_row(2, "linus"),
        make_row(3, "grace"),
    };
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "people"},
                                 absl::MakeConstSpan(rows))
                    .ok());
  }

  struct CatalogBundle {
    std::unique_ptr<::googlesql::TypeFactory> type_factory{};
    std::unique_ptr<catalog::GoogleSqlCatalog> catalog{};
  };
  CatalogBundle MakeCatalog() {
    auto type_factory = std::make_unique<::googlesql::TypeFactory>();
    auto catalog = std::make_unique<catalog::GoogleSqlCatalog>(
        "proj-test", storage_.get(), type_factory.get(), MakeLanguageOptions());
    return {std::move(type_factory), std::move(catalog)};
  }

  absl::Status RunDdl(absl::string_view sql) {
    CatalogBundle bundle = MakeCatalog();
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    ::googlesql::TypeFactory type_factory;
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status analyze = ::googlesql::AnalyzeStatement(
        sql, options, bundle.catalog.get(), &type_factory, &output);
    if (!analyze.ok()) return analyze;
    if (output == nullptr || output->resolved_statement() == nullptr) {
      return absl::InternalError(
          "ControlOpExecutorCtasTest::RunDdl: analyzer produced no resolved "
          "statement");
    }
    return executor_->ExecuteDdl(
        MakeRequest(sql), *output->resolved_statement(), bundle.catalog.get());
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<ControlOpExecutor> executor_{};
};

TEST_F(ControlOpExecutorCtasTest, CreateTableAsSelectMaterializesSourceRows) {
  CreatePeopleTable();
  absl::Status s =
      RunDdl("CREATE TABLE ds.people_copy AS SELECT id, name FROM ds.people");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "people_copy"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);

  auto scan = storage_->ScanRows({"proj-test", "ds", "people_copy"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  std::vector<std::pair<int64_t, std::string>> seen;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    seen.emplace_back(row.cells[0].int64_value(), row.cells[1].string_value());
  }
  std::sort(seen.begin(), seen.end());
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  EXPECT_EQ(seen, want);
}

TEST_F(ControlOpExecutorCtasTest,
       CreateTableAsSelectUnnestNarrowsToOutputSchema) {
  // Regression for TestCopiesAndExtracts / generateTableCTAS: UNNEST
  // ordinality adds `__bq_input_rn` to the inner scan emit; without the
  // outer projection narrow the drained DuckDB table has one extra
  // column and `arrow_to_bq` rejects the schema mismatch.
  absl::Status s = RunDdl(
      "CREATE TABLE ds.unnest_copy AS "
      "SELECT 2000 + r AS year, IF(r > 1, 'foo', 'bar') AS token "
      "FROM UNNEST(GENERATE_ARRAY(0, 2)) AS r");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "unnest_copy"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);
  ASSERT_EQ(schema->columns[0].name, "year");
  ASSERT_EQ(schema->columns[1].name, "token");

  auto scan = storage_->ScanRows({"proj-test", "ds", "unnest_copy"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  storage::Row row;
  int rows = 0;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    ++rows;
  }
  EXPECT_EQ(rows, 3);
}

TEST_F(ControlOpExecutorCtasTest, CreateTableAsSelectCrossJoinUnnestSubquery) {
  // Bench heavy-case setup: CTAS over a subquery whose FROM is two
  // standalone UNNEST relations cross-joined (BigQuery's >1M-row
  // GENERATE_ARRAY pattern).
  absl::Status s = RunDdl(
      "CREATE TABLE ds.cross_join_ctas AS "
      "SELECT id, MOD(id, 7) AS k "
      "FROM ("
      "  SELECT n + (m - 1) * 10 AS id "
      "  FROM UNNEST(GENERATE_ARRAY(1, 3)) AS n "
      "  CROSS JOIN UNNEST(GENERATE_ARRAY(1, 2)) AS m"
      ")");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "cross_join_ctas"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);

  auto scan = storage_->ScanRows({"proj-test", "ds", "cross_join_ctas"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  int rows = 0;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    ++rows;
  }
  EXPECT_EQ(rows, 6);
}

TEST_F(ControlOpExecutorCtasTest, CreateTableAsSelectExceptRowNumberDedup) {
  schema::TableSchema src_schema;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kString;
  id.mode = schema::ColumnMode::kRequired;
  src_schema.columns.push_back(id);
  schema::ColumnSchema tie;
  tie.name = "tie_break";
  tie.type = schema::ColumnType::kInt64;
  tie.mode = schema::ColumnMode::kRequired;
  src_schema.columns.push_back(tie);
  schema::ColumnSchema value;
  value.name = "value";
  value.type = schema::ColumnType::kInt64;
  value.mode = schema::ColumnMode::kRequired;
  src_schema.columns.push_back(value);
  ASSERT_TRUE(
      storage_->CreateTable({"proj-test", "ds", "merge_src"}, src_schema).ok());

  std::vector<storage::Row> seed = {
      storage::Row{{storage::Value::String("a"),
                    storage::Value::Int64(1),
                    storage::Value::Int64(10)}},
      storage::Row{{storage::Value::String("a"),
                    storage::Value::Int64(2),
                    storage::Value::Int64(20)}},
      storage::Row{{storage::Value::String("b"),
                    storage::Value::Int64(1),
                    storage::Value::Int64(30)}},
  };
  ASSERT_TRUE(storage_
                  ->AppendRows({"proj-test", "ds", "merge_src"},
                               absl::MakeConstSpan(seed))
                  .ok());

  absl::Status s = RunDdl(
      "CREATE OR REPLACE TABLE ds.merge_deduped AS "
      "SELECT * EXCEPT(rn) FROM ("
      "  SELECT *, ROW_NUMBER() OVER ("
      "    PARTITION BY id ORDER BY tie_break DESC"
      "  ) AS rn FROM ds.merge_src"
      ") WHERE rn = 1");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "merge_deduped"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 3u);

  auto scan = storage_->ScanRows({"proj-test", "ds", "merge_deduped"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  storage::Row row;
  int rows = 0;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 3u);
    ++rows;
  }
  EXPECT_EQ(rows, 2);
}

}  // namespace
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
