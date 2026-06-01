// Unit tests for the numeric-edges semantic-executor functions.
//
// These tests bypass the analyzer and call the per-function helpers
// directly with hand-built `Value` arguments. End-to-end coverage
// (analyzer -> dispatch -> result) lives in `dispatch_test.cc`.

#include "backend/engine/semantic/functions/numeric_edges.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

TEST(BitCountTest, ZeroHasNoSetBits) {
  auto v = BitCount({Value::Int64(0)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 0);
}

TEST(BitCountTest, SmallPositive) {
  auto v = BitCount({Value::Int64(7)});  // 0b111
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST(BitCountTest, NegativeOneAllBitsSet) {
  // -1 in two's-complement has every bit set; BigQuery returns 64.
  // This is the key correctness case the row exists to pin: a
  // thin macro on DuckDB's unsigned `bit_count` cannot match this
  // without re-interpreting the bit pattern.
  auto v = BitCount({Value::Int64(-1)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 64);
}

TEST(BitCountTest, Int64MinSignBitOnly) {
  auto v = BitCount({Value::Int64(std::numeric_limits<int64_t>::min())});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 1);
}

TEST(BitCountTest, NullPropagates) {
  auto v = BitCount({Value::NullInt64()});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(BitCountTest, NonInt64Rejected) {
  auto v = BitCount({Value::String("hi")});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST(BitCountTest, ArityMismatchRejected) {
  auto v = BitCount({Value::Int64(1), Value::Int64(2)});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(v.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(IeeeDivideTest, FinitePath) {
  auto v = IeeeDivide({Value::Double(10.0), Value::Double(4.0)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_DOUBLE_EQ(v->double_value(), 2.5);
}

TEST(IeeeDivideTest, DivisionByZeroProducesInf) {
  // IEEE_DIVIDE diverges from SAFE_DIVIDE and `/` here: SAFE_DIVIDE
  // returns NULL, `/` raises, IEEE_DIVIDE returns +/-Inf per IEEE 754.
  auto v = IeeeDivide({Value::Double(1.0), Value::Double(0.0)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(std::isinf(v->double_value()));
  EXPECT_GT(v->double_value(), 0.0);
}

TEST(IeeeDivideTest, NegativeOverZeroProducesNegativeInf) {
  auto v = IeeeDivide({Value::Double(-1.0), Value::Double(0.0)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(std::isinf(v->double_value()));
  EXPECT_LT(v->double_value(), 0.0);
}

TEST(IeeeDivideTest, ZeroOverZeroProducesNan) {
  auto v = IeeeDivide({Value::Double(0.0), Value::Double(0.0)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(std::isnan(v->double_value()));
}

TEST(IeeeDivideTest, NullPropagates) {
  auto v = IeeeDivide({Value::NullDouble(), Value::Double(1.0)});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST(IeeeDivideTest, NonDoubleRejected) {
  auto v = IeeeDivide({Value::Int64(1), Value::Int64(2)});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
