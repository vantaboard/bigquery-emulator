#include "backend/engine/semantic/functions/specialized_funcs.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
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
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/net.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

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
    return absl::InvalidArgumentError("NET.IP_FROM_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError("NET.IP_FROM_STRING expects STRING");
  }
  std::string out;
  absl::Status error;
  if (!::googlesql::functions::net::IPFromString(args[0].string_value(), &out,
                                                   &error)) {
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
                                                      absl::string_view*, bool*)) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        absl::StrCat(name, " expects one STRING argument"));
  }
  if (args[0].is_null()) return Value::NullString();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError(
        absl::StrCat(name, " expects STRING"));
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
  return NetUrlString("NET.PUBLIC_SUFFIX", args,
                      ::googlesql::functions::net::PublicSuffix);
}

absl::StatusOr<Value> NetRegDomain(const std::vector<Value>& args) {
  if (!args.empty() && !args[0].is_null() &&
      args[0].type_kind() == ::googlesql::TYPE_STRING &&
      NetUrlHasInvalidHost(args[0].string_value())) {
    return Value::NullString();
  }
  return NetUrlString("NET.REG_DOMAIN", args,
                      ::googlesql::functions::net::RegDomain);
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

struct RowValue {
  Value v;
  bool is_null = false;
};

std::vector<RowValue> CollectAggregateInputs(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> out;
  if (call.argument_list_size() == 0) return out;
  const size_t nrows = input_column_values.empty() ? 0 : input_column_values[0].size();
  out.reserve(nrows);
  for (size_t r = 0; r < nrows; ++r) {
    RowValue row;
    if (call.argument_list_size() > 0) {
      const Value& cell = input_column_values[0][r];
      row.is_null = cell.is_null();
      row.v = cell;
    }
    out.push_back(std::move(row));
  }
  return out;
}

absl::StatusOr<Value> ApproxCountDistinct(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> rows = CollectAggregateInputs(call, input_column_values);
  std::set<std::string> seen;
  for (const RowValue& row : rows) {
    if (row.is_null) continue;
    seen.insert(row.v.DebugString());
  }
  return Value::Int64(static_cast<int64_t>(seen.size()));
}

struct SortedNumeric {
  std::vector<double> values;
  bool have_null = false;
};

SortedNumeric SortedNumericValues(const std::vector<RowValue>& rows,
                                  bool distinct) {
  SortedNumeric out;
  std::set<std::string> seen;
  for (const RowValue& row : rows) {
    if (row.is_null) {
      out.have_null = true;
      continue;
    }
    if (row.v.type_kind() != ::googlesql::TYPE_INT64 &&
        row.v.type_kind() != ::googlesql::TYPE_DOUBLE) {
      continue;
    }
    const std::string key = row.v.DebugString();
    if (distinct) {
      if (!seen.insert(key).second) continue;
    }
    out.values.push_back(row.v.type_kind() == ::googlesql::TYPE_INT64
                             ? static_cast<double>(row.v.int64_value())
                             : row.v.double_value());
  }
  std::sort(out.values.begin(), out.values.end());
  return out;
}

absl::StatusOr<Value> ApproxQuantiles(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  if (call.argument_list_size() < 2) {
    return absl::InvalidArgumentError("APPROX_QUANTILES expects two arguments");
  }
  const bool distinct = call.distinct();
  const bool respect_nulls =
      call.null_handling_modifier() ==
      ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS;
  std::vector<RowValue> rows = CollectAggregateInputs(call, input_column_values);
  SortedNumeric sorted = SortedNumericValues(rows, distinct);
  int64_t n = 2;
  if (input_column_values.size() > 1 && !input_column_values[1].empty() &&
      !input_column_values[1][0].is_null() &&
      input_column_values[1][0].type_kind() == ::googlesql::TYPE_INT64) {
    n = input_column_values[1][0].int64_value();
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                         : nullptr;
  std::vector<Value> elements;
  elements.reserve(static_cast<size_t>(n) + 1);
  const std::vector<double>& vals = sorted.values;
  if (vals.empty() && !(respect_nulls && sorted.have_null)) {
    for (int64_t i = 0; i <= n; ++i) {
      elements.push_back(Value::NullInt64());
    }
  } else {
    int64_t null_slots = 0;
    if (respect_nulls && sorted.have_null) {
      null_slots = distinct ? 1 : static_cast<int64_t>(std::count_if(
          rows.begin(), rows.end(),
          [](const RowValue& row) { return row.is_null; }));
    }
    const int64_t population =
        null_slots + static_cast<int64_t>(vals.size());
    for (int64_t i = 0; i <= n; ++i) {
      if (population <= 0) {
        elements.push_back(Value::NullInt64());
        continue;
      }
      const double pos =
          static_cast<double>(i) * static_cast<double>(population - 1) /
          static_cast<double>(n);
      const int64_t idx =
          static_cast<int64_t>(pos < 0 ? 0
                                        : std::min(pos, static_cast<double>(
                                                           population - 1)));
      if (idx < null_slots) {
        elements.push_back(Value::NullInt64());
        continue;
      }
      const size_t val_idx =
          static_cast<size_t>(idx - null_slots);
      if (vals.empty()) {
        elements.push_back(Value::NullInt64());
      } else {
        const size_t pick =
            std::min(val_idx, static_cast<size_t>(vals.size() - 1));
        elements.push_back(
            Value::Int64(static_cast<int64_t>(vals[pick])));
      }
    }
  }
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "APPROX_QUANTILES requires ARRAY return type");
  }
  return Value::Array(arr_type, std::move(elements));
}

