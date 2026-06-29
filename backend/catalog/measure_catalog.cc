#include "backend/catalog/measure_catalog.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "googlesql/common/measure_utils.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

constexpr absl::string_view kMeasureMarker = "bqemu_measure:";

const ::googlesql::ResolvedExpr* ExtractSingleMeasureExpression(
    const ::googlesql::ResolvedScan* scan);

const ::googlesql::ResolvedExpr* ExtractFromProjectScan(
    const ::googlesql::ResolvedProjectScan* project) {
  if (project == nullptr) return nullptr;
  if (project->input_scan() != nullptr) {
    if (const ::googlesql::ResolvedExpr* inner =
            ExtractSingleMeasureExpression(project->input_scan())) {
      return inner;
    }
  }
  if (project->expr_list_size() != 1) return nullptr;
  const ::googlesql::ResolvedComputedColumn* cc = project->expr_list(0);
  if (cc == nullptr || cc->expr() == nullptr) return nullptr;
  return cc->expr();
}

const ::googlesql::ResolvedExpr* ExtractFromAggregateScan(
    const ::googlesql::ResolvedScan* scan) {
  const auto* aggregate =
      dynamic_cast<const ::googlesql::ResolvedAggregateScanBase*>(scan);
  if (aggregate == nullptr) return nullptr;
  if (aggregate->aggregate_list_size() != 1) return nullptr;
  const ::googlesql::ResolvedComputedColumnBase* cc =
      aggregate->aggregate_list(0);
  if (cc == nullptr || cc->expr() == nullptr) return nullptr;
  return cc->expr();
}

const ::googlesql::ResolvedScan* NextPassthroughInputScan(
    const ::googlesql::ResolvedScan* scan) {
  if (scan == nullptr) return nullptr;
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_FILTER_SCAN:
      return scan->GetAs<::googlesql::ResolvedFilterScan>()->input_scan();
    case ::googlesql::RESOLVED_BARRIER_SCAN:
      return scan->GetAs<::googlesql::ResolvedBarrierScan>()->input_scan();
    default:
      return nullptr;
  }
}

const ::googlesql::ResolvedExpr* ExtractSingleMeasureExpression(
    const ::googlesql::ResolvedScan* scan) {
  while (scan != nullptr) {
    switch (scan->node_kind()) {
      case ::googlesql::RESOLVED_PROJECT_SCAN:
        return ExtractFromProjectScan(
            scan->GetAs<::googlesql::ResolvedProjectScan>());
      case ::googlesql::RESOLVED_AGGREGATE_SCAN:
      case ::googlesql::RESOLVED_ANONYMIZED_AGGREGATE_SCAN:
      case ::googlesql::RESOLVED_DIFFERENTIAL_PRIVACY_AGGREGATE_SCAN:
      case ::googlesql::RESOLVED_AGGREGATION_THRESHOLD_AGGREGATE_SCAN:
        return ExtractFromAggregateScan(scan);
      default:
        scan = NextPassthroughInputScan(scan);
        if (scan == nullptr) return nullptr;
        continue;
    }
  }
  return nullptr;
}

absl::StatusOr<std::unique_ptr<const ::googlesql::ResolvedExpr>>
ConvertMeasureExprToExpressionColumns(const ::googlesql::ResolvedExpr& expr);

absl::StatusOr<std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>>
ConvertChildExprs(
    absl::Span<const std::unique_ptr<const ::googlesql::ResolvedExpr>>
        children) {
  std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>> converted;
  converted.reserve(children.size());
  for (const auto& child : children) {
    if (child == nullptr) {
      return absl::InternalError(
          "measure_catalog: null child while converting measure expression");
    }
    absl::StatusOr<std::unique_ptr<const ::googlesql::ResolvedExpr>> copied =
        ConvertMeasureExprToExpressionColumns(*child);
    if (!copied.ok()) return copied.status();
    converted.push_back(std::move(*copied));
  }
  return converted;
}

