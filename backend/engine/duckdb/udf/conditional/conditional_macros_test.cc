// Unit tests for the BigQuery conditional polyfill macros.
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

class ConditionalMacrosTest : public ::testing::Test {
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
    EXPECT_EQ(rc, ::DuckDBSuccess) << "DuckDB rejected: "
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
    EXPECT_EQ(rc, ::DuckDBSuccess) << "DuckDB rejected: " << sql;
    bool v = ::duckdb_value_is_null(&result, 0, 0);
    ::duckdb_destroy_result(&result);
    return v;
  }

  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

// --- bq_if -------------------------------------------------------

TEST_F(ConditionalMacrosTest, IfTrueReturnsThen) {
  EXPECT_EQ(RunInt64("SELECT bq_if(TRUE, 1, 2)"), 1);
}

TEST_F(ConditionalMacrosTest, IfFalseReturnsElse) {
  EXPECT_EQ(RunInt64("SELECT bq_if(FALSE, 1, 2)"), 2);
}

TEST_F(ConditionalMacrosTest, IfNullCondFallsThroughToElse) {
  // BigQuery edge case pinned: a NULL condition does NOT select the
  // THEN branch; it falls through to ELSE. A regression that
  // mapped IF to `CASE cond WHEN TRUE THEN ... END` (BOOLEAN
  // equality semantics) would surface here as NULL.
  EXPECT_EQ(RunInt64("SELECT bq_if(NULL::BOOLEAN, 1, 2)"), 2);
}

TEST_F(ConditionalMacrosTest, IfPreservesNullResults) {
  // The THEN / ELSE arms preserve NULL when chosen.
  EXPECT_TRUE(RunIsNull("SELECT bq_if(TRUE, NULL::BIGINT, 2)"));
  EXPECT_TRUE(RunIsNull("SELECT bq_if(FALSE, 1, NULL::BIGINT)"));
}

}  // namespace
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
