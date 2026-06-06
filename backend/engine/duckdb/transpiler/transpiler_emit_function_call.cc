#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "backend/engine/duckdb/transpiler/types.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

// Lowers internal GoogleSQL operators (`$add`, `$equal`, ...) to DuckDB
// SQL infix / prefix forms. Disposition rows use `duckdb_native` with
// `duckdb_name` carrying the token; this path is what
// `route_classifier.cc` refers to as the dedicated operator emit.
static std::string TryEmitInternalOperator(
    absl::string_view name,
    const FnEntry* entry,
    const std::vector<std::string>& args) {
  if (entry == nullptr || !absl::StartsWith(name, "$")) return "";
  if (entry->disposition != Disposition::kDuckdbNative &&
      entry->disposition != Disposition::kDuckdbRewrite) {
    return "";
  }
  if (name == "$not") {
    if (args.size() != 1) return "";
    return absl::StrCat("(NOT ", args[0], ")");
  }
  if (name == "$unary_minus") {
    if (args.size() != 1) return "";
    return absl::StrCat("(-", args[0], ")");
  }
  if (name == "$is_null") {
    if (args.size() != 1) return "";
    return absl::StrCat("(", args[0], " IS NULL)");
  }
  if (name == "$is_not_null") {
    if (args.size() != 1) return "";
    return absl::StrCat("(", args[0], " IS NOT NULL)");
  }
  if (name == "$and" || name == "$or") {
    if (args.size() < 2) return "";
    const char* joiner = (name == "$and") ? " AND " : " OR ";
    return absl::StrCat("(", absl::StrJoin(args, joiner), ")");
  }
  if (args.size() != 2 || entry->duckdb_name.empty()) return "";
  return absl::StrCat("(", args[0], " ", entry->duckdb_name, " ", args[1], ")");
}

