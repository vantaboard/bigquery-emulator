// Unit tests for EXTERNAL_QUERY fixture manifest resolution.

#include "backend/engine/semantic/external_query_fixture.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "absl/strings/string_view.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

namespace fs = std::filesystem;

class ScopedDataDirEnv {
 public:
  explicit ScopedDataDirEnv(const fs::path& dir) : name_("BIGQUERY_EMULATOR_DATA_DIR") {
    if (const char* prev = std::getenv(name_)) {
      prev_ = prev;
    }
    setenv(name_, dir.string().c_str(), /*overwrite=*/1);
  }
  ~ScopedDataDirEnv() {
    if (prev_.has_value()) {
      setenv(name_, prev_->c_str(), /*overwrite=*/1);
    } else {
      unsetenv(name_);
    }
  }

 private:
  const char* name_;
  std::optional<std::string> prev_;
};

void WriteFile(const fs::path& path, absl::string_view content) {
  fs::create_directories(path.parent_path());
  std::ofstream out(path);
  ASSERT_TRUE(out) << path;
  out << content;
}

fs::path ConnDir(const fs::path& data_dir, absl::string_view conn_id) {
  return data_dir / "external" / "connections" / conn_id;
}

TEST(ExternalQueryFixtureTest, JsonManifestExactQueryLookup) {
  const fs::path data_dir =
      fs::temp_directory_path() / "bqemu_ext_query_json_exact";
  fs::remove_all(data_dir);
  ScopedDataDirEnv env(data_dir);
  const fs::path conn = ConnDir(data_dir, "my_conn");
  WriteFile(conn / "queries.json", R"({
  "queries": [
    {"query": "SELECT id, name FROM users ORDER BY id", "result": "users.json"}
  ]
})");
  WriteFile(conn / "users.json", R"({
  "schema": [
    {"name": "id", "type": "INT64"},
    {"name": "name", "type": "STRING"}
  ],
  "rows": [
    {"id": 1, "name": "ada"},
    {"id": 2, "name": "linus"}
  ]
})");

  ::googlesql::TypeFactory type_factory;
  absl::StatusOr<ExternalQueryFixtureResult> result = LoadExternalQueryFixture(
      "us.my_conn", "SELECT id, name FROM users ORDER BY id", &type_factory);
  ASSERT_TRUE(result.ok()) << result.status();
  ASSERT_EQ(result->schema.size(), 2u);
  EXPECT_EQ(result->schema[0].name, "id");
  EXPECT_EQ(result->schema[1].name, "name");
  ASSERT_EQ(result->rows.size(), 2u);
}

TEST(ExternalQueryFixtureTest, YamlManifestAliasLookup) {
  const fs::path data_dir =
      fs::temp_directory_path() / "bqemu_ext_query_yaml_alias";
  fs::remove_all(data_dir);
  ScopedDataDirEnv env(data_dir);
  const fs::path conn = ConnDir(data_dir, "alias_conn");
  WriteFile(conn / "queries.yaml", R"(queries:
  - alias: info_schema_tables
    result: info.json
)");
  WriteFile(conn / "info.json", R"({
  "schema": [{"name": "n", "type": "INT64"}],
  "rows": [{"n": 42}]
})");

  ::googlesql::TypeFactory type_factory;
  absl::StatusOr<ExternalQueryFixtureResult> result =
      LoadExternalQueryFixture("alias_conn", "info_schema_tables", &type_factory);
  ASSERT_TRUE(result.ok()) << result.status();
  ASSERT_EQ(result->schema.size(), 1u);
  ASSERT_EQ(result->rows.size(), 1u);
}

TEST(ExternalQueryFixtureTest, MissingManifestEntryReturnsNotFound) {
  const fs::path data_dir =
      fs::temp_directory_path() / "bqemu_ext_query_missing_entry";
  fs::remove_all(data_dir);
  ScopedDataDirEnv env(data_dir);
  const fs::path conn = ConnDir(data_dir, "empty_conn");
  WriteFile(conn / "queries.yaml", "queries: []\n");

  ::googlesql::TypeFactory type_factory;
  absl::StatusOr<ExternalQueryFixtureResult> result =
      LoadExternalQueryFixture("empty_conn", "SELECT 1", &type_factory);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kNotFound);
  EXPECT_NE(std::string(result.status().message()).find("no YAML entry"),
            std::string::npos);
}

TEST(ExternalQueryFixtureTest, MissingResultFileReturnsNotFound) {
  const fs::path data_dir =
      fs::temp_directory_path() / "bqemu_ext_query_missing_result";
  fs::remove_all(data_dir);
  ScopedDataDirEnv env(data_dir);
  const fs::path conn = ConnDir(data_dir, "broken_conn");
  WriteFile(conn / "queries.json", R"({
  "queries": [{"query": "SELECT 1", "result": "missing.json"}]
})");

  ::googlesql::TypeFactory type_factory;
  absl::StatusOr<ExternalQueryFixtureResult> result =
      LoadExternalQueryFixture("broken_conn", "SELECT 1", &type_factory);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kNotFound);
  EXPECT_NE(std::string(result.status().message()).find("result file not found"),
            std::string::npos);
}

TEST(ExternalQueryFixtureTest, MissingConnectionDirectoryReturnsNotFound) {
  const fs::path data_dir =
      fs::temp_directory_path() / "bqemu_ext_query_missing_conn";
  fs::remove_all(data_dir);
  ScopedDataDirEnv env(data_dir);

  ::googlesql::TypeFactory type_factory;
  absl::StatusOr<ExternalQueryFixtureResult> result =
      LoadExternalQueryFixture("nope", "SELECT 1", &type_factory);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kNotFound);
  EXPECT_NE(std::string(result.status().message()).find("fixture directory not found"),
            std::string::npos);
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
