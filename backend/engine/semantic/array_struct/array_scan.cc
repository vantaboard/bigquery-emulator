#include "backend/engine/semantic/array_struct/array_scan.h"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace array_struct {

namespace {

void AliasUnnestPublicColumnIds(const ::googlesql::ResolvedArrayScan& scan,
                                int n_arrays,
                                ColumnBindings& bindings);
void InjectArrayScanInternalColumns(const ::googlesql::ResolvedArrayScan& scan,
                                    ColumnBindings& bindings);

enum class ZipMode {
  kPad,
  kTruncate,
  kStrict,
};

// Resolve the `array_zip_mode` enum literal the analyzer attaches
// to a multi-array UNNEST. The analyzer guarantees the expr is a
// `ResolvedLiteral` of opaque enum kind `ARRAY_ZIP_MODE` with one
// of three values: `PAD`, `TRUNCATE`, `STRICT`. We extract the
// enum NAME (not value index) so the dispatch below stays
// human-readable.
absl::StatusOr<ZipMode> ResolveZipMode(
    const ::googlesql::ResolvedExpr* mode_expr) {
  // Default mode per GoogleSQL is PAD. The analyzer always attaches
  // a literal here when `array_zip_mode` is non-null, but
  // defense-in-depth: if a nullptr leaks through, treat it as PAD.
  if (mode_expr == nullptr) return ZipMode::kPad;
  // `EvalExpr` would recurse into a `ResolvedLiteral` and copy the
  // analyzer-validated `Value`; the enum here is opaque, so we
  // resolve directly from the literal without going through the
  // generic dispatch.
  if (mode_expr->node_kind() != ::googlesql::RESOLVED_LITERAL) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: ResolvedArrayScan.array_zip_mode expected a "
                     "ResolvedLiteral; got ",
                     mode_expr->node_kind_string()));
  }
  const auto& lit = *mode_expr->GetAs<::googlesql::ResolvedLiteral>();
  // cpp-lint:allow(statusor-unchecked-value) -- ResolvedLiteral, not StatusOr
  const Value& v = lit.value();
  if (v.is_null() || v.type_kind() != ::googlesql::TYPE_ENUM) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: ResolvedArrayScan.array_zip_mode literal is not a "
        "non-NULL ENUM value");
  }
  auto name = v.EnumName();
  if (!name.ok()) return name.status();
  // ARRAY_ZIP_MODE enum names per the analyzer.
  if (*name == "PAD") return ZipMode::kPad;
  if (*name == "TRUNCATE") return ZipMode::kTruncate;
  if (*name == "STRICT") return ZipMode::kStrict;
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      absl::StrCat("semantic: unrecognized ARRAY_ZIP_MODE '", *name, "'"));
}

// Produce a NULL value of the analyzer-declared element type. Used
// by the outer-UNNEST path (`is_outer == true`) and the PAD zip
// mode when one of the input arrays is shorter than the longest.
Value NullElement(const ::googlesql::Type* element_type) {
  if (element_type == nullptr) return Value::NullInt64();
  return Value::Null(element_type);
}

// Evaluate `array_expr_list(i)` against `ctx` and return the
// resulting `Value`. The analyzer guarantees the expression has
// ARRAY type; we double-check to keep the failure path
// human-readable.
absl::StatusOr<Value> EvaluateArrayExpr(const ::googlesql::ResolvedExpr* expr,
                                        const EvalContext& ctx) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedArrayScan has a null array_expr_list entry");
  }
  auto v = EvalExpr(*expr, ctx);
  if (!v.ok()) return v.status();
  if (v->type_kind() != ::googlesql::TYPE_ARRAY) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat(
            "semantic: ResolvedArrayScan.array_expr_list entry is not an "
            "ARRAY; got ",
            v->type()->DebugString()));
  }
  return v;
}

int ComputeZipRowCount(ZipMode zip_mode,
                       const std::vector<int>& sizes,
                       int n_arrays) {
  if (n_arrays == 1) return sizes[0];
  switch (zip_mode) {
    case ZipMode::kPad: {
      int row_count = 0;
      for (int s : sizes)
        row_count = std::max(row_count, s);
      return row_count;
    }
    case ZipMode::kTruncate: {
      int row_count = sizes[0];
      for (int i = 1; i < n_arrays; ++i) {
        row_count = std::min(row_count, sizes[i]);
      }
      return row_count;
    }
    case ZipMode::kStrict:
      return sizes[0];
  }
  return 0;
}

