// Unit tests for the route-typed DuckDB executor. These exercise
// the contract the future `LocalCoordinatorEngine` relies on: each
// method takes an already-analyzed `ResolvedStatement` and produces
// the same wire-facing reply the legacy `DuckDBEngine` shim used
// to produce before `docs/ENGINE_POLICY.md` deleted it
// in favor of `LocalCoordinatorEngine`. We
// rebuild the analyzer up-front (the way the coordinator will) and
// hand the resolved root straight to the executor, so the executor's
// pre-resolution validation, transpiler invocation, DuckDB
// per-query connection, and Arrow result-row path are all on the
// critical path.

#include "backend/engine/duckdb/duckdb_executor.h"

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
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
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
namespace duckdb {
namespace {

namespace fs = std::filesystem;

::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

// Mirrors `LocalCoordinatorEngine`'s analyzer setup (with the
// supports-all-statements allowlist flipped on for DML / DDL).
::googlesql::AnalyzerOptions MakeAnalyzerOptions(bool all_statements) {
  ::googlesql::AnalyzerOptions options(MakeLanguageOptions());
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();
  if (all_statements) {
    options.mutable_language()->SetSupportsAllStatementKinds();
  }
  return options;
}

class DuckDbExecutorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-duckdb-executor-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    executor_ = std::make_unique<DuckDbExecutor>(storage_.get());
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

  // Two-column people table (id INT64 REQUIRED, name STRING
  // NULLABLE). Matches the canonical fixture used by the engine
  // tests so the executor's wire output stays aligned with the
  // engine path's.
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
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
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

