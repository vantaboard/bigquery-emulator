#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_INTERNAL_H_

// Shared helpers for DuckDB transpiler emit translation units.

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace internal {

// Double-quote escape a DuckDB identifier. DuckDB doubles embedded
// `"` characters; we do the same so column / table names with quotes
// or hyphens round-trip safely through the emitted SQL.
inline std::string QuoteIdent(absl::string_view name) {
  return absl::StrCat("\"", absl::StrReplaceAll(name, {{"\"", "\"\""}}), "\"");
}

// Single-quote escape a DuckDB string literal. DuckDB doubles embedded
// `'` characters; we do the same so BQ string literals with embedded
// apostrophes round-trip safely. Used for both ResolvedLiteral
// strings and for STRUCT field-name keys in `{'k': v}` literals.
inline std::string QuoteString(absl::string_view text) {
  return absl::StrCat("'", absl::StrReplaceAll(text, {{"'", "''"}}), "'");
}

inline std::string ResolveFunctionName(const ::googlesql::Function* fn) {
  if (fn == nullptr) return "";
  return absl::AsciiStrToLower(fn->FullName(/*include_group=*/false));
}

inline std::string WrapArrayAggRespectNulls(absl::string_view body,
                                            absl::string_view arg) {
  return absl::StrCat(
      "if(count(",
      arg,
      ") < count(*), error('ARRAY_AGG: input value must be not null'), ",
      body,
      ")");
}

inline constexpr const char kBqInputRnCol[] = "__bq_input_rn";

inline std::string JoinColumnIdAlias(int column_id) {
  return QuoteIdent(absl::StrCat("__bq_j_", column_id));
}
inline constexpr const char kBqPctCoalesceCol[] = "__bq_pct_coalesce";
inline constexpr char kBqPctNullSentinel[] = "'!__BQ_NULL__!'";
inline constexpr const char kBqUnionOrdCol[] = "__bq_union_ord";

// Suffix for ORDER BY direction + NULL ordering.
inline std::string OrderByItemSuffix(
    const ::googlesql::ResolvedOrderByItem* item,
    bool bigquery_null_defaults = false) {
  const char* dir = item->is_descending() ? "DESC" : "ASC";
  const char* nulls = "";
  switch (item->null_order()) {
    case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
      nulls = " NULLS FIRST";
      break;
    case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
      nulls = " NULLS LAST";
      break;
    case ::googlesql::ResolvedOrderByItem::ORDER_UNSPECIFIED:
    default:
      if (bigquery_null_defaults) {
        nulls = item->is_descending() ? " NULLS LAST" : " NULLS FIRST";
      }
      break;
  }
  return absl::StrCat(" ", dir, nulls);
}

inline std::optional<std::string> TryLiteralString(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr || expr->node_kind() != ::googlesql::RESOLVED_LITERAL) {
    return std::nullopt;
  }
  const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
  if (lit == nullptr) return std::nullopt;
  const ::googlesql::Value& v = lit->value();
  if (v.is_null() || v.type_kind() != ::googlesql::TYPE_STRING) {
    return std::nullopt;
  }
  return v.string_value();
}

// DuckDB BLOB literals use per-byte `\xHH` escapes inside single quotes
// (`'\x61\x62\x63'::BLOB`). A bare hex digit run (`'616263'::BLOB`) is
// six ASCII bytes, not three decoded bytes.
inline std::string EmitBlobLiteral(absl::string_view bytes) {
  static const char kHex[] = "0123456789abcdef";
  std::string escaped;
  escaped.reserve(bytes.size() * 4);
  for (unsigned char c : bytes) {
    escaped.push_back('\\');
    escaped.push_back('x');
    escaped.push_back(kHex[c >> 4]);
    escaped.push_back(kHex[c & 0xf]);
  }
  return absl::StrCat("'", escaped, "'::BLOB");
}

// Synthesize a stable DuckDB-side field name for a BigQuery STRUCT
// field that was declared without one (e.g. `STRUCT(1, 'a')`). DuckDB
// requires every struct field to be named, so we pick a positional
// scheme (`_0`, `_1`, ...) and use the *same* convention everywhere
// the transpiler emits SQL that mentions the field:
//
//   * `EmitValueLiteral` (folded constant struct) and `EmitMakeStruct`
//     emit the synthesized name as the key in `{'_<i>': <value>}`.
//   * `EmitGetStructField` resolves a positional access to the same
//     synthesized name on the dotted form (`<expr>."_<i>"`).
//
// Stable, monotonic positional names match BigQuery's positional
// field-order semantics one-for-one and keep the conformance harness
// from having to round-trip the BQ-side name (which is empty
// regardless of how the user spelled the access).
inline std::string SynthesizeAnonymousFieldName(int idx) {
  return absl::StrCat("_", idx);
}

