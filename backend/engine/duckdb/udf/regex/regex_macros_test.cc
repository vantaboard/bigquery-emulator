// Unit tests for the BigQuery regex polyfill macros.
//
// Each test drives the macro directly against an in-process DuckDB
// connection and exercises both the common path and the
// BigQuery-specific edge case the wrapper exists to pin.

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

class RegexMacrosTest : public ::testing::Test {
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

  bool RunBool(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    EXPECT_EQ(rc, ::DuckDBSuccess) << "DuckDB rejected: "
                                   << (::duckdb_result_error(&result) == nullptr
                                           ? "(no error)"
                                           : ::duckdb_result_error(&result))
                                   << " (sql=" << sql << ")";
    bool v = ::duckdb_value_boolean(&result, 0, 0);
    ::duckdb_destroy_result(&result);
    return v;
  }

  std::string RunString(const std::string& sql) {
    ::duckdb_result result;
    auto rc = ::duckdb_query(conn_, sql.c_str(), &result);
    EXPECT_EQ(rc, ::DuckDBSuccess) << "DuckDB rejected: "
                                   << (::duckdb_result_error(&result) == nullptr
                                           ? "(no error)"
                                           : ::duckdb_result_error(&result))
                                   << " (sql=" << sql << ")";
    char* raw = ::duckdb_value_varchar(&result, 0, 0);
    std::string out = raw == nullptr ? std::string("") : std::string(raw);
    ::duckdb_free(raw);
    ::duckdb_destroy_result(&result);
    return out;
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

// --- bq_regexp_contains ------------------------------------------

TEST_F(RegexMacrosTest, RegexpContainsCommonPath) {
  EXPECT_TRUE(RunBool("SELECT bq_regexp_contains('abc', 'b')"));
  EXPECT_FALSE(RunBool("SELECT bq_regexp_contains('abc', 'd')"));
}

TEST_F(RegexMacrosTest, RegexpContainsAnchoredMatch) {
  // Edge case pinned: `^` anchors to the start. A regression that
  // wrapped the regex with `^...$` (full-string match) would
  // break the contains-anywhere contract.
  EXPECT_TRUE(RunBool("SELECT bq_regexp_contains('abc', '^a')"));
  EXPECT_FALSE(RunBool("SELECT bq_regexp_contains('abc', '^b')"));
  EXPECT_TRUE(RunBool("SELECT bq_regexp_contains('abc', 'c$')"));
}

TEST_F(RegexMacrosTest, RegexpContainsCaseSensitiveByDefault) {
  // Edge case pinned: BigQuery RE2 is case-sensitive unless the
  // pattern includes an `(?i)` inline flag. A regression that
  // implicitly case-folded would surface here.
  EXPECT_FALSE(RunBool("SELECT bq_regexp_contains('ABC', 'abc')"));
  EXPECT_TRUE(RunBool("SELECT bq_regexp_contains('ABC', 'ABC')"));
}

TEST_F(RegexMacrosTest, RegexpContainsHonorsInlineFlags) {
  // The `(?i)` inline flag is the canonical BQ way to do
  // case-insensitive regex (BQ does not expose a separate flags
  // parameter on REGEXP_CONTAINS). Verifying it round-trips
  // through the DuckDB RE2 binding pins the dialect compatibility.
  EXPECT_TRUE(RunBool("SELECT bq_regexp_contains('ABC', '(?i)abc')"));
  EXPECT_TRUE(RunBool("SELECT bq_regexp_contains('hello\nworld', '(?s)hello.world')"));
}

TEST_F(RegexMacrosTest, RegexpContainsNullPropagation) {
  EXPECT_TRUE(RunIsNull("SELECT bq_regexp_contains(NULL::VARCHAR, 'a')"));
  EXPECT_TRUE(RunIsNull("SELECT bq_regexp_contains('abc', NULL::VARCHAR)"));
}

// --- bq_regexp_replace -------------------------------------------

TEST_F(RegexMacrosTest, RegexpReplaceIsGlobal) {
  // Edge case pinned: BigQuery REGEXP_REPLACE replaces ALL
  // matches; DuckDB's `regexp_replace` defaults to only the
  // first. A regression that dropped the `'g'` flag would
  // surface here as 'baaa' instead of 'bbbb'.
  EXPECT_EQ(RunString("SELECT bq_regexp_replace('aaaa', 'a', 'b')"), "bbbb");
  EXPECT_EQ(RunString("SELECT bq_regexp_replace('foo bar foo', 'foo', 'baz')"),
            "baz bar baz");
}

TEST_F(RegexMacrosTest, RegexpReplaceNoMatchReturnsInput) {
  EXPECT_EQ(RunString("SELECT bq_regexp_replace('abc', 'xyz', 'q')"), "abc");
}

TEST_F(RegexMacrosTest, RegexpReplaceHonorsBackreferences) {
  // BigQuery and DuckDB both support `\1`, `\2`, ... backrefs in
  // the replacement string. Swapping two captures is the
  // canonical pin: `'(\w+) (\w+)'` -> `'\2 \1'`.
  EXPECT_EQ(RunString(
                "SELECT bq_regexp_replace('John Doe', '(\\w+) (\\w+)', "
                "'\\2 \\1')"),
            "Doe John");
}

TEST_F(RegexMacrosTest, RegexpReplaceNullPropagation) {
  EXPECT_TRUE(
      RunIsNull("SELECT bq_regexp_replace(NULL::VARCHAR, 'a', 'b')"));
  EXPECT_TRUE(
      RunIsNull("SELECT bq_regexp_replace('abc', NULL::VARCHAR, 'b')"));
  EXPECT_TRUE(
      RunIsNull("SELECT bq_regexp_replace('abc', 'a', NULL::VARCHAR)"));
}

}  // namespace
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
