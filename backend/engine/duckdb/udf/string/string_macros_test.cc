// Unit tests for the BigQuery string polyfill macros.
//
// Each test drives the macro directly against an in-process DuckDB
// connection and exercises both the common path and the
// BigQuery-specific edge case the wrapper exists to pin.

#include <cstdint>
#include <string>

#include "absl/status/status.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "duckdb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {
namespace {

class StringMacrosTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(::duckdb_open(nullptr, &db_), ::DuckDBSuccess);
    ASSERT_EQ(::duckdb_connect(db_, &conn_), ::DuckDBSuccess);
    absl::Status reg = RegisterAll(conn_);
    ASSERT_TRUE(reg.ok()) << reg;
  }

  void TearDown() override {
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
  }

  int64_t RunInt64(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    EXPECT_EQ(rc, ::DuckDBSuccess)
        << "DuckDB rejected: "
        << (::duckdb_result_error(&result) == nullptr
                ? "(no error)"
                : ::duckdb_result_error(&result))
        << " (sql=" << sql << ")";
    int64_t v = ::duckdb_value_int64(&result, 0, 0);
    ::duckdb_destroy_result(&result);
    return v;
  }

  bool RunIsNull(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    EXPECT_EQ(rc, ::DuckDBSuccess)
        << "DuckDB rejected: " << sql;
    bool v = ::duckdb_value_is_null(&result, 0, 0);
    ::duckdb_destroy_result(&result);
    return v;
  }

  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

// --- bq_strpos ---------------------------------------------------

TEST_F(StringMacrosTest, StrposReturnsOneBasedIndex) {
  // Edge case pinned: BigQuery STRPOS is 1-based. A regression
  // that returned a 0-based index would surface here as 2 instead
  // of 3.
  EXPECT_EQ(RunInt64("SELECT bq_strpos('hello', 'll')"), 3);
  EXPECT_EQ(RunInt64("SELECT bq_strpos('hello', 'h')"), 1);
}

TEST_F(StringMacrosTest, StrposMissingNeedleReturnsZero) {
  // Edge case pinned: BigQuery STRPOS returns 0 (NOT -1, NOT NULL)
  // when the needle is not found.
  EXPECT_EQ(RunInt64("SELECT bq_strpos('hello', 'zz')"), 0);
}

TEST_F(StringMacrosTest, StrposEmptyNeedle) {
  // Edge case pinned: BigQuery STRPOS('abc', '') returns 1
  // (empty substring matches at position 1).
  EXPECT_EQ(RunInt64("SELECT bq_strpos('abc', '')"), 1);
}

TEST_F(StringMacrosTest, StrposNullPropagation) {
  EXPECT_TRUE(RunIsNull("SELECT bq_strpos(NULL::VARCHAR, 'll')"));
  EXPECT_TRUE(RunIsNull("SELECT bq_strpos('hello', NULL::VARCHAR)"));
  EXPECT_TRUE(RunIsNull("SELECT bq_strpos(NULL::VARCHAR, NULL::VARCHAR)"));
}

}  // namespace
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
