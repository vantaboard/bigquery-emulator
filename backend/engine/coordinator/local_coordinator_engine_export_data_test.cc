#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/engine/control/avro_test_reader.h"
#include "backend/engine/coordinator/local_coordinator_engine_test_fixture.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {

using control::avro_test::AvroCursor;
using control::avro_test::AvroFile;
using control::avro_test::ReadAvroFile;

constexpr absl::string_view kUserHashFunction = R"sql(
CREATE TEMP FUNCTION user_hash(id STRING) AS ((
  SELECT ((SAFE_CAST(CONCAT('0x', SUBSTR(h, 1, 8)) AS INT64) ^ SAFE_CAST(CONCAT('0x', SUBSTR(h, 17, 8)) AS INT64)) << 32)
       | (SAFE_CAST(CONCAT('0x', SUBSTR(h, 9, 8)) AS INT64) ^ SAFE_CAST(CONCAT('0x', SUBSTR(h, 25, 8)) AS INT64))
  FROM (SELECT REPLACE(LOWER(id), '-', '') AS h)
  WHERE REGEXP_CONTAINS(h, r'^[0-9a-f]{32}$')
)))sql";

std::string SemanticQuery(bool with_sketch) {
  return absl::StrCat(R"sql(
WITH filtered AS (
  SELECT kind, CAST(event_day AS DATE) AS event_date, user_id
  FROM ds.events
  WHERE kind IN UNNEST(@kinds)
)
SELECT
  kind,
  CAST(event_date AS STRING) AS day,)sql",
                      with_sketch
                          ? "\n  HLL_COUNT.INIT(user_id, 15) AS sketch,"
                          : "",
                      R"sql(
  HLL_COUNT.EXTRACT(HLL_COUNT.INIT(user_id, 15)) AS users,
  MAX(user_hash(user_id)) AS max_hash,
  COUNT(*) AS n
FROM filtered
WHERE event_date >= @since
GROUP BY kind, event_date
ORDER BY kind, day)sql");
}

class LocalCoordinatorEngineExportDataTest : public LocalCoordinatorEngineTest {
 protected:
  void SetUp() override {
    LocalCoordinatorEngineTest::SetUp();
    std::error_code ec;
    fs::create_directories(ExportDir(), ec);
    ASSERT_FALSE(ec) << ec.message();
  }

  fs::path ExportDir() const { return data_dir_ / "exports"; }

  std::string ExportUri(absl::string_view file) const {
    return absl::StrCat("file://", (ExportDir() / std::string(file)).string());
  }

  void CreateEventsTable() {
    schema::TableSchema bq_schema;
    for (const auto& [name, type] :
         std::vector<std::pair<std::string, schema::ColumnType>>{
             {"kind", schema::ColumnType::kString},
             {"event_day", schema::ColumnType::kString},
             {"user_id", schema::ColumnType::kString},
         }) {
      schema::ColumnSchema column;
      column.name = name;
      column.type = type;
      column.mode = schema::ColumnMode::kNullable;
      bq_schema.columns.push_back(column);
    }
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
    ASSERT_TRUE(
        storage_->CreateTable({"proj-test", "ds", "events"}, bq_schema).ok());
    auto row = [](std::string kind, std::string day, std::string user) {
      storage::Row r;
      r.cells = {
          storage::Value::String(std::move(kind)),
          storage::Value::String(std::move(day)),
          storage::Value::String(std::move(user)),
      };
      return r;
    };
    std::vector<storage::Row> rows = {
        row("click", "2024-01-01", "00000000000000010000000000000002"),
        row("click", "2024-01-01", "00000000-0000-0001-0000-000000000002"),
        row("click", "2024-01-01", "00000001000000000000000000000000"),
        row("click", "2024-01-01", "not-a-user-id"),
        row("view", "2024-01-02", "00000000000000010000000000000002"),
        row("view", "2023-12-31", "00000001000000000000000000000000"),
        row("other", "2024-01-02", "00000001000000000000000000000000"),
    };
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "events"},
                                 absl::MakeConstSpan(rows))
                    .ok());
  }

  QueryRequest ExportRequest(absl::string_view options, bool with_sketch) {
    QueryRequest req = MakeRequest(absl::StrCat(
        "EXPORT DATA OPTIONS (", options, ") AS", SemanticQuery(with_sketch)));
    QueryParameter kinds;
    kinds.name = "kinds";
    kinds.type_kind = "ARRAY";
    kinds.type_json = "STRING";
    kinds.value_json = R"(["click","view"])";
    QueryParameter since;
    since.name = "since";
    since.type_kind = "DATE";
    since.value_json = "2024-01-01";
    req.parameters = {kinds, since};
    return req;
  }

  static std::string ReadFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
  }
};

