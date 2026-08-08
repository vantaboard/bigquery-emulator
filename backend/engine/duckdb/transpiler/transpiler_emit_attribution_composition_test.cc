#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

// R17: multi-CTE attribution SELECT / INSERT...SELECT / CTAS with
// COALESCE(SUM) + ROW_NUMBER over joins must bind in DuckDB (no stale
// `__bq_j_<id>` across CTE / LEFT JOIN / analytic boundaries).

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

class TranspilerAttributionCompositionTest : public TranspilerBindFixture {
 protected:
  void SetUp() override {
    TranspilerBindFixture::SetUp();

    // R17 attribution shape (bench/cases/attribution_insert_10k.yaml).
    auto activity_events = std::make_unique<::googlesql::SimpleTable>(
        "activity_events",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"tenant_id", type_factory_->get_string()},
            {"activity_id", type_factory_->get_string()},
            {"user_public_id", type_factory_->get_string()},
            {"action_type", type_factory_->get_string()},
            {"channel", type_factory_->get_string()},
            {"occurred_at", type_factory_->get_timestamp()},
            {"spend", type_factory_->get_double()},
        });
    catalog_->AddOwnedTable(std::move(activity_events));

    auto attr_users = std::make_unique<::googlesql::SimpleTable>(
        "attr_users",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"public_id", type_factory_->get_string()},
        });
    catalog_->AddOwnedTable(std::move(attr_users));

    auto attr_transactions = std::make_unique<::googlesql::SimpleTable>(
        "attr_transactions",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"user_id", type_factory_->get_int64()},
            {"total_amount", type_factory_->get_double()},
            {"total_reversed", type_factory_->get_double()},
            {"is_deleted", type_factory_->get_bool()},
            {"status", type_factory_->get_string()},
            {"source_created_at", type_factory_->get_timestamp()},
        });
    catalog_->AddOwnedTable(std::move(attr_transactions));

    auto attr_summary = std::make_unique<::googlesql::SimpleTable>(
        "attr_summary",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"activity_id", type_factory_->get_string()},
            {"total_transactions", type_factory_->get_int64()},
            {"gross_value", type_factory_->get_double()},
        });
    catalog_->AddOwnedTable(std::move(attr_summary));

    ExecDdl(
        "CREATE TABLE activity_events (tenant_id VARCHAR, activity_id "
        "VARCHAR, user_public_id VARCHAR, action_type VARCHAR, channel "
        "VARCHAR, occurred_at TIMESTAMPTZ, spend DOUBLE)");
    ExecDdl("CREATE TABLE attr_users (id BIGINT, public_id VARCHAR)");
    ExecDdl(
        "CREATE TABLE attr_transactions (id BIGINT, user_id BIGINT, "
        "total_amount DOUBLE, total_reversed DOUBLE, is_deleted BOOLEAN, "
        "status VARCHAR, source_created_at TIMESTAMPTZ)");
    ExecDdl(
        "CREATE TABLE attr_summary (activity_id VARCHAR, "
        "total_transactions BIGINT, gross_value DOUBLE)");
  }

  // Compact R17 attribution body: multi-CTE + COALESCE(SUM) + JOIN +
  // ROW_NUMBER + LEFT JOIN + final COALESCE(SUM). Matches the bench
  // case shape that previously stayed on semantic_executor.
  static constexpr const char kAttributionSelectBody[] = R"sql(
WITH ActivityLogs AS (
  SELECT tenant_id, activity_id, user_public_id, action_type, channel,
         occurred_at, spend
  FROM activity_events
),
MetricStats AS (
  SELECT
    tenant_id,
    activity_id,
    COUNT(DISTINCT IF(action_type = 'action_1', user_public_id, NULL))
      AS metric_a,
    COUNTIF(action_type = 'action_3') AS metric_c_total,
    COALESCE(SUM(spend), 0) AS total_spend
  FROM ActivityLogs
  GROUP BY tenant_id, activity_id
),
ValidActivityDispatches AS (
  SELECT user_public_id, activity_id, channel, occurred_at AS dispatched_at
  FROM ActivityLogs
  WHERE action_type = 'action_3'
),
LatestTransactions AS (
  SELECT * FROM attr_transactions
),
AttributedTransactionsRaw AS (
  SELECT
    logs.activity_id,
    logs.channel,
    txn.id AS txn_id,
    txn.total_amount,
    COALESCE(txn.total_reversed, 0) AS reversed_amount,
    ROW_NUMBER() OVER (
      PARTITION BY txn.id ORDER BY logs.dispatched_at DESC
    ) AS attribution_rank
  FROM ValidActivityDispatches logs
  JOIN attr_users users ON logs.user_public_id = users.public_id
  JOIN LatestTransactions txn ON users.id = txn.user_id
  WHERE txn.source_created_at > logs.dispatched_at
    AND txn.source_created_at <= TIMESTAMP_ADD(
      logs.dispatched_at, INTERVAL 7 DAY
    )
    AND COALESCE(txn.is_deleted, FALSE) = FALSE
    AND txn.status NOT IN ('status_x', 'status_y')
),
ValueStats AS (
  SELECT
    activity_id,
    COUNT(txn_id) AS total_transactions,
    COALESCE(SUM(total_amount), 0) AS gross_value
  FROM AttributedTransactionsRaw
  WHERE attribution_rank = 1
  GROUP BY activity_id
),
Attribution AS (
  SELECT
    p.activity_id AS id,
    COALESCE(r.total_transactions, 0) AS total_transactions,
    COALESCE(r.gross_value, 0) AS gross_value
  FROM MetricStats p
  LEFT JOIN ValueStats r ON p.activity_id = r.activity_id
)
)sql";
};