absl::StatusOr<std::unique_ptr<const ::googlesql::ResolvedExpr>>
ConvertMeasureExprToExpressionColumns(const ::googlesql::ResolvedExpr& expr) {
  switch (expr.node_kind()) {
    case ::googlesql::RESOLVED_EXPRESSION_COLUMN:
      return ::googlesql::MakeResolvedExpressionColumn(
          expr.type(),
          expr.GetAs<::googlesql::ResolvedExpressionColumn>()->name());
    case ::googlesql::RESOLVED_COLUMN_REF: {
      const auto* cref = expr.GetAs<::googlesql::ResolvedColumnRef>();
      if (cref == nullptr) {
        return absl::InternalError(
            "measure_catalog: RESOLVED_COLUMN_REF node cast failed");
      }
      return ::googlesql::MakeResolvedExpressionColumn(
          cref->type(), absl::AsciiStrToLower(cref->column().name()));
    }
    case ::googlesql::RESOLVED_CAST: {
      const auto* cast = expr.GetAs<::googlesql::ResolvedCast>();
      if (cast == nullptr || cast->expr() == nullptr) {
        return absl::InternalError("measure_catalog: invalid RESOLVED_CAST");
      }
      absl::StatusOr<std::unique_ptr<const ::googlesql::ResolvedExpr>> input =
          ConvertMeasureExprToExpressionColumns(*cast->expr());
      if (!input.ok()) return input.status();
      return ::googlesql::MakeResolvedCast(
          cast->type(), std::move(*input), cast->return_null_on_error());
    }
    case ::googlesql::RESOLVED_FUNCTION_CALL: {
      const auto* call = expr.GetAs<::googlesql::ResolvedFunctionCall>();
      if (call == nullptr) {
        return absl::InternalError(
            "measure_catalog: RESOLVED_FUNCTION_CALL node cast failed");
      }
      absl::StatusOr<
          std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>>
          args = ConvertChildExprs(call->argument_list());
      if (!args.ok()) return args.status();
      return ::googlesql::MakeResolvedFunctionCall(call->type(),
                                                   call->function(),
                                                   call->signature(),
                                                   std::move(*args),
                                                   call->error_mode(),
                                                   call->function_call_info());
    }
    case ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL: {
      const auto* agg =
          expr.GetAs<::googlesql::ResolvedAggregateFunctionCall>();
      if (agg == nullptr) {
        return absl::InternalError(
            "measure_catalog: RESOLVED_AGGREGATE_FUNCTION_CALL cast failed");
      }
      absl::StatusOr<
          std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>>
          args = ConvertChildExprs(agg->argument_list());
      if (!args.ok()) return args.status();
      std::unique_ptr<const ::googlesql::ResolvedExpr> where_expr;
      if (agg->where_expr() != nullptr) {
        absl::StatusOr<std::unique_ptr<const ::googlesql::ResolvedExpr>> where =
            ConvertMeasureExprToExpressionColumns(*agg->where_expr());
        if (!where.ok()) return where.status();
        where_expr = std::move(*where);
      }
      return ::googlesql::MakeResolvedAggregateFunctionCall(
          agg->type(),
          agg->function(),
          agg->signature(),
          std::move(*args),
          agg->error_mode(),
          agg->distinct(),
          agg->null_handling_modifier(),
          /*having_modifier=*/nullptr,
          /*order_by_item_list=*/{},
          /*limit=*/nullptr);
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "measure_catalog: unsupported node kind in measure expression: ",
          expr.node_kind_string()));
  }
}

::googlesql::AnalyzerOptions MakeMeasureAnalyzerOptions(
    const ::googlesql::LanguageOptions& language) {
  ::googlesql::LanguageOptions lang = language;
  lang.EnableMaximumLanguageFeaturesForDevelopment();
  lang.EnableLanguageFeature(::googlesql::FEATURE_ENABLE_MEASURES);
  lang.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ::googlesql::AnalyzerOptions options(lang);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.disable_rewrite(::googlesql::REWRITE_MEASURE_TYPE);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

absl::StatusOr<std::vector<int>> RowKeyIndicesForTable(
    const ::googlesql::SimpleTable& table,
    absl::Span<const std::string> row_key_names) {
  std::vector<int> indices;
  indices.reserve(row_key_names.size());
  for (absl::string_view key_name : row_key_names) {
    bool found = false;
    for (int i = 0; i < table.NumColumns(); ++i) {
      const ::googlesql::Column* column = table.GetColumn(i);
      if (column != nullptr && column->Name() == key_name) {
        indices.push_back(i);
        found = true;
        break;
      }
    }
    if (!found) {
      return absl::InvalidArgumentError(
          absl::StrCat("measure_catalog: row key column '",
                       key_name,
                       "' is not present on table '",
                       table.Name(),
                       "'"));
    }
  }
  return indices;
}

}  // namespace

