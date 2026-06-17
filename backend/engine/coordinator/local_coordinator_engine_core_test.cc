// LocalCoordinatorEngine integration tests: analyze, dry-run, DDL, DML.

#include <algorithm>
#include <map>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "backend/engine/coordinator/local_coordinator_engine_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {
TEST_F(LocalCoordinatorEngineTest, AnalyzeSelectStarReflectsSchema) {
  // `Analyze` does not invoke the router; it just resolves the
  // statement and reflects the output schema. Pin that the
  // coordinator's `Analyze` returns the same shape the
  // `DuckDBEngine` returned for the same query.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = engine_->Analyze(
      MakeRequest("SELECT id, name FROM ds.people"), bundle.catalog.get());
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const schema::TableSchema& s = (*analyzed)->output_schema();
  ASSERT_EQ(s.columns.size(), 2u);
  EXPECT_EQ(s.columns[0].name, "id");
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(s.columns[1].name, "name");
  EXPECT_EQ(s.columns[1].type, schema::ColumnType::kString);
}

TEST_F(LocalCoordinatorEngineTest, DryRunSelectStarReturnsSchemaAndZeroBytes) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto dry_run = engine_->DryRun(MakeRequest("SELECT * FROM ds.people"),
                                 bundle.catalog.get());
  ASSERT_TRUE(dry_run.ok()) << dry_run.status();
  ASSERT_EQ(dry_run->schema.columns.size(), 2u);
  EXPECT_EQ(dry_run->estimated_bytes_processed, 0);
}

