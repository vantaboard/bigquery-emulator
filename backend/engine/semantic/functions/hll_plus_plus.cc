#include "backend/engine/semantic/functions/hll_plus_plus.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/numeric/bits.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/functions/hll_plus_plus_data.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace hll_plus_plus {
namespace {

constexpr int kHyperLogLogPlusUniqueType = 112;
constexpr int kEncodingVersion = 2;
constexpr int kMinimumPrecision = 4;
constexpr int kMaximumPrecision = 24;
constexpr int kRhoWBits = 6;
constexpr int32_t kRhoWMask = (1 << kRhoWBits) - 1;

constexpr int kTypeLong = 1 << 0;
constexpr int kTypeInteger = 1 << 1;
constexpr int kTypeString = 1 << 2;
constexpr int kTypeBytes = 1 << 3;
constexpr int kValueTypeUint32 = 7;

constexpr uint32_t kTypeTag = 1 << 3;
constexpr uint32_t kNumValuesTag = 2 << 3;
constexpr uint32_t kEncodingVersionTag = 3 << 3;
constexpr uint32_t kValueTypeTag = 4 << 3;
constexpr uint32_t kHllStateTag = (112 << 3) | 2;
constexpr uint32_t kSparseSizeTag = 2 << 3;
constexpr uint32_t kPrecisionTag = 3 << 3;
constexpr uint32_t kSparsePrecisionTag = 4 << 3;
constexpr uint32_t kDataTag = (5 << 3) | 2;
constexpr uint32_t kSparseDataTag = (6 << 3) | 2;

absl::Status InvalidSketch(absl::string_view reason) {
  return absl::InvalidArgumentError(
      absl::StrCat("Invalid HLL_COUNT sketch: ", reason));
}


void PutVarint(uint64_t value, std::string* out) {
  while (value >= 0x80) {
    out->push_back(static_cast<char>((value & 0x7f) | 0x80));
    value >>= 7;
  }
  out->push_back(static_cast<char>(value));
}

void PutInt32(int32_t value, std::string* out) {
  PutVarint(static_cast<uint64_t>(static_cast<int64_t>(value)), out);
}

class Reader {
 public:
  explicit Reader(absl::string_view bytes) : bytes_(bytes) {}

  bool AtEnd() const {
    return position_ >= bytes_.size();
  }

  absl::StatusOr<uint64_t> Varint() {
    uint64_t result = 0;
    for (int shift = 0; shift < 64; shift += 7) {
      if (AtEnd()) return InvalidSketch("truncated varint");
      const auto byte = static_cast<unsigned char>(bytes_[position_++]);
      result |= static_cast<uint64_t>(byte & 0x7f) << shift;
      if ((byte & 0x80) == 0) return result;
    }
    return InvalidSketch("varint too long");
  }

  absl::StatusOr<absl::string_view> LengthDelimited() {
    auto length = Varint();
    if (!length.ok()) return length.status();
    if (*length > bytes_.size() - position_) {
      return InvalidSketch("truncated field");
    }
    absl::string_view out = bytes_.substr(position_, *length);
    position_ += *length;
    return out;
  }

  absl::Status Skip(uint32_t tag) {
    switch (tag & 7) {
      case 0:
        return Varint().status();
      case 1:
        return Advance(8);
      case 2:
        return LengthDelimited().status();
      case 5:
        return Advance(4);
      default:
        return InvalidSketch("unsupported wire type");
    }
  }

 private:
  absl::Status Advance(size_t bytes) {
    if (bytes > bytes_.size() - position_) {
      return InvalidSketch("truncated field");
    }
    position_ += bytes;
    return absl::OkStatus();
  }

