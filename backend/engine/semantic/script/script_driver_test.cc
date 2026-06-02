// Unit tests for the script-level variable environment.
//
// The variable environment is the foundation Family-1 surface for
// BigQuery scripting. Tests pin the BEGIN..END frame semantics
// (push/pop, innermost-binding wins, redeclaration in the same
// frame is rejected, redeclaration in a nested frame is allowed),
// case-insensitive identifier matching (BigQuery scripting
// reference), and the structured `kAlreadyExists` /
// `kNotFound` error surfaces the higher-level statement handlers
// (`DECLARE`, `SET`) translate into BigQuery's REST envelope.

#include "backend/engine/semantic/script/script_driver.h"

#include "absl/status/status.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {
namespace {

TEST(VariableEnvironmentTest, NewEnvHasGlobalFrame) {
  VariableEnvironment env;
  EXPECT_EQ(env.frame_count(), 1u);
  EXPECT_FALSE(env.Has("x"));
}

TEST(VariableEnvironmentTest, DeclareAndLookupRoundTrips) {
  VariableEnvironment env;
  ASSERT_TRUE(env.Declare("x", Value::Int64(42)).ok());
  ASSERT_TRUE(env.Has("x"));
  auto got = env.Lookup("x");
  ASSERT_TRUE(got.ok()) << got.status();
  EXPECT_EQ(got->int64_value(), 42);
}

TEST(VariableEnvironmentTest, IdentifierMatchIsCaseInsensitive) {
  VariableEnvironment env;
  ASSERT_TRUE(env.Declare("FooBar", Value::Int64(1)).ok());
  EXPECT_TRUE(env.Has("foobar"));
  EXPECT_TRUE(env.Has("FOOBAR"));
  auto got = env.Lookup("foobar");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 1);
  ASSERT_TRUE(env.Set("FOOBAR", Value::Int64(2)).ok());
  got = env.Lookup("FooBar");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 2);
}

TEST(VariableEnvironmentTest, RedeclareSameFrameRejected) {
  VariableEnvironment env;
  ASSERT_TRUE(env.Declare("x", Value::Int64(1)).ok());
  auto status = env.Declare("x", Value::Int64(2));
  EXPECT_EQ(status.code(), absl::StatusCode::kAlreadyExists) << status;
}

TEST(VariableEnvironmentTest, SetWithoutDeclareReturnsNotFound) {
  VariableEnvironment env;
  auto status = env.Set("missing", Value::Int64(0));
  EXPECT_EQ(status.code(), absl::StatusCode::kNotFound) << status;
}

TEST(VariableEnvironmentTest, LookupMissingReturnsNotFound) {
  VariableEnvironment env;
  auto got = env.Lookup("missing");
  ASSERT_FALSE(got.ok());
  EXPECT_EQ(got.status().code(), absl::StatusCode::kNotFound);
}

TEST(VariableEnvironmentTest, NestedFrameInnerBindingWins) {
  VariableEnvironment env;
  ASSERT_TRUE(env.Declare("x", Value::Int64(1)).ok());
  env.PushFrame();
  EXPECT_EQ(env.frame_count(), 2u);
  ASSERT_TRUE(env.Declare("x", Value::Int64(2)).ok());
  auto got = env.Lookup("x");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 2);
  ASSERT_TRUE(env.PopFrame().ok());
  EXPECT_EQ(env.frame_count(), 1u);
  got = env.Lookup("x");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 1);
}

TEST(VariableEnvironmentTest, SetUpdatesInnermostBinding) {
  VariableEnvironment env;
  ASSERT_TRUE(env.Declare("x", Value::Int64(1)).ok());
  env.PushFrame();
  ASSERT_TRUE(env.Declare("x", Value::Int64(2)).ok());
  ASSERT_TRUE(env.Set("x", Value::Int64(20)).ok());
  auto inner = env.Lookup("x");
  ASSERT_TRUE(inner.ok());
  EXPECT_EQ(inner->int64_value(), 20);
  ASSERT_TRUE(env.PopFrame().ok());
  // The outer frame's binding is unchanged: `Set` updated only the
  // innermost binding, not every frame's copy.
  auto outer = env.Lookup("x");
  ASSERT_TRUE(outer.ok());
  EXPECT_EQ(outer->int64_value(), 1);
}

TEST(VariableEnvironmentTest, SetReachesOuterFrame) {
  VariableEnvironment env;
  ASSERT_TRUE(env.Declare("x", Value::Int64(1)).ok());
  env.PushFrame();
  // Inner frame has no `x` binding; `Set` walks up and finds the
  // outer frame's binding.
  ASSERT_TRUE(env.Set("x", Value::Int64(99)).ok());
  ASSERT_TRUE(env.PopFrame().ok());
  auto got = env.Lookup("x");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 99);
}

TEST(VariableEnvironmentTest, PopGlobalFrameRejected) {
  VariableEnvironment env;
  auto status = env.PopFrame();
  EXPECT_EQ(status.code(), absl::StatusCode::kFailedPrecondition) << status;
}

TEST(ScriptDriverTest, OwnsAVariableEnvironment) {
  ScriptDriver driver;
  EXPECT_EQ(driver.variables().frame_count(), 1u);
  ASSERT_TRUE(driver.variables().Declare("x", Value::Int64(7)).ok());
  auto got = driver.variables().Lookup("x");
  ASSERT_TRUE(got.ok());
  EXPECT_EQ(got->int64_value(), 7);
}

}  // namespace
}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