TEST_F(LocalCoordinatorEngineTest, ExecuteQuerySelectStarRoundTripsViaDuckDb) {
  // This is the "representative SELECT round-trip" the plan's
  // Tests section requires. The classifier picks `kDuckdbNative`
  // for the pure-DuckDB shape, the coordinator dispatches to the
  // `DuckDbExecutor`, and the row stream comes back with the same
  // shape `Storage::AppendRows` saw on the write path.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT * FROM ds.people"),
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
  ASSERT_EQ(seen.size(), 3u);
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  std::sort(seen.begin(), seen.end());
  std::sort(want.begin(), want.end());
  EXPECT_EQ(seen, want);
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQuerySessionUserStubReturnsPlaceholder) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source =
      engine_->ExecuteQuery(MakeRequest("SELECT SESSION_USER() FROM ds.people"),
                            bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  int rows_seen = 0;
  storage::Row row;
  while (true) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 1u);
    ASSERT_FALSE(row.cells[0].is_null());
    EXPECT_EQ(row.cells[0].string_value(), "bigquery-emulator@local");
    ++rows_seen;
  }
  EXPECT_EQ(rows_seen, 3);
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteDdlCreateTableRoutesThroughControlOp) {
  // `ResolvedCreateTableStmt` is `disposition=control_op` (no
  // `status=planned`) in `node_dispositions.yaml`. The classifier
  // routes the root statement to `kControlOp` and the coordinator
  // dispatches CREATE TABLE through
  // `backend/engine/control/control_op_executor.cc::RunCreateTable`
  // -- which mutates the `Storage` backend directly. This pins the
  // gateway/e2e/ddl_create_drop test's expected behavior against
  // the coordinator-aware engine.
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest("CREATE TABLE ds.new_table (a INT64, b STRING)"),
      bundle.catalog.get());
  EXPECT_TRUE(status.ok()) << status;
  // The table should exist on the storage backend post-DDL; the
  // schema lookup is the cheapest "table exists" probe Storage
  // exposes (see `backend/storage/storage.h::GetSchema`).
  auto schema = storage_->GetSchema({"proj-test", "ds", "new_table"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  EXPECT_EQ(schema->columns.size(), 2u);
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteDdlCreateTableWithStructColumnRoutesThroughControlOp) {
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(
      MakeRequest("CREATE TABLE ds.t (k INT64, s STRUCT<a INT64, b STRING>)"),
      bundle.catalog.get());
  ASSERT_TRUE(status.ok()) << status;
  auto schema = storage_->GetSchema({"proj-test", "ds", "t"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);
  EXPECT_EQ(schema->columns[1].type, schema::ColumnType::kStruct);
  ASSERT_EQ(schema->columns[1].fields.size(), 2u);
}

// ---------------------------------------------------------------------------
// MERGE / DDL coverage migrated from the legacy `duckdb_engine_test.cc`.
//
// These pin the through-the-coordinator behavior of every shape
// the legacy `DuckDBEngine` used to own end-to-end. MERGE
// classifies to `kDuckdbNative` and dispatches through the
// `DuckDbExecutor`; CREATE TABLE / CTAS / DROP TABLE classify to
// `kControlOp` and dispatch through
// `backend/engine/control/control_op_executor.cc`; ALTER TABLE
// classifies to `kControlOp` and dispatches through
// `RunAlterTable`. The gateway/e2e suite leans on
// the same paths; pinning them at the engine surface lets a
// regression here surface as a unit-test failure first.
// ---------------------------------------------------------------------------

TEST_F(LocalCoordinatorEngineTest,
       ExecuteDmlMergeMatchedAndNotMatchedUpdatesStorage) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto result = engine_->ExecuteDml(
      MakeRequest("MERGE INTO ds.people T USING ("
                  "  SELECT 2 AS id, 'linus-updated' AS name "
                  "  UNION ALL "
                  "  SELECT 4 AS id, 'rust' AS name) S "
                  "ON T.id = S.id "
                  "WHEN MATCHED THEN UPDATE SET name = S.name "
                  "WHEN NOT MATCHED THEN INSERT (id, name) "
                  "VALUES (S.id, S.name)"),
      bundle.catalog.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.inserted_row_count, 1);
  EXPECT_EQ(result->stats.updated_row_count, 1);
  EXPECT_EQ(result->stats.deleted_row_count, 0);

  auto scan = storage_->ScanRows({"proj-test", "ds", "people"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  std::map<int64_t, std::string> by_id;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    by_id[row.cells[0].int64_value()] = row.cells[1].string_value();
  }
  EXPECT_EQ(by_id.size(), 4u);
  EXPECT_EQ(by_id[1], "ada");
  EXPECT_EQ(by_id[2], "linus-updated");
  EXPECT_EQ(by_id[3], "grace");
  EXPECT_EQ(by_id[4], "rust");
}

TEST_F(LocalCoordinatorEngineTest, ExecuteDdlCreateTableAsSelectRoundTrips) {
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

// Regression for the recidiviz-fork report: "querying the view itself
// returned nothing". Creating a view registers it as a SimpleSQLView;
// reading from it must inline the view's definition (REWRITE_INLINE_
// SQL_VIEWS) so the underlying base-table rows flow through. Without
// the rewrite the view scan resolves to a storage table that does not
// exist and the read silently yields zero rows.
TEST_F(LocalCoordinatorEngineTest, ExecuteQueryReadsRowsThroughView) {
  CreatePeopleTable();
  CatalogBundle ddl_bundle = MakeCatalog();
  auto created = engine_->ExecuteDdl(
      MakeRequest("CREATE VIEW ds.people_view AS "
                  "SELECT id, name FROM ds.people WHERE id >= 2"),
      ddl_bundle.catalog.get());
  ASSERT_TRUE(created.ok()) << created;

  // Fresh catalog for the read, exactly like a separate query RPC: the
  // view must be discoverable via the (global) view registry and its
  // definition inlined, not read back from storage.
  CatalogBundle read_bundle = MakeCatalog();
  auto source =
      engine_->ExecuteQuery(MakeRequest("SELECT id, name FROM ds.people_view"),
                            read_bundle.catalog.get());
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
  std::sort(seen.begin(), seen.end());
  std::vector<std::pair<int64_t, std::string>> want = {{2, "linus"},
                                                       {3, "grace"}};
  EXPECT_EQ(seen, want);
}

TEST_F(LocalCoordinatorEngineTest, ExecuteDdlDropTableRemovesStorage) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto status = engine_->ExecuteDdl(MakeRequest("DROP TABLE ds.people"),
                                    bundle.catalog.get());
  ASSERT_TRUE(status.ok()) << status;
  auto sch = storage_->GetSchema({"proj-test", "ds", "people"});
  ASSERT_FALSE(sch.ok());
  EXPECT_EQ(sch.status().code(), absl::StatusCode::kNotFound) << sch.status();
}

TEST_F(LocalCoordinatorEngineTest, ExecuteDdlAlterTableAddColumnPadsRows) {
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
  auto scan = storage_->ScanRows({"proj-test", "ds", "people"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  int rows_seen = 0;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 3u);
    EXPECT_EQ(row.cells[2].kind(), storage::Value::Kind::kNull);
    ++rows_seen;
  }
  EXPECT_EQ(rows_seen, 3);
}
}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
