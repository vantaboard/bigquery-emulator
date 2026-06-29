

#include "absl/status/status.h"
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
  Value key_type = Value::String("AEAD_AES_GCM_256");
  auto r = Dispatch("keys.new_keyset", {key_type}, /*return_type=*/nullptr);
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(r->ok()) << r->status();
  EXPECT_EQ(r->value().type_kind(), ::googlesql::TYPE_BYTES);
}

TEST(StubDispatchTest, KeysKeysetLengthIsDispatched) {
  Value keyset = Value::Bytes("anything");
  auto r = Dispatch("keys.keyset_length", {keyset}, /*return_type=*/nullptr);
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
  Value wrong_type = Value::Int64(7);
  auto r = Dispatch("keys.new_keyset", {wrong_type}, /*return_type=*/nullptr);
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
