#include <memory>
#include <string>
#include <vector>

#include "absl/strings/cord.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/value.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/proto_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using ::googlesql::Value;

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
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: proto field cpp_type ",
                       field->CppTypeName(field->cpp_type()),
                       " is not yet supported in UpdateConstructor"));
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
          "semantic: intermediate UpdateConstructor path is not a message");
    }
    const google::protobuf::Reflection* reflection = current->GetReflection();
    if (!reflection->HasField(*current, field)) {
      if (!create_missing) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: missing proto submessage in UpdateConstructor path");
      }
      current = reflection->MutableMessage(current, field);
    } else {
      current = reflection->MutableMessage(current, field);
    }
  }
  return current;
}

absl::Status ApplyUpdateFieldItem(
    google::protobuf::Message* root,
    const ::googlesql::ResolvedUpdateFieldItem& item,
    const EvalContext& ctx) {
  if (item.proto_field_path().empty()) {
    return absl::InvalidArgumentError(
        "semantic: UpdateFieldItem has empty proto_field_path");
  }
  const bool create_missing =
      item.operation() !=
      ::googlesql::ResolvedUpdateFieldItem::UPDATE_SINGLE_NO_CREATION;
  auto msg_or =
      MutableMessageAtPath(root, item.proto_field_path(), create_missing);
  if (!msg_or.ok()) return msg_or.status();
  google::protobuf::Message* target = *msg_or;
  const google::protobuf::FieldDescriptor* leaf =
      item.proto_field_path().back();
  const google::protobuf::Reflection* reflection = target->GetReflection();

  if (item.expr() == nullptr) {
    reflection->ClearField(target, leaf);
    return absl::OkStatus();
  }
  auto value_or = EvalExpr(*item.expr(), ctx);
  if (!value_or.ok()) return value_or.status();

  if (leaf->is_repeated()) {
    if (item.operation() != ::googlesql::ResolvedUpdateFieldItem::UPDATE_MANY) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: repeated proto field updates require UPDATE_MANY");
    }
    reflection->ClearField(target, leaf);
    if (!value_or->is_null()) {
      return SetScalarProtoField(target, leaf, *value_or);
    }
    return absl::OkStatus();
  }

  return SetScalarProtoField(target, leaf, *value_or);
}

}  // namespace

absl::StatusOr<Value> EvalUpdateConstructor(
    const ::googlesql::ResolvedUpdateConstructor& node,
    const EvalContext& ctx) {
  if (node.type() == nullptr || !node.type()->IsProto()) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: UpdateConstructor result type is not PROTO");
  }
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: UpdateConstructor has null base expr");
  }
  auto base_or = EvalExpr(*node.expr(), ctx);
  if (!base_or.ok()) return base_or.status();

  google::protobuf::DynamicMessageFactory factory;
  std::unique_ptr<google::protobuf::Message> owned;
  google::protobuf::Message* msg = nullptr;
  if (base_or->is_null()) {
    const auto* proto_type = node.type()->AsProto();
    if (proto_type == nullptr || proto_type->descriptor() == nullptr) {
      return absl::InternalError(
          "semantic: UpdateConstructor PROTO type missing descriptor");
    }
    owned.reset(factory.GetPrototype(proto_type->descriptor())->New());
    msg = owned.get();
  } else {
    if (base_or->type_kind() != ::googlesql::TYPE_PROTO) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: UpdateConstructor base expr is not PROTO");
    }
    msg = base_or->ToMessage(&factory, /*return_null_on_error=*/true);
    if (msg == nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: UpdateConstructor base proto is invalid");
    }
    owned.reset(msg);
  }

  for (int i = 0; i < node.update_field_item_list_size(); ++i) {
    const ::googlesql::ResolvedUpdateFieldItem* item =
        node.update_field_item_list(i);
    if (item == nullptr) {
      return absl::InternalError("semantic: UpdateConstructor null field item");
    }
    absl::Status applied = ApplyUpdateFieldItem(msg, *item, ctx);
    if (!applied.ok()) return applied;
  }

  const auto* proto_type = node.type()->AsProto();
  return Value::Proto(proto_type, absl::Cord(msg->SerializeAsString()));
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
