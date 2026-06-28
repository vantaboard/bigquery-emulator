#include <algorithm>
#include <functional>
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
#include "backend/engine/duckdb/transpiler/transpiler_emit_datetime.h"
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

namespace {

using EmitExprFn = std::function<std::string(const ::googlesql::ResolvedExpr*)>;

std::string TryEmitInternalOperator(absl::string_view name,
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
  if (name == "$interval") {
    return "";
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

// Returns true when the node is a SQL UDF inline body; `out` holds the emit
// result (possibly empty, which still short-circuits normal dispatch).
bool TryEmitSqlUdfPath(const ::googlesql::ResolvedFunctionCall* node,
                       const EmitExprFn& emit_expr,
                       std::string* out) {
  if (const std::shared_ptr<::googlesql::ResolvedFunctionCallInfo>& info =
          node->function_call_info();
      info != nullptr) {
    if (const auto* templated =
            dynamic_cast<const ::googlesql::TemplatedSQLFunctionCall*>(
                info.get());
        templated != nullptr && templated->expr() != nullptr) {
      *out = emit_expr(templated->expr());
      return true;
    }
  }
  if (const auto* sql_fn =
          dynamic_cast<const ::googlesql::SQLFunction*>(node->function());
      sql_fn != nullptr) {
    const ::googlesql::ResolvedExpr* body_expr = sql_fn->FunctionExpression();
    if (body_expr != nullptr) {
      *out = emit_expr(body_expr);
      return true;
    }
  }
  return false;
}

std::string TryEmitGenerateArray(absl::string_view name,
                                 const std::vector<std::string>& args) {
  if (name != "generate_array") return "";
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

std::string TryEmitFormatTimestamp(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const std::vector<std::string>& args) {
  if (name != "format_timestamp" || args.size() < 2) return "";
  if (std::optional<std::string> fmt =
          internal::TryLiteralString(node->argument_list(0));
      fmt.has_value() && *fmt == "%X") {
    if (args.size() == 3) {
      return absl::StrCat(
          "strftime('%H:%M:%S', (", args[1], ") AT TIME ZONE ", args[2], ")");
    }
    return absl::StrCat("strftime('%H:%M:%S', ", args[1], ")");
  }
  return "";
}

std::string TryEmitPadFunction(absl::string_view name,
                               const ::googlesql::ResolvedFunctionCall* node,
                               const std::vector<std::string>& args) {
  if (name != "lpad" && name != "rpad") return "";
  const ::googlesql::ResolvedExpr* arg0 =
      node->argument_list_size() > 0 ? node->argument_list(0) : nullptr;
  const bool is_bytes = arg0 != nullptr && arg0->type() != nullptr &&
                        arg0->type()->kind() == ::googlesql::TYPE_BYTES;
  if (!is_bytes) {
    if (name == "lpad" && args.size() == 2) {
      return absl::StrCat("LPAD(", args[0], ", ", args[1], ", ' ')");
    }
    if (name == "rpad") {
      if (args.size() == 2) {
        return absl::StrCat("RPAD(", args[0], ", ", args[1], ", ' ')");
      }
      if (args.size() == 3) {
        return absl::StrCat(
            "RPAD(", args[0], ", ", args[1], ", ", args[2], ")");
      }
    }
    return "";
  }
  const std::string val = absl::StrCat("CAST(", args[0], " AS BLOB)");
  if (name == "lpad") {
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
      return absl::StrCat("bq_lpad_bytes(", val, ", ", args[1], ", ", pat, ")");
    }
    return "";
  }
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
    return absl::StrCat("bq_rpad_bytes(", val, ", ", args[1], ", ", pat, ")");
  }
  return "";
}

std::string TryEmitSubscript(const ::googlesql::ResolvedFunctionCall* node,
                             const std::vector<std::string>& args) {
  if (args.size() != 2) return "";
  const ::googlesql::Type* ret = node->type();
  const bool json_result = ret != nullptr && ret->IsJson();
  const ::googlesql::ResolvedExpr* base_expr = node->argument_list(0);
  const ::googlesql::ResolvedExpr* idx_expr = node->argument_list(1);
  const bool json_base = base_expr != nullptr && base_expr->type() != nullptr &&
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
    if (idx_kind == ::googlesql::TYPE_STRING && (json_base || json_result)) {
      if (json_result) {
        return absl::StrCat("(", args[0], " -> ", args[1], ")");
      }
      return absl::StrCat("(", args[0], " ->> ", args[1], ")");
    }
  }
  if (json_base) {
    return json_result
               ? absl::StrCat("json_extract(", args[0], ", ", args[1], ")")
               : absl::StrCat("(", args[0], " ->> ", args[1], ")");
  }
  return "";
}

std::optional<std::string> TryEmitLiteralBuiltinSpecial(
    absl::string_view name, const std::vector<std::string>& args) {
  if (name == "$make_array") {
    return absl::StrCat("[", absl::StrJoin(args, ", "), "]");
  }
  if (name == "$case_with_value") {
    return internal::EmitCaseWithValue(args);
  }
  if (name == "$case_no_value") {
    return internal::EmitCaseNoValue(args);
  }
  if (name == "format" && args.size() == 2 && args[0] == "'%T'") {
    return absl::StrCat("CAST(", args[1], " AS VARCHAR)");
  }
  return std::nullopt;
}

std::optional<std::string> TryEmitScalarBuiltinSpecial(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const std::vector<std::string>& args) {
  if (name == "date" && args.size() == 3) {
    return absl::StrCat("make_date(CAST(",
                        args[0],
                        " AS INTEGER), CAST(",
                        args[1],
                        " AS INTEGER), CAST(",
                        args[2],
                        " AS INTEGER))");
  }
  if (name == "timestamp" && args.size() == 1) {
    return absl::StrCat("CAST(", args[0], " AS TIMESTAMPTZ)");
  }
  if (name == "generate_array") {
    if (std::string gen = TryEmitGenerateArray(name, args); !gen.empty()) {
      return gen;
    }
    return std::string();
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
  if (name == "sqrt" && node->argument_list_size() > 0) {
    const ::googlesql::ResolvedExpr* arg0 = node->argument_list(0);
    if (arg0 != nullptr && arg0->type() != nullptr) {
      const auto arg_kind = arg0->type()->kind();
      if (arg_kind == ::googlesql::TYPE_NUMERIC ||
          arg_kind == ::googlesql::TYPE_BIGNUMERIC) {
        LOG(INFO) << "duckdb transpiler: SQRT(NUMERIC) routes to "
                     "semantic_executor";
        return std::string();
      }
    }
  }
  return std::nullopt;
}

std::optional<std::string> TryEmitCoreBuiltinSpecial(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const std::vector<std::string>& args) {
  if (auto literal = TryEmitLiteralBuiltinSpecial(name, args);
      literal.has_value()) {
    return literal;
  }
  return TryEmitScalarBuiltinSpecial(name, node, args);
}

std::optional<std::string> TryEmitStringBuiltinSpecial(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const std::vector<std::string>& args) {
  if (std::string fmt = TryEmitFormatTimestamp(name, node, args);
      !fmt.empty()) {
    return fmt;
  }
  if (name == "lpad" || name == "rpad") {
    if (std::string pad = TryEmitPadFunction(name, node, args); !pad.empty()) {
      return pad;
    }
    const ::googlesql::ResolvedExpr* arg0 =
        node->argument_list_size() > 0 ? node->argument_list(0) : nullptr;
    const bool is_bytes = arg0 != nullptr && arg0->type() != nullptr &&
                          arg0->type()->kind() == ::googlesql::TYPE_BYTES;
    if (is_bytes || name == "lpad") {
      return std::string();
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
  if (name == "$subscript") {
    if (std::string sub = TryEmitSubscript(node, args); !sub.empty()) {
      return sub;
    }
    return std::nullopt;
  }
  return std::nullopt;
}

std::optional<std::string> TryEmitBuiltinSpecialCase(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const std::vector<std::string>& args) {
  if (auto core = TryEmitCoreBuiltinSpecial(name, node, args);
      core.has_value()) {
    return core;
  }
  return TryEmitStringBuiltinSpecial(name, node, args);
}

std::string EmitDispositionFunctionCall(absl::string_view name,
                                        const std::vector<std::string>& args,
                                        const FnEntry* entry) {
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

}  // namespace

std::string Transpiler::EmitFunctionCall(
    const ::googlesql::ResolvedFunctionCall* node) {
  if (node == nullptr || node->function() == nullptr) return "";

  const EmitExprFn emit_expr = [this](const ::googlesql::ResolvedExpr* expr) {
    return EmitExpr(expr);
  };

  if (std::string udf_body; TryEmitSqlUdfPath(node, emit_expr, &udf_body)) {
    return udf_body;
  }

  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    LOG(INFO) << "duckdb transpiler: SAFE function call surfaces "
                 "UNIMPLEMENTED (function="
              << node->function()->Name() << ")";
    return "";
  }
  const std::string name = internal::ResolveFunctionName(node->function());
  if (auto dt = internal::TryEmitDateTimeFunctionCall(name, node, emit_expr);
      dt.has_value()) {
    return *dt;
  }
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = emit_expr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  if (auto special = TryEmitBuiltinSpecialCase(name, node, args);
      special.has_value()) {
    return *special;
  }
  return EmitDispositionFunctionCall(name, args, LookupFunction(name));
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
