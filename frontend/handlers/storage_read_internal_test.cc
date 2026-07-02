#include "frontend/handlers/storage_read_internal.h"

#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {
namespace {

TEST(StorageReadInternalTest, TimestampValueToMicrosStringWireForms) {
  struct Case {
    const char* wire;
    const char* want;
  };
  static constexpr Case kCases[] = {
      {"2025-12-01 10:49:40+00", "1764586180000000"},
      {"2026-06-05 20:26:43.220623+00", "1780691203220623"},
      {"1764586180000000", "1764586180000000"},
  };
  for (const Case& c : kCases) {
    SCOPED_TRACE(c.wire);
    auto got =
        TimestampValueToMicrosString(backend::storage::Value::String(c.wire));
    ASSERT_TRUE(got.ok()) << got.status();
    EXPECT_EQ(*got, c.want);
  }
}

TEST(StorageReadInternalTest, TimestampValueToMicrosStringInt64Passthrough) {
  auto got = TimestampValueToMicrosString(
      backend::storage::Value::Int64(1780691203220623));
  ASSERT_TRUE(got.ok()) << got.status();
  EXPECT_EQ(*got, "1780691203220623");
}

}  // namespace
}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
