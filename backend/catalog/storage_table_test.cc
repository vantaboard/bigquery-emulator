#include "backend/catalog/storage_table.h"

#include "absl/status/statusor.h"
#include "backend/storage/storage.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

TEST(StorageValueToGoogleSqlValueTest, DateStringBecomesDate) {
  absl::StatusOr<::googlesql::Value> v = StorageValueToGoogleSqlValue(
      storage::Value::String("2024-01-02"), ::googlesql::types::DateType());
  ASSERT_TRUE(v.ok()) << v.status();
  ASSERT_EQ(v->type_kind(), ::googlesql::TYPE_DATE);
  EXPECT_EQ(v->date_value(), 19724);
}

TEST(StorageValueToGoogleSqlValueTest, DatetimeStringBecomesDatetime) {
  absl::StatusOr<::googlesql::Value> v = StorageValueToGoogleSqlValue(
      storage::Value::String("2024-01-02 03:04:05.250000"),
      ::googlesql::types::DatetimeType());
  ASSERT_TRUE(v.ok()) << v.status();
  ASSERT_EQ(v->type_kind(), ::googlesql::TYPE_DATETIME);
  EXPECT_EQ(v->DebugString(), "2024-01-02 03:04:05.250");
}

TEST(StorageValueToGoogleSqlValueTest, TimeStringBecomesTime) {
  absl::StatusOr<::googlesql::Value> v = StorageValueToGoogleSqlValue(
      storage::Value::String("01:02:03.000000"), ::googlesql::types::TimeType());
  ASSERT_TRUE(v.ok()) << v.status();
  ASSERT_EQ(v->type_kind(), ::googlesql::TYPE_TIME);
  EXPECT_EQ(v->DebugString(), "01:02:03");
}

TEST(StorageValueToGoogleSqlValueTest, InvalidDateStringIsRejected) {
  absl::StatusOr<::googlesql::Value> v = StorageValueToGoogleSqlValue(
      storage::Value::String("not-a-date"), ::googlesql::types::DateType());
  EXPECT_FALSE(v.ok());
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