std::optional<MeasureColumnSpec> ParseMeasureColumnSpec(
    const schema::ColumnSchema& column) {
  if (!absl::StartsWith(column.description, kMeasureMarker)) {
    return std::nullopt;
  }
  std::string payload = column.description.substr(kMeasureMarker.size());
  const std::vector<std::string> parts = absl::StrSplit(payload, ':');
  if (parts.size() != 2 || parts[0].empty()) {
    return std::nullopt;
  }
  MeasureColumnSpec spec;
  spec.name = column.name;
  spec.expression = parts[0];
  for (absl::string_view key :
       absl::StrSplit(parts[1], ',', absl::SkipEmpty())) {
    spec.row_key_names.push_back(std::string(absl::StripAsciiWhitespace(key)));
  }
  return spec;
}

schema::TableSchema StripMeasureColumns(const schema::TableSchema& schema) {
  schema::TableSchema physical;
  physical.columns.reserve(schema.columns.size());
  for (const schema::ColumnSchema& column : schema.columns) {
    if (ParseMeasureColumnSpec(column).has_value()) continue;
    physical.columns.push_back(column);
  }
  return physical;
}

absl::StatusOr<std::vector<::googlesql::SimpleTable::NameAndType>>
BuildPhysicalNameAndTypes(const schema::TableSchema& schema,
                          schema::TableSchema* physical_schema,
                          ColumnTypeFn to_type) {
  if (physical_schema == nullptr) {
    return absl::InvalidArgumentError(
        "measure_catalog: BuildPhysicalNameAndTypes requires physical_schema");
  }
  physical_schema->columns.clear();
  physical_schema->columns.reserve(schema.columns.size());
  std::vector<::googlesql::SimpleTable::NameAndType> columns;
  columns.reserve(schema.columns.size());
  for (const schema::ColumnSchema& column : schema.columns) {
    if (ParseMeasureColumnSpec(column).has_value()) continue;
    physical_schema->columns.push_back(column);
    absl::StatusOr<const ::googlesql::Type*> column_type = to_type(column);
    if (!column_type.ok()) return column_type.status();
    columns.emplace_back(column.name, *column_type);
  }
  return columns;
}

