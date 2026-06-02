#include "backend/engine/semantic/array_struct/array_scan.h"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

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

// Resolve the `array_zip_mode` enum literal the analyzer attaches
// to a multi-array UNNEST. The analyzer guarantees the expr is a
// `ResolvedLiteral` of opaque enum kind `ARRAY_ZIP_MODE` with one
// of three values: `PAD`, `TRUNCATE`, `STRICT`. We extract the
// enum NAME (not value index) so the dispatch below stays
// human-readable.
enum class ZipMode {
  kPad,
  kTruncate,
  kStrict,
};

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

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> EvaluateArrayScan(
    const ::googlesql::ResolvedArrayScan& scan, const EvalContext& parent_ctx) {
  // -------- Out-of-scope shapes (deferred to follow-up subagents).
  //
  // A non-null, non-`SingleRowScan` `input_scan` is the correlated
  // shape (`FROM t, UNNEST(t.arr)`); a non-null `join_expr` is the
  // `LEFT JOIN UNNEST(arr) ON ...` shape. Both need a row source
  // adapter (`RowSource` -> per-row `ColumnBindings`) which is
  // Family 4 of `array-struct-semantic-path.plan.md`. Until that
  // lands, surface a structured `kNotImplemented` so the gateway's
  // envelope is consistent with other "planned but not landed"
  // routes.
  if (scan.input_scan() != nullptr &&
      scan.input_scan()->node_kind() != ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: correlated ResolvedArrayScan (FROM t, UNNEST(t.arr)) is "
        "deferred to a follow-up subagent of "
        "array-struct-semantic-path.plan.md (Family 4)");
  }
  if (scan.join_expr() != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: ResolvedArrayScan.join_expr (LEFT JOIN UNNEST ... ON ...) "
        "is deferred to a follow-up subagent of "
        "array-struct-semantic-path.plan.md (Family 4)");
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
  if (n_arrays == 1) {
    row_count = sizes[0];
  } else {
    switch (zip_mode) {
      case ZipMode::kPad: {
        for (int s : sizes)
          row_count = std::max(row_count, s);
        break;
      }
      case ZipMode::kTruncate: {
        row_count = sizes[0];
        for (int s : sizes)
          row_count = std::min(row_count, s);
        break;
      }
      case ZipMode::kStrict: {
        row_count = sizes[0];
        for (int i = 1; i < n_arrays; ++i) {
          if (sizes[i] != row_count) {
            return MakeSemanticError(
                SemanticErrorReason::kInvalidArgument,
                absl::StrCat(
                    "semantic: UNNEST with mode STRICT requires arrays of "
                    "equal length; got array #0 length ",
                    sizes[0],
                    " and array #",
                    i,
                    " length ",
                    sizes[i]));
          }
        }
        break;
      }
    }
  }

  // -------- Outer UNNEST emits one all-NULL row on empty arrays.
  //
  // BigQuery semantics: `LEFT JOIN UNNEST(arr) ... ` (or
  // `is_outer=true` set via `WITH OFFSET` on an empty side) emits
  // one row with NULL for every element column and NULL for the
  // offset column. DuckDB drops the row in its lateral unnest;
  // promoting this shape to the semantic executor and emitting
  // the NULL row here is what restores the contract.
  std::vector<ColumnBindings> rows;
  if (row_count == 0 && scan.is_outer()) {
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
    rows.push_back(std::move(bindings));
    return rows;
  }

  // -------- Materialize the rows.
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
    rows.push_back(std::move(bindings));
  }
  return rows;
}

}  // namespace array_struct
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
