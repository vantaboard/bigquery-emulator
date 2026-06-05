#include "backend/engine/semantic/functions/array_funcs.h"

#include <vector>

#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

using ::googlesql::Value;

const ::googlesql::ArrayType* Int64ArrayType() {
  return ::googlesql::types::Int64ArrayType();
}

}  // namespace

TEST(ArrayFuncsTest, GenerateArrayBasic) {
  const ::googlesql::Type* ret = Int64ArrayType();
  auto got = GenerateArray({Value::Int64(1), Value::Int64(5)}, ret);
  ASSERT_TRUE(got.ok());
  ASSERT_EQ(got->num_elements(), 5);
  EXPECT_EQ(got->element(0).int64_value(), 1);
  EXPECT_EQ(got->element(4).int64_value(), 5);
}

TEST(ArrayFuncsTest, ArrayConcatMerges) {
  const ::googlesql::Type* ret = Int64ArrayType();
  Value a = Value::Array(ret->AsArray(), {Value::Int64(1), Value::Int64(2)});
  Value b = Value::Array(ret->AsArray(), {Value::Int64(3)});
  auto got = ArrayConcat({a, b}, ret);
  ASSERT_TRUE(got.ok());
  ASSERT_EQ(got->num_elements(), 3);
  EXPECT_EQ(got->element(2).int64_value(), 3);
}

TEST(ArrayFuncsTest, ArrayLengthAndReverse) {
  Value a = Value::Array(Int64ArrayType(),
                         {Value::Int64(1), Value::Int64(2), Value::Int64(3)});
  auto len = ArrayLength({a});
  ASSERT_TRUE(len.ok());
  EXPECT_EQ(len->int64_value(), 3);
  auto rev = ArrayReverse({a}, Int64ArrayType());
  ASSERT_TRUE(rev.ok());
  EXPECT_EQ(rev->element(0).int64_value(), 3);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
