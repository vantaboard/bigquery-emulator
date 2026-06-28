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
  Value robert = Value::String("Robert");
  EXPECT_EQ(Soundex({robert})->string_value(), "R163");
  Value rupert = Value::String("Rupert");
  EXPECT_EQ(Soundex({rupert})->string_value(), "R163");
  Value rubin = Value::String("Rubin");
  EXPECT_EQ(Soundex({rubin})->string_value(), "R150");
  Value ashcraft = Value::String("Ashcraft");
  EXPECT_EQ(Soundex({ashcraft})->string_value(), "A261");
  Value tymczak = Value::String("Tymczak");
  EXPECT_EQ(Soundex({tymczak})->string_value(), "T522");
  Value pfister = Value::String("Pfister");
  EXPECT_EQ(Soundex({pfister})->string_value(), "P236");
  Value honeman = Value::String("Honeyman");
  EXPECT_EQ(Soundex({honeman})->string_value(), "H555");
}

TEST(SoundexTest, EmptyStringRoundTrips) {
  Value arg = Value::String("");
  auto v = Soundex({arg});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "");
}

TEST(SoundexTest, NullPropagates) {
  Value arg = Value::NullString();
  auto v = Soundex({arg});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(SoundexTest, ShortStringPadsWithZeros) {
  Value a = Value::String("A");
  EXPECT_EQ(Soundex({a})->string_value(), "A000");
  Value hi = Value::String("Hi");
  EXPECT_EQ(Soundex({hi})->string_value(), "H000");
}

TEST(SoundexTest, NonLetterPrefixSkipped) {
  // Leading punctuation/digits do not anchor SOUNDEX -- the
  // first SOUNDEX-eligible letter does.
  Value arg = Value::String("123Robert");
  EXPECT_EQ(Soundex({arg})->string_value(), "R163");
}

TEST(SoundexTest, AllLettersWithoutCodeReturnsLetterAndZeros) {
  Value arg = Value::String("Aeiouy");
  EXPECT_EQ(Soundex({arg})->string_value(), "A000");
}

TEST(SoundexTest, NonStringRejected) {
  Value arg = Value::Int64(7);
  auto v = Soundex({arg});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(InstrTest, BasicTwoArg) {
  Value haystack = Value::String("hello world");
  Value needle = Value::String("world");
  auto v = Instr({haystack, needle});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 7);
}

TEST(InstrTest, NotFoundReturnsZero) {
  Value haystack = Value::String("hello");
  Value needle = Value::String("xyz");
  auto v = Instr({haystack, needle});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 0);
}

TEST(InstrTest, PositionStartsSearch) {
  Value haystack = Value::String("ababab");
  Value needle = Value::String("ab");
  Value position = Value::Int64(2);
  auto v = Instr({haystack, needle, position});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST(InstrTest, OccurrencePicksNthMatch) {
  Value haystack = Value::String("ababab");
  Value needle = Value::String("ab");
  Value position = Value::Int64(1);
  Value occurrence = Value::Int64(2);
  auto v = Instr({haystack, needle, position, occurrence});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST(InstrTest, ThirdOccurrence) {
  Value haystack = Value::String("ababab");
  Value needle = Value::String("ab");
  Value position = Value::Int64(1);
  Value occurrence = Value::Int64(3);
  auto v = Instr({haystack, needle, position, occurrence});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 5);
}

TEST(InstrTest, NegativePositionSearchesFromEnd) {
  // BigQuery: INSTR('ababab', 'ab', -1) returns the last
  // occurrence's 1-based position (5).
  Value haystack = Value::String("ababab");
  Value needle = Value::String("ab");
  Value position = Value::Int64(-1);
  auto v = Instr({haystack, needle, position});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 5);
}

TEST(InstrTest, NegativePositionWithOccurrence) {
  Value haystack = Value::String("ababab");
  Value needle = Value::String("ab");
  Value position = Value::Int64(-1);
  Value occurrence = Value::Int64(2);
  auto v = Instr({haystack, needle, position, occurrence});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST(InstrTest, EmptySubvalueReturnsPosition) {
  Value haystack = Value::String("hello");
  Value needle = Value::String("");
  auto v = Instr({haystack, needle});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 1);
}

TEST(InstrTest, NullPropagates) {
  Value haystack = Value::String("hi");
  Value needle = Value::NullString();
  auto v = Instr({haystack, needle});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(InstrTest, ZeroPositionRejected) {
  Value haystack = Value::String("hi");
  Value needle = Value::String("h");
  Value position = Value::Int64(0);
  auto v = Instr({haystack, needle, position});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(InstrTest, NonPositiveOccurrenceRejected) {
  Value haystack = Value::String("hi");
  Value needle = Value::String("h");
  Value position = Value::Int64(1);
  Value occurrence = Value::Int64(0);
  auto v = Instr({haystack, needle, position, occurrence});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(ContainsSubstrTest, CaseInsensitiveStringMatch) {
  Value haystack = Value::String("the blue house");
  Value needle = Value::String("Blue house");
  auto v = ContainsSubstr({haystack, needle});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST(ContainsSubstrTest, UnicodeNormalizationMatch) {
  Value haystack = Value::String("\u2168");
  Value needle = Value::String("IX");
  auto v = ContainsSubstr({haystack, needle});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST(ContainsSubstrTest, NullSearchValueRejected) {
  Value haystack = Value::String("hello");
  Value needle = Value::NullString();
  auto v = ContainsSubstr({haystack, needle});
  ASSERT_FALSE(v.ok());
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
