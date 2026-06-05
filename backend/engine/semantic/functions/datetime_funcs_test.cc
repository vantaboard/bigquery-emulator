#include "backend/engine/semantic/functions/datetime_funcs.h"

#include <vector>

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
  std::vector<Value> args = {Value::String("%Y%m%d"),
                             Value::String("20081225")};
  auto got = ParseDate(args);
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->date_value(), 14238);
}

TEST(DatetimeFuncsTest, DateAddDay) {
  auto base =
      ParseDate({Value::String("%Y-%m-%d"), Value::String("2020-09-22")});
  ASSERT_TRUE(base.ok());
  std::vector<Value> args = {
      *base,
      Value::Int64(1),
      Value::Int64(static_cast<int64_t>(::googlesql::functions::DAY))};
  auto got = DateAddSubDiffTrunc("date_add", args);
  ASSERT_TRUE(got.ok());
  auto next =
      ParseDate({Value::String("%Y-%m-%d"), Value::String("2020-09-23")});
  ASSERT_TRUE(next.ok());
  EXPECT_EQ(got->date_value(), next->date_value());
}

TEST(DatetimeFuncsTest, FormatDatePercentF) {
  std::vector<Value> args = {Value::String("%F"), Value::Date(14238)};
  auto got = FormatDate(args);
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->string_value(), "2008-12-25");
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
