#include "backend/engine/semantic/functions/hll_funcs.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/hll_plus_plus.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

constexpr int64_t kMinPrecision = 10;
constexpr int64_t kMaxPrecision = 24;
constexpr uint64_t kHashK0 = 0xa5b85c5e198ed849ULL;
constexpr uint64_t kHashK1 = 0x8d58ac26afe12e47ULL;
constexpr uint64_t kHashK2 = 0xc47b6e9e3a970ed3ULL;
constexpr uint64_t kHashK3 = 0xc6a4a7935bd1e995ULL;

inline uint64_t RotateRight(uint64_t x, int bits) {
  return (x >> bits) | (x << (64 - bits));
}

uint64_t ShiftMix(uint64_t value) {
  return value ^ (value >> 47);
}

uint64_t Load64(absl::string_view bytes, size_t offset) {
  uint64_t out = 0;
  std::memcpy(&out, bytes.data() + offset, sizeof(out));
  return out;
}

uint64_t Load64Safely(absl::string_view bytes, size_t offset, size_t len) {
  uint64_t out = 0;
  for (size_t i = 0; i < len; ++i) {
    out |= static_cast<uint64_t>(static_cast<unsigned char>(bytes[offset + i]))
           << (8 * i);
  }
  return out;
}

uint64_t Hash128To64(uint64_t high, uint64_t low) {
  uint64_t a = (low ^ high) * kHashK3;
  a ^= a >> 47;
  uint64_t b = (high ^ a) * kHashK3;
  b ^= b >> 47;
  b *= kHashK3;
  return b;
}

uint64_t MurmurHash64WithSeed(absl::string_view bytes, uint64_t seed) {
  const uint64_t mul = kHashK3;
  const size_t aligned = bytes.size() & ~static_cast<size_t>(0x7);
  const size_t remainder = bytes.size() & static_cast<size_t>(0x7);
  uint64_t hash = seed ^ (bytes.size() * mul);

  for (size_t i = 0; i < aligned; i += 8) {
    uint64_t loaded = Load64(bytes, i);
    uint64_t data = ShiftMix(loaded * mul) * mul;
    hash ^= data;
    hash *= mul;
  }
  if (remainder != 0) {
    uint64_t data = Load64Safely(bytes, aligned, remainder);
    hash ^= data;
    hash *= mul;
  }
  hash = ShiftMix(hash) * mul;
  hash = ShiftMix(hash);
  return hash;
}

void WeakHashLength32WithSeeds(absl::string_view bytes,
                               size_t offset,
                               uint64_t seed_a,
                               uint64_t seed_b,
                               uint64_t* out0,
                               uint64_t* out1) {
  uint64_t part1 = Load64(bytes, offset);
  uint64_t part2 = Load64(bytes, offset + 8);
  uint64_t part3 = Load64(bytes, offset + 16);
  uint64_t part4 = Load64(bytes, offset + 24);
  seed_a += part1;
  seed_b = RotateRight(seed_b + seed_a + part4, 51);
  uint64_t c = seed_a;
  seed_a += part2;
  seed_a += part3;
  seed_b += RotateRight(seed_a, 23);
  *out0 = seed_a + part4;
  *out1 = seed_b + c;
}

uint64_t HashLength33To64(absl::string_view bytes) {
  const size_t length = bytes.size();
  uint64_t z = Load64(bytes, 24);
  uint64_t a =
      Load64(bytes, 0) + (length + Load64(bytes, length - 16)) * kHashK0;
  uint64_t b = RotateRight(a + z, 52);
  uint64_t c = RotateRight(a, 37);
  a += Load64(bytes, 8);
  c += RotateRight(a, 7);
  a += Load64(bytes, 16);
  uint64_t vf = a + z;
  uint64_t vs = b + RotateRight(a, 31) + c;
  a = Load64(bytes, 16) + Load64(bytes, length - 32);
  z = Load64(bytes, length - 8);
  b = RotateRight(a + z, 52);
  c = RotateRight(a, 37);
  a += Load64(bytes, length - 24);
  c += RotateRight(a, 7);
  a += Load64(bytes, length - 16);
  uint64_t wf = a + z;
  uint64_t ws = b + RotateRight(a, 31) + c;
  uint64_t r = ShiftMix((vf + ws) * kHashK2 + (wf + vs) * kHashK0);
  return ShiftMix(r * kHashK0 + vs) * kHashK2;
}

