#include "backend/engine/semantic/functions/json_funcs.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/string_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/json.h"
#include "googlesql/public/json_value.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

using ::googlesql::JSONValue;
using ::googlesql::JSONValueConstRef;
using ::googlesql::ProductMode;
using ::googlesql::Value;
using ::googlesql::functions::ConvertJsonToBool;
using ::googlesql::functions::ConvertJsonToDouble;
using ::googlesql::functions::ConvertJsonToInt64;
using ::googlesql::functions::ConvertJsonToString;
using ::googlesql::functions::JsonPathEvaluator;
using ::googlesql::functions::WideNumberMode;

constexpr bool kEscapingValues = true;
constexpr bool kEscapingKeys = true;

Value NullForReturnType(const ::googlesql::Type* return_type) {
  if (return_type == nullptr) return Value::NullString();
  switch (return_type->kind()) {
    case ::googlesql::TYPE_BOOL:
      return Value::NullBool();
    case ::googlesql::TYPE_INT64:
      return Value::NullInt64();
    case ::googlesql::TYPE_DOUBLE:
      return Value::NullDouble();
    case ::googlesql::TYPE_STRING:
      return Value::NullString();
    case ::googlesql::TYPE_JSON:
      return Value::NullJson();
    case ::googlesql::TYPE_ARRAY:
      return Value::Null(return_type);
    default:
      return Value::NullString();
  }
}

absl::StatusOr<std::string> JsonInputText(const Value& arg) {
  if (arg.type_kind() == ::googlesql::TYPE_JSON) {
    return arg.json_string();
  }
  if (arg.type_kind() == ::googlesql::TYPE_STRING) {
    return arg.string_value();
  }
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           "semantic: JSON function first arg must be STRING "
                           "or JSON");
}

absl::StatusOr<JSONValueConstRef> JsonRefFromArg(
    const Value& arg, std::optional<JSONValue>* parsed_storage) {
  if (arg.type_kind() == ::googlesql::TYPE_JSON) {
    if (arg.is_validated_json()) {
      return arg.json_value();
    }
    absl::string_view text =
        arg.is_unparsed_json() ? arg.json_value_unparsed() : arg.json_string();
    auto parsed = JSONValue::ParseJSONString(text);
    if (!parsed.ok()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: invalid JSON input");
    }
    *parsed_storage = std::move(*parsed);
    return parsed_storage->value().GetConstRef();
  }
  absl::string_view text;
  if (arg.type_kind() == ::googlesql::TYPE_STRING) {
    text = arg.string_value();
  } else {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: JSON function first arg must be STRING "
                             "or JSON");
  }
  auto parsed = JSONValue::ParseJSONString(text);
  if (!parsed.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: invalid JSON input");
  }
  *parsed_storage = std::move(*parsed);
  return parsed_storage->value().GetConstRef();
}

absl::StatusOr<std::string> PathFromArgs(const std::vector<Value>& args,
                                         size_t path_index,
                                         bool default_root) {
  if (args.size() <= path_index) {
    if (default_root) return "$";
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: JSON function missing path argument");
  }
  if (args[path_index].is_null()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: JSON path must not be NULL");
  }
  if (args[path_index].type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: JSON path must be STRING");
  }
  return args[path_index].string_value();
}

absl::StatusOr<std::unique_ptr<JsonPathEvaluator>> MakeEvaluator(
    absl::string_view path, bool sql_standard_mode) {
  auto evaluator = JsonPathEvaluator::Create(
      path, sql_standard_mode, kEscapingValues, kEscapingKeys);
  if (!evaluator.ok()) return evaluator.status();
  return std::move(*evaluator);
}

