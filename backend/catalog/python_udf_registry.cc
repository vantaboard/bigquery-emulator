#include "backend/catalog/python_udf_registry.h"

#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "backend/catalog/python_udf_registry_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct ProjectPythonUdfs {
  absl::flat_hash_map<std::string, PythonUdfDefinition> by_name{};
};

absl::Mutex mu;
absl::flat_hash_map<std::string, ProjectPythonUdfs> by_project
    ABSL_GUARDED_BY(mu);

std::string NormalizeFnKey(absl::string_view fn_name) {
  return absl::AsciiStrToLower(fn_name);
}

::googlesql::TypeKind TypeKindFromArgument(
    const ::googlesql::FunctionArgumentType& arg) {
  const ::googlesql::Type* type = arg.type();
  if (type == nullptr) return ::googlesql::TYPE_UNKNOWN;
  return type->kind();
}

std::optional<std::string> TryLiteralString(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr || expr->node_kind() != ::googlesql::RESOLVED_LITERAL) {
    return std::nullopt;
  }
  const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
  if (lit == nullptr) return std::nullopt;
  const ::googlesql::Value& v = lit->value();
  if (v.is_null() || v.type_kind() != ::googlesql::TYPE_STRING) {
    return std::nullopt;
  }
  return v.string_value();
}

absl::StatusOr<std::vector<std::string>> ExtractStringArrayLiteral(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError(
        "python_udf_registry: expected STRING array literal, got null expr");
  }
  if (expr->node_kind() == ::googlesql::RESOLVED_LITERAL) {
    const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
    if (lit == nullptr) {
      return absl::InvalidArgumentError(
          "python_udf_registry: packages literal is null");
    }
    const ::googlesql::Value& v = lit->value();
    if (v.type_kind() == ::googlesql::TYPE_ARRAY) {
      std::vector<std::string> out;
      out.reserve(v.num_elements());
      for (int i = 0; i < v.num_elements(); ++i) {
        const ::googlesql::Value& elem = v.element(i);
        if (elem.is_null() || elem.type_kind() != ::googlesql::TYPE_STRING) {
          return absl::InvalidArgumentError(
              "python_udf_registry: packages array must contain STRING "
              "literals");
        }
        out.push_back(elem.string_value());
      }
      return out;
    }
  }
  if (expr->node_kind() == ::googlesql::RESOLVED_FUNCTION_CALL) {
    const auto* call = expr->GetAs<::googlesql::ResolvedFunctionCall>();
    if (call == nullptr) {
      return absl::InvalidArgumentError(
          "python_udf_registry: packages function call is null");
    }
    std::vector<std::string> out;
    out.reserve(call->argument_list_size());
    for (int i = 0; i < call->argument_list_size(); ++i) {
      auto elem = TryLiteralString(call->argument_list(i));
      if (!elem.has_value()) {
        return absl::InvalidArgumentError(
            "python_udf_registry: packages array must contain STRING "
            "literals");
      }
      out.push_back(*elem);
    }
    if (!out.empty()) return out;
  }
  if (auto single = TryLiteralString(expr); single.has_value()) {
    return std::vector<std::string>{*single};
  }
  return absl::InvalidArgumentError(
      "python_udf_registry: expected STRING array literal for packages");
}

absl::StatusOr<std::vector<std::string>> ExtractPackagesOption(
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt) {
  std::vector<std::string> packages;
  for (int i = 0; i < create_function_stmt.option_list_size(); ++i) {
    const ::googlesql::ResolvedOption* opt =
        create_function_stmt.option_list(i);
    if (opt == nullptr || !absl::EqualsIgnoreCase(opt->name(), "packages")) {
      continue;
    }
    absl::StatusOr<std::vector<std::string>> parsed =
        ExtractStringArrayLiteral(opt->value());
    if (!parsed.ok()) return parsed.status();
    packages = std::move(*parsed);
    break;
  }
  return packages;
}

std::string ExtractEntryPointOption(
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt) {
  for (int i = 0; i < create_function_stmt.option_list_size(); ++i) {
    const ::googlesql::ResolvedOption* opt =
        create_function_stmt.option_list(i);
    if (opt == nullptr || !absl::EqualsIgnoreCase(opt->name(), "entry_point")) {
      continue;
    }
    if (auto value = TryLiteralString(opt->value()); value.has_value()) {
      return *value;
    }
    break;
  }
  return std::string();
}

