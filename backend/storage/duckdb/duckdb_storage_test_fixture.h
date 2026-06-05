#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_TEST_FIXTURE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_TEST_FIXTURE_H_

#include <cstdlib>
#include <filesystem>
#include <random>
#include <string>

#include "absl/strings/str_cat.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

class DuckDBStorageTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ = std::filesystem::path(tmpdir) /
                absl::StrCat("bqemu-duckdb-storage-test-", rng());
    std::error_code ec;
    std::filesystem::remove_all(data_dir_, ec);
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(data_dir_, ec);
  }

  std::filesystem::path data_dir_{};
};

schema::TableSchema PeopleSchema();

Row MakePerson(int64_t id, absl::string_view name);

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif
