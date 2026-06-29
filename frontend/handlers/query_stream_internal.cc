#include <string>

#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/engine/phase_recorder.h"
#include "backend/schema/schema.h"
#include "frontend/handlers/query_internal.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {
::grpc::Status ValidateQueryRequest(
    const backend::storage::Storage* storage,
    const v1::QueryRequest& request,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  if (!write) {
    return ::grpc::Status(
        ::grpc::StatusCode::INTERNAL,
        "QueryService::ExecuteQuery: write callback is empty");
  }
  if (storage == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "QueryService::ExecuteQuery: storage backend is not configured");
  }
  if (request.use_legacy_sql()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::ExecuteQuery: useLegacySql=true is not supported; "
        "the emulator only implements GoogleSQL");
  }
  if (request.project_id().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::ExecuteQuery: request.project_id is required");
  }
  if (request.sql().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::ExecuteQuery: request.sql is required");
  }
  return ::grpc::Status::OK;
}

// Emit the trailing `statement_type` message every successful
// `ExecuteQuery` reply now carries (per
// `docs/ENGINE_POLICY.md` Item 5). The gateway
// folds the value into BigQuery's
// `Job.statistics.query.statementType` envelope. We never emit the
// trailer when `statement_type` is empty (e.g. shapes BigQuery REST
// does not enumerate); the gateway then omits the envelope as well.
::grpc::Status EmitStatementType(
    absl::string_view statement_type,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  if (statement_type.empty()) return ::grpc::Status::OK;
  v1::QueryResultRow trailer;
  trailer.set_statement_type(std::string(statement_type));
  if (!write(trailer)) {
    return ::grpc::Status(
        ::grpc::StatusCode::CANCELLED,
        "QueryService::ExecuteQuery: client cancelled stream before "
        "statement_type trailer");
  }
  return ::grpc::Status::OK;
}

// Emit the trailing `emulator_route` message that pairs with the
// `statement_type` trailer (per
// `docs/ENGINE_POLICY.md`). The value is
// the canonical lowercase-snake spelling of the `Disposition` the
// coordinator's `RouteClassifier` chose (see
// `backend/engine/disposition.cc::DispositionToString`). Empty
// `emulator_route` strings are skipped just like `statement_type`
// so unrecognized routes don't pollute the gateway's
// `Job.statistics.query.emulatorRoute` envelope. The trailer is
// emitted unconditionally for every successful reply (SELECT, DML,
// DDL) so the conformance runner has a stable read-back surface
// independent of whether the query produced rows.
::grpc::Status EmitEmulatorRoute(
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  if (emulator_route.empty()) return ::grpc::Status::OK;
  v1::QueryResultRow trailer;
  trailer.set_emulator_route(std::string(emulator_route));
  if (!write(trailer)) {
    return ::grpc::Status(
        ::grpc::StatusCode::CANCELLED,
        "QueryService::ExecuteQuery: client cancelled stream before "
        "emulator_route trailer");
  }
  return ::grpc::Status::OK;
}

// Combined helper for the two end-of-stream trailers every
// successful reply emits. Order matches the proto contract
// documented on `QueryResultRow`: `statement_type` then
// `emulator_route`. Either trailer may legitimately be empty (an
// unrecognized statement kind or an unmapped disposition) and is
// then skipped, leaving the gateway free to omit the corresponding
// JSON envelope.
::grpc::Status EmitPhaseTimings(
    const backend::engine::PhaseRecorder* recorder,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  if (recorder == nullptr) return ::grpc::Status::OK;
  v1::QueryResultRow msg;
  recorder->ToProto(msg.mutable_phase_timings());
  if (msg.phase_timings().phases().empty()) return ::grpc::Status::OK;
  if (!write(msg)) {
    return ::grpc::Status(
        ::grpc::StatusCode::CANCELLED,
        "QueryService::ExecuteQuery: client cancelled stream before "
        "phase_timings trailer");
  }
  return ::grpc::Status::OK;
}

::grpc::Status EmitTrailers(
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const backend::engine::PhaseRecorder* phase_recorder,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  ::grpc::Status st = EmitPhaseTimings(phase_recorder, write);
  if (!st.ok()) return st;
  st = EmitStatementType(statement_type, write);
  if (!st.ok()) return st;
  return EmitEmulatorRoute(emulator_route, write);
}

