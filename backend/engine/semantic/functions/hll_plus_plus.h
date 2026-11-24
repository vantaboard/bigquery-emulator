#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_PLUS_PLUS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_PLUS_PLUS_H_

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace hll_plus_plus {

// `AggregatorStateProto.value_type` of a sketch over INT64 values.
inline constexpr int kValueTypeUint64 = 8;

// `AggregatorStateProto.value_type` of a sketch over STRING or BYTES values.
inline constexpr int kValueTypeBytesOrUtf8String = 11;

// The highest sparse precision a sketch supports.
inline constexpr int kMaximumSparsePrecision = 25;

// A HyperLogLog++ sketch in BigQuery's `HLL_COUNT` format: an
// `AggregatorStateProto` with a `HyperLogLogPlusUniqueStateProto` extension.
//
// It is a port of zetasketch's `HyperLogLogPlusPlus` (the reference that
// matches BigQuery byte for byte), including when the sparse representation
// flushes its buffer and switches to the normal one, so a sketch built from
// the same hashes in the same order serializes to the same bytes.
class Sketch {
 public:
  // An empty sketch at the given normal and sparse precisions, for values of
  // `value_type`.
  static absl::StatusOr<Sketch> Create(int precision,
                                       int sparse_precision,
                                       int value_type);

  // Parses a serialized sketch.
  static absl::StatusOr<Sketch> Parse(absl::string_view proto);

  // Adds a value's 64-bit Fingerprint2011 hash.
  void AddHash(uint64_t hash);

  // Merges `other` into this sketch, downgrading to the lower precisions of
  // the two. Fails when the value types are incompatible or the precisions
  // cannot be ordered.
  absl::Status Merge(const Sketch& other);

  // The cardinality estimate, as `HLL_COUNT.EXTRACT` returns it.
  int64_t Estimate();

  // The serialized `AggregatorStateProto`.
  std::string Serialize();

 private:
  struct State {
    int type = 0;
    int64_t num_values = 0;
    int encoding_version = 1;
    int value_type = 0;
    int sparse_size = 0;
    int precision = 0;
    int sparse_precision = 0;
    std::optional<std::string> data;
    std::optional<std::string> sparse_data;

    bool HasData() const {
      return data.has_value() && !data->empty();
    }
    bool HasSparseData() const {
      return sparse_data.has_value() && !sparse_data->empty();
    }
  };

  struct SparseEncoding {
    int precision;
    int sparse_precision;
  };

  explicit Sketch(State state);

  absl::Status Validate() const;
  SparseEncoding encoding() const;

  void AddSparseValue(SparseEncoding source, int32_t value);
  void AddSparseValues(SparseEncoding source,
                       const std::optional<std::vector<int32_t>>& values);

  // Sparse representation.
  void AddUnsortedSparseValues(SparseEncoding source,
                               const std::vector<int32_t>& values);
  void DowngradeSparse(SparseEncoding target);
  void Compact();
  void UpdateRepresentation();
  void Normalize();
  void FlushBuffer();
  void SetSparseValues(const std::vector<int32_t>& values);
  std::vector<int32_t> DataValues() const;
  std::vector<int32_t> BufferValues() const;
  std::optional<std::vector<int32_t>> SortedValues() const;
  void MergeInto(Sketch& normal) const;

  // Normal representation.
  void EnsureData();
  void MaybeDowngrade(int precision, int sparse_precision);
  void MergeNormalData(const std::optional<std::string>& source,
                       int source_precision);

  State state_;
  bool sparse_ = false;
  int allowed_types_ = 0;
  std::unordered_set<int32_t> buffer_;
};

}  // namespace hll_plus_plus
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_PLUS_PLUS_H_
