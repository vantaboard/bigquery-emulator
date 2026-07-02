#include "frontend/handlers/storage_read_internal.h"

#include <cstddef>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

namespace {

std::string NormalizeTimestampOffsetSuffix(std::string text) {
  if (text.size() >= 3) {
    const size_t n = text.size();
    const char sign = text[n - 3];
    const char d0 = text[n - 2];
    const char d1 = text[n - 1];
    if ((sign == '+' || sign == '-') && absl::ascii_isdigit(d0) &&
        absl::ascii_isdigit(d1) && text[n - 3] == sign) {
      if (text.find(':') == std::string::npos &&
          text.find('T') == std::string::npos) {
        return text;
      }
      absl::StrAppend(&text, ":00");
    }
  }
  return text;
}

bool IsAllDigits(absl::string_view s) {
  if (s.empty()) return false;
  for (char c : s) {
    if (!absl::ascii_isdigit(static_cast<unsigned char>(c))) return false;
  }
  return true;
}

absl::StatusOr<absl::Time> ParseTimestampWireString(absl::string_view body) {
  std::string text(body);
  if (absl::EndsWith(text, " UTC")) {
    text.resize(text.size() - 4);
  }
  text = NormalizeTimestampOffsetSuffix(std::move(text));
  absl::Time t;
  std::string err;
  if (absl::ParseTime(absl::RFC3339_full, text, &t, &err) ||
      absl::ParseTime("%Y-%m-%d %H:%M:%E*S%Ez", text, &t, &err) ||
      absl::ParseTime("%Y-%m-%dT%H:%M:%E*S%Ez", text, &t, &err) ||
      absl::ParseTime("%Y-%m-%d %H:%M:%E*S", text, &t, &err) ||
      absl::ParseTime("%Y-%m-%dT%H:%M:%E*S", text, &t, &err) ||
      absl::ParseTime("%Y-%m-%d", text, &t, &err)) {
    return t;
  }
  return absl::InvalidArgumentError(
      absl::StrCat("StorageRead: invalid TIMESTAMP value '", body, "'"));
}

}  // namespace

// SchemasEqualByShape compares two `TableSchema` snapshots column-by-
// column, looking only at the bits the handler cares about for drift
// detection: column count, column names, BigQuery types, and modes
// (NULLABLE / REQUIRED / REPEATED). Descriptions and nested struct
// field details are intentionally ignored — they can change without
// invalidating an already-served stream, and pinning them would
// surface noisy false-positive FAILED_PRECONDITION replies when the
// catalog handler updates them out-of-band.
bool SchemasEqualByShape(const backend::schema::TableSchema& a,
                         const backend::schema::TableSchema& b) {
  if (a.columns.size() != b.columns.size()) return false;
  for (size_t i = 0; i < a.columns.size(); ++i) {
    if (a.columns[i].name != b.columns[i].name) return false;
    if (a.columns[i].type != b.columns[i].type) return false;
    if (a.columns[i].mode != b.columns[i].mode) return false;
  }
  return true;
}

// FindColumnByName returns the index of the column named `name` in
// `schema`, or `npos` if no top-level column has that name. Used to
// validate `selected_fields` at CreateReadSession time so a typo
// surfaces as INVALID_ARGUMENT before the streaming RPC starts.
std::size_t FindColumnByName(const backend::schema::TableSchema& schema,
                             absl::string_view name) {
  for (std::size_t i = 0; i < schema.columns.size(); ++i) {
    if (schema.columns[i].name == name) return i;
  }
  return kColumnNotFound;
}

// Builds a projected TableSchema from `schema` containing only the
// columns named by `field_names`, in the order they appear in
// `field_names`. The caller must have already validated each name
// against `schema` (CreateReadSession does this); a still-unknown
// name here is a programming error.
backend::schema::TableSchema ProjectSchemaForResponse(
    const backend::schema::TableSchema& schema,
    const std::vector<std::string>& field_names) {
  backend::schema::TableSchema out;
  out.columns.reserve(field_names.size());
  for (const std::string& name : field_names) {
    const std::size_t idx = FindColumnByName(schema, name);
    if (idx != kColumnNotFound) {
      out.columns.push_back(schema.columns[idx]);
    }
  }
  return out;
}

absl::StatusOr<std::string> TimestampValueToMicrosString(
    const backend::storage::Value& value) {
  using Kind = backend::storage::Value::Kind;
  if (value.kind() == Kind::kNull) {
    return absl::InvalidArgumentError("StorageRead: TIMESTAMP cell is NULL");
  }
  if (value.kind() == Kind::kInt64) {
    return absl::StrCat(value.int64_value());
  }
  if (value.kind() != Kind::kString) {
    return absl::InvalidArgumentError(
        "StorageRead: TIMESTAMP cell has unsupported kind");
  }
  const absl::string_view text = value.string_value();
  if (IsAllDigits(text)) {
    return std::string(text);
  }
  auto parsed = ParseTimestampWireString(text);
  if (!parsed.ok()) return parsed.status();
  return absl::StrCat(absl::ToUnixMicros(*parsed));
}

void ValueToCellForStorageRead(const backend::storage::Value& value,
                               backend::schema::ColumnType column_type,
                               v1::Cell* out) {
  if (column_type == backend::schema::ColumnType::kTimestamp &&
      value.kind() != backend::storage::Value::Kind::kNull) {
    auto micros = TimestampValueToMicrosString(value);
    if (micros.ok()) {
      out->Clear();
      out->set_string_value(*micros);
      return;
    }
  }
  ValueToCell(value, out);
}

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
