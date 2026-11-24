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
#include "backend/engine/control/avro_test_reader.h"
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
using avro_test::AvroCursor;
using avro_test::AvroFile;
using avro_test::ReadAvroFile;

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
  options.CreateDefaultArenasIfNotSet();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

schema::ColumnSchema Column(std::string name,
                            schema::ColumnType type,
                            schema::ColumnMode mode) {
  schema::ColumnSchema c;
  c.name = std::move(name);
  c.type = type;
  c.mode = mode;
  return c;
}

constexpr absl::string_view kItemsSchema =
    R"({"type":"record","name":"Root","fields":[)"
    R"({"name":"id","type":["null","long"]},)"
    R"({"name":"name","type":["null","string"]},)"
    R"({"name":"payload","type":["null","bytes"]},)"
    R"({"name":"day","type":["null",{"type":"int","logicalType":"date"}]},)"
    R"({"name":"tags","type":{"type":"array","items":"string"}}]})";

class ExportDataAvroTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmp = std::getenv("TEST_TMPDIR");
    std::random_device rd;
    root_ = fs::path(tmp != nullptr ? tmp : "/tmp") /
            absl::StrCat("bqemu-export-avro-", rd());
    std::error_code ec;
    fs::remove_all(root_, ec);
    fs::create_directories(root_ / "out", ec);
    auto opened = storage::duckdb::DuckDBStorage::Open((root_ / "db").string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    executor_ = std::make_unique<ControlOpExecutor>(storage_.get());
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  }

  void TearDown() override {
    executor_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(root_, ec);
  }

  void CreateItemsTable() {
    using schema::ColumnMode;
    using schema::ColumnType;
    schema::TableSchema s;
    s.columns = {
        Column("id", ColumnType::kInt64, ColumnMode::kRequired),
        Column("name", ColumnType::kString, ColumnMode::kNullable),
        Column("day", ColumnType::kDate, ColumnMode::kNullable),
        Column("tags", ColumnType::kString, ColumnMode::kRepeated),
    };
    ASSERT_TRUE(storage_->CreateTable({"proj-test", "ds", "items"}, s).ok());
    using storage::Value;
    std::vector<storage::Row> rows(2);
    rows[0].cells = {
        Value::Int64(1),
        Value::String("ada"),
        Value::String("2024-01-02"),
        Value::Array({Value::String("a"), Value::String("b")}),
    };
    rows[1].cells = {
        Value::Int64(2),
        Value::Null(),
        Value::Null(),
        Value::Array({}),
    };
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "items"},
                                 absl::MakeConstSpan(rows))
                    .ok());
  }

  void CreateBulkTable(int n) {
    schema::TableSchema s;
    s.columns = {
        Column("id", schema::ColumnType::kInt64, schema::ColumnMode::kNullable),
        Column(
            "name", schema::ColumnType::kString, schema::ColumnMode::kNullable),
    };
    ASSERT_TRUE(storage_->CreateTable({"proj-test", "ds", "bulk"}, s).ok());
    std::vector<storage::Row> rows(n);
    for (int i = 0; i < n; ++i) {
      rows[i].cells = {storage::Value::Int64(i),
                       storage::Value::String(BulkName(i))};
    }
    ASSERT_TRUE(
        storage_
            ->AppendRows({"proj-test", "ds", "bulk"}, absl::MakeConstSpan(rows))
            .ok());
  }

  void CreateAmountsTable() {
    schema::TableSchema s;
    s.columns = {Column(
        "amount", schema::ColumnType::kNumeric, schema::ColumnMode::kNullable)};
    ASSERT_TRUE(storage_->CreateTable({"proj-test", "ds", "amounts"}, s).ok());
    std::vector<storage::Row> rows(1);
    rows[0].cells = {storage::Value::String("-1.5")};
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "amounts"},
                                 absl::MakeConstSpan(rows))
                    .ok());
  }

  static std::string BulkName(int i) {
    return absl::StrCat("customer-name-", i % 7, "-customer-name-", i % 3);
  }

  absl::Status Run(absl::string_view sql) {
    auto type_factory = std::make_unique<::googlesql::TypeFactory>();
    catalog::GoogleSqlCatalog catalog(
        "proj-test", storage_.get(), type_factory.get(), MakeLanguageOptions());
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    ::googlesql::TypeFactory analyzer_types;
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status analyze = ::googlesql::AnalyzeStatement(
        sql, options, &catalog, &analyzer_types, &output);
    if (!analyze.ok()) return analyze;
    QueryRequest req;
    req.project_id = "proj-test";
    req.sql = std::string(sql);
    return executor_->ExecuteDdl(req, *output->resolved_statement(), &catalog);
  }

  absl::Status Export(absl::string_view file,
                      absl::string_view extra_options,
                      absl::string_view query) {
    return Run(absl::StrCat("EXPORT DATA OPTIONS(uri='file://",
                            (root_ / "out" / std::string(file)).string(),
                            "', format='AVRO'",
                            extra_options,
                            ") AS ",
                            query));
  }

  std::string OutPath(absl::string_view file) const {
    return (root_ / "out" / std::string(file)).string();
  }

  void ExpectItemsRows(const AvroFile& file) {
    EXPECT_EQ(file.schema, kItemsSchema);
    EXPECT_EQ(file.row_count, 2);
    AvroCursor cur(file.data);
    EXPECT_EQ(cur.ReadLong(), 1);
    EXPECT_EQ(cur.ReadLong(), 1);
    EXPECT_EQ(cur.ReadLong(), 1);
    EXPECT_EQ(cur.ReadBytes(), "ada");
    EXPECT_EQ(cur.ReadLong(), 1);
    EXPECT_EQ(cur.ReadBytes(), std::string("\x00\x01\xff", 3));
    EXPECT_EQ(cur.ReadLong(), 1);
    EXPECT_EQ(cur.ReadLong(), 19724);
    EXPECT_EQ(cur.ReadStringArray(), (std::vector<std::string>{"a", "b"}));
    EXPECT_EQ(cur.ReadLong(), 1);
    EXPECT_EQ(cur.ReadLong(), 2);
    EXPECT_EQ(cur.ReadLong(), 0);
    EXPECT_EQ(cur.ReadLong(), 0);
    EXPECT_EQ(cur.ReadLong(), 0);
    EXPECT_TRUE(cur.ReadStringArray().empty());
    EXPECT_TRUE(cur.AtEnd());
  }

  void ExpectBulkRows(const AvroFile& file, int n) {
    EXPECT_EQ(file.row_count, n);
    AvroCursor cur(file.data);
    for (int i = 0; i < n; ++i) {
      ASSERT_EQ(cur.ReadLong(), 1);
      ASSERT_EQ(cur.ReadLong(), i);
      ASSERT_EQ(cur.ReadLong(), 1);
      ASSERT_EQ(cur.ReadBytes(), BulkName(i));
    }
    EXPECT_TRUE(cur.AtEnd());
  }

  fs::path root_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<ControlOpExecutor> executor_{};
};