// Pick the DuckDB field name to use for STRUCT field `idx` of type
// `st`. Returns the analyzer's name when set, or the synthesized
// positional name (`_<idx>`) for an anonymous field. Centralizing the
// choice keeps `EmitValueLiteral`, `EmitMakeStruct`, and
// `EmitGetStructField` aligned -- a drift between the literal/maker
// emit and the field-access emit would silently produce DuckDB
// "field does not exist" runtime errors.
inline std::string ResolveStructFieldName(const ::googlesql::StructType& st,
                                          int idx) {
  const ::googlesql::StructField& f = st.field(idx);
  if (f.name.empty()) return SynthesizeAnonymousFieldName(idx);
  return f.name;
}

// Lower a GoogleSQL `Value` into a DuckDB SQL literal expression.
//
// Scalars route through `Value::GetSQLLiteral(PRODUCT_EXTERNAL)`
// because that path already matches DuckDB syntax for INT / FLOAT /
// BOOL / DATE / NUMERIC / DATETIME etc. Strings, arrays, and structs
// each need a bespoke shape:
//
// * Strings: DuckDB reads double-quoted text as an *identifier*, so we
//   emit the single-quoted form (`'hi'`).
// * Arrays: DuckDB's array literal is `[e1, e2, ...]`, same shape as
//   GoogleSQL's `kSQLLiteral` output, but we recurse so nested
//   STRINGs / STRUCTs get the DuckDB-flavored quoting above instead
//   of GoogleSQL's `"..."` and `(...)` shapes.
// * Structs: DuckDB struct literals are `{'k1': v1, 'k2': v2, ...}`
//   keyed by name. BQ STRUCT field order is positional (the type
//   carries the names), so we walk the StructType for the keys in
//   parallel with the value list. Anonymous BigQuery fields (empty
//   name) get a synthesized positional name (`_0`, `_1`, ...) via
//   `ResolveStructFieldName` so the literal emits as
//   `{'_0': 1, '_1': 'a'}`; `EmitGetStructField` uses the same
//   convention on the access side.
//
inline std::string FormatDateLiteral(int32_t days_since_epoch) {
  int32_t z = days_since_epoch + 719468;
  int32_t era = (z >= 0 ? z : z - 146096) / 146097;
  unsigned doe = static_cast<unsigned>(z - era * 146097);
  unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int y = static_cast<int>(yoe) + era * 400;
  unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  unsigned mp = (5 * doy + 2) / 153;
  unsigned d = doy - (153 * mp + 2) / 5 + 1;
  unsigned m = mp < 10 ? mp + 3 : mp - 9;
  if (m <= 2) ++y;
  return absl::StrFormat("%04d-%02u-%02u", y, m, d);
}