  absl::string_view bytes_;
  size_t position_ = 0;
};


uint8_t ComputeRhoW(uint64_t value, int bits) {
  const uint64_t w = bits >= 64 ? value : value << (64 - bits);
  return w == 0 ? static_cast<uint8_t>(bits + 1)
                : static_cast<uint8_t>(absl::countl_zero(w) + 1);
}

int32_t NormalIndex(uint64_t hash, int precision) {
  return static_cast<int32_t>(hash >> (64 - precision));
}

uint8_t NormalRhoW(uint64_t hash, int precision) {
  return ComputeRhoW(hash, 64 - precision);
}

uint8_t DowngradeRhoW(int32_t index,
                      uint8_t rho_w,
                      int source_precision,
                      int target_precision) {
  if (source_precision == target_precision) return rho_w;
  const uint32_t suffix = static_cast<uint32_t>(index)
                          << (32 - source_precision + target_precision);
  if (suffix == 0) {
    return static_cast<uint8_t>(rho_w + source_precision - target_precision);
  }
  return static_cast<uint8_t>(1 + absl::countl_zero(suffix));
}


int32_t RhoEncodedFlag(int precision, int sparse_precision) {
  return 1 << std::max(sparse_precision, precision + kRhoWBits);
}

int32_t EncodeSparse(int precision,
                     int sparse_precision,
                     int32_t sparse_index,
                     uint8_t sparse_rho_w) {
  const int32_t mask = (1 << (sparse_precision - precision)) - 1;
  if ((sparse_index & mask) != 0) return sparse_index;
  const int32_t normal_index = sparse_index >> (sparse_precision - precision);
  return RhoEncodedFlag(precision, sparse_precision) |
         (normal_index << kRhoWBits) | sparse_rho_w;
}

int32_t EncodeSparseHash(int precision, int sparse_precision, uint64_t hash) {
  const auto sparse_index =
      static_cast<int32_t>(hash >> (64 - sparse_precision));
  return EncodeSparse(precision,
                      sparse_precision,
                      sparse_index,
                      ComputeRhoW(hash, 64 - sparse_precision));
}

int32_t DecodeSparseIndex(int precision, int sparse_precision, int32_t value) {
  const int32_t flag = RhoEncodedFlag(precision, sparse_precision);
  if ((value & flag) == 0) return value;
  return ((value ^ flag) >> kRhoWBits) << (sparse_precision - precision);
}

uint8_t DecodeSparseRhoWIfPresent(int precision,
                                  int sparse_precision,
                                  int32_t value) {
  if ((value & RhoEncodedFlag(precision, sparse_precision)) == 0) return 0;
  return static_cast<uint8_t>(value & kRhoWMask);
}

int32_t DecodeNormalIndex(int precision, int sparse_precision, int32_t value) {
  const int32_t flag = RhoEncodedFlag(precision, sparse_precision);
  if ((value & flag) == 0) return value >> (sparse_precision - precision);
  return (value ^ flag) >> kRhoWBits;
}

uint8_t DecodeNormalRhoW(int precision, int sparse_precision, int32_t value) {
  if ((value & RhoEncodedFlag(precision, sparse_precision)) == 0) {
    return ComputeRhoW(static_cast<uint64_t>(static_cast<int64_t>(value)),
                       sparse_precision - precision);
  }
  return static_cast<uint8_t>((value & kRhoWMask) + sparse_precision -
                              precision);
}

int32_t DowngradeSparseValue(int precision,
                             int sparse_precision,
                             int32_t value,
                             int target_precision,
                             int target_sparse_precision) {
  const int32_t old_index =
      DecodeSparseIndex(precision, sparse_precision, value);
  const uint8_t old_rho_w =
      DecodeSparseRhoWIfPresent(precision, sparse_precision, value);
  const int32_t new_index =
      old_index >> (sparse_precision - target_sparse_precision);
  const uint8_t new_rho_w = DowngradeRhoW(
      old_index, old_rho_w, sparse_precision, target_sparse_precision);
  return EncodeSparse(
      target_precision, target_sparse_precision, new_index, new_rho_w);
}

std::vector<int32_t> Dedupe(int precision,
                            int sparse_precision,
                            const std::vector<int32_t>& sorted) {
  const int32_t flag = RhoEncodedFlag(precision, sparse_precision);
  std::vector<int32_t> out;
  size_t i = 0;
  while (i < sorted.size()) {
    const int32_t next = sorted[i];
    if ((next & flag) == 0) {
      out.push_back(next);
      while (i < sorted.size() && sorted[i] == next)
        ++i;
      continue;
    }
    const int32_t index = DecodeSparseIndex(precision, sparse_precision, next);
    int32_t max_value = next;
    while (i < sorted.size() &&
           DecodeSparseIndex(precision, sparse_precision, sorted[i]) == index) {
      max_value = sorted[i];
      ++i;
    }
    out.push_back(max_value);
  }
  return out;
}

bool IsLessThan(int precision,
                int sparse_precision,
                int other_precision,
                int other_sparse_precision) {
  return precision < other_precision ||
         sparse_precision < other_sparse_precision;
}

absl::Status CheckCompatible(int precision,
                             int sparse_precision,
                             int other_precision,
                             int other_sparse_precision) {
  if ((precision <= other_precision &&
       sparse_precision <= other_sparse_precision) ||
      (precision >= other_precision &&
       sparse_precision >= other_sparse_precision)) {
    return absl::OkStatus();
  }
  return absl::InvalidArgumentError(
      absl::StrCat("HLL_COUNT sketches with precisions (",
                   precision,
                   ", ",
                   sparse_precision,
                   ") and (",
                   other_precision,
                   ", ",
                   other_sparse_precision,
                   ") cannot be merged"));
}

int AllowedTypes(int value_type) {
  switch (value_type) {
    case 0:
      return kTypeLong | kTypeInteger | kTypeString | kTypeBytes;
    case kValueTypeUint64:
      return kTypeLong;
    case kValueTypeUint32:
      return kTypeInteger;
    case kValueTypeBytesOrUtf8String:
      return kTypeString | kTypeBytes;
    default:
      return 0;
  }
}

}  // namespace

