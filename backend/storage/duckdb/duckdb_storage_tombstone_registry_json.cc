#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_tombstone_registry_internal.h"
#include "backend/storage/storage.h"
#include "backend/storage/table_governance.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace tombstone_registry_internal {

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
  std::string sig =
      rec.signature_json.empty()
          ? "null"
          : absl::StrCat("\"", internal::JsonEscape(rec.signature_json), "\"");
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

namespace {

bool AdvancePastWhitespaceAndComma(absl::string_view array_json, size_t& pos) {
  while (pos < array_json.size() &&
         (array_json[pos] == ' ' || array_json[pos] == ',' ||
          array_json[pos] == '\n')) {
    ++pos;
  }
  return pos < array_json.size() && array_json[pos] != ']';
}

std::optional<absl::string_view> ExtractBalancedObject(
    absl::string_view array_json, size_t& pos) {
  if (pos >= array_json.size() || array_json[pos] != '{') {
    return std::nullopt;
  }
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
      if (c == '"') {
        in_string = false;
      }
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
        return array_json.substr(start, pos - start + 1);
      }
    }
  }
  return std::nullopt;
}

}  // namespace

std::vector<absl::string_view> SplitTopLevelJsonObjects(
    absl::string_view array_json) {
  std::vector<absl::string_view> out;
  size_t pos = array_json.find('[');
  if (pos == absl::string_view::npos) {
    return out;
  }
  ++pos;
  while (AdvancePastWhitespaceAndComma(array_json, pos)) {
    if (array_json[pos] != '{') {
      break;
    }
    auto object = ExtractBalancedObject(array_json, pos);
    if (!object.has_value()) {
      break;
    }
    out.push_back(*object);
    ++pos;
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
      policies.push_back(absl::StrCat("{\"policy_id\":\"",
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
      columns.push_back(
          absl::StrCat("{\"column\":\"",
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

}  // namespace tombstone_registry_internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
