#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

namespace {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

std::string StratifyFingerprint(const std::vector<Value>& keys) {
  std::string fp;
  for (const Value& key : keys) {
    absl::StrAppend(&fp, key.DebugString(), "\x1e");
  }
  return fp;
}

absl::StatusOr<double> EvalSampleWeight(
    const ::googlesql::ResolvedSampleScan& scan,
    const ColumnBindings& row,
    EvalContext& ctx) {
  if (scan.weight_column() == nullptr) return 1.0;
  const int col_id = scan.weight_column()->column().column_id();
  auto it = row.find(col_id);
  if (it == row.end()) {
    const absl::string_view weight_name = scan.weight_column()->column().name();
    const ::googlesql::ResolvedScan* input =
        StripBarrierScans(scan.input_scan());
    const auto* table = input != nullptr
                            ? input->GetAs<::googlesql::ResolvedTableScan>()
                            : nullptr;
    if (table != nullptr) {
      for (int i = 0; i < table->column_list_size(); ++i) {
        const ::googlesql::ResolvedColumn& col = table->column_list(i);
        if (col.name() == weight_name) {
          it = row.find(col.column_id());
          if (it != row.end()) break;
        }
      }
    }
  }
  if (it == row.end()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: sample weight column missing");
  }
  const Value& weight = it->second;
  if (weight.is_null()) return 0.0;
  if (weight.type_kind() == ::googlesql::TYPE_INT64) {
    return static_cast<double>(weight.int64_value());
  }
  if (weight.type_kind() == ::googlesql::TYPE_DOUBLE) {
    return weight.double_value();
  }
  if (weight.type_kind() == ::googlesql::TYPE_FLOAT) {
    return static_cast<double>(weight.float_value());
  }
  if (weight.type_kind() == ::googlesql::TYPE_NUMERIC) {
    return weight.numeric_value().ToDouble();
  }
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           "semantic: sample weight must be numeric");
}

absl::StatusOr<std::vector<Value>> StratifyKeys(
    const ::googlesql::ResolvedSampleScan& scan,
    const ColumnBindings& row,
    const EvalContext& ctx) {
  std::vector<Value> keys;
  keys.reserve(static_cast<size_t>(scan.partition_by_list_size()));
  for (int i = 0; i < scan.partition_by_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* expr = scan.partition_by_list(i);
    if (expr == nullptr) continue;
    EvalContext row_ctx = ctx;
    row_ctx.columns = &row;
    auto key = EvalExpr(*expr, row_ctx);
    if (!key.ok()) return key.status();
    keys.push_back(*std::move(key));
  }
  return keys;
}

uint64_t SampleSeed(const ::googlesql::ResolvedSampleScan& scan,
                    EvalContext& ctx) {
  if (scan.repeatable_argument() == nullptr) {
    std::random_device rd;
    return static_cast<uint64_t>(rd());
  }
  auto seed_or = EvalExpr(*scan.repeatable_argument(), ctx);
  if (!seed_or.ok() || seed_or->is_null()) return 0;
  if (seed_or->type_kind() == ::googlesql::TYPE_INT64) {
    return static_cast<uint64_t>(seed_or->int64_value());
  }
  if (seed_or->type_kind() == ::googlesql::TYPE_DOUBLE) {
    return static_cast<uint64_t>(seed_or->double_value());
  }
  return 0;
}

absl::StatusOr<std::vector<ColumnBindings>> BernoulliPercentSample(
    const ::googlesql::ResolvedSampleScan& scan,
    std::vector<ColumnBindings> rows,
    EvalContext& ctx) {
  if (scan.size() == nullptr) {
    return absl::InvalidArgumentError("semantic: TABLESAMPLE missing size");
  }
  auto size_or = EvalExpr(*scan.size(), ctx);
  if (!size_or.ok()) return size_or.status();
  if (size_or->is_null()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: TABLESAMPLE size must not be NULL");
  }
  double percent = 0.0;
  if (size_or->type_kind() == ::googlesql::TYPE_INT64) {
    percent = static_cast<double>(size_or->int64_value());
  } else if (size_or->type_kind() == ::googlesql::TYPE_DOUBLE) {
    percent = size_or->double_value();
  } else {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: TABLESAMPLE size must be numeric");
  }
  if (percent < 0.0 || percent > 100.0) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: TABLESAMPLE percent out of range");
  }
  const double fraction = percent / 100.0;
  const uint64_t seed = SampleSeed(scan, ctx);
  std::mt19937_64 rng(seed);

  absl::flat_hash_map<std::string, double> stratum_max_weight;
  for (size_t r = 0; r < rows.size(); ++r) {
    auto keys_or = StratifyKeys(scan, rows[r], ctx);
    if (!keys_or.ok()) return keys_or.status();
    auto weight_or = EvalSampleWeight(scan, rows[r], ctx);
    if (!weight_or.ok()) return weight_or.status();
    const std::string fp = StratifyFingerprint(*keys_or);
    stratum_max_weight[fp] = std::max(stratum_max_weight[fp], *weight_or);
  }

  std::vector<ColumnBindings> out;
  out.reserve(rows.size());
  for (size_t r = 0; r < rows.size(); ++r) {
    auto keys_or = StratifyKeys(scan, rows[r], ctx);
    if (!keys_or.ok()) return keys_or.status();
    auto weight_or = EvalSampleWeight(scan, rows[r], ctx);
    if (!weight_or.ok()) return weight_or.status();
    const std::string fp = StratifyFingerprint(*keys_or);
    const double max_w = stratum_max_weight[fp];
    double probability = fraction;
    if (fraction < 1.0 && max_w > 0.0 && scan.weight_column() != nullptr) {
      probability = fraction * (*weight_or / max_w);
    }
    if (probability >= 1.0) {
      out.push_back(rows[r]);
      continue;
    }
    if (probability <= 0.0) continue;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < probability) {
      out.push_back(rows[r]);
    }
  }
  return out;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSampleScan(
    const ::googlesql::ResolvedSampleScan& scan, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  std::vector<ColumnBindings> rows = *std::move(input_or);
  if (rows.empty()) return rows;

  const std::string method = absl::AsciiStrToLower(scan.method());
  if (scan.unit() != ::googlesql::ResolvedSampleScan::PERCENT) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic: TABLESAMPLE ",
                     method,
                     " with ROWS unit on semantic path is not implemented"));
  }
  if (method != "bernoulli" && method != "system") {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic: TABLESAMPLE method ",
                     method,
                     " is not implemented on semantic path"));
  }
  // SYSTEM weighted/stratified sampling is approximated with the same
  // per-row Bernoulli draw used for BERNOULLI; exact block semantics
  // differ from BigQuery production.
  return BernoulliPercentSample(scan, std::move(rows), ctx);
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