uint64_t FullFingerprint(absl::string_view bytes) {
  size_t length = bytes.size();
  size_t offset = 0;
  uint64_t x = Load64(bytes, 0);
  uint64_t y = Load64(bytes, length - 16) ^ kHashK1;
  uint64_t z = Load64(bytes, length - 56) ^ kHashK0;
  uint64_t v0 = 0, v1 = 0, w0 = 0, w1 = 0;
  WeakHashLength32WithSeeds(bytes, length - 64, length, y, &v0, &v1);
  WeakHashLength32WithSeeds(
      bytes, length - 32, length * kHashK1, kHashK0, &w0, &w1);
  z += ShiftMix(v1) * kHashK1;
  x = RotateRight(z + x, 39) * kHashK1;
  y = RotateRight(y, 33) * kHashK1;
  length = (length - 1) & ~static_cast<size_t>(63);
  do {
    x = RotateRight(x + y + v0 + Load64(bytes, offset + 16), 37) * kHashK1;
    y = RotateRight(y + v1 + Load64(bytes, offset + 48), 42) * kHashK1;
    x ^= w1;
    y ^= v0;
    z = RotateRight(z ^ w0, 33);
    WeakHashLength32WithSeeds(bytes, offset, v1 * kHashK1, x + w0, &v0, &v1);
    WeakHashLength32WithSeeds(bytes, offset + 32, z + w1, y, &w0, &w1);
    uint64_t tmp = z;
    z = x;
    x = tmp;
    offset += 64;
    length -= 64;
  } while (length != 0);
  return Hash128To64(Hash128To64(v0, w0) + ShiftMix(y) * kHashK1 + z,
                     Hash128To64(v1, w1) + x);
}

uint64_t Fingerprint2011(absl::string_view bytes) {
  uint64_t result = 0;
  if (bytes.size() <= 32) {
    result = MurmurHash64WithSeed(bytes, kHashK0 ^ kHashK1 ^ kHashK2);
  } else if (bytes.size() <= 64) {
    result = HashLength33To64(bytes);
  } else {
    result = FullFingerprint(bytes);
  }
  uint64_t u = bytes.size() >= 8 ? Load64(bytes, 0) : kHashK0;
  uint64_t v = bytes.size() >= 9 ? Load64(bytes, bytes.size() - 8) : kHashK0;
  result = Hash128To64(result + v, u);
  if (result == 0 || result == 1) return result + ~static_cast<uint64_t>(1);
  return result;
}

uint64_t HashInt64(int64_t value) {
  std::string bytes(8, '\0');
  for (int i = 0; i < 8; ++i) {
    bytes[i] =
        static_cast<char>((static_cast<uint64_t>(value) >> (8 * i)) & 0xff);
  }
  return Fingerprint2011(bytes);
}

absl::StatusOr<int> ParseAndValidatePrecision(
    std::optional<int64_t> precision) {
  if (!precision.has_value()) return 15;
  if (*precision < kMinPrecision || *precision > kMaxPrecision) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("HLL_COUNT.INIT precision must be in [",
                     kMinPrecision,
                     ", ",
                     kMaxPrecision,
                     "]"));
  }
  return static_cast<int>(*precision);
}

absl::StatusOr<int> ValueType(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return hll_plus_plus::kValueTypeUint64;
    case ::googlesql::TYPE_STRING:
    case ::googlesql::TYPE_BYTES:
      return hll_plus_plus::kValueTypeBytesOrUtf8String;
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "HLL_COUNT currently supports INT64, STRING and BYTES inputs");
  }
}

