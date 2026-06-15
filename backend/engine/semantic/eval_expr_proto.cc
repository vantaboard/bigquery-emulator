#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/proto_helpers.h"
#include "backend/engine/semantic/value.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/proto_type.h"
#include "googlesql/public/types/struct_type.h"
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
namespace proto = proto_helpers;

absl::Status ClearAllFields(google::protobuf::Message* msg) {
  msg->Clear();
  return absl::OkStatus();
}

absl::StatusOr<Value> EvalMakeProtoImpl(
    const ::googlesql::ResolvedMakeProto& node, const EvalContext& ctx) {
  if (node.type() == nullptr || !node.type()->IsProto()) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: MakeProto result type is not PROTO");
  }
  const auto* proto_type = node.type()->AsProto();
  if (proto_type == nullptr || proto_type->descriptor() == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: MakeProto PROTO type missing descriptor");
  }
  google::protobuf::DynamicMessageFactory factory;
  std::unique_ptr<google::protobuf::Message> msg(
      factory.GetPrototype(proto_type->descriptor())->New());
  for (int i = 0; i < node.field_list_size(); ++i) {
    const ::googlesql::ResolvedMakeProtoField* field = node.field_list(i);
    if (field == nullptr || field->field_descriptor() == nullptr) {
      return absl::InternalError("semantic: MakeProtoField is null");
    }
    if (field->expr() == nullptr) {
      msg->GetReflection()->ClearField(msg.get(), field->field_descriptor());
      continue;
    }
    auto value_or = EvalExpr(*field->expr(), ctx);
    if (!value_or.ok()) return value_or.status();
    if (field->field_descriptor()->is_repeated()) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: MakeProto repeated fields require ARRAY expr");
    }
    absl::Status applied = proto::SetScalarProtoField(
        msg.get(), field->field_descriptor(), *value_or);
    if (!applied.ok()) return applied;
  }
  return proto::MessageToProtoValue(*msg, proto_type);
}

absl::StatusOr<Value> EvalGetProtoFieldImpl(
    const ::googlesql::ResolvedGetProtoField& node,
    const EvalContext& ctx,
    const ::googlesql::Type* result_type) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetProtoField has null expr");
  }
  if (node.field_descriptor() == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: GetProtoField missing field descriptor");
  }
  auto base_or = EvalExpr(*node.expr(), ctx);
  if (!base_or.ok()) return base_or.status();
  if (base_or->is_null()) {
    if (node.return_default_value_when_unset()) {
      return node.default_value();
    }
    return NullOfType(result_type);
  }
  if (base_or->type_kind() != ::googlesql::TYPE_PROTO) {
    return absl::InvalidArgumentError(
        "semantic: GetProtoField base is not PROTO");
  }
  google::protobuf::DynamicMessageFactory factory;
  auto msg_or = proto::CloneProtoMessage(*base_or, &factory);
  if (!msg_or.ok()) return msg_or.status();
  return proto::ReadProtoFieldValue(**msg_or,
                                    node.field_descriptor(),
                                    result_type,
                                    node.format(),
                                    node.default_value(),
                                    node.get_has_bit());
}

absl::StatusOr<Value> EvalGetProtoOneofImpl(
    const ::googlesql::ResolvedGetProtoOneof& node,
    const EvalContext& ctx,
    const ::googlesql::Type* result_type) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetProtoOneof has null expr");
  }
  if (node.oneof_descriptor() == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: GetProtoOneof missing oneof descriptor");
  }
  auto base_or = EvalExpr(*node.expr(), ctx);
  if (!base_or.ok()) return base_or.status();
  if (base_or->is_null()) {
    return NullOfType(result_type);
  }
  if (base_or->type_kind() != ::googlesql::TYPE_PROTO) {
    return absl::InvalidArgumentError(
        "semantic: GetProtoOneof base is not PROTO");
  }
  google::protobuf::DynamicMessageFactory factory;
  google::protobuf::Message* msg =
      base_or->ToMessage(&factory, /*return_null_on_error=*/true);
  if (msg == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: GetProtoOneof invalid proto bytes");
  }
  const google::protobuf::FieldDescriptor* set_field =
      msg->GetReflection()->GetOneofFieldDescriptor(*msg,
                                                    node.oneof_descriptor());
  if (set_field == nullptr) {
    return NullOfType(result_type);
  }
  return Value::String(set_field->name());
}

