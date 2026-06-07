#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_INTERNAL_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_INTERNAL_H_

#include <functional>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "grpcpp/support/status.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

::grpc::Status AnalyzeStatusToGrpc(const absl::Status& status);

::googlesql::AnalyzerOptions MakeAnalyzerOptions();

void QueryResultValueToCell(const backend::storage::Value& value,
                            v1::Cell* out);

backend::engine::QueryRequest ProtoToEngineRequest(
    const v1::QueryRequest& request);

enum class StatementClass { kSelect, kDml, kDdl, kOther };

StatementClass ClassifyStatement(::googlesql::ResolvedNodeKind kind);

// Multi-statement scripts (DECLARE/CALL/CREATE CONSTANT) must bypass
// single-statement pre-classification and run through the engine's
// AnalyzeNextStatement loop.
bool NeedsScriptStatementLoop(absl::string_view sql);

absl::string_view StatementTypeFor(const ::googlesql::ResolvedStatement& stmt);

::grpc::Status ValidateQueryRequest(
    const backend::storage::Storage* storage,
    const v1::QueryRequest& request,
    const std::function<bool(const v1::QueryResultRow&)>& write);

::grpc::Status EmitStatementType(
    absl::string_view statement_type,
    const std::function<bool(const v1::QueryResultRow&)>& write);

::grpc::Status EmitEmulatorRoute(
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write);

::grpc::Status EmitTrailers(
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write);

::grpc::Status EmitDmlStats(
    backend::engine::Engine* engine,
    const backend::engine::QueryRequest& request,
    ::googlesql::Catalog* catalog,
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write);

::grpc::Status EmitDdlResult(
    backend::engine::Engine* engine,
    const backend::engine::QueryRequest& request,
    ::googlesql::Catalog* catalog,
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write);

::grpc::Status StreamRows(
    backend::engine::Engine* engine,
    const backend::engine::QueryRequest& request,
    ::googlesql::Catalog* catalog,
    absl::string_view statement_type,
    absl::string_view emulator_route,
    const std::function<bool(const v1::QueryResultRow&)>& write);

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_INTERNAL_H_
