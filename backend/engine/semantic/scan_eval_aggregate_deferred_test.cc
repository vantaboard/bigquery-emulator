#include <memory>
#include <utility>
#include <vector>

#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

TEST(ScanEvalAggregateDeferredTest, DeferredComputedColumnEvaluatesValue) {
  ::googlesql::TypeFactory type_factory;
  const ::googlesql::Type* int64 = type_factory.get_int64();
  const ::googlesql::Type* bytes = type_factory.get_bytes();

  ::googlesql::ResolvedColumn out_col(
      /*column_id=*/300,
      ::googlesql::IdString::MakeGlobal("$query"),
      ::googlesql::IdString::MakeGlobal("v"),
      int64);
  ::googlesql::ResolvedColumn side_col(
      /*column_id=*/301,
      ::googlesql::IdString::MakeGlobal("$query"),
      ::googlesql::IdString::MakeGlobal("_se"),
      bytes);

  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumnBase>>
      aggregate_list;
  aggregate_list.push_back(::googlesql::MakeResolvedDeferredComputedColumn(
      out_col,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(7)),
      side_col));

  auto agg_scan = ::googlesql::MakeResolvedAggregateScan(
      /*column_list=*/{out_col, side_col},
      ::googlesql::MakeResolvedSingleRowScan(),
      /*group_by_list=*/{},
      std::move(aggregate_list),
      /*grouping_set_list=*/{},
      /*rollup_column_list=*/{});

  EvalContext ctx{.project_id = "test"};
  auto rows_or = MaterializeScan(agg_scan.get(), ctx);
  ASSERT_TRUE(rows_or.ok()) << rows_or.status();
  ASSERT_EQ(rows_or->size(), 1u);
  const ColumnBindings& row = rows_or->at(0);
  auto vit = row.find(out_col.column_id());
  ASSERT_NE(vit, row.end());
  EXPECT_EQ(vit->second.int64_value(), 7);
  auto sit = row.find(side_col.column_id());
  ASSERT_NE(sit, row.end());
  EXPECT_TRUE(sit->second.is_null());
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
