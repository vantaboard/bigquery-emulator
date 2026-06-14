#include <memory>
#include <utility>
#include <vector>

#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

std::unique_ptr<const ::googlesql::ResolvedScan> MakePrivacyAggregateScan(
    ::googlesql::ResolvedNodeKind kind,
    const ::googlesql::ResolvedColumn& out_col,
    const ::googlesql::ResolvedColumn& side_col) {
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumnBase>>
      aggregate_list;
  aggregate_list.push_back(::googlesql::MakeResolvedDeferredComputedColumn(
      out_col,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(7)),
      side_col));

  switch (kind) {
    case ::googlesql::RESOLVED_ANONYMIZED_AGGREGATE_SCAN:
      return ::googlesql::MakeResolvedAnonymizedAggregateScan(
          /*column_list=*/{out_col, side_col},
          ::googlesql::MakeResolvedSingleRowScan(),
          /*group_by_list=*/{},
          std::move(aggregate_list),
          /*grouping_set_list=*/{},
          /*rollup_column_list=*/{},
          ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(1)),
          /*anonymization_option_list=*/{});
    case ::googlesql::RESOLVED_DIFFERENTIAL_PRIVACY_AGGREGATE_SCAN:
      return ::googlesql::MakeResolvedDifferentialPrivacyAggregateScan(
          /*column_list=*/{out_col, side_col},
          ::googlesql::MakeResolvedSingleRowScan(),
          /*group_by_list=*/{},
          std::move(aggregate_list),
          /*grouping_set_list=*/{},
          /*rollup_column_list=*/{},
          /*group_selection_threshold_expr=*/nullptr,
          /*option_list=*/{});
    case ::googlesql::RESOLVED_AGGREGATION_THRESHOLD_AGGREGATE_SCAN:
      return ::googlesql::MakeResolvedAggregationThresholdAggregateScan(
          /*column_list=*/{out_col, side_col},
          ::googlesql::MakeResolvedSingleRowScan(),
          /*group_by_list=*/{},
          std::move(aggregate_list),
          /*grouping_set_list=*/{},
          /*rollup_column_list=*/{},
          /*option_list=*/{});
    default:
      return nullptr;
  }
}

class PrivacyAggregateStubTest
    : public ::testing::TestWithParam<::googlesql::ResolvedNodeKind> {};

TEST_P(PrivacyAggregateStubTest, MaterializeScanEvaluatesPlainAggregate) {
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

  auto scan = MakePrivacyAggregateScan(GetParam(), out_col, side_col);
  ASSERT_NE(scan, nullptr);

  EvalContext ctx{.project_id = "test"};
  auto rows_or = MaterializeScan(scan.get(), ctx);
  ASSERT_TRUE(rows_or.ok()) << rows_or.status();
  ASSERT_EQ(rows_or->size(), 1u);
  const ColumnBindings& row = rows_or->at(0);
  auto vit = row.find(out_col.column_id());
  ASSERT_NE(vit, row.end());
  EXPECT_EQ(vit->second.int64_value(), 7);
}

INSTANTIATE_TEST_SUITE_P(
    PrivacyAggregateScanKinds,
    PrivacyAggregateStubTest,
    ::testing::Values(::googlesql::RESOLVED_ANONYMIZED_AGGREGATE_SCAN,
                      ::googlesql::RESOLVED_DIFFERENTIAL_PRIVACY_AGGREGATE_SCAN,
                      ::googlesql::RESOLVED_AGGREGATION_THRESHOLD_AGGREGATE_SCAN));

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
