#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/hll_funcs.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/net.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

bool ArgIsNull(const std::vector<Value>& args, size_t i) {
  return i >= args.size() || args[i].is_null();
}

absl::StatusOr<std::string> BytesPayload(const Value& v) {
  if (v.is_null()) return std::string();
  if (v.type_kind() != ::googlesql::TYPE_BYTES) {
    return absl::InvalidArgumentError("expected BYTES");
  }
  return v.bytes_value();
}

absl::StatusOr<int64_t> Int64Arg(const std::vector<Value>& args, size_t i) {
  if (ArgIsNull(args, i)) {
    return absl::InvalidArgumentError("unexpected NULL");
  }
  if (args[i].type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError("expected INT64");
  }
  return args[i].int64_value();
}

absl::StatusOr<Value> ErrorFn(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("ERROR expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError("ERROR message must be STRING");
  }
  return absl::InvalidArgumentError(args[0].string_value());
}

absl::StatusOr<Value> SessionUser(const std::vector<Value>& args) {
  if (!args.empty()) {
    return absl::InvalidArgumentError("SESSION_USER expects no arguments");
  }
  return Value::String("dummy");
}

absl::StatusOr<Value> GenerateUuid(const std::vector<Value>& args) {
  if (!args.empty()) {
    return absl::InvalidArgumentError("GENERATE_UUID expects no arguments");
  }
  return Value::String("00000000-0000-4000-8000-000000000001");
}

absl::StatusOr<std::string> ParseIpv4ToBytes(absl::string_view ip) {
  std::vector<absl::string_view> parts = absl::StrSplit(ip, '.');
  if (parts.size() != 4) {
    return absl::InvalidArgumentError("invalid IPv4 address");
  }
  std::string out;
  out.reserve(4);
  for (absl::string_view part : parts) {
    int byte = 0;
    if (!absl::SimpleAtoi(part, &byte) || byte < 0 || byte > 255) {
      return absl::InvalidArgumentError("invalid IPv4 address");
    }
    out.push_back(static_cast<char>(byte));
  }
  return out;
}

absl::StatusOr<std::string> ParseIpv6ToBytes(absl::string_view ip) {
  std::string compact;
  compact.reserve(ip.size());
  for (char c : ip) {
    if (c != ':') compact.push_back(c);
  }
  if (compact.size() != 32) {
    return absl::InvalidArgumentError("invalid IPv6 address");
  }
  std::string out;
  out.reserve(16);
  for (size_t i = 0; i < compact.size(); i += 2) {
    std::string byte_str = compact.substr(i, 2);
    uint64_t byte = 0;
    if (!absl::SimpleHexAtoi(byte_str, &byte) || byte > 255) {
      return absl::InvalidArgumentError("invalid IPv6 address");
    }
    out.push_back(static_cast<char>(byte));
  }
  return out;
}

