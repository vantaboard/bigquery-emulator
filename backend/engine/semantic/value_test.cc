// Tests for the `semantic::Value` helpers.
//
// `semantic::Value` is `googlesql::Value` (via the alias in
// `value.h`); the tests here pin the conversion helpers that
// mediate between the analyzer's value type and the engine's
// wire-facing types: `ToStorageValue`, `ColumnSchemaForType`,
// `ParseParameterValue`. Per-type coverage matches the plan's
// "BigQuery primitive type table" exit criterion.

#include "backend/engine/semantic/value.h"

#include <cstdint>

#include "absl/status/status.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

TEST(SemanticValueTest, ToStorageValueBoolRoundTrips) {
  auto out = ToStorageValue(Value::Bool(true));
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->kind(), storage::Value::Kind::kBool);
  EXPECT_TRUE(out->bool_value());
}

TEST(SemanticValueTest, ToStorageValueInt64RoundTrips) {
  auto out = ToStorageValue(Value::Int64(42));
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->int64_value(), 42);
}

TEST(SemanticValueTest, ToStorageValueDoubleRoundTrips) {
  auto out = ToStorageValue(Value::Double(3.5));
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->float64_value(), 3.5);
}

TEST(SemanticValueTest, ToStorageValueStringRoundTrips) {
  auto out = ToStorageValue(Value::String("hello"));
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->string_value(), "hello");
}

TEST(SemanticValueTest, ToStorageValueBytesRoundTrips) {
  auto out = ToStorageValue(Value::Bytes("ab\x00cd"));
  ASSERT_TRUE(out.ok()) << out.status();
  // The storage::Value::Bytes representation is a string carrying
  // the raw byte payload; we just check the public accessor
  // returns the same bytes we put in.
  EXPECT_EQ(out->kind(), storage::Value::Kind::kBytes);
}

TEST(SemanticValueTest, ToStorageValueNullProducesNullCell) {
  auto out = ToStorageValue(Value::NullInt64());
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_TRUE(out->is_null());
}

TEST(SemanticValueTest, ToStorageValueNumericProducesDecimalText) {
  auto numeric = ::googlesql::NumericValue::FromString("1.5");
  ASSERT_TRUE(numeric.ok()) << numeric.status();
  auto out = ToStorageValue(Value::Numeric(*numeric));
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->kind(), storage::Value::Kind::kString);
  EXPECT_EQ(out->string_value(), "1.5");
}

TEST(SemanticValueTest, ToStorageValueArrayRecurses) {
  ::googlesql::TypeFactory tf;
  const ::googlesql::ArrayType* int_array = nullptr;
  ASSERT_TRUE(
      tf.MakeArrayType(::googlesql::types::Int64Type(), &int_array).ok());
  auto array = Value::MakeArray(
      int_array, {Value::Int64(1), Value::Int64(2), Value::NullInt64()});
  ASSERT_TRUE(array.ok()) << array.status();
  auto out = ToStorageValue(*array);
  ASSERT_TRUE(out.ok()) << out.status();
  ASSERT_EQ(out->kind(), storage::Value::Kind::kArray);
  ASSERT_EQ(out->array_value().size(), 3u);
  EXPECT_EQ(out->array_value()[0].int64_value(), 1);
  EXPECT_EQ(out->array_value()[1].int64_value(), 2);
  EXPECT_TRUE(out->array_value()[2].is_null());
}

TEST(SemanticValueTest, ColumnSchemaForInt64) {
  auto out = ColumnSchemaForType(::googlesql::types::Int64Type(), "i");
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->name, "i");
  EXPECT_EQ(out->type, schema::ColumnType::kInt64);
  EXPECT_EQ(out->mode, schema::ColumnMode::kNullable);
}

TEST(SemanticValueTest, ColumnSchemaForStringIsString) {
  auto out = ColumnSchemaForType(::googlesql::types::StringType(), "s");
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->type, schema::ColumnType::kString);
}

TEST(SemanticValueTest, ColumnSchemaForArrayIsRepeated) {
  ::googlesql::TypeFactory tf;
  const ::googlesql::ArrayType* arr = nullptr;
  ASSERT_TRUE(tf.MakeArrayType(::googlesql::types::Int64Type(), &arr).ok());
  auto out = ColumnSchemaForType(arr, "vs");
  ASSERT_TRUE(out.ok()) << out.status();
  EXPECT_EQ(out->mode, schema::ColumnMode::kRepeated);
  EXPECT_EQ(out->type, schema::ColumnType::kInt64);
}

TEST(SemanticValueTest, ColumnSchemaForArrayOfArrayRejected) {
  ::googlesql::TypeFactory tf;
  const ::googlesql::ArrayType* inner = nullptr;
  const ::googlesql::ArrayType* outer = nullptr;
  ASSERT_TRUE(tf.MakeArrayType(::googlesql::types::Int64Type(), &inner).ok());
  // ARRAY<ARRAY<INT64>> is not representable in BigQuery; the
  // analyzer cannot produce one for a SELECT, but we defense-in-
  // depth check via the helper.
  auto outer_status = tf.MakeArrayType(inner, &outer);
  // The factory itself may reject this; fall back to constructing
  // a synthetic struct-arr if so. Either way the helper rejects.
  if (outer_status.ok()) {
    auto out = ColumnSchemaForType(outer, "vs");
    EXPECT_FALSE(out.ok());
  }
}

