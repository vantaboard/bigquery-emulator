#include "backend/catalog/python_udf_registry_internal.h"

#include <regex>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/type.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

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
  if (upper.find("CREATE AGGREGATE FUNCTION") != std::string::npos) {
    return absl::InvalidArgumentError(
        "CREATE AGGREGATE FUNCTION with language python is not supported");
  }
  const size_t lang_pos = upper.find("LANGUAGE PYTHON");
  if (lang_pos == std::string::npos) {
    return absl::InvalidArgumentError(
        "ParsePythonUdfFromDdl: ddl_sql is not LANGUAGE python");
  }
  size_t as_pos = upper.find(" AS ", lang_pos);
  if (as_pos == std::string::npos) {
    as_pos = upper.find("\nAS ", lang_pos);
  }
  if (as_pos == std::string::npos) {
    as_pos = upper.find("\r\nAS ", lang_pos);
  }
  if (as_pos == std::string::npos) {
    return absl::InvalidArgumentError(
        "ParsePythonUdfFromDdl: missing AS clause");
  }
  absl::string_view after_as = ddl.substr(as_pos);
  after_as = SkipWhitespace(after_as);
  if (!absl::StartsWithIgnoreCase(after_as, "AS")) {
    return absl::InvalidArgumentError(
        "ParsePythonUdfFromDdl: missing AS clause");
  }
  after_as.remove_prefix(2);
  after_as = SkipWhitespace(after_as);
  absl::StatusOr<std::string> body_or = ParseQuotedPythonBody(after_as);
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

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
