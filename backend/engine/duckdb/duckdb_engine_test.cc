// Unit tests for the DuckDB query engine. The tests mirror the
// reference-impl engine fixture so the two engines run against the
// same `InMemoryStorage` shape and the assertions read symmetric:
// the only intentional difference is the engine kind under test.
//
// The tests stay one layer below the gRPC service boundary so the
// engine machinery (analyze + transpile + attach + execute + cell
// conversion) is exercised end-to-end without spinning up the
// frontend. Queries the transpiler does not yet lower
// (`SHAPE_TRACKER.md` rows still on `not_started`) return
// UNIMPLEMENTED -- that is the contract the engine factory's
// `--on_unknown_fn=fallback` wrapper reads to delegate to the
// reference-impl evaluator.

#include "backend/engine/duckdb/duckdb_engine.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/memory/in_memory_storage.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace {

// Mirrors the LanguageOptions snapshot the engine uses internally so
// the per-call `GoogleSqlCatalog` resolves names the same way.
::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

class DuckDBEngineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    storage_ = std::make_unique<storage::memory::InMemoryStorage>();
    engine_ = std::make_unique<DuckDBEngine>(storage_.get());
  }

  QueryRequest MakeRequest(absl::string_view sql) {
    QueryRequest req;
    req.project_id = "proj-test";
    req.sql = std::string(sql);
    return req;
  }

  // Two-column people table (id INT64 REQUIRED, name STRING
  // NULLABLE). Matches the reference-impl engine test's fixture so
  // the two engines can be compared head-to-head against the same
  // shape.
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

    auto make_row = [](int64_t id, std::string name) {
      storage::Row r;
      r.cells = {
          storage::Value::Int64(id),
          storage::Value::String(std::move(name)),
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
    std::unique_ptr<::googlesql::TypeFactory> type_factory;
    std::unique_ptr<catalog::GoogleSqlCatalog> catalog;
  };
  CatalogBundle MakeCatalog() {
    auto type_factory = std::make_unique<::googlesql::TypeFactory>();
    auto catalog = std::make_unique<catalog::GoogleSqlCatalog>(
        "proj-test", storage_.get(), type_factory.get(),
        MakeLanguageOptions());
    return {std::move(type_factory), std::move(catalog)};
  }

  std::unique_ptr<storage::memory::InMemoryStorage> storage_;
  std::unique_ptr<DuckDBEngine> engine_;
};

TEST_F(DuckDBEngineTest, AnalyzeSelect1ReturnsInt64Column) {
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = engine_->Analyze(MakeRequest("SELECT 1"),
                                    bundle.catalog.get());
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const schema::TableSchema& s = (*analyzed)->output_schema();
  ASSERT_EQ(s.columns.size(), 1u);
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);
}

TEST_F(DuckDBEngineTest, DryRunSelect1ReturnsInt64Column) {
  CatalogBundle bundle = MakeCatalog();
  auto dry_run = engine_->DryRun(MakeRequest("SELECT 1"),
                                  bundle.catalog.get());
  ASSERT_TRUE(dry_run.ok()) << dry_run.status();
  ASSERT_EQ(dry_run->schema.columns.size(), 1u);
  EXPECT_EQ(dry_run->schema.columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(dry_run->estimated_bytes_processed, 0);
}

TEST_F(DuckDBEngineTest, ExecuteQuerySelect1FallsBackToUnimplemented) {
  // `SELECT 1` analyzes to ProjectScan(SingleRowScan, computed
  // literal). The transpiler's `EmitProjectScan` is `not_started`,
  // so the engine must report UNIMPLEMENTED -- the engine factory's
  // `--on_unknown_fn=fallback` wrapper takes the reference-impl path
  // from there. We pin the status code so the wrapper has a stable
  // contract to read.
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT 1 AS one"),
                                       bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kUnimplemented)
      << source.status();
}

