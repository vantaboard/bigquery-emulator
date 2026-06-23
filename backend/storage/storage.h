#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_

// Storage is the C++ engine's row store interface.
//
// `Storage` lives below the gRPC service boundary: the gateway never
// touches it directly, the catalog handler does. The only concrete
// implementation is `backend/storage/duckdb/duckdb_storage.{h,cc}`,
// the persistent Parquet/Arrow-on-disk store backed by DuckDB.
//
// **Engine-agnostic types only**: this header MUST NOT mention
// `googlesql::Value` or any other engine-specific type. The DuckDB
// engine translates to its own representations at the call sites in
// `backend/engine/`. See ROADMAP "Pluggable engine and storage" for
// the rationale.

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/table_governance.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {

// Stable identifiers for catalog entries. These mirror the proto
// `DatasetRef` / `TableRef` shape (proto/emulator.proto) but the
// storage layer does not depend on the proto runtime.
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

// Stable identifiers for persisted routines (UDF / UDAF / TVF /
// procedure). Mirrors the BigQuery `RoutineReference` REST shape.
struct RoutineId {
  std::string project_id;
  std::string dataset_id;
  std::string routine_id;
};

inline bool operator==(const RoutineId& a, const RoutineId& b) {
  return a.project_id == b.project_id && a.dataset_id == b.dataset_id &&
         a.routine_id == b.routine_id;
}

// Kind of routine stored in `catalog.duckdb`. Values are persisted as
// the lowercase snake strings below (see duckdb_storage_routines.cc).
enum class RoutineKind {
  kScalarFunction = 0,
  kAggregateFunction,
  kTableValuedFunction,
  kProcedure,
};

// Durable routine metadata. `ddl_sql` carries the original CREATE
// statement (the source of truth for re-analysis at rehydrate time);
// `signature_json` stores argument metadata (including ANY TYPE
// markers) for REST round-trip without re-parsing the DDL body.
struct RoutineRecord {
  RoutineId id;
  RoutineKind kind = RoutineKind::kScalarFunction;
  std::string language;
  std::string ddl_sql;
  bool is_temp = false;
  std::string signature_json;
};

// Durable logical-view metadata. `ddl_sql` is the original CREATE VIEW
// statement (re-analyzed at rehydrate time into the view registry).
struct ViewId {
  std::string project_id;
  std::string dataset_id;
  std::string view_id;
};

struct ViewRecord {
  ViewId id;
  std::string ddl_sql;
  std::string view_query;
  schema::TableSchema schema;
};

struct TableResourceInfo {
  std::string table_type;
  std::string view_query;
  std::string ddl_sql;
};

// Engine-agnostic cell value. The variant covers the BigQuery scalar
// types plus ARRAY and STRUCT containers.
//
// We deliberately do not reuse `googlesql::Value` here; the DuckDB
// engine marshals to and from `Value` at its own boundary. NULL is
// an explicit kind so callers do not have to thread
// `std::optional<Value>` through every API.
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

  static Value Null() {
    return Value();
  }
  static Value Bool(bool v);
  static Value Int64(int64_t v);
  static Value Float64(double v);
  static Value String(std::string v);
  static Value Bytes(std::string v);
  static Value Array(std::vector<Value> elements);
  static Value Struct(std::vector<Value> fields);

  Kind kind() const {
    return kind_;
  }
  bool is_null() const {
    return kind_ == Kind::kNull;
  }

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
  using Variant = std::variant<std::monostate,
                               bool,
                               int64_t,
                               double,
                               std::string,
                               std::vector<Value>>;

  Kind kind_ = Kind::kNull;
  Variant data_{};
};

// One row in a table: cells are ordered by the column list of the
// table's `schema::TableSchema`. A NULL cell is represented as
// `Value::Null()`, never as a missing entry.
struct Row {
  std::vector<Value> cells{};
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
  //
  // `[[nodiscard]]`: the StatusOr carries the iteration error; a
  // caller that drops the result is silently swallowing a backend
  // failure that would otherwise surface to the gateway as a 5xx.
  [[nodiscard]] virtual absl::StatusOr<bool> Next(Row* row) = 0;
};

