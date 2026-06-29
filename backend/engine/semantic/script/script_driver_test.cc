// Unit tests for the script-level driver scaffold.
//
// `ScriptDriver` owns the per-script-run state the procedural
// scripting executor threads through every statement family. Today
// the driver only owns the shared `semantic::FrameStack` for the
// script-level variable environment; the primitive itself is
// covered by `backend/engine/semantic/frame_stack_test.cc` so this
// file just pins the driver-side wiring (the script driver owns a
// usable frame stack, callers reach the variables through
// `variables()`).

#include "backend/engine/semantic/script/script_driver.h"

#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {
namespace {

TEST(ScriptDriverTest, OwnsAFrameStack) {
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
