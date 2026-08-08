#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"
#include "googlesql/public/types/array_type.h"

// R9: Anti-join over QUALIFY-deduped views — DuckDB binder "column id not
// found". Indexed in conformance/REGRESSIONS.md. Part of the transpiler
// binding property-test suite: generated query compositions must always
// transpile to SQL that binds in DuckDB.

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace {

constexpr int kCompositionGeneratorSeed = 0x06060606;
constexpr int kCompositionGeneratorCases = 24;
constexpr int kDistinctAfterDedupGeneratorCases = 12;

std::string WrapQualifyDedupSubquery(absl::string_view partition_col,
                                     absl::string_view inner_sql) {
  return absl::StrCat(
      "SELECT * FROM (SELECT *, ROW_NUMBER() OVER (PARTITION BY ",
      partition_col,
      " ORDER BY ",
      partition_col,
      " DESC) AS rn FROM (",
      inner_sql,
      ")) WHERE rn = 1");
}

uint32_t LcgNext(uint32_t* state) {
  *state = *state * 1664525u + 1013904223u;
  return *state;
}

}  // namespace

class TranspilerCompositionTest : public TranspilerBindFixture {
 protected:
  void SetUp() override {
    TranspilerBindFixture::SetUp();

    const ::googlesql::ArrayType* profile_tags_type = nullptr;
    ASSERT_TRUE(
        type_factory_
            ->MakeArrayType(type_factory_->get_string(), &profile_tags_type)
            .ok());
    auto profiles = std::make_unique<::googlesql::SimpleTable>(
        "profiles",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"name", type_factory_->get_string()},
        });
    catalog_->AddOwnedTable(std::move(profiles));

    auto dedup_profiles = std::make_unique<::googlesql::SimpleTable>(
        "dedup_profiles",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"city", type_factory_->get_string()},
            {"tags", profile_tags_type},
            {"source_updated_at", type_factory_->get_timestamp()},
        });
    catalog_->AddOwnedTable(std::move(dedup_profiles));

    auto bq_orders = std::make_unique<::googlesql::SimpleTable>(
        "bq_orders",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"order_id", type_factory_->get_int64()},
            {"customer_id", type_factory_->get_int64()},
        });
    catalog_->AddOwnedTable(std::move(bq_orders));

    const ::googlesql::ArrayType* vals_type = nullptr;
    ASSERT_TRUE(
        type_factory_->MakeArrayType(type_factory_->get_int64(), &vals_type)
            .ok());
    auto items = std::make_unique<::googlesql::SimpleTable>(
        "items",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"vals", vals_type},
        });
    catalog_->AddOwnedTable(std::move(items));

    const ::googlesql::ArrayType* tags_type = nullptr;
    ASSERT_TRUE(
        type_factory_->MakeArrayType(type_factory_->get_string(), &tags_type)
            .ok());
    auto arrays = std::make_unique<::googlesql::SimpleTable>(
        "arrays",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"tags", tags_type},
        });
    catalog_->AddOwnedTable(std::move(arrays));

    const ::googlesql::ArrayType* collection_ids_type = nullptr;
    ASSERT_TRUE(
        type_factory_
            ->MakeArrayType(type_factory_->get_int64(), &collection_ids_type)
            .ok());
    auto products = std::make_unique<::googlesql::SimpleTable>(
        "products",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"title", type_factory_->get_string()},
            {"is_published", type_factory_->get_bool()},
            {"collection_ids", collection_ids_type},
        });
    catalog_->AddOwnedTable(std::move(products));

    auto collections = std::make_unique<::googlesql::SimpleTable>(
        "collections",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"title", type_factory_->get_string()},
            {"is_published", type_factory_->get_bool()},
            {"products_count", type_factory_->get_int64()},
        });
    catalog_->AddOwnedTable(std::move(collections));

    ExecDdl("CREATE TABLE bq_orders (order_id BIGINT, customer_id BIGINT)");
    ExecDdl("CREATE TABLE profiles (id BIGINT, name VARCHAR)");
    ExecDdl("CREATE TABLE people (id BIGINT, name VARCHAR)");
    ExecDdl("CREATE TABLE items (id BIGINT, vals BIGINT[])");
    ExecDdl("CREATE TABLE arrays (tags STRING[])");
    ExecDdl(
        "CREATE TABLE dedup_profiles (id BIGINT, city VARCHAR, tags STRING[], "
        "source_updated_at TIMESTAMPTZ)");
    ExecDdl(
        "CREATE TABLE products (id BIGINT, title VARCHAR, is_published "
        "BOOLEAN, collection_ids BIGINT[])");
    ExecDdl(
        "CREATE TABLE collections (id BIGINT, title VARCHAR, is_published "
        "BOOLEAN, products_count BIGINT)");

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