// Returns the empty string when any element / field cannot be lowered;
// callers propagate that up so the engine fallback fires per the
// per-shape disposition in SHAPE_TRACKER.md.
inline std::string EmitValueLiteral(const ::googlesql::Value& v) {
  if (v.is_null()) return "NULL";
  const ::googlesql::Type* type = v.type();
  if (type == nullptr) return "";
  switch (type->kind()) {
    case ::googlesql::TYPE_STRING:
      return QuoteString(v.string_value());
    case ::googlesql::TYPE_BYTES:
      return EmitBlobLiteral(v.bytes_value());
    case ::googlesql::TYPE_ARRAY: {
      std::vector<std::string> elems;
      elems.reserve(v.num_elements());
      for (int i = 0; i < v.num_elements(); ++i) {
        std::string e = EmitValueLiteral(v.element(i));
        if (e.empty()) return "";
        elems.push_back(std::move(e));
      }
      return absl::StrCat("[", absl::StrJoin(elems, ", "), "]");
    }
    case ::googlesql::TYPE_STRUCT: {
      const ::googlesql::StructType* st = type->AsStruct();
      if (st == nullptr || st->num_fields() != v.num_fields()) return "";
      std::vector<std::string> kvs;
      kvs.reserve(v.num_fields());
      for (int i = 0; i < v.num_fields(); ++i) {
        std::string fv = EmitValueLiteral(v.field(i));
        if (fv.empty()) return "";
        kvs.push_back(absl::StrCat(
            QuoteString(ResolveStructFieldName(*st, i)), ": ", fv));
      }
      return absl::StrCat("{", absl::StrJoin(kvs, ", "), "}");
    }
    case ::googlesql::TYPE_TIMESTAMP: {
      const absl::TimeZone utc = absl::UTCTimeZone();
      const absl::Time t = v.ToTime();
      const int64_t micros = absl::ToUnixMicros(t);
      std::string formatted;
      if (micros % 1000000 == 0) {
        formatted = absl::FormatTime("%Y-%m-%d %H:%M:%S+00", t, utc);
      } else {
        formatted = absl::StrCat(
            absl::FormatTime("%Y-%m-%d %H:%M:%E6S", t, utc), "+00");
      }
      return absl::StrCat("CAST(", QuoteString(formatted), " AS TIMESTAMPTZ)");
    }
    case ::googlesql::TYPE_DATETIME: {
      std::string out = v.datetime_value().DebugString();
      const size_t sep = out.find(' ');
      if (sep != std::string::npos) {
        out[sep] = 'T';
      }
      return absl::StrCat("CAST(", QuoteString(out), " AS TIMESTAMP)");
    }
    case ::googlesql::TYPE_DATE:
      return absl::StrCat(
          "CAST(", QuoteString(FormatDateLiteral(v.date_value())), " AS DATE)");
    case ::googlesql::TYPE_TIME:
      return absl::StrCat(
          "CAST(", QuoteString(v.time_value().DebugString()), " AS TIME)");
    default:
      return v.GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL);
  }
}

// BigQuery STRUCT-to-STRUCT casts match fields by positional index;
// DuckDB `CAST(... AS STRUCT(...))` requires overlapping field names.
// Remap each target field from the source struct via dotted access and
// emit a DuckDB struct literal keyed by the target names.
inline std::string EmitStructPositionalCastRemap(
    absl::string_view inner,
    const ::googlesql::StructType& source_st,
    const ::googlesql::StructType& target_st) {
  if (source_st.num_fields() != target_st.num_fields()) return "";
  std::vector<std::string> kvs;
  kvs.reserve(target_st.num_fields());
  const std::string wrapped = absl::StrCat("(", inner, ")");
  for (int i = 0; i < target_st.num_fields(); ++i) {
    const std::string source_field = ResolveStructFieldName(source_st, i);
    const std::string target_field = ResolveStructFieldName(target_st, i);
    kvs.push_back(absl::StrCat(QuoteString(target_field),
                               ": ",
                               wrapped,
                               ".",
                               QuoteIdent(source_field)));
  }
  return absl::StrCat("{", absl::StrJoin(kvs, ", "), "}");
}

// Whitelist of GoogleSQL `TypeKind`s the `EmitCast` path will lower.
// `DuckDBSqlTypeName` itself is intentionally total (it falls through
// to `VARCHAR` for unsupported kinds so column-def emit always
// compiles), but for `CAST(<expr> AS T)` we'd rather take the engine
// fallback than silently retype `GEOGRAPHY` / proto / enum / range /
// graph values to a DuckDB string -- the runtime semantics would not
// match the BigQuery cast contract.
inline bool IsCastTargetSupported(::googlesql::TypeKind kind) {
  switch (kind) {
    case ::googlesql::TYPE_BOOL:
    case ::googlesql::TYPE_INT32:
    case ::googlesql::TYPE_INT64:
    case ::googlesql::TYPE_UINT32:
    case ::googlesql::TYPE_UINT64:
    case ::googlesql::TYPE_FLOAT:
    case ::googlesql::TYPE_DOUBLE:
    case ::googlesql::TYPE_STRING:
    case ::googlesql::TYPE_BYTES:
    case ::googlesql::TYPE_DATE:
    case ::googlesql::TYPE_TIME:
    case ::googlesql::TYPE_DATETIME:
    case ::googlesql::TYPE_TIMESTAMP:
    case ::googlesql::TYPE_NUMERIC:
    case ::googlesql::TYPE_BIGNUMERIC:
    case ::googlesql::TYPE_JSON:
    case ::googlesql::TYPE_INTERVAL:
    case ::googlesql::TYPE_UUID:
    case ::googlesql::TYPE_ARRAY:
    case ::googlesql::TYPE_STRUCT:
      return true;
    default:
      return false;
  }
}