absl::StatusOr<int> ValidateStrictZipSizes(const std::vector<int>& sizes) {
  const int row_count = sizes[0];
  for (int i = 1; i < static_cast<int>(sizes.size()); ++i) {
    if (sizes[i] != row_count) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("semantic: UNNEST with mode STRICT requires arrays of "
                       "equal length; got array #0 length ",
                       sizes[0],
                       " and array #",
                       i,
                       " length ",
                       sizes[i]));
    }
  }
  return row_count;
}

ColumnBindings BuildOuterUnnestNullRow(
    const ::googlesql::ResolvedArrayScan& scan, int n_arrays) {
  ColumnBindings bindings;
  bindings.reserve(n_arrays + 1);
  for (int i = 0; i < n_arrays; ++i) {
    bindings.emplace(scan.element_column_list(i).column_id(),
                     NullElement(scan.element_column_list(i).type()));
  }
  if (scan.array_offset_column() != nullptr) {
    bindings.emplace(scan.array_offset_column()->column().column_id(),
                     Value::NullInt64());
  }
  AliasUnnestPublicColumnIds(scan, n_arrays, bindings);
  InjectArrayScanInternalColumns(scan, bindings);
  return bindings;
}

}  // namespace

void AliasUnnestPublicColumnIds(const ::googlesql::ResolvedArrayScan& scan,
                                int n_arrays,
                                ColumnBindings& bindings) {
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const int public_id = scan.column_list(i).column_id();
    if (bindings.count(public_id) != 0) continue;
    if (i < n_arrays) {
      const int element_id = scan.element_column_list(i).column_id();
      auto it = bindings.find(element_id);
      if (it != bindings.end()) {
        bindings.emplace(public_id, it->second);
        continue;
      }
    }
    const std::string& pub_name = scan.column_list(i).name();
    for (int j = 0; j < n_arrays; ++j) {
      if (scan.element_column_list(j).name() != pub_name) continue;
      auto it = bindings.find(scan.element_column_list(j).column_id());
      if (it != bindings.end()) {
        bindings.emplace(public_id, it->second);
      }
      break;
    }
  }
}

void InjectArrayScanInternalColumns(const ::googlesql::ResolvedArrayScan& scan,
                                    ColumnBindings& bindings) {
  absl::flat_hash_set<int> known_ids;
  known_ids.reserve(scan.element_column_list_size() + 2);
  for (int i = 0; i < scan.element_column_list_size(); ++i) {
    known_ids.insert(scan.element_column_list(i).column_id());
  }
  if (scan.array_offset_column() != nullptr) {
    known_ids.insert(scan.array_offset_column()->column().column_id());
  }
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    if (known_ids.contains(col.column_id())) continue;
    if (bindings.count(col.column_id()) != 0) continue;
    const absl::string_view name = col.name();
    if (name.empty() || name[0] != '$') continue;
    if (name == "$side_effects") {
      const ::googlesql::Type* type = col.type();
      if (type != nullptr && type->kind() == ::googlesql::TYPE_BYTES) {
        bindings.emplace(col.column_id(), Value::NullBytes());
      } else {
        bindings.emplace(col.column_id(), Value::Null(type));
      }
      continue;
    }
    const ::googlesql::Type* type = col.type();
    if (type != nullptr && type->kind() == ::googlesql::TYPE_BOOL) {
      bindings.emplace(col.column_id(), Value::Bool(false));
    } else {
      bindings.emplace(col.column_id(), Value::Null(type));
    }
  }
}

