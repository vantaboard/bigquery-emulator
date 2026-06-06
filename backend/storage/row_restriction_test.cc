// Unit tests for the `<column> = <literal>` parser that backs the
// Storage Read API's `ReadOptions.row_restriction` field.
//
// Plan 39 ships exactly three literal kinds (INT64 / BOOL / STRING)
// against top-level scalar columns. Every other shape — connectives,
// relational ops, range, IN, NULL, ARRAY / STRUCT columns, FLOAT64 /
// NUMERIC / DATE literals — is rejected at parse time with
// INVALID_ARGUMENT so the gateway can surface the BigQuery REST 400
// envelope before any rows are read.

#include "backend/storage/row_restriction.h"

#include <string>

#include "absl/status/status.h"
#include "backend/schema/schema.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace {

// Four-column toy schema covering all three supported literal kinds
// plus an array column so the rejection paths have something to
// point at.
schema::TableSchema PeopleSchema() {
  schema::TableSchema s;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kInt64;
  id.mode = schema::ColumnMode::kRequired;
  schema::ColumnSchema name;
  name.name = "name";
  name.type = schema::ColumnType::kString;
  name.mode = schema::ColumnMode::kNullable;
  schema::ColumnSchema active;
  active.name = "active";
  active.type = schema::ColumnType::kBool;
  active.mode = schema::ColumnMode::kNullable;
  schema::ColumnSchema tags;
  tags.name = "tags";
  tags.type = schema::ColumnType::kString;
  tags.mode = schema::ColumnMode::kRepeated;
  s.columns = {id, name, active, tags};
  return s;
}

TEST(ParseRowRestriction, EmptyRestrictionLeavesOutUntouched) {
  EqualityPredicate pred;
  pred.column = "should-stay-empty-after-parse";
  ASSERT_TRUE(ParseRowRestriction("", PeopleSchema(), &pred).ok());
  ASSERT_TRUE(ParseRowRestriction("   \t  ", PeopleSchema(), &pred).ok());
  // Sentinel value untouched; caller treats this as "no predicate".
  EXPECT_EQ(pred.column, "should-stay-empty-after-parse");
}

TEST(ParseRowRestriction, ParsesInt64Equality) {
  EqualityPredicate pred;
  ASSERT_TRUE(ParseRowRestriction("id = 42", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.column, "id");
  EXPECT_EQ(pred.column_index, 0u);
  EXPECT_EQ(pred.kind, EqualityPredicate::Kind::kInt64);
  EXPECT_EQ(pred.int64_value, 42);
}

TEST(ParseRowRestriction, ParsesNegativeInt64Equality) {
  EqualityPredicate pred;
  ASSERT_TRUE(ParseRowRestriction("id = -7", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.int64_value, -7);
}

TEST(ParseRowRestriction, ParsesBoolEqualityTrue) {
  EqualityPredicate pred;
  ASSERT_TRUE(ParseRowRestriction("active = true", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.kind, EqualityPredicate::Kind::kBool);
  EXPECT_TRUE(pred.bool_value);
}

TEST(ParseRowRestriction, ParsesBoolEqualityFalseCaseInsensitive) {
  EqualityPredicate pred;
  ASSERT_TRUE(
      ParseRowRestriction("active = FALSE", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.kind, EqualityPredicate::Kind::kBool);
  EXPECT_FALSE(pred.bool_value);
}

TEST(ParseRowRestriction, ParsesStringEquality) {
  EqualityPredicate pred;
  ASSERT_TRUE(ParseRowRestriction("name = 'ada'", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.kind, EqualityPredicate::Kind::kString);
  EXPECT_EQ(pred.column_index, 1u);
  EXPECT_EQ(pred.string_value, "ada");
}

TEST(ParseRowRestriction, ParsesStringEqualityWithEscapedQuote) {
  EqualityPredicate pred;
  ASSERT_TRUE(
      ParseRowRestriction("name = 'O''Reilly'", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.string_value, "O'Reilly");
}

TEST(ParseRowRestriction, ParsesDoubleQuotedStringEquality) {
  schema::TableSchema s;
  schema::ColumnSchema state;
  state.name = "state";
  state.type = schema::ColumnType::kString;
  state.mode = schema::ColumnMode::kNullable;
  s.columns = {state};

  EqualityPredicate pred;
  ASSERT_TRUE(ParseRowRestriction(R"(state = "WA")", s, &pred).ok());
  EXPECT_EQ(pred.column, "state");
  EXPECT_EQ(pred.kind, EqualityPredicate::Kind::kString);
  EXPECT_EQ(pred.string_value, "WA");
}

TEST(ParseRowRestriction, ParsesBacktickQuotedColumn) {
  EqualityPredicate pred;
  ASSERT_TRUE(ParseRowRestriction("`id` = 42", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.column, "id");
  EXPECT_EQ(pred.int64_value, 42);
}

TEST(ParseRowRestriction, IgnoresSurroundingWhitespace) {
  EqualityPredicate pred;
  ASSERT_TRUE(
      ParseRowRestriction("   id   =   42   ", PeopleSchema(), &pred).ok());
  EXPECT_EQ(pred.column, "id");
  EXPECT_EQ(pred.int64_value, 42);
}

// ---------------------------------------------------------------------------
// Rejection paths
// ---------------------------------------------------------------------------

TEST(ParseRowRestriction, RejectsAndConnective) {
  EqualityPredicate pred;
  auto s =
      ParseRowRestriction("id = 1 AND name = 'ada'", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsOrConnective) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id = 1 OR id = 2", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsRangeOperator) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id > 1", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsInequalityOperator) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id != 1", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsInList) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id IN (1, 2)", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsIsNull) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("name IS NULL", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsUnknownColumn) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("nope = 1", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsRepeatedColumn) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("tags = 'kernel'", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsMissingLiteral) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id =", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsMissingEquals) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id 42", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsFloat64Literal) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id = 1.5", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsBoolLiteralOnInt64Column) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id = true", PeopleSchema(), &pred);
  // Lit is "true" which fails INT64 parse — caller sees
  // INVALID_ARGUMENT with the message naming the failure.
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsStringLiteralOnInt64Column) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("id = '42'", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsIntLiteralOnStringColumn) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("name = 42", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

TEST(ParseRowRestriction, RejectsUnterminatedString) {
  EqualityPredicate pred;
  auto s = ParseRowRestriction("name = 'ada", PeopleSchema(), &pred);
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kInvalidArgument);
}

}  // namespace
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
