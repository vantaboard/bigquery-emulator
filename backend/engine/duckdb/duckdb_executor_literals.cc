#include <cmath>

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

// --- DuckDB SQL literal rendering -----------------------------------------
//
// Mirrors the helpers in `backend/storage/duckdb/duckdb_storage.cc`
// for the same Value -> DuckDB-literal job. The two copies stay in
// lockstep because the storage layer renders literals for INSERT
// statements against a Parquet-backed table and the engine renders
// literals for INSERT statements against a per-query in-memory
// DuckDB table; folding them into a shared helper is on the followup
// plan that consolidates DuckDB plumbing.

std::string QuoteIdent(absl::string_view ident) {
  std::string escaped = absl::StrReplaceAll(ident, {{"\"", "\"\""}});
  return absl::StrCat("\"", escaped, "\"");
}

std::string EscapeStringLiteralInner(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "''"}});
}

std::string RenderBlobLiteral(absl::string_view bytes) {
  static const char* kHex = "0123456789abcdef";
  std::string out;
  out.reserve(bytes.size() * 2 + 4);
  absl::StrAppend(&out, "X'");
  for (unsigned char c : bytes) {
    out += kHex[c >> 4];
    out += kHex[c & 0x0f];
  }
  absl::StrAppend(&out, "'");
  return out;
}

absl::StatusOr<std::string> RenderCellLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column);

// Render the DuckDB literal for a FLOAT64 cell. Special-cases NaN /
// +Inf / -Inf because DuckDB cannot parse them out of a bare numeric
// literal; the typed cast lowers them through the IEEE-754 path
// instead.
std::string RenderFloatLiteral(const storage::Value& cell) {
  if (cell.kind() == storage::Value::Kind::kString) {
    return absl::StrCat("CAST('",
                        EscapeStringLiteralInner(cell.string_value()),
                        "' AS DOUBLE)");
  }
  const double v = cell.float64_value();
  if (std::isnan(v)) return std::string("'NaN'::DOUBLE");
  if (std::isinf(v)) {
    return std::string(v > 0 ? "'Infinity'::DOUBLE" : "'-Infinity'::DOUBLE");
  }
  return absl::StrFormat("%.17g", v);
}

// Render the DuckDB STRUCT literal `{'k1': v1, ...}` for a STRUCT
// cell. Validates that the cell carries the expected struct shape
// before recursing.
absl::StatusOr<std::string> RenderStructLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  // Parquet round-trip via DuckDBStorage::ReadCell materializes STRUCT
  // columns as VARCHAR; pass the literal through when it already looks
  // like `{'field': value, ...}`.
  if (cell.kind() == storage::Value::Kind::kString) {
    absl::string_view text = absl::StripAsciiWhitespace(cell.string_value());
    if (!text.empty() && text.front() == '{') {
      return std::string(text);
    }
  }
  if (cell.kind() != storage::Value::Kind::kStruct) {
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine: column '",
                     column.name,
                     "' expects STRUCT but row provided non-struct cell"));
  }
  const auto& fields = cell.struct_value();
  if (fields.size() != column.fields.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine: STRUCT column '",
                     column.name,
                     "' has ",
                     column.fields.size(),
                     " fields but row provided ",
                     fields.size()));
  }
  std::string out = "{";
  for (size_t i = 0; i < fields.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(
        &out, "'", EscapeStringLiteralInner(column.fields[i].name), "': ");
    auto inner_or = RenderCellLiteral(fields[i], column.fields[i]);
    if (!inner_or.ok()) return inner_or.status();
    absl::StrAppend(&out, *inner_or);
  }
  absl::StrAppend(&out, "}");
  return out;
}

