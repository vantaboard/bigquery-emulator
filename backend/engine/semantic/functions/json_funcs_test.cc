

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
  Value doc = Value::String(R"({ "name" : "Jakob", "age" : "6" })");
  Value path = Value::String("$.name");
  auto v = JsonExtractScalar({doc, path}, /*return_type=*/nullptr);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "Jakob");
}

TEST(JsonFuncsTest, JsonExtractReturnsCompactObject) {
  Value doc =
      Value::UnvalidatedJsonString(R"({"class":{"students":[{"id":5}]}})");
  Value path = Value::String("$.class");
  auto v = JsonExtract({doc, path}, /*return_type=*/nullptr);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), R"({"students":[{"id":5}]})");
}

TEST(JsonFuncsTest, JsonExtractNullFieldReturnsNull) {
  Value doc = Value::String(R"({"a":null})");
  Value path = Value::String("$.a");
  auto v = JsonExtract({doc, path}, /*return_type=*/nullptr);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(JsonFuncsTest, JsonExtractArraySingleArgRoot) {
  Value doc = Value::String("[1,2,3]");
  auto v = JsonExtractArray({doc}, StringArrayType());
  ASSERT_TRUE(v.ok()) << v.status();
  ASSERT_EQ(v->num_elements(), 3);
  EXPECT_EQ(v->element(0).string_value(), "1");
  EXPECT_EQ(v->element(2).string_value(), "3");
}

TEST(JsonFuncsTest, ParseJsonRoundTrip) {
  Value json_text = Value::String(R"({"coordinates":[10,20],"id":1})");
  auto v = ParseJson({json_text});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->json_string(), R"({"coordinates":[10,20],"id":1})");
}

TEST(JsonFuncsTest, JsonCastBoolFromJsonLiteral) {
  Value literal = Value::UnvalidatedJsonString("true");
  auto v = JsonCastBool({literal});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST(JsonFuncsTest, JsonCastInt64FromJsonLiteral) {
  Value literal = Value::UnvalidatedJsonString("2005");
  auto v = JsonCastInt64({literal});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 2005);
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
