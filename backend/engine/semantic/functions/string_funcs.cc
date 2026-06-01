#include "backend/engine/semantic/functions/string_funcs.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

// Soundex code for a single uppercase letter. Returns '0' for
// vowels / Y / H / W (skipped during the run-length collapse)
// and '?' for non-letters (treated as ignored by the caller).
//
// Mapping per the classic Soundex (and BigQuery's documented
// English-alphabet table):
//
//   B F P V             -> 1
//   C G J K Q S X Z     -> 2
//   D T                 -> 3
//   L                   -> 4
//   M N                 -> 5
//   R                   -> 6
//   A E I O U Y H W     -> 0 (skipped after the first letter)
char SoundexCode(char ch) {
  switch (ch) {
    case 'B':
    case 'F':
    case 'P':
    case 'V':
      return '1';
    case 'C':
    case 'G':
    case 'J':
    case 'K':
    case 'Q':
    case 'S':
    case 'X':
    case 'Z':
      return '2';
    case 'D':
    case 'T':
      return '3';
    case 'L':
      return '4';
    case 'M':
    case 'N':
      return '5';
    case 'R':
      return '6';
    case 'A':
    case 'E':
    case 'I':
    case 'O':
    case 'U':
    case 'Y':
    case 'H':
    case 'W':
      return '0';
    default:
      return '?';
  }
}

}  // namespace

absl::StatusOr<Value> Soundex(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: SOUNDEX expects exactly one argument");
  }
  const Value& v = args[0];
  if (v.is_null()) return Value::NullString();
  if (v.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: SOUNDEX requires STRING, got ",
                     v.type()->DebugString()));
  }
  absl::string_view in = v.string_value();
  if (in.empty()) return Value::String("");

  // Find the first ASCII letter and use its uppercased form as
  // the result's first character. BigQuery's algorithm anchors
  // on the first letter; non-letter prefix bytes are skipped.
  // If there is no letter at all, return the input verbatim
  // (matching BigQuery's observed behavior of returning the
  // input when no SOUNDEX-eligible character is present).
  size_t start = 0;
  while (start < in.size() && !absl::ascii_isalpha(in[start]))
    ++start;
  if (start == in.size()) return Value::String(std::string(in));

  std::string out;
  out.reserve(4);
  out.push_back(static_cast<char>(absl::ascii_toupper(in[start])));
  char prev_code = SoundexCode(out[0]);

  // Walk the remaining characters, mapping each letter to its
  // SOUNDEX code and collapsing runs of the same non-zero code.
  // Letters classified as '0' (vowels + Y/H/W) act as a "code
  // reset" -- they themselves are skipped from the output but
  // they break the adjacency chain so a subsequent same-coded
  // consonant emits again (matches the classic algorithm; e.g.
  // ASHCRAFT -> A261 because the H between S and C resets the
  // 2-run).
  for (size_t i = start + 1; i < in.size() && out.size() < 4; ++i) {
    if (!absl::ascii_isalpha(in[i])) continue;
    char upper = static_cast<char>(absl::ascii_toupper(in[i]));
    char code = SoundexCode(upper);
    if (code == '?') continue;  // non-letter; defensive (isalpha filters)
    if (code == '0') {
      prev_code = '0';
      continue;
    }
    if (code != prev_code) {
      out.push_back(code);
    }
    prev_code = code;
  }
  while (out.size() < 4)
    out.push_back('0');
  return Value::String(std::move(out));
}

absl::StatusOr<Value> Instr(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 4) {
    return absl::InvalidArgumentError(
        "semantic: INSTR expects 2 to 4 arguments");
  }
  for (const auto& v : args) {
    if (v.is_null()) return Value::NullInt64();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_STRING ||
      args[1].type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: INSTR requires STRING value and STRING subvalue");
  }
  absl::string_view value = args[0].string_value();
  absl::string_view sub = args[1].string_value();
  int64_t position = 1;
  int64_t occurrence = 1;
  if (args.size() >= 3) {
    if (args[2].type_kind() != ::googlesql::TYPE_INT64) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: INSTR position argument must be INT64");
    }
    position = args[2].int64_value();
    if (position == 0) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: INSTR position must be non-zero");
    }
  }
  if (args.size() == 4) {
    if (args[3].type_kind() != ::googlesql::TYPE_INT64) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: INSTR occurrence argument must be INT64");
    }
    occurrence = args[3].int64_value();
    if (occurrence <= 0) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: INSTR occurrence must be positive");
    }
  }

  // BigQuery's INSTR semantics in terms of byte-level offsets:
  //   * empty `subvalue` always returns the starting position
  //     (or its absolute value when `position` is negative).
  //   * positive `position` searches left-to-right starting at
  //     1-based byte index `position`.
  //   * negative `position` searches right-to-left starting at
  //     the byte `len(value) + position` (0-based) and counts
  //     occurrences walking left.
  //   * Returns 1-based position of the Nth match, or 0 if not
  //     found.
  int64_t len = static_cast<int64_t>(value.size());
  if (sub.empty()) {
    int64_t start = position > 0 ? position : (len + position + 1);
    if (start < 1) start = 1;
    if (start > len + 1) start = len + 1;
    return Value::Int64(start);
  }

  int64_t sub_len = static_cast<int64_t>(sub.size());
  if (position > 0) {
    int64_t start_idx = position - 1;
    if (start_idx >= len) return Value::Int64(0);
    int64_t seen = 0;
    for (int64_t i = start_idx; i + sub_len <= len; ++i) {
      if (value.substr(i, sub_len) == sub) {
        if (++seen == occurrence) return Value::Int64(i + 1);
      }
    }
    return Value::Int64(0);
  }

  // Negative position: 1-based end-relative anchor. Walking
  // right-to-left counts non-overlapping occurrences; the
  // returned index is still 1-based absolute.
  int64_t end_idx = len + position;
  if (end_idx < 0) return Value::Int64(0);
  int64_t seen = 0;
  for (int64_t i = end_idx; i >= 0; --i) {
    if (i + sub_len > len) continue;
    if (value.substr(i, sub_len) == sub) {
      if (++seen == occurrence) return Value::Int64(i + 1);
    }
  }
  return Value::Int64(0);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