absl::StatusOr<std::vector<ColumnBindings>> EvaluateArrayScan(
    const ::googlesql::ResolvedArrayScan& scan, const EvalContext& parent_ctx) {
  // -------- Correlated shapes without outer bindings.
  //
  // `MaterializeArrayScan` binds each outer row via `MakeOuterRowFrame`
  // before calling here. A direct call with a non-`SingleRowScan`
  // `input_scan` / `join_expr` but no `parent_ctx.columns` is a
  // caller bug — surface `kNotImplemented` so the gateway envelope
  // stays consistent.
  if (scan.join_expr() != nullptr && parent_ctx.columns == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: ResolvedArrayScan.join_expr must be lowered by "
        "scan_eval MaterializeArrayScan");
  }
  if (scan.input_scan() != nullptr &&
      scan.input_scan()->node_kind() != ::googlesql::RESOLVED_SINGLE_ROW_SCAN &&
      parent_ctx.columns == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: correlated ResolvedArrayScan requires outer row bindings");
  }

  // -------- Shape-shape invariants enforced by the analyzer.
  const int n_arrays = scan.array_expr_list_size();
  if (n_arrays <= 0) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedArrayScan has empty array_expr_list");
  }
  if (scan.element_column_list_size() != n_arrays) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ResolvedArrayScan.element_column_list size (",
                     scan.element_column_list_size(),
                     ") does not match array_expr_list size (",
                     n_arrays,
                     ")"));
  }
  if (n_arrays > 1 && scan.array_offset_column() != nullptr) {
    // The analyzer rejects `WITH OFFSET` alongside multi-array
    // UNNEST, but mirror the contract here so a malformed AST
    // surfaces a structured error instead of an out-of-bounds
    // access.
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: WITH OFFSET is not allowed with multi-array UNNEST");
  }

  // -------- Evaluate each array expression once.
  std::vector<Value> arrays;
  arrays.reserve(n_arrays);
  for (int i = 0; i < n_arrays; ++i) {
    auto v = EvaluateArrayExpr(scan.array_expr_list(i), parent_ctx);
    if (!v.ok()) return v.status();
    arrays.push_back(*std::move(v));
  }

  // -------- Compute the row count per mode / NULL-array contract.
  //
  // A NULL array (not just an empty array) in BigQuery semantics
  // produces zero rows in the inner case, and one all-NULL row in
  // the outer case -- the same contract as an empty array. The
  // documented BigQuery behavior is "NULL array is treated as an
  // empty array" for UNNEST.
  //
  // For the single-array path the row count is `num_elements()`
  // (or 0 if NULL); the offset column gets `0..N-1`.
  //
  // For the multi-array zip path the row count depends on
  // `array_zip_mode`:
  //   * `PAD` -> `max(|arr_i|)`; missing positions become NULL.
  //   * `TRUNCATE` -> `min(|arr_i|)`; extras are dropped.
  //   * `STRICT` -> all arrays must have the same length, else
  //     INVALID_ARGUMENT.
  ZipMode zip_mode = ZipMode::kPad;
  if (n_arrays > 1) {
    auto m = ResolveZipMode(scan.array_zip_mode());
    if (!m.ok()) return m.status();
    zip_mode = *m;
  }

  // Treat NULL arrays as length-0 (matches BigQuery's "NULL array
  // is empty array" contract for UNNEST). Record an explicit
  // is_null flag per-array so the multi-zip path can short-circuit
  // PAD-of-NULL-vs-empty consistently.
  std::vector<int> sizes;
  std::vector<bool> null_array;
  sizes.reserve(n_arrays);
  null_array.reserve(n_arrays);
  for (const Value& arr : arrays) {
    if (arr.is_null()) {
      sizes.push_back(0);
      null_array.push_back(true);
    } else {
      sizes.push_back(arr.num_elements());
      null_array.push_back(false);
    }
  }

  int row_count = 0;
  if (n_arrays > 1 && zip_mode == ZipMode::kStrict) {
    auto strict_or = ValidateStrictZipSizes(sizes);
    if (!strict_or.ok()) return strict_or.status();
    row_count = *strict_or;
  } else {
    row_count = ComputeZipRowCount(zip_mode, sizes, n_arrays);
  }

  std::vector<ColumnBindings> rows;
  if (row_count == 0 && scan.is_outer()) {
    rows.push_back(BuildOuterUnnestNullRow(scan, n_arrays));
    return rows;
  }

  rows.reserve(row_count);
  for (int idx = 0; idx < row_count; ++idx) {
    ColumnBindings bindings;
    bindings.reserve(n_arrays + 1);
    for (int i = 0; i < n_arrays; ++i) {
      Value element;
      if (idx < sizes[i] && !null_array[i]) {
        element = arrays[i].element(idx);
      } else {
        // PAD reaches here for the array that ran out of elements;
        // TRUNCATE / STRICT capped `row_count` so they cannot
        // reach this branch. NULL arrays in PAD also produce
        // NULL elements per the BQ contract.
        element = NullElement(scan.element_column_list(i).type());
      }
      bindings.emplace(scan.element_column_list(i).column_id(),
                       std::move(element));
    }
    if (scan.array_offset_column() != nullptr) {
      bindings.emplace(scan.array_offset_column()->column().column_id(),
                       Value::Int64(static_cast<int64_t>(idx)));
    }
    AliasUnnestPublicColumnIds(scan, n_arrays, bindings);
    InjectArrayScanInternalColumns(scan, bindings);
    rows.push_back(std::move(bindings));
  }
  return rows;
}

}  // namespace array_struct
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