TEST_F(LocalCoordinatorEngineExportDataTest,
       SemanticQueryWithTempFunctionExportsAvro) {
  CreateEventsTable();
  CatalogBundle bundle = MakeCatalog();
  absl::Status created = engine_->ExecuteDdl(MakeRequest(kUserHashFunction),
                                             bundle.catalog.get());
  ASSERT_TRUE(created.ok()) << created;
  CatalogBundle export_bundle = MakeCatalog();

  absl::Status exported = engine_->ExecuteDdl(
      ExportRequest(absl::StrCat("uri = '",
                                 ExportUri("part-*.avro"),
                                 "', format = 'AVRO', compression = 'SNAPPY', "
                                 "overwrite = true"),
                    /*with_sketch=*/true),
      export_bundle.catalog.get());
  ASSERT_TRUE(exported.ok()) << exported;

  AvroFile file =
      ReadAvroFile((ExportDir() / "part-000000000000.avro").string());
  EXPECT_EQ(file.codec, "snappy");
  EXPECT_EQ(file.schema,
            R"({"type":"record","name":"Root","fields":[)"
            R"({"name":"kind","type":["null","string"]},)"
            R"({"name":"day","type":["null","string"]},)"
            R"({"name":"sketch","type":["null","bytes"]},)"
            R"({"name":"users","type":["null","long"]},)"
            R"({"name":"max_hash","type":["null","long"]},)"
            R"({"name":"n","type":["null","long"]}]})");
  EXPECT_EQ(file.row_count, 2);
  AvroCursor cur(file.data);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "click");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "2024-01-01");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_FALSE(cur.ReadBytes().empty());
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 4);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 4294967296);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 4);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "view");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "2024-01-02");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_FALSE(cur.ReadBytes().empty());
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 3);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_TRUE(cur.AtEnd());
}

TEST_F(LocalCoordinatorEngineExportDataTest,
       SemanticQueryWithTempFunctionExportsCsv) {
  CreateEventsTable();
  CatalogBundle bundle = MakeCatalog();
  absl::Status created = engine_->ExecuteDdl(MakeRequest(kUserHashFunction),
                                             bundle.catalog.get());
  ASSERT_TRUE(created.ok()) << created;
  CatalogBundle export_bundle = MakeCatalog();

  absl::Status exported = engine_->ExecuteDdl(
      ExportRequest(
          absl::StrCat("uri = '", ExportUri("users.csv"), "', format = 'CSV'"),
          /*with_sketch=*/false),
      export_bundle.catalog.get());
  ASSERT_TRUE(exported.ok()) << exported;

  EXPECT_EQ(ReadFile(ExportDir() / "users.csv"),
            "kind,day,users,max_hash,n\n"
            "click,2024-01-01,4,4294967296,4\n"
            "view,2024-01-02,1,3,1\n");
}

TEST_F(LocalCoordinatorEngineExportDataTest, DuckDbQueryStillExportsCsv) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  absl::Status exported = engine_->ExecuteDdl(
      MakeRequest(absl::StrCat("EXPORT DATA OPTIONS (uri = '",
                               ExportUri("people.csv"),
                               "', format = 'CSV') AS "
                               "SELECT id, name FROM ds.people ORDER BY id")),
      bundle.catalog.get());
  ASSERT_TRUE(exported.ok()) << exported;

  EXPECT_EQ(ReadFile(ExportDir() / "people.csv"),
            "id,name\n1,ada\n2,linus\n3,grace\n");
}

TEST_F(LocalCoordinatorEngineExportDataTest,
       ScriptConstantReachesSemanticExport) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest(absl::StrCat("CREATE CONSTANT min_id = 2;\n"
                               "EXPORT DATA OPTIONS (uri = '",
                               ExportUri("script.csv"),
                               "', format = 'CSV') AS "
                               "SELECT id, REVERSE(name) AS eman "
                               "FROM ds.people WHERE id >= min_id "
                               "ORDER BY id;")),
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  EXPECT_EQ(ReadFile(ExportDir() / "script.csv"),
            "id,eman\n2,sunil\n3,ecarg\n");
}

TEST_F(LocalCoordinatorEngineExportDataTest,
       ControlFlowScriptVariableReachesSemanticExport) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest(absl::StrCat("DECLARE n INT64 DEFAULT 2;\n"
                               "IF n > 1 THEN\n"
                               "  EXPORT DATA OPTIONS (uri = '",
                               ExportUri("branch.csv"),
                               "', format = 'CSV') AS "
                               "SELECT id, REVERSE(name) AS eman "
                               "FROM ds.people WHERE id <= n ORDER BY id;\n"
                               "END IF;")),
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  EXPECT_EQ(ReadFile(ExportDir() / "branch.csv"),
            "id,eman\n1,ada\n2,sunil\n");
}

TEST_F(LocalCoordinatorEngineExportDataTest, ScriptDuckDbExportExportsCsv) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest(absl::StrCat("DECLARE n INT64 DEFAULT 2;\n"
                               "IF n > 1 THEN\n"
                               "  EXPORT DATA OPTIONS (uri = '",
                               ExportUri("duckdb.csv"),
                               "', format = 'CSV') AS "
                               "SELECT id, name FROM ds.people ORDER BY id;\n"
                               "END IF;")),
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  EXPECT_EQ(ReadFile(ExportDir() / "duckdb.csv"),
            "id,name\n1,ada\n2,linus\n3,grace\n");
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
