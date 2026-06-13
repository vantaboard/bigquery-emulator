#include "backend/engine/semantic/geography_value.h"

#include <cstring>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "googlesql/public/types/value_representations.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

using ::googlesql::TypeKind;
using ::googlesql::Value;
using GeographyRef = ::googlesql::internal::GeographyRef;

absl::Mutex g_geography_wkt_mu;
absl::flat_hash_map<const GeographyRef*, std::string> g_geography_wkt;

int64_t NonNullGeographyMetadata() {
  const Value null_geo = Value::NullGeography();
  const Value json_shell = Value::UnvalidatedJsonString("emu");
  int64_t geo_meta = 0;
  int64_t json_meta = 0;
  std::memcpy(&geo_meta, &null_geo, sizeof(int64_t));
  std::memcpy(&json_meta, &json_shell, sizeof(int64_t));
  const int64_t diff = geo_meta ^ json_meta;
  return (json_meta & ~diff) | (geo_meta & diff);
}

Value ValueFromGeographyRef(GeographyRef* ref) {
  const int64_t meta = NonNullGeographyMetadata();
  alignas(Value) unsigned char bytes[sizeof(Value)];
  std::memcpy(bytes, &meta, sizeof(int64_t));
  std::memcpy(bytes + sizeof(int64_t), &ref, sizeof(ref));
  Value out;
  std::memcpy(&out, bytes, sizeof(Value));
  return out;
}

const GeographyRef* GeographyRefFromValue(const Value& value) {
  const GeographyRef* ref = nullptr;
  std::memcpy(&ref, reinterpret_cast<const unsigned char*>(&value) + sizeof(int64_t),
              sizeof(ref));
  return ref;
}

}  // namespace

::googlesql::Value GeographyFromWkt(std::string wkt) {
  auto* ref = new GeographyRef();
  {
    absl::MutexLock lock(&g_geography_wkt_mu);
    g_geography_wkt[ref] = std::move(wkt);
  }
  return ValueFromGeographyRef(ref);
}

std::string GeographyWkt(const Value& value) {
  if (!value.is_valid() || value.is_null() ||
      value.type_kind() != TypeKind::TYPE_GEOGRAPHY) {
    return {};
  }
  const GeographyRef* ref = GeographyRefFromValue(value);
  if (ref == nullptr) {
    return {};
  }
  absl::MutexLock lock(&g_geography_wkt_mu);
  auto it = g_geography_wkt.find(ref);
  if (it == g_geography_wkt.end()) {
    return {};
  }
  return it->second;
}

absl::StatusOr<std::string> GeographySqlLiteral(const Value& value) {
  if (value.is_null()) {
    return std::string("NULL");
  }
  const std::string wkt = GeographyWkt(value);
  if (wkt.empty()) {
    return absl::InvalidArgumentError(
        "semantic: geography value has no WKT payload");
  }
  if (wkt.rfind("POINT(", 0) == 0) {
    std::string inner = wkt.substr(6);
    if (!inner.empty() && inner.back() == ')') {
      inner.pop_back();
    }
    const size_t space = inner.find(' ');
    if (space == std::string::npos) {
      return absl::InvalidArgumentError("semantic: invalid POINT WKT");
    }
    return absl::StrCat("ST_GEOGPOINT(", inner.substr(0, space), ", ",
                        inner.substr(space + 1), ")");
  }
  return absl::StrCat("ST_GEOGFROMTEXT('", wkt, "')");
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