absl::StatusOr<Value> ExtractJsonText(const std::vector<Value>& args,
                                      const ::googlesql::Type* return_type,
                                      bool sql_standard_mode) {
  if (args.empty()) {
    return absl::InvalidArgumentError(
        "semantic: JSON extract function expects at least one argument");
  }
  if (args[0].is_null()) {
    return NullForReturnType(return_type);
  }
  auto path = PathFromArgs(args, /*path_index=*/1, /*default_root=*/false);
  if (!path.ok()) return path.status();
  if (args.size() > 1 && args[1].is_null()) {
    return NullForReturnType(return_type);
  }

  auto json_text = JsonInputText(args[0]);
  if (!json_text.ok()) return NullForReturnType(return_type);

  auto evaluator = MakeEvaluator(*path, sql_standard_mode);
  if (!evaluator.ok()) return evaluator.status();

  std::string out;
  bool is_null = false;
  absl::Status st = (*evaluator)->Extract(*json_text, &out, &is_null);
  if (!st.ok()) return st;
  if (is_null) return NullForReturnType(return_type);
  return Value::String(std::move(out));
}

absl::StatusOr<Value> ExtractScalarText(const std::vector<Value>& args,
                                        const ::googlesql::Type* return_type,
                                        bool sql_standard_mode) {
  if (args.empty()) {
    return absl::InvalidArgumentError(
        "semantic: JSON scalar extract expects at least one argument");
  }
  if (args[0].is_null()) {
    return NullForReturnType(return_type);
  }
  auto path = PathFromArgs(args, /*path_index=*/1, /*default_root=*/false);
  if (!path.ok()) return path.status();
  if (args.size() > 1 && args[1].is_null()) {
    return NullForReturnType(return_type);
  }

  auto json_text = JsonInputText(args[0]);
  if (!json_text.ok()) return NullForReturnType(return_type);

  auto evaluator = MakeEvaluator(*path, sql_standard_mode);
  if (!evaluator.ok()) return evaluator.status();

  std::string out;
  bool is_null = false;
  absl::Status st = (*evaluator)->ExtractScalar(*json_text, &out, &is_null);
  if (!st.ok()) return st;
  if (is_null) return NullForReturnType(return_type);
  return Value::String(std::move(out));
}

absl::StatusOr<Value> ExtractArrayValues(const std::vector<Value>& args,
                                         const ::googlesql::Type* return_type,
                                         bool sql_standard_mode,
                                         bool unquote_strings) {
  if (args.empty()) {
    return absl::InvalidArgumentError(
        "semantic: JSON_*_ARRAY expects at least one argument");
  }
  if (args[0].is_null()) {
    return NullForReturnType(return_type);
  }
  auto path = PathFromArgs(args, /*path_index=*/1, /*default_root=*/true);
  if (!path.ok()) return path.status();
  if (args.size() > 1 && args[1].is_null()) {
    return NullForReturnType(return_type);
  }

  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                       : nullptr;
  if (arr_type == nullptr) {
    return absl::InternalError(
        "semantic: JSON_*_ARRAY missing ARRAY return type");
  }
  const ::googlesql::Type* elem_type = arr_type->element_type();
  const bool elem_is_json =
      elem_type != nullptr && elem_type->kind() == ::googlesql::TYPE_JSON;

  auto make_elem = [&](std::string text) -> Value {
    if (elem_is_json) {
      return Value::UnvalidatedJsonString(std::move(text));
    }
    return Value::String(std::move(text));
  };

  auto json_text = JsonInputText(args[0]);
  if (!json_text.ok()) return NullForReturnType(return_type);

  auto evaluator = MakeEvaluator(*path, sql_standard_mode);
  if (!evaluator.ok()) return evaluator.status();

  std::vector<Value> values;
  if (unquote_strings) {
    std::vector<std::optional<std::string>> elems;
    bool is_null = false;
    absl::Status st =
        (*evaluator)->ExtractStringArray(*json_text, &elems, &is_null);
    if (!st.ok()) return st;
    if (is_null) return NullForReturnType(return_type);
    values.reserve(elems.size());
    for (const auto& elem : elems) {
      if (!elem.has_value()) {
        values.push_back(Value::NullString());
      } else {
        values.push_back(Value::String(*elem));
      }
    }
  } else {
    std::vector<std::string> elems;
    bool is_null = false;
    absl::Status st = (*evaluator)->ExtractArray(*json_text, &elems, &is_null);
    if (!st.ok()) return st;
    if (is_null) return NullForReturnType(return_type);
    values.reserve(elems.size());
    for (std::string& elem : elems) {
      values.push_back(make_elem(std::move(elem)));
    }
  }
  return Value::Array(arr_type->AsArray(), std::move(values));
}

}  // namespace