absl::StatusOr<const ::googlesql::ResolvedExpr*> ResolveMeasureExpression(
    absl::string_view measure_expr,
    const ::googlesql::Table& table,
    ::googlesql::Catalog& catalog,
    ::googlesql::TypeFactory& type_factory,
    const ::googlesql::LanguageOptions& language,
    std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>&
        measure_outputs,
    std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>&
        measure_resolved_exprs) {
  ::googlesql::AnalyzerOptions options = MakeMeasureAnalyzerOptions(language);
  const std::string sql =
      absl::StrCat("SELECT ", measure_expr, " FROM ", table.FullName());
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed = ::googlesql::AnalyzeStatement(
      sql, options, &catalog, &type_factory, &output);
  if (!analyzed.ok()) return analyzed;
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "measure_catalog: AnalyzeStatement returned null output");
  }
  const auto* query_stmt =
      output->resolved_statement()->GetAs<::googlesql::ResolvedQueryStmt>();
  if (query_stmt == nullptr || query_stmt->query() == nullptr) {
    return absl::InternalError(
        "measure_catalog: measure expression did not analyze to a query");
  }
  const ::googlesql::ResolvedScan* query = query_stmt->query();
  const ::googlesql::ResolvedExpr* resolved =
      ExtractSingleMeasureExpression(query);
  if (resolved == nullptr) {
    return absl::InternalError(absl::StrCat(
        "measure_catalog: measure expression analyzed to unsupported "
        "scan kind ",
        query == nullptr ? "null" : query->node_kind_string()));
  }
  absl::StatusOr<std::unique_ptr<const ::googlesql::ResolvedExpr>>
      catalog_expr = ConvertMeasureExprToExpressionColumns(*resolved);
  if (!catalog_expr.ok()) return catalog_expr.status();
  absl::Status validated = ::googlesql::ValidateMeasureExpression(
      measure_expr, **catalog_expr, language, table.Name());
  if (!validated.ok()) return validated;

  const ::googlesql::ResolvedExpr* held = catalog_expr->get();
  measure_resolved_exprs.push_back(std::move(*catalog_expr));
  return held;
}

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
        measure_resolved_exprs) {
  if (table.FindColumnByName(std::string(name)) != nullptr) {
    return absl::AlreadyExistsError(
        absl::StrCat("measure_catalog: measure column '",
                     name,
                     "' already exists on table '",
                     table.Name(),
                     "'"));
  }
  absl::StatusOr<const ::googlesql::ResolvedExpr*> resolved_or =
      ResolveMeasureExpression(expression,
                               table,
                               catalog,
                               type_factory,
                               language,
                               measure_outputs,
                               measure_resolved_exprs);
  if (!resolved_or.ok()) return resolved_or.status();
  const ::googlesql::ResolvedExpr* resolved = *resolved_or;
  if (resolved->type() == nullptr) {
    return absl::InvalidArgumentError(absl::StrCat(
        "measure_catalog: measure expression for '", name, "' has null type"));
  }
  const ::googlesql::Type* result_type = resolved->type();
  absl::StatusOr<const ::googlesql::Type*> measure_type_or =
      type_factory.MakeMeasureType(result_type);
  if (!measure_type_or.ok()) return measure_type_or.status();

  std::optional<std::vector<int>> row_identity_indices;
  if (!row_key_names.empty()) {
    absl::StatusOr<std::vector<int>> indices_or =
        RowKeyIndicesForTable(table, row_key_names);
    if (!indices_or.ok()) return indices_or.status();
    row_identity_indices = std::move(*indices_or);
  }

  absl::StatusOr<::googlesql::Column::ExpressionAttributes> attrs_or =
      ::googlesql::Column::ExpressionAttributes::Create(
          ::googlesql::Column::ExpressionAttributes::ExpressionKind::
              MEASURE_EXPRESSION,
          std::string(expression),
          resolved,
          row_identity_indices);
  if (!attrs_or.ok()) return attrs_or.status();

  ::googlesql::SimpleColumn::Attributes attrs;
  attrs.column_expression = std::move(*attrs_or);
  auto column = std::make_unique<::googlesql::SimpleColumn>(
      std::string(table.Name()), std::string(name), *measure_type_or, attrs);
  return table.AddColumn(std::move(column));
}

absl::Status ApplyMeasureColumnsFromSchema(
    ::googlesql::SimpleTable& table,
    const schema::TableSchema& schema,
    ::googlesql::Catalog& catalog,
    ::googlesql::TypeFactory& type_factory,
    const ::googlesql::LanguageOptions& language,
    std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>&
        measure_outputs,
    std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>>&
        measure_resolved_exprs) {
  std::vector<std::string> table_row_keys;
  for (const schema::ColumnSchema& column : schema.columns) {
    std::optional<MeasureColumnSpec> spec = ParseMeasureColumnSpec(column);
    if (!spec.has_value()) continue;
    absl::Span<const std::string> keys = spec->row_key_names;
    if (keys.empty()) {
      keys = table_row_keys;
    } else if (table_row_keys.empty()) {
      table_row_keys = spec->row_key_names;
    }
    absl::Status added = AddMeasureColumnToTable(table,
                                                 spec->name,
                                                 spec->expression,
                                                 keys,
                                                 catalog,
                                                 type_factory,
                                                 language,
                                                 measure_outputs,
                                                 measure_resolved_exprs);
    if (!added.ok()) return added;
  }
  if (!table_row_keys.empty()) {
    absl::StatusOr<std::vector<int>> indices_or =
        RowKeyIndicesForTable(table, table_row_keys);
    if (!indices_or.ok()) return indices_or.status();
    return table.SetRowIdentityColumns(std::move(*indices_or));
  }
  return absl::OkStatus();
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
