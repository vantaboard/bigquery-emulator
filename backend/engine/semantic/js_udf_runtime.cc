
#include <cmath>

#include "duktape.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

absl::Status JsUdfError(absl::string_view message) {
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      absl::StrCat("User-defined function error: ", message));
}

void PushGooglesqlValueToJs(duk_context* ctx,
                            const Value& value,
                            ::googlesql::TypeKind type_kind) {
  if (value.is_null()) {
    duk_push_null(ctx);
    return;
  }
  switch (type_kind) {
    case ::googlesql::TYPE_BOOL:
      duk_push_boolean(ctx, value.bool_value() ? 1 : 0);
      return;
    case ::googlesql::TYPE_INT64:
      duk_push_number(ctx, static_cast<double>(value.int64_value()));
      return;
    case ::googlesql::TYPE_DOUBLE:
      duk_push_number(ctx, value.double_value());
      return;
    case ::googlesql::TYPE_STRING:
      duk_push_string(ctx, value.string_value().c_str());
      return;
    default:
      duk_push_undefined(ctx);
      return;
  }
}

absl::StatusOr<Value> PopJsValueToGooglesql(
    duk_context* ctx, int stack_index, const ::googlesql::Type* return_type) {
  if (return_type == nullptr) {
    return absl::InternalError("js_udf_runtime: missing return type");
  }
  if (duk_is_null(ctx, stack_index) || duk_is_undefined(ctx, stack_index)) {
    return Value::Null(return_type);
  }
  switch (return_type->kind()) {
    case ::googlesql::TYPE_BOOL: {
      if (!duk_is_boolean(ctx, stack_index)) {
        return JsUdfError("JavaScript UDF must return BOOL");
      }
      return Value::Bool(duk_get_boolean(ctx, stack_index) != 0);
    }
    case ::googlesql::TYPE_INT64: {
      if (duk_is_number(ctx, stack_index)) {
        const double num = duk_get_number(ctx, stack_index);
        if (!std::isfinite(num) ||
            num < static_cast<double>(std::numeric_limits<int64_t>::min()) ||
            num > static_cast<double>(std::numeric_limits<int64_t>::max())) {
          return JsUdfError("JavaScript UDF INT64 return is out of range");
        }
        return Value::Int64(static_cast<int64_t>(num));
      }
      if (duk_is_string(ctx, stack_index)) {
        int64_t out = 0;
        const char* text = duk_get_string(ctx, stack_index);
        if (text == nullptr || !absl::SimpleAtoi(text, &out)) {
          return JsUdfError(
              "JavaScript UDF INT64 return is not a valid integer");
        }
        return Value::Int64(out);
      }
      return JsUdfError("JavaScript UDF must return INT64 as Number or String");
    }
    case ::googlesql::TYPE_DOUBLE: {
      if (!duk_is_number(ctx, stack_index)) {
        return JsUdfError("JavaScript UDF must return FLOAT64");
      }
      return Value::Double(duk_get_number(ctx, stack_index));
    }
    case ::googlesql::TYPE_STRING: {
      const char* text = duk_to_string(ctx, stack_index);
      if (text == nullptr) {
        return JsUdfError("JavaScript UDF must return STRING");
      }
      return Value::String(text);
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("JavaScript UDF return type ",
                       ::googlesql::TypeKind_Name(return_type->kind()),
                       " is not supported"));
  }
}

}  // namespace

absl::StatusOr<Value> EvalJsUdfCall(
    const catalog::JsUdfDefinition& definition,
    const std::vector<Value>& arg_values,
    const ::googlesql::Type* return_type,
    const std::vector<const ::googlesql::Type*>& arg_types) {
  if (definition.is_aggregate) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "JavaScript aggregate UDF call-time evaluation is not implemented");
  }
  if (definition.arg_names.size() != arg_values.size()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "semantic: JavaScript UDF argument count mismatch (expected ",
        definition.arg_names.size(),
        ", got ",
        arg_values.size(),
        ")"));
  }

  duk_context* duk_ctx = duk_create_heap_default();
  if (duk_ctx == nullptr) {
    return absl::ResourceExhaustedError(
        "js_udf_runtime: failed to allocate Duktape heap");
  }

  const std::string arg_list = absl::StrJoin(definition.arg_names, ",");
  duk_push_global_object(duk_ctx);
  duk_get_prop_string(duk_ctx, -1, "Function");
  duk_remove(duk_ctx, -2);
  duk_push_string(duk_ctx, arg_list.c_str());
  duk_push_string(duk_ctx, definition.js_body.c_str());
  if (duk_pcall(duk_ctx, 2) != 0) {
    const char* err = duk_safe_to_string(duk_ctx, -1);
    duk_destroy_heap(duk_ctx);
    return JsUdfError(err != nullptr ? err
                                     : "failed to construct JavaScript UDF");
  }

  for (size_t i = 0; i < arg_values.size(); ++i) {
    const ::googlesql::TypeKind type_kind = i < definition.arg_type_kinds.size()
                                                ? definition.arg_type_kinds[i]
                                                : ::googlesql::TYPE_UNKNOWN;
    const ::googlesql::Type* arg_type =
        i < arg_types.size() ? arg_types[i] : nullptr;
    const ::googlesql::TypeKind effective_kind =
        type_kind != ::googlesql::TYPE_UNKNOWN
            ? type_kind
            : (arg_type != nullptr ? arg_type->kind()
                                   : ::googlesql::TYPE_UNKNOWN);
    PushGooglesqlValueToJs(duk_ctx, arg_values[i], effective_kind);
  }

  if (duk_pcall(duk_ctx, static_cast<int>(arg_values.size())) != 0) {
    const char* err = duk_safe_to_string(duk_ctx, -1);
    duk_destroy_heap(duk_ctx);
    return JsUdfError(err != nullptr ? err : "JavaScript UDF failed");
  }

  absl::StatusOr<Value> result =
      PopJsValueToGooglesql(duk_ctx, -1, return_type);
  duk_destroy_heap(duk_ctx);
  return result;
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
