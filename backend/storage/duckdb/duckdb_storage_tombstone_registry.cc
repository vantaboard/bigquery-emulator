#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_governance.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;

namespace {

constexpr absl::string_view kViewsTable = "main.__bqemu_views";
constexpr absl::string_view kRoutinesTable = "main.__bqemu_routines";
constexpr absl::string_view kRowAccessPoliciesTable =
    "main.__bqemu_row_access_policies";
constexpr absl::string_view kColumnGovernanceTable =
    "main.__bqemu_column_governance";

std::string RoutineKindToString(RoutineKind kind) {
  switch (kind) {
    case RoutineKind::kScalarFunction:
      return "scalar_function";
    case RoutineKind::kAggregateFunction:
      return "aggregate_function";
    case RoutineKind::kTableValuedFunction:
      return "table_valued_function";
    case RoutineKind::kProcedure:
      return "procedure";
  }
  return "scalar_function";
}

absl::StatusOr<RoutineKind> RoutineKindFromString(absl::string_view s) {
  const std::string lower = absl::AsciiStrToLower(s);
  if (lower == "scalar_function") return RoutineKind::kScalarFunction;
  if (lower == "aggregate_function") return RoutineKind::kAggregateFunction;
  if (lower == "table_valued_function") {
    return RoutineKind::kTableValuedFunction;
  }
  if (lower == "procedure") return RoutineKind::kProcedure;
  return absl::InvalidArgumentError(absl::StrCat("unknown routine kind: ", s));
}

std::string RenderQuotedJsonStrings(const std::vector<std::string>& values) {
  std::vector<std::string> quoted;
  quoted.reserve(values.size());
  for (const std::string& v : values) {
    quoted.push_back(absl::StrCat("\"", internal::JsonEscape(v), "\""));
  }
  return absl::StrCat("[", absl::StrJoin(quoted, ","), "]");
}

std::vector<std::string> ParseQuotedJsonStringArray(absl::string_view json) {
  std::vector<std::string> out;
  size_t pos = 0;
  while ((pos = json.find('"', pos)) != absl::string_view::npos) {
    ++pos;
    std::string id;
    for (; pos < json.size(); ++pos) {
      if (json[pos] == '\\' && pos + 1 < json.size()) {
        id.push_back(json[pos + 1]);
        ++pos;
        continue;
      }
      if (json[pos] == '"') break;
      id.push_back(json[pos]);
    }
    if (!id.empty()) {
      out.push_back(std::move(id));
    }
    ++pos;
  }
  return out;
}

absl::StatusOr<std::string> ParseJsonStringField(absl::string_view json,
                                                 absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":\"");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InvalidArgumentError(
        absl::StrCat("json missing string field ", field));
  }
  absl::string_view rest = json.substr(pos + needle.size());
  std::string out;
  for (size_t i = 0; i < rest.size(); ++i) {
    if (rest[i] == '\\' && i + 1 < rest.size()) {
      out.push_back(rest[i + 1]);
      ++i;
      continue;
    }
    if (rest[i] == '"') break;
    out.push_back(rest[i]);
  }
  return out;
}

bool ParseJsonBoolField(absl::string_view json, absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) return false;
  absl::string_view rest = json.substr(pos + needle.size());
  return absl::StartsWith(rest, "true");
}

std::string RenderViewRecordJson(const ViewRecord& rec) {
  return absl::StrCat("{\"view_id\":\"",
                      internal::JsonEscape(rec.id.view_id),
                      "\",\"ddl_sql\":\"",
                      internal::JsonEscape(rec.ddl_sql),
                      "\"}");
}

absl::StatusOr<ViewRecord> ParseViewRecordJson(absl::string_view json,
                                               const DatasetId& ds) {
  auto view_id_or = ParseJsonStringField(json, "view_id");
  if (!view_id_or.ok()) return view_id_or.status();
  auto ddl_or = ParseJsonStringField(json, "ddl_sql");
  if (!ddl_or.ok()) return ddl_or.status();
  ViewRecord rec;
  rec.id = ViewId{ds.project_id, ds.dataset_id, *view_id_or};
  rec.ddl_sql = *ddl_or;
  return rec;
}