// ReadFilter constrains the rows a `CreateReadStream` iterator yields.
//
// The StorageRead gRPC surface (`ReadRows`) is wired on top of
// `CreateReadStream`, so the filter shape mirrors what the public
// `bigquery_emulator.v1.ReadOptions` proto can ask for. The DuckDB
// backend pushes the knobs down natively (a SQL `LIMIT` / `WHERE`)
// and the iterator surfaces them on the wire so the handler does
// not have to re-apply them.
//
// `equality_predicate` carries the typed parse of
// `<column> = <literal>` from `ReadOptions.row_restriction`. The raw
// `row_restriction` string is left in for debugging / introspection
// but the backends consume the parsed `equality_predicate` slot so
// the parse happens exactly once per session. `selected_fields`
// carries the per-column projection; see the field comments below.
struct ReadFilter {
  // Maximum number of rows the iterator will yield before signaling
  // end-of-stream. <= 0 means "no limit" (return every row in the
  // table snapshot). Both backends enforce this knob so
  // ReadRows can honor caller-supplied caps without re-counting at
  // the handler layer.
  std::int64_t row_limit = 0;

  // Number of rows to skip from the head of the stream before the
  // first emitted row. ReadRows uses this to honor
  // `ReadRowsRequest.offset` so a caller resuming a stream after a
  // transient failure does not re-receive rows it already processed.
  // <= 0 means "start at the first row".
  std::int64_t offset = 0;

  // Subset of column names the caller wants returned. Empty means
  // "all columns". The DuckDB backend wires this
  // natively: the backend builds a projected schema from the
  // listed names and the `RowIterator` yields rows in that order.
  // The handler validates the names exist at session-create time, so
  // an unknown column surfaces as INVALID_ARGUMENT before the stream
  // opens; the backend does the same defensive check (re-validating
  // here would be redundant, but the helper is shared with the
  // CreateReadSession parse path so an unknown column never reaches
  // a real SELECT).
  std::vector<std::string> selected_fields;

  // Raw SQL-shaped predicate the caller supplied. Retained on the
  // filter for debugging / log lines.
  std::string row_restriction;

  // DuckDB `WHERE` body produced by `TranspileRowRestriction` (analyzer
  // + transpiler). Empty when no restriction was supplied.
  std::optional<std::string> where_sql;

  // Legacy typed parse of single-equality restrictions. Prefer
  // `where_sql`; kept so existing unit tests can exercise the narrow
  // parser without pulling in the analyzer.
  std::optional<EqualityPredicate> equality_predicate;

  // Inclusive lower bound on DuckDB `file_row_number` for this stream
  // partition. Defaults to 0 (table head).
  std::int64_t row_start = 0;

  // Exclusive upper bound on `file_row_number`. Negative means
  // unbounded (read through table tail).
  std::int64_t row_end = -1;
};

// Storage is the abstract interface every backend implements.
//
// All methods are **thread-safe**. The DuckDB-backed store relies on
// DuckDB's own concurrency model.
//
// Lifetime: a `Storage` instance is created once at engine startup and
// shared by every gRPC request handler.
class Storage {
 public:
  virtual ~Storage() = default;

  // ------------------------------------------------------------------
  // Dataset CRUD. `location` is the BigQuery region the dataset is
  // pinned to (e.g. "US", "EU"); the DuckDB store stashes it for
  // round-tripping but otherwise ignores it. `delete_contents=true`
  // mirrors the BigQuery REST `deleteContents` query parameter on
  // `datasets.delete`.
  //
  // The `absl::Status` returns are `[[nodiscard]]` so a caller that
  // drops the result is caught by the compiler — the gateway has no
  // other way to surface dataset CRUD failures, and silent drops
  // were the motivating example for the rollout's status-discarded
  // rule. The same rationale applies to every other Status-returning
  // method on this interface.
  // ------------------------------------------------------------------
  [[nodiscard]] virtual absl::Status CreateDataset(
      const DatasetId& id, absl::string_view location) = 0;
  [[nodiscard]] virtual absl::Status DropDataset(const DatasetId& id,
                                                 bool delete_contents) = 0;

