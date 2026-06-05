// LocalCoordinatorEngine integration tests: semantic routing and pipe DDL.

#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "backend/engine/coordinator/local_coordinator_engine_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {
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
