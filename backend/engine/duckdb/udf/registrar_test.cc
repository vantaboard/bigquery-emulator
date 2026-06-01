// Tests for the BigQuery polyfill UDF library registrar.
//
// The registrar's contract is:
//
//   * `RegisterAll(conn)` returns OK against a fresh, healthy
//     `duckdb_connection`.
//   * After `RegisterAll`, every UDF the registrar promises to
//     install is callable via plain DuckDB SQL.
//   * `RegisterAll(nullptr)` returns INVALID_ARGUMENT (the executor
//     contract guarantees a non-null connection, but the registrar
//     defends in depth so a future caller that forgets to check
//     `duckdb_connect`'s return code does not crash inside DuckDB).
//   * Registration failure (e.g. a malformed `CREATE MACRO` body
//     landing in a future commit) propagates out as a non-OK status
//     -- the registrar never partially registers and silently
//     pretends to succeed. The `RegisterAll(nullptr)` case stands in
//     for the failure shape today; per-family tests cover the
//     malformed-macro case in their own files.

#include "backend/engine/duckdb/udf/registrar.h"

#include "absl/status/status.h"
#include "duckdb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {
namespace {

// Test fixture that owns a fresh, in-memory DuckDB database and
// connection per TEST_F. Mirrors the per-query connection lifecycle
// `DuckDbExecutor` uses, so the registrar runs in the same
// environment it sees at runtime.
class RegistrarTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(::duckdb_open(nullptr, &db_), ::DuckDBSuccess);
    ASSERT_EQ(::duckdb_connect(db_, &conn_), ::DuckDBSuccess);
  }

  void TearDown() override {
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
  }

  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

TEST_F(RegistrarTest, NullConnectionReturnsInvalidArgument) {
  absl::Status s = RegisterAll(nullptr);
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument) << s;
}

TEST_F(RegistrarTest, FreshConnectionRegistersWithoutError) {
  absl::Status s = RegisterAll(conn_);
  EXPECT_TRUE(s.ok()) << s;
}

TEST_F(RegistrarTest, IdempotentAcrossMultipleCalls) {
  // Subsequent commits use `CREATE OR REPLACE MACRO`; the registrar
  // must remain idempotent so the executor can reuse a connection
  // (a regression that switched to `CREATE MACRO` would surface here
  // as DuckDB's "macro already exists" error).
  EXPECT_TRUE(RegisterAll(conn_).ok());
  EXPECT_TRUE(RegisterAll(conn_).ok());
}

}  // namespace
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