TEST_F(DuckDBEngineTest, ExecuteQuerySelectStarFromTableStreamsAllRows) {
  // `SELECT * FROM ds.people` is the engine's smoke test: the
  // analyzer wraps the TableScan in a pass-through ProjectScan
  // (same column_list, no expr_list), the engine strips it, the
  // transpiler emits `SELECT "id", "name" FROM "people"`, DuckDB
  // executes it against the materialized in-memory table we ATTACH
  // from `Storage::ScanRows`, and the result rows round-trip back
  // with the same shape `Storage::AppendRows` saw on the write
  // path.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("SELECT * FROM ds.people"), bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  const schema::TableSchema& s = (*source)->schema();
  ASSERT_EQ(s.columns.size(), 2u);
  EXPECT_EQ(s.columns[0].name, "id");
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(s.columns[1].name, "name");
  EXPECT_EQ(s.columns[1].type, schema::ColumnType::kString);

  // DuckDB does not promise a stable row order without an explicit
  // ORDER BY; collect into a set so the assertion stays
  // order-insensitive.
  std::vector<std::pair<int64_t, std::string>> seen;
  storage::Row row;
  while (true) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
    ASSERT_EQ(row.cells[1].kind(), storage::Value::Kind::kString);
    seen.emplace_back(row.cells[0].int64_value(),
                       row.cells[1].string_value());
  }
  ASSERT_EQ(seen.size(), 3u);
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  std::sort(seen.begin(), seen.end());
  std::sort(want.begin(), want.end());
  EXPECT_EQ(seen, want);
}

TEST_F(DuckDBEngineTest, ExecuteQuerySelectStarOrderByLowersToDuckDB) {
  // `ORDER BY id` lifts the inner scan into a ResolvedOrderByScan
  // wrapped by the pass-through ProjectScan. After stripping, the
  // transpiler emits the OrderByScan SQL and DuckDB returns the
  // rows in the requested order. This pins the engine's
  // composability over the Phase 5h scan emits.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("SELECT * FROM ds.people ORDER BY id"),
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  std::vector<int64_t> ids;
  storage::Row row;
  while (true) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    ids.push_back(row.cells[0].int64_value());
  }
  ASSERT_EQ(ids.size(), 3u);
  EXPECT_EQ(ids[0], 1);
  EXPECT_EQ(ids[1], 2);
  EXPECT_EQ(ids[2], 3);
}

TEST_F(DuckDBEngineTest, ExecuteQuerySelectIdOnlyReturnsUnimplemented) {
  // `SELECT id FROM t` lands a ProjectScan whose column_list
  // ([id]) differs from the input TableScan's column_list
  // ([id, name]), so the engine refuses to strip it and returns
  // UNIMPLEMENTED. This is the contract the fallback policy reads
  // to retry against the reference-impl engine; pin it so a future
  // tightening of the strip rules surfaces here.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("SELECT id FROM ds.people"), bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kUnimplemented)
      << source.status();
}

TEST_F(DuckDBEngineTest, ExecuteQuerySyntaxErrorIsInvalidArgument) {
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT FROM"),
                                       bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument)
      << source.status();
}

