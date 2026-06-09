#include "backend/engine/semantic/functions/dispatch.h"

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/functions/array_funcs.h"
#include "backend/engine/semantic/functions/datetime_funcs.h"
#include "backend/engine/semantic/functions/geog_funcs.h"
#include "backend/engine/semantic/functions/json_funcs.h"
#include "backend/engine/semantic/functions/numeric_edges.h"
#include "backend/engine/semantic/functions/operator_funcs.h"
#include "backend/engine/semantic/functions/range_funcs.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/functions/string_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

std::optional<absl::StatusOr<Value>> Dispatch(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type,
    const EvalContext* ctx) {
  (void)return_type;
  if (name == "$extract") return Extract(args, return_type);
  if (name == "$subscript") return JsonSubscript(args, return_type);
  // Numeric edges.
  if (name == "bit_count") return BitCount(args);
  if (name == "ieee_divide") return IeeeDivide(args);
  // String family.
  if (name == "soundex") return Soundex(args);
  if (name == "instr") return Instr(args);
  if (name == "ascii") return Ascii(args);
  if (name == "byte_length") return ByteLength(args);
  if (name == "char_length" || name == "character_length") {
    return CharLength(args);
  }
  if (name == "octet_length") return OctetLength(args);
  if (name == "chr") return Chr(args);
  if (name == "unicode") return Unicode(args);
  if (name == "concat") return Concat(args);
  if (name == "length") return Length(args);
  if (name == "lower") return Lower(args);
  if (name == "upper") return Upper(args);
  if (name == "trim") return Trim(args);
  if (name == "ltrim") return Ltrim(args);
  if (name == "rtrim") return Rtrim(args);
  if (name == "lpad") return Lpad(args);
  if (name == "rpad") return Rpad(args);
  if (name == "floor") return Floor(args);
  if (name == "ceil" || name == "ceiling") return Ceil(args);
  if (name == "round") return Round(args);
  if (name == "trunc") return Trunc(args);
  if (name == "sign") return Sign(args);
  if (name == "div") return Div(args);
  if (name == "mod") return Mod(args);
  if (name == "pow" || name == "power") return Pow(args);
  if (name == "log") return Log(args);
  if (name == "ln") return Log(args);
  if (name == "sqrt") return Sqrt(args);
  if (name == "sin") return Sin(args);
  if (name == "cos") return Cos(args);
  if (name == "asin") return Asin(args);
  if (name == "acos") return Acos(args);
  if (name == "atan2") return Atan2(args);
  if (name == "replace") return Replace(args);
  if (name == "translate") return Translate(args);
  if (name == "reverse") return Reverse(args);
  if (name == "starts_with") return StartsWith(args);
  if (name == "ends_with") return EndsWith(args);
  if (name == "left") return Left(args);
  if (name == "right") return Right(args);
  if (name == "substr" || name == "substring") return Substr(args);
  if (name == "strpos") return Strpos(args);
  if (name == "split") return Split(args, return_type);
  if (name == "to_hex") return ToHex(args);
  if (name == "from_hex") return FromHex(args);
  if (name == "to_base64") return ToBase64(args);
  if (name == "from_base64") return FromBase64(args);
  if (name == "to_base32") return ToBase32(args);
  if (name == "from_base32") return FromBase32(args);
  if (name == "md5") return Md5(args);
  if (name == "sha1") return Sha1(args);
  if (name == "sha256") return Sha256(args);
  if (name == "sha512") return Sha512(args);
  if (name == "farm_fingerprint") return FarmFingerprintFunc(args);
  if (name == "initcap") return InitcapFunc(args);
  if (name == "normalize") return NormalizeFunc(args);
  if (name == "normalize_and_casefold") return NormalizeAndCasefoldFunc(args);
  if (name == "safe_convert_bytes_to_string") {
    return SafeConvertBytesToString(args);
  }
  if (name == "code_points_to_string") return CodePointsToString(args);
  if (name == "code_points_to_bytes") return CodePointsToBytes(args);
  if (name == "to_code_points") return ToCodePoints(args, return_type);
  if (name == "least") return Least(args, return_type);
  if (name == "greatest") return Greatest(args, return_type);
  if (name == "parse_numeric") return ParseNumericFunc(args);
  if (name == "parse_bignumeric") return ParseBignumeric(args, ctx);
  if (name == "regexp_contains") return RegexpContains(args);
  if (name == "regexp_extract") return RegexpExtract(args);
  if (name == "regexp_extract_all") {
    return RegexpExtractAll(args, return_type);
  }
  if (name == "regexp_replace") return RegexpReplace(args);
  if (name == "regexp_instr") return RegexpInstr(args);
  if (name == "format") {
    if (args.size() == 2 && !args[0].is_null() &&
        args[0].string_value() == "%T" && !args[1].is_null() &&
        args[1].type_kind() == ::googlesql::TYPE_GEOGRAPHY) {
      return EmuFormatTypeLiteral({args[1]});
    }
    return Format(args);
  }
  if (name == "st_geogpoint") return StGeogPoint(args);
  if (name == "to_json") return ToJson(args);
  if (name == "to_json_string") return ToJsonString(args);
  if (name == "parse_json") return ParseJson(args);
  if (name == "json_extract") return JsonExtract(args, return_type);
  if (name == "json_query") return JsonQuery(args, return_type);
  if (name == "json_value") return JsonValue(args, return_type);
  if (name == "json_extract_scalar") {
    return JsonExtractScalar(args, return_type);
  }
  if (name == "json_extract_array") return JsonExtractArray(args, return_type);
  if (name == "json_extract_string_array") {
    return JsonExtractStringArray(args, return_type);
  }
  if (name == "json_query_array") return JsonQueryArray(args, return_type);
  if (name == "json_value_array") return JsonValueArray(args, return_type);
  if (name == "json_type") return JsonTypeFunc(args);
  if (name == "bool") return JsonCastBool(args);
  if (name == "int64") return JsonCastInt64(args);
  if (name == "float64") return JsonCastFloat64(args);
  if (name == "string") {
    if (args.size() == 1) return JsonCastString(args);
    return StringFunc(args);  // datetime_funcs.h
  }
  if (name == "justify_days") return JustifyDays(args);
  if (name == "justify_hours") return JustifyHours(args);
  if (name == "justify_interval") return JustifyInterval(args);
  // Parse functions.
  if (name == "parse_date") return ParseDate(args);
  if (name == "parse_datetime") return ParseDatetime(args);
  if (name == "parse_timestamp") return ParseTimestamp(args);
  if (name == "parse_time") return ParseTime(args);
  // Format functions.
  if (name == "format_date") return FormatDate(args);
  if (name == "format_datetime") return FormatDatetime(args);
  if (name == "format_timestamp") return FormatTimestamp(args);
  if (name == "format_time") return FormatTime(args);
  // Date/time arithmetic and current_* / unix_*.
  if (name == "date_add" || name == "date_sub" || name == "date_diff" ||
      name == "date_trunc") {
    return DateAddSubDiffTrunc(name, args);
  }
  if (name == "datetime_add" || name == "datetime_sub" ||
      name == "datetime_diff" || name == "datetime_trunc") {
    return DatetimeAddSubDiffTrunc(name, args);
  }
  if (name == "timestamp_add" || name == "timestamp_sub" ||
      name == "timestamp_diff" || name == "timestamp_trunc") {
    return TimestampAddSubDiffTrunc(name, args);
  }
  if (name == "time_add" || name == "time_sub" || name == "time_diff" ||
      name == "time_trunc") {
    return TimeAddSubDiffTrunc(name, args);
  }
  if (name == "current_date") return CurrentDate(args);
  if (name == "current_datetime") return CurrentDatetime(args);
  if (name == "current_time") return CurrentTime(args);
  if (name == "current_timestamp") return CurrentTimestamp(args);
  if (name == "unix_seconds") return UnixSeconds(args);
  if (name == "unix_millis") return UnixMillis(args);
  if (name == "unix_micros") return UnixMicros(args);
  if (name == "unix_date") return UnixDate(args);
  if (name == "timestamp_seconds") return TimestampSeconds(args);
  if (name == "timestamp_millis") return TimestampMillis(args);
  if (name == "timestamp_micros") return TimestampMicros(args);
  if (name == "date_from_unix_date") return DateFromUnixDate(args);
  if (name == "last_day") return LastDay(args);
  if (name == "timestamp_bucket") return TimestampBucket(args);
  if (name == "generate_array") {
    return GenerateArray(args, return_type);
  }
  if (name == "array_concat") {
    return ArrayConcat(args, return_type);
  }
  if (name == "array_length") return ArrayLength(args);
  if (name == "array_to_string") return ArrayToString(args);
  if (name == "array_reverse") {
    return ArrayReverse(args, return_type);
  }
  if (name == "$array_at_offset") {
    return ArrayAtOffset(args, return_type, /*safe=*/false);
  }
  if (name == "$safe_array_at_offset") {
    return ArrayAtOffset(args, return_type, /*safe=*/true);
  }
  if (name == "$array_at_ordinal") {
    return ArrayAtOrdinal(args, return_type, /*safe=*/false);
  }
  if (name == "$safe_array_at_ordinal") {
    return ArrayAtOrdinal(args, return_type, /*safe=*/true);
  }
  if (name == "generate_date_array") {
    return GenerateDateArray(args, return_type);
  }
  if (name == "generate_timestamp_array") {
    return GenerateTimestampArray(args, return_type);
  }
  if (name == "time") return TimeConstructor(args);
  if (name == "date") return DateConstructor(args);
  if (name == "datetime") return DatetimeConstructor(name, args);
  if (name == "timestamp" || name == "timestamp_from_date" ||
      name == "timestamp_from_datetime") {
    return TimestampConstructor(name, args);
  }
  // Intervals and extract.
  if (name == "make_interval") return MakeInterval(args);
  if (name == "range") return RangeCtor(args, return_type);
  if (name == "range_start") return RangeStart(args);
  if (name == "range_end") return RangeEnd(args);
  if (name == "range_overlaps") return RangeOverlaps(args);
  if (name == "extract") return Extract(args, return_type);
  if (auto specialized = DispatchSpecializedScalar(name, args, return_type)) {
    return *specialized;
  }
  return std::nullopt;
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
