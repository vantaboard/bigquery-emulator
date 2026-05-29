#include "backend/schema/googlesql_to_bq.h"

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_column.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace schema {

namespace {

// Returns the BigQuery scalar type name for `kind`. Empty string means
// "this kind is not a BigQuery scalar"; callers route the four
// container / unsupported branches (ARRAY, STRUCT, unsupported) by
// inspecting the kind directly.
absl::string_view BqScalarName(::googlesql::TypeKind kind) {
  switch (kind) {
    case ::googlesql::TYPE_BOOL:
      return "BOOL";
    case ::googlesql::TYPE_INT64:
      return "INT64";
    case ::googlesql::TYPE_DOUBLE:
      return "FLOAT64";
    case ::googlesql::TYPE_STRING:
      return "STRING";
    case ::googlesql::TYPE_BYTES:
      return "BYTES";
    case ::googlesql::TYPE_DATE:
      return "DATE";
    case ::googlesql::TYPE_TIME:
      return "TIME";
    case ::googlesql::TYPE_DATETIME:
      return "DATETIME";
    case ::googlesql::TYPE_TIMESTAMP:
      return "TIMESTAMP";
    case ::googlesql::TYPE_NUMERIC:
      return "NUMERIC";
    case ::googlesql::TYPE_BIGNUMERIC:
      return "BIGNUMERIC";
    case ::googlesql::TYPE_JSON:
      return "JSON";
    case ::googlesql::TYPE_GEOGRAPHY:
      return "GEOGRAPHY";
    default:
      return {};
  }
}

absl::Status FillScalarOrStruct(const ::googlesql::Type* type,
                                const std::string& nested_path,
                                v1::FieldSchema* out);

// Populate `*nested` (a STRUCT child field) for an ARRAY-typed
// `field.type`. Sets the REPEATED mode + lowers the element type
// through `FillScalarOrStruct`. ARRAY<ARRAY<...>> is rejected because
// BigQuery has no wire shape for it.
absl::Status FillArrayField(const ::googlesql::Type* array_type,
                            const std::string& child_path,
                            v1::FieldSchema* nested) {
  const ::googlesql::Type* element = array_type->AsArray()->element_type();
  if (element != nullptr && element->IsArray()) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '",
                     child_path,
                     "': ARRAY<ARRAY<...>> is not representable in BigQuery"));
  }
  absl::Status s = FillScalarOrStruct(element, child_path, nested);
  if (!s.ok()) return s;
  nested->set_mode("REPEATED");
  return absl::OkStatus();
}

// Lower the field list of a STRUCT type into `out->fields`. Caller
// has already set `out->type` to "STRUCT". `nested_path` is the
// dotted-path prefix used in error messages.
absl::Status FillStructFields(const ::googlesql::StructType* st,
                              const std::string& nested_path,
                              v1::FieldSchema* out) {
  for (int i = 0; i < st->num_fields(); ++i) {
    const ::googlesql::StructType::StructField& field = st->field(i);
    v1::FieldSchema* nested = out->add_fields();
    // BigQuery requires every STRUCT field to be named; unnamed
    // fields are an analyzer error elsewhere but we surface them
    // here as well so the failure mode is consistent.
    if (field.name.empty()) {
      return absl::InvalidArgumentError(absl::StrCat(
          "field '",
          nested_path,
          "': STRUCT field #",
          i,
          " has no name; BigQuery requires a name for every nested field"));
    }
    nested->set_name(field.name);
    const std::string child_path =
        nested_path.empty() ? field.name
                            : absl::StrCat(nested_path, ".", field.name);
    if (field.type != nullptr && field.type->IsArray()) {
      absl::Status s = FillArrayField(field.type, child_path, nested);
      if (!s.ok()) return s;
    } else {
      absl::Status s = FillScalarOrStruct(field.type, child_path, nested);
      if (!s.ok()) return s;
    }
  }
  return absl::OkStatus();
}

// Fills `*out->type` + `out->fields` for a struct or scalar type.
// `out->name` is set by the caller; `out->mode` is set by the caller
// when needed (the public entrypoint handles REPEATED). `nested_path`
// is only used in error messages so a malformed leaf field inside a
// deeply nested STRUCT names the offending path.
absl::Status FillScalarOrStruct(const ::googlesql::Type* type,
                                const std::string& nested_path,
                                v1::FieldSchema* out) {
  if (type == nullptr) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", nested_path, "': null type"));
  }
  const absl::string_view scalar = BqScalarName(type->kind());
  if (!scalar.empty()) {
    out->set_type(std::string(scalar));
    return absl::OkStatus();
  }
  if (type->IsStruct()) {
    out->set_type("STRUCT");
    return FillStructFields(type->AsStruct(), nested_path, out);
  }
  // Unsupported kinds (PROTO, ENUM, INTERVAL, RANGE, GRAPH_ELEMENT,
  // EXTENDED, UUID, ...) fall through here. Surface the GoogleSQL
  // debug name so the analyzer error message is actionable.
  return absl::InvalidArgumentError(
      absl::StrCat("field '",
                   nested_path,
                   "': unsupported GoogleSQL type for BigQuery reflection: ",
                   type->DebugString()));
}

}  // namespace

absl::Status TypeToFieldSchema(const ::googlesql::Type* type,
                               absl::string_view name,
                               v1::FieldSchema* out) {
  if (out == nullptr) {
    return absl::InvalidArgumentError("TypeToFieldSchema: output is null");
  }
  out->Clear();
  out->set_name(std::string(name));
  if (type == nullptr) {
    return absl::InvalidArgumentError(
        absl::StrCat("TypeToFieldSchema: column '", name, "' has null type"));
  }
  if (type->IsArray()) {
    const ::googlesql::Type* element = type->AsArray()->element_type();
    if (element != nullptr && element->IsArray()) {
      return absl::InvalidArgumentError(absl::StrCat(
          "TypeToFieldSchema: column '",
          name,
          "': ARRAY<ARRAY<...>> is not representable in BigQuery"));
    }
    absl::Status s = FillScalarOrStruct(element, std::string(name), out);
    if (!s.ok()) return s;
    out->set_mode("REPEATED");
    return absl::OkStatus();
  }
  return FillScalarOrStruct(type, std::string(name), out);
}

absl::Status OutputColumnListToTableSchema(
    const std::vector<std::unique_ptr<const ::googlesql::ResolvedOutputColumn>>&
        output_columns,
    v1::TableSchema* out) {
  if (out == nullptr) {
    return absl::InvalidArgumentError(
        "OutputColumnListToTableSchema: output is null");
  }
  out->Clear();
  for (const auto& column : output_columns) {
    if (column == nullptr) {
      return absl::InvalidArgumentError(
          "OutputColumnListToTableSchema: null ResolvedOutputColumn entry");
    }
    v1::FieldSchema* field = out->add_fields();
    absl::Status s =
        TypeToFieldSchema(column->column().type(), column->name(), field);
    if (!s.ok()) return s;
  }
  return absl::OkStatus();
}

}  // namespace schema
}  // namespace backend
}  // namespace bigquery_emulator
