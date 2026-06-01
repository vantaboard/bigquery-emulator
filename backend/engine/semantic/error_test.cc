// Tests for the semantic-executor structured error surface.
//
// The error surface is consumed by the coordinator / gateway to
// map evaluation failures onto BigQuery REST error envelopes, so
// the tests pin the public contracts (status code per reason,
// payload round-trip, REST reason token spelling) the gateway
// depends on.

#include "backend/engine/semantic/error.h"

#include "absl/status/status.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

TEST(SemanticErrorTest, MakeSemanticErrorEncodesReasonPayload) {
  absl::Status status =
      MakeSemanticError(SemanticErrorReason::kDivisionByZero, "1 / 0");
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(GetSemanticErrorReason(status),
            SemanticErrorReason::kDivisionByZero);
  EXPECT_EQ(status.message(), "1 / 0");
}

TEST(SemanticErrorTest, NotImplementedReasonUsesUnimplementedCode) {
  absl::Status status = MakeSemanticError(SemanticErrorReason::kNotImplemented,
                                          "unimplemented op");
  EXPECT_EQ(status.code(), absl::StatusCode::kUnimplemented);
  EXPECT_EQ(GetSemanticErrorReason(status),
            SemanticErrorReason::kNotImplemented);
}

TEST(SemanticErrorTest, OverflowReasonRoundTrips) {
  absl::Status status =
      MakeSemanticError(SemanticErrorReason::kOverflow, "int64 overflow");
  EXPECT_EQ(GetSemanticErrorReason(status), SemanticErrorReason::kOverflow);
}

TEST(SemanticErrorTest, InvalidArgumentReasonRoundTrips) {
  absl::Status status =
      MakeSemanticError(SemanticErrorReason::kInvalidArgument, "bad operand");
  EXPECT_EQ(GetSemanticErrorReason(status),
            SemanticErrorReason::kInvalidArgument);
}

TEST(SemanticErrorTest, GetReasonOnOkStatusDefaultsToInvalidArgument) {
  EXPECT_EQ(GetSemanticErrorReason(absl::OkStatus()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(SemanticErrorTest, GetReasonOnNonSemanticStatusDefaultsToInvalidArgument) {
  EXPECT_EQ(GetSemanticErrorReason(absl::InvalidArgumentError("plain status")),
            SemanticErrorReason::kInvalidArgument);
}

TEST(SemanticErrorTest, BigQueryReasonTokensAreStable) {
  EXPECT_EQ(BigQueryReasonToken(SemanticErrorReason::kInvalidArgument),
            "invalidQuery");
  EXPECT_EQ(BigQueryReasonToken(SemanticErrorReason::kDivisionByZero),
            "invalidQuery");
  EXPECT_EQ(BigQueryReasonToken(SemanticErrorReason::kOverflow),
            "invalidQuery");
  EXPECT_EQ(BigQueryReasonToken(SemanticErrorReason::kNotImplemented),
            "notImplemented");
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
