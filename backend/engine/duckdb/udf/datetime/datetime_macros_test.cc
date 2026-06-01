// Unit tests for the BigQuery datetime polyfill macros.
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

class DatetimeMacrosTest : public ::testing::Test {
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

  bool RunBool(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    EXPECT_EQ(rc, ::DuckDBSuccess) << "DuckDB rejected: " << sql;
    bool v = ::duckdb_value_boolean(&result, 0, 0);
    ::duckdb_destroy_result(&result);
    return v;
  }

  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

// 2024-01-01 00:00:00 UTC -> 1704067200 seconds since epoch.
constexpr int64_t kSec2024_01_01 = 1704067200;
constexpr int64_t kMs2024_01_01  = 1704067200LL * 1000;
constexpr int64_t kUs2024_01_01  = 1704067200LL * 1000 * 1000;

// --- bq_unix_seconds ---------------------------------------------

TEST_F(DatetimeMacrosTest, UnixSecondsWholeSecond) {
  EXPECT_EQ(RunInt64(
                "SELECT bq_unix_seconds(TIMESTAMPTZ '2024-01-01 00:00:00+00')"),
            kSec2024_01_01);
}

TEST_F(DatetimeMacrosTest, UnixSecondsTruncatesSubsecond) {
  // Edge case pinned: BigQuery UNIX_SECONDS truncates higher
  // precision. A regression that switched the CAST to a rounding
  // form would surface here as 1704067201 (rounding .999 up to
  // the next second) instead of 1704067200.
  EXPECT_EQ(
      RunInt64(
          "SELECT bq_unix_seconds(TIMESTAMPTZ '2024-01-01 00:00:00.999+00')"),
      kSec2024_01_01);
}

TEST_F(DatetimeMacrosTest, UnixSecondsNullPropagation) {
  EXPECT_TRUE(
      RunBool("SELECT bq_unix_seconds(NULL::TIMESTAMPTZ) IS NULL"));
}

// --- bq_unix_millis ----------------------------------------------

TEST_F(DatetimeMacrosTest, UnixMillisWholeSecond) {
  EXPECT_EQ(RunInt64(
                "SELECT bq_unix_millis(TIMESTAMPTZ '2024-01-01 00:00:00+00')"),
            kMs2024_01_01);
}

TEST_F(DatetimeMacrosTest, UnixMillisSubsecond) {
  // Fractional milliseconds. 2024-01-01 00:00:00.5 -> +500 ms.
  EXPECT_EQ(
      RunInt64(
          "SELECT bq_unix_millis(TIMESTAMPTZ '2024-01-01 00:00:00.5+00')"),
      kMs2024_01_01 + 500);
}

TEST_F(DatetimeMacrosTest, UnixMillisNullPropagation) {
  EXPECT_TRUE(RunBool("SELECT bq_unix_millis(NULL::TIMESTAMPTZ) IS NULL"));
}

// --- bq_unix_micros ----------------------------------------------

TEST_F(DatetimeMacrosTest, UnixMicrosWholeSecond) {
  EXPECT_EQ(RunInt64(
                "SELECT bq_unix_micros(TIMESTAMPTZ '2024-01-01 00:00:00+00')"),
            kUs2024_01_01);
}

TEST_F(DatetimeMacrosTest, UnixMicrosSubsecond) {
  // 2024-01-01 00:00:00.123456 -> +123456 microseconds.
  EXPECT_EQ(
      RunInt64("SELECT bq_unix_micros(TIMESTAMPTZ "
               "'2024-01-01 00:00:00.123456+00')"),
      kUs2024_01_01 + 123456);
}

TEST_F(DatetimeMacrosTest, UnixMicrosNullPropagation) {
  EXPECT_TRUE(RunBool("SELECT bq_unix_micros(NULL::TIMESTAMPTZ) IS NULL"));
}

// --- bq_unix_date ------------------------------------------------

TEST_F(DatetimeMacrosTest, UnixDateEpochIsZero) {
  EXPECT_EQ(RunInt64("SELECT bq_unix_date(DATE '1970-01-01')"), 0);
}

TEST_F(DatetimeMacrosTest, UnixDatePreEpochIsNegative) {
  // Edge case pinned: pre-1970 dates return negative day counts.
  // BigQuery documents this explicitly. A regression that wrapped
  // the result in `ABS(...)` or `GREATEST(..., 0)` would surface
  // here.
  EXPECT_EQ(RunInt64("SELECT bq_unix_date(DATE '1969-12-31')"), -1);
  EXPECT_EQ(RunInt64("SELECT bq_unix_date(DATE '1969-12-01')"), -31);
}

TEST_F(DatetimeMacrosTest, UnixDateFutureDate) {
  // 2024-01-01 is 19723 days after 1970-01-01.
  // (54 years + 13 leap days = 19723 days.)
  EXPECT_EQ(RunInt64("SELECT bq_unix_date(DATE '2024-01-01')"), 19723);
}

TEST_F(DatetimeMacrosTest, UnixDateNullPropagation) {
  EXPECT_TRUE(RunBool("SELECT bq_unix_date(NULL::DATE) IS NULL"));
}

}  // namespace
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
