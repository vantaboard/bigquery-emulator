#include "backend/engine/semantic/functions/string_funcs.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/hash.h"
#include "googlesql/public/functions/normalize_mode.pb.h"
#include "googlesql/public/functions/numeric.h"
#include "googlesql/public/functions/regexp.h"
#include "googlesql/public/functions/string.h"
#include "googlesql/public/functions/string_format.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

using ::googlesql::BigNumericValue;
using ::googlesql::NumericValue;
using ::googlesql::ProductMode;
using ::googlesql::functions::Hasher;
using ::googlesql::functions::MakeRegExpBytes;
using ::googlesql::functions::MakeRegExpUtf8;
using ::googlesql::functions::RegExp;
using ::googlesql::functions::StringFormatUtf8;

// Soundex code for a single uppercase letter. Returns '0' for
// vowels / Y / H / W (skipped during the run-length collapse)
// and '?' for non-letters (treated as ignored by the caller).
bool AnyNull(const std::vector<Value>& args) {
  for (const Value& v : args) {
    if (v.is_null()) return true;
  }
  return false;
}

absl::string_view AsStringOrBytes(const Value& v) {
  return v.type_kind() == ::googlesql::TYPE_BYTES ? v.bytes_value()
                                                  : v.string_value();
}

Value StringOrBytesFromView(const Value& template_value,
                            absl::string_view out) {
  if (template_value.type_kind() == ::googlesql::TYPE_BYTES) {
    return Value::Bytes(std::string(out));
  }
  return Value::String(std::string(out));
}

absl::StatusOr<std::unique_ptr<const RegExp>> MakeRegExpForValue(
    const Value& pattern) {
  if (pattern.type_kind() == ::googlesql::TYPE_BYTES) {
    return MakeRegExpBytes(pattern.bytes_value());
  }
  return MakeRegExpUtf8(pattern.string_value());
}

std::string EncodeBase32(absl::string_view input) {
  static const char* kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  std::string out;
  int buffer = 0;
  int bits = 0;
  for (unsigned char c : input) {
    buffer = (buffer << 8) | c;
    bits += 8;
    while (bits >= 5) {
      bits -= 5;
      out.push_back(kAlphabet[(buffer >> bits) & 0x1F]);
    }
  }
  if (bits > 0) {
    out.push_back(kAlphabet[(buffer << (5 - bits)) & 0x1F]);
  }
  while (out.size() % 8 != 0) {
    out.push_back('=');
  }
  return out;
}

