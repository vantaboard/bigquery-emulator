#include "backend/engine/semantic/python_udf_runtime_internal.h"

#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/python_udf_runtime_json.h"

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

constexpr absl::string_view kPackagePreflightRunner = R"PY(
import importlib.util
import json
import sys

def _module_name(pkg):
    for sep in ("==", ">=", "<=", "!=", "~=", "<", ">"):
        if sep in pkg:
            return pkg.split(sep, 1)[0].strip()
    return pkg.strip()

def main():
    packages = json.load(sys.stdin)
    for pkg in packages:
        mod = _module_name(pkg)
        if importlib.util.find_spec(mod) is None:
            json.dump({"ok": False, "missing": pkg}, sys.stdout)
            return
    json.dump({"ok": True}, sys.stdout)

if __name__ == "__main__":
    main()
)PY";

absl::Status PythonUdfError(absl::string_view message) {
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      absl::StrCat("User-defined function error: ", message));
}

absl::Status MissingPythonPackageError(absl::string_view package) {
  return PythonUdfError(
      absl::StrCat("Python package ",
                   package,
                   " is not installed in the configured Python environment. "
                   "Install dependencies with `task python-udf:provision` "
                   "(see docs/guides/python-udfs.md)."));
}

absl::StatusOr<std::string> InvokePythonScript(
    absl::string_view python_path,
    absl::string_view script,
    absl::string_view stdin_payload) {
  int stdin_pipe[2] = {-1, -1};
  int stdout_pipe[2] = {-1, -1};
  if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0) {
    return absl::InternalError("python_udf_runtime: failed to create pipe");
  }

  const std::string runner(script);
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
    execlp(python_path.data(),
           python_path.data(),
           "-c",
           runner.c_str(),
           static_cast<char*>(nullptr));
    _exit(127);
  }

  close(stdin_pipe[0]);
  close(stdout_pipe[1]);
  if (!stdin_payload.empty() &&
      write(stdin_pipe[1], stdin_payload.data(), stdin_payload.size()) < 0) {
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

struct PackagePreflightCacheEntry {
  bool ok = false;
  std::string missing_package;
};

absl::Mutex package_preflight_mu;
absl::flat_hash_map<std::string, PackagePreflightCacheEntry>
    package_preflight_cache ABSL_GUARDED_BY(package_preflight_mu);

std::string PackagePreflightCacheKey(absl::string_view python_path,
                                     const std::vector<std::string>& packages) {
  return absl::StrCat(python_path, "\0", absl::StrJoin(packages, "\0"));
}

absl::Status DoPreflightPythonPackages(
    absl::string_view python_path, const std::vector<std::string>& packages) {
  if (packages.empty()) {
    return absl::OkStatus();
  }

  const std::string cache_key = PackagePreflightCacheKey(python_path, packages);
  {
    absl::MutexLock lock(&package_preflight_mu);
    auto it = package_preflight_cache.find(cache_key);
    if (it != package_preflight_cache.end()) {
      if (it->second.ok) {
        return absl::OkStatus();
      }
      return MissingPythonPackageError(it->second.missing_package);
    }
  }

  std::string request = "[";
  for (size_t i = 0; i < packages.size(); ++i) {
    if (i > 0) request.push_back(',');
    absl::StrAppend(&request, JsonEscape(packages[i]));
  }
  request.push_back(']');

  absl::StatusOr<std::string> response_or =
      InvokePythonScript(python_path, kPackagePreflightRunner, request);
  PackagePreflightCacheEntry entry;
  if (!response_or.ok()) {
    return response_or.status();
  }

  absl::StatusOr<bool> ok_or = ExtractJsonBoolField(*response_or, "ok");
  if (!ok_or.ok()) {
    return ok_or.status();
  }
  entry.ok = *ok_or;
  if (!entry.ok) {
    absl::StatusOr<std::string> missing_or =
        ExtractJsonStringField(*response_or, "missing");
    entry.missing_package = missing_or.ok() ? *missing_or : packages.front();
  }

  {
    absl::MutexLock lock(&package_preflight_mu);
    package_preflight_cache.emplace(cache_key, entry);
  }

  if (entry.ok) {
    return absl::OkStatus();
  }
  return MissingPythonPackageError(entry.missing_package);
}

}  // namespace

absl::Status PreflightPythonPackages(absl::string_view python_path,
                                     const std::vector<std::string>& packages) {
  return DoPreflightPythonPackages(python_path, packages);
}

absl::StatusOr<std::string> InvokePythonUdfRunner(
    absl::string_view python_path, absl::string_view request_json) {
  return InvokePythonScript(python_path, kPythonRunner, request_json);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