std::string Transpiler::EmitFunctionCall(
    const ::googlesql::ResolvedFunctionCall* node) {
  // Scalar function dispatch goes through the YAML-backed disposition
  // table in `functions.h` for the well-known BigQuery scalar surface
  // (math / string / conditional / regex / datetime / array). Two
  // narrow special cases stay inline:
  //   * `SAFE.<fn>(...)` (`SAFE_ERROR_MODE`) has no native DuckDB
  //     analog; we short-circuit to "" so the fallback fires
  //     regardless of the underlying function's disposition.
  //   * `$make_array(...)` is the analyzer's representation of a
  //     non-const ARRAY literal (`[a, col, b]`); DuckDB shares the
  //     bracket-literal syntax so we emit it directly rather than
  //     going through a `kDuckdbNative` entry that would render as
  //     `$MAKE_ARRAY(...)`.
  // Anything outside the table surfaces UNIMPLEMENTED via the
  // empty-string contract; the LOG(INFO) records the miss so debug
  // builds can audit which functions still need a disposition row.
  if (node == nullptr || node->function() == nullptr) return "";
  // User-defined SQL functions: the analyzer stores the resolved body on
  // function_call_info (TemplatedSQLFunction) or on the SQLFunction object.
  if (const std::shared_ptr<::googlesql::ResolvedFunctionCallInfo>& info =
          node->function_call_info();
      info != nullptr) {
    if (const auto* templated =
            dynamic_cast<const ::googlesql::TemplatedSQLFunctionCall*>(
                info.get());
        templated != nullptr && templated->expr() != nullptr) {
      std::string body = EmitExpr(templated->expr());
      if (!body.empty()) return body;
      return "";
    }
  }
  if (const auto* sql_fn =
          dynamic_cast<const ::googlesql::SQLFunction*>(node->function());
      sql_fn != nullptr) {
    const ::googlesql::ResolvedExpr* body_expr = sql_fn->FunctionExpression();
    if (body_expr != nullptr) {
      std::string body = EmitExpr(body_expr);
      if (!body.empty()) return body;
      return "";
    }
  }
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    LOG(INFO) << "duckdb transpiler: SAFE function call surfaces "
                 "UNIMPLEMENTED (function="
              << node->function()->Name() << ")";
    return "";
  }
  const std::string name = internal::ResolveFunctionName(node->function());
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = EmitExpr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  if (name == "$make_array") {
    return absl::StrCat("[", absl::StrJoin(args, ", "), "]");
  }
  if (name == "$case_with_value") {
    return internal::EmitCaseWithValue(args);
  }
  if (name == "$case_no_value") {
    return internal::EmitCaseNoValue(args);
  }
  // FORMAT('%T', expr) smoke for ARRAY literal checks in
  // Aggregate verify; full FORMAT lowering is deferred.
  if (name == "format" && args.size() == 2 && args[0] == "'%T'") {
    return absl::StrCat("CAST(", args[1], " AS VARCHAR)");
  }
  // BigQuery DATE(year, month, day) vs DuckDB DATE(expr) unary cast.
  if (name == "date" && args.size() == 3) {
    return absl::StrCat("make_date(CAST(",
                        args[0],
                        " AS INTEGER), CAST(",
                        args[1],
                        " AS INTEGER), CAST(",
                        args[2],
                        " AS INTEGER))");
  }
  // BigQuery TIMESTAMP(string) is a cast; DuckDB's TIMESTAMP() is not a
  // unary string parser (syntax error at string literal).
  if (name == "timestamp" && args.size() == 1) {
    return absl::StrCat("CAST(", args[0], " AS TIMESTAMPTZ)");
  }
  if (name == "generate_array") {
    if (args.size() == 2) {
      return absl::StrCat("list_transform(generate_series(",
                          args[0],
                          ", ",
                          args[1],
                          ", 1), x -> CAST(x AS BIGINT))");
    }
    if (args.size() == 3) {
      return absl::StrCat("list_transform(generate_series(",
                          args[0],
                          ", ",
                          args[1],
                          ", ",
                          args[2],
                          "), x -> CAST(x AS BIGINT))");
    }
    return "";
  }
  if (name == "array_to_string" && args.size() == 3) {
    return absl::StrCat("array_to_string(list_transform(",
                        args[0],
                        ", e -> coalesce(CAST(e AS VARCHAR), ",
                        args[2],
                        ")), ",
                        args[1],
                        ")");
  }
  if (name == "byte_length" && !args.empty()) {
    const ::googlesql::ResolvedExpr* arg0 =
        node->argument_list_size() > 0 ? node->argument_list(0) : nullptr;
    if (arg0 != nullptr && arg0->type() != nullptr &&
        arg0->type()->kind() == ::googlesql::TYPE_STRING) {
      return absl::StrCat("OCTET_LENGTH(ENCODE(", args[0], "))");
    }
  }
  // Window/CTE queries keep `duckdb_native` while `format_timestamp` stays
  // `status=planned` in functions.yaml (no route promotion). Emit POSIX
  // strftime for literal '%X' (time-only) so CTE bodies lower.
  if (name == "format_timestamp" && args.size() >= 2) {
    if (std::optional<std::string> fmt =
            internal::TryLiteralString(node->argument_list(0));
        fmt.has_value() && *fmt == "%X") {
      if (args.size() == 3) {
        return absl::StrCat(
            "strftime('%H:%M:%S', (", args[1], ") AT TIME ZONE ", args[2], ")");
      }
      return absl::StrCat("strftime('%H:%M:%S', ", args[1], ")");
    }
  }
  if (name == "lpad") {
    const ::googlesql::ResolvedExpr* arg0 =
        node->argument_list_size() > 0 ? node->argument_list(0) : nullptr;
    const bool is_bytes = arg0 != nullptr && arg0->type() != nullptr &&
                          arg0->type()->kind() == ::googlesql::TYPE_BYTES;
    if (is_bytes) {
      const std::string val = absl::StrCat("CAST(", args[0], " AS BLOB)");
      if (args.size() == 2) {
        return absl::StrCat(
            "CAST((CASE WHEN octet_length(",
            val,
            ") >= CAST(",
            args[1],
            " AS INTEGER) THEN CAST(array_slice(",
            val,
            ", 1, CAST(",
            args[1],
            " AS INTEGER)) AS BLOB) ELSE CAST(concat(repeat('\\x20'::BLOB, "
            "greatest(0, CAST(",
            args[1],
            " AS INTEGER) - octet_length(",
            val,
            "))), ",
            val,
            ") AS BLOB) END) AS BLOB)");
      }
      if (args.size() == 3) {
        const std::string pat = absl::StrCat("CAST(", args[2], " AS BLOB)");
        return absl::StrCat(
            "bq_lpad_bytes(", val, ", ", args[1], ", ", pat, ")");
      }
      return "";
    }
    if (args.size() == 2) {
      return absl::StrCat("LPAD(", args[0], ", ", args[1], ", ' ')");
    }
  }
  if (name == "rpad") {
    const ::googlesql::ResolvedExpr* arg0 =
        node->argument_list_size() > 0 ? node->argument_list(0) : nullptr;
    const bool is_bytes = arg0 != nullptr && arg0->type() != nullptr &&
                          arg0->type()->kind() == ::googlesql::TYPE_BYTES;
    if (is_bytes) {
      const std::string val = absl::StrCat("CAST(", args[0], " AS BLOB)");
      if (args.size() == 2) {
        return absl::StrCat("CAST((CASE WHEN octet_length(",
                            val,
                            ") >= CAST(",
                            args[1],
                            " AS INTEGER) THEN CAST(array_slice(",
                            val,
                            ", 1, CAST(",
                            args[1],
                            " AS INTEGER)) AS BLOB) ELSE CAST(concat(",
                            val,
                            ", repeat('\\x20'::BLOB, greatest(0, CAST(",
                            args[1],
                            " AS INTEGER) - octet_length(",
                            val,
                            ")))) AS BLOB) END) AS BLOB)");
      }
      if (args.size() == 3) {
        const std::string pat = absl::StrCat("CAST(", args[2], " AS BLOB)");
        return absl::StrCat(
            "bq_rpad_bytes(", val, ", ", args[1], ", ", pat, ")");
      }
      return "";
    }
    if (args.size() == 2) {
      return absl::StrCat("RPAD(", args[0], ", ", args[1], ", ' ')");
    }
    if (args.size() == 3) {
      return absl::StrCat("RPAD(", args[0], ", ", args[1], ", ", args[2], ")");
    }
  }
  if (name == "right" && !args.empty()) {
    const ::googlesql::ResolvedExpr* arg0 =
        node->argument_list_size() > 0 ? node->argument_list(0) : nullptr;
    if (arg0 != nullptr && arg0->type() != nullptr &&
        arg0->type()->kind() == ::googlesql::TYPE_BYTES && args.size() == 2) {
      const std::string val = absl::StrCat("CAST(", args[0], " AS BLOB)");
      return absl::StrCat("CAST(array_slice(",
                          val,
                          ", greatest(1, octet_length(",
                          val,
                          ") - CAST(",
                          args[1],
                          " AS INTEGER) + 1), octet_length(",
                          val,
                          ")) AS BLOB)");
    }
  }
  if (name == "$subscript" && args.size() == 2) {
    const ::googlesql::Type* ret = node->type();
    const bool json_result = ret != nullptr && ret->IsJson();
    const ::googlesql::ResolvedExpr* base_expr = node->argument_list(0);
    const ::googlesql::ResolvedExpr* idx_expr = node->argument_list(1);
    const bool json_base = base_expr != nullptr &&
                           base_expr->type() != nullptr &&
                           base_expr->type()->IsJson();
    if (idx_expr != nullptr && idx_expr->type() != nullptr) {
      const auto idx_kind = idx_expr->type()->kind();
      if (idx_kind == ::googlesql::TYPE_INT64 ||
          idx_kind == ::googlesql::TYPE_UINT64 ||
          idx_kind == ::googlesql::TYPE_INT32) {
        if (json_base || json_result) {
          return absl::StrCat("json_extract(", args[0], ", ", args[1], ")");
        }
        return absl::StrCat(
            "list_extract(", args[0], ", CAST(", args[1], " AS BIGINT) + 1)");
      }
      if (idx_kind == ::googlesql::TYPE_STRING) {
        if (json_base || json_result) {
          if (json_result) {
            return absl::StrCat("(", args[0], " -> ", args[1], ")");
          }
          return absl::StrCat("(", args[0], " ->> ", args[1], ")");
        }
      }
    }
    if (json_base) {
      return json_result
                 ? absl::StrCat("json_extract(", args[0], ", ", args[1], ")")
                 : absl::StrCat("(", args[0], " ->> ", args[1], ")");
    }
  }
  const auto* entry = LookupFunction(name);
  if (entry == nullptr) {
    LOG(INFO) << "duckdb transpiler: function '" << name
              << "' has no disposition; surfacing UNIMPLEMENTED";
    return "";
  }
  if (std::string infix = TryEmitInternalOperator(name, entry, args);
      !infix.empty()) {
    return infix;
  }
  switch (entry->disposition) {
    case Disposition::kDuckdbNative:
    case Disposition::kDuckdbRewrite:
      return absl::StrCat(
          entry->duckdb_name, "(", absl::StrJoin(args, ", "), ")");
    case Disposition::kDuckdbUdf:
      // Ready `duckdb_udf` rows lower identically to `duckdb_native`:
      // the YAML row's `duckdb_name=` field carries the registered
      // BigQuery polyfill UDF / macro name (installed via
      // `backend/engine/duckdb/udf::RegisterAll(conn)` on every
      // executor-opened connection), and the UDF body owns the
      // BigQuery semantic gap. `status=planned` rows still surface
      // UNIMPLEMENTED (no wrapper installed yet); the YAML
      // generator enforces "ready row has duckdb_name, planned row
      // doesn't" at build time.
      if (!entry->planned && !entry->duckdb_name.empty()) {
        return absl::StrCat(
            entry->duckdb_name, "(", absl::StrJoin(args, ", "), ")");
      }
      LOG(INFO) << "duckdb transpiler: function '" << name
                << "' route=duckdb_udf (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kSemanticExecutor:
    case Disposition::kControlOp:
    case Disposition::kLocalStub:
      // `kLocalStub` (e.g. `KEYS.NEW_KEYSET`) is handled by the
      // semantic executor's per-family stub dispatch (see
      // `backend/engine/semantic/stubs/`); the DuckDB transpiler
      // does not lower stub families. The route classifier
      // promotes the surrounding query to `kLocalStub` (or
      // `kSemanticExecutor` depending on what else is in the
      // statement), so this branch is reached only when the
      // transpiler is asked to lower a stub call inline (which
      // it cannot do). Surfacing the empty string keeps the
      // no-silent-approximation contract intact.
      LOG(INFO) << "duckdb transpiler: function '" << name
                << "' route=" << DispositionToString(entry->disposition)
                << " (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kUnsupported:
      LOG(INFO) << "duckdb transpiler: function '" << name
                << "' unsupported; surfacing UNIMPLEMENTED";
      return "";
  }
  return "";
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
