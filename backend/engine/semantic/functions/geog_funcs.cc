
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/type.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

constexpr double kEarthRadiusMeters = 6371000.0;
constexpr double kPi = 3.14159265358979323846;
constexpr double kDegreeToRad = kPi / 180.0;

absl::StatusOr<double> CoerceDouble(const Value& v) {
  if (v.is_null()) {
    return absl::InvalidArgumentError("geography argument cannot be NULL");
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
      "geography function expects numeric arguments");
}

double NormalizeLongitude(double lng) {
  double out = std::fmod(lng + 180.0, 360.0);
  if (out < 0.0) {
    out += 360.0;
  }
  return out - 180.0;
}

std::string FormatPointWkt(double lng, double lat) {
  return absl::StrCat("POINT(", lng, " ", lat, ")");
}

struct ParsedGeography {
  enum class Kind { kPoint, kPolygon } kind = Kind::kPoint;
  double point_lng = 0.0;
  double point_lat = 0.0;
  std::vector<std::pair<double, double>> ring{};
};

absl::StatusOr<std::vector<double>> ParseNumberList(absl::string_view body) {
  std::vector<double> nums;
  std::string token;
  for (char c : body) {
    if (c == ',' || c == ' ') {
      if (!token.empty()) {
        double v = 0.0;
        if (!absl::SimpleAtod(token, &v)) {
          return absl::InvalidArgumentError("invalid WKT coordinate");
        }
        nums.push_back(v);
        token.clear();
      }
      continue;
    }
    if (c == '(' || c == ')') {
      continue;
    }
    token.push_back(c);
  }
  if (!token.empty()) {
    double v = 0.0;
    if (!absl::SimpleAtod(token, &v)) {
      return absl::InvalidArgumentError("invalid WKT coordinate");
    }
    nums.push_back(v);
  }
  if (nums.size() % 2 != 0) {
    return absl::InvalidArgumentError("invalid WKT coordinate pair count");
  }
  return nums;
}

absl::StatusOr<ParsedGeography> ParseWkt(absl::string_view wkt) {
  std::string trimmed = std::string(absl::StripAsciiWhitespace(wkt));
  if (trimmed.empty()) {
    return absl::InvalidArgumentError("empty WKT");
  }
  std::string upper = absl::AsciiStrToUpper(trimmed);
  if (absl::StartsWith(upper, "POINT")) {
    size_t open = trimmed.find('(');
    size_t close = trimmed.rfind(')');
    if (open == std::string::npos || close == std::string::npos ||
        close <= open) {
      return absl::InvalidArgumentError("invalid POINT WKT");
    }
    auto nums = ParseNumberList(trimmed.substr(open + 1, close - open - 1));
    if (!nums.ok()) return nums.status();
    if (nums->size() != 2) {
      return absl::InvalidArgumentError("POINT WKT expects two coordinates");
    }
    ParsedGeography out;
    out.kind = ParsedGeography::Kind::kPoint;
    out.point_lng = (*nums)[0];
    out.point_lat = (*nums)[1];
    return out;
  }
  if (absl::StartsWith(upper, "POLYGON")) {
    size_t open = trimmed.find("((");
    size_t close = trimmed.rfind("))");
    if (open == std::string::npos || close == std::string::npos ||
        close <= open + 1) {
      return absl::InvalidArgumentError("invalid POLYGON WKT");
    }
    auto nums = ParseNumberList(trimmed.substr(open + 2, close - open - 2));
    if (!nums.ok()) return nums.status();
    if (nums->size() < 6) {
      return absl::InvalidArgumentError("POLYGON WKT ring too short");
    }
    ParsedGeography out;
    out.kind = ParsedGeography::Kind::kPolygon;
    out.ring.reserve(nums->size() / 2);
    for (size_t i = 0; i + 1 < nums->size(); i += 2) {
      out.ring.emplace_back((*nums)[i], (*nums)[i + 1]);
    }
    return out;
  }
  return absl::InvalidArgumentError(
      "semantic: unsupported geography WKT for MVP");
}

uint32_t ReadUint32LE(const unsigned char* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) |
         (static_cast<uint32_t>(p[3]) << 24);
}

