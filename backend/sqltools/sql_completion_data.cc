#include "backend/sqltools/sql_completion_data.h"

#include "backend/sqltools/sql_completion_data_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace {

const std::vector<FunctionInfoEntry>& BuildFunctionInfo() {
  static const std::vector<FunctionInfoEntry>* kEntries = [] {
    auto* merged = new std::vector<FunctionInfoEntry>();
    const auto& part_a = completion_data_internal::FunctionInfoEntriesA();
    const auto& part_b = completion_data_internal::FunctionInfoEntriesB();
    const auto& part_c = completion_data_internal::FunctionInfoEntriesC();
    merged->reserve(part_a.size() + part_b.size() + part_c.size());
    merged->insert(merged->end(), part_a.begin(), part_a.end());
    merged->insert(merged->end(), part_b.begin(), part_b.end());
    merged->insert(merged->end(), part_c.begin(), part_c.end());
    return merged;
  }();
  return *kEntries;
}

}  // namespace

const std::vector<ClauseWordEntry>& ClauseWords() {
  return completion_data_internal::ClauseWordEntries();
}

const std::vector<std::string>& ExpressionKeywords() {
  return completion_data_internal::ExpressionKeywordEntries();
}

const std::vector<FunctionInfoEntry>& FunctionInfo() {
  return BuildFunctionInfo();
}

const std::vector<FunctionInfoEntry>& TvfFunctionInfo() {
  return completion_data_internal::TvfFunctionInfoEntries();
}

const std::vector<std::string>& TypeNames() {
  return completion_data_internal::TypeNameEntries();
}

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
