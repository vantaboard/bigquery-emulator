#include <cstdint>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/engine/coordinator/sql_preprocess_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

bool HasDecoratorAndSystemTimeConflict(absl::string_view sql) {
  const std::string upper = absl::AsciiStrToUpper(sql);
  if (!absl::StrContains(upper, "FOR SYSTEM_TIME AS OF")) {
    return false;
  }
  bool in_backtick = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (c == '`') {
      in_backtick = !in_backtick;
      continue;
    }
    if (in_backtick && c == '@') {
      return true;
    }
  }
  return false;
}

namespace {

absl::StatusOr<std::int64_t> ResolveDecoratorEpochMs(absl::string_view raw) {
  if (absl::StartsWith(raw, "-")) {
    std::int64_t offset = 0;
    if (!absl::SimpleAtoi(raw, &offset)) {
      return absl::InvalidArgumentError(
          absl::StrCat("invalid relative table decorator offset: ", raw));
    }
    return absl::ToUnixMillis(absl::Now()) + offset;
  }
  std::int64_t epoch = 0;
  if (!absl::SimpleAtoi(raw, &epoch)) {
    return absl::InvalidArgumentError(
        absl::StrCat("invalid absolute table decorator epoch: ", raw));
  }
  return epoch;
}

}  // namespace

// Rewrites `table@epoch` backtick paths to FOR SYSTEM_TIME AS OF.
std::string LowerTableDecorators(absl::string_view sql) {
  if (HasDecoratorAndSystemTimeConflict(sql)) {
    return std::string(sql);
  }
  std::string out;
  out.reserve(sql.size() + 64);
  bool in_backtick = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (c != '`') {
      out.push_back(c);
      continue;
    }
    if (!in_backtick) {
      in_backtick = true;
      out.push_back(c);
      continue;
    }
    size_t start = i + 1;
    size_t end = start;
    while (end < sql.size() && sql[end] != '`') {
      if (sql[end] == '\\' && end + 1 < sql.size()) {
        end += 2;
        continue;
      }
      ++end;
    }
    if (end >= sql.size()) {
      out.push_back(c);
      continue;
    }
    const absl::string_view quoted = sql.substr(start, end - start);
    const size_t at = quoted.rfind('@');
    if (at == absl::string_view::npos || at == 0 || at + 1 >= quoted.size()) {
      out.push_back('`');
      out.append(quoted.data(), quoted.size());
      out.push_back('`');
      i = end;
      continue;
    }
    const absl::string_view base = quoted.substr(0, at);
    const absl::string_view raw_epoch = quoted.substr(at + 1);
    auto epoch_or = ResolveDecoratorEpochMs(raw_epoch);
    if (!epoch_or.ok()) {
      out.push_back('`');
      out.append(quoted.data(), quoted.size());
      out.push_back('`');
      i = end;
      continue;
    }
    absl::StrAppend(&out,
                    "`",
                    base,
                    "` FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(",
                    *epoch_or,
                    ")");
    i = end;
    in_backtick = false;
  }
  return out;
}

}  // namespace sql_preprocess_internal
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