// DML path: run `ExecuteDml`, then emit a single `dml_stats` message
// describing the row counts, then the trailing `statement_type` +
// `emulator_route` pair. The caller is responsible for selecting
// this path (i.e. having determined `cls == kDml`).
::grpc::Status EmitDmlStats(
    backend::engine::Engine* engine,
    const backend::engine::QueryRequest& request,
    ::googlesql::Catalog* catalog,
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  absl::StatusOr<backend::engine::DmlResult> result =
      engine->ExecuteDml(request, catalog);
  if (!result.ok()) return AnalyzeStatusToGrpc(result.status());

  if (result->returning_rows != nullptr) {
    v1::QueryResultRow schema_message;
    backend::schema::TableSchemaToProto(result->returning_rows->schema(),
                                        schema_message.mutable_schema());
    if (!write(schema_message)) {
      return ::grpc::Status(
          ::grpc::StatusCode::CANCELLED,
          "QueryService::ExecuteQuery: client cancelled stream before "
          "returning schema");
    }
    backend::storage::Row row;
    while (true) {
      absl::StatusOr<bool> next = result->returning_rows->Next(&row);
      if (!next.ok()) return AnalyzeStatusToGrpc(next.status());
      if (!*next) break;
      v1::QueryResultRow row_message;
      for (const auto& cell : row.cells) {
        QueryResultValueToCell(cell, row_message.add_cells());
      }
      if (!write(row_message)) {
        return ::grpc::Status(
            ::grpc::StatusCode::CANCELLED,
            "QueryService::ExecuteQuery: client cancelled stream before "
            "returning row");
      }
    }
  }

  v1::QueryResultRow stats_message;
  auto* proto_stats = stats_message.mutable_dml_stats();
  proto_stats->set_inserted_row_count(result->stats.inserted_row_count);
  proto_stats->set_updated_row_count(result->stats.updated_row_count);
  proto_stats->set_deleted_row_count(result->stats.deleted_row_count);
  if (!write(stats_message)) {
    return ::grpc::Status(
        ::grpc::StatusCode::CANCELLED,
        "QueryService::ExecuteQuery: client cancelled stream before "
        "dml_stats");
  }
  return EmitTrailers(
      statement_type, emulator_route, request.phase_recorder.get(), write);
}

// DDL path: run `ExecuteDdl` and propagate any failure. Successful
// DDL emits the trailing `statement_type` + `emulator_route` pair
// (the gateway uses them to populate
// `Job.statistics.query.{statementType,emulatorRoute}`); before the
// trailer pair was introduced the DDL reply was empty.
::grpc::Status EmitDdlResult(
    backend::engine::Engine* engine,
    const backend::engine::QueryRequest& request,
    ::googlesql::Catalog* catalog,
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  absl::Status ddl_status = engine->ExecuteDdl(request, catalog);
  if (!ddl_status.ok()) return AnalyzeStatusToGrpc(ddl_status);
  return EmitTrailers(
      statement_type, emulator_route, request.phase_recorder.get(), write);
}

// SELECT path: emit the schema message, then one row message per
// `RowSource::Next` until end-of-stream, then the trailing
// `statement_type` + `emulator_route` pair.
::grpc::Status StreamRows(
    backend::engine::Engine* engine,
    const backend::engine::QueryRequest& request,
    ::googlesql::Catalog* catalog,
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write) {
  absl::StatusOr<std::unique_ptr<backend::engine::RowSource>> source_or =
      engine->ExecuteQuery(request, catalog);
  if (!source_or.ok()) return AnalyzeStatusToGrpc(source_or.status());
  std::unique_ptr<backend::engine::RowSource> source =
      std::move(source_or).value();
  if (source == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::ExecuteQuery: engine returned a "
                          "null RowSource");
  }

  // First message: the schema. We always emit it (even for queries
  // that return zero rows) so the gateway can synthesize a BigQuery
  // REST `schema` field on the response without having to wait for
  // the first row.
  v1::QueryResultRow schema_message;
  backend::schema::TableSchemaToProto(source->schema(),
                                      schema_message.mutable_schema());
  if (!write(schema_message)) {
    return ::grpc::Status(
        ::grpc::StatusCode::CANCELLED,
        "QueryService::ExecuteQuery: client cancelled stream before schema");
  }

  // Subsequent messages: one per result row. `RowSource::Next`
  // returns false on end-of-stream and a non-OK status on a
  // mid-stream failure; both surface to the caller through the
  // standard gRPC status code mapping.
  backend::storage::Row row;
  int64_t row_stream_us = 0;
  while (true) {
    const absl::Time row_start = absl::Now();
    absl::StatusOr<bool> next = source->Next(&row);
    row_stream_us += absl::ToInt64Microseconds(absl::Now() - row_start);
    if (!next.ok()) return AnalyzeStatusToGrpc(next.status());
    if (!*next) break;
    v1::QueryResultRow row_message;
    for (const auto& cell : row.cells) {
      QueryResultValueToCell(cell, row_message.add_cells());
    }
    if (!write(row_message)) {
      return ::grpc::Status(
          ::grpc::StatusCode::CANCELLED,
          "QueryService::ExecuteQuery: client cancelled stream mid-row");
    }
  }
  if (request.phase_recorder != nullptr) {
    request.phase_recorder->Record("row_stream", row_stream_us);
  }
  return EmitTrailers(
      statement_type, emulator_route, request.phase_recorder.get(), write);
}
}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
