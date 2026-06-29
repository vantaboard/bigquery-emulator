
#include <unistd.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

constexpr absl::string_view kPythonRunner = R"PY(
import json
import re
import sys

def _json_default(value):
    if value is None:
        return None
    if isinstance(value, bool):
        return value
    if isinstance(value, int):
        return value
    if isinstance(value, float):
        return value
    if isinstance(value, str):
        return value
    raise TypeError(f"unsupported Python UDF return type: {type(value)!r}")

def _resolve_entry_point(body, fn_name, entry_point):
    if entry_point:
        return entry_point
    if re.search(rf"(?m)^def\s+{re.escape(fn_name)}\s*\(", body):
        return fn_name
    defs = re.findall(r"(?m)^def\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(", body)
    if len(defs) == 1:
        return defs[0]
    return ""

def _run_inline(body, arg_names, args):
    params = ", ".join(arg_names)
    indented = "\n".join("    " + line for line in body.splitlines())
    source = f"def __bqemu_inline__({params}):\n{indented}\n"
    ns = {}
    exec(source, ns)
    return ns["__bqemu_inline__"](*args)

def main():
    payload = json.load(sys.stdin)
    body = payload["body"]
    fn_name = payload["fn_name"]
    entry_point = _resolve_entry_point(body, fn_name, payload.get("entry_point") or "")
    arg_names = payload["arg_names"]
    arg_values = [payload["args"][name] for name in arg_names]

    if entry_point:
        ns = {}
        exec(body, ns)
        fn = ns.get(entry_point)
        if fn is None:
            raise NameError(f"Python UDF entry point {entry_point!r} not found")
        result = fn(*arg_values)
    else:
        result = _run_inline(body, arg_names, arg_values)

    json.dump({"ok": True, "result": _json_default(result)}, sys.stdout)

if __name__ == "__main__":
    try:
        main()
    except Exception as exc:  # noqa: BLE001
        json.dump({"ok": False, "error": str(exc)}, sys.stdout)
        sys.exit(1)
)PY";

absl::Status PythonUdfError(absl::string_view message) {
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      absl::StrCat("User-defined function error: ", message));
}

std::string JsonEscape(absl::string_view s) {
  std::string out;
  out.reserve(s.size() + 8);
  out.push_back('"');
  for (char ch : s) {
    switch (ch) {
      case '"':
        out.append("\\\"");
        break;
      case '\\':
        out.append("\\\\");
        break;
      case '\b':
        out.append("\\b");
        break;
      case '\f':
        out.append("\\f");
        break;
      case '\n':
        out.append("\\n");
        break;
      case '\r':
        out.append("\\r");
        break;
      case '\t':
        out.append("\\t");
        break;
      default:
        if (static_cast<unsigned char>(ch) < 0x20) {
          absl::StrAppend(&out, absl::CEscape(absl::string_view(&ch, 1)));
        } else {
          out.push_back(ch);
        }
        break;
    }
  }
  out.push_back('"');
  return out;
}

std::string JsonValue(const Value& value, ::googlesql::TypeKind type_kind) {
  if (value.is_null()) return "null";
  switch (type_kind) {
    case ::googlesql::TYPE_BOOL:
      return value.bool_value() ? "true" : "false";
    case ::googlesql::TYPE_INT64:
      return std::to_string(value.int64_value());
    case ::googlesql::TYPE_DOUBLE: {
      const double d = value.double_value();
      if (!std::isfinite(d)) return "null";
      return absl::StrCat(d);
    }
    case ::googlesql::TYPE_STRING:
      return JsonEscape(value.string_value());
    default:
      return "null";
  }
}

absl::StatusOr<std::string> PythonInterpreterPath() {
  if (const char* env = std::getenv("BIGQUERY_EMULATOR_PYTHON");
      env != nullptr && env[0] != '\0') {
    return std::string(env);
  }
  return std::string("python3");
}

