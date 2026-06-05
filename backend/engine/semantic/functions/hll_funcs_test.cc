#include "backend/engine/semantic/functions/hll_funcs.h"

#include <vector>

#include "backend/engine/semantic/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

std::string BrSketch() {
  return std::string("\x12\xef\x7f\x3f\xad\x55\xad\x18\x24\x06\xb9", 11);
}

std::string CzSketch() {
  return std::string("\x12\xef\x7f\x4e\x58\x83\x8d\xb9\xa1\x54\x40", 11);
}

std::string UaSketch() {
  return std::string(
      "\x12\xef\x7f\x3a\x5f\x10\xe7\xef\xa3\x56\x33\x4e\x58\x83\x8d\xb9\xa1"
      "\x54\x40",
      19);
}

std::string MergedSketch() {
  return std::string(
      "\x12\xef\x7f\x3a\x5f\x10\xe7\xef\xa3\x56\x33\x3f\xad\x55\xad\x18\x24"
      "\x06\xb9\x4e\x58\x83\x8d\xb9\xa1\x54\x40",
      27);
}

TEST(HllFuncsTest, InitMatchesExpectedSketchVectors) {
  auto br = HllCountInitValues({Value::String("customer_id_3")}, /*precision=*/10);
  ASSERT_TRUE(br.ok()) << br.status();
  EXPECT_EQ(br->bytes_value(), BrSketch());

  auto cz = HllCountInitValues({Value::String("customer_id_2")}, /*precision=*/10);
  ASSERT_TRUE(cz.ok()) << cz.status();
  EXPECT_EQ(cz->bytes_value(), CzSketch());

  auto ua = HllCountInitValues(
      {Value::String("customer_id_1"), Value::String("customer_id_2")},
      /*precision=*/10);
  ASSERT_TRUE(ua.ok()) << ua.status();
  EXPECT_EQ(ua->bytes_value(), UaSketch());
}

TEST(HllFuncsTest, MergeAndExtractMatchExpectedVectors) {
  auto merge = HllCountMergeAggregate(
      {{Value::Bytes(UaSketch()), Value::Bytes(BrSketch()), Value::Bytes(CzSketch())}});
  ASSERT_TRUE(merge.ok()) << merge.status();
  EXPECT_EQ(merge->int64_value(), 3);

  auto merge_partial = HllCountMergePartialAggregate(
      {{Value::Bytes(UaSketch()), Value::Bytes(BrSketch()), Value::Bytes(CzSketch())}});
  ASSERT_TRUE(merge_partial.ok()) << merge_partial.status();
  EXPECT_EQ(merge_partial->bytes_value(), MergedSketch());

  auto extract_br = HllCountExtractScalar({Value::Bytes(BrSketch())});
  ASSERT_TRUE(extract_br.ok()) << extract_br.status();
  EXPECT_EQ(extract_br->int64_value(), 1);

  auto extract_cz = HllCountExtractScalar({Value::Bytes(CzSketch())});
  ASSERT_TRUE(extract_cz.ok()) << extract_cz.status();
  EXPECT_EQ(extract_cz->int64_value(), 1);

  auto extract_ua = HllCountExtractScalar({Value::Bytes(UaSketch())});
  ASSERT_TRUE(extract_ua.ok()) << extract_ua.status();
  EXPECT_EQ(extract_ua->int64_value(), 2);
}

TEST(HllFuncsTest, ExtractNullReturnsZero) {
  auto got = HllCountExtractScalar({Value::NullBytes()});
  ASSERT_TRUE(got.ok()) << got.status();
  EXPECT_EQ(got->int64_value(), 0);
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
