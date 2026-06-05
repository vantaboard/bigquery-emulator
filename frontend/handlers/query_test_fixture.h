#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_TEST_FIXTURE_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_TEST_FIXTURE_H_

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/engine/coordinator/local_coordinator_engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/query.h"
#include "grpcpp/grpcpp.h"
#include "gtest/gtest.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {

namespace fs = std::filesystem;

class QueryServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ = fs::path(tmpdir) / absl::StrCat("bqemu-query-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened =
        backend::storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    engine_ =
        std::make_unique<backend::engine::coordinator::LocalCoordinatorEngine>(
            storage_.get());
    service_ = std::make_unique<QueryService>(storage_.get(), engine_.get());
  }

  void TearDown() override {
    service_.reset();
    engine_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  v1::QueryRequest MakeRequest(absl::string_view sql) {
    v1::QueryRequest req;
    req.set_project_id("proj-test");
    req.set_sql(std::string(sql));
    return req;
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

  fs::path data_dir_{};
  std::unique_ptr<backend::storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<backend::engine::coordinator::LocalCoordinatorEngine>
      engine_{};
  std::unique_ptr<QueryService> service_{};
};

class MessageCollector {
 public:
  std::function<bool(const v1::QueryResultRow&)> Writer() {
    return [this](const v1::QueryResultRow& msg) {
      messages_.push_back(msg);
      return true;
    };
  }
  const std::vector<v1::QueryResultRow>& messages() const {
    return messages_;
  }

 private:
  std::vector<v1::QueryResultRow> messages_{};
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_TEST_FIXTURE_H_