absl::StatusOr<std::string> BuildRequestJson(
    absl::string_view fn_name,
    const catalog::PythonUdfDefinition& definition,
    const std::vector<Value>& arg_values) {
  std::string json = "{";
  absl::StrAppend(&json, "\"body\":", JsonEscape(definition.python_body), ",");
  absl::StrAppend(&json, "\"fn_name\":", JsonEscape(fn_name), ",");
  absl::StrAppend(
      &json, "\"entry_point\":", JsonEscape(definition.entry_point), ",");
  absl::StrAppend(&json,
                  "\"arg_names\":[",
                  absl::StrJoin(definition.arg_names,
                                ",",
                                [](std::string* out, const std::string& name) {
                                  absl::StrAppend(out, JsonEscape(name));
                                }),
                  "],\"args\":{");
  for (size_t i = 0; i < arg_values.size(); ++i) {
    if (i > 0) json.push_back(',');
    const ::googlesql::TypeKind type_kind = i < definition.arg_type_kinds.size()
                                                ? definition.arg_type_kinds[i]
                                                : ::googlesql::TYPE_UNKNOWN;
    absl::StrAppend(&json,
                    JsonEscape(definition.arg_names[i]),
                    ":",
                    JsonValue(arg_values[i], type_kind));
  }
  json.append("}}");
  return json;
}

absl::StatusOr<std::string> ExtractJsonStringField(absl::string_view json,
                                                   absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InternalError(
        absl::StrCat("python_udf_runtime: response missing field ", field));
  }
  absl::string_view rest = json.substr(pos + needle.size());
  while (!rest.empty() && absl::ascii_isspace(rest.front())) {
    rest.remove_prefix(1);
  }
  if (rest.empty() || rest.front() != '"') {
    return absl::InternalError(
        "python_udf_runtime: expected string field in response");
  }
  rest.remove_prefix(1);
  std::string out;
  while (!rest.empty()) {
    const char ch = rest.front();
    rest.remove_prefix(1);
    if (ch == '"') return out;
    if (ch == '\\') {
      if (rest.empty()) break;
      const char esc = rest.front();
      rest.remove_prefix(1);
      switch (esc) {
        case 'n':
          out.push_back('\n');
          break;
        case 'r':
          out.push_back('\r');
          break;
        case 't':
          out.push_back('\t');
          break;
        case '"':
          out.push_back('"');
          break;
        case '\\':
          out.push_back('\\');
          break;
        default:
          out.push_back(esc);
          break;
      }
      continue;
    }
    out.push_back(ch);
  }
  return absl::InternalError(
      "python_udf_runtime: unterminated string in response");
}

absl::StatusOr<bool> ExtractJsonBoolField(absl::string_view json,
                                          absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InternalError(
        absl::StrCat("python_udf_runtime: response missing field ", field));
  }
  absl::string_view rest = json.substr(pos + needle.size());
  while (!rest.empty() && absl::ascii_isspace(rest.front())) {
    rest.remove_prefix(1);
  }
  if (absl::StartsWith(rest, "true")) return true;
  if (absl::StartsWith(rest, "false")) return false;
  return absl::InternalError("python_udf_runtime: expected bool field");
}

absl::StatusOr<Value> PopPythonValueToGooglesql(
    absl::string_view raw_result, const ::googlesql::Type* return_type) {
  if (return_type == nullptr) {
    return absl::InternalError("python_udf_runtime: missing return type");
  }
  absl::string_view trimmed = raw_result;
  while (!trimmed.empty() && absl::ascii_isspace(trimmed.front())) {
    trimmed.remove_prefix(1);
  }
  if (trimmed == "null") {
    return Value::Null(return_type);
  }
  switch (return_type->kind()) {
    case ::googlesql::TYPE_BOOL: {
      if (trimmed == "true") return Value::Bool(true);
      if (trimmed == "false") return Value::Bool(false);
      return PythonUdfError("Python UDF must return BOOL");
    }
    case ::googlesql::TYPE_INT64: {
      int64_t out = 0;
      if (!absl::SimpleAtoi(trimmed, &out)) {
        return PythonUdfError("Python UDF INT64 return is not a valid integer");
      }
      return Value::Int64(out);
    }
    case ::googlesql::TYPE_DOUBLE: {
      double out = 0.0;
      if (!absl::SimpleAtod(trimmed, &out) || !std::isfinite(out)) {
        return PythonUdfError("Python UDF must return FLOAT64");
      }
      return Value::Double(out);
    }
    case ::googlesql::TYPE_STRING: {
      absl::string_view text = trimmed;
      if (text.empty() || text.front() != '"') {
        return PythonUdfError("Python UDF must return STRING");
      }
      auto parsed = ExtractJsonStringField(
          absl::StrCat("{\"result\":", text, "}"), "result");
      if (!parsed.ok()) return parsed.status();
      return Value::String(*parsed);
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("Python UDF return type ",
                       ::googlesql::TypeKind_Name(return_type->kind()),
                       " is not supported"));
  }
}

