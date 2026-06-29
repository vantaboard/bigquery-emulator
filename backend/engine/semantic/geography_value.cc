
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "absl/status/status.h"
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

constexpr size_t kMetadataSize = sizeof(int64_t);
constexpr size_t kGeographyPtrOffset = kMetadataSize;

int64_t MetadataWord(const Value& value) {
  int64_t meta = 0;
  std::memcpy(&meta, &value, kMetadataSize);
  return meta;
}

// The prebuilt stub's EmptyGeography() aborts; flip the is_null bit on
// NullGeography metadata using INT64 null/non-null as the mask reference.
int64_t NonNullGeographyMetadata() {
  const Value null_int64{Value::NullInt64()};
  const Value non_null_int64{Value::Int64(0)};
  int64_t null_bit_mask = 0;
  null_bit_mask = MetadataWord(null_int64) ^ MetadataWord(non_null_int64);
  const Value null_geo{Value::NullGeography()};
  return MetadataWord(null_geo) ^ null_bit_mask;
}

const GeographyRef* GeographyRefFromValue(const Value& value) {
  const GeographyRef* ref = nullptr;
  std::memcpy(
      &ref,
      reinterpret_cast<const unsigned char*>(&value) + kGeographyPtrOffset,
      sizeof(ref));
  return ref;
}

Value MakeNonNullGeographyValue(GeographyRef* ref) {
  const int64_t meta = NonNullGeographyMetadata();
  Value out;
  std::memcpy(&out, &meta, kMetadataSize);
  std::memcpy(reinterpret_cast<unsigned char*>(&out) + kGeographyPtrOffset,
              &ref,
              sizeof(ref));
  return out;
}

}  // namespace

::googlesql::Value GeographyFromWkt(std::string wkt) {
  auto* ref = new GeographyRef();
  {
    absl::MutexLock lock(&g_geography_wkt_mu);
    g_geography_wkt[ref] = std::move(wkt);
  }
  return MakeNonNullGeographyValue(ref);
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
    return absl::StrCat("ST_GEOGPOINT(",
                        inner.substr(0, space),
                        ", ",
                        inner.substr(space + 1),
                        ")");
  }
  return absl::StrCat("ST_GEOGFROMTEXT('", wkt, "')");
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
