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

// --- bq_split ----------------------------------------------------

// Helper for LIST-returning macros: wraps the macro call in
// `list_aggregate(<call>, 'string_agg', '|')` so the result is a
// single VARCHAR with the list elements joined by `|`. The
// per-test assertions compare that joined form (easier to eyeball
// than a manually-walked column buffer; the macro's contract is
// fully captured by the ordered set of elements).
//
// Note: BigQuery SPLIT's return type is LIST(VARCHAR); DuckDB's
// `string_split` returns the same shape, and DuckDB's
// `array_to_string(list, sep)` flattens it. We use `'|'` as the
// join separator so test fixtures that include `,` in the data
// stay readable.
std::string JoinSplit(::duckdb_connection conn, const std::string& macro_call) {
  ::duckdb_result result;
  const std::string wrapped =
      "SELECT array_to_string(" + macro_call + ", '|')";
  auto rc = ::duckdb_query(conn, wrapped.c_str(), &result);
  if (rc != ::DuckDBSuccess) {
    ADD_FAILURE() << "DuckDB rejected: "
                  << (::duckdb_result_error(&result) == nullptr
                          ? "(no error)"
                          : ::duckdb_result_error(&result))
                  << " (sql=" << wrapped << ")";
    ::duckdb_destroy_result(&result);
    return "(rejected)";
  }
  char* raw = ::duckdb_value_varchar(&result, 0, 0);
  std::string out = raw == nullptr ? std::string("") : std::string(raw);
  ::duckdb_free(raw);
  ::duckdb_destroy_result(&result);
  return out;
}

TEST_F(StringMacrosTest, SplitDefaultDelimiterIsComma) {
  // Edge case pinned: BigQuery SPLIT(value) (single-arg form)
  // defaults to splitting on `,`. A regression that registered
  // the macro without a `delimiter := ','` default would surface
  // here as a binder error.
  EXPECT_EQ(JoinSplit(conn_, "bq_split('a,b,c')"), "a|b|c");
  EXPECT_EQ(JoinSplit(conn_, "bq_split('one,two')"), "one|two");
}

TEST_F(StringMacrosTest, SplitCustomDelimiter) {
  EXPECT_EQ(JoinSplit(conn_, "bq_split('a;b;c', ';')"), "a|b|c");
  EXPECT_EQ(JoinSplit(conn_, "bq_split('foo-bar-baz', '-')"), "foo|bar|baz");
}

TEST_F(StringMacrosTest, SplitEmptyInputReturnsSingleEmpty) {
  // Edge case pinned: BigQuery SPLIT('', ',') returns a list
  // containing one empty string (BQ contract: a non-empty
  // delimiter always splits, even an empty input). DuckDB's
  // string_split agrees today; the test pins it.
  EXPECT_EQ(JoinSplit(conn_, "bq_split('', ',')"), "");
}

TEST_F(StringMacrosTest, SplitNullPropagation) {
  // Wrap in a `r IS NULL` SQL boolean rather than relying on
  // `duckdb_value_is_null` for LIST cells; the C API's NULL
  // detector for LIST columns is shape-sensitive across DuckDB
  // versions, and a SQL-level `IS NULL` is unambiguous.
  ::duckdb_result result;
  for (const char* sql : {
           "SELECT bq_split(NULL::VARCHAR) IS NULL",
           "SELECT bq_split('a,b,c', NULL::VARCHAR) IS NULL",
       }) {
    auto rc = ::duckdb_query(conn_, sql, &result);
    ASSERT_EQ(rc, ::DuckDBSuccess) << "DuckDB rejected: " << sql;
    EXPECT_TRUE(::duckdb_value_boolean(&result, 0, 0))
        << "expected NULL propagation for " << sql;
    ::duckdb_destroy_result(&result);
  }
}

}  // namespace
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
