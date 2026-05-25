// Unit tests for the Arrow-flavored chunk -> BigQuery cell converter.
// The tests run a handful of canonical query shapes through a real
// in-memory DuckDB connection (using the libduckdb C API the engine
// itself drives) and assert that each fetched data chunk lowers to
// the same `storage::Value` shape the reference-impl engine returns.
//
// We exercise the converter against the chunk API rather than against
// the higher-level DuckDBEngine so the test does not pull in
// GoogleSQL: arrow_to_bq is purely a libduckdb-on-top-of-schema
// helper. The DuckDBEngine integration is covered separately by
// `duckdb_engine_test.cc` and by the `gateway/e2e/query_duckdb_*`
// integration suite.

#include "backend/engine/duckdb/arrow_to_bq.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace arrow_to_bq {
namespace {

// Opens a fresh in-memory DuckDB database + connection for each test.
// The destructor tears them down in the right order (connection
// before database) so the per-test state never leaks.
class ArrowToBqTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(::duckdb_open(nullptr, &db_), ::DuckDBSuccess);
    ASSERT_EQ(::duckdb_connect(db_, &conn_), ::DuckDBSuccess);
  }
  void TearDown() override {
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
  }

  // Runs `sql` and returns a vector of fetched rows, each rendered
  // against `output_schema`. The helper exhausts every chunk via
  // duckdb_fetch_chunk so the per-test assertions can read the full
  // result without juggling chunk lifetime.
  std::vector<storage::Row> RunAndFetch(absl::string_view sql,
                                         const schema::TableSchema& schema) {
    ::duckdb_result result;
    const std::string sql_str(sql);
    EXPECT_EQ(::duckdb_query(conn_, sql_str.c_str(), &result),
              ::DuckDBSuccess)
        << ::duckdb_result_error(&result);
    std::vector<storage::Row> rows;
    while (true) {
      ::duckdb_data_chunk chunk = ::duckdb_fetch_chunk(result);
      if (chunk == nullptr) break;
      const ::idx_t size = ::duckdb_data_chunk_get_size(chunk);
      for (::idx_t r = 0; r < size; ++r) {
        auto row = ChunkRowToCells(chunk, r, schema);
        EXPECT_TRUE(row.ok()) << row.status();
        if (row.ok()) rows.push_back(std::move(row).value());
      }
      ::duckdb_destroy_data_chunk(&chunk);
    }
    ::duckdb_destroy_result(&result);
    return rows;
  }

  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

// SELECT 1 against a single-column INT64 schema is the simplest
// shape: an INT64 vector with one element, no validity mask, and
// the rendered cell carries the analyzer's int64 value untouched.
TEST_F(ArrowToBqTest, RendersBigintAsInt64Cell) {
  schema::TableSchema s;
  schema::ColumnSchema col;
  col.name = "n";
  col.type = schema::ColumnType::kInt64;
  s.columns.push_back(col);

  std::vector<storage::Row> rows =
      RunAndFetch("SELECT CAST(42 AS BIGINT) AS n", s);
  ASSERT_EQ(rows.size(), 1u);
  ASSERT_EQ(rows[0].cells.size(), 1u);
  EXPECT_EQ(rows[0].cells[0].kind(), storage::Value::Kind::kInt64);
  EXPECT_EQ(rows[0].cells[0].int64_value(), 42);
}

// VARCHAR cells round-trip through duckdb_string_t's inline-or-pointer
// storage. We exercise both: "ada" fits in the 12-byte inline slot,
// and "abracadabra-extra-long-string" lives behind the pointer leg.
TEST_F(ArrowToBqTest, RendersVarcharBothInlineAndPointer) {
  schema::TableSchema s;
  schema::ColumnSchema col;
  col.name = "name";
  col.type = schema::ColumnType::kString;
  s.columns.push_back(col);

  std::vector<storage::Row> rows = RunAndFetch(
      "SELECT 'ada' UNION ALL SELECT 'abracadabra-extra-long-string' "
      "ORDER BY 1",
      s);
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_EQ(rows[0].cells[0].kind(), storage::Value::Kind::kString);
  EXPECT_EQ(rows[0].cells[0].string_value(),
            "abracadabra-extra-long-string");
  EXPECT_EQ(rows[1].cells[0].kind(), storage::Value::Kind::kString);
  EXPECT_EQ(rows[1].cells[0].string_value(), "ada");
}

// NULL cells must be detected via the validity mask, not via a raw
// data-pointer read. The fixture column is NULLABLE so the analyzer
// allocates a mask we can exercise.
TEST_F(ArrowToBqTest, NullValidityMaskProducesNullCell) {
  schema::TableSchema s;
  schema::ColumnSchema col;
  col.name = "v";
  col.type = schema::ColumnType::kInt64;
  s.columns.push_back(col);

  std::vector<storage::Row> rows = RunAndFetch(
      "SELECT CAST(NULL AS BIGINT) UNION ALL SELECT CAST(7 AS BIGINT) "
      "ORDER BY 1 NULLS FIRST",
      s);
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_TRUE(rows[0].cells[0].is_null());
  EXPECT_EQ(rows[1].cells[0].kind(), storage::Value::Kind::kInt64);
  EXPECT_EQ(rows[1].cells[0].int64_value(), 7);
}