struct TopCountEntry {
  Value key;
  int64_t sum = 0;
  bool weight_was_non_null = false;
};

absl::StatusOr<Value> ApproxTopCount(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  std::vector<RowValue> rows = CollectAggregateInputs(call, input_column_values);
  std::map<std::string, TopCountEntry> counts;
  for (const RowValue& row : rows) {
    const std::string key = row.is_null ? "NULL" : row.v.DebugString();
    auto it = counts.find(key);
    if (it == counts.end()) {
      counts.emplace(key, TopCountEntry{row.is_null ? Value::NullString() : row.v,
                                        1, true});
    } else {
      it->second.sum += 1;
    }
  }
  int64_t k = 2;
  if (input_column_values.size() > 1 && !input_column_values[1].empty() &&
      !input_column_values[1][0].is_null()) {
    k = input_column_values[1][0].int64_value();
  }
  std::vector<TopCountEntry> sorted;
  sorted.reserve(counts.size());
  for (auto& kv : counts) sorted.push_back(std::move(kv.second));
  std::sort(sorted.begin(), sorted.end(),
            [](const TopCountEntry& a, const TopCountEntry& b) {
              if (a.sum != b.sum) return a.sum > b.sum;
              return a.key.DebugString() < b.key.DebugString();
            });
  if (sorted.size() > static_cast<size_t>(k)) {
    sorted.resize(static_cast<size_t>(k));
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                         : nullptr;
  const ::googlesql::StructType* struct_type =
      arr_type != nullptr ? arr_type->element_type()->AsStruct() : nullptr;
  std::vector<Value> out_elems;
  for (const TopCountEntry& e : sorted) {
    std::vector<Value> fields = {e.key, Value::Int64(e.sum)};
    if (struct_type != nullptr) {
      out_elems.push_back(Value::Struct(struct_type, std::move(fields)));
    }
  }
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "APPROX_TOP_COUNT requires ARRAY<STRUCT>");
  }
  return Value::Array(arr_type, std::move(out_elems));
}

absl::StatusOr<Value> ApproxTopSum(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  if (input_column_values.size() < 2) {
    return absl::InvalidArgumentError("APPROX_TOP_SUM expects value and weight");
  }
  const size_t nrows = input_column_values[0].size();
  std::map<std::string, TopCountEntry> sums;
  for (size_t r = 0; r < nrows; ++r) {
    const Value& key_v = input_column_values[0][r];
    const Value& weight_v = input_column_values[1][r];
    const std::string key = key_v.is_null() ? "NULL" : key_v.DebugString();
    auto it = sums.find(key);
    if (it == sums.end()) {
      TopCountEntry entry{key_v.is_null() ? Value::NullString() : key_v, 0,
                          false};
      if (!weight_v.is_null() && weight_v.type_kind() == ::googlesql::TYPE_INT64) {
        entry.sum = weight_v.int64_value();
        entry.weight_was_non_null = true;
      }
      sums.emplace(key, std::move(entry));
      it = sums.find(key);
    } else if (!weight_v.is_null() &&
               weight_v.type_kind() == ::googlesql::TYPE_INT64) {
      it->second.sum += weight_v.int64_value();
      it->second.weight_was_non_null = true;
    }
  }
  int64_t k = 2;
  if (input_column_values.size() > 2 && !input_column_values[2].empty() &&
      !input_column_values[2][0].is_null()) {
    k = input_column_values[2][0].int64_value();
  }
  std::vector<TopCountEntry> sorted;
  for (auto& kv : sums) sorted.push_back(std::move(kv.second));
  std::sort(sorted.begin(), sorted.end(),
            [](const TopCountEntry& a, const TopCountEntry& b) {
              if (a.sum != b.sum) return a.sum > b.sum;
              if (a.weight_was_non_null != b.weight_was_non_null) {
                return a.weight_was_non_null > b.weight_was_non_null;
              }
              return a.key.DebugString() < b.key.DebugString();
            });
  if (sorted.size() > static_cast<size_t>(k)) {
    sorted.resize(static_cast<size_t>(k));
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                         : nullptr;
  const ::googlesql::StructType* struct_type =
      arr_type != nullptr ? arr_type->element_type()->AsStruct() : nullptr;
  std::vector<Value> out_elems;
  for (const TopCountEntry& e : sorted) {
    Value weight_val =
        e.weight_was_non_null ? Value::Int64(e.sum) : Value::NullInt64();
    std::vector<Value> fields = {e.key, std::move(weight_val)};
    if (struct_type != nullptr) {
      out_elems.push_back(Value::Struct(struct_type, std::move(fields)));
    }
  }
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "APPROX_TOP_SUM requires ARRAY<STRUCT>");
  }
  return Value::Array(arr_type, std::move(out_elems));
}

