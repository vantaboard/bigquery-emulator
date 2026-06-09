#include "backend/engine/semantic/functions/geog_funcs.h"

#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/functions/string_funcs.h"
#include "backend/engine/semantic/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

absl::StatusOr<double> CoerceDouble(const Value& v) {
  if (v.is_null()) {
    return absl::InvalidArgumentError("ST_GEOGPOINT argument cannot be NULL");
  }
  if (v.type_kind() == ::googlesql::TYPE_INT64) {
    return static_cast<double>(v.int64_value());
  }
  if (v.type_kind() == ::googlesql::TYPE_DOUBLE) {
    return v.double_value();
  }
  if (v.type_kind() == ::googlesql::TYPE_FLOAT) {
    return static_cast<double>(v.float_value());
  }
  return absl::InvalidArgumentError(
      "ST_GEOGPOINT expects numeric longitude/latitude arguments");
}

// typeof() matches FORMAT('%T', input) text; thread-local coords from the
// most recent ST_GEOGPOINT call when GetSQLLiteral cannot render GEOGRAPHY.
thread_local double g_last_geog_lng = 0.0;
thread_local double g_last_geog_lat = 0.0;

}  // namespace

absl::StatusOr<std::string> GeographyTypeLiteralForFormat() {
  return absl::StrCat(
      "ST_GEOGPOINT(", g_last_geog_lng, ", ", g_last_geog_lat, ")");
}

absl::StatusOr<Value> StGeogPoint(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: ST_GEOGPOINT expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullGeography();
  }
  auto lng = CoerceDouble(args[0]);
  if (!lng.ok()) return lng.status();
  auto lat = CoerceDouble(args[1]);
  if (!lat.ok()) return lat.status();
  g_last_geog_lng = *lng;
  g_last_geog_lat = *lat;
  // EmptyGeography() is ABSL_CHECK(false) in the prebuilt artifact.
  // NullGeography() is typed GEOGRAPHY; FORMAT('%T', ...) uses
  // GeographyTypeLiteralForFormat() for that type kind.
  return Value::NullGeography();
}

absl::StatusOr<Value> EmuFormatTypeLiteral(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: emu_format_t expects one argument");
  }
  if (args[0].type_kind() == ::googlesql::TYPE_GEOGRAPHY) {
    auto lit = GeographyTypeLiteralForFormat();
    if (!lit.ok()) return lit.status();
    return Value::String(*lit);
  }
  if (args[0].is_null()) {
    return Value::String("NULL");
  }
  if (args[0].type()->IsRange()) {
    return Value::String(args[0].GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL));
  }
  std::vector<Value> format_args = {Value::String("%T"), args[0]};
  return FormatString(format_args);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