std::string RenderRoutineRecordJson(const RoutineRecord& rec) {
  std::string sig = rec.signature_json.empty()
                        ? "null"
                        : absl::StrCat("\"",
                                       internal::JsonEscape(rec.signature_json),
                                       "\"");
  return absl::StrCat("{\"routine_id\":\"",
                      internal::JsonEscape(rec.id.routine_id),
                      "\",\"kind\":\"",
                      RoutineKindToString(rec.kind),
                      "\",\"language\":\"",
                      internal::JsonEscape(rec.language),
                      "\",\"ddl_sql\":\"",
                      internal::JsonEscape(rec.ddl_sql),
                      "\",\"is_temp\":",
                      rec.is_temp ? "true" : "false",
                      ",\"signature_json\":",
                      sig,
                      "}");
}

absl::StatusOr<RoutineRecord> ParseRoutineRecordJson(absl::string_view json,
                                                     const DatasetId& ds) {
  auto routine_id_or = ParseJsonStringField(json, "routine_id");
  if (!routine_id_or.ok()) return routine_id_or.status();
  auto kind_or = ParseJsonStringField(json, "kind");
  if (!kind_or.ok()) return kind_or.status();
  auto language_or = ParseJsonStringField(json, "language");
  if (!language_or.ok()) return language_or.status();
  auto ddl_or = ParseJsonStringField(json, "ddl_sql");
  if (!ddl_or.ok()) return ddl_or.status();
  RoutineRecord rec;
  rec.id = RoutineId{ds.project_id, ds.dataset_id, *routine_id_or};
  auto parsed_kind_or = RoutineKindFromString(*kind_or);
  if (!parsed_kind_or.ok()) return parsed_kind_or.status();
  rec.kind = *parsed_kind_or;
  rec.language = *language_or;
  rec.ddl_sql = *ddl_or;
  rec.is_temp = ParseJsonBoolField(json, "is_temp");
  if (json.find("\"signature_json\":null") == absl::string_view::npos) {
    auto sig_or = ParseJsonStringField(json, "signature_json");
    if (sig_or.ok()) {
      rec.signature_json = *sig_or;
    }
  }
  return rec;
}

std::vector<absl::string_view> SplitTopLevelJsonObjects(
    absl::string_view array_json) {
  std::vector<absl::string_view> out;
  size_t pos = array_json.find('[');
  if (pos == absl::string_view::npos) return out;
  ++pos;
  while (pos < array_json.size()) {
    while (pos < array_json.size() &&
           (array_json[pos] == ' ' || array_json[pos] == ',' ||
            array_json[pos] == '\n')) {
      ++pos;
    }
    if (pos >= array_json.size() || array_json[pos] == ']') break;
    if (array_json[pos] != '{') break;
    size_t depth = 0;
    bool in_string = false;
    const size_t start = pos;
    for (; pos < array_json.size(); ++pos) {
      const char c = array_json[pos];
      if (in_string) {
        if (c == '\\' && pos + 1 < array_json.size()) {
          ++pos;
          continue;
        }
        if (c == '"') in_string = false;
        continue;
      }
      if (c == '"') {
        in_string = true;
        continue;
      }
      if (c == '{') {
        ++depth;
      } else if (c == '}') {
        --depth;
        if (depth == 0) {
          out.push_back(array_json.substr(start, pos - start + 1));
          ++pos;
          break;
        }
      }
    }
  }
  return out;
}

