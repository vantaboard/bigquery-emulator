#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_MEASURE_CATALOG_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_MEASURE_CATALOG_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "absl/functional/function_ref.h"
#include "backend/schema/schema.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace googlesql {
class AnalyzerOutput;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Parsed from `ColumnSchema::description` when it uses the conformance /
// catalog marker `bqemu_measure:<expr>:<key1,key2,...>`.
struct MeasureColumnSpec {
  std::string name;
  std::string expression;
  std::vector<std::string> row_key_names;
};

// Returns a spec when `column.description` carries the `bqemu_measure:` marker.
std::optional<MeasureColumnSpec> ParseMeasureColumnSpec(
    const schema::ColumnSchema& column);

// Returns a copy of `schema` with measure-marker columns removed. DuckDB
// parquet materialization stores only physical columns; measure metadata lives
// in the sidecar schema returned by `GetSchema`.
schema::TableSchema StripMeasureColumns(const schema::TableSchema& schema);

using ColumnTypeFn = absl::FunctionRef<
    absl::StatusOr<const ::googlesql::Type*>(const schema::ColumnSchema&)>;

// Builds `SimpleTable::NameAndType` entries for every non-measure column and
// returns the matching physical schema (measure columns stripped).
absl::StatusOr<std::vector<::googlesql::SimpleTable::NameAndType>>
BuildPhysicalNameAndTypes(const schema::TableSchema& schema,
                          schema::TableSchema* physical_schema,
                          ColumnTypeFn to_type);

// Resolves `measure_expr` against the non-measure columns already present on
// `table` and returns the analyzed expression AST. The returned
// `AnalyzerOutput` is moved into `measure_outputs` so the resolved pointers
// stay valid for the table's lifetime.
absl::StatusOr<const ::googlesql::ResolvedExpr*> ResolveMeasureExpression(
    absl::string_view measure_expr,
    const ::googlesql::Table& table,
    ::googlesql::Catalog& catalog,
    ::googlesql::TypeFactory& type_factory,
    const ::googlesql::LanguageOptions& language,
    std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>&
        measure_outputs,
    std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>&
        measure_resolved_exprs);

// Adds a MEASURE-typed `SimpleColumn` with catalog metadata to `table`.
// `row_key_names` name existing columns on `table` used for grain-locking.
absl::Status AddMeasureColumnToTable(
    ::googlesql::SimpleTable& table,
    absl::string_view name,
    absl::string_view expression,
    absl::Span<const std::string> row_key_names,
    ::googlesql::Catalog& catalog,
    ::googlesql::TypeFactory& type_factory,
    const ::googlesql::LanguageOptions& language,
    std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>&
        measure_outputs,
    std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>&
        measure_resolved_exprs);

// Applies every `bqemu_measure:` column in `schema` to `table`. Non-measure
// columns must already be present. When `row_identity` is non-empty it is
// forwarded to `SimpleTable::SetRowIdentityColumns` (column indices on the
// pre-measure column list). Converted measure expression nodes are appended to
// `measure_resolved_exprs` so their pointers outlive the table.
absl::Status ApplyMeasureColumnsFromSchema(
    ::googlesql::SimpleTable& table,
    const schema::TableSchema& schema,
    ::googlesql::Catalog& catalog,
    ::googlesql::TypeFactory& type_factory,
    const ::googlesql::LanguageOptions& language,
    std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>&
        measure_outputs,
    std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>&
        measure_resolved_exprs);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_MEASURE_CATALOG_H_