absl::StatusOr<Value> JsonExtract(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  return ExtractJsonText(args, return_type, /*sql_standard_mode=*/false);
}

absl::StatusOr<Value> JsonQuery(const std::vector<Value>& args,
                                const ::googlesql::Type* return_type) {
  return ExtractJsonText(args, return_type, /*sql_standard_mode=*/true);
}

absl::StatusOr<Value> JsonValue(const std::vector<Value>& args,
                                const ::googlesql::Type* return_type) {
  return ExtractScalarText(args, return_type, /*sql_standard_mode=*/true);
}

absl::StatusOr<Value> JsonExtractScalar(const std::vector<Value>& args,
                                        const ::googlesql::Type* return_type) {
  return ExtractScalarText(args, return_type, /*sql_standard_mode=*/false);
}

absl::StatusOr<Value> JsonExtractArray(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type) {
  return ExtractArrayValues(args,
                            return_type,
                            /*sql_standard_mode=*/false,
                            /*unquote_strings=*/false);
}

absl::StatusOr<Value> JsonQueryArray(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type) {
  return ExtractArrayValues(args,
                            return_type,
                            /*sql_standard_mode=*/true,
                            /*unquote_strings=*/false);
}

absl::StatusOr<Value> JsonExtractStringArray(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  return ExtractArrayValues(args,
                            return_type,
                            /*sql_standard_mode=*/false,
                            /*unquote_strings=*/true);
}

absl::StatusOr<Value> JsonValueArray(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type) {
  return ExtractArrayValues(args,
                            return_type,
                            /*sql_standard_mode=*/true,
                            /*unquote_strings=*/true);
}

absl::StatusOr<Value> ParseJson(const std::vector<Value>& args) {
  if (args.empty() || args[0].is_null()) {
    return Value::NullJson();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: PARSE_JSON expects STRING input");
  }
  auto parsed = JSONValue::ParseJSONString(args[0].string_value());
  if (!parsed.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: PARSE_JSON failed to parse input");
  }
  return Value::UnvalidatedJsonString(parsed->GetConstRef().ToString());
}

absl::StatusOr<Value> ToJsonString(const std::vector<Value>& args) {
  return ToJson(args);
}

absl::StatusOr<Value> JsonCastBool(const std::vector<Value>& args) {
  if (args.empty() || args[0].is_null()) return Value::NullBool();
  if (args[0].type_kind() == ::googlesql::TYPE_BOOL) return args[0];
  if (args[0].type_kind() != ::googlesql::TYPE_JSON) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: BOOL cast from this type is not yet implemented");
  }
  std::optional<JSONValue> storage;
  auto ref = JsonRefFromArg(args[0], &storage);
  if (!ref.ok()) return ref.status();
  auto converted = ConvertJsonToBool(*ref);
  if (!converted.ok()) return converted.status();
  return Value::Bool(*converted);
}

absl::StatusOr<Value> JsonCastInt64(const std::vector<Value>& args) {
  if (args.empty() || args[0].is_null()) return Value::NullInt64();
  if (args[0].type_kind() == ::googlesql::TYPE_INT64) return args[0];
  if (args[0].type_kind() != ::googlesql::TYPE_JSON) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: INT64 cast from this type is not yet implemented");
  }
  std::optional<JSONValue> storage;
  auto ref = JsonRefFromArg(args[0], &storage);
  if (!ref.ok()) return ref.status();
  auto converted = ConvertJsonToInt64(*ref);
  if (!converted.ok()) return converted.status();
  return Value::Int64(*converted);
}

