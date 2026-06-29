#include "backend/catalog/emulator_ml_tvf_extensions.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/call_once.h"
#include "absl/strings/str_join.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/public/table_valued_function.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

using ::googlesql::AnalyzerOptions;
using ::googlesql::Catalog;
using ::googlesql::FunctionArgumentType;
using ::googlesql::FunctionArgumentTypeOptions;
using ::googlesql::FunctionSignature;
using ::googlesql::TableValuedFunction;
using ::googlesql::TableValuedFunctionOptions;
using ::googlesql::TVFComputeResultTypeCallback;
using ::googlesql::TVFInputArgumentType;
using ::googlesql::TVFRelation;
using ::googlesql::TVFSignature;
using ::googlesql::TypeFactory;

std::vector<std::unique_ptr<const TableValuedFunction>>* EmulatorMlTvfs() {
  static auto* tvfs =
      new std::vector<std::unique_ptr<const TableValuedFunction>>();
  return tvfs;
}

absl::once_flag g_register_once;

const TVFInputArgumentType* FindRelationArg(
    const std::vector<TVFInputArgumentType>& args) {
  for (const TVFInputArgumentType& arg : args) {
    if (arg.is_relation()) {
      return &arg;
    }
  }
  return nullptr;
}

absl::StatusOr<std::shared_ptr<TVFSignature>> MlPredictSchema(
    TypeFactory* factory, const std::vector<TVFInputArgumentType>& args) {
  const TVFInputArgumentType* relation = FindRelationArg(args);
  if (relation == nullptr) {
    return absl::InvalidArgumentError(
        "ML.PREDICT requires a TABLE input argument");
  }
  const TVFRelation& input = relation->relation();
  TVFRelation::ColumnList out_cols;
  out_cols.reserve(input.num_columns() + 1);
  for (const TVFRelation::Column& col : input.columns()) {
    out_cols.push_back(col);
  }
  out_cols.emplace_back("predicted_label", factory->get_double());
  return TVFSignature::Create(args, TVFRelation(std::move(out_cols)));
}

absl::StatusOr<std::shared_ptr<TVFSignature>> MlEvaluateSchema(
    TypeFactory* factory, const std::vector<TVFInputArgumentType>& args) {
  (void)args;
  TVFRelation::ColumnList out_cols = {
      TVFRelation::Column("mean_squared_error", factory->get_double()),
      TVFRelation::Column("r2_score", factory->get_double()),
  };
  return TVFSignature::Create(args, TVFRelation(std::move(out_cols)));
}

absl::StatusOr<std::shared_ptr<TVFSignature>> MlForecastSchema(
    TypeFactory* factory, const std::vector<TVFInputArgumentType>& args) {
  (void)args;
  TVFRelation::ColumnList out_cols = {
      TVFRelation::Column("forecast_timestamp", factory->get_timestamp()),
      TVFRelation::Column("forecast_value", factory->get_double()),
  };
  return TVFSignature::Create(args, TVFRelation(std::move(out_cols)));
}

TVFComputeResultTypeCallback MakeComputeCallback(
    absl::StatusOr<std::shared_ptr<TVFSignature>> (*schema_fn)(
        TypeFactory*, const std::vector<TVFInputArgumentType>&)) {
  return [schema_fn](Catalog* catalog,
                     TypeFactory* type_factory,
                     const FunctionSignature& signature,
                     const std::vector<TVFInputArgumentType>& args,
                     const AnalyzerOptions& options)
             -> absl::StatusOr<std::shared_ptr<TVFSignature>> {
    (void)catalog;
    (void)signature;
    (void)options;
    return schema_fn(type_factory, args);
  };
}

void AddMlTvf(std::vector<std::string> name_path,
              std::vector<FunctionSignature> signatures,
              TVFComputeResultTypeCallback compute) {
  TableValuedFunctionOptions options;
  options.set_uses_upper_case_sql_name(true);
  options.set_compute_result_type_callback(std::move(compute));
  EmulatorMlTvfs()->push_back(std::make_unique<TableValuedFunction>(
      std::move(name_path),
      ::googlesql::Function::kGoogleSQLFunctionGroupName,
      std::move(signatures),
      std::move(options)));
}