Sketch::Sketch(State state) : state_(std::move(state)) {
  sparse_ = !(state_.HasData() || state_.sparse_precision == 0);
  allowed_types_ = AllowedTypes(state_.value_type);
}

absl::Status Sketch::Validate() const {
  if (state_.type != kHyperLogLogPlusUniqueType) {
    return InvalidSketch("not a HYPERLOGLOG_PLUS_UNIQUE state");
  }
  if (state_.encoding_version != kEncodingVersion) {
    return InvalidSketch("unsupported encoding version");
  }
  if (allowed_types_ == 0) return InvalidSketch("unsupported value type");
  if (state_.precision < kMinimumPrecision ||
      state_.precision > kMaximumPrecision) {
    return InvalidSketch("precision out of range");
  }
  if (state_.HasSparseData() && state_.sparse_precision == 0) {
    return InvalidSketch("sparse data without a sparse precision");
  }
  if (sparse_ && (state_.sparse_precision < state_.precision ||
                  state_.sparse_precision > kMaximumSparsePrecision)) {
    return InvalidSketch("sparse precision out of range");
  }
  if (state_.HasData() &&
      state_.data->size() != (size_t{1} << state_.precision)) {
    return InvalidSketch("data does not match the precision");
  }
  return absl::OkStatus();
}

absl::StatusOr<Sketch> Sketch::Create(int precision,
                                      int sparse_precision,
                                      int value_type) {
  State state;
  state.type = kHyperLogLogPlusUniqueType;
  state.encoding_version = kEncodingVersion;
  state.precision = precision;
  state.sparse_precision = sparse_precision;
  state.value_type = value_type;
  Sketch sketch(std::move(state));
  if (absl::Status status = sketch.Validate(); !status.ok()) return status;
  return sketch;
}