uint32_t ReadUint32BE(const unsigned char* p) {
  return (static_cast<uint32_t>(p[0]) << 24) |
         (static_cast<uint32_t>(p[1]) << 16) |
         (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

double ReadDouble(const unsigned char* p, bool little_endian) {
  double out = 0.0;
  unsigned char buf[8];
  if (little_endian) {
    std::memcpy(buf, p, 8);
  } else {
    for (int i = 0; i < 8; ++i) {
      buf[i] = p[7 - i];
    }
  }
  static_assert(sizeof(double) == 8);
  std::memcpy(&out, buf, 8);
  return out;
}

absl::StatusOr<ParsedGeography> ParseWkbPoint(absl::string_view wkb) {
  if (wkb.size() < 21) {
    return absl::InvalidArgumentError("WKB payload too short for POINT");
  }
  const auto* data = reinterpret_cast<const unsigned char*>(wkb.data());
  if (data[0] != 0 && data[0] != 1) {
    return absl::InvalidArgumentError("invalid WKB byte order");
  }
  const bool little_endian = data[0] == 1;
  const uint32_t raw_type =
      little_endian ? ReadUint32LE(data + 1) : ReadUint32BE(data + 1);
  size_t offset = 5;
  if ((raw_type & 0x20000000u) != 0) {
    offset += 4;
  }
  const uint32_t geom_type = raw_type & 0xffu;
  if (geom_type != 1u) {
    return absl::InvalidArgumentError(
        "semantic: ST_GEOGFROMWKB MVP supports POINT geometries only");
  }
  if (wkb.size() < offset + 16) {
    return absl::InvalidArgumentError("WKB POINT payload truncated");
  }
  const double lng = ReadDouble(data + offset, little_endian);
  const double lat = ReadDouble(data + offset + 8, little_endian);
  if (lat < -90.0 || lat > 90.0) {
    return absl::InvalidArgumentError(
        "ST_GEOGFROMWKB latitude must be between -90 and 90");
  }
  ParsedGeography out;
  out.kind = ParsedGeography::Kind::kPoint;
  out.point_lng = NormalizeLongitude(lng);
  out.point_lat = lat;
  return out;
}

absl::StatusOr<ParsedGeography> ParseGeographyArg(const Value& v) {
  if (v.is_null()) {
    return absl::InvalidArgumentError("unexpected NULL geography");
  }
  if (v.type_kind() != ::googlesql::TYPE_GEOGRAPHY) {
    return absl::InvalidArgumentError("expected GEOGRAPHY argument");
  }
  const std::string wkt = GeographyWkt(v);
  if (wkt.empty()) {
    return absl::InvalidArgumentError("geography value has no WKT payload");
  }
  return ParseWkt(wkt);
}

double HaversineMeters(double lng1, double lat1, double lng2, double lat2) {
  const double phi1 = lat1 * kDegreeToRad;
  const double phi2 = lat2 * kDegreeToRad;
  const double d_phi = (lat2 - lat1) * kDegreeToRad;
  const double d_lambda = (lng2 - lng1) * kDegreeToRad;
  const double a = std::sin(d_phi / 2.0) * std::sin(d_phi / 2.0) +
                   std::cos(phi1) * std::cos(phi2) * std::sin(d_lambda / 2.0) *
                       std::sin(d_lambda / 2.0);
  const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
  return kEarthRadiusMeters * c;
}

bool PointInPolygon(double lng,
                    double lat,
                    const std::vector<std::pair<double, double>>& ring) {
  bool inside = false;
  for (size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
    const double xi = ring[i].first;
    const double yi = ring[i].second;
    const double xj = ring[j].first;
    const double yj = ring[j].second;
    const bool intersect =
        ((yi > lat) != (yj > lat)) &&
        (lng <
         (xj - xi) * (lat - yi) / ((yj - yi) == 0.0 ? 1e-30 : (yj - yi)) + xi);
    if (intersect) {
      inside = !inside;
    }
  }
  return inside;
}

bool PointsEqual(double lng1, double lat1, double lng2, double lat2) {
  return std::abs(lng1 - lng2) < 1e-9 && std::abs(lat1 - lat2) < 1e-9;
}

}  // namespace

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
  if (*lat < -90.0 || *lat > 90.0) {
    return absl::InvalidArgumentError(
        "ST_GEOGPOINT latitude must be between -90 and 90");
  }
  const double norm_lng = NormalizeLongitude(*lng);
  return GeographyFromWkt(FormatPointWkt(norm_lng, *lat));
}

absl::StatusOr<Value> StGeogFromText(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 4) {
    return absl::InvalidArgumentError(
        "semantic: ST_GEOGFROMTEXT expects one to four arguments");
  }
  if (args[0].is_null()) {
    return Value::NullGeography();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError("ST_GEOGFROMTEXT expects STRING WKT");
  }
  auto parsed = ParseWkt(args[0].string_value());
  if (!parsed.ok()) return parsed.status();
  if (parsed->kind == ParsedGeography::Kind::kPoint) {
    return GeographyFromWkt(
        FormatPointWkt(parsed->point_lng, parsed->point_lat));
  }
  std::vector<std::string> pairs;
  pairs.reserve(parsed->ring.size());
  for (const auto& p : parsed->ring) {
    pairs.push_back(absl::StrCat(p.first, " ", p.second));
  }
  return GeographyFromWkt(
      absl::StrCat("POLYGON((", absl::StrJoin(pairs, ", "), "))"));
}