absl::Status ApplyReplaceFieldItem(
    Value* current,
    const ::googlesql::ResolvedReplaceFieldItem& item,
    const EvalContext& ctx) {
  if (item.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ReplaceFieldItem has null expr");
  }
  auto value_or = EvalExpr(*item.expr(), ctx);
  if (!value_or.ok()) return value_or.status();

  if (!item.proto_field_path().empty()) {
    if (current->type_kind() != ::googlesql::TYPE_PROTO) {
      return absl::InvalidArgumentError(
          "semantic: ReplaceField proto path requires PROTO base");
    }
    const auto* proto_type = current->type()->AsProto();
    if (proto_type == nullptr || proto_type->descriptor() == nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: ReplaceField PROTO type missing descriptor");
    }
    google::protobuf::DynamicMessageFactory factory;
    auto msg_or = proto::CloneProtoMessage(*current, &factory);
    if (!msg_or.ok()) return msg_or.status();
    google::protobuf::Message* msg = msg_or->get();
    auto target_or = proto::MutableMessageAtPath(
        msg, item.proto_field_path(), /*create_missing=*/true);
    if (!target_or.ok()) return target_or.status();
    google::protobuf::Message* target_msg = *target_or;
    const google::protobuf::FieldDescriptor* leaf =
        item.proto_field_path().back();
    if (value_or->is_null()) {
      if (leaf->is_required()) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: cannot unset required proto field in REPLACE_FIELDS");
      }
      target_msg->GetReflection()->ClearField(target_msg, leaf);
    } else {
      absl::Status applied =
          proto::SetScalarProtoField(target_msg, leaf, *value_or);
      if (!applied.ok()) return applied;
    }
    auto out = proto::MessageToProtoValue(*msg, proto_type);
    if (!out.ok()) return out.status();
    *current = *std::move(out);
    return absl::OkStatus();
  }

  if (item.struct_index_path().empty()) {
    *current = *value_or;
    return absl::OkStatus();
  }
  auto updated = proto::SetStructFieldByPath(
      *current, item.struct_index_path(), *value_or);
  if (!updated.ok()) return updated.status();
  *current = *std::move(updated);
  return absl::OkStatus();
}

absl::StatusOr<Value> EvalReplaceFieldImpl(
    const ::googlesql::ResolvedReplaceField& node, const EvalContext& ctx) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedReplaceField has null expr");
  }
  auto base_or = EvalExpr(*node.expr(), ctx);
  if (!base_or.ok()) return base_or.status();
  Value current = *std::move(base_or);
  for (int i = 0; i < node.replace_field_item_list_size(); ++i) {
    const ::googlesql::ResolvedReplaceFieldItem* item =
        node.replace_field_item_list(i);
    if (item == nullptr) {
      return absl::InternalError("semantic: ReplaceFieldItem is null");
    }
    absl::Status applied = ApplyReplaceFieldItem(&current, *item, ctx);
    if (!applied.ok()) return applied;
  }
  return current;
}