  // Analyze `sql` against the fixture catalog. The returned
  // `AnalyzerOutput` owns the resolved AST -- callers must keep it
  // alive for the duration of the executor call.
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> Analyze(
      absl::string_view sql,
      ::googlesql::Catalog* catalog,
      bool all_statements) {
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions(all_statements);
    ::googlesql::TypeFactory type_factory;
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog, &type_factory, &output);
    if (!s.ok()) return s;
    return output;
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<DuckDbExecutor> executor_{};
};

TEST_F(DuckDbExecutorTest, ExecuteQuerySelectStarFromTableStreamsAllRows) {
  // The executor's smoke test: hand it a fully-analyzed
  // `SELECT * FROM ds.people` and confirm the wire output matches
  // the canonical three rows we seeded into storage. Covers the
  // analyze-then-execute split that the coordinator will rely on.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze("SELECT * FROM ds.people",
                          bundle.catalog.get(),
                          /*all_statements=*/false);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt =
      (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  absl::StatusOr<std::unique_ptr<RowSource>> source = executor_->ExecuteQuery(
      MakeRequest("SELECT * FROM ds.people"), *stmt, bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  const schema::TableSchema& s = (*source)->schema();
  ASSERT_EQ(s.columns.size(), 2u);
  EXPECT_EQ(s.columns[0].name, "id");
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(s.columns[1].name, "name");
  EXPECT_EQ(s.columns[1].type, schema::ColumnType::kString);

  // DuckDB does not promise a stable row order without ORDER BY.
  std::vector<std::pair<int64_t, std::string>> seen;
  storage::Row row;
  while (true) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
    ASSERT_EQ(row.cells[1].kind(), storage::Value::Kind::kString);
    seen.emplace_back(row.cells[0].int64_value(), row.cells[1].string_value());
  }
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  std::sort(seen.begin(), seen.end());
  std::sort(want.begin(), want.end());
  EXPECT_EQ(seen, want);
}

TEST_F(DuckDbExecutorTest,
       ExecuteQueryNarrowsColumnsWhenAnalyzerSchemaIsSubsetOfTable) {
  // Regression: the legacy executor stripped the wrapping pass-through
  // ProjectScan and handed the bare TableScan to the transpiler, which
  // emitted SELECT for *all* of the table's columns. With
  // `prune_unused_columns=false` (the analyzer setting both
  // `LocalCoordinatorEngine` and the legacy DuckDBEngine use), the
  // TableScan retains every storage column even when the user-spelled
  // SELECT only asks for a subset; the result chunk would then arrive
  // with one extra Arrow column that `arrow_to_bq::ChunkRowToCells`
  // refused to render against the analyzer-output schema:
  //
  //   arrow_to_bq: chunk has 3 columns but analyzer output schema has 2
  //
  // The fix: hand `EmitQueryStmt` the QueryStmt itself; the outermost
  // SELECT projects only the analyzer-output columns. Pin a 3-column
  // table source against a 2-column projection here so a regression
  // (e.g. someone re-introducing the strip-and-bypass shortcut) fails
  // at the executor unit-test level instead of the
  // `frontend/handlers/query_test.cc` integration suite.
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
  schema::ColumnSchema tags;
  tags.name = "tags";
  tags.type = schema::ColumnType::kString;
  tags.mode = schema::ColumnMode::kRepeated;
  bq_schema.columns.push_back(tags);
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  ASSERT_TRUE(
      storage_->CreateTable({"proj-test", "ds", "wide"}, bq_schema).ok());
  std::vector<storage::Row> rows;
  auto append = [&](int64_t v_id, std::string v_name) {
    storage::Row r;
    r.cells = {
        storage::Value::Int64(v_id),
        storage::Value::String(std::move(v_name)),
        storage::Value::Array({}),
    };
    rows.push_back(std::move(r));
  };
  append(1, "ada");
  append(2, "linus");
  append(3, "grace");
  ASSERT_TRUE(
      storage_
          ->AppendRows({"proj-test", "ds", "wide"}, absl::MakeConstSpan(rows))
          .ok());

  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze("SELECT id, name FROM ds.wide ORDER BY id",
                          bundle.catalog.get(),
                          /*all_statements=*/false);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt =
      (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  absl::StatusOr<std::unique_ptr<RowSource>> source = executor_->ExecuteQuery(
      MakeRequest("SELECT id, name FROM ds.wide ORDER BY id"),
      *stmt,
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  const schema::TableSchema& s = (*source)->schema();
  ASSERT_EQ(s.columns.size(), 2u);
  EXPECT_EQ(s.columns[0].name, "id");
  EXPECT_EQ(s.columns[1].name, "name");

  std::vector<std::pair<int64_t, std::string>> seen;
  storage::Row row;
  while (true) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    seen.emplace_back(row.cells[0].int64_value(), row.cells[1].string_value());
  }
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  EXPECT_EQ(seen, want);
}

TEST_F(DuckDbExecutorTest, ExecuteQueryRejectsNonQueryStatement) {
  // The coordinator is supposed to dispatch DDL through the control-op
  // route, not through the DuckDB executor; defensively the executor
  // returns INVALID_ARGUMENT (not UNIMPLEMENTED) when fed a non-query
  // statement on its query surface so a routing bug surfaces loudly
  // instead of looking like a transpiler gap.
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze("CREATE TABLE ds.t (id INT64)",
                          bundle.catalog.get(),
                          /*all_statements=*/true);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt =
      (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  auto source = executor_->ExecuteQuery(
      MakeRequest("CREATE TABLE ds.t (id INT64)"), *stmt, bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument)
      << source.status();
}

TEST_F(DuckDbExecutorTest, ExecuteBigframesCacheJoinShape) {
  // Regression for bigframes `cache()` join SQL: output schema column
  // `bfuid_col_4` must stay INT64 (amount=3), not pick up STRING
  // `column_0` ('John') from a misaligned DuckDB chunk.
  static constexpr char kSql[] = R"sql(
SELECT `level_0`, `column_0`, `bfuid_col_2`, `bfuid_col_4`, `column_1`, `bfuid_col_14` AS `bfuid_col_15`, `bfuid_col_10` AS `bfuid_col_16`, `bfuid_col_13` AS `bfuid_col_17`, `bfuid_col_9` AS `bfuid_col_18`, `bfuid_col_11` AS `bfuid_col_19` FROM (SELECT
  `t11`.`level_0`, `t11`.`column_0`, `t11`.`bfuid_col_2`, `t11`.`bfuid_col_9`, `t11`.`bfuid_col_10`, `t11`.`bfuid_col_11`,
  `t6`.`bfuid_col_3`, `t6`.`bfuid_col_4`, `t6`.`column_1`, `t6`.`bfuid_col_13`, `t6`.`bfuid_col_14`
FROM (
  SELECT * FROM (
    SELECT `t7`.`level_0`, `t7`.`column_0`, `t8`.`bfuid_col_2`, `t7`.`bfuid_col_5` AS `bfuid_col_9`, `t8`.`bfuid_col_7` AS `bfuid_col_10`, `t8`.`bfuid_col_8` AS `bfuid_col_11`
    FROM (SELECT * FROM (SELECT * FROM UNNEST(ARRAY<STRUCT<`level_0` INT64, `column_0` STRING, `bfuid_col_5` INT64>>[STRUCT(0, 'John', 0)]) AS `level_0`) AS `t1`) AS `t7`
    LEFT OUTER JOIN (
      SELECT `t2`.`level_0` AS `bfuid_col_1`, `t2`.`column_0` AS `bfuid_col_2`, `t2`.`bfuid_col_6` AS `bfuid_col_7`, TRUE AS `bfuid_col_8`
      FROM (SELECT * FROM UNNEST(ARRAY<STRUCT<`level_0` INT64, `column_0` STRING, `bfuid_col_6` INT64>>[STRUCT(0, 'group_1', 0)]) AS `level_0`) AS `t2`
    ) AS `t8` ON COALESCE(`t7`.`level_0`, 0) = COALESCE(`t8`.`bfuid_col_1`, 0) AND COALESCE(`t7`.`level_0`, 1) = COALESCE(`t8`.`bfuid_col_1`, 1)
  ) AS `t9`
) AS `t11`
LEFT OUTER JOIN (
  SELECT `t0`.`level_0` AS `bfuid_col_3`, `t0`.`column_0` AS `bfuid_col_4`, `t0`.`column_1`, `t0`.`bfuid_col_12` AS `bfuid_col_13`, TRUE AS `bfuid_col_14`
  FROM (SELECT * FROM UNNEST(ARRAY<STRUCT<`level_0` INT64, `column_0` INT64, `column_1` BOOLEAN, `bfuid_col_12` INT64>>[STRUCT(0, 3, TRUE, 0)]) AS `level_0`) AS `t0`
) AS `t6` ON COALESCE(`t11`.`level_0`, 0) = COALESCE(`t6`.`bfuid_col_3`, 0) AND COALESCE(`t11`.`level_0`, 1) = COALESCE(`t6`.`bfuid_col_3`, 1)) AS `t`
)sql";
  const std::string sql(kSql);
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze(sql, bundle.catalog.get(), /*all_statements=*/false);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt =
      (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  const auto* query_stmt = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(query_stmt, nullptr);
  transpiler::Transpiler transpiler;
  std::string transpiled = transpiler.Transpile(query_stmt);
  ASSERT_FALSE(transpiled.empty()) << "bigframes cache join must transpile";

  absl::StatusOr<std::unique_ptr<RowSource>> source =
      executor_->ExecuteQuery(MakeRequest(sql), *stmt, bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  const schema::TableSchema& out_schema = (*source)->schema();
  ASSERT_EQ(out_schema.columns.size(), 10u);
  int bfuid_col_4_idx = -1;
  for (size_t i = 0; i < out_schema.columns.size(); ++i) {
    if (out_schema.columns[i].name == "bfuid_col_4") {
      bfuid_col_4_idx = static_cast<int>(i);
      break;
    }
  }
  ASSERT_EQ(bfuid_col_4_idx, 3) << "bfuid_col_4 schema position drift";

  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status() << "\nSQL:\n" << transpiled;
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 10u);
  EXPECT_EQ(row.cells[static_cast<size_t>(bfuid_col_4_idx)].int64_value(), 3);
  EXPECT_EQ(row.cells[1].string_value(), "John");
}

TEST_F(DuckDbExecutorTest, ExecuteDmlInsertSelectQualifyDedupesRows) {
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
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  ASSERT_TRUE(
      storage_->CreateTable({"proj-test", "ds", "ins_src"}, src_schema).ok());
  schema::TableSchema dst_schema = src_schema;
  ASSERT_TRUE(
      storage_->CreateTable({"proj-test", "ds", "ins_dst"}, dst_schema).ok());

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
                  ->AppendRows({"proj-test", "ds", "ins_src"},
                               absl::MakeConstSpan(seed))
                  .ok());

  const std::string sql =
      "INSERT INTO ds.ins_dst (id, tie_break, value) "
      "SELECT * FROM ds.ins_src "
      "QUALIFY ROW_NUMBER() OVER ("
      "  PARTITION BY id ORDER BY tie_break DESC"
      ") = 1";
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze(sql, bundle.catalog.get(), /*all_statements=*/true);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt =
      (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  auto result =
      executor_->ExecuteDml(MakeRequest(sql), *stmt, bundle.catalog.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.inserted_row_count, 2);

  auto scan = storage_->ScanRows({"proj-test", "ds", "ins_dst"});
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
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
