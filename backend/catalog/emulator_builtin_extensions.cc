#include "backend/catalog/emulator_builtin_extensions.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/call_once.h"
#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

std::vector<std::unique_ptr<const ::googlesql::Function>>* EmulatorFunctions() {
  static auto* fns =
      new std::vector<std::unique_ptr<const ::googlesql::Function>>();
  return fns;
}

absl::once_flag g_register_once;

void EnsureEmulatorFunctionsCreated() {
  absl::call_once(g_register_once, []() {
    ::googlesql::FunctionArgumentTypeOptions arg_opts;
    arg_opts.set_argument_name("x", ::googlesql::kPositionalOrNamed);
    const int64_t context_id = 0;
    ::googlesql::FunctionSignature signature(
        ::googlesql::FunctionArgumentType(::googlesql::types::BoolType(), 1),
        {::googlesql::FunctionArgumentType(::googlesql::ARG_TYPE_ANY_1,
                                           arg_opts)},
        context_id);
    ::googlesql::FunctionOptions options;
    options.set_uses_upper_case_sql_name(false);
    auto fn = std::make_unique<::googlesql::Function>(
        std::vector<std::string>{"isnull"},
        "Emulator",
        ::googlesql::Function::SCALAR,
        std::vector<::googlesql::FunctionSignature>{signature},
        options);
    EmulatorFunctions()->push_back(std::move(fn));
  });
}

}  // namespace

void RegisterEmulatorBuiltinFunctions(::googlesql::SimpleCatalog& catalog) {
  EnsureEmulatorFunctionsCreated();
  for (const auto& fn : *EmulatorFunctions()) {
    if (fn != nullptr) {
      catalog.AddFunction(fn.get());
    }
  }
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
