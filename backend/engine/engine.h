#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_ENGINE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_ENGINE_H_

// Engine is the C++ engine's query execution interface.
//
// The only implementation lives at `backend/engine/duckdb/`: it
// transpiles the GoogleSQL ResolvedAST into DuckDB SQL via a custom
// visitor and executes it through DuckDB's C++ client.
//
// This header defines the abstract surface only. The
// `googlesql::Catalog` parameter is forward-declared so this header
// stays free of any GoogleSQL include dependency. The `AnalyzedQuery`
// and `RowSource` opaque interfaces let us return a resolved AST
// handle and a streamed result without leaking engine-specific types
// up to the gRPC handlers.

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

// Forward-declared so this header does not pull in any GoogleSQL
// headers. The DuckDB engine downcasts the `googlesql::Catalog*` to
// its own catalog adapter when it actually runs analysis.
namespace googlesql {
class Catalog;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {

// One BigQuery query parameter (named or positional). `value_json`
// carries the JSON-encoded literal value the gateway received on the
// REST request; the engine round-trips it through GoogleSQL's literal
// parser at analysis time.
struct QueryParameter {
  // Empty for positional parameters (BigQuery's `@0`, `@1`, ...).
  std::string name;
  // GoogleSQL `TypeKind` name, e.g. "INT64", "STRING".
  std::string type_kind;
  std::string value_json;
  // Gateway-encoded REST `parameterType` descriptor for STRUCT/ARRAY
  // parameters (field names and nested type kinds); empty for scalars.
  std::string type_json;
};

// One query the engine is asked to plan or execute. The fields mirror
// `bigquery_emulator.v1.QueryRequest` from `proto/emulator.proto`.
struct QueryRequest {
  std::string project_id;
  // Default dataset for unqualified table references. May be empty.
  std::string default_dataset_id;
  std::string sql;
  std::vector<QueryParameter> parameters;
  // BigQuery defaults `useLegacySql` to true on the wire; the gateway
  // rejects that case (see the gateway-HTTP-surface section of
  // ROADMAP.md) so by the time a request reaches here this field
  // should always be false. We keep it as a
  // belt-and-braces field so the engine can also error out if the
  // gateway ever stops enforcing.
  bool use_legacy_sql = false;
};

// Opaque handle for a parsed + name-resolved query. The DuckDB engine
// hides its own ResolvedAST plus any side state (extracted
// parameters, default dataset, etc.) behind this interface.
class AnalyzedQuery {
 public:
  virtual ~AnalyzedQuery();

  // The schema of the rows the query will produce on
  // `Engine::ExecuteQuery`.
  virtual const schema::TableSchema& output_schema() const = 0;
};

// Streamed query result. The engine produces rows one at a time;
// `Next` returns false on end-of-stream. The DuckDB engine batches
// internally and streams rows out one-by-one.
class RowSource {
 public:
  virtual ~RowSource();

  virtual const schema::TableSchema& schema() const = 0;

  // Pulls the next row into `*row`. Returns:
  //   * `true`  - a row was written.
  //   * `false` - end of stream; `*row` is unchanged.
  // A non-OK status indicates an execution error; further calls are
  // undefined.
  virtual absl::StatusOr<bool> Next(storage::Row* row) = 0;
};

// Result of a `DryRun`. Mirrors the BigQuery
// `Job.statistics.query.{schema,totalBytesProcessed}` shape the
// gateway exposes on `jobs.query?dryRun=true`.
struct DryRunResult {
  schema::TableSchema schema;
  int64_t estimated_bytes_processed = 0;
};

// Result of `Engine::ExecuteDml`: per-statement modification counts
// for an INSERT / UPDATE / DELETE / MERGE statement. Mirrors the
// BigQuery REST `Job.statistics.query.dmlStats` envelope; the
// frontend handler folds these counts into a final
// `QueryResultRow.dml_stats` message on the `Query.ExecuteQuery`
// stream.
struct DmlStats {
  // Number of rows added by INSERT / MERGE-INSERT branches.
  int64_t inserted_row_count = 0;
  // Number of rows updated by UPDATE / MERGE-UPDATE branches.
  int64_t updated_row_count = 0;
  // Number of rows removed by DELETE / MERGE-DELETE branches.
  int64_t deleted_row_count = 0;
};

// Engine is the abstract interface every query backend implements.
//
// Lifetime: created once at startup with a `Storage*` and a
// `googlesql::Catalog*` already wired up; shared by every gRPC
// request handler. All methods are thread-safe.
class Engine {
 public:
  virtual ~Engine();

  // Parse + name-resolve + type-check `request.sql` against `catalog`.
  // Returns an opaque `AnalyzedQuery` the caller can hand back to
  // `DryRun` / `ExecuteQuery`, OR a parse / analysis error mapped to
  // the matching absl::Status code (the gateway translates that into
  // a BigQuery error envelope; see the analyzer integration section
  // of ROADMAP.md).
  //
  // `[[nodiscard]]` is on every Status / StatusOr-returning method
  // here for the same reason it is on `backend::storage::Storage`:
  // dropping the result silently swallows a parse / analysis error
  // that the gateway has no other channel to surface.
  [[nodiscard]] virtual absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& request, googlesql::Catalog* catalog) = 0;

  // Plan-only path used by `jobs.query?dryRun=true`. Implementations
  // are free to short-circuit through `Analyze` internally.
  [[nodiscard]] virtual absl::StatusOr<DryRunResult> DryRun(
      const QueryRequest& request, googlesql::Catalog* catalog) = 0;

  // Plan + execute. The returned `RowSource` streams the result rows
  // back to the gateway one by one; the gateway paginates them out
  // through the `bigquery.jobs.query` and
  // `bigquery.jobs.getQueryResults` REST endpoints.
  [[nodiscard]] virtual absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request, googlesql::Catalog* catalog) = 0;

  // Plan + execute a DML statement (INSERT / UPDATE / DELETE / MERGE)
  // and return the per-statement modification counts. The engine is
  // expected to apply the changes to the underlying `Storage` it was
  // constructed with -- callers do not see the modified rows, only
  // the count summary the gateway folds into BigQuery's
  // `dmlStats` / `numDmlAffectedRows` fields. Engines that do not
  // implement DML yet return `absl::StatusCode::kUnimplemented`; the
  // frontend handler maps that to gRPC `UNIMPLEMENTED` so the
  // gateway can surface BigQuery's `notImplemented` reason.
  [[nodiscard]] virtual absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request, googlesql::Catalog* catalog) {
    (void)request;
    (void)catalog;
    return absl::UnimplementedError(
        "Engine::ExecuteDml is not implemented in this engine");
  }

  // Plan + execute a DDL statement
  // (CREATE TABLE / CREATE TABLE AS SELECT / DROP TABLE / ALTER TABLE
  // ADD COLUMN). The engine mutates the underlying `Storage` -- there
  // is no row-shaped reply, just success (OK) or a status mapped to
  // the matching gRPC code. Engines that do not implement DDL return
  // `absl::StatusCode::kUnimplemented`; the frontend handler maps
  // that to gRPC `UNIMPLEMENTED`.
  [[nodiscard]] virtual absl::Status ExecuteDdl(const QueryRequest& request,
                                                googlesql::Catalog* catalog) {
    (void)request;
    (void)catalog;
    return absl::UnimplementedError(
        "Engine::ExecuteDdl is not implemented in this engine");
  }
};

}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_ENGINE_H_