absl::StatusOr<std::string> RenderScalarLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  // Like `duckdb_storage.cc::RenderScalarLiteral`, we accept both the
  // natively-typed Value variants (Int64, Float64, Bool) and a
  // String carrying the textual representation -- the gateway
  // lowers every JSON cell to a string regardless of the column's
  // declared type. DuckDBStorage returns natively-typed
  // values because the Parquet file enforces the schema, so the
  // string branch is a fallback for callers that hand-construct
  // Value::String cells in unit tests.
  switch (column.type) {
    case schema::ColumnType::kBool:
      if (cell.kind() == storage::Value::Kind::kString) {
        return absl::StrCat("CAST('",
                            EscapeStringLiteralInner(cell.string_value()),
                            "' AS BOOLEAN)");
      }
      return std::string(cell.bool_value() ? "TRUE" : "FALSE");
    case schema::ColumnType::kInt64:
      if (cell.kind() == storage::Value::Kind::kString) {
        return absl::StrCat("CAST('",
                            EscapeStringLiteralInner(cell.string_value()),
                            "' AS BIGINT)");
      }
      return absl::StrCat(cell.int64_value());
    case schema::ColumnType::kFloat64:
      return RenderFloatLiteral(cell);
    case schema::ColumnType::kString:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
      return absl::StrCat(
          "'", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kBytes:
      return RenderBlobLiteral(cell.string_value());
    case schema::ColumnType::kDate:
      return absl::StrCat(
          "DATE '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTime:
      return absl::StrCat(
          "TIME '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kDatetime:
      return absl::StrCat(
          "TIMESTAMP '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTimestamp:
      return absl::StrCat(
          "TIMESTAMPTZ '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
      // Use the storage type so BIGNUMERIC stays VARCHAR: DuckDB's max
      // DECIMAL precision is 38 (DECIMAL(38, 38) cannot even hold 1.0),
      // so casting an extreme-magnitude BIGNUMERIC to DECIMAL overflows.
      // NUMERIC's storage type is still DECIMAL(38, 9). The materialized
      // table column type (RenderColumnList) uses the same storage type
      // so the literal and the column agree.
      return absl::StrCat("CAST('",
                          EscapeStringLiteralInner(cell.string_value()),
                          "' AS ",
                          schema::ToDuckDBStorageType(column.type),
                          ")");
    case schema::ColumnType::kStruct:
      return RenderStructLiteral(cell, column);
    case schema::ColumnType::kArray:
    case schema::ColumnType::kUnknown:
      return absl::StrCat(
          "'", EscapeStringLiteralInner(cell.string_value()), "'");
  }
  return absl::InternalError("RenderScalarLiteral: unreachable");
}

absl::StatusOr<std::string> RenderCellLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  if (cell.is_null()) return std::string("NULL");
  if (column.mode == schema::ColumnMode::kRepeated) {
    // REPEATED cells lower onto DuckDB LIST literals (`[v1, v2, ...]`).
    // The element renderer goes through a synthetic non-repeated
    // column so it does not re-enter the array branch.
    schema::ColumnSchema element = column;
    element.mode = schema::ColumnMode::kNullable;
    std::string out = "[";
    const auto& elems = cell.array_value();
    for (size_t i = 0; i < elems.size(); ++i) {
      if (i > 0) absl::StrAppend(&out, ", ");
      auto inner_or = RenderCellLiteral(elems[i], element);
      if (!inner_or.ok()) return inner_or.status();
      absl::StrAppend(&out, *inner_or);
    }
    absl::StrAppend(&out, "]");
    return out;
  }
  return RenderScalarLiteral(cell, column);
}

std::string RenderColumnList(const schema::TableSchema& schema) {
  std::string out = "(";
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    // Storage type so the materialized query table matches the
    // Parquet snapshot the storage layer wrote (BIGNUMERIC -> VARCHAR,
    // which preserves the full magnitude; NUMERIC stays DECIMAL(38, 9)).
    // Attaching the table with the query type DECIMAL(38, 38) for
    // BIGNUMERIC overflows on read-back of extreme values.
    absl::StrAppend(&out,
                    QuoteIdent(schema.columns[i].name),
                    " ",
                    schema::ColumnSchemaToDuckDBStorageType(schema.columns[i]));
  }
  absl::StrAppend(&out, ")");
  return out;
}

std::string RenderColumnIdentList(const schema::TableSchema& schema) {
  std::string out;
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(&out, QuoteIdent(schema.columns[i].name));
  }
  return out;
}

void RecordPhase(PhaseRecorder* recorder,
                 absl::string_view name,
                 absl::Duration elapsed) {
  if (recorder != nullptr) {
    recorder->Record(name, absl::ToInt64Microseconds(elapsed));
  }
}

// Runs `sql` on `conn`; returns OK or INTERNAL with the DuckDB
// error message attached. Use this for INSERT / CREATE statements
// where the result rowset is uninteresting.
absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql) {
  ::duckdb_result result;
  const std::string sql_str(sql);
  const auto state = ::duckdb_query(conn, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(
        absl::StrCat("DuckDBEngine: query failed: ", sql_str, ": ", detail));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

absl::StatusOr<std::string> RenderSemanticParameterLiteral(
    const semantic::Value& v) {
  if (v.is_null()) return std::string("NULL");
  auto storage_or = semantic::ToStorageValue(v);
  if (!storage_or.ok()) return storage_or.status();
  schema::ColumnSchema col;
  col.name = "p";
  col.mode = schema::ColumnMode::kNullable;
  switch (v.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      col.type = schema::ColumnType::kBool;
      break;
    case ::googlesql::TYPE_INT64:
      col.type = schema::ColumnType::kInt64;
      break;
    case ::googlesql::TYPE_DOUBLE:
      col.type = schema::ColumnType::kFloat64;
      break;
    case ::googlesql::TYPE_STRING:
    case ::googlesql::TYPE_JSON:
    case ::googlesql::TYPE_GEOGRAPHY:
      col.type = schema::ColumnType::kString;
      break;
    case ::googlesql::TYPE_BYTES:
      col.type = schema::ColumnType::kBytes;
      break;
    case ::googlesql::TYPE_DATE:
      col.type = schema::ColumnType::kDate;
      break;
    case ::googlesql::TYPE_TIMESTAMP:
      col.type = schema::ColumnType::kTimestamp;
      break;
    case ::googlesql::TYPE_NUMERIC:
      col.type = schema::ColumnType::kNumeric;
      break;
    case ::googlesql::TYPE_BIGNUMERIC:
      col.type = schema::ColumnType::kBignumeric;
      break;
    case ::googlesql::TYPE_ARRAY:
      col.mode = schema::ColumnMode::kRepeated;
      col.type = schema::ColumnType::kString;
      return RenderCellLiteral(*storage_or, col);
    default:
      col.type = schema::ColumnType::kString;
      break;
  }
  return RenderScalarLiteral(*storage_or, col);
}

absl::StatusOr<std::string> SubstituteDuckdbParameters(
    std::string sql,
    const std::vector<transpiler::Transpiler::ParameterRef>& order,
    absl::Span<const QueryParameter> parameters) {
  if (order.empty()) return sql;
  std::vector<std::string> literals(order.size());
  for (size_t i = 0; i < order.size(); ++i) {
    const transpiler::Transpiler::ParameterRef& ref = order[i];
    const QueryParameter* param = nullptr;
    if (!ref.name.empty()) {
      for (const QueryParameter& p : parameters) {
        if (absl::EqualsIgnoreCase(p.name, ref.name)) {
          param = &p;
          break;
        }
      }
    } else {
      int seen = 0;
      for (const QueryParameter& p : parameters) {
        if (!p.name.empty()) continue;
        if (++seen == ref.position) {
          param = &p;
          break;
        }
      }
    }
    if (param == nullptr) {
      return absl::InvalidArgumentError(absl::StrCat(
          "DuckDbExecutor: missing query parameter for DuckDB placeholder $",
          i + 1));
    }
    auto value = semantic::ParseParameterValue(
        param->value_json, param->type_kind, param->type_json);
    if (!value.ok()) return value.status();
    auto literal = RenderSemanticParameterLiteral(*value);
    if (!literal.ok()) return literal.status();
    literals[i] = *std::move(literal);
  }
  for (int slot = static_cast<int>(order.size()); slot >= 1; --slot) {
    const std::string placeholder = absl::StrCat("$", slot);
    sql = absl::StrReplaceAll(
        sql, {{placeholder, literals[static_cast<size_t>(slot - 1)]}});
  }
  return sql;
}

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
