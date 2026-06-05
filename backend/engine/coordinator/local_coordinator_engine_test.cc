// Integration test for `LocalCoordinatorEngine`.
//
// We exercise the wired-up coordinator (analyze + classify + route +
// execute) one layer below the gRPC service boundary so the
// engine machinery flows end-to-end without spinning up the
// frontend. The fixture sets up a `DuckDBStorage`-backed `people`
// table and a `GoogleSqlCatalog`, then drives requests through
// the public `Engine` interface the gateway sees.
//
// Plan: `docs/ENGINE_POLICY.md` "Tests"
// section requires:
//
//   1. Pure `duckdb_native` SELECT round-trips through the
//      coordinator and produces the same wire output the legacy
//      `DuckDBEngine` path produced.
//   2. Unsupported function surfaces UNIMPLEMENTED via the
//      `UnsupportedExecutor` stub with a disposition-aware
//      message.
//   3. DDL (`CREATE TABLE` / CTAS / `DROP TABLE`) routes through
//      `kControlOp` to `backend/engine/control/control_op_executor`
//      and mutates `Storage` directly; ALTER TABLE keeps using
//      `DuckDbExecutor::ExecuteDdl` until a follow-up plan lands
//      it on the control-op executor. This regression-pins the
//      end-state every gateway/e2e DDL test depends on.
//
// The harness mirrors `duckdb_engine_test.cc` so a future merge of
// the legacy `DuckDBEngine` tests onto this fixture is mechanical.

#include "backend/engine/coordinator/local_coordinator_engine.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {

namespace fs = std::filesystem;

::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

class LocalCoordinatorEngineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-coordinator-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    engine_ = std::make_unique<LocalCoordinatorEngine>(storage_.get());
  }

  void TearDown() override {
    engine_.reset();
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

  // Standard two-column `people` table the SELECT round-trip
  // test reads from. Mirrors `duckdb_engine_test.cc`'s helper so
  // a side-by-side comparison of the two harnesses is one diff.
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
    std::unique_ptr<::googlesql::TypeFactory> type_factory{};
    std::unique_ptr<catalog::GoogleSqlCatalog> catalog{};
  };
  CatalogBundle MakeCatalog() {
    auto type_factory = std::make_unique<::googlesql::TypeFactory>();
    auto catalog = std::make_unique<catalog::GoogleSqlCatalog>(
        "proj-test", storage_.get(), type_factory.get(), MakeLanguageOptions());
    return {std::move(type_factory), std::move(catalog)};
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<LocalCoordinatorEngine> engine_{};
};

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
       ExecuteQueryUnsupportedFunctionRoutesToUnsupportedStub) {
  // `APPROX_QUANTILES` is `unsupported` in `functions.yaml` (not
  // planned). The classifier promotes the route to
  // `kUnsupported`, the coordinator dispatches to the
  // `UnsupportedExecutor` stub, and the stub returns UNIMPLEMENTED
  // with a disposition-aware message that names the route and
  // points at the policy. This pins the dispatcher's
  // unsupported-route behavior against the gateway's stable
  // `notImplemented` contract.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("SELECT APPROX_QUANTILES(id, 4) FROM ds.people"),
      bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kUnimplemented);
  EXPECT_TRUE(absl::StrContains(source.status().message(), "unsupported"))
      << source.status().message();
  EXPECT_TRUE(
      absl::StrContains(source.status().message(), "docs/ENGINE_POLICY.md"))
      << source.status().message();
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
// has no registry row so it falls through to `kDuckdbNative` and
// stays on `DuckDbExecutor::ExecuteDdl` until a future plan lands
// it on the control-op executor. The gateway/e2e suite leans on
// the same paths; pinning them at the engine surface lets a
// regression here surface as a unit-test failure first.
// ---------------------------------------------------------------------------

TEST_F(LocalCoordinatorEngineTest,
       ExecuteDmlMergeMatchedAndNotMatchedUpdatesStorage) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
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
  EXPECT_EQ(stats->inserted_row_count, 1);
  EXPECT_EQ(stats->updated_row_count, 1);
  EXPECT_EQ(stats->deleted_row_count, 0);

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

