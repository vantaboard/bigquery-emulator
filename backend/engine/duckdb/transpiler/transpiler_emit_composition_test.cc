#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

// R9: Anti-join over QUALIFY-deduped views — DuckDB binder "column id not
// found". Plan:
// .cursor/plans/conformance-hardening/07-reported-bug-regression-fixtures.plan.md
// See also:
// .cursor/plans/conformance-hardening/06-transpiler-binding-property-tests.plan.md

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace {

constexpr int kCompositionGeneratorSeed = 0x06060606;
constexpr int kCompositionGeneratorCases = 24;

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

    auto profiles = std::make_unique<::googlesql::SimpleTable>(
        "profiles",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"name", type_factory_->get_string()},
        });
    catalog_->AddOwnedTable(std::move(profiles));

    auto bq_orders = std::make_unique<::googlesql::SimpleTable>(
        "bq_orders",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"order_id", type_factory_->get_int64()},
            {"customer_id", type_factory_->get_int64()},
        });
    catalog_->AddOwnedTable(std::move(bq_orders));

    ExecDdl("CREATE TABLE bq_orders (order_id BIGINT, customer_id BIGINT)");
    ExecDdl("CREATE TABLE profiles (id BIGINT, name VARCHAR)");
    ExecDdl("CREATE TABLE people (id BIGINT, name VARCHAR)");
  }
};

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

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
