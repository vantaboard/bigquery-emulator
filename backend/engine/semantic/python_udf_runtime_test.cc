#include "backend/engine/semantic/python_udf_runtime.h"

#include <cstdlib>
#include <string>

#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

class ScopedEnv {
 public:
  ScopedEnv(const char* key, const char* value) : key_(key) {
    if (const char* prev = std::getenv(key)) {
      had_prev_ = true;
      prev_ = prev;
    }
    setenv(key, value, 1);
  }
  ~ScopedEnv() {
    if (had_prev_) {
      setenv(key_, prev_.c_str(), 1);
    } else {
      unsetenv(key_);
    }
  }

 private:
  const char* key_;
  bool had_prev_ = false;
  std::string prev_;
};

}  // namespace

TEST(PythonUdfRuntimeTest, ImportModuleNameFromPackageSpecStripsVersions) {
  EXPECT_EQ(ImportModuleNameFromPackageSpec("lxml"), "lxml");
  EXPECT_EQ(ImportModuleNameFromPackageSpec("lxml==4.9.3"), "lxml");
  EXPECT_EQ(ImportModuleNameFromPackageSpec("pandas>=2.1"), "pandas");
  EXPECT_EQ(ImportModuleNameFromPackageSpec(" google-cloud-translate==3.11 "),
            "google-cloud-translate");
}

TEST(PythonUdfRuntimeTest, ResolvePythonInterpreterPathPrefersExplicitEnv) {
  ScopedEnv python("BIGQUERY_EMULATOR_PYTHON", "/tmp/bqemu-test-python");
  ScopedEnv data_dir("BIGQUERY_EMULATOR_DATA_DIR", "/tmp/bqemu-data");
  auto path_or = ResolvePythonInterpreterPath();
  ASSERT_TRUE(path_or.ok());
  EXPECT_EQ(*path_or, "/tmp/bqemu-test-python");
}

TEST(PythonUdfRuntimeTest, ResolvePythonInterpreterPathFallsBackToHost) {
  unsetenv("BIGQUERY_EMULATOR_PYTHON");
  unsetenv("BIGQUERY_EMULATOR_DATA_DIR");
  auto path_or = ResolvePythonInterpreterPath();
  ASSERT_TRUE(path_or.ok());
  EXPECT_EQ(*path_or, "python3");
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
