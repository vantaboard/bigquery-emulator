// Matrix coverage for ParseParameterValue wire forms (plan 04).
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

struct ParameterMatrixCase {
  const char* type_kind;
  const char* type_json;
  const char* value_json;
  bool should_succeed;
  const char* label;
};

std::vector<ParameterMatrixCase> AcceptedMatrixCases() {
  return {
      {"TIMESTAMP",
       nullptr,
       "2026-06-22T10:00:00",
       true,
       "timestamp_naive_iso_t"},
      {"TIMESTAMP", nullptr, "2026-06-22 10:00:00", true, "timestamp_space"},
      {"TIMESTAMP", nullptr, "2026-06-22T10:00:00Z", true, "timestamp_z"},
      {"TIMESTAMP",
       nullptr,
       "2026-06-22T10:00:00+00:00",
       true,
       "timestamp_offset"},
      {"TIMESTAMP",
       nullptr,
       "2026-06-22T10:00:00.123456",
       true,
       "timestamp_fractional"},
      {"TIMESTAMP", nullptr, "2026-06-22", true, "timestamp_date_only"},
      {"DATE", nullptr, "2020-06-15", true, "date_iso"},
      {"DATETIME", nullptr, "2020-06-15 12:30:45", true, "datetime_space"},
      {"DATETIME", nullptr, "2020-06-15T12:30:45", true, "datetime_iso_t"},
      {"TIME", nullptr, "12:30:45", true, "time_hms"},
      {"TIME", nullptr, "12:30:45.123456", true, "time_fractional"},
      {"NUMERIC", nullptr, "3.14159", true, "numeric"},
      {"BIGNUMERIC",
       nullptr,
       "99999999999999999999999999999.999999999",
       true,
       "bignumeric"},
      {"BOOL", nullptr, "true", true, "bool_true"},
      {"BOOL", nullptr, "false", true, "bool_false"},
      {"BYTES", nullptr, "\"SGVsbG8=\"", true, "bytes_base64"},
      {"ARRAY", "STRING", R"(["a","b"])", true, "array_string"},
      {"STRUCT", "x:INT64,y:STRING", R"(["1","foo"])", true, "struct_mixed"},
  };
}

std::vector<ParameterMatrixCase> RejectedMatrixCases() {
  return {
      {"TIMESTAMP", nullptr, "not-a-timestamp", false, "timestamp_garbage"},
      {"DATE", nullptr, "2020-13-45", false, "date_invalid"},
      {"BOOL", nullptr, "yes", false, "bool_garbage"},
      {"BYTES", nullptr, "\"!!!\"", false, "bytes_invalid_base64"},
  };
}

class ParameterMatrixTest : public testing::TestWithParam<ParameterMatrixCase> {
};

TEST_P(ParameterMatrixTest, ParseParameterValue) {
  const ParameterMatrixCase& c = GetParam();
  absl::string_view type_json = c.type_json != nullptr ? c.type_json : "";
  auto result = ParseParameterValue(c.value_json, c.type_kind, type_json);
  if (c.should_succeed) {
    ASSERT_TRUE(result.ok()) << c.label << ": " << result.status();
  } else {
    ASSERT_FALSE(result.ok()) << c.label;
  }
}

INSTANTIATE_TEST_SUITE_P(
    Accepted,
    ParameterMatrixTest,
    testing::ValuesIn(AcceptedMatrixCases()),
    [](const testing::TestParamInfo<ParameterMatrixCase>& info) {
      return info.param.label;
    });

INSTANTIATE_TEST_SUITE_P(
    Rejected,
    ParameterMatrixTest,
    testing::ValuesIn(RejectedMatrixCases()),
    [](const testing::TestParamInfo<ParameterMatrixCase>& info) {
      return info.param.label;
    });

TEST(ValueParametersTest, TimestampNaiveIsoDefaultsToUtc) {
  auto v = ParseParameterValue("2026-06-22T10:00:00", "TIMESTAMP");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(absl::ToUnixSeconds(v->ToTime()), 1782122400);
}

TEST(ValueParametersTest, DateRoundTrips) {
  auto v = ParseParameterValue("2020-06-15", "DATE");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_DATE);
  EXPECT_FALSE(v->is_null());
}

TEST(ValueParametersTest, DatetimeSpaceForm) {
  auto v = ParseParameterValue("2020-06-15 12:30:45", "DATETIME");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_DATETIME);
}

TEST(ValueParametersTest, TimeFractionalSeconds) {
  auto v = ParseParameterValue("12:30:45.123456", "TIME");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_TIME);
}

TEST(ValueParametersTest, BytesBase64RoundTrips) {
  auto v = ParseParameterValue("\"SGVsbG8=\"", "BYTES");
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->bytes_value(), "Hello");
}

TEST(ValueParametersTest, NumericAndBigNumeric) {
  auto n = ParseParameterValue("1.5", "NUMERIC");
  ASSERT_TRUE(n.ok()) << n.status();
  auto bn = ParseParameterValue("99999999999999999999999999999.999999999",
                                "BIGNUMERIC");
  ASSERT_TRUE(bn.ok()) << bn.status();
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