TEST_F(TranspilerCompositionTest, DistinctCityAfterQualifyDedupBinds) {
  static constexpr const char kSql[] = R"sql(
SELECT DISTINCT city
FROM (
  SELECT * FROM dedup_profiles
  QUALIFY ROW_NUMBER() OVER (PARTITION BY id ORDER BY source_updated_at DESC) = 1
)
WHERE city IS NOT NULL
)sql";
  AssertSqlTranspileBinds(kSql);
}

TEST_F(TranspilerCompositionTest, DistinctUnnestAfterQualifyDedupBinds) {
  static constexpr const char kSql[] = R"sql(
SELECT DISTINCT tag
FROM (
  SELECT * FROM dedup_profiles
  QUALIFY ROW_NUMBER() OVER (PARTITION BY id ORDER BY source_updated_at DESC) = 1
), UNNEST(tags) AS tag
)sql";
  AssertSqlTranspileBinds(kSql);
}

TEST_F(TranspilerCompositionTest, CorrelatedUnnestFromTableBinds) {
  static constexpr const char kSql[] = R"sql(
SELECT id, n
FROM items, UNNEST(items.vals) AS n
ORDER BY id, n
)sql";
  AssertSqlTranspileBinds(kSql);
}

// R13: UNNEST + GROUP BY inside a CTE, then outer LEFT JOIN on the
// unnested value — CTE alias/rn state must not leak into the outer join.
TEST_F(TranspilerCompositionTest, UnnestGroupByInCteThenJoinBinds) {
  static constexpr const char kSql[] = R"sql(
WITH product_counts AS (
  SELECT col_id, COUNT(DISTINCT p.id) AS calculated_product_count
  FROM products p, UNNEST(p.collection_ids) AS col_id
  GROUP BY col_id
)
SELECT
  c.id AS collection_id,
  c.title AS collection_title,
  c.products_count AS collection_table_count,
  COALESCE(pc.calculated_product_count, 0) AS calculated_product_count,
  (c.products_count - COALESCE(pc.calculated_product_count, 0)) AS discrepancy
FROM collections c
LEFT JOIN product_counts pc ON c.id = pc.col_id
WHERE COALESCE(c.products_count, 0) != COALESCE(pc.calculated_product_count, 0)
)sql";
  AssertSqlTranspileBinds(kSql);
}

// R13: correlated UNNEST in CTE (no GROUP BY) + outer JOIN — anchors and
// join sides must not reference stale `__bq_j_<id>` aliases.
TEST_F(TranspilerCompositionTest, UnnestInCteThenJoinBinds) {
  static constexpr const char kSql[] = R"sql(
WITH product_collections AS (
  SELECT p.id AS product_id, col_id
  FROM products p, UNNEST(p.collection_ids) AS col_id
)
SELECT c.id AS collection_id, c.title AS collection_title, pc.product_id
FROM collections c
LEFT JOIN product_collections pc ON c.id = pc.col_id
)sql";
  AssertSqlTranspileBinds(kSql);
}

TEST_F(TranspilerCompositionTest, CoreUsageUnnestArrayShapeBinds) {
  static constexpr const char kSql[] = R"sql(
SELECT tag FROM arrays, UNNEST(tags) AS tag
)sql";
  AssertSqlTranspileBinds(kSql);
}

TEST_F(TranspilerCompositionTest, NestedUnnestCrossProductBinds) {
  static constexpr const char kSql[] = R"sql(
SELECT n, m
FROM UNNEST(GENERATE_ARRAY(1, 2)) AS n
CROSS JOIN UNNEST(GENERATE_ARRAY(10, 11)) AS m
)sql";
  AssertSqlTranspileBinds(kSql);
}

TEST_F(TranspilerCompositionTest, OrphanOrdersQualifyDedupAntiJoinBinds) {
  static constexpr const char kSql[] = R"sql(
SELECT o.order_id
FROM (
  SELECT * FROM bq_orders
  QUALIFY ROW_NUMBER() OVER (PARTITION BY order_id ORDER BY order_id) = 1
) o
LEFT JOIN (
  SELECT * FROM profiles
  QUALIFY ROW_NUMBER() OVER (PARTITION BY id ORDER BY id) = 1
) p ON o.customer_id = p.id
WHERE p.id IS NULL
ORDER BY o.order_id
)sql";
  AssertSqlTranspileBinds(kSql);
}

TEST_F(TranspilerCompositionTest, OrphanOrdersSubqueryDedupAntiJoinBinds) {
  static constexpr const char kSql[] = R"sql(
SELECT o.order_id, o.customer_id
FROM (
  SELECT order_id, customer_id FROM (
    SELECT *, ROW_NUMBER() OVER (PARTITION BY order_id ORDER BY order_id) AS rn
    FROM (
      SELECT 1 AS order_id, 10 AS customer_id UNION ALL
      SELECT 2 AS order_id, 99 AS customer_id
    )
  ) WHERE rn = 1
) o
LEFT JOIN (
  SELECT id FROM (
    SELECT *, ROW_NUMBER() OVER (PARTITION BY id ORDER BY id) AS rn
    FROM (SELECT 10 AS id)
  ) WHERE rn = 1
) p ON o.customer_id = p.id
WHERE o.customer_id IS NOT NULL AND p.id IS NULL
)sql";
  AssertSqlTranspileBinds(kSql);
}

