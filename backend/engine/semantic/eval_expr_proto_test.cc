#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_column.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {
namespace {

struct TestPersonProto {
  google::protobuf::DescriptorPool pool;
  const google::protobuf::Descriptor* person_desc = nullptr;
  const google::protobuf::FieldDescriptor* name_field = nullptr;
  const google::protobuf::FieldDescriptor* id_field = nullptr;
  const google::protobuf::OneofDescriptor* contact_oneof = nullptr;
  const google::protobuf::FieldDescriptor* email_field = nullptr;
  const ::googlesql::ProtoType* proto_type = nullptr;

  void Init(::googlesql::TypeFactory* type_factory) {
    google::protobuf::FileDescriptorProto file;
    file.set_name("test_person.proto");
    file.set_package("test");
    google::protobuf::DescriptorProto* msg = file.add_message_type();
    msg->set_name("Person");
    auto* name = msg->add_field();
    name->set_name("name");
    name->set_number(1);
    name->set_type(google::protobuf::FieldDescriptorProto::TYPE_STRING);
    name->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
    auto* id = msg->add_field();
    id->set_name("id");
    id->set_number(2);
    id->set_type(google::protobuf::FieldDescriptorProto::TYPE_INT64);
    id->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
    auto* oneof = msg->add_oneof_decl();
    oneof->set_name("contact");
    auto* email = msg->add_field();
    email->set_name("email");
    email->set_number(3);
    email->set_type(google::protobuf::FieldDescriptorProto::TYPE_STRING);
    email->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
    email->set_oneof_index(0);
    auto* phone = msg->add_field();
    phone->set_name("phone");
    phone->set_number(4);
    phone->set_type(google::protobuf::FieldDescriptorProto::TYPE_STRING);
    phone->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
    phone->set_oneof_index(0);

    const google::protobuf::FileDescriptor* file_desc = pool.BuildFile(file);
    ASSERT_NE(file_desc, nullptr);
    person_desc = file_desc->FindMessageTypeByName("Person");
    ASSERT_NE(person_desc, nullptr);
    name_field = person_desc->FindFieldByName("name");
    id_field = person_desc->FindFieldByName("id");
    contact_oneof = person_desc->FindOneofByName("contact");
    email_field = person_desc->FindFieldByName("email");
    ASSERT_TRUE(type_factory->MakeProtoType(person_desc, &proto_type).ok());
    ASSERT_NE(proto_type, nullptr);
  }
};

::googlesql::Value BuildPersonProto(const TestPersonProto& fixture,
                                    absl::string_view name,
                                    int64_t id,
                                    absl::string_view email) {
  google::protobuf::DynamicMessageFactory factory;
  std::unique_ptr<google::protobuf::Message> msg(
      factory.GetPrototype(fixture.person_desc)->New());
  msg->GetReflection()->SetString(
      msg.get(), fixture.name_field, std::string(name));
  msg->GetReflection()->SetInt64(msg.get(), fixture.id_field, id);
  msg->GetReflection()->SetString(
      msg.get(), fixture.email_field, std::string(email));
  return ::googlesql::Value::Proto(fixture.proto_type,
                                   absl::Cord(msg->SerializeAsString()));
}

TEST(EvalProtoShapesTest, MakeProtoConstructsScalarFields) {
  ::googlesql::TypeFactory type_factory;
  TestPersonProto fixture;
  fixture.Init(&type_factory);
  std::vector<std::unique_ptr<const ::googlesql::ResolvedMakeProtoField>>
      fields;
  fields.push_back(::googlesql::MakeResolvedMakeProtoField(
      fixture.name_field,
      ::googlesql::FieldFormat::DEFAULT_FORMAT,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::String("Ada"))));
  fields.push_back(::googlesql::MakeResolvedMakeProtoField(
      fixture.id_field,
      ::googlesql::FieldFormat::DEFAULT_FORMAT,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(7))));

  auto node =
      ::googlesql::MakeResolvedMakeProto(fixture.proto_type, std::move(fields));
  EvalContext ctx{.project_id = "test"};
  auto result = EvalMakeProto(*node, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  ASSERT_FALSE(result->is_null());

  google::protobuf::DynamicMessageFactory factory;
  std::unique_ptr<google::protobuf::Message> msg(
      result->ToMessage(&factory, /*return_null_on_error=*/true));
  ASSERT_NE(msg, nullptr);
  EXPECT_EQ(msg->GetReflection()->GetString(*msg, fixture.name_field), "Ada");
  EXPECT_EQ(msg->GetReflection()->GetInt64(*msg, fixture.id_field), 7);
}

TEST(EvalProtoShapesTest, GetProtoFieldReadsPresentScalar) {
  ::googlesql::TypeFactory type_factory;
  TestPersonProto fixture;
  fixture.Init(&type_factory);
  auto base = ::googlesql::MakeResolvedLiteral(
      BuildPersonProto(fixture, "Grace", 3, "g@example.com"));

  auto node = ::googlesql::MakeResolvedGetProtoField(
      type_factory.get_string(),
      std::move(base),
      fixture.name_field,
      ::googlesql::Value(),
      /*get_has_bit=*/false,
      ::googlesql::FieldFormat::DEFAULT_FORMAT,
      /*return_default_value_when_unset=*/false);
  EvalContext ctx{.project_id = "test"};
  auto result = EvalGetProtoField(*node, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->string_value(), "Grace");
}

