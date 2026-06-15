#include "backend/engine/semantic/stubs/keys.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {

namespace {

// Sentinel envelope returned by `KEYS.NEW_KEYSET`. The leading
// `bigquery-emulator:keyset:v1:` prefix is the contract: any
// downstream consumer that round-trips a stub keyset can detect the
// emulator's placeholder and (a) refuse to feed it to a real
// AEAD encryption stack, (b) surface a clear "this is a placeholder"
// error in tests that mistakenly trust the BYTES round-trip. The
// trailing `<key_type>` segment echoes the caller's requested key
// type so the placeholder still carries the salient parameter; this
// matches BigQuery's contract that the returned BYTES depend on
// the input key type.
//
// We do NOT randomize -- the local-stub posture is explicitly
// deterministic so unit tests + client-library probes can pin the
// returned bytes byte-for-byte.
constexpr char kKeysetSentinelPrefix[] = "bigquery-emulator:keyset:v1:";
constexpr char kCipherSentinelPrefix[] = "bigquery-emulator:cipher:v1:";

}  // namespace

absl::StatusOr<Value> KeysNewKeyset(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic stub: KEYS.NEW_KEYSET expects exactly one ",
                     "STRING argument; got ",
                     args.size()));
  }
  const Value& key_type = args[0];
  if (key_type.is_null()) {
    return Value::NullBytes();
  }
  if (key_type.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat(
            "semantic stub: KEYS.NEW_KEYSET requires a STRING key_type; ",
            "got ",
            key_type.type()->DebugString()));
  }
  // Compose the sentinel inline -- `Value::Bytes` takes a `std::string`
  // so building it locally avoids a redundant copy.
  std::string sentinel =
      absl::StrCat(kKeysetSentinelPrefix, key_type.string_value());
  return Value::Bytes(std::move(sentinel));
}

absl::StatusOr<Value> KeysKeysetLength(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic stub: KEYS.KEYSET_LENGTH expects exactly one ",
                     "BYTES argument; got ",
                     args.size()));
  }
  const Value& keyset = args[0];
  if (keyset.is_null()) {
    return Value::NullInt64();
  }
  if (keyset.type_kind() != ::googlesql::TYPE_BYTES) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat(
            "semantic stub: KEYS.KEYSET_LENGTH requires a BYTES keyset; ",
            "got ",
            keyset.type()->DebugString()));
  }
  // The sentinel `KEYS.NEW_KEYSET` returns is a single-key keyset
  // (BigQuery's documented `NEW_KEYSET` shape). We pin the answer
  // to `1` for ANY non-NULL BYTES, including BYTES that did not
  // come from this emulator -- the local-stub contract is to
  // accept-and-return-a-shaped-answer rather than try to validate
  // a real Tink envelope. A real implementation would parse the
  // serialized `google.crypto.tink.Keyset` proto and count the
  // `key` repeated field.
  return Value::Int64(int64_t{1});
}

absl::StatusOr<Value> KeysEncrypt(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic stub: KEYS.ENCRYPT expects exactly two BYTES ",
                     "arguments; got ",
                     args.size()));
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullBytes();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_BYTES ||
      args[1].type_kind() != ::googlesql::TYPE_BYTES) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic stub: KEYS.ENCRYPT requires BYTES keyset and plaintext");
  }
  (void)args[0];
  return Value::Bytes(
      absl::StrCat(kCipherSentinelPrefix, args[1].bytes_value()));
}

absl::StatusOr<Value> KeysDecryptBytes(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic stub: KEYS.DECRYPT_BYTES expects exactly two ",
                     "BYTES arguments; got ",
                     args.size()));
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullBytes();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_BYTES ||
      args[1].type_kind() != ::googlesql::TYPE_BYTES) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic stub: KEYS.DECRYPT_BYTES requires BYTES keyset and "
        "ciphertext");
  }
  (void)args[0];
  const std::string& ciphertext = args[1].bytes_value();
  if (absl::StartsWith(ciphertext, kCipherSentinelPrefix)) {
    return Value::Bytes(ciphertext.substr(strlen(kCipherSentinelPrefix)));
  }
  return Value::Bytes("bigquery-emulator:decrypted:v1");
}

}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
