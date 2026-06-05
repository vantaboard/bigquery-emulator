#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_KEYS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_KEYS_H_

// KEYS.* `local_stub` family. The BigQuery `KEYS` functions sit
// between two real BigQuery components (the AEAD encryption stack
// and the Tink keyset round-trip). The emulator deliberately does
// NOT model that stack -- shipping a Tink-shaped key material
// implementation in a local emulator would force every user of the
// emulator onto a fake keyset bound to a real KMS root key (or onto
// a sandboxed Tink fork that does not exist today).
//
// Instead, `local-exec-15-specialized-stubs.plan.md` picks the
// `local_stub` posture for the two key-management entry points that
// client-library startup probes routinely call:
//
//   * `KEYS.NEW_KEYSET(key_type STRING) -> BYTES`. BigQuery returns
//     a serialized keyset BLOB; the stub returns a fixed-shape
//     deterministic sentinel ("bigquery-emulator:keyset:v1:<key_type>"
//     ASCII-encoded as BYTES). The sentinel is parseable as BYTES
//     and round-trips through `BYTES_TO_STRING(...)`, but it is NOT
//     a Tink keyset; the encryption-bearing peers
//     (`KEYS.ENCRYPT`, `KEYS.DECRYPT_BYTES`) stay `unsupported` and
//     surface UNIMPLEMENTED from the unsupported stub executor.
//
//   * `KEYS.KEYSET_LENGTH(keyset BYTES) -> INT64`. BigQuery returns
//     the number of keys in the serialized keyset; the stub
//     unconditionally returns `1` for any non-NULL BYTES input
//     (the documented "single-key keyset" shape `NEW_KEYSET`
//     promises). NULL input propagates to NULL INT64 per BigQuery
//     scalar contract.
//
// Both contracts are deterministic and side-effect-free so the
// "BigQuery-shaped placeholder" half of the local-stub posture is
// preserved (client-library startup probes succeed; the emulator
// never silently produces a Tink-shaped value that downstream code
// might trust). The encryption operations remain unimplemented so
// the "no silent approximation" half of the local-stub posture is
// preserved (a downstream call site that round-trips the sentinel
// through `KEYS.DECRYPT_BYTES` fails loudly with UNIMPLEMENTED
// rather than emitting plausible-looking ciphertext).
//
// The functions are registered with the semantic executor's
// per-name Dispatch in `backend/engine/semantic/stubs/dispatch.cc`;
// the route classifier promotes any query containing a
// `local_stub`-marked function to `kLocalStub` so the coordinator
// reaches the semantic executor's dispatch path.

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {

// `KEYS.NEW_KEYSET(key_type STRING) -> BYTES`. Deterministic
// sentinel; see header comment for the contract. The expected
// argument list is `[STRING key_type]`; any other arity / type
// surfaces INVALID_ARGUMENT through `MakeSemanticError(
// SemanticErrorReason::kInvalidArgument, ...)`. NULL input
// propagates to NULL BYTES.
absl::StatusOr<Value> KeysNewKeyset(const std::vector<Value>& args);

// `KEYS.KEYSET_LENGTH(keyset BYTES) -> INT64`. Unconditional `1`
// for any non-NULL BYTES; see header comment for the contract.
absl::StatusOr<Value> KeysKeysetLength(const std::vector<Value>& args);

}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_KEYS_H_