absl::StatusOr<Value> NetIpFromString(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "NET.IP_FROM_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError("NET.IP_FROM_STRING expects STRING");
  }
  std::string out;
  absl::Status error;
  if (!::googlesql::functions::net::IPFromString(
          args[0].string_value(), &out, &error)) {
    return error;
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> NetSafeIpFromString(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "NET.SAFE_IP_FROM_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError("NET.SAFE_IP_FROM_STRING expects STRING");
  }
  std::string out;
  bool is_null = false;
  absl::Status st = ::googlesql::functions::net::SafeIPFromString(
      args[0].string_value(), &out, &is_null);
  if (!st.ok()) return st;
  if (is_null) return Value::NullBytes();
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> NetIpToString(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("NET.IP_TO_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  auto bytes = BytesPayload(args[0]);
  if (!bytes.ok()) return bytes.status();
  std::string out;
  absl::Status error;
  if (!::googlesql::functions::net::IPToString(*bytes, &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> NetIpNetMask(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "NET.IP_NET_MASK expects two INT64 arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBytes();
  auto out_bytes = Int64Arg(args, 0);
  if (!out_bytes.ok()) return out_bytes.status();
  auto prefix = Int64Arg(args, 1);
  if (!prefix.ok()) return prefix.status();
  const int64_t n = *out_bytes;
  const int64_t p = *prefix;
  if (n != 4 && n != 16) {
    return absl::InvalidArgumentError(
        "NET.IP_NET_MASK: the first argument must be either 4 or 16");
  }
  const int64_t max_prefix = (n == 4) ? 32 : 128;
  if (p < 0 || p > max_prefix) {
    return absl::InvalidArgumentError(absl::StrCat(
        "NET.IP_NET_MASK: the second argument must be in the range from 0 to ",
        max_prefix));
  }
  std::string mask(n, '\0');
  for (int64_t i = 0; i < p; ++i) {
    mask[static_cast<size_t>(i / 8)] |=
        static_cast<char>(0x80 >> static_cast<int>(i % 8));
  }
  return Value::Bytes(std::move(mask));
}

absl::StatusOr<Value> NetIpTrunc(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError("NET.IP_TRUNC expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBytes();
  auto bytes = BytesPayload(args[0]);
  if (!bytes.ok()) return bytes.status();
  auto prefix_or = Int64Arg(args, 1);
  if (!prefix_or.ok()) return prefix_or.status();
  const int64_t prefix = *prefix_or;
  if (bytes->size() != 4 && bytes->size() != 16) {
    return absl::InvalidArgumentError(
        "NET.IP_TRUNC: length of the first argument must be either 4 or 16");
  }
  const int64_t max_prefix = (bytes->size() == 4) ? 32 : 128;
  if (prefix < 0 || prefix > max_prefix) {
    return absl::InvalidArgumentError(absl::StrCat(
        "NET.IP_TRUNC: length must be in the range from 0 to ", max_prefix));
  }
  std::string out = *bytes;
  for (size_t i = static_cast<size_t>(prefix); i < out.size() * 8; ++i) {
    out[i / 8] &= static_cast<char>(~(0x80 >> (i % 8)));
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> NetIpv4FromInt64(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "NET.IPV4_FROM_INT64 expects one INT64 argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  auto v = Int64Arg(args, 0);
  if (!v.ok()) return v.status();
  const uint64_t u = static_cast<uint64_t>(*v);
  std::string out(4, '\0');
  out[0] = static_cast<char>((u >> 24) & 0xff);
  out[1] = static_cast<char>((u >> 16) & 0xff);
  out[2] = static_cast<char>((u >> 8) & 0xff);
  out[3] = static_cast<char>(u & 0xff);
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> NetUrlString(absl::string_view name,
                                   const std::vector<Value>& args,
                                   absl::Status (*fn)(absl::string_view,
                                                      absl::string_view*,
                                                      bool*)) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        absl::StrCat(name, " expects one STRING argument"));
  }
  if (args[0].is_null()) return Value::NullString();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError(absl::StrCat(name, " expects STRING"));
  }
  absl::string_view out;
  bool is_null = false;
  absl::Status st = fn(args[0].string_value(), &out, &is_null);
  if (!st.ok()) return st;
  if (is_null) return Value::NullString();
  return Value::String(std::string(out));
}

absl::StatusOr<Value> NetHost(const std::vector<Value>& args) {
  return NetUrlString("NET.HOST", args, ::googlesql::functions::net::Host);
}

bool NetUrlHasInvalidHost(absl::string_view url) {
  return url.find("..") != absl::string_view::npos;
}

absl::StatusOr<Value> NetPublicSuffix(const std::vector<Value>& args) {
  if (!args.empty() && !args[0].is_null() &&
      args[0].type_kind() == ::googlesql::TYPE_STRING &&
      NetUrlHasInvalidHost(args[0].string_value())) {
    return Value::NullString();
  }
  return NetUrlString(
      "NET.PUBLIC_SUFFIX", args, ::googlesql::functions::net::PublicSuffix);
}

absl::StatusOr<Value> NetRegDomain(const std::vector<Value>& args) {
  if (!args.empty() && !args[0].is_null() &&
      args[0].type_kind() == ::googlesql::TYPE_STRING &&
      NetUrlHasInvalidHost(args[0].string_value())) {
    return Value::NullString();
  }
  return NetUrlString(
      "NET.REG_DOMAIN", args, ::googlesql::functions::net::RegDomain);
}

absl::StatusOr<Value> NetIpv4ToInt64(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "NET.IPV4_TO_INT64 expects one BYTES argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  auto bytes = BytesPayload(args[0]);
  if (!bytes.ok()) return bytes.status();
  if (bytes->size() != 4) {
    return absl::InvalidArgumentError("NET.IPV4_TO_INT64 expects 4 bytes");
  }
  uint64_t u = 0;
  for (size_t i = 0; i < 4; ++i) {
    u = (u << 8) | static_cast<unsigned char>((*bytes)[i]);
  }
  return Value::Int64(static_cast<int64_t>(u));
}

}  // namespace

std::optional<absl::StatusOr<Value>> DispatchSpecializedScalar(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type) {
  (void)return_type;
  if (name == "error") return ErrorFn(args);
  if (name == "session_user") return SessionUser(args);
  if (name == "generate_uuid") return GenerateUuid(args);
  if (name == "net.ip_from_string") return NetIpFromString(args);
  if (name == "net.safe_ip_from_string") return NetSafeIpFromString(args);
  if (name == "net.ip_to_string") return NetIpToString(args);
  if (name == "net.ip_net_mask") return NetIpNetMask(args);
  if (name == "net.ip_trunc") return NetIpTrunc(args);
  if (name == "net.ipv4_from_int64") return NetIpv4FromInt64(args);
  if (name == "net.ipv4_to_int64") return NetIpv4ToInt64(args);
  if (name == "net.host") return NetHost(args);
  if (name == "net.public_suffix") return NetPublicSuffix(args);
  if (name == "net.reg_domain") return NetRegDomain(args);
  if (name == "hll_count.extract") return HllCountExtractScalar(args);
  return std::nullopt;
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