absl::StatusOr<PythonUdfDefinition> BuildDefinitionFromCreateFunction(
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt) {
  if (create_function_stmt.language() != "python") {
    return absl::InvalidArgumentError(
        "python_udf_registry: statement is not LANGUAGE python");
  }
  if (create_function_stmt.is_aggregate()) {
    return absl::InvalidArgumentError(
        "CREATE AGGREGATE FUNCTION with language python is not supported");
  }
  PythonUdfDefinition def;
  def.python_body = create_function_stmt.code();
  def.is_aggregate = create_function_stmt.is_aggregate();
  def.arg_names = create_function_stmt.argument_name_list();
  const ::googlesql::FunctionSignature& signature =
      create_function_stmt.signature();
  def.arg_type_kinds.reserve(signature.arguments().size());
  for (const ::googlesql::FunctionArgumentType& arg : signature.arguments()) {
    def.arg_type_kinds.push_back(TypeKindFromArgument(arg));
  }
  const ::googlesql::Type* return_type = signature.result_type().type();
  if (return_type != nullptr) {
    def.return_type_kind = return_type->kind();
  }
  def.entry_point = ExtractEntryPointOption(create_function_stmt);
  absl::StatusOr<std::vector<std::string>> packages_or =
      ExtractPackagesOption(create_function_stmt);
  if (!packages_or.ok()) return packages_or.status();
  def.packages = std::move(*packages_or);
  if (def.python_body.empty()) {
    return absl::InvalidArgumentError(
        "python_udf_registry: Python UDF body is empty");
  }
  if (def.arg_names.size() != def.arg_type_kinds.size()) {
    return absl::InvalidArgumentError(
        "python_udf_registry: argument name/type count mismatch");
  }
  return def;
}

}  // namespace

absl::Status RegisterProjectPythonUdf(absl::string_view project_id,
                                      absl::string_view fn_name,
                                      PythonUdfDefinition definition) {
  if (project_id.empty() || fn_name.empty()) {
    return absl::InvalidArgumentError(
        "python_udf_registry: project_id and fn_name must be non-empty");
  }
  if (definition.python_body.empty()) {
    return absl::InvalidArgumentError(
        "python_udf_registry: Python UDF body is empty");
  }
  absl::MutexLock lock(&mu);
  by_project[std::string(project_id)].by_name[NormalizeFnKey(fn_name)] =
      std::move(definition);
  return absl::OkStatus();
}

absl::Status RegisterPythonUdfFromCreateFunction(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt) {
  if (create_function_stmt.language() != "python") {
    return absl::OkStatus();
  }
  const std::vector<std::string>& name_path = create_function_stmt.name_path();
  if (name_path.empty()) {
    return absl::InvalidArgumentError(
        "python_udf_registry: CREATE FUNCTION has empty name_path");
  }
  absl::StatusOr<PythonUdfDefinition> def_or =
      BuildDefinitionFromCreateFunction(create_function_stmt);
  if (!def_or.ok()) return def_or.status();
  if (def_or->entry_point.empty()) {
    def_or->entry_point =
        InferEntryPointFromBody(def_or->python_body, name_path.back());
  }
  return RegisterProjectPythonUdf(
      project_id, name_path.back(), *std::move(def_or));
}

const PythonUdfDefinition* LookupProjectPythonUdf(absl::string_view project_id,
                                                  absl::string_view fn_name) {
  if (project_id.empty() || fn_name.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  auto pit = by_project.find(std::string(project_id));
  if (pit == by_project.end()) return nullptr;
  auto fit = pit->second.by_name.find(NormalizeFnKey(fn_name));
  if (fit == pit->second.by_name.end()) return nullptr;
  return &fit->second;
}

absl::StatusOr<PythonUdfDefinition> ParsePythonUdfFromDdl(
    absl::string_view ddl_sql, absl::string_view fn_name) {
  return ParsePythonUdfFromDdlImpl(ddl_sql, fn_name);
}

void DropProjectPythonUdf(absl::string_view project_id,
                          absl::string_view fn_name) {
  if (project_id.empty() || fn_name.empty()) return;
  absl::MutexLock lock(&mu);
  auto pit = by_project.find(std::string(project_id));
  if (pit == by_project.end()) return;
  pit->second.by_name.erase(NormalizeFnKey(fn_name));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
