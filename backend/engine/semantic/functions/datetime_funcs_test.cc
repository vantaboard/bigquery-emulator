

#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

using ::googlesql::Value;

TEST(DatetimeFuncsTest, ParseDateBasic) {
  Value fmt = Value::String("%Y%m%d");
  Value date_str = Value::String("20081225");
  std::vector<Value> args = {fmt, date_str};
  auto got = ParseDate(args);
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->date_value(), 14238);
}

TEST(DatetimeFuncsTest, DateAddDay) {
  Value fmt = Value::String("%Y-%m-%d");
  Value base_str = Value::String("2020-09-22");
  auto base = ParseDate({fmt, base_str});
  ASSERT_TRUE(base.ok());
  std::vector<Value> args = {
      *base,
      Value::Int64(1),
      Value::Int64(static_cast<int64_t>(::googlesql::functions::DAY))};
  auto got = DateAddSubDiffTrunc("date_add", args);
  ASSERT_TRUE(got.ok());
  Value next_str = Value::String("2020-09-23");
  auto next = ParseDate({fmt, next_str});
  ASSERT_TRUE(next.ok());
  EXPECT_EQ(got->date_value(), next->date_value());
}

TEST(DatetimeFuncsTest, DateAddMonthEndClampsToLastDay) {
  Value fmt = Value::String("%Y-%m-%d");
  Value base_str = Value::String("2024-01-31");
  auto base = ParseDate({fmt, base_str});
  ASSERT_TRUE(base.ok());
  std::vector<Value> args = {
      *base,
      Value::Int64(1),
      Value::Int64(static_cast<int64_t>(::googlesql::functions::MONTH))};
  auto got = DateAddSubDiffTrunc("date_add", args);
  ASSERT_TRUE(got.ok()) << got.status();
  Value expected_str = Value::String("2024-02-29");
  auto expected = ParseDate({fmt, expected_str});
  ASSERT_TRUE(expected.ok());
  EXPECT_EQ(got->date_value(), expected->date_value());
}

TEST(DatetimeFuncsTest, DateDiffMonthBoundaryWithinSameMonth) {
  Value fmt = Value::String("%Y-%m-%d");
  Value start_str = Value::String("2024-01-31");
  Value end_str = Value::String("2024-01-01");
  auto start = ParseDate({fmt, start_str});
  auto end = ParseDate({fmt, end_str});
  ASSERT_TRUE(start.ok());
  ASSERT_TRUE(end.ok());
  std::vector<Value> args = {
      *end,
      *start,
      Value::Int64(static_cast<int64_t>(::googlesql::functions::MONTH))};
  auto got = DateAddSubDiffTrunc("date_diff", args);
  ASSERT_TRUE(got.ok()) << got.status();
  EXPECT_EQ(got->int64_value(), 0);
}

TEST(DatetimeFuncsTest, FormatDatePercentF) {
  Value fmt = Value::String("%F");
  Value date_val = Value::Date(14238);
  std::vector<Value> args = {fmt, date_val};
  auto got = FormatDate(args);
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->string_value(), "2008-12-25");
}

TEST(DatetimeFuncsTest, DateConstructorFromIsoString) {
  Value iso = Value::String("1800-01-01");
  auto got = DateConstructor({iso});
  ASSERT_TRUE(got.ok()) << got.status();
  Value fmt = Value::String("%Y-%m-%d");
  auto roundtrip = ParseDate({fmt, iso});
  ASSERT_TRUE(roundtrip.ok());
  EXPECT_EQ(got->date_value(), roundtrip->date_value());
}

TEST(DatetimeFuncsTest, DateConstructorFromYmdInts) {
  Value year = Value::Int64(2020);
  Value month = Value::Int64(9);
  Value day = Value::Int64(22);
  auto got = DateConstructor({year, month, day});
  ASSERT_TRUE(got.ok()) << got.status();
  Value fmt = Value::String("%Y-%m-%d");
  Value date_str = Value::String("2020-09-22");
  auto roundtrip = ParseDate({fmt, date_str});
  ASSERT_TRUE(roundtrip.ok());
  EXPECT_EQ(got->date_value(), roundtrip->date_value());
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