bool DecodeBase32(absl::string_view input,
                  std::string* out,
                  absl::Status* error) {
  static const char* kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  int buffer = 0;
  int bits = 0;
  for (char c : input) {
    if (c == '=') break;
    const char* p = std::strchr(kAlphabet, static_cast<char>(std::toupper(c)));
    if (p == nullptr) {
      *error = absl::InvalidArgumentError("semantic: invalid base32 input");
      return false;
    }
    buffer = (buffer << 5) | static_cast<int>(p - kAlphabet);
    bits += 5;
    if (bits >= 8) {
      bits -= 8;
      out->push_back(static_cast<char>((buffer >> bits) & 0xFF));
    }
  }
  return true;
}

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

  size_t start = 0;
  while (start < in.size() && !absl::ascii_isalpha(in[start]))
    ++start;
  if (start == in.size()) return Value::String("");

  std::string out;
  out.reserve(4);
  out.push_back(static_cast<char>(in[start]));
  char prev_code = SoundexCode(out[0]);

  for (size_t i = start + 1; i < in.size() && out.size() < 4; ++i) {
    if (!absl::ascii_isalpha(in[i])) continue;
    char upper = static_cast<char>(absl::ascii_toupper(in[i]));
    char code = SoundexCode(upper);
    if (code == '?') continue;
    if (code == '0') {
      // H/W are skipped without resetting the previous code; vowels
      // and Y break duplicate suppression for the next consonant.
      if (upper != 'H' && upper != 'W') {
        prev_code = '0';
      }
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

std::string ResolveStructFieldNameForJson(const ::googlesql::StructType& st,
                                          int idx) {
  const ::googlesql::StructField& f = st.field(idx);
  if (f.name.empty()) return absl::StrCat("_", idx);
  return f.name;
}

std::string ValueToJsonText(const Value& v);

std::string BytesLiteral(const std::string& bytes) {
  std::string out = "b\"";
  for (unsigned char c : bytes) {
    if (c == '\\' || c == '"') {
      out.push_back('\\');
      out.push_back(static_cast<char>(c));
    } else if (c >= 0x20 && c < 0x7f) {
      out.push_back(static_cast<char>(c));
    } else {
      absl::StrAppendFormat(&out, "\\x%02x", c);
    }
  }
  out.push_back('"');
  return out;
}

std::string StructToJson(const Value& v) {
  std::string out = "{";
  for (int i = 0; i < v.num_fields(); ++i) {
    if (i > 0) absl::StrAppend(&out, ",");
    const ::googlesql::StructType* st = v.type()->AsStruct();
    std::string name = st != nullptr ? ResolveStructFieldNameForJson(*st, i)
                                     : absl::StrCat("_", i);
    absl::StrAppend(&out, "\"", name, "\":", ValueToJsonText(v.field(i)));
  }
  absl::StrAppend(&out, "}");
  return out;
}

std::string ValueToJsonText(const Value& v) {
  if (v.is_null()) return "null";
  switch (v.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      return v.bool_value() ? "true" : "false";
    case ::googlesql::TYPE_INT64:
      return std::to_string(v.int64_value());
    case ::googlesql::TYPE_DOUBLE: {
      std::string s = absl::StrCat(v.double_value());
      return s;
    }
    case ::googlesql::TYPE_STRING:
      return absl::StrCat("\"", absl::Utf8SafeCEscape(v.string_value()), "\"");
    case ::googlesql::TYPE_BYTES:
      return absl::StrCat("\"", absl::Base64Escape(v.bytes_value()), "\"");
    case ::googlesql::TYPE_JSON:
      return v.json_string();
    case ::googlesql::TYPE_ARRAY: {
      std::string out = "[";
      for (int i = 0; i < v.num_elements(); ++i) {
        if (i > 0) out.push_back(',');
        absl::StrAppend(&out, ValueToJsonText(v.element(i)));
      }
      out.push_back(']');
      return out;
    }
    case ::googlesql::TYPE_STRUCT:
      return StructToJson(v);
    case ::googlesql::TYPE_RANGE: {
      // BigQuery TO_JSON_STRING serializes RANGE bounds with single
      // quotes; googlesql GetSQLLiteral uses double quotes.
      std::string lit = v.GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL);
      for (char& c : lit) {
        if (c == '"') c = '\'';
      }
      return absl::StrCat("\"", absl::Utf8SafeCEscape(lit), "\"");
    }
    default:
      return absl::StrCat("\"", absl::Utf8SafeCEscape(v.DebugString()), "\"");
  }
}

absl::StatusOr<Value> RegexpContains(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: REGEXP_CONTAINS expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  auto re = MakeRegExpForValue(args[1]);
  if (!re.ok()) return re.status();
  absl::Status error;
  bool out = false;
  if (!(*re)->Contains(AsStringOrBytes(args[0]), &out, &error)) {
    return error;
  }
  return Value::Bool(out);
}

absl::StatusOr<Value> Format(const std::vector<Value>& args) {
  return FormatString(args);
}

absl::StatusOr<Value> ToJson(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: TO_JSON expects one or two arguments");
  }
  if (!args[0].is_valid() || args[0].is_null()) return Value::NullJson();
  return Value::UnvalidatedJsonString(ValueToJsonText(args[0]));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