absl::StatusOr<Value> JsonCastFloat64(const std::vector<Value>& args) {
  if (args.empty() || args[0].is_null()) return Value::NullDouble();
  if (args[0].type_kind() == ::googlesql::TYPE_DOUBLE) return args[0];
  if (args[0].type_kind() != ::googlesql::TYPE_JSON) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: FLOAT64 cast from this type is not yet implemented");
  }
  std::optional<JSONValue> storage;
  auto ref = JsonRefFromArg(args[0], &storage);
  if (!ref.ok()) return ref.status();
  auto converted = ConvertJsonToDouble(
      *ref, WideNumberMode::kRound, ProductMode::PRODUCT_EXTERNAL);
  if (!converted.ok()) return converted.status();
  return Value::Double(*converted);
}

absl::StatusOr<Value> JsonCastString(const std::vector<Value>& args) {
  if (args.empty() || args[0].is_null()) return Value::NullString();
  if (args[0].type_kind() == ::googlesql::TYPE_STRING) return args[0];
  if (args[0].type_kind() != ::googlesql::TYPE_JSON) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: STRING cast from this type is not yet implemented");
  }
  std::optional<JSONValue> storage;
  auto ref = JsonRefFromArg(args[0], &storage);
  if (!ref.ok()) return ref.status();
  auto converted = ConvertJsonToString(*ref);
  if (!converted.ok()) return converted.status();
  return Value::String(*converted);
}

absl::StatusOr<Value> JsonRefToSemanticValue(
    JSONValueConstRef ref, const ::googlesql::Type* return_type) {
  if (ref.IsNull()) return NullForReturnType(return_type);
  if (return_type != nullptr && return_type->IsJson()) {
    return Value::UnvalidatedJsonString(ref.ToString());
  }
  if (return_type != nullptr &&
      return_type->kind() == ::googlesql::TYPE_STRING && ref.IsString()) {
    return Value::String(ref.GetString());
  }
  return Value::String(ref.ToString());
}

absl::StatusOr<Value> JsonGetField(const Value& base,
                                   absl::string_view field_name,
                                   const ::googlesql::Type* return_type) {
  if (base.is_null()) return NullForReturnType(return_type);
  std::optional<JSONValue> storage;
  auto ref = JsonRefFromArg(base, &storage);
  if (!ref.ok()) return ref.status();
  if (!ref->IsObject()) return NullForReturnType(return_type);
  auto member = ref->GetMemberIfExists(field_name);
  if (!member.has_value()) return NullForReturnType(return_type);
  return JsonRefToSemanticValue(*member, return_type);
}

absl::StatusOr<Value> JsonSubscript(const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: $subscript expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return NullForReturnType(return_type);
  }
  std::optional<JSONValue> storage;
  auto ref = JsonRefFromArg(args[0], &storage);
  if (!ref.ok()) return ref.status();
  if (args[1].type_kind() == ::googlesql::TYPE_INT64) {
    if (!ref->IsArray()) return NullForReturnType(return_type);
    const int64_t idx = args[1].int64_value();
    if (idx < 0 || static_cast<size_t>(idx) >= ref->GetArraySize()) {
      return NullForReturnType(return_type);
    }
    return JsonRefToSemanticValue(
        ref->GetArrayElement(static_cast<size_t>(idx)), return_type);
  }
  if (args[1].type_kind() == ::googlesql::TYPE_STRING) {
    if (!ref->IsObject()) return NullForReturnType(return_type);
    auto member = ref->GetMemberIfExists(args[1].string_value());
    if (!member.has_value()) return NullForReturnType(return_type);
    return JsonRefToSemanticValue(*member, return_type);
  }
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      "semantic: $subscript index must be INT64 or STRING");
}

absl::StatusOr<Value> JsonTypeFunc(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: JSON_TYPE expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  std::optional<JSONValue> storage;
  auto ref = JsonRefFromArg(args[0], &storage);
  if (!ref.ok()) return ref.status();
  if (ref->IsNull()) return Value::String("null");
  if (ref->IsString()) return Value::String("string");
  if (ref->IsNumber()) return Value::String("number");
  if (ref->IsBoolean()) return Value::String("boolean");
  if (ref->IsObject()) return Value::String("object");
  if (ref->IsArray()) return Value::String("array");
  return Value::NullString();
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
