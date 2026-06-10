// Unit tests for the string-functions semantic-executor helpers.

#include "backend/engine/semantic/functions/string_funcs.h"

#include <vector>

#include "absl/status/status.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

// Reference SOUNDEX values from
// https://en.wikipedia.org/wiki/Soundex and BigQuery's
// `string_functions` documentation (`SOUNDEX('Ashcraft') ->
// A261`). Holding these stable is the contract of the row in
// `functions.yaml`.
TEST(SoundexTest, ClassicReferenceValues) {
  EXPECT_EQ(Soundex({Value::String("Robert")})->string_value(), "R163");
  EXPECT_EQ(Soundex({Value::String("Rupert")})->string_value(), "R163");
  EXPECT_EQ(Soundex({Value::String("Rubin")})->string_value(), "R150");
  EXPECT_EQ(Soundex({Value::String("Ashcraft")})->string_value(), "A261");
  EXPECT_EQ(Soundex({Value::String("Tymczak")})->string_value(), "T522");
  EXPECT_EQ(Soundex({Value::String("Pfister")})->string_value(), "P236");
  EXPECT_EQ(Soundex({Value::String("Honeyman")})->string_value(), "H555");
}

TEST(SoundexTest, EmptyStringRoundTrips) {
  auto v = Soundex({Value::String("")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "");
}

TEST(SoundexTest, NullPropagates) {
  auto v = Soundex({Value::NullString()});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(SoundexTest, ShortStringPadsWithZeros) {
  EXPECT_EQ(Soundex({Value::String("A")})->string_value(), "A000");
  EXPECT_EQ(Soundex({Value::String("Hi")})->string_value(), "H000");
}

TEST(SoundexTest, NonLetterPrefixSkipped) {
  // Leading punctuation/digits do not anchor SOUNDEX -- the
  // first SOUNDEX-eligible letter does.
  EXPECT_EQ(Soundex({Value::String("123Robert")})->string_value(), "R163");
}

TEST(SoundexTest, AllLettersWithoutCodeReturnsLetterAndZeros) {
  EXPECT_EQ(Soundex({Value::String("Aeiouy")})->string_value(), "A000");
}

TEST(SoundexTest, NonStringRejected) {
  auto v = Soundex({Value::Int64(7)});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(InstrTest, BasicTwoArg) {
  auto v = Instr({Value::String("hello world"), Value::String("world")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 7);
}

TEST(InstrTest, NotFoundReturnsZero) {
  auto v = Instr({Value::String("hello"), Value::String("xyz")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 0);
}

TEST(InstrTest, PositionStartsSearch) {
  auto v =
      Instr({Value::String("ababab"), Value::String("ab"), Value::Int64(2)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST(InstrTest, OccurrencePicksNthMatch) {
  auto v = Instr({Value::String("ababab"),
                  Value::String("ab"),
                  Value::Int64(1),
                  Value::Int64(2)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST(InstrTest, ThirdOccurrence) {
  auto v = Instr({Value::String("ababab"),
                  Value::String("ab"),
                  Value::Int64(1),
                  Value::Int64(3)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 5);
}

TEST(InstrTest, NegativePositionSearchesFromEnd) {
  // BigQuery: INSTR('ababab', 'ab', -1) returns the last
  // occurrence's 1-based position (5).
  auto v =
      Instr({Value::String("ababab"), Value::String("ab"), Value::Int64(-1)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 5);
}

TEST(InstrTest, NegativePositionWithOccurrence) {
  auto v = Instr({Value::String("ababab"),
                  Value::String("ab"),
                  Value::Int64(-1),
                  Value::Int64(2)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST(InstrTest, EmptySubvalueReturnsPosition) {
  auto v = Instr({Value::String("hello"), Value::String("")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 1);
}

TEST(InstrTest, NullPropagates) {
  auto v = Instr({Value::String("hi"), Value::NullString()});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(InstrTest, ZeroPositionRejected) {
  auto v = Instr({Value::String("hi"), Value::String("h"), Value::Int64(0)});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(InstrTest, NonPositiveOccurrenceRejected) {
  auto v = Instr({Value::String("hi"),
                  Value::String("h"),
                  Value::Int64(1),
                  Value::Int64(0)});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(ContainsSubstrTest, CaseInsensitiveStringMatch) {
  auto v = ContainsSubstr(
      {Value::String("the blue house"), Value::String("Blue house")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST(ContainsSubstrTest, UnicodeNormalizationMatch) {
  auto v = ContainsSubstr({Value::String("\u2168"), Value::String("IX")});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST(ContainsSubstrTest, NullSearchValueRejected) {
  auto v = ContainsSubstr({Value::String("hello"), Value::NullString()});
  ASSERT_FALSE(v.ok());
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
