

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {

namespace {

absl::flat_hash_map<std::string, int> InputColumnIdsByName(
    const ::googlesql::ResolvedScan* input_scan) {
  absl::flat_hash_map<std::string, int> out;
  if (input_scan == nullptr) {
    return out;
  }
  for (int i = 0; i < input_scan->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = input_scan->column_list(i);
    out.emplace(absl::AsciiStrToLower(col.name()), col.column_id());
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> SingleNullRow(
    const ::googlesql::ResolvedTVFScan& tvf_scan) {
  ColumnBindings row;
  row.reserve(tvf_scan.column_list_size());
  for (int i = 0; i < tvf_scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = tvf_scan.column_list(i);
    if (col.type() == nullptr) {
      return absl::InternalError(
          "semantic stub: ML TVF output column has null type");
    }
    row.emplace(col.column_id(), Value::Null(col.type()));
  }
  return std::vector<ColumnBindings>{std::move(row)};
}

absl::StatusOr<std::vector<ColumnBindings>> PassThroughWithNullExtensions(
    const ::googlesql::ResolvedTVFScan& tvf_scan,
    const std::vector<ColumnBindings>& input_rows,
    const ::googlesql::ResolvedScan* input_scan) {
  const absl::flat_hash_map<std::string, int> input_ids =
      InputColumnIdsByName(input_scan);
  std::vector<ColumnBindings> out;
  out.reserve(input_rows.size());
  for (const ColumnBindings& in_row : input_rows) {
    ColumnBindings row;
    row.reserve(tvf_scan.column_list_size());
    for (int i = 0; i < tvf_scan.column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& out_col = tvf_scan.column_list(i);
      if (out_col.type() == nullptr) {
        return absl::InternalError(
            "semantic stub: ML TVF output column has null type");
      }
      const auto it = input_ids.find(absl::AsciiStrToLower(out_col.name()));
      if (it != input_ids.end()) {
        const auto value_it = in_row.find(it->second);
        if (value_it != in_row.end()) {
          row.emplace(out_col.column_id(), value_it->second);
          continue;
        }
      }
      row.emplace(out_col.column_id(), Value::Null(out_col.type()));
    }
    out.push_back(std::move(row));
  }
  return out;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MlPredictStub(
    const ::googlesql::ResolvedTVFScan& scan,
    const std::vector<ColumnBindings>& input_rows,
    const ::googlesql::ResolvedScan* input_scan) {
  if (input_scan == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic stub: ML.PREDICT requires a TABLE input argument");
  }
  return PassThroughWithNullExtensions(scan, input_rows, input_scan);
}

absl::StatusOr<std::vector<ColumnBindings>> MlEvaluateStub(
    const ::googlesql::ResolvedTVFScan& scan) {
  return SingleNullRow(scan);
}

absl::StatusOr<std::vector<ColumnBindings>> MlForecastStub(
    const ::googlesql::ResolvedTVFScan& scan) {
  return SingleNullRow(scan);
}

}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
