#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_H_

// GoogleSqlCatalog is the GoogleSQL-facing catalog that the analyzer
// (Phase 4 / 5.A) consults during name resolution. It is a thin
// adapter: every `FindTable` lookup is forwarded to the active
// `backend::storage::Storage` instance through
// `Storage::GetSchema`, and the engine-agnostic
// `schema::TableSchema` is converted on the fly into a
// `googlesql::SimpleTable` whose columns carry the matching
// `googlesql::Type*` allocations from the supplied `TypeFactory`.
//
// The adapter intentionally does no preloading: BigQuery datasets
// can hold millions of tables and the analyzer touches only the ones
// referenced by the query, so the adapter materializes each
// `SimpleTable` the first time it's named and then caches it for
// the lifetime of the catalog instance. The cache is protected by an
// `absl::Mutex` so the catalog is safe to share across analyzer
// threads, but each instance is normally per-query: the engine
// constructs one when a `Query.ExecuteQuery` RPC arrives, hands it
// to GoogleSQL, and discards it when the query completes.
//
// Name resolution rules (mirroring BigQuery's REST identifier shape):
//
//   * Two-element paths are interpreted as `<dataset>.<table>` inside
//     `project_id_`, which is the project the engine was constructed
//     for (typically the `project_id` field on `QueryRequest`).
//   * Three-element paths are interpreted as
//     `<project>.<dataset>.<table>` and the first segment overrides
//     `project_id_`. This is the fully-qualified BigQuery shape.
//   * Anything else (zero, one, or four-plus elements) returns
//     `NOT_FOUND`. GoogleSQL's analyzer surfaces that as an
//     "unrecognized name" error with the offending path, which the
//     gateway maps to a `notFound` BigQuery REST error.
//
// The adapter does NOT yet wire up `CreateEvaluatorTableIterator` on
// the synthesized `SimpleTable`s: row iteration belongs to the
// `Storage`-backed `googlesql::Table` subclass that the reference
// impl engine will use (Phase 5.A). Until then, the `SimpleTable`s
// returned here are analysis-only; calling them as scan sources will
// hit GoogleSQL's "Table does not support the API in evaluator.h"
// default error.

#include <memory>
#include <string>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

class GoogleSqlCatalog : public ::googlesql::Catalog {
 public:
  // `storage` and `type_factory` must outlive the catalog. The
  // catalog does not take ownership; the typical lifetime is
  // `engine constructs storage + type_factory at startup` and the
  // catalog is constructed per query and discarded after the query
  // completes.
  //
  // `project_id` is the implicit project for two-element table
  // paths. It must be non-empty; the BigQuery REST surface always
  // supplies a project on `jobs.query` / `jobs.insert` requests.
  GoogleSqlCatalog(absl::string_view project_id,
                   storage::Storage* storage,
                   ::googlesql::TypeFactory* type_factory);

  ~GoogleSqlCatalog() override = default;

  GoogleSqlCatalog(const GoogleSqlCatalog&) = delete;
  GoogleSqlCatalog& operator=(const GoogleSqlCatalog&) = delete;

  std::string FullName() const override { return project_id_; }

  // Path resolution rules are documented in the file header. A miss
  // returns `absl::StatusCode::kNotFound`; any other status indicates
  // a storage-level failure mid-lookup.
  absl::Status FindTable(
      const absl::Span<const std::string>& path,
      const ::googlesql::Table** table,
      const FindOptions& options = FindOptions()) override
      ABSL_LOCKS_EXCLUDED(mu_);

  // Convert a `schema::ColumnSchema` into a freshly-allocated
  // `googlesql::Type*`. Public so the (eventually) Storage-backed
  // `Table` subclass in `backend/engine/reference_impl/` can reuse
  // the same translation when it builds its evaluator columns.
  //
  // The mapping is total for the BigQuery scalar / structural types
  // defined on `schema::ColumnType`. Unknown / unsupported types
  // (`kUnknown`, `kGeography`) yield `INVALID_ARGUMENT` so the
  // analyzer fails fast instead of silently substituting a string.
  // ARRAY-mode columns wrap the inner scalar/struct type in an
  // `ARRAY<T>`; STRUCT columns recurse on `fields`.
  static absl::StatusOr<const ::googlesql::Type*> ToGoogleSqlType(
      const schema::ColumnSchema& column,
      ::googlesql::TypeFactory* type_factory);

 private:
  // Looks up `dataset_id.table_id` in `project` via `storage_->GetSchema`
  // and materializes a `SimpleTable` for it, populating `table_cache_`
  // so repeated lookups during the same analysis pass return the
  // same pointer. The returned pointer is owned by the cache and
  // stays valid for the lifetime of the catalog.
  absl::StatusOr<const ::googlesql::Table*> MaterializeTable(
      absl::string_view project_id,
      absl::string_view dataset_id,
      absl::string_view table_id) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  // Cache key shape mirrors `storage::TableId` so a query that
  // qualifies the same table two different ways (with vs. without
  // the project prefix) still collapses onto one cache entry.
  static std::string CacheKey(absl::string_view project_id,
                              absl::string_view dataset_id,
                              absl::string_view table_id);

  const std::string project_id_;
  storage::Storage* const storage_;
  ::googlesql::TypeFactory* const type_factory_;

  mutable absl::Mutex mu_;
  // Owns every `SimpleTable` we hand out to the analyzer. Keys are
  // `CacheKey(project, dataset, table)`; values are non-null.
  std::vector<std::unique_ptr<::googlesql::SimpleTable>> tables_
      ABSL_GUARDED_BY(mu_);
  // Parallel-by-index lookup from cache key to `tables_` entry.
  // `flat_hash_map` would be cleaner but the catalog stays small per
  // query (BigQuery queries rarely reference more than a handful of
  // tables) and a linear scan keeps the dependency surface narrow.
  std::vector<std::string> keys_ ABSL_GUARDED_BY(mu_);
};

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_H_