TEST_F(DuckDBEngineTest, ExecuteQueryRejectsLegacySql) {
  CatalogBundle bundle = MakeCatalog();
  QueryRequest req = MakeRequest("SELECT 1");
  req.use_legacy_sql = true;
  auto source = engine_->ExecuteQuery(req, bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(DuckDBEngineTest, ExecuteQueryRejectsNullCatalog) {
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT 1"), nullptr);
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kFailedPrecondition);
}

// ---------------------------------------------------------------------------
// ExecuteDml (Plan 34 -- DuckDB-only MERGE)
//
// The DuckDB engine's DML surface is intentionally narrow: only MERGE
// lands here today, since INSERT / UPDATE / DELETE already run on the
// reference-impl engine through `PreparedModify`. The `FallbackEngine`
// wrapper that the canonical Phase 5i binary configures (see
// `--engine=duckdb --on_unknown_fn=fallback`) routes the non-MERGE
// DML kinds to the reference-impl engine when this engine returns
// UNIMPLEMENTED.
//
// These tests pin:
//
//   * MERGE WHEN MATCHED + WHEN NOT MATCHED rewrites the target table
//     atomically through `Storage::OverwriteRows` and surfaces an
//     accurate per-branch DmlStats (insertedRowCount,
//     updatedRowCount, deletedRowCount) by diffing the pre-MERGE
//     snapshot against the post-MERGE state on the synthetic primary
//     key (column 0; see `backend/catalog/storage_table.cc`).
//   * INSERT / UPDATE / DELETE return UNIMPLEMENTED so the
//     FallbackEngine wrapper hands them off to the reference-impl
//     engine (which already runs all three through PreparedModify).
//   * MERGE on a target table whose schema is not backed by a
//     StorageTable surfaces FAILED_PRECONDITION (the engine has no
//     way to write rows back through a non-storage catalog Table).
// ---------------------------------------------------------------------------

TEST_F(DuckDBEngineTest, ExecuteDmlMergeMatchedAndNotMatchedUpdatesStorage) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  // MATCHED branch hits id=2 (already in the table, name 'linus' ->
  // 'linus-updated'); NOT MATCHED branch inserts id=4 (not in the
  // table). The USING clause sources both rows from a single SELECT
  // UNION ALL so the resolved MergeStmt has no extra table
  // dependencies beyond the target.
  auto stats = engine_->ExecuteDml(
      MakeRequest("MERGE INTO ds.people T USING ("
                  "  SELECT 2 AS id, 'linus-updated' AS name "
                  "  UNION ALL "
                  "  SELECT 4 AS id, 'rust' AS name) S "
                  "ON T.id = S.id "
                  "WHEN MATCHED THEN UPDATE SET name = S.name "
                  "WHEN NOT MATCHED THEN INSERT (id, name) "
                  "VALUES (S.id, S.name)"),
      bundle.catalog.get());
  ASSERT_TRUE(stats.ok()) << stats.status();
  EXPECT_EQ(stats->inserted_row_count, 1) << "id=4 should land via INSERT";
  EXPECT_EQ(stats->updated_row_count, 1) << "id=2 should land via UPDATE";
  EXPECT_EQ(stats->deleted_row_count, 0) << "no DELETE branch in MERGE";

  // Storage round-trip: id=2 is now 'linus-updated', id=4 is the
  // newly merged-in row, and id=1 / id=3 ride through unchanged.
  auto scan = storage_->ScanRows({"proj-test", "ds", "people"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  std::map<int64_t, std::string> by_id;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_GE(row.cells.size(), 2u);
    by_id[row.cells[0].int64_value()] = row.cells[1].string_value();
  }
  EXPECT_EQ(by_id.size(), 4u);
  EXPECT_EQ(by_id[1], "ada");
  EXPECT_EQ(by_id[2], "linus-updated");
  EXPECT_EQ(by_id[3], "grace");
  EXPECT_EQ(by_id[4], "rust");
}

TEST_F(DuckDBEngineTest, ExecuteDmlInsertFallsBackToUnimplemented) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  // INSERT lives on the reference-impl engine; the DuckDB engine
  // returns UNIMPLEMENTED so the FallbackEngine wrapper takes over.
  // We pin the status code so the wrapper has a stable contract to
  // read.
  auto stats = engine_->ExecuteDml(
      MakeRequest("INSERT INTO ds.people (id, name) VALUES (10, 'kay')"),
      bundle.catalog.get());
  ASSERT_FALSE(stats.ok());
  EXPECT_EQ(stats.status().code(), absl::StatusCode::kUnimplemented)
      << stats.status();
}

TEST_F(DuckDBEngineTest, ExecuteDmlUpdateFallsBackToUnimplemented) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto stats = engine_->ExecuteDml(
      MakeRequest("UPDATE ds.people SET name = 'unknown' WHERE id = 2"),
      bundle.catalog.get());
  ASSERT_FALSE(stats.ok());
  EXPECT_EQ(stats.status().code(), absl::StatusCode::kUnimplemented)
      << stats.status();
}

TEST_F(DuckDBEngineTest, ExecuteDmlDeleteFallsBackToUnimplemented) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto stats = engine_->ExecuteDml(
      MakeRequest("DELETE FROM ds.people WHERE id = 1"),
      bundle.catalog.get());
  ASSERT_FALSE(stats.ok());
  EXPECT_EQ(stats.status().code(), absl::StatusCode::kUnimplemented)
      << stats.status();
}

