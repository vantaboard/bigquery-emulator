#include "backend/engine/semantic/functions/kll_funcs.h"

#include <optional>
#include <vector>

#include "backend/engine/semantic/value.h"
#include "googlesql/public/types/array_type.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

const ::googlesql::ArrayType* Int64ArrayType() {
  return ::googlesql::types::Int64ArrayType();
}

absl::StatusOr<Value> InitInt64(const std::vector<int64_t>& values,
                                int64_t precision = 1000) {
  std::vector<Value> inputs;
  inputs.reserve(values.size());
  for (int64_t v : values) {
    inputs.push_back(Value::Int64(v));
  }
  return KllQuantilesInitInt64Values(inputs, precision);
}

}  // namespace

TEST(KllFuncsTest, InitInt64ProducesNonEmptySketch) {
  auto sketch = InitInt64({1, 2, 3, 4, 5});
  ASSERT_TRUE(sketch.ok()) << sketch.status();
  EXPECT_FALSE(sketch->is_null());
  EXPECT_EQ(sketch->type_kind(), ::googlesql::TYPE_BYTES);
  EXPECT_GT(sketch->bytes_value().size(), 4u);
}

TEST(KllFuncsTest, ExtractInt64HalvesOnSmallSet) {
  auto sketch = InitInt64({1, 2, 3, 4, 5});
  ASSERT_TRUE(sketch.ok()) << sketch.status();

  auto halves = KllQuantilesExtractInt64Scalar(
      {*sketch, Value::Int64(2)}, Int64ArrayType());
  ASSERT_TRUE(halves.ok()) << halves.status();
  ASSERT_TRUE(halves->type()->IsArray());
  ASSERT_EQ(halves->elements().size(), 3u);
  EXPECT_EQ(halves->elements()[0].int64_value(), 1);
  EXPECT_EQ(halves->elements()[1].int64_value(), 3);
  EXPECT_EQ(halves->elements()[2].int64_value(), 5);
}

TEST(KllFuncsTest, ExtractPointInt64P80) {
  auto sketch = InitInt64({1, 2, 3, 4, 5});
  ASSERT_TRUE(sketch.ok()) << sketch.status();

  auto p80 = KllQuantilesExtractPointInt64Scalar({*sketch, Value::Double(0.8)});
  ASSERT_TRUE(p80.ok()) << p80.status();
  EXPECT_EQ(p80->int64_value(), 4);
}

TEST(KllFuncsTest, MergePartialRoundTrip) {
  auto sketch_a = InitInt64({1, 2, 3});
  auto sketch_b = InitInt64({4, 5});
  ASSERT_TRUE(sketch_a.ok()) << sketch_a.status();
  ASSERT_TRUE(sketch_b.ok()) << sketch_b.status();

  auto merged = KllQuantilesMergePartialAggregate(
      {{*sketch_a, *sketch_b}});
  ASSERT_TRUE(merged.ok()) << merged.status();
  EXPECT_FALSE(merged->is_null());

  auto halves = KllQuantilesExtractInt64Scalar(
      {*merged, Value::Int64(2)}, Int64ArrayType());
  ASSERT_TRUE(halves.ok()) << halves.status();
  EXPECT_EQ(halves->elements()[0].int64_value(), 1);
  EXPECT_EQ(halves->elements()[2].int64_value(), 5);
}

TEST(KllFuncsTest, MergeInt64AggregateReturnsHalves) {
  auto sketch_a = InitInt64({1, 2, 3});
  auto sketch_b = InitInt64({4, 5});
  ASSERT_TRUE(sketch_a.ok()) << sketch_a.status();
  ASSERT_TRUE(sketch_b.ok()) << sketch_b.status();

  auto merged = KllQuantilesMergeInt64Aggregate(
      {{*sketch_a, *sketch_b}, {Value::Int64(2)}}, Int64ArrayType());
  ASSERT_TRUE(merged.ok()) << merged.status();
  ASSERT_TRUE(merged->type()->IsArray());
  EXPECT_EQ(merged->elements().size(), 3u);
  EXPECT_EQ(merged->elements()[0].int64_value(), 1);
  EXPECT_EQ(merged->elements()[2].int64_value(), 5);
}

TEST(KllFuncsTest, InitWithWeightSkewsMedian) {
  std::vector<Value> inputs = {Value::Int64(1), Value::Int64(2),
                               Value::Int64(3), Value::Int64(4),
                               Value::Int64(5)};
  std::vector<Value> weights = {Value::Int64(1), Value::Int64(3),
                                Value::Int64(1), Value::Int64(1),
                                Value::Int64(1)};
  auto sketch = KllQuantilesInitInt64Values(inputs, 1000, &weights);
  ASSERT_TRUE(sketch.ok()) << sketch.status();

  auto halves = KllQuantilesExtractInt64Scalar(
      {*sketch, Value::Int64(2)}, Int64ArrayType());
  ASSERT_TRUE(halves.ok()) << halves.status();
  EXPECT_EQ(halves->elements()[0].int64_value(), 1);
  EXPECT_EQ(halves->elements()[1].int64_value(), 2);
  EXPECT_EQ(halves->elements()[2].int64_value(), 5);
}

TEST(KllFuncsTest, InvalidSketchBytesRejected) {
  auto got = KllQuantilesExtractInt64Scalar(
      {Value::Bytes("not-a-sketch"), Value::Int64(2)}, Int64ArrayType());
  EXPECT_FALSE(got.ok());
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
