// Regression tests for the UDF registry's object lifetime contract.
//
// Catalogs (notably the long-lived per-project registration catalog in
// udf_registration_catalog.cc) hold raw `googlesql::Function*` pointers
// handed out via SimpleCatalog::AddFunction. Re-registering a function
// (the routine-update / upsert path) used to destroy the old object,
// leaving those raw pointers dangling; the next
// ReplayFunctionsIntoCatalog dereferenced them and crashed the engine
// (use-after-free surfacing as an InsertOrDie duplicate-key abort or a
// SIGSEGV). See the node-bigquery-tests "Delete Routine" before-all
// hook failure in CI.

#include "backend/catalog/udf_registry.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

std::unique_ptr<const ::googlesql::Function> MakeScalarFn(
    const std::string& name) {
  ::googlesql::FunctionSignature signature(
      ::googlesql::FunctionArgumentType(::googlesql::types::Int64Type()),
      /*arguments=*/{},
      /*context_id=*/static_cast<int64_t>(0));
  return std::make_unique<::googlesql::Function>(
      std::vector<std::string>{name},
      /*group=*/"External_function",
      ::googlesql::Function::SCALAR,
      std::vector<::googlesql::FunctionSignature>{signature});
}

TEST(UdfRegistryTest, ReRegisterKeepsOldPointerValidAndReplaysNewFunction) {
  // Distinct project id: the registry is process-global state.
  const std::string project = "udf_registry_test_reregister";
  const std::string fn_name = "regtest_ds.fn1";

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog(project, &type_factory);

  ASSERT_TRUE(RegisterProjectFunction(project,
                                      /*dataset_id=*/"",
                                      /*is_temp=*/false,
                                      /*analyzer_output=*/nullptr,
                                      MakeScalarFn(fn_name))
                  .ok());
  ReplayFunctionsIntoCatalog(project, catalog);

  const ::googlesql::Function* first = nullptr;
  ASSERT_TRUE(catalog.GetFunction(fn_name, &first).ok());
  ASSERT_NE(first, nullptr);

  // Routine update path: re-register the same name. The old object
  // must stay alive because `catalog` still holds a raw pointer.
  ASSERT_TRUE(RegisterProjectFunction(project,
                                      /*dataset_id=*/"",
                                      /*is_temp=*/false,
                                      /*analyzer_output=*/nullptr,
                                      MakeScalarFn(fn_name))
                  .ok());
  EXPECT_EQ(first->Name(), fn_name);  // dereference: dies pre-fix under ASAN

  // Replay must replace the catalog entry with the new registration
  // instead of aborting on a duplicate key.
  ReplayFunctionsIntoCatalog(project, catalog);
  const ::googlesql::Function* second = nullptr;
  ASSERT_TRUE(catalog.GetFunction(fn_name, &second).ok());
  ASSERT_NE(second, nullptr);
  EXPECT_NE(second, first);
  EXPECT_TRUE(IsProjectRegisteredFunction(project, fn_name));
}

TEST(UdfRegistryTest, DropRemovesFunctionFromCatalogOnNextReplay) {
  const std::string project = "udf_registry_test_drop";
  const std::string fn_name = "regtest_ds.fn2";

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog(project, &type_factory);

  ASSERT_TRUE(RegisterProjectFunction(project,
                                      /*dataset_id=*/"",
                                      /*is_temp=*/false,
                                      /*analyzer_output=*/nullptr,
                                      MakeScalarFn(fn_name))
                  .ok());
  ReplayFunctionsIntoCatalog(project, catalog);

  const ::googlesql::Function* registered = nullptr;
  ASSERT_TRUE(catalog.GetFunction(fn_name, &registered).ok());
  ASSERT_NE(registered, nullptr);

  ASSERT_TRUE(DropProjectFunction(project, fn_name).ok());
  EXPECT_FALSE(IsProjectRegisteredFunction(project, fn_name));

  // The dropped function is retired, not destroyed, and the next
  // replay purges the stale catalog entry.
  ReplayFunctionsIntoCatalog(project, catalog);
  const ::googlesql::Function* after_drop = nullptr;
  ASSERT_TRUE(catalog.GetFunction(fn_name, &after_drop).ok());
  EXPECT_EQ(after_drop, nullptr);
}

TEST(UdfRegistryTest, FindProjectFunctionResolvesDatasetQualifiedRoutine) {
  const std::string project = "udf_registry_test_qualified";
  const std::string dataset = "ds_udf";
  const std::string routine = "add_one";

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog(project, &type_factory);

  ASSERT_TRUE(RegisterProjectFunction(project,
                                      dataset,
                                      /*is_temp=*/false,
                                      /*analyzer_output=*/nullptr,
                                      MakeScalarFn(routine))
                  .ok());
  ReplayFunctionsIntoCatalog(project, catalog);

  const ::googlesql::Function* by_short = nullptr;
  ASSERT_TRUE(catalog.GetFunction(routine, &by_short).ok());
  ASSERT_NE(by_short, nullptr);

  const ::googlesql::Function* qualified =
      FindProjectFunction(project, dataset, routine);
  ASSERT_NE(qualified, nullptr);
  EXPECT_EQ(qualified, by_short);

  EXPECT_EQ(FindProjectFunction(project, "other_ds", routine), nullptr);
  EXPECT_EQ(FindProjectFunction(project, dataset, "missing"), nullptr);

  const ::googlesql::Function* from_path = FindProjectFunctionFromPath(
      {project, dataset, routine}, project, dataset);
  ASSERT_NE(from_path, nullptr);
  EXPECT_EQ(from_path, qualified);

  const ::googlesql::Function* dotted = FindProjectFunctionFromPath(
      {project + "." + dataset + "." + routine}, project, dataset);
  ASSERT_NE(dotted, nullptr);
  EXPECT_EQ(dotted, qualified);
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