std::string RenderGovernanceSnapshotJson(
    const std::vector<std::pair<TableId, TableGovernance>>& tables) {
  if (tables.empty()) return "[]";
  std::vector<std::string> entries;
  entries.reserve(tables.size());
  for (const auto& [tid, gov] : tables) {
    std::vector<std::string> policies;
    for (const RowAccessPolicyRecord& p : gov.row_access_policies) {
      policies.push_back(absl::StrCat(
          "{\"policy_id\":\"",
          internal::JsonEscape(p.policy_id),
          "\",\"filter_predicate\":\"",
          internal::JsonEscape(p.filter_predicate),
          "\",\"grantees\":",
          RenderQuotedJsonStrings(p.grantees),
          ",\"creation_time_ms\":",
          p.creation_time_ms,
          ",\"last_modified_time_ms\":",
          p.last_modified_time_ms,
          "}"));
    }
    std::vector<std::string> columns;
    for (const auto& [col, col_gov] : gov.columns) {
      std::string mask;
      switch (col_gov.mask_kind) {
        case DataMaskKind::kNullify:
          mask = "NULLIFY";
          break;
        case DataMaskKind::kSha256:
          mask = "SHA256";
          break;
        case DataMaskKind::kDefaultValue:
          mask = "DEFAULT_VALUE";
          break;
        case DataMaskKind::kDenied:
          mask = "DENIED";
          break;
        default:
          mask = "NONE";
          break;
      }
      columns.push_back(absl::StrCat(
          "{\"column\":\"",
          internal::JsonEscape(col),
          "\",\"policy_tags\":",
          RenderQuotedJsonStrings(col_gov.policy_tags),
          ",\"mask_kind\":\"",
          mask,
          "\",\"mask_grantees\":",
          RenderQuotedJsonStrings(col_gov.mask_grantees),
          ",\"default_mask_value\":\"",
          internal::JsonEscape(col_gov.default_mask_value),
          "\"}"));
    }
    entries.push_back(absl::StrCat("{\"table_id\":\"",
                                   internal::JsonEscape(tid.table_id),
                                   "\",\"row_access_policies\":[",
                                   absl::StrJoin(policies, ","),
                                   "],\"columns\":[",
                                   absl::StrJoin(columns, ","),
                                   "]}"));
  }
  return absl::StrCat("[", absl::StrJoin(entries, ","), "]");
}

DataMaskKind MaskKindFromString(absl::string_view s) {
  const std::string upper = absl::AsciiStrToUpper(s);
  if (upper == "NULLIFY") return DataMaskKind::kNullify;
  if (upper == "SHA256") return DataMaskKind::kSha256;
  if (upper == "DEFAULT_VALUE") return DataMaskKind::kDefaultValue;
  if (upper == "DENIED") return DataMaskKind::kDenied;
  return DataMaskKind::kNone;
}

absl::Status RunDatasetScopedDelete(DuckDBStorage::Impl* impl,
                                    absl::string_view table,
                                    const DatasetId& id) {
  const std::string sql = absl::StrCat(
      "DELETE FROM ",
      table,
      " WHERE project_id = '",
      internal::EscapeStringLiteralInner(id.project_id),
      "' AND dataset_id = '",
      internal::EscapeStringLiteralInner(id.dataset_id),
      "'");
  return internal::RunSql(impl, sql);
}

absl::StatusOr<std::vector<ViewRecord>> QueryViewsForDataset(
    DuckDBStorage::Impl* impl, const DatasetId& id) {
  const std::string where = absl::StrCat(
      "project_id = '",
      internal::EscapeStringLiteralInner(id.project_id),
      "' AND dataset_id = '",
      internal::EscapeStringLiteralInner(id.dataset_id),
      "'");
  const std::string sql = absl::StrCat(
      "SELECT project_id, dataset_id, view_id, ddl_sql FROM ",
      kViewsTable,
      " WHERE ",
      where,
      " ORDER BY view_id");
  ::duckdb_result result;
  if (::duckdb_query(impl->connection, sql.c_str(), &result) != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return internal::DuckDBError(absl::StatusCode::kInternal,
                                 "QueryViewsForDataset failed",
                                 detail);
  }
  std::vector<ViewRecord> out;
  const idx_t n = ::duckdb_row_count(&result);
  out.reserve(static_cast<size_t>(n));
  for (idx_t row = 0; row < n; ++row) {
    ViewRecord rec;
    rec.id.project_id = std::string(::duckdb_value_varchar(&result, 0, row));
    rec.id.dataset_id = std::string(::duckdb_value_varchar(&result, 1, row));
    rec.id.view_id = std::string(::duckdb_value_varchar(&result, 2, row));
    rec.ddl_sql = std::string(::duckdb_value_varchar(&result, 3, row));
    out.push_back(std::move(rec));
  }
  ::duckdb_destroy_result(&result);
  return out;
}