absl::StatusOr<Sketch> Sketch::Parse(absl::string_view proto) {
  State state;
  Reader reader(proto);
  while (!reader.AtEnd()) {
    auto tag = reader.Varint();
    if (!tag.ok()) return tag.status();
    const auto key = static_cast<uint32_t>(*tag);
    if (key == kHllStateTag) {
      auto hll = reader.LengthDelimited();
      if (!hll.ok()) return hll.status();
      Reader inner(*hll);
      while (!inner.AtEnd()) {
        auto inner_tag = inner.Varint();
        if (!inner_tag.ok()) return inner_tag.status();
        const auto inner_key = static_cast<uint32_t>(*inner_tag);
        if (inner_key == kDataTag || inner_key == kSparseDataTag) {
          auto bytes = inner.LengthDelimited();
          if (!bytes.ok()) return bytes.status();
          (inner_key == kDataTag ? state.data : state.sparse_data) =
              std::string(*bytes);
        } else if (inner_key == kSparseSizeTag || inner_key == kPrecisionTag ||
                   inner_key == kSparsePrecisionTag) {
          auto value = inner.Varint();
          if (!value.ok()) return value.status();
          const auto as_int = static_cast<int32_t>(*value);
          if (inner_key == kSparseSizeTag) state.sparse_size = as_int;
          if (inner_key == kPrecisionTag) state.precision = as_int;
          if (inner_key == kSparsePrecisionTag) {
            state.sparse_precision = as_int;
          }
        } else if (absl::Status status = inner.Skip(inner_key); !status.ok()) {
          return status;
        }
      }
    } else if (key == kTypeTag || key == kNumValuesTag ||
               key == kEncodingVersionTag || key == kValueTypeTag) {
      auto value = reader.Varint();
      if (!value.ok()) return value.status();
      if (key == kTypeTag) state.type = static_cast<int32_t>(*value);
      if (key == kNumValuesTag) state.num_values = static_cast<int64_t>(*value);
      if (key == kEncodingVersionTag) {
        state.encoding_version = static_cast<int32_t>(*value);
      }
      if (key == kValueTypeTag) state.value_type = static_cast<int32_t>(*value);
    } else if (absl::Status status = reader.Skip(key); !status.ok()) {
      return status;
    }
  }
  Sketch sketch(std::move(state));
  if (absl::Status status = sketch.Validate(); !status.ok()) return status;
  return sketch;
}

Sketch::SparseEncoding Sketch::encoding() const {
  return {state_.precision, state_.sparse_precision};
}

void Sketch::AddHash(uint64_t hash) {
  if (sparse_) {
    buffer_.insert(
        EncodeSparseHash(state_.precision, state_.sparse_precision, hash));
    UpdateRepresentation();
  } else {
    EnsureData();
    const int32_t index = NormalIndex(hash, state_.precision);
    auto& current = reinterpret_cast<uint8_t&>((*state_.data)[index]);
    current = std::max(current, NormalRhoW(hash, state_.precision));
  }
  ++state_.num_values;
}

absl::Status Sketch::Merge(const Sketch& other) {
  const int types = allowed_types_ & other.allowed_types_;
  if (types == 0) {
    return absl::InvalidArgumentError(
        "HLL_COUNT sketches of different value types cannot be merged");
  }
  if (sparse_ && other.sparse_) {
    if (absl::Status status = CheckCompatible(state_.precision,
                                              state_.sparse_precision,
                                              other.state_.precision,
                                              other.state_.sparse_precision);
        !status.ok()) {
      return status;
    }
  }
  allowed_types_ = types;
  if (!sparse_ && !other.sparse_) {
    MaybeDowngrade(other.state_.precision, other.state_.sparse_precision);
    MergeNormalData(other.state_.data, other.state_.precision);
  } else if (!sparse_) {
    other.MergeInto(*this);
  } else if (!other.sparse_) {
    Normalize();
    MaybeDowngrade(other.state_.precision, other.state_.sparse_precision);
    MergeNormalData(other.state_.data, other.state_.precision);
  } else {
    AddSparseValues(other.encoding(), other.SortedValues());
  }
  state_.num_values += other.state_.num_values;
  return absl::OkStatus();
}