TEST_F(LocalCoordinatorEngineTest, ExecuteQueryScalarSelectRoutesToSemantic) {
  // Scalar-only SELECT (no FROM) classifies to
  // `kSemanticExecutor` after `docs/ENGINE_POLICY.md`
  // landed; the coordinator dispatches to the local
  // `semantic::SemanticExecutor`, which evaluates the expression
  // tree directly and returns a one-row Arrow batch matching the
  // DuckDB fast-path output shape. This pins the end-to-end
  // happy path for the scalar SELECT family.
  CatalogBundle bundle = MakeCatalog();
  auto source =
      engine_->ExecuteQuery(MakeRequest("SELECT 1 + 2"), bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  ASSERT_EQ((*source)->schema().columns.size(), 1u);
  EXPECT_EQ((*source)->schema().columns[0].type, schema::ColumnType::kInt64);
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  EXPECT_EQ(row.cells[0].int64_value(), 3);
  has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryScalarSelectDivisionByZeroSurfacesError) {
  // `SELECT 1.0 / 0` lowers through the semantic executor's
  // strict arithmetic; the error envelope carries an
  // INVALID_ARGUMENT status with the semantic
  // `kDivisionByZero` reason payload that the gateway maps onto
  // BigQuery's REST envelope.
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT 1.0 / 0"),
                                      bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryScalarSelectWithParameterBindsViaCoordinator) {
  // End-to-end coordinator wire-up for named parameters: the
  // request carries `@p = 41`, the coordinator declares the
  // parameter to the analyzer (so the resolved AST contains a
  // typed `ResolvedParameter`), and the semantic executor reads
  // the value off `request.parameters`.
  CatalogBundle bundle = MakeCatalog();
  QueryRequest req = MakeRequest("SELECT @p + 1");
  QueryParameter p;
  p.name = "p";
  p.type_kind = "INT64";
  p.value_json = "41";
  req.parameters.push_back(p);
  auto source = engine_->ExecuteQuery(req, bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  EXPECT_EQ(row.cells[0].int64_value(), 42);
}

TEST_F(LocalCoordinatorEngineTest, ExecuteQueryRejectsLegacySql) {
  CatalogBundle bundle = MakeCatalog();
  QueryRequest req = MakeRequest("SELECT 1");
  req.use_legacy_sql = true;
  auto source = engine_->ExecuteQuery(req, bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(LocalCoordinatorEngineTest, ExecuteQueryRejectsNullCatalog) {
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT 1"), nullptr);
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kFailedPrecondition);
}

// Pipe-operator DDL control-op routing tests.
// forms (`FROM ... |> EXPORT DATA ...` and `FROM ... |> CREATE
// TABLE ...`) arrive at the engine as a `ResolvedQueryStmt`
// whose body is a `ResolvedPipeExportDataScan` /
// `ResolvedPipeCreateTableScan` scan. The classifier routes
// them to the control-op surface, but
// `ControlOpExecutor::ExecuteQuery` rejects every
// ResolvedStatement (control-op is contractually a no-row-stream
// surface). The coordinator pre-dispatches these two shapes to
// `backend/engine/control/pipe_{export_data,create_table}.cc`
// so the per-shape UNIMPLEMENTED message reaches the gateway
// without going through the misleading executor error.
TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryPipeExportDataRoutesToControlOpHandler) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("FROM ds.people "
                  "|> EXPORT DATA OPTIONS (uri = 'gs://b/o.csv', "
                  "format = 'CSV')"),
      bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kUnimplemented);
  EXPECT_TRUE(
      absl::StrContains(source.status().message(), "pipe-form EXPORT DATA"))
      << source.status();
  EXPECT_TRUE(
      absl::StrContains(source.status().message(), "EXPORT DATA writer family"))
      << source.status();
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryPipeCreateTableRoutesToControlOpHandler) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("FROM ds.people |> CREATE TABLE ds.people_copy"),
      bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kUnimplemented);
  EXPECT_TRUE(
      absl::StrContains(source.status().message(), "pipe-form CREATE TABLE"))
      << source.status();
  EXPECT_TRUE(absl::StrContains(source.status().message(),
                                "pipe-form CREATE TABLE adapter"))
      << source.status();
}

TEST_F(LocalCoordinatorEngineTest, CreateFunctionThenCallScalarUdf) {
  CatalogBundle bundle = MakeCatalog();
  ASSERT_TRUE(engine_
                  ->ExecuteDdl(MakeRequest(R"(CREATE FUNCTION customfunc(
  arr ARRAY<STRUCT<name STRING, val INT64>>
) AS (
  (SELECT SUM(IF(elem.name = "foo",elem.val,null)) FROM UNNEST(arr) AS elem)
))"),
                               bundle.catalog.get())
                  .ok());
  CatalogBundle bundle2 = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest(R"(SELECT customfunc([
  STRUCT<name STRING, val INT64>("foo", 10),
  STRUCT<name STRING, val INT64>("bar", 40),
  STRUCT<name STRING, val INT64>("foo", 20)
]))"),
                                      bundle2.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
  EXPECT_EQ(row.cells[0].int64_value(), 30);
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
