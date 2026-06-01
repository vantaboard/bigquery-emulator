// Unit tests for the BigQuery numeric polyfill macros.
//
// Each test drives the macro directly against an in-process DuckDB
// connection (mirroring the per-query connection lifecycle
// `DuckDbExecutor` uses) and exercises both the common path and the
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

// Test fixture that owns a fresh, in-memory DuckDB connection with
// the polyfill UDF library registered. Identical to the executor's
// per-query setup; per-UDF tests run against the same surface.
class NumericMacrosTest : public ::testing::Test {
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

  // Runs `sql` and returns a single INT64 from row 0, column 0.
  // Fails the test if the query rejected or returned no rows.
  int64_t RunInt64(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    if (rc != ::DuckDBSuccess) {
      ADD_FAILURE() << "DuckDB rejected: "
                    << (::duckdb_result_error(&result) == nullptr
                            ? "(no error string)"
                            : ::duckdb_result_error(&result))
                    << " (sql=" << sql << ")";
      ::duckdb_destroy_result(&result);
      return INT64_MIN;
    }
    int64_t value = ::duckdb_value_int64(&result, 0, 0);
    ::duckdb_destroy_result(&result);
    return value;
  }

  // Returns true when the cell at (0, 0) is NULL. NULL-aware
  // helper because `duckdb_value_int64` returns 0 for a NULL cell
  // (no way to distinguish "literal 0" from "NULL" without this).
  bool RunIsNull(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    if (rc != ::DuckDBSuccess) {
      ADD_FAILURE() << "DuckDB rejected: "
                    << (::duckdb_result_error(&result) == nullptr
                            ? "(no error string)"
                            : ::duckdb_result_error(&result))
                    << " (sql=" << sql << ")";
      ::duckdb_destroy_result(&result);
      return false;
    }
    bool is_null = ::duckdb_value_is_null(&result, 0, 0);
    ::duckdb_destroy_result(&result);
    return is_null;
  }

  // Returns true iff `sql` was rejected by DuckDB. Used to assert
  // that non-SAFE divide-by-zero raises (matching BigQuery).
  bool RunRejects(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    ::duckdb_destroy_result(&result);
    return rc != ::DuckDBSuccess;
  }

  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

// --- bq_mod ------------------------------------------------------

TEST_F(NumericMacrosTest, ModCommonPath) {
  EXPECT_EQ(RunInt64("SELECT bq_mod(7, 3)"), 1);
  EXPECT_EQ(RunInt64("SELECT bq_mod(6, 3)"), 0);
  EXPECT_EQ(RunInt64("SELECT bq_mod(0, 5)"), 0);
}

TEST_F(NumericMacrosTest, ModSignTracksDividend) {
  // Edge case pinned: BigQuery MOD's sign follows the dividend
  // (truncated division), not the divisor (floor / Python-like).
  // A regression switching to floor-mod would surface here.
  EXPECT_EQ(RunInt64("SELECT bq_mod(-7, 3)"), -1);
  EXPECT_EQ(RunInt64("SELECT bq_mod(7, -3)"), 1);
  EXPECT_EQ(RunInt64("SELECT bq_mod(-7, -3)"), -1);
}

TEST_F(NumericMacrosTest, ModNullPropagation) {
  EXPECT_TRUE(RunIsNull("SELECT bq_mod(NULL::BIGINT, 3)"));
  EXPECT_TRUE(RunIsNull("SELECT bq_mod(7, NULL::BIGINT)"));
  EXPECT_TRUE(RunIsNull("SELECT bq_mod(NULL::BIGINT, NULL::BIGINT)"));
}

TEST_F(NumericMacrosTest, ModByZeroRaises) {
  // BigQuery non-SAFE MOD raises on Y=0; DuckDB raises on integer
  // `% 0`. The macro inherits that behavior.
  EXPECT_TRUE(RunRejects("SELECT bq_mod(7, 0)"));
}

// --- bq_div ------------------------------------------------------

TEST_F(NumericMacrosTest, DivCommonPath) {
  EXPECT_EQ(RunInt64("SELECT bq_div(7, 3)"), 2);
  EXPECT_EQ(RunInt64("SELECT bq_div(6, 3)"), 2);
  EXPECT_EQ(RunInt64("SELECT bq_div(0, 5)"), 0);
}

TEST_F(NumericMacrosTest, DivTruncatesNotFloors) {
  // Edge case pinned: BigQuery DIV truncates toward zero. DuckDB's
  // bare `//` is FLOOR division, which would give -3 for
  // `-5 // 2`. The macro restores truncation through the
  // `(x - x % y) / y` identity. A regression that swapped the
  // macro body back to `x // y` would surface as -3 here.
  EXPECT_EQ(RunInt64("SELECT bq_div(-5, 2)"), -2);
  EXPECT_EQ(RunInt64("SELECT bq_div(5, -2)"), -2);
  EXPECT_EQ(RunInt64("SELECT bq_div(-5, -2)"), 2);
  EXPECT_EQ(RunInt64("SELECT bq_div(-6, 2)"), -3);
  EXPECT_EQ(RunInt64("SELECT bq_div(6, -2)"), -3);
}

TEST_F(NumericMacrosTest, DivNullPropagation) {
  EXPECT_TRUE(RunIsNull("SELECT bq_div(NULL::BIGINT, 3)"));
  EXPECT_TRUE(RunIsNull("SELECT bq_div(7, NULL::BIGINT)"));
}

TEST_F(NumericMacrosTest, DivByZeroRaises) {
  EXPECT_TRUE(RunRejects("SELECT bq_div(7, 0)"));
}

}  // namespace
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
