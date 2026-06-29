
#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

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
        "python_udf_registry: Python aggregate UDFs are not supported");
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

absl::string_view SkipWhitespace(absl::string_view s) {
  while (!s.empty() && absl::ascii_isspace(s.front())) {
    s.remove_prefix(1);
  }
  return s;
}

absl::StatusOr<std::string> ParseQuotedPythonBody(absl::string_view ddl) {
  ddl = SkipWhitespace(ddl);
  if (ddl.empty()) {
    return absl::InvalidArgumentError("ParsePythonUdfFromDdl: empty ddl_sql");
  }
  const char quote = ddl.front();
  if (quote != '"' && quote != '\'') {
    if (absl::StartsWith(ddl, "r") &&
        (ddl.size() >= 2 && (ddl[1] == '"' || ddl[1] == '\''))) {
      ddl.remove_prefix(1);
      return ParseQuotedPythonBody(ddl);
    }
    return absl::InvalidArgumentError(
        "ParsePythonUdfFromDdl: expected quoted Python body");
  }
  ddl.remove_prefix(1);
  std::string body;
  body.reserve(ddl.size());
  const bool triple = ddl.size() >= 2 && ddl[0] == quote && ddl[1] == quote;
  if (triple) {
    ddl.remove_prefix(2);
    const std::string end = std::string(3, quote);
    const size_t pos = ddl.find(end);
    if (pos == absl::string_view::npos) {
      return absl::InvalidArgumentError(
          "ParsePythonUdfFromDdl: unterminated triple-quoted body");
    }
    body.assign(ddl.data(), pos);
    return body;
  }
  while (!ddl.empty()) {
    const char ch = ddl.front();
    ddl.remove_prefix(1);
    if (ch == '\\') {
      if (ddl.empty()) {
        return absl::InvalidArgumentError(
            "ParsePythonUdfFromDdl: trailing escape in body");
      }
      const char esc = ddl.front();
      ddl.remove_prefix(1);
      switch (esc) {
        case 'n':
          body.push_back('\n');
          break;
        case 'r':
          body.push_back('\r');
          break;
        case 't':
          body.push_back('\t');
          break;
        case '"':
          body.push_back('"');
          break;
        case '\'':
          body.push_back('\'');
          break;
        case '\\':
          body.push_back('\\');
          break;
        default:
          body.push_back(esc);
          break;
      }
      continue;
    }
    if (ch == quote) {
      return body;
    }
    body.push_back(ch);
  }
  return absl::InvalidArgumentError(
      "ParsePythonUdfFromDdl: unterminated quoted body");
}

::googlesql::TypeKind ParseTypeKindToken(absl::string_view token) {
  token = SkipWhitespace(token);
  if (absl::EqualsIgnoreCase(token, "INT64")) return ::googlesql::TYPE_INT64;
  if (absl::EqualsIgnoreCase(token, "FLOAT64")) return ::googlesql::TYPE_DOUBLE;
  if (absl::EqualsIgnoreCase(token, "FLOAT")) return ::googlesql::TYPE_DOUBLE;
  if (absl::EqualsIgnoreCase(token, "STRING")) return ::googlesql::TYPE_STRING;
  if (absl::EqualsIgnoreCase(token, "BOOL")) return ::googlesql::TYPE_BOOL;
  if (absl::EqualsIgnoreCase(token, "BOOLEAN")) return ::googlesql::TYPE_BOOL;
  return ::googlesql::TYPE_UNKNOWN;
}

std::string InferEntryPointFromBody(absl::string_view body,
                                    absl::string_view fn_name) {
  if (!fn_name.empty()) {
    const std::string needle = absl::StrCat("def ", fn_name, "(");
    for (absl::string_view line : absl::StrSplit(body, '\n')) {
      line = SkipWhitespace(line);
      if (absl::StartsWith(line, needle)) {
        return std::string(fn_name);
      }
    }
  }
  static const std::regex kNamedDef(R"(^def\s+([A-Za-z_][A-Za-z0-9_]*)\s*\()");
  std::vector<std::string> defs;
  for (absl::string_view line : absl::StrSplit(body, '\n')) {
    line = SkipWhitespace(line);
    std::smatch match;
    const std::string haystack(line);
    if (std::regex_search(haystack, match, kNamedDef) && match.size() > 1) {
      defs.push_back(match[1].str());
    }
  }
  if (defs.size() == 1) return defs.front();
  return std::string();
}