TEST(EvalProtoShapesTest, GetProtoFieldHasBitReflectsPresence) {
  ::googlesql::TypeFactory type_factory;
  TestPersonProto fixture;
  fixture.Init(&type_factory);
  auto base = ::googlesql::MakeResolvedLiteral(
      BuildPersonProto(fixture, "Grace", 3, "g@example.com"));

  auto node = ::googlesql::MakeResolvedGetProtoField(
      type_factory.get_bool(),
      std::move(base),
      fixture.name_field,
      ::googlesql::Value(),
      /*get_has_bit=*/true,
      ::googlesql::FieldFormat::DEFAULT_FORMAT,
      /*return_default_value_when_unset=*/false);
  EvalContext ctx{.project_id = "test"};
  auto result = EvalGetProtoField(*node, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_TRUE(result->bool_value());
}

TEST(EvalProtoShapesTest, GetProtoOneofReturnsSetFieldName) {
  ::googlesql::TypeFactory type_factory;
  TestPersonProto fixture;
  fixture.Init(&type_factory);
  auto base = ::googlesql::MakeResolvedLiteral(
      BuildPersonProto(fixture, "Grace", 3, "g@example.com"));

  auto node = ::googlesql::MakeResolvedGetProtoOneof(
      type_factory.get_string(), std::move(base), fixture.contact_oneof);
  EvalContext ctx{.project_id = "test"};
  auto result = EvalGetProtoOneof(*node, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->string_value(), "email");
}

TEST(EvalProtoShapesTest, ReplaceFieldMutatesStructLeaf) {
  ::googlesql::TypeFactory type_factory;
  const ::googlesql::StructType* struct_type = nullptr;
  ASSERT_TRUE(type_factory
                  .MakeStructType({{"a", type_factory.get_int64()},
                                   {"b", type_factory.get_string()}},
                                  &struct_type)
                  .ok());
  auto base = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Struct(
      struct_type,
      {::googlesql::Value::Int64(1), ::googlesql::Value::String("keep")}));

  std::vector<std::unique_ptr<const ::googlesql::ResolvedReplaceFieldItem>>
      items;
  items.push_back(::googlesql::MakeResolvedReplaceFieldItem(
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(99)),
      std::vector<int>{0},
      std::vector<const google::protobuf::FieldDescriptor*>{}));

  auto node = ::googlesql::MakeResolvedReplaceField(
      struct_type, std::move(base), std::move(items));
  EvalContext ctx{.project_id = "test"};
  auto result = EvalReplaceField(*node, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->field(0).int64_value(), 99);
  EXPECT_EQ(result->field(1).string_value(), "keep");
}

TEST(EvalProtoShapesTest, FilterFieldIncludesSelectedPaths) {
  ::googlesql::TypeFactory type_factory;
  TestPersonProto fixture;
  fixture.Init(&type_factory);
  auto base = ::googlesql::MakeResolvedLiteral(
      BuildPersonProto(fixture, "Grace", 3, "g@example.com"));

  std::vector<std::unique_ptr<const ::googlesql::ResolvedFilterFieldArg>> args;
  args.push_back(::googlesql::MakeResolvedFilterFieldArg(
      /*include=*/true,
      std::vector<const google::protobuf::FieldDescriptor*>{
          fixture.name_field}));

  auto node = ::googlesql::MakeResolvedFilterField(
      fixture.proto_type,
      std::move(base),
      std::move(args),
      /*reset_cleared_required_fields=*/false);
  EvalContext ctx{.project_id = "test"};
  auto result = EvalFilterField(*node, ctx);
  ASSERT_TRUE(result.ok()) << result.status();

  google::protobuf::DynamicMessageFactory factory;
  std::unique_ptr<google::protobuf::Message> msg(
      result->ToMessage(&factory, /*return_null_on_error=*/true));
  ASSERT_NE(msg, nullptr);
  EXPECT_EQ(msg->GetReflection()->GetString(*msg, fixture.name_field), "Grace");
  EXPECT_FALSE(msg->GetReflection()->HasField(*msg, fixture.id_field));
  EXPECT_FALSE(msg->GetReflection()->HasField(*msg, fixture.email_field));
}

TEST(EvalProtoShapesTest, GetRowFieldReadsNamedStructField) {
  ::googlesql::TypeFactory type_factory;
  const ::googlesql::StructType* struct_type = nullptr;
  ASSERT_TRUE(type_factory
                  .MakeStructType({{"x", type_factory.get_int64()},
                                   {"y", type_factory.get_string()}},
                                  &struct_type)
                  .ok());
  auto base = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Struct(
      struct_type,
      {::googlesql::Value::Int64(5), ::googlesql::Value::String("z")}));
  ::googlesql::SimpleColumn col("t", "y", struct_type->field(1).type);

  auto node = ::googlesql::MakeResolvedGetRowField(
      type_factory.get_string(), std::move(base), &col);
  EvalContext ctx{.project_id = "test"};
  auto result = EvalGetRowField(*node, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->string_value(), "z");
}

}  // namespace
}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