// R17 follow-up: full multi-CTE attribution SELECT with COALESCE(SUM)
// must bind (no stale `__bq_j_<id>` across CTE / LEFT JOIN boundaries).
TEST_F(TranspilerAttributionCompositionTest, AttributionMultiCteSelectBinds) {
  const std::string sql =
      absl::StrCat(kAttributionSelectBody,
                   "SELECT COUNT(*) AS activity_rows, "
                   "COALESCE(SUM(total_transactions), 0) AS txn_sum, "
                   "COALESCE(SUM(gross_value), 0) AS gross_sum "
                   "FROM Attribution");
  AssertSqlTranspileBinds(sql);
}

// R17 follow-up: compact INSERT...SELECT (single CTE + ROW_NUMBER over
// joins + COALESCE(SUM)) must bind — this is the conformance fixture
// shape that previously stayed on semantic_executor.
TEST_F(TranspilerAttributionCompositionTest,
       AttributionCompactInsertSelectBinds) {
  static constexpr const char kCompactSql[] = R"sql(
INSERT INTO attr_summary (activity_id, total_transactions, gross_value)
WITH Attributed AS (
  SELECT
    logs.activity_id,
    txn.id AS txn_id,
    txn.total_amount,
    ROW_NUMBER() OVER (
      PARTITION BY txn.id ORDER BY logs.occurred_at DESC
    ) AS attribution_rank
  FROM activity_events logs
  JOIN attr_users users ON logs.user_public_id = users.public_id
  JOIN attr_transactions txn ON users.id = txn.user_id
  WHERE txn.source_created_at > logs.occurred_at
    AND txn.source_created_at <= TIMESTAMP_ADD(
      logs.occurred_at, INTERVAL 7 DAY
    )
    AND txn.status NOT IN ('status_x', 'status_y')
)
SELECT
  activity_id,
  COUNT(txn_id) AS total_transactions,
  COALESCE(SUM(total_amount), 0) AS gross_value
FROM Attributed
WHERE attribution_rank = 1
GROUP BY activity_id
)sql";
  const ::googlesql::ResolvedStatement* stmt = Analyze(kCompactSql);
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_INSERT_STMT);
  TestTranspiler t;
  const std::string emitted =
      t.EmitInsertSelect(stmt->GetAs<::googlesql::ResolvedInsertStmt>());
  ASSERT_FALSE(emitted.empty()) << "EmitInsertSelect returned empty for:\n"
                                << kCompactSql;
  SCOPED_TRACE(emitted);
  ::duckdb_result result{};
  const auto rc = ::duckdb_query(conn_, emitted.c_str(), &result);
  if (rc != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    FAIL() << "DuckDB rejected EmitInsertSelect SQL\n"
           << "source_sql:\n"
           << kCompactSql << "\n"
           << "emitted_sql:\n"
           << emitted << "\n"
           << "duckdb_error:\n"
           << (err == nullptr ? "(null)" : err);
  }
  ::duckdb_destroy_result(&result);
}

// R17 follow-up: INSERT...SELECT materialization of the attribution
// body must bind (EmitInsertSelect clears join-alias flags).
TEST_F(TranspilerAttributionCompositionTest, AttributionInsertSelectBinds) {
  const std::string sql = absl::StrCat(
      "INSERT INTO attr_summary "
      "(activity_id, total_transactions, gross_value) ",
      kAttributionSelectBody,
      "SELECT id, total_transactions, gross_value FROM Attribution");
  const ::googlesql::ResolvedStatement* stmt = Analyze(sql);
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_INSERT_STMT);
  TestTranspiler t;
  const std::string emitted =
      t.EmitInsertSelect(stmt->GetAs<::googlesql::ResolvedInsertStmt>());
  ASSERT_FALSE(emitted.empty()) << "EmitInsertSelect returned empty for:\n"
                                << sql;
  SCOPED_TRACE(emitted);
  ::duckdb_result result{};
  const auto rc = ::duckdb_query(conn_, emitted.c_str(), &result);
  if (rc != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    FAIL() << "DuckDB rejected EmitInsertSelect SQL\n"
           << "source_sql:\n"
           << sql << "\n"
           << "emitted_sql:\n"
           << emitted << "\n"
           << "duckdb_error:\n"
           << (err == nullptr ? "(null)" : err);
  }
  ::duckdb_destroy_result(&result);
}

// R17 follow-up: CTAS materialization of the attribution body must bind.
TEST_F(TranspilerAttributionCompositionTest, AttributionCtasSelectBinds) {
  const std::string sql =
      absl::StrCat("CREATE TABLE attr_summary_ctas AS ",
                   kAttributionSelectBody,
                   "SELECT id AS activity_id, total_transactions, gross_value "
                   "FROM Attribution");
  const ::googlesql::ResolvedStatement* stmt = Analyze(sql);
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(),
            ::googlesql::RESOLVED_CREATE_TABLE_AS_SELECT_STMT);
  TestTranspiler t;
  const std::string emitted = t.EmitCtasSelect(
      stmt->GetAs<::googlesql::ResolvedCreateTableAsSelectStmt>());
  ASSERT_FALSE(emitted.empty()) << "EmitCtasSelect returned empty for:\n"
                                << sql;
  SCOPED_TRACE(emitted);
  ::duckdb_result result{};
  const auto rc = ::duckdb_query(conn_, emitted.c_str(), &result);
  if (rc != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    FAIL() << "DuckDB rejected EmitCtasSelect SQL\n"
           << "source_sql:\n"
           << sql << "\n"
           << "emitted_sql:\n"
           << emitted << "\n"
           << "duckdb_error:\n"
           << (err == nullptr ? "(null)" : err);
  }
  ::duckdb_destroy_result(&result);
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