void EnsureEmulatorMlTvfsCreated() {
  absl::call_once(g_register_once, []() {
    const int64_t context_id = 0;
    const FunctionArgumentType relation_result =
        FunctionArgumentType::AnyRelation();

    AddMlTvf({"ML", "PREDICT"},
             {FunctionSignature(relation_result,
                                {FunctionArgumentType::AnyModel(),
                                 FunctionArgumentType::AnyRelation()},
                                context_id)},
             MakeComputeCallback(MlPredictSchema));

    AddMlTvf(
        {"ML", "EVALUATE"},
        {FunctionSignature(
             relation_result, {FunctionArgumentType::AnyModel()}, context_id),
         FunctionSignature(relation_result,
                           {FunctionArgumentType::AnyModel(),
                            FunctionArgumentType::AnyRelation()},
                           context_id)},
        MakeComputeCallback(MlEvaluateSchema));

    FunctionArgumentTypeOptions struct_opts;
    struct_opts.set_argument_name("data", ::googlesql::kPositionalOrNamed);
    AddMlTvf(
        {"ML", "FORECAST"},
        {FunctionSignature(
             relation_result, {FunctionArgumentType::AnyModel()}, context_id),
         FunctionSignature(
             relation_result,
             {FunctionArgumentType::AnyModel(),
              FunctionArgumentType(::googlesql::ARG_TYPE_ANY_1, struct_opts)},
             context_id)},
        MakeComputeCallback(MlForecastSchema));
  });
}

}  // namespace

absl::StatusOr<const ::googlesql::Model*> MaterializeMlStubModel(
    const ::googlesql::TypeFactory* type_factory,
    const absl::Span<const std::string>& path) {
  if (type_factory == nullptr) {
    return absl::InvalidArgumentError(
        "MaterializeMlStubModel: type_factory is null");
  }
  if (path.empty()) {
    return absl::InvalidArgumentError(
        "MaterializeMlStubModel: model path is empty");
  }
  static auto* owned_models =
      new std::vector<std::unique_ptr<const ::googlesql::SimpleModel>>();
  const std::string name = absl::StrJoin(path, ".");
  owned_models->push_back(std::make_unique<::googlesql::SimpleModel>(
      name,
      std::vector<::googlesql::SimpleModel::NameAndType>{},
      std::vector<::googlesql::SimpleModel::NameAndType>{}));
  return owned_models->back().get();
}

absl::Status ResolveMlStubModelForAnalysis(
    ::googlesql::SimpleCatalog& catalog,
    const ::googlesql::TypeFactory* type_factory,
    const absl::Span<const std::string>& path,
    const ::googlesql::Model** model,
    const ::googlesql::Catalog::FindOptions& options) {
  if (model == nullptr) {
    return absl::InvalidArgumentError("FindModel: output pointer is null");
  }
  *model = nullptr;
  absl::Status found = catalog.SimpleCatalog::FindModel(path, model, options);
  if (found.ok() && *model != nullptr) {
    return found;
  }
  absl::StatusOr<const ::googlesql::Model*> stub =
      MaterializeMlStubModel(type_factory, path);
  if (!stub.ok()) {
    return stub.status();
  }
  *model = *stub;
  return absl::OkStatus();
}

absl::Status FindTableValuedFunctionWithUnqualifiedFallback(
    ::googlesql::SimpleCatalog& catalog,
    const absl::Span<const std::string>& path,
    const ::googlesql::TableValuedFunction** function,
    const ::googlesql::Catalog::FindOptions& options) {
  if (function == nullptr) {
    return absl::InvalidArgumentError(
        "FindTableValuedFunction: output pointer is null");
  }
  *function = nullptr;
  absl::Status found =
      catalog.SimpleCatalog::FindTableValuedFunction(path, function, options);
  if (found.ok() && *function != nullptr) {
    return found;
  }
  if (path.size() >= 2) {
    const std::vector<std::string> unqualified = {path.back()};
    return catalog.SimpleCatalog::FindTableValuedFunction(
        unqualified, function, options);
  }
  return found;
}

void RegisterEmulatorMlTvfStubs(::googlesql::SimpleCatalog& catalog) {
  EnsureEmulatorMlTvfsCreated();
  auto ml_catalog = std::make_unique<::googlesql::SimpleCatalog>(
      "ml", catalog.type_factory());
  for (const auto& tvf : *EmulatorMlTvfs()) {
    if (tvf != nullptr) {
      ml_catalog->AddTableValuedFunction(tvf.get());
    }
  }
  catalog.AddOwnedCatalog("ML", std::move(ml_catalog));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
