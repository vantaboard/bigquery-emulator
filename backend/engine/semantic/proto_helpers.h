#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PROTO_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PROTO_HELPERS_H_

#include <memory>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "backend/engine/semantic/value.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "googlesql/public/proto/type_annotation.pb.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace proto_helpers {

using ::googlesql::Value;

absl::Status SetScalarProtoField(google::protobuf::Message* msg,
                                 const google::protobuf::FieldDescriptor* field,
                                 const Value& val);

absl::StatusOr<google::protobuf::Message*> MutableMessageAtPath(
    google::protobuf::Message* root,
    const std::vector<const google::protobuf::FieldDescriptor*>& path,
    bool create_missing);

absl::StatusOr<Value> ReadProtoFieldValue(
    const google::protobuf::Message& msg,
    const google::protobuf::FieldDescriptor* field,
    const ::googlesql::Type* target_type,
    ::googlesql::FieldFormat::Format format,
    const Value& default_value,
    bool get_has_bit);

absl::StatusOr<Value> StructFieldByPath(const Value& root,
                                        absl::Span<const int> field_path);

absl::StatusOr<Value> SetStructFieldByPath(const Value& root,
                                           absl::Span<const int> field_path,
                                           const Value& leaf);

absl::Status ClearFieldSubtree(
    google::protobuf::Message* root,
    const std::vector<const google::protobuf::FieldDescriptor*>& path);

absl::Status CopyFieldSubtree(
    const google::protobuf::Message& src,
    google::protobuf::Message* dst,
    const std::vector<const google::protobuf::FieldDescriptor*>& path);

absl::StatusOr<std::unique_ptr<google::protobuf::Message>> CloneProtoMessage(
    const Value& proto_value, google::protobuf::DynamicMessageFactory* factory);

absl::StatusOr<Value> MessageToProtoValue(
    const google::protobuf::Message& msg,
    const ::googlesql::ProtoType* proto_type);

}  // namespace proto_helpers
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PROTO_HELPERS_H_