uint64_t HashInputValue(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return HashInt64(v.int64_value());
    case ::googlesql::TYPE_STRING:
      return Fingerprint2011(v.string_value());
    default:
      return Fingerprint2011(v.bytes_value());
  }
}

absl::StatusOr<hll_plus_plus::Sketch> ParseSketch(const Value& v) {
  if (v.type_kind() != ::googlesql::TYPE_BYTES) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "HLL_COUNT sketches must be BYTES");
  }
  auto sketch = hll_plus_plus::Sketch::Parse(v.bytes_value());
  if (!sketch.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             sketch.status().message());
  }
  return sketch;
}

absl::StatusOr<std::optional<hll_plus_plus::Sketch>> MergeSketches(
    const std::vector<std::vector<Value>>& input_column_values,
    absl::string_view function) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             absl::StrCat(function, " expects one argument"));
  }
  std::optional<hll_plus_plus::Sketch> merged;
  for (const Value& v : input_column_values[0]) {
    if (v.is_null()) continue;
    auto sketch = ParseSketch(v);
    if (!sketch.ok()) return sketch.status();
    if (!merged.has_value()) {
      merged = std::move(*sketch);
      continue;
    }
    if (absl::Status status = merged->Merge(*sketch); !status.ok()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               status.message());
    }
  }
  return merged;
}

}  // namespace

absl::StatusOr<Value> HllCountInitAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  (void)call;
  if (input_column_values.empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "HLL_COUNT.INIT expects one argument");
  }
  std::optional<int64_t> precision = std::nullopt;
  if (input_column_values.size() >= 2) {
    for (const Value& v : input_column_values[1]) {
      if (v.is_null()) continue;
      if (v.type_kind() != ::googlesql::TYPE_INT64) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 "HLL_COUNT.INIT precision must be INT64");
      }
      precision = v.int64_value();
      break;
    }
  }
  return HllCountInitValues(input_column_values[0], precision);
}

absl::StatusOr<Value> HllCountInitValues(const std::vector<Value>& input_values,
                                         std::optional<int64_t> precision) {
  auto checked_precision = ParseAndValidatePrecision(precision);
  if (!checked_precision.ok()) return checked_precision.status();
  std::optional<hll_plus_plus::Sketch> sketch;
  for (const Value& v : input_values) {
    if (v.is_null()) continue;
    if (!sketch.has_value()) {
      auto value_type = ValueType(v);
      if (!value_type.ok()) return value_type.status();
      auto created = hll_plus_plus::Sketch::Create(
          *checked_precision,
          std::min(*checked_precision + 5,
                   hll_plus_plus::kMaximumSparsePrecision),
          *value_type);
      if (!created.ok()) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 created.status().message());
      }
      sketch = std::move(*created);
    }
    sketch->AddHash(HashInputValue(v));
  }
  if (!sketch.has_value()) return Value::NullBytes();
  return Value::Bytes(sketch->Serialize());
}

absl::StatusOr<Value> HllCountMergeAggregate(
    const std::vector<std::vector<Value>>& input_column_values) {
  auto merged = MergeSketches(input_column_values, "HLL_COUNT.MERGE");
  if (!merged.ok()) return merged.status();
  if (!merged->has_value()) return Value::Int64(0);
  return Value::Int64((*merged)->Estimate());
}

absl::StatusOr<Value> HllCountMergePartialAggregate(
    const std::vector<std::vector<Value>>& input_column_values) {
  auto merged = MergeSketches(input_column_values, "HLL_COUNT.MERGE_PARTIAL");
  if (!merged.ok()) return merged.status();
  if (!merged->has_value()) return Value::NullBytes();
  return Value::Bytes((*merged)->Serialize());
}

absl::StatusOr<Value> HllCountExtractScalar(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "HLL_COUNT.EXTRACT expects one argument");
  }
  if (args[0].is_null()) return Value::Int64(0);
  auto sketch = ParseSketch(args[0]);
  if (!sketch.ok()) return sketch.status();
  return Value::Int64(sketch->Estimate());
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
