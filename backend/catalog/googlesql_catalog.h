#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_H_

// GoogleSqlCatalog is the GoogleSQL-facing catalog that the analyzer
// consults during name resolution. It is a thin
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
//   * One-element paths resolve to `<default_dataset>.<table>` inside
//     `project_id_` when `default_dataset_id_` is non-empty (BigQuery
//     `defaultDataset` on `jobs.query`).
//   * Two-element paths are interpreted as `<dataset>.<table>` inside
//     `project_id_`, except `INFORMATION_SCHEMA.<view>` which is the
//     project-scoped metadata view shape.
//   * Three-element paths are interpreted as
//     `<project>.<dataset>.<table>` when the middle segment is not
//     `INFORMATION_SCHEMA`, otherwise as
//     `<dataset>.INFORMATION_SCHEMA.<view>`.
//   * Table ids ending in `*` resolve as BigQuery wildcard tables
//     (UNION ALL of every matching physical table in the dataset).
//   * Anything else (zero or four-plus elements) returns `NOT_FOUND`.
//
// Materialized tables are `backend::catalog::StorageTable` instances
// (a `SimpleTable` subclass with a working
// `CreateEvaluatorTableIterator` that streams rows out of the
// underlying `Storage`). The catalog drives analyzer name
// resolution; the DuckDB engine then reads the resolved AST and
// executes through DuckDB directly.
//
// The catalog inherits from `googlesql::SimpleCatalog` so the
// analyzer can look up GoogleSQL built-in functions and types
// through the standard `SimpleCatalog::AddBuiltinFunctionsAndTypes`
// path -- the constructor wires that up once per query so the
// analyzer sees `COUNT`, `SUM`, `CONCAT`, and friends. We override
// `FindTable` to short-circuit the SimpleCatalog default and hit
// `Storage` directly for the BigQuery `<dataset>.<table>` /
// `<project>.<dataset>.<table>` path shapes.

#include <memory>
#include <string>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

class GoogleSqlCatalog : public ::googlesql::SimpleCatalog {
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
  // `language` controls which GoogleSQL feature set is registered on
  // the catalog (via `SimpleCatalog::AddBuiltinFunctionsAndTypes`).
  // Pass the same `LanguageOptions` the analyzer is configured with
  // so the analyzer and the catalog agree on what's resolvable.
  GoogleSqlCatalog(absl::string_view project_id,
                   storage::Storage* storage,
                   ::googlesql::TypeFactory* type_factory,
                   const ::googlesql::LanguageOptions& language,
                   absl::string_view default_dataset_id = "");

  ~GoogleSqlCatalog() override = default;

  GoogleSqlCatalog(const GoogleSqlCatalog&) = delete;
  GoogleSqlCatalog& operator=(const GoogleSqlCatalog&) = delete;

  // SimpleCatalog::FullName() returns the catalog name we passed to
  // its constructor; mirror the previous "project as catalog name"
  // contract by always returning the project_id.
  std::string FullName() const override {
    return project_id_;
  }

  // The `TypeFactory` the caller pinned for this catalog's lifetime.
  // Analyzer passes must allocate resolved types through this factory
  // so pointers in the AST stay valid until the query completes.
  ::googlesql::TypeFactory* type_factory() const {
    return type_factory_;
  }

  storage::Storage* storage() const {
    return storage_;
  }

  // Path resolution rules are documented in the file header. A miss
  // returns `absl::StatusCode::kNotFound`; any other status indicates
  // a storage-level failure mid-lookup.
  absl::Status FindTable(const absl::Span<const std::string>& path,
                         const ::googlesql::Table** table,
                         const FindOptions& options = FindOptions()) override
      ABSL_LOCKS_EXCLUDED(mu_);

  absl::Status FindTableValuedFunction(
      const absl::Span<const std::string>& path,
      const ::googlesql::TableValuedFunction** function,
      const FindOptions& options = FindOptions()) override;

  absl::Status FindProcedure(
      const absl::Span<const std::string>& path,
      const ::googlesql::Procedure** procedure,
      const FindOptions& options = FindOptions()) override;

  // Convert a `schema::ColumnSchema` into a freshly-allocated
  // `googlesql::Type*`. Public so other catalog adapters can reuse
  // the same translation when they build their typed columns.
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

  absl::StatusOr<const ::googlesql::Table*> MaterializeInfoSchemaView(
      absl::string_view project_id,
      absl::string_view dataset_id,
      absl::string_view view_name) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  absl::StatusOr<const ::googlesql::Table*> MaterializeWildcardTable(
      absl::string_view project_id,
      absl::string_view dataset_id,
      absl::string_view wildcard_table_id) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  // Cache key shape mirrors `storage::TableId` so a query that
  // qualifies the same table two different ways (with vs. without
  // the project prefix) still collapses onto one cache entry.
  static std::string CacheKey(absl::string_view project_id,
                              absl::string_view dataset_id,
                              absl::string_view table_id);

  const std::string project_id_;
  const std::string default_dataset_id_;
  storage::Storage* const storage_ = nullptr;
  ::googlesql::TypeFactory* const type_factory_ = nullptr;

  mutable absl::Mutex mu_;
  // Owns every catalog table we hand out to the analyzer / evaluator.
  // Keys are `CacheKey(project, dataset, table)`; values are non-null.
  std::vector<std::unique_ptr<::googlesql::SimpleTable>> tables_
      ABSL_GUARDED_BY(mu_){};
  // Parallel-by-index lookup from cache key to `tables_` entry.
  // `flat_hash_map` would be cleaner but the catalog stays small per
  // query (BigQuery queries rarely reference more than a handful of
  // tables) and a linear scan keeps the dependency surface narrow.
  std::vector<std::string> keys_ ABSL_GUARDED_BY(mu_){};
  // Non-owning cache of registry-backed views (owned by view_registry).
  std::vector<std::string> registered_view_keys_ ABSL_GUARDED_BY(mu_){};
  std::vector<const ::googlesql::Table*> registered_views_
      ABSL_GUARDED_BY(mu_){};
};

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_H_