// BOOL / FLOAT64 / DATE / TIMESTAMP cover the per-type renderings
// every BigQuery cell shape relies on. We pin the exact string
// formats for DATE / TIMESTAMP because those are what the gateway
// passes through verbatim to the REST wire envelope.
TEST_F(ArrowToBqTest, RendersScalarTypeFamily) {
  schema::TableSchema s;
  schema::ColumnSchema flag;
  flag.name = "flag";
  flag.type = schema::ColumnType::kBool;
  schema::ColumnSchema score;
  score.name = "score";
  score.type = schema::ColumnType::kFloat64;
  schema::ColumnSchema dt;
  dt.name = "dt";
  dt.type = schema::ColumnType::kDate;
  schema::ColumnSchema ts;
  ts.name = "ts";
  ts.type = schema::ColumnType::kTimestamp;
  s.columns = {flag, score, dt, ts};

  std::vector<storage::Row> rows = RunAndFetch(
      "SELECT TRUE AS flag, CAST(1.5 AS DOUBLE) AS score, "
      "DATE '2024-03-09' AS dt, TIMESTAMP '2024-03-09 12:34:56' AS ts",
      s);
  ASSERT_EQ(rows.size(), 1u);
  ASSERT_EQ(rows[0].cells.size(), 4u);
  EXPECT_EQ(rows[0].cells[0].kind(), storage::Value::Kind::kBool);
  EXPECT_TRUE(rows[0].cells[0].bool_value());
  EXPECT_EQ(rows[0].cells[1].kind(), storage::Value::Kind::kFloat64);
  EXPECT_DOUBLE_EQ(rows[0].cells[1].float64_value(), 1.5);
  EXPECT_EQ(rows[0].cells[2].kind(), storage::Value::Kind::kString);
  EXPECT_EQ(rows[0].cells[2].string_value(), "2024-03-09");
  EXPECT_EQ(rows[0].cells[3].kind(), storage::Value::Kind::kString);
  EXPECT_EQ(rows[0].cells[3].string_value(), "2024-03-09 12:34:56.000000");
}

// LIST vectors lower into `Value::Array`, with each element rendered
// against the inner column schema. The fixture mixes a non-null and
// an empty array so the offset / length math gets exercised twice.
TEST_F(ArrowToBqTest, RendersListAsRepeatedArrayCell) {
  schema::TableSchema s;
  schema::ColumnSchema col;
  col.name = "tags";
  col.type = schema::ColumnType::kString;
  col.mode = schema::ColumnMode::kRepeated;
  s.columns.push_back(col);

  std::vector<storage::Row> rows = RunAndFetch(
      "SELECT * FROM (VALUES (CAST([] AS VARCHAR[])), "
      "(['x', 'y'])) AS t(tags) ORDER BY array_length(tags)",
      s);
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_EQ(rows[0].cells[0].kind(), storage::Value::Kind::kArray);
  EXPECT_TRUE(rows[0].cells[0].array_value().empty());
  EXPECT_EQ(rows[1].cells[0].kind(), storage::Value::Kind::kArray);
  ASSERT_EQ(rows[1].cells[0].array_value().size(), 2u);
  EXPECT_EQ(rows[1].cells[0].array_value()[0].string_value(), "x");
  EXPECT_EQ(rows[1].cells[0].array_value()[1].string_value(), "y");
}

// STRUCT vectors lower into `Value::Struct` with positional fields in
// the same order the analyzer's `ColumnSchema::fields` lists them.
TEST_F(ArrowToBqTest, RendersStructAsStructCell) {
  schema::TableSchema s;
  schema::ColumnSchema col;
  col.name = "rec";
  col.type = schema::ColumnType::kStruct;
  schema::ColumnSchema f_id;
  f_id.name = "id";
  f_id.type = schema::ColumnType::kInt64;
  schema::ColumnSchema f_name;
  f_name.name = "name";
  f_name.type = schema::ColumnType::kString;
  col.fields = {f_id, f_name};
  s.columns.push_back(col);

  std::vector<storage::Row> rows = RunAndFetch(
      "SELECT {'id': CAST(7 AS BIGINT), 'name': 'grace'} AS rec", s);
  ASSERT_EQ(rows.size(), 1u);
  ASSERT_EQ(rows[0].cells[0].kind(), storage::Value::Kind::kStruct);
  ASSERT_EQ(rows[0].cells[0].struct_value().size(), 2u);
  EXPECT_EQ(rows[0].cells[0].struct_value()[0].int64_value(), 7);
  EXPECT_EQ(rows[0].cells[0].struct_value()[1].string_value(), "grace");
}

// Column-count mismatch surfaces as INVALID_ARGUMENT instead of
// silently truncating the rendered row. This is the contract the
// engine relies on when the analyzer and DuckDB disagree on shape.
TEST_F(ArrowToBqTest, ColumnCountMismatchReturnsInvalidArgument) {
  schema::TableSchema s;
  schema::ColumnSchema a;
  a.name = "a";
  a.type = schema::ColumnType::kInt64;
  schema::ColumnSchema b;
  b.name = "b";
  b.type = schema::ColumnType::kInt64;
  s.columns = {a, b};

  ::duckdb_result result;
  ASSERT_EQ(::duckdb_query(conn_, "SELECT CAST(1 AS BIGINT)", &result),
            ::DuckDBSuccess);
  ::duckdb_data_chunk chunk = ::duckdb_fetch_chunk(result);
  ASSERT_NE(chunk, nullptr);
  auto row = ChunkRowToCells(chunk, 0, s);
  EXPECT_FALSE(row.ok());
  EXPECT_EQ(row.status().code(), absl::StatusCode::kInvalidArgument);
  ::duckdb_destroy_data_chunk(&chunk);
  ::duckdb_destroy_result(&result);
}

}  // namespace
}  // namespace arrow_to_bq
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