absl::StatusOr<std::string> InvokePythonRunner(absl::string_view request_json) {
  absl::StatusOr<std::string> python_or = PythonInterpreterPath();
  if (!python_or.ok()) return python_or.status();

  int stdin_pipe[2] = {-1, -1};
  int stdout_pipe[2] = {-1, -1};
  if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0) {
    return absl::InternalError("python_udf_runtime: failed to create pipe");
  }

  const std::string runner = std::string(kPythonRunner);
  const pid_t child = fork();
  if (child < 0) {
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    return absl::InternalError("python_udf_runtime: fork failed");
  }

  if (child == 0) {
    dup2(stdin_pipe[0], STDIN_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    execlp(python_or->c_str(),
           python_or->c_str(),
           "-c",
           runner.c_str(),
           static_cast<char*>(nullptr));
    _exit(127);
  }

  close(stdin_pipe[0]);
  close(stdout_pipe[1]);
  const std::string request(request_json);
  if (!request.empty() &&
      write(stdin_pipe[1], request.data(), request.size()) < 0) {
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    waitpid(child, nullptr, 0);
    return absl::InternalError("python_udf_runtime: failed to write request");
  }
  close(stdin_pipe[1]);

  std::string output;
  std::array<char, 4096> buf{};
  ssize_t nread = 0;
  while ((nread = read(stdout_pipe[0], buf.data(), buf.size())) > 0) {
    output.append(buf.data(), static_cast<size_t>(nread));
  }
  close(stdout_pipe[0]);

  int status = 0;
  waitpid(child, &status, 0);
  if (output.empty()) {
    return absl::InternalError(
        "python_udf_runtime: Python interpreter produced no output");
  }
  return output;
}

}  // namespace

absl::StatusOr<Value> EvalPythonUdfCall(
    absl::string_view fn_name,
    const catalog::PythonUdfDefinition& definition,
    const std::vector<Value>& arg_values,
    const ::googlesql::Type* return_type,
    const std::vector<const ::googlesql::Type*>& arg_types) {
  (void)arg_types;
  if (definition.is_aggregate) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "Python aggregate UDF call-time evaluation is not implemented");
  }
  if (definition.arg_names.size() != arg_values.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: Python UDF argument count mismatch (expected ",
                     definition.arg_names.size(),
                     ", got ",
                     arg_values.size(),
                     ")"));
  }

  for (const std::string& pkg : definition.packages) {
    const std::string module = pkg.substr(0, pkg.find("=="));
    if (module == "lxml") continue;
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("Python UDF package ",
                     pkg,
                     " is not available in the emulator runtime"));
  }

  absl::StatusOr<std::string> request_or =
      BuildRequestJson(fn_name, definition, arg_values);
  if (!request_or.ok()) return request_or.status();

  absl::StatusOr<std::string> response_or = InvokePythonRunner(*request_or);
  if (!response_or.ok()) return response_or.status();

  absl::StatusOr<bool> ok_or = ExtractJsonBoolField(*response_or, "ok");
  if (!ok_or.ok()) return ok_or.status();
  if (!*ok_or) {
    absl::StatusOr<std::string> err_or =
        ExtractJsonStringField(*response_or, "error");
    return PythonUdfError(err_or.ok() ? *err_or : "Python UDF failed");
  }

  const std::string response = *response_or;
  const std::string result_needle = "\"result\":";
  const size_t result_pos = response.find(result_needle);
  if (result_pos == std::string::npos) {
    return absl::InternalError("python_udf_runtime: response missing result");
  }
  absl::string_view result_json = response;
  result_json.remove_prefix(result_pos + result_needle.size());
  while (!result_json.empty() && absl::ascii_isspace(result_json.front())) {
    result_json.remove_prefix(1);
  }
  const size_t end = result_json.find('}');
  if (end != absl::string_view::npos) {
    result_json = result_json.substr(0, end);
  }
  return PopPythonValueToGooglesql(result_json, return_type);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