inline std::string OrderItemLeadingColumn(const std::string& item) {
  if (item.empty() || item[0] != '"') return "";
  const size_t end = item.find('"', 1);
  if (end == std::string::npos) return "";
  return item.substr(0, end + 1);
}

inline bool OutputListContainsColumn(
    absl::string_view quoted_col, const ::googlesql::ResolvedQueryStmt* node) {
  if (node == nullptr) return false;
  for (int i = 0; i < node->output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* out = node->output_column_list(i);
    if (out == nullptr) continue;
    if (quoted_col == QuoteIdent(out->column().name())) return true;
  }
  return false;
}

inline std::vector<std::string> ExtraOrderColumnsForWrap(
    const std::vector<std::string>& order_items,
    const ::googlesql::ResolvedQueryStmt* node) {
  std::vector<std::string> extra;
  for (const std::string& item : order_items) {
    const std::string col = OrderItemLeadingColumn(item);
    if (col.empty() || col == QuoteIdent(kBqInputRnCol)) continue;
    if (OutputListContainsColumn(col, node)) continue;
    if (std::find(extra.begin(), extra.end(), col) == extra.end()) {
      extra.push_back(col);
    }
  }
  return extra;
}

inline std::vector<std::string> FilterOutputOrderItems(
    const std::vector<std::string>& items,
    const ::googlesql::ResolvedQueryStmt* node) {
  if (node == nullptr) return {};
  std::vector<std::string> filtered;
  filtered.reserve(items.size());
  for (const std::string& item : items) {
    if (item.empty() || item[0] != '"') continue;
    const size_t end = item.find('"', 1);
    if (end == std::string::npos) continue;
    const std::string quoted_col = item.substr(0, end + 1);
    for (int i = 0; i < node->output_column_list_size(); ++i) {
      const ::googlesql::ResolvedOutputColumn* out =
          node->output_column_list(i);
      if (out == nullptr) continue;
      if (quoted_col == QuoteIdent(out->column().name())) {
        filtered.push_back(item);
        break;
      }
    }
  }
  return filtered;
}

// `CASE val WHEN w1 THEN t1 ... ELSE e END` for analyzer `$case_with_value`.
inline std::string EmitCaseWithValue(const std::vector<std::string>& args) {
  if (args.size() < 2 || (args.size() % 2) != 0) return "";
  std::string sql = absl::StrCat("CASE ", args[0], " ");
  for (size_t i = 1; i + 1 < args.size(); i += 2) {
    absl::StrAppend(&sql, "WHEN ", args[i], " THEN ", args[i + 1], " ");
  }
  absl::StrAppend(&sql, "ELSE ", args.back(), " END");
  return sql;
}

// `CASE WHEN c1 THEN t1 ... ELSE e END` for analyzer `$case_no_value`.
inline std::string EmitCaseNoValue(const std::vector<std::string>& args) {
  if (args.size() < 1 || (args.size() % 2) == 0) return "";
  std::string sql = "CASE ";
  for (size_t i = 0; i + 1 < args.size(); i += 2) {
    absl::StrAppend(&sql, "WHEN ", args[i], " THEN ", args[i + 1], " ");
  }
  absl::StrAppend(&sql, "ELSE ", args.back(), " END");
  return sql;
}

inline bool SupportsOrderedAggregateModifiers(absl::string_view name) {
  return name == "array_agg" || name == "string_agg" ||
         name == "array_concat_agg";
}

inline std::string AppendArrayAggNullFilter(absl::string_view body,
                                            absl::string_view arg,
                                            bool ignore_nulls) {
  if (!ignore_nulls) return std::string(body);
  return absl::StrCat(body, " FILTER (WHERE ", arg, " IS NOT NULL)");
}

bool ScanTreeContainsAnalytic(const ::googlesql::ResolvedScan* scan);
bool AnalyticOrderNeedsInputRn(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group);
bool AnalyticGroupNeedsInputRnForEmptyOrder(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group);
bool AnalyticGroupHasRangeFrame(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group);
bool AggregateScanNeedsInputRn(const ::googlesql::ResolvedAggregateScan* node);
}  // namespace internal
}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_INTERNAL_H_