absl::StatusOr<Value> StGeogFromWkb(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: ST_GEOGFROMWKB expects one BYTES argument");
  }
  if (args[0].is_null()) {
    return Value::NullGeography();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_BYTES) {
    return absl::InvalidArgumentError("ST_GEOGFROMWKB expects BYTES WKB");
  }
  const std::string& wkb = args[0].bytes_value();
  auto parsed = ParseWkbPoint(wkb);
  if (!parsed.ok()) return parsed.status();
  return GeographyFromWkt(FormatPointWkt(parsed->point_lng, parsed->point_lat));
}

absl::StatusOr<Value> StAsText(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: ST_ASTEXT expects one argument");
  }
  if (args[0].is_null()) {
    return Value::NullString();
  }
  const std::string wkt = GeographyWkt(args[0]);
  if (wkt.empty()) {
    return absl::InvalidArgumentError("geography value has no WKT payload");
  }
  return Value::String(wkt);
}

absl::StatusOr<Value> StDistance(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: ST_DISTANCE expects two or three arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullDouble();
  }
  auto g1 = ParseGeographyArg(args[0]);
  if (!g1.ok()) return g1.status();
  auto g2 = ParseGeographyArg(args[1]);
  if (!g2.ok()) return g2.status();
  if (g1->kind != ParsedGeography::Kind::kPoint ||
      g2->kind != ParsedGeography::Kind::kPoint) {
    return absl::InvalidArgumentError(
        "semantic: ST_DISTANCE MVP supports POINT geometries only");
  }
  (void)args.size();
  return Value::Double(HaversineMeters(
      g1->point_lng, g1->point_lat, g2->point_lng, g2->point_lat));
}

absl::StatusOr<Value> StIntersects(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: ST_INTERSECTS expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullBool();
  }
  auto g1 = ParseGeographyArg(args[0]);
  if (!g1.ok()) return g1.status();
  auto g2 = ParseGeographyArg(args[1]);
  if (!g2.ok()) return g2.status();

  if (g1->kind == ParsedGeography::Kind::kPoint &&
      g2->kind == ParsedGeography::Kind::kPoint) {
    return Value::Bool(PointsEqual(
        g1->point_lng, g1->point_lat, g2->point_lng, g2->point_lat));
  }
  if (g1->kind == ParsedGeography::Kind::kPolygon &&
      g2->kind == ParsedGeography::Kind::kPoint) {
    return Value::Bool(PointInPolygon(g2->point_lng, g2->point_lat, g1->ring));
  }
  if (g1->kind == ParsedGeography::Kind::kPoint &&
      g2->kind == ParsedGeography::Kind::kPolygon) {
    return Value::Bool(PointInPolygon(g1->point_lng, g1->point_lat, g2->ring));
  }
  if (g1->kind == ParsedGeography::Kind::kPolygon &&
      g2->kind == ParsedGeography::Kind::kPolygon) {
    for (const auto& p : g1->ring) {
      if (PointInPolygon(p.first, p.second, g2->ring)) {
        return Value::Bool(true);
      }
    }
    for (const auto& p : g2->ring) {
      if (PointInPolygon(p.first, p.second, g1->ring)) {
        return Value::Bool(true);
      }
    }
    return Value::Bool(false);
  }
  return absl::InvalidArgumentError(
      "semantic: ST_INTERSECTS MVP supports POINT/POLYGON only");
}

absl::StatusOr<Value> StContains(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: ST_CONTAINS expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullBool();
  }
  auto outer = ParseGeographyArg(args[0]);
  if (!outer.ok()) return outer.status();
  auto inner = ParseGeographyArg(args[1]);
  if (!inner.ok()) return inner.status();

  if (outer->kind == ParsedGeography::Kind::kPolygon &&
      inner->kind == ParsedGeography::Kind::kPoint) {
    return Value::Bool(
        PointInPolygon(inner->point_lng, inner->point_lat, outer->ring));
  }
  if (outer->kind == ParsedGeography::Kind::kPoint &&
      inner->kind == ParsedGeography::Kind::kPoint) {
    return Value::Bool(PointsEqual(outer->point_lng,
                                   outer->point_lat,
                                   inner->point_lng,
                                   inner->point_lat));
  }
  return absl::InvalidArgumentError(
      "semantic: ST_CONTAINS MVP supports POLYGON contains POINT");
}

absl::StatusOr<Value> StWithin(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: ST_WITHIN expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullBool();
  }
  return StContains({args[1], args[0]});
}

absl::StatusOr<Value> EmuFormatTypeLiteral(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: emu_format_t expects one argument");
  }
  if (args[0].type_kind() == ::googlesql::TYPE_GEOGRAPHY) {
    auto lit = GeographySqlLiteral(args[0]);
    if (!lit.ok()) return lit.status();
    return Value::String(*lit);
  }
  if (args[0].is_null()) {
    return Value::String("NULL");
  }
  if (args[0].type()->IsRange()) {
    return Value::String(args[0].GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL));
  }
  Value format_pattern = Value::String("%T");
  std::vector<Value> format_args = {format_pattern, args[0]};
  return FormatString(format_args);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