TEST_F(DuckDBEngineTest, ExecuteDmlRejectsNullCatalog) {
  auto stats = engine_->ExecuteDml(
      MakeRequest("MERGE INTO ds.people T USING (SELECT 1 AS id, "
                  "'ada' AS name) S ON T.id = S.id "
                  "WHEN NOT MATCHED THEN INSERT (id, name) "
                  "VALUES (S.id, S.name)"),
      nullptr);
  ASSERT_FALSE(stats.ok());
  EXPECT_EQ(stats.status().code(), absl::StatusCode::kFailedPrecondition);
}

// ---------------------------------------------------------------------------
// ExecuteDdl (Plan 35 -- DuckDB-only DDL)
//
// The DuckDB engine owns the DDL surface end-to-end. The
// reference-impl engine returns UNIMPLEMENTED for ExecuteDdl, so
// the FallbackEngine wrapper retries DDL against this engine.
// These tests pin the four DDL shapes the plan ships:
//
//   * CREATE TABLE (schema-only) lands an empty table on storage
//     under the BigQuery-typed schema we pass through
//     `Storage::CreateTable`.
//   * CREATE TABLE AS SELECT materializes the inner SELECT against
//     a per-query in-memory DuckDB instance, reads the resulting
//     rows back, and lands them on storage under the analyzer's
//     `column_definition_list` (i.e. BigQuery type fidelity).
//   * DROP TABLE removes the table from storage; DROP TABLE IF
//     EXISTS swallows the missing-table case.
//   * ALTER TABLE ADD COLUMN runs a scan-drop-create-append cycle
//     because `Storage` has no in-place schema evolution. Existing
//     rows get padded with `Value::Null()` for the new column.
// ---------------------------------------------------------------------------

TEST_F(DuckDBEngineTest, ExecuteDdlCreateTableSchemaOnly) {
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest("CREATE TABLE ds.empty (id INT64 NOT NULL, name STRING)"),
      bundle.catalog.get());
  ASSERT_TRUE(status.ok()) << status;

  auto sch = storage_->GetSchema({"proj-test", "ds", "empty"});
  ASSERT_TRUE(sch.ok()) << sch.status();
  ASSERT_EQ(sch->columns.size(), 2u);
  EXPECT_EQ(sch->columns[0].name, "id");
  EXPECT_EQ(sch->columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(sch->columns[0].mode, schema::ColumnMode::kRequired);
  EXPECT_EQ(sch->columns[1].name, "name");
  EXPECT_EQ(sch->columns[1].type, schema::ColumnType::kString);
  EXPECT_EQ(sch->columns[1].mode, schema::ColumnMode::kNullable);
}

TEST_F(DuckDBEngineTest, ExecuteDdlCreateTableIfNotExistsSwallowsAlreadyExists) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest("CREATE TABLE IF NOT EXISTS ds.people "
                  "(id INT64, name STRING)"),
      bundle.catalog.get());
  EXPECT_TRUE(status.ok()) << status;
}

TEST_F(DuckDBEngineTest, ExecuteDdlCreateTableConflictIsAlreadyExists) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest("CREATE TABLE ds.people (id INT64, name STRING)"),
      bundle.catalog.get());
  ASSERT_FALSE(status.ok());
  EXPECT_EQ(status.code(), absl::StatusCode::kAlreadyExists) << status;
}

