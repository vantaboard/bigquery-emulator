#include "backend/catalog/python_udf_registry.h"

#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

TEST(PythonUdfRegistryTest, ParsePythonUdfFromDdlExtractsPackages) {
  const char* ddl = R"(
CREATE FUNCTION py_lxml(x STRING) RETURNS STRING
LANGUAGE python
OPTIONS (entry_point='do_lxml', packages=['lxml'])
AS r"""
from lxml import etree
def do_lxml(x):
  return x
""")";
  absl::StatusOr<PythonUdfDefinition> def_or =
      ParsePythonUdfFromDdl(ddl, "py_lxml");
  ASSERT_TRUE(def_or.ok()) << def_or.status();
  ASSERT_EQ(def_or->packages.size(), 1u);
  EXPECT_EQ(def_or->packages[0], "lxml");
  EXPECT_EQ(def_or->entry_point, "do_lxml");
}

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
