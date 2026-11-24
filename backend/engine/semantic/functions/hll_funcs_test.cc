#include "backend/engine/semantic/functions/hll_funcs.h"

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "absl/strings/escaping.h"
#include "backend/engine/semantic/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

std::string Init(const std::vector<Value>& values, int64_t precision) {
  auto sketch = HllCountInitValues(values, precision);
  EXPECT_TRUE(sketch.ok()) << sketch.status();
  return absl::Base64Escape(sketch->bytes_value());
}

std::vector<Value> Strings() {
  return {Value::String("customer_id_1"),
          Value::String("customer_id_2"),
          Value::String("customer_id_3"),
          Value::String("")};
}

std::vector<Value> Int64s() {
  return {Value::Int64(0),
          Value::Int64(1),
          Value::Int64(-1),
          Value::Int64(std::numeric_limits<int64_t>::max()),
          Value::Int64(std::numeric_limits<int64_t>::min()),
          Value::Int64(42)};
}

std::vector<Value> Bytes() {
  return {Value::Bytes(std::string()),
          Value::Bytes(std::string("\x00", 1)),
          Value::Bytes("abc"),
          Value::Bytes("\xff\xfe")};
}

TEST(HllFuncsTest, InitMatchesBigQueryForStrings) {
  EXPECT_EQ(Init(Strings(), 10), "CHAQBBgCIAuCBxEQBBgKIA8yCdYjrL0B9wPNCA==");
  EXPECT_EQ(Init(Strings(), 15), "CHAQBBgCIAuCBxMQBBgPIBQyC9f1CPmqL+p9pJMC");
  EXPECT_EQ(Init(Strings(), 20),
            "CHAQBBgCIAuCBxYQBBgUIBkyDvm1nQKP3uoL1rof+OhE");
  EXPECT_EQ(Init(Strings(), 21),
            "CHAQBBgCIAuCBxYQBBgVIBkyDvm1nQKP3uoL1rof+OhE");
  EXPECT_EQ(Init(Strings(), 24),
            "CHAQBBgCIAuCBxkQBBgYIBkyEfm1nQKJzOe/B7/V7geAnpoR");
}

TEST(HllFuncsTest, InitMatchesBigQueryForInt64s) {
  EXPECT_EQ(Init(Int64s(), 10), "CHAQBhgCIAiCBxQQBhgKIA8yDJ1YzRazFuEvjgT9IA==");
  EXPECT_EQ(Init(Int64s(), 15),
            "CHAQBhgCIAiCBxoQBhgPIBQyErKHFqfTBcnMBb34C7eDAaefCA==");
  EXPECT_EQ(Init(Int64s(), 24),
            "CHAQBhgCIAiCByEQBhgYIBkyGdfswQXY6bQBlpKzAYz9ngPY6YcCmq2t3AY=");
}

TEST(HllFuncsTest, InitMatchesBigQueryForBytes) {
  EXPECT_EQ(Init(Bytes(), 15), "CHAQBBgCIAuCBxMQBBgPIBQyC/MJrN8FuIwD4csg");
}

TEST(HllFuncsTest, MatchesBigQueryForTheGatewayEndToEndCases) {
  EXPECT_EQ(
      Init({Value::String("customer_id_1"), Value::String("customer_id_2")},
           10),
      "CHAQAhgCIAuCBw0QAhgKIA8yBfnkAc0I");
  EXPECT_EQ(
      Init({Value::String("customer_id_2"), Value::String("customer_id_2")},
           10),
      "CHAQAhgCIAuCBwsQARgKIA8yA/nkAQ==");
  EXPECT_EQ(Init({Value::String("customer_id_3")}, 10),
            "CHAQARgCIAuCBwsQARgKIA8yA4LhAQ==");

  auto ua = HllCountInitValues(
      {Value::String("customer_id_1"), Value::String("customer_id_2")}, 15);
  auto br = HllCountInitValues(
      {Value::String("customer_id_3"), Value::String("customer_id_3")}, 15);
  auto cz = HllCountInitValues(
      {Value::String("customer_id_2"), Value::String("customer_id_2")}, 15);
  ASSERT_TRUE(ua.ok() && br.ok() && cz.ok());
  auto partial = HllCountMergePartialAggregate({{*ua, *br, *cz}});
  ASSERT_TRUE(partial.ok()) << partial.status();
  EXPECT_EQ(absl::Base64Escape(partial->bytes_value()),
            "CHAQBhgCIAuCBxAQAxgPIBQyCNCgOOp9pJMC");
}

TEST(HllFuncsTest, InitOfOnlyNullsIsNull) {
  auto sketch = HllCountInitValues({Value::NullString()}, 15);
  ASSERT_TRUE(sketch.ok()) << sketch.status();
  EXPECT_TRUE(sketch->is_null());
}

TEST(HllFuncsTest, InitRejectsPrecisionsOutOfRange) {
  EXPECT_FALSE(HllCountInitValues(Strings(), 9).ok());
  EXPECT_FALSE(HllCountInitValues(Strings(), 25).ok());
}

TEST(HllFuncsTest, MergeAndExtractCountDistinctValues) {
  auto first = HllCountInitValues(
      {Value::String("customer_id_1"), Value::String("customer_id_2")}, 15);
  auto second = HllCountInitValues(
      {Value::String("customer_id_2"), Value::String("customer_id_3")}, 15);
  ASSERT_TRUE(first.ok() && second.ok());

  auto merge = HllCountMergeAggregate({{*first, Value::NullBytes(), *second}});
  ASSERT_TRUE(merge.ok()) << merge.status();
  EXPECT_EQ(merge->int64_value(), 3);

  auto partial = HllCountMergePartialAggregate({{*first, *second}});
  ASSERT_TRUE(partial.ok()) << partial.status();
  auto extract = HllCountExtractScalar({*partial});
  ASSERT_TRUE(extract.ok()) << extract.status();
  EXPECT_EQ(extract->int64_value(), 3);

  auto single = HllCountExtractScalar({*first});
  ASSERT_TRUE(single.ok()) << single.status();
  EXPECT_EQ(single->int64_value(), 2);
}

TEST(HllFuncsTest, MergeOfOnlyNulls) {
  auto merge = HllCountMergeAggregate({{Value::NullBytes()}});
  ASSERT_TRUE(merge.ok()) << merge.status();
  EXPECT_EQ(merge->int64_value(), 0);

  auto partial = HllCountMergePartialAggregate({{Value::NullBytes()}});
  ASSERT_TRUE(partial.ok()) << partial.status();
  EXPECT_TRUE(partial->is_null());
}

TEST(HllFuncsTest, MergeRejectsSketchesOfDifferentTypes) {
  auto strings = HllCountInitValues(Strings(), 15);
  auto int64s = HllCountInitValues(Int64s(), 15);
  ASSERT_TRUE(strings.ok() && int64s.ok());
  EXPECT_FALSE(HllCountMergeAggregate({{*strings, *int64s}}).ok());
}

TEST(HllFuncsTest, ExtractRejectsBytesThatAreNotASketch) {
  EXPECT_FALSE(HllCountExtractScalar({Value::Bytes("not a sketch")}).ok());
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