  // Lists the datasets registered under `project_id`. Returns an empty
  // vector when the project has no datasets (i.e. never registered any
  // or all have been dropped). Implementations MUST return ids in
  // deterministic order so list pagination / callers that diff against
  // a prior listing are stable; the DuckDB backend orders
  // lexicographically by `dataset_id`. INVALID_ARGUMENT when
  // `project_id` is empty.
  //
  // Used by the gateway's `datasets.list` REST handler to enumerate
  // the catalog after `RegisterDataset` calls; the
  // `frontend/handlers/catalog.cc` gRPC wrapper exposes this through
  // the `Catalog.ListDatasets` RPC.
  [[nodiscard]] virtual absl::StatusOr<std::vector<DatasetId>> ListDatasets(
      absl::string_view project_id) const = 0;

  // ------------------------------------------------------------------
  // Table CRUD. `CreateTable` is idempotent at the dataset level only:
  // creating a table that already exists is an error
  // (ALREADY_EXISTS). `DropTable` is the inverse and is NOT_FOUND on
  // missing tables; callers that want "drop if exists" semantics
  // should swallow that status.
  // ------------------------------------------------------------------
  [[nodiscard]] virtual absl::Status CreateTable(
      const TableId& id, const schema::TableSchema& schema) = 0;
  [[nodiscard]] virtual absl::Status DropTable(const TableId& id) = 0;

  // Restores a table previously soft-deleted by `DropTable`. When
  // `deleted_ms` is zero, restores the newest tombstone for the table.
  [[nodiscard]] virtual absl::Status RestoreTable(const TableId& id,
                                                  std::int64_t deleted_ms = 0) {
    (void)id;
    (void)deleted_ms;
    return absl::UnimplementedError(
        "Storage::RestoreTable is not implemented for this backend");
  }

  // Lists the tables registered under `dataset_id`. Returns an empty
  // vector when the dataset is empty. NOT_FOUND when the dataset does
  // not exist; INVALID_ARGUMENT when `project_id` / `dataset_id` is
  // empty. Like ListDatasets, implementations MUST return ids in
  // deterministic (lexicographic) order. Used by the gateway's
  // `tables.list` REST handler.
  [[nodiscard]] virtual absl::StatusOr<std::vector<TableId>> ListTables(
      const DatasetId& dataset_id) const = 0;

  // Returns the schema the table was created with. NOT_FOUND if the
  // dataset or table does not exist.
  [[nodiscard]] virtual absl::StatusOr<schema::TableSchema> GetSchema(
      const TableId& id) const = 0;

