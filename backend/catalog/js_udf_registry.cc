#include "backend/catalog/js_udf_registry.h"

#include <memory>
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

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct ProjectJsUdfs {
  absl::flat_hash_map<std::string, JsUdfDefinition> by_name;
};

absl::Mutex mu;
absl::flat_hash_map<std::string, ProjectJsUdfs> by_project ABSL_GUARDED_BY(mu);

std::string NormalizeFnKey(absl::string_view fn_name) {
  return absl::AsciiStrToLower(fn_name);
}

::googlesql::TypeKind TypeKindFromArgument(
    const ::googlesql::FunctionArgumentType& arg) {
  const ::googlesql::Type* type = arg.type();
  if (type == nullptr) return ::googlesql::TYPE_UNKNOWN;
  return type->kind();
}

absl::StatusOr<JsUdfDefinition> BuildDefinitionFromCreateFunction(
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt) {
  if (create_function_stmt.language() != "js") {
    return absl::InvalidArgumentError(
        "js_udf_registry: statement is not LANGUAGE js");
  }
  if (create_function_stmt.is_aggregate()) {
    return absl::InvalidArgumentError(
        "js_udf_registry: JavaScript aggregate UDFs are not supported");
  }
  JsUdfDefinition def;
  def.js_body = create_function_stmt.code();
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
  if (def.js_body.empty()) {
    return absl::InvalidArgumentError(
        "js_udf_registry: JavaScript UDF body is empty");
  }
  if (def.arg_names.size() != def.arg_type_kinds.size()) {
    return absl::InvalidArgumentError(
        "js_udf_registry: argument name/type count mismatch");
  }
  return def;
}

absl::string_view SkipWhitespace(absl::string_view s) {
  while (!s.empty() && absl::ascii_isspace(s.front())) {
    s.remove_prefix(1);
  }
  return s;
}

absl::StatusOr<std::string> ParseQuotedJsBody(absl::string_view ddl) {
  ddl = SkipWhitespace(ddl);
  if (ddl.empty()) {
    return absl::InvalidArgumentError("ParseJsUdfFromDdl: empty ddl_sql");
  }
  const char quote = ddl.front();
  if (quote != '"' && quote != '\'') {
    return absl::InvalidArgumentError(
        "ParseJsUdfFromDdl: expected quoted JavaScript body");
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
          "ParseJsUdfFromDdl: unterminated triple-quoted body");
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
            "ParseJsUdfFromDdl: trailing escape in body");
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
      "ParseJsUdfFromDdl: unterminated quoted body");
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

absl::StatusOr<JsUdfDefinition> ParseJsUdfFromDdlImpl(absl::string_view ddl) {
  const std::string upper = absl::AsciiStrToUpper(ddl);
  const size_t lang_pos = upper.find("LANGUAGE JS");
  if (lang_pos == std::string::npos) {
    return absl::InvalidArgumentError(
        "ParseJsUdfFromDdl: ddl_sql is not LANGUAGE js");
  }
  const size_t as_pos = upper.find(" AS ", lang_pos);
  if (as_pos == std::string::npos) {
    return absl::InvalidArgumentError("ParseJsUdfFromDdl: missing AS clause");
  }
  absl::StatusOr<std::string> body_or =
      ParseQuotedJsBody(ddl.substr(as_pos + 4));
  if (!body_or.ok()) return body_or.status();

  JsUdfDefinition def;
  def.js_body = std::move(*body_or);
  def.return_type_kind = ::googlesql::TYPE_UNKNOWN;

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
  return def;
}

}  // namespace

absl::Status RegisterProjectJsUdf(absl::string_view project_id,
                                  absl::string_view fn_name,
                                  JsUdfDefinition definition) {
  if (project_id.empty() || fn_name.empty()) {
    return absl::InvalidArgumentError(
        "js_udf_registry: project_id and fn_name must be non-empty");
  }
  if (definition.js_body.empty()) {
    return absl::InvalidArgumentError(
        "js_udf_registry: JavaScript UDF body is empty");
  }
  absl::MutexLock lock(&mu);
  by_project[std::string(project_id)].by_name[NormalizeFnKey(fn_name)] =
      std::move(definition);
  return absl::OkStatus();
}

absl::Status RegisterJsUdfFromCreateFunction(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt) {
  if (create_function_stmt.language() != "js") {
    return absl::OkStatus();
  }
  const std::vector<std::string>& name_path = create_function_stmt.name_path();
  if (name_path.empty()) {
    return absl::InvalidArgumentError(
        "js_udf_registry: CREATE FUNCTION has empty name_path");
  }
  absl::StatusOr<JsUdfDefinition> def_or =
      BuildDefinitionFromCreateFunction(create_function_stmt);
  if (!def_or.ok()) return def_or.status();
  return RegisterProjectJsUdf(project_id, name_path.back(), *std::move(def_or));
}

const JsUdfDefinition* LookupProjectJsUdf(absl::string_view project_id,
                                          absl::string_view fn_name) {
  if (project_id.empty() || fn_name.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  auto pit = by_project.find(std::string(project_id));
  if (pit == by_project.end()) return nullptr;
  auto fit = pit->second.by_name.find(NormalizeFnKey(fn_name));
  if (fit == pit->second.by_name.end()) return nullptr;
  return &fit->second;
}

absl::StatusOr<JsUdfDefinition> ParseJsUdfFromDdl(absl::string_view ddl_sql) {
  return ParseJsUdfFromDdlImpl(ddl_sql);
}

void DropProjectJsUdf(absl::string_view project_id, absl::string_view fn_name) {
  if (project_id.empty() || fn_name.empty()) return;
  absl::MutexLock lock(&mu);
  auto pit = by_project.find(std::string(project_id));
  if (pit == by_project.end()) return;
  pit->second.by_name.erase(NormalizeFnKey(fn_name));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
