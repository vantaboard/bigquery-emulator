#include "backend/engine/duckdb/transpiler/node_dispositions.h"

#include <set>
#include <string>

#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace {

// `googlesqlite-15-specialized-stubs.plan.md` is the canonical owner of
// every `kUnsupported` row. The plan summary in
// `.cursor/plans/local-execution-roadmap-index.plan.md` calls this
// out explicitly; pin it here so a future YAML edit that points an
// unsupported row at the wrong plan fails the build.
constexpr absl::string_view kSpecializedFeaturePolicy =
    "googlesqlite-15-specialized-stubs.plan.md";

TEST(NodeDispositionTableTest, LookupReturnsExpectedRows) {
  // Sanity check on one row per disposition (sentinel rows the
  // generator must always emit). Adds confidence that the YAML
  // generator produces the expected per-disposition initializer
  // shape; the broader coverage check lives in the
  // `EveryDispositionIsReachable` test below.
  const auto* query_stmt = LookupNodeDisposition("ResolvedQueryStmt");
  ASSERT_NE(query_stmt, nullptr);
  EXPECT_EQ(query_stmt->disposition, Disposition::kDuckdbNative);
  EXPECT_FALSE(query_stmt->planned);

  const auto* explain_stmt = LookupNodeDisposition("ResolvedExplainStmt");
  ASSERT_NE(explain_stmt, nullptr);
  EXPECT_EQ(explain_stmt->disposition, Disposition::kUnsupported);
  EXPECT_EQ(explain_stmt->plan, kSpecializedFeaturePolicy);

  const auto* make_struct = LookupNodeDisposition("ResolvedMakeStruct");
  ASSERT_NE(make_struct, nullptr);
  EXPECT_EQ(make_struct->disposition, Disposition::kDuckdbRewrite);

  const auto* create_table = LookupNodeDisposition("ResolvedCreateTableStmt");
  ASSERT_NE(create_table, nullptr);
  EXPECT_EQ(create_table->disposition, Disposition::kControlOp);
  EXPECT_TRUE(create_table->planned);
}

TEST(NodeDispositionTableTest, LookupUnknownReturnsNull) {
  EXPECT_EQ(LookupNodeDisposition("ResolvedTotallyMadeUpNode"), nullptr);
  // The YAML is case-sensitive (mirrors the GoogleSQL class name
  // spelling); a wrong-case query is treated as "not in the table".
  EXPECT_EQ(LookupNodeDisposition("resolvedquerystmt"), nullptr);
}

TEST(NodeDispositionTableTest, EveryNodeApplicableDispositionIsReachable) {
  // Every `Disposition` enum value that is meaningful at the
  // node-kind level appears at least once in the table. A
  // regression that drops the last entry of a given route (e.g.
  // mass-promoting every `kUnsupported` row to a real route) would
  // otherwise quietly leave the route un-tested.
  //
  // `kDuckdbUdf` is intentionally *not* expected here: UDF
  // disposition only applies to per-function lowering (the
  // `functions.yaml` table tests
  // `LookupPlannedDuckdbUdfFunction`). No `ResolvedAST` node kind
  // routes through a UDF; UDFs lower individual function calls
  // inside a node, not the node itself.
  //
  // `kLocalStub` IS expected at the node level today:
  // `ResolvedCreateModelStmt` is the statement-level stub that
  // `googlesqlite-15-specialized-stubs.plan.md` introduces (the
  // coordinator pre-dispatches `RESOLVED_CREATE_MODEL_STMT` to
  // `backend/engine/control/stubs/create_model.cc`). The function-
  // level stubs (`KEYS.NEW_KEYSET`, ...) live in
  // `functions.yaml`, not here.
  const std::set<Disposition> kNodeApplicable = {
      Disposition::kDuckdbNative,
      Disposition::kDuckdbRewrite,
      Disposition::kSemanticExecutor,
      Disposition::kControlOp,
      Disposition::kLocalStub,
      Disposition::kUnsupported,
  };
  std::set<Disposition> seen;
  for (const auto& view : internal::AllNodeDispositions()) {
    ASSERT_NE(view.entry, nullptr);
    seen.insert(view.entry->disposition);
  }
  for (Disposition d : kNodeApplicable) {
    SCOPED_TRACE(DispositionToString(d));
    EXPECT_NE(seen.find(d), seen.end())
        << "node_dispositions.yaml is missing a row for this disposition";
  }
  EXPECT_EQ(seen.find(Disposition::kDuckdbUdf), seen.end())
      << "kDuckdbUdf is per-function, not per-node-kind; tested via "
         "functions.yaml LookupPlannedDuckdbUdfFunction instead";
}

TEST(NodeDispositionTableTest, EveryUnsupportedRowPointsAtPolicyPlan) {
  // Every `kUnsupported` row must point at
  // `googlesqlite-15-specialized-stubs.plan.md`, per the plan's done
  // criteria. The YAML generator already rejects an unsupported
  // row without any plan pointer; this test pins the *value*.
  int unsupported_rows = 0;
  for (const auto& view : internal::AllNodeDispositions()) {
    ASSERT_NE(view.entry, nullptr);
    if (view.entry->disposition != Disposition::kUnsupported) continue;
    unsupported_rows++;
    SCOPED_TRACE(std::string(view.name));
    EXPECT_EQ(view.entry->plan, kSpecializedFeaturePolicy);
  }
  // Spot check that we actually have unsupported rows in the YAML;
  // a buggy generator that silently dropped every row would also
  // make the previous loop trivially pass.
  EXPECT_GT(unsupported_rows, 0);
}

TEST(NodeDispositionTableTest, EveryLocalStubRowPointsAtPolicyPlan) {
  // Same posture-row contract as `EveryUnsupportedRowPointsAtPolicy
  // Plan` above, applied to `kLocalStub`. Every stub row must point
  // at `googlesqlite-15-specialized-stubs.plan.md` so a reader can trace
  // why the deliberate-stub posture was chosen. The YAML generator
  // rejects a `local_stub` row without any `plan=` pointer; this
  // test pins the *value*. Today's only entry is
  // `ResolvedCreateModelStmt`; future statement-level stubs (e.g.
  // JS UDF metadata, once plan 13's deferred UDF body storage
  // lands) join the same row.
  int local_stub_rows = 0;
  for (const auto& view : internal::AllNodeDispositions()) {
    ASSERT_NE(view.entry, nullptr);
    if (view.entry->disposition != Disposition::kLocalStub) continue;
    local_stub_rows++;
    SCOPED_TRACE(std::string(view.name));
    EXPECT_EQ(view.entry->plan, kSpecializedFeaturePolicy);
  }
  // Spot check that `local_stub` rows exist. Setting this to >0
  // (rather than ==1) so adding the next statement-level stub
  // family does not require a test update.
  EXPECT_GT(local_stub_rows, 0);
}

TEST(NodeDispositionTableTest, RegistryNonTrivial) {
  // Final guardrail: the table is not empty. A generator regression
  // that produced an empty `.inc` would make every other lookup
  // test pass (every "expected row" would have a null lookup that
  // the ASSERT_NE catches, but the more-permissive tests above
  // would silently pass too).
  EXPECT_GT(internal::AllNodeDispositions().size(), 50u)
      << "node_dispositions.yaml should cover every SHAPE_TRACKER.md "
         "row (~80 entries today)";
}

}  // namespace
}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
