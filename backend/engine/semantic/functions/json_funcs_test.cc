#include "backend/engine/semantic/functions/json_funcs.h"

#include <vector>

#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

const ::googlesql::ArrayType* StringArrayType() {
  static ::googlesql::TypeFactory factory;
  const ::googlesql::ArrayType* arr = nullptr;
  factory.MakeArrayType(factory.get_string(), &arr);
  return arr;
}

TEST(JsonFuncsTest, JsonExtractScalarUnquotesString) {
  auto v =
      JsonExtractScalar({Value::String(R"({ "name" : "Jakob", "age" : "6" })"),
                         Value::String("$.name")},
                        /*return_type=*/nullptr);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "Jakob");
}

TEST(JsonFuncsTest, JsonExtractReturnsCompactObject) {
  auto v = JsonExtract(
      {Value::UnvalidatedJsonString(R"({"class":{"students":[{"id":5}]}})"),
       Value::String("$.class")},
      /*return_type=*/nullptr);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), R"({"students":[{"id":5}]})");
}

TEST(JsonFuncsTest, JsonExtractNullFieldReturnsNull) {
  auto v = JsonExtract({Value::String(R"({"a":null})"), Value::String("$.a")},
                       /*return_type=*/nullptr);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(JsonFuncsTest, JsonExtractArraySingleArgRoot) {
  auto v = JsonExtractArray({Value::String("[1,2,3]")}, StringArrayType());
  ASSERT_TRUE(v.ok()) << v.status();
  ASSERT_EQ(v->num_elements(), 3);
  EXPECT_EQ(v->element(0).string_value(), "1");
  EXPECT_EQ(v->element(2).string_value(), "3");
}

TEST(JsonFuncsTest, ParseJsonRoundTrip) {
  auto v = ParseJson({Value::String(R"({"coordinates":[10,20],"id":1})")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->json_string(), R"({"coordinates":[10,20],"id":1})");
}

TEST(JsonFuncsTest, JsonCastBoolFromJsonLiteral) {
  auto v = JsonCastBool({Value::UnvalidatedJsonString("true")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST(JsonFuncsTest, JsonCastInt64FromJsonLiteral) {
  auto v = JsonCastInt64({Value::UnvalidatedJsonString("2005")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 2005);
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
