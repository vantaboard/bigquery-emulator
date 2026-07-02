#include "backend/catalog/emulator_external_query_tvf_extensions.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/call_once.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/external_query_fixture.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/public/input_argument_type.h"
#include "googlesql/public/table_valued_function.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

using ::googlesql::AnalyzerOptions;
using ::googlesql::Catalog;
using ::googlesql::FunctionArgumentType;
using ::googlesql::FunctionSignature;
using ::googlesql::InputArgumentType;
using ::googlesql::TableValuedFunction;
using ::googlesql::TableValuedFunctionOptions;
using ::googlesql::TVFComputeResultTypeCallback;
using ::googlesql::TVFInputArgumentType;
using ::googlesql::TVFRelation;
using ::googlesql::TVFSignature;
using ::googlesql::TypeFactory;
using ::googlesql::Value;

std::vector<std::unique_ptr<const TableValuedFunction>>*
EmulatorExternalQueryTvfs() {
  static auto* tvfs =
      new std::vector<std::unique_ptr<const TableValuedFunction>>();
  return tvfs;
}

absl::once_flag g_register_once;
::googlesql::TypeFactory* ExternalQueryTypeFactory() {
  static auto* factory = new ::googlesql::TypeFactory();
  return factory;
}

absl::StatusOr<const ::googlesql::Type*> TypeForSchemaColumn(
    absl::string_view type_name, TypeFactory* factory) {
  const std::string upper = absl::AsciiStrToUpper(type_name);
  if (upper == "INT64" || upper == "INTEGER") {
    return factory->get_int64();
  }
  if (upper == "FLOAT64" || upper == "FLOAT" || upper == "DOUBLE") {
    return factory->get_double();
  }
  if (upper == "BOOL" || upper == "BOOLEAN") {
    return factory->get_bool();
  }
  if (upper == "STRING") {
    return factory->get_string();
  }
  if (upper == "BYTES") {
    return factory->get_bytes();
  }
  if (upper == "DATE") {
    return factory->get_date();
  }
  if (upper == "TIME") {
    return factory->get_time();
  }
  if (upper == "DATETIME") {
    return factory->get_datetime();
  }
  if (upper == "TIMESTAMP") {
    return factory->get_timestamp();
  }
  if (upper == "NUMERIC") {
    return factory->get_numeric();
  }
  if (upper == "BIGNUMERIC") {
    return factory->get_bignumeric();
  }
  if (upper == "JSON") {
    return factory->get_json();
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "EXTERNAL_QUERY fixture: unsupported column type '", type_name, "'"));
}

absl::StatusOr<std::string> ExtractStringLiteral(
    const TVFInputArgumentType& arg, absl::string_view label) {
  if (!arg.is_scalar()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "EXTERNAL_QUERY requires a constant ", label, " argument"));
  }
  absl::StatusOr<InputArgumentType> input_type = arg.GetScalarArgType();
  if (!input_type.ok()) {
    return input_type.status();
  }
  const Value* lit = input_type->literal_value();
  if (lit == nullptr || lit->type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError(absl::StrCat(
        "EXTERNAL_QUERY requires a constant STRING ",
        label,
        " argument (fixture mode does not infer schema without a manifest "
        "entry)"));
  }
  return std::string(lit->string_value());
}

absl::StatusOr<std::shared_ptr<TVFSignature>> ExternalQuerySchema(
    TypeFactory* factory, const std::vector<TVFInputArgumentType>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "EXTERNAL_QUERY requires (connection STRING, query STRING)");
  }
  absl::StatusOr<std::string> connection =
      ExtractStringLiteral(args[0], "connection");
  if (!connection.ok()) {
    return connection.status();
  }
  absl::StatusOr<std::string> query = ExtractStringLiteral(args[1], "query");
  if (!query.ok()) {
    return query.status();
  }
  absl::StatusOr<engine::semantic::ExternalQueryFixtureResult> fixture =
      engine::semantic::LoadExternalQueryFixture(*connection, *query, factory);
  if (!fixture.ok()) {
    return fixture.status();
  }
  TVFRelation::ColumnList out_cols;
  out_cols.reserve(fixture->schema.size());
  for (const engine::semantic::ExternalQueryFixtureColumn& col :
       fixture->schema) {
    absl::StatusOr<const ::googlesql::Type*> type =
        TypeForSchemaColumn(col.type, factory);
    if (!type.ok()) {
      return type.status();
    }
    out_cols.emplace_back(col.name, *type);
  }
  return TVFSignature::Create(args, TVFRelation(std::move(out_cols)));
}

TVFComputeResultTypeCallback MakeComputeCallback() {
  return [](Catalog* catalog,
            TypeFactory* type_factory,
            const FunctionSignature& signature,
            const std::vector<TVFInputArgumentType>& args,
            const AnalyzerOptions& options)
             -> absl::StatusOr<std::shared_ptr<TVFSignature>> {
    (void)catalog;
    (void)signature;
    (void)options;
    return ExternalQuerySchema(type_factory, args);
  };
}

void EnsureEmulatorExternalQueryTvfsCreated() {
  absl::call_once(g_register_once, []() {
    const int64_t context_id = 0;
    const FunctionArgumentType relation_result =
        FunctionArgumentType::AnyRelation();
    TableValuedFunctionOptions options;
    options.set_uses_upper_case_sql_name(true);
    options.set_compute_result_type_callback(MakeComputeCallback());
    const FunctionArgumentType string_arg =
        FunctionArgumentType(ExternalQueryTypeFactory()->get_string());
    EmulatorExternalQueryTvfs()->push_back(
        std::make_unique<TableValuedFunction>(
            std::vector<std::string>{"EXTERNAL_QUERY"},
            ::googlesql::Function::kGoogleSQLFunctionGroupName,
            std::vector<FunctionSignature>{FunctionSignature(
                relation_result, {string_arg, string_arg}, context_id)},
            std::move(options)));
  });
}

}  // namespace

void RegisterEmulatorExternalQueryTvf(::googlesql::SimpleCatalog& catalog) {
  EnsureEmulatorExternalQueryTvfsCreated();
  for (const auto& tvf : *EmulatorExternalQueryTvfs()) {
    if (tvf != nullptr) {
      catalog.AddTableValuedFunction(tvf.get());
    }
  }
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
