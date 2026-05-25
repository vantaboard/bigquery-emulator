#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_

// Storage is the C++ engine's row store interface.
//
// `Storage` lives below the gRPC service boundary: the gateway never
// touches it directly, the catalog handler does. Two concrete impls
// land later in Phase 3:
//
//   * `backend/storage/memory/in_memory_storage.{h,cc}` — volatile,
//     CI-friendly, dataset → table → rows map.
//   * `backend/storage/duckdb/duckdb_storage.{h,cc}` — persistent,
//     Parquet/Arrow on disk through DuckDB.
//
// **Engine-agnostic types only**: this header MUST NOT mention
// `googlesql::Value` or any other engine-specific type. The
// reference-impl and DuckDB engines each translate to their own
// representations at the call sites in `backend/engine/`. See ROADMAP
// "Pluggable engine and storage" for the rationale.

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {

// Stable identifiers for catalog entries. These mirror the proto
// `DatasetRef` / `TableRef` shape (proto/emulator.proto) but the
// storage layer does not depend on the proto runtime so the same types
// flow through the in-memory and DuckDB stores.
struct DatasetId {
  std::string project_id;
  std::string dataset_id;
};

struct TableId {
  std::string project_id;
  std::string dataset_id;
  std::string table_id;
};

inline bool operator==(const DatasetId& a, const DatasetId& b) {
  return a.project_id == b.project_id && a.dataset_id == b.dataset_id;
}
inline bool operator==(const TableId& a, const TableId& b) {
  return a.project_id == b.project_id && a.dataset_id == b.dataset_id &&
         a.table_id == b.table_id;
}

// Engine-agnostic cell value. The variant covers the BigQuery scalar
// types plus ARRAY and STRUCT containers.
//
// We deliberately do not reuse `googlesql::Value` here; the reference
// impl and DuckDB engines each marshal to and from `Value` at their
// own boundary. NULL is an explicit kind so callers do not have to
// thread `std::optional<Value>` through every API.
class Value {
 public:
  enum class Kind {
    kNull = 0,
    kBool,
    kInt64,
    kFloat64,
    // Both BigQuery STRING and BYTES land in `string_value()`. The
    // Cell-level type is carried out-of-band on the matching
    // `ColumnSchema`; we avoid duplicating that info per row.
    kString,
    kBytes,
    kArray,
    kStruct,
  };

  Value() = default;

  static Value Null() { return Value(); }
  static Value Bool(bool v);
  static Value Int64(int64_t v);
  static Value Float64(double v);
  static Value String(std::string v);
  static Value Bytes(std::string v);
  static Value Array(std::vector<Value> elements);
  static Value Struct(std::vector<Value> fields);

  Kind kind() const { return kind_; }
  bool is_null() const { return kind_ == Kind::kNull; }

  // Accessors. Each returns a default value (false / 0 / empty) when
  // the active kind does not match, so callers can use these to fold
  // values into wire encoders without an inspection cascade. Use
  // `kind()` to disambiguate when correctness matters.
  bool bool_value() const;
  int64_t int64_value() const;
  double float64_value() const;
  const std::string& string_value() const;
  const std::vector<Value>& array_value() const;
  const std::vector<Value>& struct_value() const;

 private:
  // The kString and kBytes kinds share the std::string slot in
  // `data_`; the active kind discriminates which BigQuery type the
  // bytes were parsed as.
  using Variant = std::variant<std::monostate, bool, int64_t, double,
                                std::string, std::vector<Value>>;

  Kind kind_ = Kind::kNull;
  Variant data_;
};

// One row in a table: cells are ordered by the column list of the
// table's `schema::TableSchema`. A NULL cell is represented as
// `Value::Null()`, never as a missing entry.
struct Row {
  std::vector<Value> cells;
};

// Forward iterator over a single scan of a table's rows. Storage
// implementations may stream rows lazily; a single `RowIterator`
// instance is owned by the caller and is **not** thread-safe.
class RowIterator {
 public:
  virtual ~RowIterator() = default;

  // Pulls the next row into `*row`. Returns:
  //   * `true`  - a row was written.
  //   * `false` - end of stream; `*row` is unchanged.
  // A non-OK status indicates a backend error mid-iteration; further
  // calls are undefined.
  virtual absl::StatusOr<bool> Next(Row* row) = 0;
};

// ReadFilter constrains the rows a `CreateReadStream` iterator yields.
//
// Plan 38 wires the StorageRead gRPC surface (`ReadRows`) on top of
// `CreateReadStream`, so the filter shape mirrors what the public
// `bigquery_emulator.v1.ReadOptions` proto can ask for. Each backend
// applies the knobs it can push down natively (DuckDB layers a SQL
// `LIMIT`; the memory backend caps iteration in C++); both speak the
// same wire shape after the iterator, so the handler does not branch
// on the storage type.
//
// Plan-38 scope is intentionally narrow: only `row_limit` is honored.
// `selected_fields` and `row_restriction` are documented in the proto
// for forward compatibility but are deferred to a follow-up plan when
// the engine wires per-column projection / pushdown — the storage
// layer accepts them today as `std::vector<std::string>` / `std::string`
// only so call sites compose cleanly with `ReadOptions`.
struct ReadFilter {
  // Maximum number of rows the iterator will yield before signaling
  // end-of-stream. <= 0 means "no limit" (return every row in the
  // table snapshot). Plan 38 enforces this knob on both backends so
  // ReadRows can honor caller-supplied caps without re-counting at
  // the handler layer.
  std::int64_t row_limit = 0;

  // Number of rows to skip from the head of the stream before the
  // first emitted row. Plan 38 uses this to honor
  // `ReadRowsRequest.offset` so a caller resuming a stream after a
  // transient failure does not re-receive rows it already processed.
  // <= 0 means "start at the first row".
  std::int64_t offset = 0;

  // Subset of column names the caller wants returned. Empty means
  // "all columns". Plan 38 accepts this list to keep the interface
  // forward-compatible with `ReadOptions.selected_fields`; the
  // backends do not yet project rows down to the subset (the engine
  // wiring lands in a follow-up plan).
  std::vector<std::string> selected_fields;

  // SQL-shaped predicate the caller wants pushed down. Plan 38
  // accepts but does not honor this knob; backends ignore it. Same
  // forward-compatibility story as `selected_fields`.
  std::string row_restriction;
};

// Storage is the abstract interface every backend implements.
//
// All methods are **thread-safe**. Implementations are free to provide
// finer-grained guarantees (the in-memory store uses a single mutex;
// the DuckDB-backed store relies on DuckDB's own concurrency model).
//
// Lifetime: a `Storage` instance is created once at engine startup and
// shared by every gRPC request handler.
class Storage {
 public:
  virtual ~Storage() = default;

  // ------------------------------------------------------------------
  // Dataset CRUD. `location` is the BigQuery region the dataset is
  // pinned to (e.g. "US", "EU"); the in-memory store stashes it for
  // round-tripping but otherwise ignores it. `delete_contents=true`
  // mirrors the BigQuery REST `deleteContents` query parameter on
  // `datasets.delete`.
  // ------------------------------------------------------------------
  virtual absl::Status CreateDataset(const DatasetId& id,
                                      absl::string_view location) = 0;
  virtual absl::Status DropDataset(const DatasetId& id,
                                    bool delete_contents) = 0;

  // ------------------------------------------------------------------
  // Table CRUD. `CreateTable` is idempotent at the dataset level only:
  // creating a table that already exists is an error
  // (ALREADY_EXISTS). `DropTable` is the inverse and is NOT_FOUND on
  // missing tables; callers that want "drop if exists" semantics
  // should swallow that status.
  // ------------------------------------------------------------------
  virtual absl::Status CreateTable(const TableId& id,
                                    const schema::TableSchema& schema) = 0;
  virtual absl::Status DropTable(const TableId& id) = 0;

  // Returns the schema the table was created with. NOT_FOUND if the
  // dataset or table does not exist.
  virtual absl::StatusOr<schema::TableSchema> GetSchema(
      const TableId& id) const = 0;

  // Appends `rows` to `id` as a single batch. Implementations may
  // require all rows in the batch to share the table's schema shape
  // (cell count == column count); validation lives in the impl, not
  // here, so the in-memory store can stay cheap.
  virtual absl::Status AppendRows(const TableId& id,
                                   absl::Span<const Row> rows) = 0;

  // Atomically replaces every row in `id` with `rows`. Used by the
  // DML engine's scan-and-rewrite path for UPDATE / DELETE / MERGE:
  // the engine pulls the existing rows, computes the post-mutation
  // shape, and hands the result back through this method so the
  // store can swap the row vector / parquet file in one shot.
  //
  // Same shape-check contract as `AppendRows`: row cell count must
  // equal the table's top-level column count, otherwise
  // INVALID_ARGUMENT. NOT_FOUND if the dataset / table does not
  // exist. The new row vector replaces the existing one in full,
  // including the empty-vector case (`rows.empty()` truncates the
  // table).
  virtual absl::Status OverwriteRows(const TableId& id,
                                      absl::Span<const Row> rows) = 0;

  // Begins a fresh scan of `id`'s rows. The returned iterator captures
  // a snapshot at call time; rows appended afterward may or may not be
  // visible depending on the impl. NOT_FOUND if the table does not
  // exist.
  virtual absl::StatusOr<std::unique_ptr<RowIterator>> ScanRows(
      const TableId& id) const = 0;

  // CreateReadStream is the StorageRead.ReadRows-shaped scan: same
  // snapshot semantics as `ScanRows`, but the returned iterator is
  // constrained by `filter` (see `ReadFilter` above). The memory
  // backend caps iteration in C++; the DuckDB backend pushes the
  // limit / offset into the underlying SELECT so we don't materialize
  // rows we will never emit. Both backends speak the same wire shape
  // afterward — the iterator is a `RowIterator` either way — so the
  // `StorageReadService` handler does not branch on backend type.
  //
  // Plan 38 enforces `row_limit` and `offset` on both backends;
  // `selected_fields` and `row_restriction` are accepted but not
  // honored (see the field comments on `ReadFilter`). NOT_FOUND if
  // the table does not exist.
  virtual absl::StatusOr<std::unique_ptr<RowIterator>> CreateReadStream(
      const TableId& id, const ReadFilter& filter) const = 0;
};

}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_
