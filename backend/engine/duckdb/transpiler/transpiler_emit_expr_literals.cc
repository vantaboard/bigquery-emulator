#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "backend/engine/duckdb/transpiler/types.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

// Expressions ---------------------------------------------------------------

std::string Transpiler::EmitLiteral(const ::googlesql::ResolvedLiteral* node) {
  // Delegates to the file-private `EmitValueLiteral` helper. Scalar
  // kinds keep going through `Value::GetSQLLiteral(PRODUCT_EXTERNAL)`
  // because that path already matches DuckDB syntax for INT / FLOAT /
  // BOOL / DATE / NUMERIC / DATETIME etc. The helper carves out the
  // three cases DuckDB spells differently from GoogleSQL:
  //
  //   * STRING: DuckDB reads double-quoted text as an *identifier*,
  //     so we emit the single-quoted form (`'hi'`).
  //   * ARRAY: GoogleSQL's `[1, 2]` shape happens to match DuckDB's,
  //     but we recurse so nested STRINGs / STRUCTs get the
  //     DuckDB-flavored quoting rather than `"..."` / `(...)`.
  //   * STRUCT: DuckDB struct literals are `{'k': v, ...}` keyed by
  //     field name. Anonymous fields (empty name) have no DuckDB
  //     analog; we surface UNIMPLEMENTED via the empty-string
  //     contract.
  if (node == nullptr) return "";
  return internal::EmitValueLiteral(node->value());
}

std::string Transpiler::EmitParameter(
    const ::googlesql::ResolvedParameter* node) {
  // GoogleSQL named parameters (`@CustomerId`) and positional
  // parameters (`?`) both lower to DuckDB's `$N` bind shape. We
  // accumulate one `ParameterRef` per unique slot in
  // `parameter_order_` so the engine can copy values into DuckDB's
  // bind buffer in slot order; multiple references to the same
  // named parameter share a slot, and positional parameters get
  // their own slot every time the analyzer hands us one (the
  // analyzer's 1-based `position()` carries the bind-time
  // identity).
  //
  // Untyped parameters (the analyzer's stand-in for "the engine
  // will fill in the type later") are out of scope for a transpiled
  // query: DuckDB infers types from the bound value, but we have
  // no type to attach to the placeholder. Return the empty string
  // so the engine surfaces UNIMPLEMENTED.
  if (node == nullptr) return "";
  if (node->is_untyped()) return "";
  if (!node->name().empty()) {
    int slot = LookupOrAssignNamedParameter(node->name());
    return absl::StrCat("$", slot);
  }
  if (node->position() > 0) {
    int slot = AssignPositionalParameter(node->position());
    return absl::StrCat("$", slot);
  }
  return "";
}

int Transpiler::LookupOrAssignNamedParameter(absl::string_view name) {
  std::string key(name);
  auto it = name_to_slot_.find(key);
  if (it != name_to_slot_.end()) return it->second;
  int slot = 0;
  slot = static_cast<int>(parameter_order_.size()) + 1;
  parameter_order_.push_back({key, /*position=*/0});
  name_to_slot_.emplace(std::move(key), slot);
  return slot;
}

int Transpiler::AssignPositionalParameter(int analyzer_position) {
  int slot = 0;
  slot = static_cast<int>(parameter_order_.size()) + 1;
  parameter_order_.push_back({/*name=*/std::string(),
                              /*position=*/analyzer_position});
  return slot;
}

std::string Transpiler::EmitColumnRef(
    const ::googlesql::ResolvedColumnRef* node) {
  // We reference the column by the `ResolvedColumn::name()` it
  // carries. `EmitTableScan` aliases each underlying storage column
  // onto exactly this name, so the emitted SQL stays consistent
  // whether the column flows straight from a table scan or through
  // a wrapping FilterScan / ProjectScan.
  if (node == nullptr) return "";
  if (join_output_uses_id_aliases_) {
    return internal::JoinColumnIdAlias(node->column().column_id());
  }
  return internal::QuoteIdent(node->column().name());
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