  // Appends `rows` to `id` as a single batch. Implementations may
  // require all rows in the batch to share the table's schema shape
  // (cell count == column count); validation lives in the impl, not
  // here.
  [[nodiscard]] virtual absl::Status AppendRows(const TableId& id,
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
  [[nodiscard]] virtual absl::Status OverwriteRows(
      const TableId& id, absl::Span<const Row> rows) = 0;

  // Begins a fresh scan of `id`'s rows. The returned iterator captures
  // a snapshot at call time; rows appended afterward may or may not be
  // visible depending on the impl. NOT_FOUND if the table does not
  // exist.
  [[nodiscard]] virtual absl::StatusOr<std::unique_ptr<RowIterator>> ScanRows(
      const TableId& id) const = 0;

  // CreateReadStream is the StorageRead.ReadRows-shaped scan: same
  // snapshot semantics as `ScanRows`, but the returned iterator is
  // constrained by `filter` (see `ReadFilter` above). The DuckDB
  // backend pushes the limit / offset into the underlying SELECT so
  // we don't materialize rows we will never emit. The iterator is a
  // `RowIterator` so the `StorageReadService` handler does not
  // branch on backend type.
  //
  // `row_limit`, `offset`, `row_restriction` (via its typed
  // `equality_predicate` parse), and `selected_fields` are all
  // honored. See the field comments on `ReadFilter`
  // for the per-knob contract. NOT_FOUND if the table does not
  // exist.
  [[nodiscard]] virtual absl::StatusOr<std::unique_ptr<RowIterator>>
  CreateReadStream(const TableId& id, const ReadFilter& filter) const = 0;

  // Returns the row count of `id`'s on-disk snapshot at call time
  // (DuckDB: `COUNT(*)` over `read_parquet(..., file_row_number=true)`).
  // Used by Storage Read session minting to partition streams.
  [[nodiscard]] virtual absl::StatusOr<std::int64_t> CountRows(
      const TableId& id) const = 0;

  // When the backend persists table data as a Parquet snapshot, returns
  // the absolute path to that file so the DuckDB executor can attach it
  // via `read_parquet` without a row round-trip. Default: no snapshot.
  [[nodiscard]] virtual std::optional<std::string> ParquetSnapshotPath(
      const TableId& id) const {
    (void)id;
    return std::nullopt;
  }

  // When the backend supports historical Parquet snapshots, returns the
  // absolute path to the snapshot that was current at `as_of_ms` (Unix epoch
  // milliseconds). Default: same as `ParquetSnapshotPath` (ignores as-of).
  [[nodiscard]] virtual absl::StatusOr<std::optional<std::string>>
  ParquetSnapshotPathAt(const TableId& id, std::int64_t as_of_ms) const {
    (void)as_of_ms;
    return ParquetSnapshotPath(id);
  }

  // ------------------------------------------------------------------
  // Routine CRUD. Persists UDF / UDAF / TVF / procedure DDL so the
  // per-project registries can rehydrate across engine restarts.
  // `UpsertRoutine` replaces an existing row keyed by
  // (project_id, dataset_id, routine_id). Temp routines (`is_temp`)
  // are accepted for API symmetry but rehydration skips them.
  // ------------------------------------------------------------------
  [[nodiscard]] virtual absl::Status UpsertRoutine(
      const RoutineRecord& record) = 0;
  [[nodiscard]] virtual absl::Status DeleteRoutine(const RoutineId& id) = 0;
  [[nodiscard]] virtual absl::StatusOr<RoutineRecord> GetRoutine(
      const RoutineId& id) const = 0;
  [[nodiscard]] virtual absl::StatusOr<std::vector<RoutineRecord>> ListRoutines(
      const DatasetId& dataset_id) const = 0;
  [[nodiscard]] virtual absl::StatusOr<std::vector<RoutineRecord>>
  ListAllRoutines() const = 0;

  // ------------------------------------------------------------------
  // Logical view DDL persistence (CREATE VIEW only; materialized views
  // are storage-backed and do not use this table).
  // ------------------------------------------------------------------
  [[nodiscard]] virtual absl::Status UpsertView(const ViewRecord& record) = 0;
  [[nodiscard]] virtual absl::Status DeleteView(const ViewId& id) = 0;
  [[nodiscard]] virtual absl::StatusOr<std::vector<ViewRecord>> ListAllViews()
      const = 0;

  [[nodiscard]] virtual absl::StatusOr<TableResourceInfo> GetTableResourceInfo(
      const TableId& id) const {
    (void)id;
    return absl::UnimplementedError(
        "Storage backend does not expose table resource metadata");
  }

  // Row-access policies and column-level security metadata.
  [[nodiscard]] virtual absl::StatusOr<TableGovernance> GetTableGovernance(
      const TableId& id) const {
    (void)id;
    return TableGovernance{};
  }
  [[nodiscard]] virtual absl::Status UpsertRowAccessPolicy(
      const TableId& id, const RowAccessPolicyRecord& policy) {
    (void)id;
    (void)policy;
    return absl::UnimplementedError(
        "Storage backend does not persist row access policies");
  }
  [[nodiscard]] virtual absl::Status DeleteRowAccessPolicy(
      const TableId& id, absl::string_view policy_id) {
    (void)id;
    (void)policy_id;
    return absl::UnimplementedError(
        "Storage backend does not persist row access policies");
  }
  [[nodiscard]] virtual absl::Status SetColumnGovernance(
      const TableId& id,
      absl::string_view column_name,
      const ColumnGovernanceRecord& column) {
    (void)id;
    (void)column_name;
    (void)column;
    return absl::UnimplementedError(
        "Storage backend does not persist column governance");
  }

  // Persistent-store root (DuckDB catalog + external-source snapshots).
  [[nodiscard]] virtual absl::string_view data_dir() const {
    return "";
  }
};

}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_
