#include "backend/catalog/python_udf_registry.h"

#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

TEST(PythonUdfRegistryTest, ParsePythonUdfFromDdlRejectsAggregate) {
  const char* ddl = R"(
CREATE AGGREGATE FUNCTION weighted_avg(x FLOAT64, w FLOAT64)
RETURNS FLOAT64
LANGUAGE python
OPTIONS (entry_point='weighted_avg')
AS R"""
def weighted_avg(x, w):
  return sum(x * w) / sum(w)
""")";
  absl::StatusOr<PythonUdfDefinition> def_or =
      ParsePythonUdfFromDdl(ddl, "weighted_avg");
  ASSERT_FALSE(def_or.ok());
  EXPECT_EQ(def_or.status().message(),
            "CREATE AGGREGATE FUNCTION with language python is not supported");
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
