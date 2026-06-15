#include "backend/engine/semantic/stubs/keys.h"

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
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

TEST(KeysNewKeysetTest, ReturnsSentinelBytesForStringArg) {
  // The contract pins the sentinel byte-for-byte so client-library
  // probes can assert on `KEYS.NEW_KEYSET(...)`'s exact return
  // (e.g. checking the leading `bigquery-emulator:keyset:v1:`
  // marker before round-tripping through any downstream consumer).
  // The trailing segment echoes the requested key type so the
  // sentinel still carries the salient input parameter -- BigQuery
  // documents that the returned BYTES depend on the key type, and
  // the placeholder preserves that observable difference.
  auto r = KeysNewKeyset({Value::String("AEAD_AES_GCM_256")});
  ASSERT_TRUE(r.ok()) << r.status();
  EXPECT_EQ(r->type_kind(), ::googlesql::TYPE_BYTES);
  EXPECT_EQ(r->bytes_value(), "bigquery-emulator:keyset:v1:AEAD_AES_GCM_256");

  auto other =
      KeysNewKeyset({Value::String("DETERMINISTIC_AEAD_AES_SIV_CMAC_256")});
  ASSERT_TRUE(other.ok()) << other.status();
  EXPECT_EQ(other->bytes_value(),
            "bigquery-emulator:keyset:v1:DETERMINISTIC_AEAD_AES_SIV_CMAC_256");
  // The two BigQuery-distinct inputs surface as two distinct BYTES
  // outputs. A future regression that hard-codes a single byte
  // string for every input would silently approximate the
  // BigQuery contract; this assertion catches it.
  EXPECT_NE(r->bytes_value(), other->bytes_value());
}

TEST(KeysNewKeysetTest, NullStringPropagatesToNullBytes) {
  // Standard BigQuery scalar contract: NULL input -> NULL output of
  // the documented return type. The semantic executor's SAFE-mode
  // unwrap relies on the function itself producing a typed NULL
  // (NULL BYTES) rather than `Value::Null()` so the surrounding
  // projection's column-type tag remains BYTES.
  auto r = KeysNewKeyset({Value::NullString()});
  ASSERT_TRUE(r.ok()) << r.status();
  EXPECT_TRUE(r->is_null());
  EXPECT_EQ(r->type_kind(), ::googlesql::TYPE_BYTES);
}

TEST(KeysNewKeysetTest, RejectsWrongArity) {
  // 0 or 2+ arguments is a caller bug. The analyzer should reject
  // it before the dispatch even fires; this is defense-in-depth so
  // a downstream caller invoking the helper directly gets a clean
  // INVALID_ARGUMENT instead of an out-of-bounds access.
  auto zero = KeysNewKeyset({});
  EXPECT_EQ(zero.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(std::string(zero.status().message()),
              HasSubstr("KEYS.NEW_KEYSET"));

  auto two = KeysNewKeyset({Value::String("a"), Value::String("b")});
  EXPECT_EQ(two.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(KeysNewKeysetTest, RejectsNonStringArg) {
  // Wrong type also surfaces INVALID_ARGUMENT. The analyzer
  // normally inserts an implicit cast from the literal kind to the
  // declared signature kind; if this branch fires the caller has
  // bypassed the analyzer (or hit a future overload the stub does
  // not yet model) and we surface a clean failure rather than
  // emitting a sentinel keyed on an INT64.
  auto r = KeysNewKeyset({Value::Int64(7)});
  EXPECT_EQ(r.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(std::string(r.status().message()), HasSubstr("STRING"));
}

TEST(KeysKeysetLengthTest, ReturnsOneForAnyNonNullBytes) {
  // BigQuery's `NEW_KEYSET` always returns a single-key keyset;
  // the stub pins `KEYSET_LENGTH` to the matching answer. The
  // value is identical for the sentinel BYTES the emulator emits
  // and any other BYTES the caller might pass -- the local-stub
  // posture is "accept-and-return-shaped-answer", not "validate
  // the keyset envelope".
  auto from_sentinel = KeysKeysetLength(
      {Value::Bytes("bigquery-emulator:keyset:v1:AEAD_AES_GCM_256")});
  ASSERT_TRUE(from_sentinel.ok()) << from_sentinel.status();
  EXPECT_EQ(from_sentinel->type_kind(), ::googlesql::TYPE_INT64);
  EXPECT_EQ(from_sentinel->int64_value(), 1);

  auto from_arbitrary = KeysKeysetLength({Value::Bytes("\x01\x02\x03")});
  ASSERT_TRUE(from_arbitrary.ok()) << from_arbitrary.status();
  EXPECT_EQ(from_arbitrary->int64_value(), 1);

  auto from_empty = KeysKeysetLength({Value::Bytes("")});
  ASSERT_TRUE(from_empty.ok()) << from_empty.status();
  EXPECT_EQ(from_empty->int64_value(), 1);
}

TEST(KeysKeysetLengthTest, NullBytesPropagatesToNullInt64) {
  // Same NULL-propagation contract as the NEW_KEYSET stub. NULL ->
  // NULL INT64 so the projection column type tag stays INT64.
  auto r = KeysKeysetLength({Value::NullBytes()});
  ASSERT_TRUE(r.ok()) << r.status();
  EXPECT_TRUE(r->is_null());
  EXPECT_EQ(r->type_kind(), ::googlesql::TYPE_INT64);
}

TEST(KeysKeysetLengthTest, RejectsWrongArity) {
  auto zero = KeysKeysetLength({});
  EXPECT_EQ(zero.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(std::string(zero.status().message()),
              HasSubstr("KEYS.KEYSET_LENGTH"));

  auto two = KeysKeysetLength({Value::Bytes("a"), Value::Bytes("b")});
  EXPECT_EQ(two.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(KeysKeysetLengthTest, RejectsNonBytesArg) {
  // Wrong type -> INVALID_ARGUMENT. Same reasoning as
  // `RejectsNonStringArg` on the NEW_KEYSET stub.
  auto r = KeysKeysetLength({Value::String("not-bytes")});
  EXPECT_EQ(r.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(std::string(r.status().message()), HasSubstr("BYTES"));
}

TEST(KeysEncryptDecryptTest, RoundTripPlaintextThroughCipherPrefix) {
  auto keyset = KeysNewKeyset({Value::String("AEAD_AES_GCM_256")});
  ASSERT_TRUE(keyset.ok()) << keyset.status();
  auto enc = KeysEncrypt({*keyset, Value::Bytes("abc")});
  ASSERT_TRUE(enc.ok()) << enc.status();
  EXPECT_TRUE(
      absl::StartsWith(enc->bytes_value(), "bigquery-emulator:cipher:v1:"));
  auto dec = KeysDecryptBytes({*keyset, *enc});
  ASSERT_TRUE(dec.ok()) << dec.status();
  EXPECT_EQ(dec->bytes_value(), "abc");
}

TEST(KeysEncryptDecryptTest, NullArgsPropagateToNullBytes) {
  auto keyset = KeysNewKeyset({Value::String("AEAD_AES_GCM_256")});
  ASSERT_TRUE(keyset.ok()) << keyset.status();
  auto enc_null = KeysEncrypt({Value::NullBytes(), Value::Bytes("x")});
  ASSERT_TRUE(enc_null.ok()) << enc_null.status();
  EXPECT_TRUE(enc_null->is_null());
  auto dec_null = KeysDecryptBytes({*keyset, Value::NullBytes()});
  ASSERT_TRUE(dec_null.ok()) << dec_null.status();
  EXPECT_TRUE(dec_null->is_null());
}

}  // namespace
}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
