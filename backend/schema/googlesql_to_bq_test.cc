// Unit tests for the googlesql::Type -> proto FieldSchema reflector.
//
// We exercise the mapping by constructing GoogleSQL types via a
// `TypeFactory` (the same path the analyzer uses) and asserting on
// the resulting `FieldSchema` proto. Tests cover every BigQuery
// scalar, ARRAY/STRUCT containers, and the error cases for kinds we
// do not yet model.

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace schema {
namespace {

using ::googlesql::StructType;
using ::googlesql::Type;
using ::googlesql::TypeFactory;

class TypeReflectionTest : public ::testing::Test {
 protected:
  TypeFactory factory_{};
};

TEST_F(TypeReflectionTest, ScalarsRoundTripToBigQueryNames) {
  struct Case {
    const Type* type = nullptr;
    absl::string_view expected;
  };
  const Case cases[] = {
      {factory_.get_bool(), "BOOL"},
      {factory_.get_int64(), "INT64"},
      {factory_.get_double(), "FLOAT64"},
      {factory_.get_string(), "STRING"},
      {factory_.get_bytes(), "BYTES"},
      {factory_.get_date(), "DATE"},
      {factory_.get_time(), "TIME"},
      {factory_.get_datetime(), "DATETIME"},
      {factory_.get_timestamp(), "TIMESTAMP"},
      {factory_.get_numeric(), "NUMERIC"},
      {factory_.get_bignumeric(), "BIGNUMERIC"},
      {factory_.get_json(), "JSON"},
      {factory_.get_geography(), "GEOGRAPHY"},
  };
  for (const Case& c : cases) {
    v1::FieldSchema out;
    absl::Status status = TypeToFieldSchema(c.type, "col", &out);
    ASSERT_TRUE(status.ok()) << status.message();
    EXPECT_EQ(out.name(), "col");
    EXPECT_EQ(out.type(), c.expected);
    EXPECT_EQ(out.mode(), "");
    EXPECT_EQ(out.fields_size(), 0);
  }
}

TEST_F(TypeReflectionTest, ArrayOfScalarBecomesRepeated) {
  const Type* array_type = nullptr;
  ASSERT_TRUE(factory_.MakeArrayType(factory_.get_string(), &array_type).ok());
  v1::FieldSchema out;
  ASSERT_TRUE(TypeToFieldSchema(array_type, "tags", &out).ok());
  EXPECT_EQ(out.name(), "tags");
  EXPECT_EQ(out.type(), "STRING");
  EXPECT_EQ(out.mode(), "REPEATED");
}

TEST_F(TypeReflectionTest, NestedStructRecursesIntoFields) {
  std::vector<StructType::StructField> inner_fields = {
      {"city", factory_.get_string()},
      {"zip", factory_.get_int64()},
  };
  const StructType* inner = nullptr;
  ASSERT_TRUE(factory_.MakeStructType(inner_fields, &inner).ok());
  std::vector<StructType::StructField> outer_fields = {
      {"name", factory_.get_string()},
      {"address", inner},
  };
  const StructType* outer = nullptr;
  ASSERT_TRUE(factory_.MakeStructType(outer_fields, &outer).ok());

  v1::FieldSchema out;
  ASSERT_TRUE(TypeToFieldSchema(outer, "person", &out).ok());
  EXPECT_EQ(out.name(), "person");
  EXPECT_EQ(out.type(), "STRUCT");
  ASSERT_EQ(out.fields_size(), 2);
  EXPECT_EQ(out.fields(0).name(), "name");
  EXPECT_EQ(out.fields(0).type(), "STRING");
  EXPECT_EQ(out.fields(1).name(), "address");
  EXPECT_EQ(out.fields(1).type(), "STRUCT");
  ASSERT_EQ(out.fields(1).fields_size(), 2);
  EXPECT_EQ(out.fields(1).fields(0).name(), "city");
  EXPECT_EQ(out.fields(1).fields(0).type(), "STRING");
  EXPECT_EQ(out.fields(1).fields(1).name(), "zip");
  EXPECT_EQ(out.fields(1).fields(1).type(), "INT64");
}

TEST_F(TypeReflectionTest, ArrayOfStructPropagatesNestedFields) {
  std::vector<StructType::StructField> fields = {
      {"id", factory_.get_int64()},
      {"label", factory_.get_string()},
  };
  const StructType* st = nullptr;
  ASSERT_TRUE(factory_.MakeStructType(fields, &st).ok());
  const Type* arr = nullptr;
  ASSERT_TRUE(factory_.MakeArrayType(st, &arr).ok());

  v1::FieldSchema out;
  ASSERT_TRUE(TypeToFieldSchema(arr, "items", &out).ok());
  EXPECT_EQ(out.name(), "items");
  EXPECT_EQ(out.type(), "STRUCT");
  EXPECT_EQ(out.mode(), "REPEATED");
  ASSERT_EQ(out.fields_size(), 2);
  EXPECT_EQ(out.fields(0).type(), "INT64");
  EXPECT_EQ(out.fields(1).type(), "STRING");
}

TEST_F(TypeReflectionTest, StructFieldWithRepeatedArraySetsMode) {
  const Type* arr = nullptr;
  ASSERT_TRUE(factory_.MakeArrayType(factory_.get_string(), &arr).ok());
  std::vector<StructType::StructField> fields = {
      {"tags", arr},
      {"count", factory_.get_int64()},
  };
  const StructType* st = nullptr;
  ASSERT_TRUE(factory_.MakeStructType(fields, &st).ok());

  v1::FieldSchema out;
  ASSERT_TRUE(TypeToFieldSchema(st, "rec", &out).ok());
  ASSERT_EQ(out.fields_size(), 2);
  EXPECT_EQ(out.fields(0).name(), "tags");
  EXPECT_EQ(out.fields(0).type(), "STRING");
  EXPECT_EQ(out.fields(0).mode(), "REPEATED");
  EXPECT_EQ(out.fields(1).name(), "count");
  EXPECT_EQ(out.fields(1).type(), "INT64");
  EXPECT_EQ(out.fields(1).mode(), "");
}

TEST_F(TypeReflectionTest, ArrayOfArrayIsRejected) {
  const Type* inner = nullptr;
  ASSERT_TRUE(factory_.MakeArrayType(factory_.get_int64(), &inner).ok());
  const Type* outer = nullptr;
  // GoogleSQL's MakeArrayType refuses ARRAY<ARRAY<...>> with an
  // InvalidArgument; we mirror that signal here. If a future
  // GoogleSQL release ever loosens the restriction, this test will
  // fail and we want to keep the BigQuery side rejecting it
  // explicitly anyway.
  absl::Status make = factory_.MakeArrayType(inner, &outer);
  if (make.ok()) {
    v1::FieldSchema out;
    EXPECT_EQ(TypeToFieldSchema(outer, "bad", &out).code(),
              absl::StatusCode::kInvalidArgument);
  } else {
    EXPECT_EQ(make.code(), absl::StatusCode::kInvalidArgument);
  }
}

TEST_F(TypeReflectionTest, NullTypeAndNullOutputAreInvalid) {
  v1::FieldSchema out;
  EXPECT_EQ(TypeToFieldSchema(nullptr, "x", &out).code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(TypeToFieldSchema(factory_.get_int64(), "x", nullptr).code(),
            absl::StatusCode::kInvalidArgument);
}

}  // namespace
}  // namespace schema
}  // namespace backend
}  // namespace bigquery_emulator
