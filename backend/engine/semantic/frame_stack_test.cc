// Unit tests for the shared `semantic::FrameStack` primitive.
//
// `FrameStack` is the foundation shared between the script driver
// (BEGIN..END variables) and the UDF / TVF invocation surface
// (argument bindings). Tests pin the frame semantics callers
// depend on: push/pop, innermost-binding wins, redeclaration in
// the same frame is rejected, redeclaration in a nested frame is
// allowed, and case-insensitive identifier matching.
//
// The script-statement / UDF-statement tests assert the
// `kAlreadyExists` / `kNotFound` error surfaces translate into the
// matching BigQuery REST envelope on top of this primitive's
// `absl::Status` codes.

#include "backend/engine/semantic/frame_stack.h"

#include "absl/status/status.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

TEST(FrameStackTest, NewStackHasOuterFrame) {
  FrameStack stack;
  EXPECT_EQ(stack.frame_count(), 1u);
  EXPECT_FALSE(stack.Has("x"));
}

TEST(FrameStackTest, DeclareAndLookupRoundTrips) {
  FrameStack stack;
  ASSERT_TRUE(stack.Declare("x", Value::Int64(42)).ok());
  ASSERT_TRUE(stack.Has("x"));
  auto got = stack.Lookup("x");
  ASSERT_TRUE(got.ok()) << got.status();
  EXPECT_EQ(got->int64_value(), 42);
}

TEST(FrameStackTest, IdentifierMatchIsCaseInsensitive) {
  FrameStack stack;
  ASSERT_TRUE(stack.Declare("FooBar", Value::Int64(1)).ok());
  EXPECT_TRUE(stack.Has("foobar"));
  EXPECT_TRUE(stack.Has("FOOBAR"));
  auto got = stack.Lookup("foobar");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 1);
  ASSERT_TRUE(stack.Set("FOOBAR", Value::Int64(2)).ok());
  got = stack.Lookup("FooBar");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 2);
}

TEST(FrameStackTest, RedeclareSameFrameRejected) {
  FrameStack stack;
  ASSERT_TRUE(stack.Declare("x", Value::Int64(1)).ok());
  auto status = stack.Declare("x", Value::Int64(2));
  EXPECT_EQ(status.code(), absl::StatusCode::kAlreadyExists) << status;
}

TEST(FrameStackTest, SetWithoutDeclareReturnsNotFound) {
  FrameStack stack;
  auto status = stack.Set("missing", Value::Int64(0));
  EXPECT_EQ(status.code(), absl::StatusCode::kNotFound) << status;
}

TEST(FrameStackTest, LookupMissingReturnsNotFound) {
  FrameStack stack;
  auto got = stack.Lookup("missing");
  ASSERT_FALSE(got.ok());
  EXPECT_EQ(got.status().code(), absl::StatusCode::kNotFound);
}

TEST(FrameStackTest, NestedFrameInnerBindingWins) {
  FrameStack stack;
  ASSERT_TRUE(stack.Declare("x", Value::Int64(1)).ok());
  stack.PushFrame();
  EXPECT_EQ(stack.frame_count(), 2u);
  ASSERT_TRUE(stack.Declare("x", Value::Int64(2)).ok());
  auto got = stack.Lookup("x");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 2);
  ASSERT_TRUE(stack.PopFrame().ok());
  EXPECT_EQ(stack.frame_count(), 1u);
  got = stack.Lookup("x");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 1);
}

TEST(FrameStackTest, SetUpdatesInnermostBinding) {
  FrameStack stack;
  ASSERT_TRUE(stack.Declare("x", Value::Int64(1)).ok());
  stack.PushFrame();
  ASSERT_TRUE(stack.Declare("x", Value::Int64(2)).ok());
  ASSERT_TRUE(stack.Set("x", Value::Int64(20)).ok());
  auto inner = stack.Lookup("x");
  ASSERT_TRUE(inner.ok());
  EXPECT_EQ(inner->int64_value(), 20);
  ASSERT_TRUE(stack.PopFrame().ok());
  // The outer frame's binding is unchanged: `Set` updated only the
  // innermost binding, not every frame's copy.
  auto outer = stack.Lookup("x");
  ASSERT_TRUE(outer.ok());
  EXPECT_EQ(outer->int64_value(), 1);
}

TEST(FrameStackTest, SetReachesOuterFrame) {
  FrameStack stack;
  ASSERT_TRUE(stack.Declare("x", Value::Int64(1)).ok());
  stack.PushFrame();
  // Inner frame has no `x` binding; `Set` walks up and finds the
  // outer frame's binding.
  ASSERT_TRUE(stack.Set("x", Value::Int64(99)).ok());
  ASSERT_TRUE(stack.PopFrame().ok());
  auto got = stack.Lookup("x");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 99);
}

TEST(FrameStackTest, PopOuterFrameRejected) {
  FrameStack stack;
  auto status = stack.PopFrame();
  EXPECT_EQ(status.code(), absl::StatusCode::kFailedPrecondition) << status;
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
