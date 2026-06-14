#include <memory>
#include <utility>
#include <vector>

#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/proto_helpers.h"
#include "backend/engine/semantic/value.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "googlesql/public/types/proto_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using ::googlesql::Value;
namespace proto = proto_helpers;

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
  auto msg_or = proto::MutableMessageAtPath(
      root, item.proto_field_path(), create_missing);
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
      return proto::SetScalarProtoField(target, leaf, *value_or);
    }
    return absl::OkStatus();
  }

  return proto::SetScalarProtoField(target, leaf, *value_or);
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
  return proto::MessageToProtoValue(*msg, proto_type);
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
