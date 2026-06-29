#include "backend/engine/duckdb/transpiler/node_dispositions.h"

#include <set>

#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace {

TEST(NodeDispositionTableTest, LookupReturnsExpectedRows) {
  const auto* query_stmt = LookupNodeDisposition("ResolvedQueryStmt");
  ASSERT_NE(query_stmt, nullptr);
  EXPECT_EQ(query_stmt->disposition, Disposition::kDuckdbNative);
  EXPECT_FALSE(query_stmt->planned);

  const auto* explain_stmt = LookupNodeDisposition("ResolvedExplainStmt");
  ASSERT_NE(explain_stmt, nullptr);
  EXPECT_EQ(explain_stmt->disposition, Disposition::kUnsupported);

  const auto* make_struct = LookupNodeDisposition("ResolvedMakeStruct");
  ASSERT_NE(make_struct, nullptr);
  EXPECT_EQ(make_struct->disposition, Disposition::kDuckdbRewrite);

  const auto* create_table = LookupNodeDisposition("ResolvedCreateTableStmt");
  ASSERT_NE(create_table, nullptr);
  EXPECT_EQ(create_table->disposition, Disposition::kControlOp);
  EXPECT_FALSE(create_table->planned);
}

TEST(NodeDispositionTableTest, LookupUnknownReturnsNull) {
  EXPECT_EQ(LookupNodeDisposition("ResolvedTotallyMadeUpNode"), nullptr);
  EXPECT_EQ(LookupNodeDisposition("resolvedquerystmt"), nullptr);
}

TEST(NodeDispositionTableTest, EveryNodeApplicableDispositionIsReachable) {
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

TEST(NodeDispositionTableTest, UnsupportedAndLocalStubRowsExist) {
  int unsupported_rows = 0;
  int local_stub_rows = 0;
  for (const auto& view : internal::AllNodeDispositions()) {
    ASSERT_NE(view.entry, nullptr);
    if (view.entry->disposition == Disposition::kUnsupported) {
      unsupported_rows++;
    }
    if (view.entry->disposition == Disposition::kLocalStub) {
      local_stub_rows++;
    }
  }
  EXPECT_GT(unsupported_rows, 0);
  EXPECT_GT(local_stub_rows, 0);
}

TEST(NodeDispositionTableTest, RegistryNonTrivial) {
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