TEST_F(ExportDataAvroTest, WritesUncompressedContainerWithRows) {
  CreateItemsTable();
  absl::Status s =
      Export("items.avro",
             "",
             "SELECT id, name, IF(id = 1, b'\\x00\\x01\\xff', NULL) AS "
             "payload, day, tags FROM ds.items "
             "ORDER BY id");
  ASSERT_TRUE(s.ok()) << s;
  AvroFile file = ReadAvroFile(OutPath("items.avro"));
  EXPECT_EQ(file.codec, "null");
  ExpectItemsRows(file);
}

TEST_F(ExportDataAvroTest, WildcardUriWritesFirstShard) {
  CreateItemsTable();
  absl::Status s =
      Export("items-*.avro",
             ", overwrite=true",
             "SELECT id, name, IF(id = 1, b'\\x00\\x01\\xff', NULL) AS "
             "payload, day, tags FROM ds.items "
             "ORDER BY id");
  ASSERT_TRUE(s.ok()) << s;
  ExpectItemsRows(ReadAvroFile(OutPath("items-000000000000.avro")));
}

TEST_F(ExportDataAvroTest, DeflateCodec) {
  CreateBulkTable(500);
  absl::Status s = Export("bulk.avro",
                          ", compression='DEFLATE'",
                          "SELECT id, name FROM ds.bulk ORDER BY id");
  ASSERT_TRUE(s.ok()) << s;
  AvroFile file = ReadAvroFile(OutPath("bulk.avro"));
  EXPECT_EQ(file.codec, "deflate");
  ExpectBulkRows(file, 500);
}