std::string ParseEntryPointFromDdl(absl::string_view ddl) {
  const std::string upper = absl::AsciiStrToUpper(ddl);
  const std::string needle = "ENTRY_POINT";
  const size_t pos = upper.find(needle);
  if (pos == std::string::npos) return std::string();
  absl::string_view after = ddl.substr(pos + needle.size());
  after = SkipWhitespace(after);
  if (!absl::StartsWith(after, "=")) return std::string();
  after.remove_prefix(1);
  after = SkipWhitespace(after);
  if (after.empty()) return std::string();
  const char quote = after.front();
  if (quote != '"' && quote != '\'') return std::string();
  after.remove_prefix(1);
  std::string out;
  while (!after.empty() && after.front() != quote) {
    out.push_back(after.front());
    after.remove_prefix(1);
  }
  return out;
}

std::vector<std::string> ParsePackagesFromDdl(absl::string_view ddl) {
  const std::string upper = absl::AsciiStrToUpper(ddl);
  const size_t pos = upper.find("PACKAGES");
  if (pos == std::string::npos) return {};
  const size_t open = ddl.find('[', pos);
  const size_t close =
      open == std::string::npos ? std::string::npos : ddl.find(']', open);
  if (open == std::string::npos || close == std::string::npos ||
      close <= open) {
    return {};
  }
  std::vector<std::string> packages;
  absl::string_view inner = ddl.substr(open + 1, close - open - 1);
  for (absl::string_view part : absl::StrSplit(inner, ',')) {
    part = SkipWhitespace(part);
    if (part.empty()) continue;
    if ((part.front() == '"' || part.front() == '\'') && part.size() >= 2) {
      const char quote = part.front();
      if (part.back() == quote) {
        packages.emplace_back(part.substr(1, part.size() - 2));
      }
    }
  }
  return packages;
}

absl::StatusOr<PythonUdfDefinition> ParsePythonUdfFromDdlImpl(
    absl::string_view ddl, absl::string_view fn_name) {
  const std::string upper = absl::AsciiStrToUpper(ddl);
  const size_t lang_pos = upper.find("LANGUAGE PYTHON");
  if (lang_pos == std::string::npos) {
    return absl::InvalidArgumentError(
        "ParsePythonUdfFromDdl: ddl_sql is not LANGUAGE python");
  }
  const size_t as_pos = upper.find(" AS ", lang_pos);
  if (as_pos == std::string::npos) {
    return absl::InvalidArgumentError(
        "ParsePythonUdfFromDdl: missing AS clause");
  }
  absl::StatusOr<std::string> body_or =
      ParseQuotedPythonBody(ddl.substr(as_pos + 4));
  if (!body_or.ok()) return body_or.status();

  PythonUdfDefinition def;
  def.python_body = std::move(*body_or);
  def.return_type_kind = ::googlesql::TYPE_UNKNOWN;
  def.entry_point = ParseEntryPointFromDdl(ddl);
  def.packages = ParsePackagesFromDdl(ddl);

  const size_t open_paren = upper.find('(');
  const size_t close_paren = open_paren == std::string::npos
                                 ? std::string::npos
                                 : upper.find(')', open_paren);
  if (open_paren != std::string::npos && close_paren != std::string::npos &&
      close_paren > open_paren) {
    const absl::string_view args =
        ddl.substr(open_paren + 1, close_paren - open_paren - 1);
    for (absl::string_view part : absl::StrSplit(args, ',')) {
      part = SkipWhitespace(part);
      if (part.empty()) continue;
      const size_t space = part.find_first_of(" \t\r\n");
      const absl::string_view name =
          space == absl::string_view::npos ? part : part.substr(0, space);
      const absl::string_view rest = space == absl::string_view::npos
                                         ? absl::string_view()
                                         : SkipWhitespace(part.substr(space));
      def.arg_names.emplace_back(name);
      def.arg_type_kinds.push_back(ParseTypeKindToken(rest));
    }
  }

  const size_t returns_pos = upper.find("RETURNS", close_paren);
  if (returns_pos != std::string::npos) {
    const absl::string_view after_returns =
        SkipWhitespace(ddl.substr(returns_pos + 7));
    const size_t space = after_returns.find_first_of(" \t\r\n");
    const absl::string_view type_token = space == absl::string_view::npos
                                             ? after_returns
                                             : after_returns.substr(0, space);
    def.return_type_kind = ParseTypeKindToken(type_token);
  }

  if (def.entry_point.empty()) {
    def.entry_point = InferEntryPointFromBody(def.python_body, fn_name);
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