absl::StatusOr<std::vector<RoutineRecord>> QueryRoutinesForDataset(
    DuckDBStorage::Impl* impl, const DatasetId& id) {
  const std::string where = absl::StrCat(
      "project_id = '",
      internal::EscapeStringLiteralInner(id.project_id),
      "' AND dataset_id = '",
      internal::EscapeStringLiteralInner(id.dataset_id),
      "'");
  const std::string sql = absl::StrCat(
      "SELECT project_id, dataset_id, routine_id, kind, language, ddl_sql, ",
      "is_temp, signature_json FROM ",
      kRoutinesTable,
      " WHERE ",
      where,
      " ORDER BY routine_id");
  ::duckdb_result result;
  if (::duckdb_query(impl->connection, sql.c_str(), &result) != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return internal::DuckDBError(absl::StatusCode::kInternal,
                                 "QueryRoutinesForDataset failed",
                                 detail);
  }
  std::vector<RoutineRecord> out;
  const idx_t n = ::duckdb_row_count(&result);
  out.reserve(static_cast<size_t>(n));
  for (idx_t row = 0; row < n; ++row) {
    RoutineRecord rec;
    rec.id.project_id = std::string(::duckdb_value_varchar(&result, 0, row));
    rec.id.dataset_id = std::string(::duckdb_value_varchar(&result, 1, row));
    rec.id.routine_id = std::string(::duckdb_value_varchar(&result, 2, row));
    auto kind_or = RoutineKindFromString(::duckdb_value_varchar(&result, 3, row));
    if (!kind_or.ok()) {
      ::duckdb_destroy_result(&result);
      return kind_or.status();
    }
    rec.kind = *kind_or;
    rec.language = std::string(::duckdb_value_varchar(&result, 4, row));
    rec.ddl_sql = std::string(::duckdb_value_varchar(&result, 5, row));
    rec.is_temp = ::duckdb_value_boolean(&result, 6, row);
    if (!::duckdb_value_is_null(&result, 7, row)) {
      rec.signature_json = std::string(::duckdb_value_varchar(&result, 7, row));
    }
    out.push_back(std::move(rec));
  }
  ::duckdb_destroy_result(&result);
  return out;
}

std::vector<std::string> ListTableIdsInDatasetDir(const fs::path& ds_dir) {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto& entry : fs::directory_iterator(ds_dir, ec)) {
    if (ec) break;
    const std::string name = entry.path().filename().string();
    if (name == internal::kDatasetMetaFile) continue;
    if (name.size() <= internal::kTableMetaSuffix.size()) continue;
    if (!absl::EndsWith(name, internal::kTableMetaSuffix)) continue;
    out.push_back(name.substr(0, name.size() - internal::kTableMetaSuffix.size()));
  }
  std::sort(out.begin(), out.end());
  return out;
}