int64_t Sketch::Estimate() {
  if (sparse_) {
    FlushBuffer();
    const int64_t buckets = int64_t{1} << state_.sparse_precision;
    const int64_t zeros = buckets - state_.sparse_size;
    const double estimate =
        static_cast<double>(buckets) *
        std::log(static_cast<double>(buckets) / static_cast<double>(zeros));
    return static_cast<int64_t>(std::floor(estimate + 0.5));
  }
  if (!state_.HasData()) return 0;
  int zeros = 0;
  double sum = 0;
  for (char c : *state_.data) {
    const auto v = static_cast<uint8_t>(c);
    if (v == 0) ++zeros;
    sum += 1.0 / static_cast<double>(int64_t{1} << v);
  }
  const double m = static_cast<double>(1 << state_.precision);
  if (zeros > 0) {
    const double h = m * std::log(m / zeros);
    if (h <= LinearCountingThreshold(state_.precision)) {
      return static_cast<int64_t>(std::floor(h + 0.5));
    }
  }
  const double estimate = Alpha(state_.precision) * m * m / sum;
  return static_cast<int64_t>(
      std::floor(estimate - EstimateBias(estimate, state_.precision) + 0.5));
}

std::string Sketch::Serialize() {
  if (sparse_) Compact();

  std::string hll;
  if (state_.sparse_size != 0) {
    PutVarint(kSparseSizeTag, &hll);
    PutInt32(state_.sparse_size, &hll);
  }
  if (state_.precision != 0) {
    PutVarint(kPrecisionTag, &hll);
    PutInt32(state_.precision, &hll);
  }
  if (state_.sparse_precision != 0) {
    PutVarint(kSparsePrecisionTag, &hll);
    PutInt32(state_.sparse_precision, &hll);
  }
  if (state_.data.has_value()) {
    PutVarint(kDataTag, &hll);
    PutVarint(state_.data->size(), &hll);
    hll.append(*state_.data);
  }
  if (state_.sparse_data.has_value()) {
    PutVarint(kSparseDataTag, &hll);
    PutVarint(state_.sparse_data->size(), &hll);
    hll.append(*state_.sparse_data);
  }

  std::string out;
  PutVarint(kTypeTag, &out);
  PutInt32(state_.type, &out);
  PutVarint(kNumValuesTag, &out);
  PutVarint(static_cast<uint64_t>(state_.num_values), &out);
  if (state_.encoding_version != 1) {
    PutVarint(kEncodingVersionTag, &out);
    PutInt32(state_.encoding_version, &out);
  }
  if (state_.value_type != 0) {
    PutVarint(kValueTypeTag, &out);
    PutInt32(state_.value_type, &out);
  }
  PutVarint(kHllStateTag, &out);
  PutVarint(hll.size(), &out);
  out.append(hll);
  return out;
}

void Sketch::AddSparseValue(SparseEncoding source, int32_t value) {
  if (!sparse_) {
    MaybeDowngrade(source.precision, source.sparse_precision);
    EnsureData();
    int32_t index;
    uint8_t rho_w;
    if (state_.precision < source.precision) {
      index =
          DecodeNormalIndex(source.precision, source.sparse_precision, value) >>
          (source.precision - state_.precision);
      rho_w = DowngradeRhoW(
          DecodeNormalIndex(source.precision, source.sparse_precision, value),
          DecodeNormalRhoW(source.precision, source.sparse_precision, value),
          source.precision,
          state_.precision);
    } else {
      index =
          DecodeNormalIndex(source.precision, source.sparse_precision, value);
      rho_w =
          DecodeNormalRhoW(source.precision, source.sparse_precision, value);
    }
    auto& current = reinterpret_cast<uint8_t&>((*state_.data)[index]);
    current = std::max(current, rho_w);
    return;
  }
  if (IsLessThan(source.precision,
                 source.sparse_precision,
                 state_.precision,
                 state_.sparse_precision)) {
    DowngradeSparse(source);
    AddSparseValue(source, value);
    return;
  }
  if (IsLessThan(state_.precision,
                 state_.sparse_precision,
                 source.precision,
                 source.sparse_precision)) {
    buffer_.insert(DowngradeSparseValue(source.precision,
                                        source.sparse_precision,
                                        value,
                                        state_.precision,
                                        state_.sparse_precision));
  } else {
    buffer_.insert(value);
  }
  UpdateRepresentation();
}