absl::StatusOr<Value> EvalFilterFieldImpl(
    const ::googlesql::ResolvedFilterField& node, const EvalContext& ctx) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedFilterField has null expr");
  }
  if (node.filter_field_arg_list_size() == 0) {
    return absl::InvalidArgumentError(
        "semantic: FILTER_FIELDS requires at least one field arg");
  }
  auto base_or = EvalExpr(*node.expr(), ctx);
  if (!base_or.ok()) return base_or.status();
  if (base_or->is_null()) {
    return NullOfType(node.type());
  }
  if (base_or->type_kind() != ::googlesql::TYPE_PROTO) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: FILTER_FIELDS base is not PROTO");
  }
  const auto* proto_type = base_or->type()->AsProto();
  if (proto_type == nullptr || proto_type->descriptor() == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: FILTER_FIELDS PROTO type missing descriptor");
  }
  google::protobuf::DynamicMessageFactory factory;
  auto src_or = proto::CloneProtoMessage(*base_or, &factory);
  if (!src_or.ok()) return src_or.status();
  const google::protobuf::Message& src = **src_or;
  std::unique_ptr<google::protobuf::Message> dst(
      factory.GetPrototype(proto_type->descriptor())->New());

  if (node.filter_field_arg_list(0)->include()) {
    ClearAllFields(dst.get());
  } else {
    dst->CopyFrom(src);
  }

  for (int i = 0; i < node.filter_field_arg_list_size(); ++i) {
    const ::googlesql::ResolvedFilterFieldArg* arg =
        node.filter_field_arg_list(i);
    if (arg == nullptr) {
      return absl::InternalError("semantic: FilterFieldArg is null");
    }
    std::vector<const google::protobuf::FieldDescriptor*> path;
    path.reserve(arg->field_descriptor_path_size());
    for (int j = 0; j < arg->field_descriptor_path_size(); ++j) {
      path.push_back(arg->field_descriptor_path(j));
    }
    if (arg->include()) {
      absl::Status copied = proto::CopyFieldSubtree(src, dst.get(), path);
      if (!copied.ok()) return copied;
    } else {
      absl::Status cleared = proto::ClearFieldSubtree(dst.get(), path);
      if (!cleared.ok()) return cleared;
    }
  }

  return proto::MessageToProtoValue(*dst, proto_type);
}

absl::StatusOr<Value> EvalGetRowFieldImpl(
    const ::googlesql::ResolvedGetRowField& node,
    const EvalContext& ctx,
    const ::googlesql::Type* result_type) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetRowField has null expr");
  }
  if (node.column() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetRowField has null column");
  }
  auto base_or = EvalExpr(*node.expr(), ctx);
  if (!base_or.ok()) return base_or.status();
  if (base_or->is_null()) {
    return NullOfType(result_type);
  }
  if (!base_or->type()->IsStruct()) {
    return absl::InvalidArgumentError(
        "semantic: GetRowField base is not STRUCT");
  }
  const ::googlesql::StructType* st = base_or->type()->AsStruct();
  if (st == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: GetRowField STRUCT type cast failed");
  }
  const std::string& name = node.column()->Name();
  for (int i = 0; i < st->num_fields(); ++i) {
    if (st->field(i).name == name) {
      return base_or->field(i);
    }
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "semantic: GetRowField column '", name, "' not found in row STRUCT"));
}

}  // namespace

absl::StatusOr<Value> EvalMakeProto(const ::googlesql::ResolvedMakeProto& node,
                                    const EvalContext& ctx) {
  return EvalMakeProtoImpl(node, ctx);
}

absl::StatusOr<Value> EvalGetProtoField(
    const ::googlesql::ResolvedGetProtoField& node, const EvalContext& ctx) {
  return EvalGetProtoFieldImpl(node, ctx, node.type());
}

absl::StatusOr<Value> EvalGetProtoOneof(
    const ::googlesql::ResolvedGetProtoOneof& node, const EvalContext& ctx) {
  return EvalGetProtoOneofImpl(node, ctx, node.type());
}

absl::StatusOr<Value> EvalReplaceField(
    const ::googlesql::ResolvedReplaceField& node, const EvalContext& ctx) {
  return EvalReplaceFieldImpl(node, ctx);
}

absl::StatusOr<Value> EvalFilterField(
    const ::googlesql::ResolvedFilterField& node, const EvalContext& ctx) {
  return EvalFilterFieldImpl(node, ctx);
}

absl::StatusOr<Value> EvalGetRowField(
    const ::googlesql::ResolvedGetRowField& node, const EvalContext& ctx) {
  return EvalGetRowFieldImpl(node, ctx, node.type());
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
