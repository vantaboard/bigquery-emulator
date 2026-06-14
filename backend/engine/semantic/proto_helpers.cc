#include "backend/engine/semantic/proto_helpers.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/cord.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "backend/engine/semantic/error.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/reflection.h"
#include "googlesql/public/types/proto_type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace proto_helpers {

namespace {

using ::googlesql::Value;

absl::StatusOr<Value> ScalarFromReflection(
    const google::protobuf::Message& msg,
    const google::protobuf::FieldDescriptor* field,
    const ::googlesql::Type* target_type) {
  const google::protobuf::Reflection* reflection = msg.GetReflection();
  switch (field->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      return Value::Int64(reflection->GetInt32(msg, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      return Value::Int64(reflection->GetInt64(msg, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
      return Value::Int64(static_cast<int64_t>(reflection->GetUInt32(msg, field)));
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
      return Value::Int64(static_cast<int64_t>(reflection->GetUInt64(msg, field)));
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
      return Value::Double(reflection->GetDouble(msg, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
      return Value::Double(reflection->GetFloat(msg, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
      return Value::Bool(reflection->GetBool(msg, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
      return Value::String(reflection->GetString(msg, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
      return Value::Int64(reflection->GetEnum(msg, field)->number());
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
      if (target_type == nullptr || !target_type->IsProto()) {
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            "semantic: nested proto field read requires PROTO target type");
      }
      const auto* proto_type = target_type->AsProto();
      if (proto_type == nullptr) {
        return absl::InternalError(
            "semantic: PROTO target type missing ProtoType payload");
      }
      const google::protobuf::Message& nested =
          reflection->GetMessage(msg, field);
      return Value::Proto(proto_type,
                          absl::Cord(nested.SerializeAsString()));
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: unsupported proto field cpp_type ",
                       field->CppTypeName(field->cpp_type())));
  }
}

}  // namespace

absl::Status SetScalarProtoField(google::protobuf::Message* msg,
                                 const google::protobuf::FieldDescriptor* field,
                                 const Value& val) {
  const google::protobuf::Reflection* reflection = msg->GetReflection();
  if (val.is_null()) {
    reflection->ClearField(msg, field);
    return absl::OkStatus();
  }
  switch (field->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      if (val.type_kind() != ::googlesql::TYPE_INT64) {
        return absl::InvalidArgumentError(
            "semantic: proto field expects INT64-compatible value");
      }
      reflection->SetInt32(msg, field, static_cast<int32_t>(val.int64_value()));
      return absl::OkStatus();
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      if (val.type_kind() != ::googlesql::TYPE_INT64) {
        return absl::InvalidArgumentError(
            "semantic: proto field expects INT64-compatible value");
      }
      reflection->SetInt64(msg, field, val.int64_value());
      return absl::OkStatus();
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
      if (val.type_kind() != ::googlesql::TYPE_STRING) {
        return absl::InvalidArgumentError(
            "semantic: proto field expects STRING value");
      }
      reflection->SetString(msg, field, val.string_value());
      return absl::OkStatus();
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
      if (val.type_kind() != ::googlesql::TYPE_BOOL) {
        return absl::InvalidArgumentError(
            "semantic: proto field expects BOOL value");
      }
      reflection->SetBool(msg, field, val.bool_value());
      return absl::OkStatus();
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
      if (val.type_kind() != ::googlesql::TYPE_DOUBLE &&
          val.type_kind() != ::googlesql::TYPE_FLOAT) {
        return absl::InvalidArgumentError(
            "semantic: proto field expects FLOAT64-compatible value");
      }
      reflection->SetDouble(msg, field, val.double_value());
      return absl::OkStatus();
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
      if (val.type_kind() != ::googlesql::TYPE_INT64 &&
          val.type_kind() != ::googlesql::TYPE_ENUM) {
        return absl::InvalidArgumentError(
            "semantic: proto enum field expects INT64-compatible value");
      }
      reflection->SetEnumValue(msg, field, static_cast<int>(val.int64_value()));
      return absl::OkStatus();
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
      if (val.type_kind() != ::googlesql::TYPE_PROTO) {
        return absl::InvalidArgumentError(
            "semantic: nested proto field expects PROTO value");
      }
      google::protobuf::DynamicMessageFactory factory;
      google::protobuf::Message* nested =
          val.ToMessage(&factory, /*return_null_on_error=*/true);
      if (nested == nullptr) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 "semantic: invalid nested PROTO value");
      }
      reflection->MutableMessage(msg, field)->CopyFrom(*nested);
      return absl::OkStatus();
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: proto field cpp_type ",
                       field->CppTypeName(field->cpp_type()),
                       " is not yet supported"));
  }
}

absl::StatusOr<google::protobuf::Message*> MutableMessageAtPath(
    google::protobuf::Message* root,
    const std::vector<const google::protobuf::FieldDescriptor*>& path,
    bool create_missing) {
  google::protobuf::Message* current = root;
  for (size_t i = 0; i + 1 < path.size(); ++i) {
    const google::protobuf::FieldDescriptor* field = path[i];
    if (field->cpp_type() !=
        google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      return absl::InvalidArgumentError(
          "semantic: intermediate proto path is not a message");
    }
    const google::protobuf::Reflection* reflection = current->GetReflection();
    if (!reflection->HasField(*current, field)) {
      if (!create_missing) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: missing proto submessage in path");
      }
      current = reflection->MutableMessage(current, field);
    } else {
      current = reflection->MutableMessage(current, field);
    }
  }
  return current;
}

absl::StatusOr<Value> ReadProtoFieldValue(
    const google::protobuf::Message& msg,
    const google::protobuf::FieldDescriptor* field,
    const ::googlesql::Type* target_type,
    ::googlesql::FieldFormat::Format /*format*/,
    const Value& default_value,
    bool get_has_bit) {
  if (field == nullptr) {
    return absl::InvalidArgumentError("semantic: null proto field descriptor");
  }
  const google::protobuf::Reflection* reflection = msg.GetReflection();
  if (get_has_bit) {
    if (field->is_repeated()) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: get_has_bit is not supported for repeated proto fields");
    }
    return Value::Bool(reflection->HasField(msg, field));
  }
  if (field->is_repeated()) {
    if (target_type == nullptr || !target_type->IsArray()) {
      return absl::InvalidArgumentError(
          "semantic: repeated proto field requires ARRAY target type");
    }
    const ::googlesql::ArrayType* array_type = target_type->AsArray();
    if (array_type == nullptr || array_type->element_type() == nullptr) {
      return absl::InternalError(
          "semantic: repeated proto field ARRAY type is malformed");
    }
    const int count = reflection->FieldSize(msg, field);
    std::vector<Value> elements;
    elements.reserve(count);
    for (int i = 0; i < count; ++i) {
      switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
          elements.push_back(Value::Int64(
              reflection->GetRepeatedInt32(msg, field, i)));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
          elements.push_back(Value::Int64(
              reflection->GetRepeatedInt64(msg, field, i)));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
          elements.push_back(Value::String(
              reflection->GetRepeatedString(msg, field, i)));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
          elements.push_back(Value::Bool(
              reflection->GetRepeatedBool(msg, field, i)));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
          elements.push_back(Value::Double(
              reflection->GetRepeatedDouble(msg, field, i)));
          break;
        default:
          return MakeSemanticError(
              SemanticErrorReason::kNotImplemented,
              "semantic: repeated proto element type is not yet supported");
      }
    }
    return Value::Array(array_type, std::move(elements));
  }
  if (!reflection->HasField(msg, field)) {
    if (field->is_required()) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: required proto field is not set");
    }
    if (default_value.is_valid()) {
      return default_value;
    }
    return Value::Null(target_type);
  }
  return ScalarFromReflection(msg, field, target_type);
}

absl::StatusOr<Value> StructFieldByPath(const Value& root,
                                        absl::Span<const int> field_path) {
  if (field_path.empty()) {
    return root;
  }
  if (root.is_null()) {
    return Value::Null(root.type());
  }
  if (!root.type()->IsStruct()) {
    return absl::InvalidArgumentError(
        "semantic: StructFieldByPath base is not STRUCT");
  }
  const int idx = field_path[0];
  if (idx < 0 || idx >= root.num_fields()) {
    return absl::InvalidArgumentError(
        "semantic: StructFieldByPath index out of range");
  }
  return StructFieldByPath(root.field(idx), field_path.subspan(1));
}

absl::StatusOr<Value> SetStructFieldByPath(const Value& root,
                                           absl::Span<const int> field_path,
                                           const Value& leaf) {
  if (field_path.empty()) {
    return leaf;
  }
  if (root.is_null()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: cannot set nested STRUCT field on NULL");
  }
  if (!root.type()->IsStruct()) {
    return absl::InvalidArgumentError(
        "semantic: SetStructFieldByPath base is not STRUCT");
  }
  const ::googlesql::StructType* st = root.type()->AsStruct();
  if (st == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: SetStructFieldByPath STRUCT type cast failed");
  }
  const int idx = field_path[0];
  if (idx < 0 || idx >= root.num_fields()) {
    return absl::InvalidArgumentError(
        "semantic: SetStructFieldByPath index out of range");
  }
  std::vector<Value> fields;
  fields.reserve(root.num_fields());
  for (int i = 0; i < root.num_fields(); ++i) {
    fields.push_back(root.field(i));
  }
  if (field_path.size() == 1) {
    fields[idx] = leaf;
    return Value::Struct(st, std::move(fields));
  }
  auto nested = SetStructFieldByPath(fields[idx], field_path.subspan(1), leaf);
  if (!nested.ok()) return nested.status();
  fields[idx] = *std::move(nested);
  return Value::Struct(st, std::move(fields));
}

absl::Status ClearFieldSubtree(
    google::protobuf::Message* root,
    const std::vector<const google::protobuf::FieldDescriptor*>& path) {
  if (path.empty()) {
    return absl::InvalidArgumentError("semantic: empty proto field path");
  }
  auto msg_or = MutableMessageAtPath(root, path, /*create_missing=*/false);
  if (!msg_or.ok()) return msg_or.status();
  google::protobuf::Message* target = *msg_or;
  const google::protobuf::FieldDescriptor* leaf = path.back();
  target->GetReflection()->ClearField(target, leaf);
  return absl::OkStatus();
}

absl::Status CopyFieldSubtree(
    const google::protobuf::Message& src,
    google::protobuf::Message* dst,
    const std::vector<const google::protobuf::FieldDescriptor*>& path) {
  if (path.empty()) {
    return absl::InvalidArgumentError("semantic: empty proto field path");
  }
  const google::protobuf::Message* src_current = &src;
  google::protobuf::Message* dst_current = dst;
  for (size_t i = 0; i < path.size(); ++i) {
    const google::protobuf::FieldDescriptor* field = path[i];
    const google::protobuf::Reflection* src_reflection =
        src_current->GetReflection();
    if (!src_reflection->HasField(*src_current, field)) {
      return absl::OkStatus();
    }
    if (i + 1 == path.size()) {
      if (field->is_repeated()) {
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            "semantic: FILTER_FIELDS on repeated proto fields is not yet "
            "supported");
      }
      const google::protobuf::Reflection* dst_reflection =
          dst_current->GetReflection();
      switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
          dst_reflection->SetInt32(
              dst_current, field,
              src_reflection->GetInt32(*src_current, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
          dst_reflection->SetInt64(
              dst_current, field,
              src_reflection->GetInt64(*src_current, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
          dst_reflection->SetString(
              dst_current, field,
              src_reflection->GetString(*src_current, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
          dst_reflection->SetBool(
              dst_current, field,
              src_reflection->GetBool(*src_current, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
          dst_reflection->SetDouble(
              dst_current, field,
              src_reflection->GetDouble(*src_current, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
          dst_reflection->MutableMessage(dst_current, field)
              ->CopyFrom(src_reflection->GetMessage(*src_current, field));
          break;
        default:
          return MakeSemanticError(
              SemanticErrorReason::kNotImplemented,
              "semantic: FILTER_FIELDS copy for field cpp_type is unsupported");
      }
      return absl::OkStatus();
    }
    if (field->cpp_type() !=
        google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      return absl::InvalidArgumentError(
          "semantic: intermediate FILTER_FIELDS path is not a message");
    }
    src_current = &src_reflection->GetMessage(*src_current, field);
    dst_current =
        dst_current->GetReflection()->MutableMessage(dst_current, field);
  }
  return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<google::protobuf::Message>> CloneProtoMessage(
    const Value& proto_value,
    google::protobuf::DynamicMessageFactory* factory) {
  if (proto_value.is_null()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: cannot clone NULL proto");
  }
  if (proto_value.type_kind() != ::googlesql::TYPE_PROTO) {
    return absl::InvalidArgumentError("semantic: clone source is not PROTO");
  }
  google::protobuf::Message* msg =
      proto_value.ToMessage(factory, /*return_null_on_error=*/true);
  if (msg == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: invalid proto bytes");
  }
  return std::unique_ptr<google::protobuf::Message>(msg);
}

absl::StatusOr<Value> MessageToProtoValue(
    const google::protobuf::Message& msg, const ::googlesql::ProtoType* proto_type) {
  if (proto_type == nullptr) {
    return absl::InternalError("semantic: null ProtoType for serialization");
  }
  return Value::Proto(proto_type, absl::Cord(msg.SerializeAsString()));
}

}  // namespace proto_helpers
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