void Sketch::AddSparseValues(
    SparseEncoding source, const std::optional<std::vector<int32_t>>& values) {
  if (!sparse_) {
    MaybeDowngrade(source.precision, source.sparse_precision);
    if (!values.has_value()) return;
    EnsureData();
    for (int32_t value : *values)
      AddSparseValue(source, value);
    return;
  }
  if (IsLessThan(source.precision,
                 source.sparse_precision,
                 state_.precision,
                 state_.sparse_precision)) {
    DowngradeSparse(source);
    AddSparseValues(source, values);
    return;
  }
  if (!values.has_value()) return;
  if (IsLessThan(state_.precision,
                 state_.sparse_precision,
                 source.precision,
                 source.sparse_precision)) {
    std::vector<int32_t> downgraded;
    downgraded.reserve(values->size());
    for (int32_t value : *values) {
      downgraded.push_back(DowngradeSparseValue(source.precision,
                                                source.sparse_precision,
                                                value,
                                                state_.precision,
                                                state_.sparse_precision));
    }
    AddUnsortedSparseValues(encoding(), downgraded);
    return;
  }
  std::vector<int32_t> merged = SortedValues().value_or(std::vector<int32_t>());
  merged.insert(merged.end(), values->begin(), values->end());
  std::sort(merged.begin(), merged.end());
  SetSparseValues(Dedupe(state_.precision, state_.sparse_precision, merged));
  UpdateRepresentation();
}

void Sketch::AddUnsortedSparseValues(SparseEncoding source,
                                     const std::vector<int32_t>& values) {
  for (int32_t value : values)
    AddSparseValue(source, value);
}

void Sketch::DowngradeSparse(SparseEncoding target) {
  const SparseEncoding original = encoding();
  if (!IsLessThan(target.precision,
                  target.sparse_precision,
                  original.precision,
                  original.sparse_precision)) {
    return;
  }
  std::optional<std::string> original_data = std::move(state_.sparse_data);
  std::vector<int32_t> original_buffer = BufferValues();
  state_.sparse_data.reset();
  state_.precision = std::min(original.precision, target.precision);
  state_.sparse_precision =
      std::min(original.sparse_precision, target.sparse_precision);
  buffer_.clear();

  std::vector<int32_t> values;
  if (original_data.has_value()) {
    Reader reader(*original_data);
    int32_t last = 0;
    while (!reader.AtEnd()) {
      last += static_cast<int32_t>(*reader.Varint());
      values.push_back(last);
    }
  }
  values.insert(values.end(), original_buffer.begin(), original_buffer.end());

  std::vector<int32_t> downgraded;
  downgraded.reserve(values.size());
  for (int32_t value : values) {
    downgraded.push_back(DowngradeSparseValue(original.precision,
                                              original.sparse_precision,
                                              value,
                                              target.precision,
                                              target.sparse_precision));
  }
  AddUnsortedSparseValues(target, downgraded);
}

void Sketch::Compact() {
  FlushBuffer();
  if (!state_.sparse_data.has_value()) state_.sparse_data = std::string();
  UpdateRepresentation();
}

void Sketch::UpdateRepresentation() {
  if (!sparse_) return;
  const int m = 1 << state_.precision;
  const auto max_buffer_elements = static_cast<size_t>(m * (1 - 0.75f));
  const auto max_sparse_data_bytes = static_cast<size_t>(m * 0.75f);
  if (buffer_.size() > max_buffer_elements) FlushBuffer();
  if (state_.sparse_data.has_value() &&
      state_.sparse_data->size() >= max_sparse_data_bytes) {
    Normalize();
  }
}

