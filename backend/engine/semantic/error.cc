#include "backend/engine/semantic/error.h"

#include <string>

#include "absl/status/status.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

// Encode the reason as a single byte so the payload round-trip is
// trivial. The wire shape is private to this package; the gateway
// reads it back through `GetSemanticErrorReason` (or equivalent
// Go code that mirrors the same byte encoding) so the encoding
// stays stable.
absl::Cord EncodeReason(SemanticErrorReason reason) {
  char byte = static_cast<char>(static_cast<int>(reason));
  return absl::Cord(absl::string_view(&byte, 1));
}

SemanticErrorReason DecodeReason(const absl::Cord& cord) {
  std::string copy(cord);
  if (copy.empty()) return SemanticErrorReason::kInvalidArgument;
  int v = static_cast<unsigned char>(copy[0]);
  switch (v) {
    case 0:
      return SemanticErrorReason::kInvalidArgument;
    case 1:
      return SemanticErrorReason::kDivisionByZero;
    case 2:
      return SemanticErrorReason::kOverflow;
    case 3:
      return SemanticErrorReason::kNotImplemented;
    default:
      return SemanticErrorReason::kInvalidArgument;
  }
}

}  // namespace

absl::string_view SemanticErrorReasonName(SemanticErrorReason reason) {
  switch (reason) {
    case SemanticErrorReason::kInvalidArgument:
      return "invalid_argument";
    case SemanticErrorReason::kDivisionByZero:
      return "division_by_zero";
    case SemanticErrorReason::kOverflow:
      return "overflow";
    case SemanticErrorReason::kNotImplemented:
      return "not_implemented";
  }
  return "invalid_argument";
}

absl::string_view BigQueryReasonToken(SemanticErrorReason reason) {
  // Token spellings come from BigQuery's REST error envelope. See
  // `docs/bigquery/docs/reference/standard-sql/data-types.md` and the
  // REST jobs.query reference: division-by-zero and overflow both
  // surface under the `invalidQuery` umbrella in BigQuery's public
  // surface today, with the message distinguishing the cause. We
  // keep the structured `reason` enum so future per-token mapping
  // (e.g. gRPC trailers) doesn't have to relitigate the error
  // classification; for the wire envelope the public reason is the
  // shared `invalidQuery` token.
  switch (reason) {
    case SemanticErrorReason::kInvalidArgument:
      return "invalidQuery";
    case SemanticErrorReason::kDivisionByZero:
      return "invalidQuery";
    case SemanticErrorReason::kOverflow:
      return "invalidQuery";
    case SemanticErrorReason::kNotImplemented:
      return "notImplemented";
  }
  return "invalidQuery";
}

absl::Status MakeSemanticError(SemanticErrorReason reason,
                               absl::string_view message) {
  absl::StatusCode code = reason == SemanticErrorReason::kNotImplemented
                              ? absl::StatusCode::kUnimplemented
                              : absl::StatusCode::kInvalidArgument;
  absl::Status status(code, message);
  status.SetPayload(kSemanticErrorReasonPayloadUrl, EncodeReason(reason));
  return status;
}

SemanticErrorReason GetSemanticErrorReason(const absl::Status& status) {
  if (status.ok()) return SemanticErrorReason::kInvalidArgument;
  auto payload = status.GetPayload(kSemanticErrorReasonPayloadUrl);
  if (!payload.has_value()) return SemanticErrorReason::kInvalidArgument;
  return DecodeReason(*payload);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
