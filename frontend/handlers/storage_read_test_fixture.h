#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_TEST_FIXTURE_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_TEST_FIXTURE_H_

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/storage_read.h"
#include "gtest/gtest.h"
#include "proto/storage_read.pb.h"

namespace bigquery_emulator {
namespace frontend {

namespace fs = std::filesystem;

inline fs::path MakeTempDataDir(absl::string_view prefix) {
  const char* tmpdir_env = std::getenv("TMPDIR");
  const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
  std::random_device rd;
  std::seed_seq seed{rd(), rd()};
  std::mt19937_64 rng(seed);
  fs::path out = fs::path(tmpdir) /
                 absl::StrCat("bqemu-", std::string(prefix), "-", rng());
  std::error_code ec;
  fs::remove_all(out, ec);
  return out;
}

class StorageReadServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    data_dir_ = MakeTempDataDir("storage-read-test");
    auto opened =
        backend::storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    service_ = std::make_unique<StorageReadService>(storage_.get());
  }

  void TearDown() override {
    service_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  void CreatePeopleTable() {
    backend::schema::TableSchema schema;
    backend::schema::ColumnSchema id;
    id.name = "id";
    id.type = backend::schema::ColumnType::kInt64;
    id.mode = backend::schema::ColumnMode::kRequired;
    schema.columns.push_back(id);
    backend::schema::ColumnSchema name;
    name.name = "name";
    name.type = backend::schema::ColumnType::kString;
    name.mode = backend::schema::ColumnMode::kNullable;
    schema.columns.push_back(name);
    backend::schema::ColumnSchema tags;
    tags.name = "tags";
    tags.type = backend::schema::ColumnType::kString;
    tags.mode = backend::schema::ColumnMode::kRepeated;
    schema.columns.push_back(tags);
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
    ASSERT_TRUE(storage_->CreateTable({"proj-test", "ds", "t"}, schema).ok());
  }

  v1::CreateReadSessionRequest MakePeopleRequest() {
    v1::CreateReadSessionRequest req;
    req.set_parent("projects/proj-test");
    req.mutable_read_session()->set_table(
        "projects/proj-test/datasets/ds/tables/t");
    return req;
  }

  fs::path data_dir_{};
  std::unique_ptr<backend::storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<StorageReadService> service_{};
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_TEST_FIXTURE_H_
