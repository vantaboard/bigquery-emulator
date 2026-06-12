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

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryNamedParameterP0LabelBindsViaCoordinator) {
  CatalogBundle bundle = MakeCatalog();
  QueryRequest req = MakeRequest("SELECT @p0 + @p1");
  QueryParameter p0;
  p0.name = "p0";
  p0.type_kind = "INT64";
  p0.value_json = "40";
  QueryParameter p1;
  p1.name = "p1";
  p1.type_kind = "INT64";
  p1.value_json = "2";
  req.parameters.push_back(p0);
  req.parameters.push_back(p1);
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

// Pipe-operator DDL control-op routing: pipe EXPORT DATA is rejected
// by the export handler (cloud-storage URIs are unsupported); pipe
// CREATE TABLE materializes the pipe input into a new table.
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
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_TRUE(absl::StrContains(source.status().message(),
                                "cloud-storage URI 'gs://b/o.csv'"))
      << source.status();
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryPipeCreateTableRoutesToControlOpHandler) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("FROM ds.people |> CREATE TABLE ds.people_copy"),
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  auto count_or = storage_->CountRows({"proj-test", "ds", "people_copy"});
  ASSERT_TRUE(count_or.ok()) << count_or.status();
  EXPECT_EQ(*count_or, 3);
}

TEST_F(LocalCoordinatorEngineTest, CreateFromHexBqutilsFixtureUdf) {
  CatalogBundle bundle = MakeCatalog();
  const absl::Status ddl =
      engine_->ExecuteDdl(MakeRequest(R"(CREATE FUNCTION from_hex(value STRING) 

AS
(
  (
    SELECT 
      SUM(
      	CAST(
      	  CONCAT('0x', SUBSTR(value, byte * 2 + 1, 2)) 
      	    AS INT64) << ((LENGTH(value) - (byte + 1) * 2) * 4))
    FROM UNNEST(GENERATE_ARRAY(1, LENGTH(value) / 2)) WITH OFFSET byte
  )
);)"),
                          bundle.catalog.get());
  ASSERT_TRUE(ddl.ok()) << ddl;
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

TEST_F(LocalCoordinatorEngineTest, CreateFromHexFixtureUdf) {
  CatalogBundle bundle = MakeCatalog();
  const std::string sql = R"(/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

-- from_hex:
-- Input: STRING representing a number in hexadecimal form
-- Output: INT64 number in decimal form
CREATE FUNCTION from_hex(value STRING) 

AS
(
  (
    SELECT 
      SUM(
      	CAST(
      	  CONCAT('0x', SUBSTR(value, byte * 2 + 1, 2)) 
      	    AS INT64) << ((LENGTH(value) - (byte + 1) * 2) * 4))
    FROM UNNEST(GENERATE_ARRAY(1, LENGTH(value) / 2)) WITH OFFSET byte
  )
);)";
  ASSERT_TRUE(engine_->ExecuteDdl(MakeRequest(sql), bundle.catalog.get()).ok());
  CatalogBundle bundle2 = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest(R"(SELECT from_hex("000000000001e240"))"),
      bundle2.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells[0].int64_value(), 123456);
}

TEST_F(LocalCoordinatorEngineTest, CreateAnyTypeUdfShadowsBuiltinThenCalls) {
  CatalogBundle bundle = MakeCatalog();
  ASSERT_TRUE(engine_
                  ->ExecuteDdl(MakeRequest(R"(CREATE FUNCTION nullifzero(
  expr ANY TYPE
) AS (
  IF(CAST(expr AS INT64) = 0, NULL, expr)
))"),
                               bundle.catalog.get())
                  .ok());
  CatalogBundle bundle2 = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("SELECT nullifzero(CAST(0 AS INT64)) IS NULL"),
      bundle2.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kBool);
  EXPECT_TRUE(row.cells[0].bool_value());
}

TEST_F(LocalCoordinatorEngineTest, CreateAnyTypeUdfCastsStringToFloat) {
  CatalogBundle bundle = MakeCatalog();
  ASSERT_TRUE(
      engine_
          ->ExecuteDdl(MakeRequest(R"(CREATE FUNCTION int(v ANY TYPE) AS (
  CAST(FLOOR(CAST(v AS FLOAT64)) AS INT64)
))"),
                       bundle.catalog.get())
          .ok());
  CatalogBundle bundle2 = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest(R"(SELECT int("-1"))"),
                                      bundle2.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
  EXPECT_EQ(row.cells[0].int64_value(), -1);
}

TEST_F(LocalCoordinatorEngineTest,
       CorrelatedSubqueryWithTableAliasRoutesToSemanticAndFilters) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source =
      engine_->ExecuteQuery(MakeRequest("SELECT id FROM ds.people profiles "
                                        "WHERE (SELECT COUNT(*) FROM ds.people "
                                        "AS o WHERE o.id = profiles.id) "
                                        ">= 1 "
                                        "ORDER BY id"),
                            bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  std::vector<int64_t> ids;
  while (true) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 1u);
    ids.push_back(row.cells[0].int64_value());
  }
  ASSERT_EQ(ids.size(), 3u);
  EXPECT_EQ(ids[0], 1);
  EXPECT_EQ(ids[1], 2);
  EXPECT_EQ(ids[2], 3);
}

TEST_F(LocalCoordinatorEngineTest, CreateFromHexBqutilsFixtureViaExecuteDdl) {
  CatalogBundle bundle = MakeCatalog();
  const std::string sql = R"(/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

-- from_hex:
CREATE FUNCTION from_hex(value STRING) 

AS
(
  (
    SELECT 
      SUM(
      	CAST(
      	  CONCAT('0x', SUBSTR(value, byte * 2 + 1, 2)) 
      	    AS INT64) << ((LENGTH(value) - (byte + 1) * 2) * 4))
    FROM UNNEST(GENERATE_ARRAY(1, LENGTH(value) / 2)) WITH OFFSET byte
  )
);)";
  absl::Status created =
      engine_->ExecuteDdl(MakeRequest(sql), bundle.catalog.get());
  ASSERT_TRUE(created.ok()) << created;
  CatalogBundle bundle2 = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest(R"(SELECT from_hex("000000000001e240"))"),
      bundle2.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
  EXPECT_EQ(row.cells[0].int64_value(), 123456);
}
}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
