#include "backend/engine/semantic/stubs/dispatch.h"

#include <optional>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "gmock/gmock.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {
namespace {

using ::testing::HasSubstr;

TEST(StubDispatchTest, KeysNewKeysetIsDispatched) {
  // `keys.new_keyset` is the dotted BigQuery name the route
  // classifier (`route_classifier.cc::CheckFunction`) writes by
  // calling `Function::FullName(/*include_group=*/false)`. The
  // semantic executor (`eval_expr.cc::DispatchFunctionByName`)
  // hands that name verbatim to this Dispatch table. A regression
  // that renames the key on either side breaks the round-trip
  // silently; pinning the key here forces an explicit update.
  auto r = Dispatch("keys.new_keyset",
                    {Value::String("AEAD_AES_GCM_256")},
                    /*return_type=*/nullptr);
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(r->ok()) << r->status();
  EXPECT_EQ(r->value().type_kind(), ::googlesql::TYPE_BYTES);
}

TEST(StubDispatchTest, KeysKeysetLengthIsDispatched) {
  auto r = Dispatch("keys.keyset_length",
                    {Value::Bytes("anything")},
                    /*return_type=*/nullptr);
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(r->ok()) << r->status();
  EXPECT_EQ(r->value().type_kind(), ::googlesql::TYPE_INT64);
  EXPECT_EQ(r->value().int64_value(), 1);
}

TEST(StubDispatchTest, UnknownNameReturnsNullopt) {
  auto r = Dispatch("not.a.stub.family", {}, /*return_type=*/nullptr);
  EXPECT_FALSE(r.has_value());
}

TEST(StubDispatchTest, InvalidArgsBubbleUpAsStatus) {
  // When the stub fires but the caller passed the wrong type, the
  // dispatch returns `optional<StatusOr<Value>>` with a non-OK
  // status. The caller treats it as "stub fired, evaluation
  // failed" and surfaces the status verbatim -- it does NOT fall
  // through to its own NOT_IMPLEMENTED branch (which would mask a
  // real argument-shape failure as a missing-function error).
  auto r = Dispatch("keys.new_keyset",
                    {Value::Int64(7)},
                    /*return_type=*/nullptr);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(std::string(r->status().message()), HasSubstr("STRING"));
}

}  // namespace
}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