TEST_F(TranspilerCompositionTest, NestedQualifyJoinCteExceptBinds) {
  static constexpr const char kSql[] = R"sql(
WITH dedup AS (
  SELECT id, name FROM (
    SELECT *, ROW_NUMBER() OVER (PARTITION BY id ORDER BY id) AS rn
    FROM people
  ) WHERE rn = 1
)
SELECT a.id FROM dedup a
LEFT JOIN dedup b ON a.id = b.id
WHERE b.id IS NULL
EXCEPT DISTINCT
SELECT CAST(0 AS INT64) AS id
)sql";
  AssertSqlTranspileBinds(kSql);
}

// R17 follow-up: full multi-CTE attribution SELECT with COALESCE(SUM)
// must bind (no stale `__bq_j_<id>` across CTE / LEFT JOIN boundaries).
TEST_F(TranspilerCompositionTest, AttributionMultiCteSelectBinds) {
  const std::string sql =
      absl::StrCat(kAttributionSelectBody,
                   "SELECT COUNT(*) AS activity_rows, "
                   "COALESCE(SUM(total_transactions), 0) AS txn_sum, "
                   "COALESCE(SUM(gross_value), 0) AS gross_sum "
                   "FROM Attribution");
  AssertSqlTranspileBinds(sql);
}

// R17 follow-up: INSERT...SELECT materialization of the attribution
// body must bind (EmitInsertSelect clears join-alias flags).
TEST_F(TranspilerCompositionTest, AttributionInsertSelectBinds) {
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
TEST_F(TranspilerCompositionTest, AttributionCtasSelectBinds) {
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

TEST_F(TranspilerCompositionTest, SeededCompositionGeneratorBinds) {
  uint32_t rng = kCompositionGeneratorSeed;
  for (int i = 0; i < kCompositionGeneratorCases; ++i) {
    const int base = static_cast<int>(LcgNext(&rng) % 2);
    const int wrap1 = static_cast<int>(LcgNext(&rng) % 3);
    const int wrap2 = static_cast<int>(LcgNext(&rng) % 3);

    const char* key_col = base == 0 ? "id" : "order_id";
    std::string inner = base == 0
                            ? "SELECT id, name FROM people"
                            : "SELECT order_id, customer_id FROM bq_orders";

    if (wrap1 == 1) {
      inner = WrapQualifyDedupSubquery(key_col, inner);
    } else if (wrap1 == 2) {
      inner = absl::StrCat("SELECT * EXCEPT(rn) FROM (",
                           WrapQualifyDedupSubquery(key_col, inner),
                           ")");
    }

    std::string sql;
    if (wrap2 == 0) {
      sql = absl::StrCat(
          "SELECT ", key_col, " FROM (", inner, ") t WHERE ", key_col, " >= 0");
    } else if (wrap2 == 1) {
      sql = absl::StrCat("WITH w AS (", inner, ") SELECT COUNT(*) AS c FROM w");
    } else {
      sql = absl::StrCat("SELECT a.",
                         key_col,
                         " FROM (",
                         inner,
                         ") a LEFT JOIN (",
                         inner,
                         ") b ON a.",
                         key_col,
                         " = b.",
                         key_col,
                         " WHERE b.",
                         key_col,
                         " IS NULL");
    }

    SCOPED_TRACE(absl::StrCat("case=", i, " sql=", sql));
    AssertSqlTranspileBinds(sql);
  }
}

TEST_F(TranspilerCompositionTest, SeededDistinctAfterDedupGeneratorBinds) {
  uint32_t rng = kCompositionGeneratorSeed ^ 0xD157111Cu;
  for (int i = 0; i < kDistinctAfterDedupGeneratorCases; ++i) {
    const int wrap3 = static_cast<int>(LcgNext(&rng) % 3);
    const std::string deduped = WrapQualifyDedupSubquery(
        "id", "SELECT id, city, tags FROM dedup_profiles");

    std::string sql;
    if (wrap3 == 0) {
      sql = absl::StrCat(
          "SELECT DISTINCT city FROM (", deduped, ") WHERE city IS NOT NULL");
    } else if (wrap3 == 1) {
      sql = absl::StrCat(
          "SELECT DISTINCT tag FROM (", deduped, "), UNNEST(tags) AS tag");
    } else {
      sql = absl::StrCat(
          "SELECT city, COUNT(*) AS c FROM (", deduped, ") GROUP BY city");
    }

    SCOPED_TRACE(absl::StrCat("distinct_case=", i, " sql=", sql));
    AssertSqlTranspileBinds(sql);
  }
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