absl::StatusOr<Value> ArrayConcatAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  if (call.argument_list_size() < 1) {
    return absl::InvalidArgumentError(
        "ARRAY_CONCAT_AGG expects one array argument");
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                         : nullptr;
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "ARRAY_CONCAT_AGG requires ARRAY return type");
  }
  std::vector<Value> out;
  const size_t nrows =
      input_column_values.empty() ? 0 : input_column_values[0].size();
  for (size_t r = 0; r < nrows; ++r) {
    const Value& arr = input_column_values[0][r];
    if (arr.is_null()) continue;
    for (int i = 0; i < arr.num_elements(); ++i) {
      out.push_back(arr.element(i));
    }
  }
  return Value::Array(arr_type, std::move(out));
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

absl::StatusOr<Value> NullOfAggregateType(const ::googlesql::Type* type) {
  if (type == nullptr) return Value::NullInt64();
  switch (type->kind()) {
    case ::googlesql::TYPE_INT64:
      return Value::NullInt64();
    case ::googlesql::TYPE_DOUBLE:
      return Value::NullDouble();
    case ::googlesql::TYPE_NUMERIC:
      return Value::NullNumeric();
    default:
      return Value::NullInt64();
  }
}

absl::StatusOr<Value> SumAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: SUM expects one argument column");
  }
  if (input_column_values[0].empty()) {
    return NullOfAggregateType(call.type());
  }
  const ::googlesql::Type* out_type = call.type();
  const std::vector<Value>& cells = input_column_values[0];
  bool any_non_null = false;
  switch (cells.front().type_kind()) {
    case ::googlesql::TYPE_INT64: {
      int64_t total = 0;
      for (const Value& v : cells) {
        if (v.is_null()) continue;
        if (v.type_kind() != ::googlesql::TYPE_INT64) {
          return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 "semantic: SUM argument type mismatch");
        }
        any_non_null = true;
        total += v.int64_value();
      }
      if (!any_non_null) return NullOfAggregateType(out_type);
      return Value::Int64(total);
    }
    case ::googlesql::TYPE_DOUBLE: {
      double total = 0;
      for (const Value& v : cells) {
        if (v.is_null()) continue;
        if (v.type_kind() != ::googlesql::TYPE_DOUBLE) {
          return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 "semantic: SUM argument type mismatch");
        }
        any_non_null = true;
        total += v.double_value();
      }
      if (!any_non_null) return NullOfAggregateType(out_type);
      return Value::Double(total);
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: SUM is not implemented for this argument type");
  }
}

absl::StatusOr<Value> EvalAggregateCall(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  if (call.function() == nullptr) {
    return absl::InvalidArgumentError("aggregate call has null function");
  }
  std::string name =
      absl::AsciiStrToLower(call.function()->FullName(/*include_group=*/false));
  if (name.empty()) {
    name = absl::AsciiStrToLower(call.function()->Name());
  }
  if (name == "sum") {
    return SumAggregate(call, input_column_values);
  }
  if (name == "approx_count_distinct") {
    return ApproxCountDistinct(call, input_column_values);
  }
  if (name == "approx_quantiles") {
    return ApproxQuantiles(call, input_column_values, call.type());
  }
  if (name == "approx_top_count") {
    return ApproxTopCount(call, input_column_values, call.type());
  }
  if (name == "approx_top_sum") {
    return ApproxTopSum(call, input_column_values, call.type());
  }
  if (name == "hll_count.init") {
    return HllCountInitAggregate(call, input_column_values);
  }
  if (name == "hll_count.merge") {
    return HllCountMergeAggregate(input_column_values);
  }
  if (name == "hll_count.merge_partial") {
    return HllCountMergePartialAggregate(input_column_values);
  }
  if (name == "array_concat_agg") {
    return ArrayConcatAgg(call, input_column_values, call.type());
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: aggregate '", name, "' is not implemented"));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