absl::Status UpsertViewLocked(DuckDBStorage::Impl* impl,
                              absl::string_view data_dir,
                              const ViewRecord& record) {
  const TableId table_id{
      record.id.project_id, record.id.dataset_id, record.id.view_id};
  const fs::path ds_dir = fs::path(std::string(data_dir)) /
                          table_id.project_id / table_id.dataset_id;
  absl::string_view view_query = record.view_query.empty() ? record.ddl_sql
                                                           : record.view_query;
  auto meta_json_or = internal::RenderViewTableMetaJson(
      record.schema, view_query, record.ddl_sql);
  if (!meta_json_or.ok()) return meta_json_or.status();
  const fs::path meta_path =
      ds_dir / absl::StrCat(table_id.table_id, internal::kTableMetaSuffix);
  absl::Status written = internal::WriteFileAtomic(meta_path, *meta_json_or);
  if (!written.ok()) return written;
  const std::string sql = absl::StrCat(
      "INSERT INTO ",
      kViewsTable,
      " (project_id, dataset_id, view_id, ddl_sql) VALUES ('",
      internal::EscapeStringLiteralInner(record.id.project_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.dataset_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.view_id),
      "', '",
      internal::EscapeStringLiteralInner(record.ddl_sql),
      "') ON CONFLICT (project_id, dataset_id, view_id) DO UPDATE SET ",
      "ddl_sql = excluded.ddl_sql");
  return internal::RunSql(impl, sql);
}

absl::Status UpsertRoutineLocked(DuckDBStorage::Impl* impl,
                                 const RoutineRecord& record) {
  const std::string sql = absl::StrCat(
      "INSERT INTO ",
      kRoutinesTable,
      " (project_id, dataset_id, routine_id, kind, language, ddl_sql, ",
      "is_temp, signature_json) VALUES (",
      "'",
      internal::EscapeStringLiteralInner(record.id.project_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.dataset_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.routine_id),
      "', '",
      RoutineKindToString(record.kind),
      "', '",
      internal::EscapeStringLiteralInner(record.language),
      "', '",
      internal::EscapeStringLiteralInner(record.ddl_sql),
      "', ",
      record.is_temp ? "TRUE" : "FALSE",
      ", ",
      record.signature_json.empty()
          ? "NULL"
          : absl::StrCat(
                "'",
                internal::EscapeStringLiteralInner(record.signature_json),
                "'"),
      ") ON CONFLICT (project_id, dataset_id, routine_id) DO UPDATE SET ",
      "kind = excluded.kind, ",
      "language = excluded.language, ",
      "ddl_sql = excluded.ddl_sql, ",
      "is_temp = excluded.is_temp, ",
      "signature_json = excluded.signature_json");
  return internal::RunSql(impl, sql);
}