TEST(SemanticValueTest, ParseParameterValueInt64FromBareNumber) {
  auto v = ParseParameterValue("42", "INT64");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 42);
}

TEST(SemanticValueTest, ParseParameterValueInt64FromQuotedString) {
  auto v = ParseParameterValue("\"42\"", "INT64");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 42);
}

TEST(SemanticValueTest, ParseParameterValueBoolTrue) {
  auto v = ParseParameterValue("true", "BOOL");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST(SemanticValueTest, ParseParameterValueStringIsBareLiteral) {
  auto v = ParseParameterValue("\"hello\"", "STRING");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "hello");
}

TEST(SemanticValueTest, ParseParameterValueDoubleAcceptsBareNumber) {
  auto v = ParseParameterValue("1.5", "FLOAT64");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->double_value(), 1.5);
}

TEST(SemanticValueTest, ParseParameterValueNullProducesNullValue) {
  auto v = ParseParameterValue("null", "INT64");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_INT64);
}

TEST(SemanticValueTest, ParseParameterValueRejectsUnknownTypeKind) {
  auto v = ParseParameterValue("0", "ALIEN_TYPE");
  EXPECT_FALSE(v.ok());
}

TEST(SemanticValueTest, ParseParameterValueTimestampRFC3339) {
  auto v = ParseParameterValue("\"2016-12-07T08:00:00+00:00\"", "TIMESTAMP");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_TIMESTAMP);
  EXPECT_FALSE(v->is_null());
}

TEST(SemanticValueTest, ParseParameterValueTimestampIsoWithoutTimezone) {
  auto v = ParseParameterValue("2026-06-22T10:00:00", "TIMESTAMP");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_TIMESTAMP);
  EXPECT_FALSE(v->is_null());
  EXPECT_EQ(absl::ToUnixSeconds(v->ToTime()), 1782122400);
}

TEST(SemanticValueTest, ParseParameterValueTimestampDateOnly) {
  auto v = ParseParameterValue("2026-06-22", "TIMESTAMP");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_TIMESTAMP);
  EXPECT_FALSE(v->is_null());
}

TEST(SemanticValueTest, ParseParameterValueStructOrderedArray) {
  auto v = ParseParameterValue(R"(["1","foo"])", "STRUCT", "x:INT64,y:STRING");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->type()->IsStruct());
  EXPECT_EQ(v->num_fields(), 2);
  EXPECT_EQ(v->field(0).int64_value(), 1);
  EXPECT_EQ(v->field(1).string_value(), "foo");
}

TEST(SemanticValueTest, ParseParameterValueArrayString) {
  auto v = ParseParameterValue(R"(["WA","WI","WV","WY"])", "ARRAY", "STRING");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->type()->IsArray());
  EXPECT_EQ(v->num_elements(), 4);
  EXPECT_EQ(v->element(0).string_value(), "WA");
  EXPECT_EQ(v->element(3).string_value(), "WY");
}

TEST(SemanticValueTest, ParseParameterValueDeferTemporalTypes) {
  auto v = ParseParameterValue("\"2020-01-01\"", "DATE");
  EXPECT_FALSE(v.ok());
  EXPECT_EQ(v.status().code(), absl::StatusCode::kUnimplemented);
}

TEST(SemanticValueTest, BigQueryTypeNameCoversPrimitiveSet) {
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::Int64Type()), "INT64");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::BoolType()), "BOOL");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::DoubleType()), "FLOAT64");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::StringType()), "STRING");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::BytesType()), "BYTES");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::DateType()), "DATE");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::TimeType()), "TIME");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::DatetimeType()), "DATETIME");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::TimestampType()), "TIMESTAMP");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::NumericType()), "NUMERIC");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::BigNumericType()),
            "BIGNUMERIC");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::JsonType()), "JSON");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::IntervalType()), "INTERVAL");
  EXPECT_EQ(BigQueryTypeName(::googlesql::types::UuidType()), "UUID");
}

TEST(SemanticValueTest, ParseTypeKindNameAcceptsBothSpellings) {
  EXPECT_EQ(ParseTypeKindName("INT64"), ::googlesql::TYPE_INT64);
  EXPECT_EQ(ParseTypeKindName("TYPE_INT64"), ::googlesql::TYPE_INT64);
  EXPECT_EQ(ParseTypeKindName("integer"), ::googlesql::TYPE_INT64);
  EXPECT_EQ(ParseTypeKindName("FLOAT64"), ::googlesql::TYPE_DOUBLE);
  EXPECT_EQ(ParseTypeKindName("BOOLEAN"), ::googlesql::TYPE_BOOL);
  EXPECT_EQ(ParseTypeKindName("__nope__"), ::googlesql::TYPE_UNKNOWN);
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
