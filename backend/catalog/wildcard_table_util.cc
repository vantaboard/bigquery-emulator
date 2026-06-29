#include "backend/catalog/wildcard_table_util.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using schema::ColumnMode;
using schema::ColumnSchema;
using schema::ColumnType;
using schema::TableSchema;

bool ColumnTypesCompatible(const ColumnSchema& a, const ColumnSchema& b) {
  if (a.type != b.type) return false;
  if (a.mode != b.mode) return false;
  if (a.type == ColumnType::kStruct) {
    if (a.fields.size() != b.fields.size()) return false;
    for (size_t i = 0; i < a.fields.size(); ++i) {
      if (!ColumnTypesCompatible(a.fields[i], b.fields[i])) return false;
    }
  }
  return true;
}

ColumnSchema TableSuffixColumn() {
  ColumnSchema col;
  col.name = kTableSuffixColumnName;
  col.type = ColumnType::kString;
  col.mode = ColumnMode::kRequired;
  return col;
}

bool IsDataColumn(absl::string_view name) {
  return name != kTableSuffixColumnName;
}

bool SchemaHasColumn(absl::string_view name,
                     const std::vector<ColumnSchema>& columns) {
  return std::any_of(columns.begin(),
                     columns.end(),
                     [name](const ColumnSchema& c) { return c.name == name; });
}

}  // namespace

bool IsWildcardTableId(absl::string_view table_id) {
  return !table_id.empty() && table_id.back() == '*';
}

std::string WildcardTablePrefix(absl::string_view wildcard_table_id) {
  if (wildcard_table_id.empty() || wildcard_table_id.back() != '*') {
    return std::string(wildcard_table_id);
  }
  return std::string(wildcard_table_id.substr(0, wildcard_table_id.size() - 1));
}

bool TableMatchesWildcard(absl::string_view table_id,
                          absl::string_view wildcard_table_id) {
  if (!IsWildcardTableId(wildcard_table_id)) return false;
  const absl::string_view prefix = WildcardTablePrefix(wildcard_table_id);
  return absl::StartsWith(table_id, prefix);
}

std::string TableSuffixFor(absl::string_view table_id,
                           absl::string_view prefix) {
  if (prefix.empty()) return std::string(table_id);
  if (!absl::StartsWith(table_id, prefix)) return std::string(table_id);
  return std::string(table_id.substr(prefix.size()));
}

absl::StatusOr<TableSchema> UnifyWildcardTableSchemas(
    const storage::Storage* storage,
    const std::vector<storage::TableId>& matched) {
  if (storage == nullptr) {
    return absl::FailedPreconditionError(
        "UnifyWildcardTableSchemas: storage is null");
  }
  if (matched.empty()) {
    return absl::InvalidArgumentError(
        "UnifyWildcardTableSchemas: matched table set is empty");
  }

  absl::flat_hash_map<std::string, ColumnSchema> by_name;
  for (const storage::TableId& id : matched) {
    absl::StatusOr<TableSchema> schema_or = storage->GetSchema(id);
    if (!schema_or.ok()) return schema_or.status();
    for (const ColumnSchema& col : schema_or->columns) {
      auto it = by_name.find(col.name);
      if (it == by_name.end()) {
        by_name.emplace(col.name, col);
        continue;
      }
      if (!ColumnTypesCompatible(it->second, col)) {
        return absl::InvalidArgumentError(
            absl::StrCat("Cannot read field of type ",
                         schema::ColumnTypeName(col.type),
                         " as ",
                         schema::ColumnTypeName(it->second.type),
                         " Field: ",
                         col.name));
      }
    }
  }

  absl::StatusOr<TableSchema> newest_or = storage->GetSchema(matched.back());
  if (!newest_or.ok()) return newest_or.status();

  TableSchema unified;
  unified.columns.reserve(by_name.size() + 1);
  for (const ColumnSchema& col : newest_or->columns) {
    auto it = by_name.find(col.name);
    if (it == by_name.end()) {
      return absl::InternalError(
          absl::StrCat("UnifyWildcardTableSchemas: missing unified column '",
                       col.name,
                       "' for table ",
                       matched.back().table_id));
    }
    unified.columns.push_back(it->second);
  }
  for (const auto& entry : by_name) {
    if (!SchemaHasColumn(entry.first, newest_or->columns)) {
      unified.columns.push_back(entry.second);
    }
  }
  unified.columns.push_back(TableSuffixColumn());
  return unified;
}

absl::StatusOr<std::vector<WildcardColumnMap>> BuildWildcardColumnMaps(
    const storage::Storage* storage,
    const std::vector<storage::TableId>& matched,
    const TableSchema& union_schema) {
  if (storage == nullptr) {
    return absl::FailedPreconditionError(
        "BuildWildcardColumnMaps: storage is null");
  }

  std::vector<WildcardColumnMap> out;
  out.reserve(matched.size());
  for (const storage::TableId& id : matched) {
    absl::StatusOr<TableSchema> schema_or = storage->GetSchema(id);
    if (!schema_or.ok()) return schema_or.status();

    absl::flat_hash_map<std::string, int> physical_by_name;
    for (size_t i = 0; i < schema_or->columns.size(); ++i) {
      physical_by_name.emplace(schema_or->columns[i].name, static_cast<int>(i));
    }

    WildcardColumnMap map;
    map.table_id = id;
    map.schema = *std::move(schema_or);
    map.union_to_physical.reserve(union_schema.columns.size());
    for (const ColumnSchema& union_col : union_schema.columns) {
      if (!IsDataColumn(union_col.name)) {
        map.union_to_physical.push_back(-1);
        continue;
      }
      auto it = physical_by_name.find(union_col.name);
      map.union_to_physical.push_back(
          it == physical_by_name.end() ? -1 : it->second);
    }
    out.push_back(std::move(map));
  }
  return out;
}

bool SuffixMatchesAllowList(absl::string_view suffix,
                            const std::vector<std::string>& allowlist) {
  if (allowlist.empty()) return false;
  if (allowlist.size() == 2 && allowlist[0] <= allowlist[1]) {
    return suffix >= allowlist[0] && suffix <= allowlist[1];
  }
  return std::find(allowlist.begin(), allowlist.end(), suffix) !=
         allowlist.end();
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
