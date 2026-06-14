#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/value.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {
namespace {

const google::protobuf::Descriptor* BuildTestRowDescriptor(
    google::protobuf::DescriptorPool* pool) {
  google::protobuf::FileDescriptorProto file;
  file.set_name("test_update_constructor.proto");
  file.set_package("test");
  google::protobuf::DescriptorProto* msg = file.add_message_type();
  msg->set_name("Row");
  google::protobuf::FieldDescriptorProto* field = msg->add_field();
  field->set_name("val");
  field->set_number(1);
  field->set_type(google::protobuf::FieldDescriptorProto::TYPE_INT64);
  field->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
  const google::protobuf::FileDescriptor* file_desc =
      pool->BuildFile(file);
  return file_desc == nullptr ? nullptr : file_desc->FindMessageTypeByName("Row");
}

TEST(EvalUpdateConstructorTest, SetsProtoFieldOnNullBase) {
  google::protobuf::DescriptorPool pool;
  const google::protobuf::Descriptor* row_desc = BuildTestRowDescriptor(&pool);
  ASSERT_NE(row_desc, nullptr);

  ::googlesql::TypeFactory type_factory;
  const ::googlesql::ProtoType* proto_type = nullptr;
  ASSERT_TRUE(type_factory.MakeProtoType(row_desc, &proto_type).ok());
  ASSERT_NE(proto_type, nullptr);

  std::vector<const google::protobuf::FieldDescriptor*> path = {
      row_desc->FindFieldByName("val")};
  std::vector<std::unique_ptr<const ::googlesql::ResolvedUpdateFieldItem>> items;
  items.push_back(::googlesql::MakeResolvedUpdateFieldItem(
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(42)),
      path,
      ::googlesql::ResolvedUpdateFieldItem::UPDATE_SINGLE));

  auto ctor = ::googlesql::MakeResolvedUpdateConstructor(
      proto_type,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Null(proto_type)),
      /*alias=*/"",
      std::move(items));

  EvalContext ctx{.project_id = "test"};
  auto result = EvalUpdateConstructor(*ctor, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  ASSERT_FALSE(result->is_null());
  EXPECT_EQ(result->type_kind(), ::googlesql::TYPE_PROTO);

  google::protobuf::DynamicMessageFactory factory;
  std::unique_ptr<google::protobuf::Message> msg(
      result->ToMessage(&factory, /*return_null_on_error=*/true));
  ASSERT_NE(msg, nullptr);
  const google::protobuf::FieldDescriptor* val_field =
      msg->GetDescriptor()->FindFieldByName("val");
  ASSERT_NE(val_field, nullptr);
  EXPECT_EQ(msg->GetReflection()->GetInt64(*msg, val_field), 42);
}

}  // namespace
}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