TEST_F(ExportDataAvroTest, SnappyCodec) {
  CreateBulkTable(500);
  absl::Status s = Export("bulk.avro",
                          ", compression='SNAPPY'",
                          "SELECT id, name FROM ds.bulk ORDER BY id");
  ASSERT_TRUE(s.ok()) << s;
  AvroFile file = ReadAvroFile(OutPath("bulk.avro"));
  EXPECT_EQ(file.codec, "snappy");
  ExpectBulkRows(file, 500);
}

TEST_F(ExportDataAvroTest, GzipIsRejected) {
  CreateItemsTable();
  absl::Status s =
      Export("items.avro", ", compression='GZIP'", "SELECT id FROM ds.items");
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument) << s;
  EXPECT_NE(s.message().find("compression 'GZIP' is not supported for AVRO"),
            absl::string_view::npos)
      << s;
}

constexpr absl::string_view kRichQuery =
    "SELECT STRUCT(1 AS a, 'x' AS b) AS s, "
    "TIMESTAMP '2024-01-02 03:04:05.123456+00' AS ts, "
    "amount AS n, 2.5 AS f, TRUE AS ok, "
    "DATETIME '2024-01-02 03:04:05' AS dt, TIME '01:02:03' AS t "
    "FROM ds.amounts";

TEST_F(ExportDataAvroTest, LogicalTypesStructAndDecimal) {
  CreateAmountsTable();
  absl::Status s = Export("rich.avro", "", kRichQuery);
  ASSERT_TRUE(s.ok()) << s;
  AvroFile file = ReadAvroFile(OutPath("rich.avro"));
  EXPECT_EQ(
      file.schema,
      R"({"type":"record","name":"Root","fields":[)"
      R"({"name":"s","type":["null",{"type":"record","name":"s",)"
      R"("namespace":"root","fields":[{"name":"a","type":["null","long"]},)"
      R"({"name":"b","type":["null","string"]}]}]},)"
      R"({"name":"ts","type":["null",)"
      R"({"type":"long","logicalType":"timestamp-micros"}]},)"
      R"({"name":"n","type":["null",{"type":"bytes","logicalType":"decimal",)"
      R"("precision":38,"scale":9}]},)"
      R"({"name":"f","type":["null","double"]},)"
      R"({"name":"ok","type":["null","boolean"]},)"
      R"({"name":"dt","type":["null",)"
      R"({"type":"string","logicalType":"datetime"}]},)"
      R"({"name":"t","type":["null",)"
      R"({"type":"long","logicalType":"time-micros"}]}]})");
  EXPECT_EQ(file.row_count, 1);
  AvroCursor cur(file.data);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "x");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 1704164645123456);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), std::string("\xa6\x97\xd1\x00", 4));
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadDouble(), 2.5);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_TRUE(cur.ReadBool());
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "2024-01-02T03:04:05");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadLong(), 3723000000);
  EXPECT_TRUE(cur.AtEnd());
}

TEST_F(ExportDataAvroTest, LogicalTypesDisabledUsesStrings) {
  CreateItemsTable();
  absl::Status s = Export("plain.avro",
                          ", use_avro_logical_types=false",
                          "SELECT day, TIMESTAMP '2024-01-02 03:04:05.5+00' "
                          "AS ts, TIME '01:02:03' AS t FROM ds.items "
                          "WHERE id = 1");
  ASSERT_TRUE(s.ok()) << s;
  AvroFile file = ReadAvroFile(OutPath("plain.avro"));
  EXPECT_EQ(file.schema,
            R"({"type":"record","name":"Root","fields":[)"
            R"({"name":"day","type":["null","string"]},)"
            R"({"name":"ts","type":["null","string"]},)"
            R"({"name":"t","type":["null","string"]}]})");
  AvroCursor cur(file.data);
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "2024-01-02");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "2024-01-02T03:04:05.500000Z");
  EXPECT_EQ(cur.ReadLong(), 1);
  EXPECT_EQ(cur.ReadBytes(), "01:02:03");
  EXPECT_TRUE(cur.AtEnd());
}

}  // namespace
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
