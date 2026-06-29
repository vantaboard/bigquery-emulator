#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

// --- Functions disposition table ----------------------------------------

TEST(FunctionsTableTest, LookupKnownMappedScalar) {
  // Sanity check on a representative `kDuckdbNative` entry. The
  // lookup is case-insensitive (we accept `ABS`, `abs`, `Abs` all
  // the same).
  const FnEntry* e = LookupFunction("abs");
  ASSERT_NE(e, nullptr);
  EXPECT_EQ(e->disposition, Disposition::kDuckdbNative);
  EXPECT_EQ(e->duckdb_name, "ABS");
  EXPECT_FALSE(e->planned);
  const FnEntry* upper = LookupFunction("ABS");
  ASSERT_NE(upper, nullptr);
  EXPECT_EQ(upper->disposition, Disposition::kDuckdbNative);
  EXPECT_EQ(upper->duckdb_name, "ABS");
}

TEST(FunctionsTableTest, LookupKnownAggregate) {
  // `array_agg` / `string_agg` are `duckdb_rewrite` because ORDER BY /
  // LIMIT modifiers lower to list()/list_slice() in the aggregate emit.
  const FnEntry* agg = LookupFunction("array_agg");
  ASSERT_NE(agg, nullptr);
  EXPECT_EQ(agg->disposition, Disposition::kDuckdbRewrite);
  EXPECT_EQ(agg->duckdb_name, "ARRAY_AGG");
  const FnEntry* string_agg = LookupFunction("string_agg");
  ASSERT_NE(string_agg, nullptr);
  EXPECT_EQ(string_agg->disposition, Disposition::kDuckdbRewrite);
  EXPECT_EQ(string_agg->duckdb_name, "STRING_AGG");
  const FnEntry* sum = LookupFunction("sum");
  ASSERT_NE(sum, nullptr);
  EXPECT_EQ(sum->disposition, Disposition::kDuckdbNative);
  EXPECT_EQ(sum->duckdb_name, "SUM");
}

TEST(FunctionsTableTest, LookupUnsupportedFunction) {
  // `unsupported` disposition: the lookup succeeds but the
  // disposition tells the caller to short-circuit to "" so the
  // engine surfaces UNIMPLEMENTED. Owning plan is the specialised
  // feature policy.
  const FnEntry* e = LookupFunction("approx_quantiles");
  ASSERT_NE(e, nullptr);
  EXPECT_EQ(e->disposition, Disposition::kSemanticExecutor);
  EXPECT_TRUE(e->duckdb_name.empty());
  EXPECT_FALSE(e->planned);
}

// `LookupPlannedDuckdbUdfFunction` was deleted alongside the
// polyfill UDF library plan's wrap-up commit (every former
// `status=planned duckdb_udf` row pointing at the polyfill plan
// either flipped to ready `duckdb_udf` / `duckdb_native` or
// re-pointed to `status=planned semantic_executor` per the
// plan's "no silent approximation" rule). The reverse-direction
// invariant ("no `status=planned duckdb_udf` row points at the
// polyfill plan anymore") is enforced by the YAML genrule + the
// `CoverageMeetsPlanThreshold` test below; tabling the
// per-function planned-shape probe here would just shadow that.

TEST(FunctionsTableTest, LookupReadyDuckdbUdfFunction) {
  // Ready `duckdb_udf` rows store the registered macro name in
  // `duckdb_name=`; the transpiler emits the call identically to a
  // `duckdb_native` row. `mod` and `div` flipped from
  // `status=planned` to ready in the polyfill UDF library's
  // numeric-family commit.
  const FnEntry* mod = LookupFunction("mod");
  ASSERT_NE(mod, nullptr);
  EXPECT_EQ(mod->disposition, Disposition::kDuckdbUdf);
  EXPECT_EQ(mod->duckdb_name, "bq_mod");
  EXPECT_FALSE(mod->planned);
  const FnEntry* div = LookupFunction("div");
  ASSERT_NE(div, nullptr);
  EXPECT_EQ(div->disposition, Disposition::kDuckdbUdf);
  EXPECT_EQ(div->duckdb_name, "bq_div");
  EXPECT_FALSE(div->planned);
}

TEST(FunctionsTableTest, LookupPlannedSemanticExecutorFunction) {
  // SAFE-family rows route to the semantic executor (BigQuery-exact
  // semantics differ from DuckDB's raise-on-overflow). Runtime
  // stays UNIMPLEMENTED until `docs/ENGINE_POLICY.md`
  // lands.
  const FnEntry* e = LookupFunction("safe_divide");
  ASSERT_NE(e, nullptr);
  EXPECT_EQ(e->disposition, Disposition::kSemanticExecutor);
  EXPECT_TRUE(e->duckdb_name.empty());
  EXPECT_FALSE(e->planned);
}

TEST(FunctionsTableTest, LookupUnknownReturnsNull) {
  // Functions not in the YAML disposition table return nullptr; the
  // transpiler treats nullptr the same as a planned-but-not-
  // implemented entry, but the distinction lets the LOG(INFO) tell
  // "configured planned route" from "no disposition row".
  EXPECT_EQ(LookupFunction("totally_made_up_function"), nullptr);
}

TEST(FunctionsTableTest, CoverageMeetsPlanThreshold) {
  // The plan requires the disposition table to cover at least 50
  // BigQuery functions across the math / string / datetime /
  // conditional / array / aggregation / window / unsupported-family
  // categories. We spot-check a few entries from each category here
  // rather than hard-counting the size of the underlying map (which
  // is private to `functions.cc`) -- a regression in the YAML would
  // surface as one of these sentinel lookups returning nullptr.
  const std::vector<std::string> required = {
      // math
      "abs",
      "ceil",
      "floor",
      "round",
      "trunc",
      "sqrt",
      "exp",
      "sign",
      "greatest",
      "least",
      "pi",
      "ln",
      "pow",
      // string
      "concat",
      "length",
      "lower",
      "upper",
      "substr",
      "replace",
      "trim",
      "ltrim",
      "rtrim",
      "lpad",
      "rpad",
      "reverse",
      "starts_with",
      "ends_with",
      // datetime (fallback)
      "current_timestamp",
      "current_date",
      "date_add",
      "format_timestamp",
      // conditional
      "ifnull",
      "coalesce",
      "nullif",
      // array
      "array_length",
      "array_concat",
      "generate_array",
      // aggregation
      "count",
      "sum",
      "avg",
      "min",
      "max",
      "any_value",
      "array_agg",
      "string_agg",
      // local_stub families (specialized-feature-policy)
      "approx_quantiles",
      "ml.predict",
      "keys.new_keyset",
      "net.ip_from_string",
      // window
      "row_number",
      "rank",
      "dense_rank",
  };
  for (const auto& name : required) {
    EXPECT_NE(LookupFunction(name), nullptr) << "missing entry: " << name;
  }
  EXPECT_GE(required.size(), 50u);
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