absl::Status RestoreGovernanceFromSnapshotLocked(DuckDBStorage::Impl* impl,
                                                 const DatasetId& ds,
                                                 absl::string_view json) {
  for (absl::string_view entry : SplitTopLevelJsonObjects(json)) {
    auto table_id_or = ParseJsonStringField(entry, "table_id");
    if (!table_id_or.ok()) return table_id_or.status();
    TableId tid{ds.project_id, ds.dataset_id, *table_id_or};
    TableGovernance gov;
    const size_t rap_pos = entry.find("\"row_access_policies\"");
    if (rap_pos != absl::string_view::npos) {
      const size_t open = entry.find('[', rap_pos);
      const size_t close = entry.find(']', open);
      if (open != absl::string_view::npos && close != absl::string_view::npos) {
        for (absl::string_view policy_json :
             SplitTopLevelJsonObjects(entry.substr(open, close - open + 1))) {
          auto policy_id_or = ParseJsonStringField(policy_json, "policy_id");
          auto filter_or = ParseJsonStringField(policy_json, "filter_predicate");
          if (!policy_id_or.ok() || !filter_or.ok()) continue;
          RowAccessPolicyRecord policy;
          policy.policy_id = *policy_id_or;
          policy.filter_predicate = *filter_or;
          const size_t grantees_pos = policy_json.find("\"grantees\"");
          if (grantees_pos != absl::string_view::npos) {
            const size_t g_open = policy_json.find('[', grantees_pos);
            const size_t g_close = policy_json.find(']', g_open);
            if (g_open != absl::string_view::npos &&
                g_close != absl::string_view::npos) {
              policy.grantees = ParseQuotedJsonStringArray(
                  policy_json.substr(g_open, g_close - g_open + 1));
            }
          }
          std::int64_t created = 0;
          std::int64_t modified = 0;
          const std::string created_needle = "\"creation_time_ms\":";
          const size_t cpos = policy_json.find(created_needle);
          if (cpos != absl::string_view::npos) {
            (void)absl::SimpleAtoi(
                policy_json.substr(cpos + created_needle.size()), &created);
          }
          const std::string modified_needle = "\"last_modified_time_ms\":";
          const size_t mpos = policy_json.find(modified_needle);
          if (mpos != absl::string_view::npos) {
            (void)absl::SimpleAtoi(
                policy_json.substr(mpos + modified_needle.size()), &modified);
          }
          policy.creation_time_ms = created;
          policy.last_modified_time_ms = modified;
          gov.row_access_policies.push_back(std::move(policy));
        }
      }
    }
    const size_t col_pos = entry.find("\"columns\"");
    if (col_pos != absl::string_view::npos) {
      const size_t open = entry.find('[', col_pos);
      const size_t close = entry.find(']', open);
      if (open != absl::string_view::npos && close != absl::string_view::npos) {
        for (absl::string_view col_json :
             SplitTopLevelJsonObjects(entry.substr(open, close - open + 1))) {
          auto column_or = ParseJsonStringField(col_json, "column");
          if (!column_or.ok()) continue;
          ColumnGovernanceRecord col_gov;
          const size_t tags_pos = col_json.find("\"policy_tags\"");
          if (tags_pos != absl::string_view::npos) {
            const size_t t_open = col_json.find('[', tags_pos);
            const size_t t_close = col_json.find(']', t_open);
            if (t_open != absl::string_view::npos &&
                t_close != absl::string_view::npos) {
              col_gov.policy_tags = ParseQuotedJsonStringArray(
                  col_json.substr(t_open, t_close - t_open + 1));
            }
          }
          auto mask_or = ParseJsonStringField(col_json, "mask_kind");
          if (mask_or.ok()) {
            col_gov.mask_kind = MaskKindFromString(*mask_or);
          }
          const size_t mg_pos = col_json.find("\"mask_grantees\"");
          if (mg_pos != absl::string_view::npos) {
            const size_t m_open = col_json.find('[', mg_pos);
            const size_t m_close = col_json.find(']', m_open);
            if (m_open != absl::string_view::npos &&
                m_close != absl::string_view::npos) {
              col_gov.mask_grantees = ParseQuotedJsonStringArray(
                  col_json.substr(m_open, m_close - m_open + 1));
            }
          }
          auto default_or = ParseJsonStringField(col_json, "default_mask_value");
          if (default_or.ok()) {
            col_gov.default_mask_value = *default_or;
          }
          gov.columns.emplace_back(*column_or, std::move(col_gov));
        }
      }
    }
    if (!gov.row_access_policies.empty()) {
      absl::Status saved = SaveRowAccessPolicies(impl, tid, gov);
      if (!saved.ok()) return saved;
    }
    for (const auto& [col, col_gov] : gov.columns) {
      absl::Status col_saved = SaveColumnGovernance(impl, tid, col, col_gov);
      if (!col_saved.ok()) return col_saved;
    }
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status DuckDBStorage::SnapshotDatasetRegistryForTombstoneLocked(
    const DatasetId& id, const fs::path& tombstone_dir) {
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;

  auto views_or = QueryViewsForDataset(impl_.get(), id);
  if (!views_or.ok()) return views_or.status();
  std::vector<std::string> view_json;
  view_json.reserve(views_or->size());
  for (const ViewRecord& rec : *views_or) {
    view_json.push_back(RenderViewRecordJson(rec));
  }
  absl::Status write_views = internal::WriteFileAtomic(
      tombstone_dir / "views.json",
      absl::StrCat("[", absl::StrJoin(view_json, ","), "]"));
  if (!write_views.ok()) return write_views;

  auto routines_or = QueryRoutinesForDataset(impl_.get(), id);
  if (!routines_or.ok()) return routines_or.status();
  std::vector<std::string> routine_json;
  routine_json.reserve(routines_or->size());
  for (const RoutineRecord& rec : *routines_or) {
    routine_json.push_back(RenderRoutineRecordJson(rec));
  }
  absl::Status write_routines = internal::WriteFileAtomic(
      tombstone_dir / "routines.json",
      absl::StrCat("[", absl::StrJoin(routine_json, ","), "]"));
  if (!write_routines.ok()) return write_routines;

  const fs::path ds_dir = fs::path(data_dir_) / id.project_id / id.dataset_id;
  std::vector<std::pair<TableId, TableGovernance>> gov_tables;
  for (const std::string& table_id : ListTableIdsInDatasetDir(ds_dir)) {
    TableId tid{id.project_id, id.dataset_id, table_id};
    auto gov_or = LoadTableGovernance(impl_.get(), tid);
    if (!gov_or.ok()) return gov_or.status();
    if (gov_or->row_access_policies.empty() && gov_or->columns.empty()) {
      continue;
    }
    gov_tables.emplace_back(tid, *gov_or);
  }
  return internal::WriteFileAtomic(
      tombstone_dir / "governance.json",
      RenderGovernanceSnapshotJson(gov_tables));
}

absl::Status DuckDBStorage::RestoreDatasetRegistryFromTombstoneLocked(
    const DatasetId& id, const fs::path& tombstone_dir) {
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;

  const fs::path views_path = tombstone_dir / "views.json";
  std::error_code ec;
  if (fs::exists(views_path, ec)) {
    auto contents_or = internal::ReadFile(views_path);
    if (!contents_or.ok()) return contents_or.status();
    for (absl::string_view entry : SplitTopLevelJsonObjects(*contents_or)) {
      auto rec_or = ParseViewRecordJson(entry, id);
      if (!rec_or.ok()) return rec_or.status();
      absl::Status upserted =
          UpsertViewLocked(impl_.get(), data_dir_, *rec_or);
      if (!upserted.ok()) return upserted;
    }
  }

  const fs::path routines_path = tombstone_dir / "routines.json";
  if (fs::exists(routines_path, ec)) {
    auto contents_or = internal::ReadFile(routines_path);
    if (!contents_or.ok()) return contents_or.status();
    for (absl::string_view entry : SplitTopLevelJsonObjects(*contents_or)) {
      auto rec_or = ParseRoutineRecordJson(entry, id);
      if (!rec_or.ok()) return rec_or.status();
      absl::Status upserted = UpsertRoutineLocked(impl_.get(), *rec_or);
      if (!upserted.ok()) return upserted;
    }
  }

  const fs::path gov_path = tombstone_dir / "governance.json";
  if (fs::exists(gov_path, ec)) {
    auto contents_or = internal::ReadFile(gov_path);
    if (!contents_or.ok()) return contents_or.status();
    return RestoreGovernanceFromSnapshotLocked(impl_.get(), id, *contents_or);
  }
  return absl::OkStatus();
}

absl::Status DuckDBStorage::PurgeDatasetRegistryRowsLocked(const DatasetId& id) {
  absl::Status views = RunDatasetScopedDelete(impl_.get(), kViewsTable, id);
  if (!views.ok()) return views;
  absl::Status routines =
      RunDatasetScopedDelete(impl_.get(), kRoutinesTable, id);
  if (!routines.ok()) return routines;
  absl::Status rap =
      RunDatasetScopedDelete(impl_.get(), kRowAccessPoliciesTable, id);
  if (!rap.ok()) return rap;
  return RunDatasetScopedDelete(impl_.get(), kColumnGovernanceTable, id);
}

absl::StatusOr<std::string> DuckDBStorage::GetDatasetRestMetadataJsonLocked(
    const DatasetId& id) const {
  const fs::path meta_path = DatasetMetaPath(id);
  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  return internal::ExtractRestMetadataFromDatasetMetaJson(*contents_or);
}

absl::StatusOr<std::string> DuckDBStorage::GetDatasetRestMetadataJson(
    const DatasetId& id) const {
  absl::MutexLock lock(&mu_);
  return GetDatasetRestMetadataJsonLocked(id);
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