void Sketch::Normalize() {
  Sketch sparse = *this;
  sparse_ = false;
  buffer_.clear();
  sparse.MergeInto(*this);
  state_.sparse_data.reset();
  state_.sparse_size = 0;
}

void Sketch::FlushBuffer() {
  if (buffer_.empty()) return;
  std::vector<int32_t> values = DataValues();
  std::vector<int32_t> buffered = BufferValues();
  values.insert(values.end(), buffered.begin(), buffered.end());
  std::sort(values.begin(), values.end());
  SetSparseValues(Dedupe(state_.precision, state_.sparse_precision, values));
}

void Sketch::SetSparseValues(const std::vector<int32_t>& values) {
  std::string data;
  int32_t last = 0;
  for (int32_t value : values) {
    PutVarint(static_cast<uint32_t>(value - last), &data);
    last = value;
  }
  buffer_.clear();
  state_.sparse_data = std::move(data);
  state_.sparse_size = static_cast<int>(values.size());
}

std::vector<int32_t> Sketch::DataValues() const {
  std::vector<int32_t> values;
  if (!state_.HasSparseData()) return values;
  Reader reader(*state_.sparse_data);
  int32_t last = 0;
  while (!reader.AtEnd()) {
    last += static_cast<int32_t>(*reader.Varint());
    values.push_back(last);
  }
  return values;
}

std::vector<int32_t> Sketch::BufferValues() const {
  std::vector<int32_t> values(buffer_.begin(), buffer_.end());
  std::sort(values.begin(), values.end());
  return values;
}

std::optional<std::vector<int32_t>> Sketch::SortedValues() const {
  if (!state_.HasSparseData() && buffer_.empty()) return std::nullopt;
  std::vector<int32_t> values = DataValues();
  std::vector<int32_t> buffered = BufferValues();
  values.insert(values.end(), buffered.begin(), buffered.end());
  std::sort(values.begin(), values.end());
  return values;
}

void Sketch::MergeInto(Sketch& normal) const {
  if (state_.HasSparseData()) normal.AddSparseValues(encoding(), DataValues());
  normal.AddSparseValues(encoding(), BufferValues());
}

void Sketch::EnsureData() {
  if (!state_.HasData()) {
    state_.data = std::string(size_t{1} << state_.precision, '\0');
  }
}

void Sketch::MaybeDowngrade(int precision, int sparse_precision) {
  if (state_.precision <= precision &&
      state_.sparse_precision <= sparse_precision) {
    return;
  }
  if (state_.precision > precision) {
    std::optional<std::string> source = std::move(state_.data);
    const int source_precision = state_.precision;
    state_.data = std::string(size_t{1} << precision, '\0');
    state_.precision = precision;
    MergeNormalData(source, source_precision);
  }
  state_.sparse_precision = std::min(state_.sparse_precision, sparse_precision);
}

void Sketch::MergeNormalData(const std::optional<std::string>& source,
                             int source_precision) {
  if (!source.has_value()) return;
  EnsureData();
  std::string& target = *state_.data;
  if (state_.precision == source_precision) {
    for (size_t i = 0; i < source->size(); ++i) {
      target[i] = static_cast<char>(std::max(
          static_cast<uint8_t>(target[i]), static_cast<uint8_t>((*source)[i])));
    }
    return;
  }
  for (size_t old_index = 0; old_index < source->size(); ++old_index) {
    const auto old_rho_w = static_cast<uint8_t>((*source)[old_index]);
    const auto index = static_cast<int32_t>(old_index);
    const int32_t new_index = index >> (source_precision - state_.precision);
    const uint8_t new_rho_w =
        old_rho_w == 0
            ? 0
            : DowngradeRhoW(
                  index, old_rho_w, source_precision, state_.precision);
    auto& current = reinterpret_cast<uint8_t&>(target[new_index]);
    current = std::max(current, new_rho_w);
  }
}

}  // namespace hll_plus_plus
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