TEST_F(DuckDBEngineTest, ExecuteDdlCreateTableAsSelectRoundTrips) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest(
          "CREATE TABLE ds.people_copy AS SELECT id, name FROM ds.people"),
      bundle.catalog.get());
  ASSERT_TRUE(status.ok()) << status;

  auto sch = storage_->GetSchema({"proj-test", "ds", "people_copy"});
  ASSERT_TRUE(sch.ok()) << sch.status();
  ASSERT_EQ(sch->columns.size(), 2u);
  EXPECT_EQ(sch->columns[0].name, "id");
  EXPECT_EQ(sch->columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(sch->columns[1].name, "name");
  EXPECT_EQ(sch->columns[1].type, schema::ColumnType::kString);

  auto scan = storage_->ScanRows({"proj-test", "ds", "people_copy"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  std::vector<std::pair<int64_t, std::string>> seen;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    seen.emplace_back(row.cells[0].int64_value(),
                       row.cells[1].string_value());
  }
  std::sort(seen.begin(), seen.end());
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  EXPECT_EQ(seen, want);
}

TEST_F(DuckDBEngineTest, ExecuteDdlDropTableRemovesStorage) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(MakeRequest("DROP TABLE ds.people"),
                                     bundle.catalog.get());
  ASSERT_TRUE(status.ok()) << status;

  auto sch = storage_->GetSchema({"proj-test", "ds", "people"});
  ASSERT_FALSE(sch.ok());
  EXPECT_EQ(sch.status().code(), absl::StatusCode::kNotFound) << sch.status();
}

TEST_F(DuckDBEngineTest, ExecuteDdlDropTableMissingIsNotFound) {
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(MakeRequest("DROP TABLE ds.missing"),
                                     bundle.catalog.get());
  ASSERT_FALSE(status.ok());
  EXPECT_EQ(status.code(), absl::StatusCode::kNotFound) << status;
}

TEST_F(DuckDBEngineTest, ExecuteDdlDropTableIfExistsSwallowsMissing) {
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest("DROP TABLE IF EXISTS ds.missing"), bundle.catalog.get());
  EXPECT_TRUE(status.ok()) << status;
}

TEST_F(DuckDBEngineTest, ExecuteDdlAlterTableAddColumnPadsExistingRows) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest("ALTER TABLE ds.people ADD COLUMN age INT64"),
      bundle.catalog.get());
  ASSERT_TRUE(status.ok()) << status;

  auto sch = storage_->GetSchema({"proj-test", "ds", "people"});
  ASSERT_TRUE(sch.ok()) << sch.status();
  ASSERT_EQ(sch->columns.size(), 3u);
  EXPECT_EQ(sch->columns[2].name, "age");
  EXPECT_EQ(sch->columns[2].type, schema::ColumnType::kInt64);

  // Existing rows survive the rewrite. The new `age` column is NULL
  // for every pre-existing row.
  auto scan = storage_->ScanRows({"proj-test", "ds", "people"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  int rows_seen = 0;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 3u);
    EXPECT_EQ(row.cells[2].kind(), storage::Value::Kind::kNull)
        << "post-ALTER row for id=" << row.cells[0].int64_value()
        << " should have NULL age";
    ++rows_seen;
  }
  EXPECT_EQ(rows_seen, 3);
}

TEST_F(DuckDBEngineTest, ExecuteDdlAlterTableAddColumnIfNotExistsIsIdempotent) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  ASSERT_TRUE(engine_
                  ->ExecuteDdl(
                      MakeRequest(
                          "ALTER TABLE ds.people ADD COLUMN age INT64"),
                      bundle.catalog.get())
                  .ok());
  auto second = engine_->ExecuteDdl(
      MakeRequest("ALTER TABLE ds.people ADD COLUMN IF NOT EXISTS age INT64"),
      bundle.catalog.get());
  EXPECT_TRUE(second.ok()) << second;

  // Duplicate add without IF NOT EXISTS surfaces as ALREADY_EXISTS
  // (per BigQuery's error contract for ALTER TABLE).
  auto duplicate = engine_->ExecuteDdl(
      MakeRequest("ALTER TABLE ds.people ADD COLUMN age INT64"),
      bundle.catalog.get());
  ASSERT_FALSE(duplicate.ok());
  EXPECT_EQ(duplicate.code(), absl::StatusCode::kAlreadyExists) << duplicate;
}

TEST_F(DuckDBEngineTest, ExecuteDdlRejectsNullCatalog) {
  auto status = engine_->ExecuteDdl(
      MakeRequest("CREATE TABLE ds.empty (id INT64)"), nullptr);
  ASSERT_FALSE(status.ok());
  EXPECT_EQ(status.code(), absl::StatusCode::kFailedPrecondition) << status;
}

}  // namespace
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
