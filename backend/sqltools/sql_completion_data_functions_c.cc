#include "backend/sqltools/sql_completion_data_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace completion_data_internal {
const std::vector<FunctionInfoEntry>& FunctionInfoEntriesC() {
  static const std::vector<FunctionInfoEntry>* kEntries = new std::vector<
      FunctionInfoEntry>{
      {"TAN",
       "numeric_expr",
       "Computes tangent of <numeric_expr>. Generates an error if an overflow "
       "occurs.",
       ""},
      {"TANH",
       "numeric_expr",
       "Computes hyperbolic tangent of <numeric_expr>. Does not fail.",
       ""},
      {"TIMESTAMP",
       "timestamp_expr [, time_zone]",
       "Converts a STRING expression to a TIMESTAMP data type.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#timestamp"},
      {"TIMESTAMP_ADD",
       "timestamp_expr, INTERVAL int64_expr date_part",
       "Adds <int64_expr> units of <date_part> to the timestamp, independent "
       "of any time zone.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#timestamp_add"},
      {"TIMESTAMP_DIFF",
       "timestamp_expr, timestamp_expr, date_part",
       "Returns the number of whole specified <date_part> intervals between "
       "two timestamps. The first <timestamp_expr> represents the later date; "
       "if the first <timestamp_expr> is earlier than the second "
       "<timestamp_expr>, the output is negative. Throws an error if the "
       "computation overflows the result type, such as if the difference in "
       "microseconds between the two timestamps would overflow an INT64 value.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#timestamp_diff"},
      {"TIMESTAMP_MICROS",
       "INT64_expr",
       "Interprets <INT64_expr> as the number of microseconds since 1970-01-01 "
       "00:00:00 UTC.",
       ""},
      {"TIMESTAMP_MILLIS",
       "INT64_expr",
       "Interprets <INT64_expr> as the number of milliseconds since 1970-01-01 "
       "00:00:00 UTC.",
       ""},
      {"TIMESTAMP_SECONDS",
       "INT64_expr",
       "Interprets <INT64_expr> as the number of seconds since 1970-01-01 "
       "00:00:00 UTC.",
       ""},
      {"TIMESTAMP_SUB",
       "timestamp_expr, INTERVAL int64_expr date_part",
       "Subtracts <int64_expr> units of <date_part> from the timestamp, "
       "independent of any time zone.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#timestamp_sub"},
      {"TIMESTAMP_TRUNC",
       "timestamp_expr, date_part [, time_zone]",
       "Truncates a timestamp to the granularity of <date_part>.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#timestamp_trunc"},
      {"TRANSLATE",
       "expression, source_characters, target_characters",
       "In the input <expression>, replace the characters in "
       "<source_characters> to the corresponding characters in "
       "<target_characters>. Note that each character will be translated at "
       "most once. Characters in <expression> but not in <source_characters> "
       "will be added to the result with no change.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#translate"},
      {"TRIM",
       "value1 [, value2]",
       "Removes all leading and trailing characters that match <value2>. If "
       "<value2> is not specified, all leading and trailing whitespace "
       "characters (as defined by the Unicode standard) are removed. If the "
       "first argument is of type BYTES, the second argument is required.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#trim"},
      {"TRUNC",
       "numeric_expr [, decimal_places]",
       "If only <numeric_expr> is present, TRUNC rounds <numeric_expr> to the "
       "nearest integer whose absolute value is not greater than the absolute "
       "value of <numeric_expr>. If <decimal_places> is also present, TRUNC "
       "behaves like ROUND(<numeric_expr>, <decimal_places>), but always "
       "rounds towards zero and never overflows.",
       ""},
      {"UNICODE",
       "input",
       "Returns the Unicode code point for the first character of the <input> "
       "string. Function returns 0 if the string is empty, or the resulted "
       "unicode code point is 0.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#unicode"},
      {"UNIX_DATE",
       "date_expr",
       "Returns the number of days since 1970-01-01.",
       ""},
      {"UNIX_MICROS",
       "timestamp_expr",
       "Returns the number of microseconds since 1970-01-01 00:00:00 UTC. "
       "Truncates higher levels of precision.",
       ""},
      {"UNIX_MILLIS",
       "timestamp_expr",
       "Returns the number of milliseconds since 1970-01-01 00:00:00 UTC. "
       "Truncates higher levels of precision.",
       ""},
      {"UNIX_SECONDS",
       "timestamp_expr",
       "Returns the number of seconds since 1970-01-01 00:00:00 UTC. Truncates "
       "higher levels of precision.",
       ""},
      {"UNNEST",
       "array_expr",
       "The UNNEST operator takes an ARRAY and returns a table, with one row "
       "for each element in the ARRAY. ",
       ""},
      {"UPPER",
       "value",
       "For STRING arguments, returns the original string with all alphabetic "
       "characters in uppercase. Mapping between uppercase and lowercase is "
       "done according to the Unicode Character Database without taking into "
       "account language-specific mappings.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#upper"},
      {"AGG", "expression", "", ""},
      {"APPROX_TOP_SUM", "expression, weight, number", "", ""},
      {"ARRAY", "subquery", "", ""},
      {"ARRAY_REVERSE", "value", "", ""},
      {"BIT_COUNT", "expression", "", ""},
      {"BOOL",
       "json_expr",
       "Takes a JSON expression, extracts a JSON boolean, and returns that "
       "value as a SQL BOOL. If the expression is SQL NULL, the function "
       "returns SQL NULL. If the extracted JSON value is not a boolean, an "
       "error is produced.",
       ""},
      {"CLASSIFIER",
       "",
       "Returns the matched symbol of a row in a match inside MATCH_RECOGNIZE",
       ""},
      {"COALESCE", "expr[, ...]", "", ""},
      {"CODE_POINTS_TO_BYTES", "ascii_values", "", ""},
      {"CODE_POINTS_TO_STRING", "value", "", ""},
      {"CONTAINS_SUBSTR", "expression, search_value_literal", "", ""},
      {"CORR", "X1, X2", "", ""},
      {"COVAR_POP", "X1, X2", "", ""},
      {"COVAR_SAMP", "X1, X2", "", ""},
      {"CURRENT_DATETIME", "[timezone]", "", ""},
      {"CURRENT_TIME", "[timezone]", "", ""},
      {"DATETIME", "year, month, day, hour, minute, second", "", ""},
      {"DATETIME_ADD",
       "datetime_expression, INTERVAL int64_expression part",
       "",
       ""},
      {"DATETIME_BUCKET",
       "datetime_expression, bucket_width [, origin]",
       "Gets the lower bound of the datetime bucket that contains a datetime.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#datetime_bucket"},
      {"DATETIME_DIFF",
       "datetime_expression_a, datetime_expression_b, part",
       "",
       ""},
      {"DATETIME_SUB",
       "datetime_expression, INTERVAL int64_expression part",
       "",
       ""},
      {"DATETIME_TRUNC", "datetime_expression, part", "", ""},
      {"DATE_BUCKET",
       "date_expression, bucket_width [, origin]",
       "Gets the lower bound of the date bucket that contains a date.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#date_bucket"},
      {"ERROR", "error_message", "", ""},
      {"FARM_FINGERPRINT", "value", "", ""},
      {"FIRST",
       "expression",
       "Returns the first row in a match inside MATCH_RECOGNIZE",
       ""},
      {"FORMAT_DATETIME", "format_string, datetime_expression", "", ""},
      {"FORMAT_TIME", "format_string, time_object", "", ""},
      {"FROM_BASE32", "string_expr", "", ""},
      {"FROM_BASE64", "string_expr", "", ""},
      {"FROM_HEX", "string", "", ""},
      {"GENERATE_RANGE_ARRAY",
       "range_expression, INTERVAL step_expression [, bool_expression]",
       "Splits a range into an array of subranges.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "range-functions#generate_range_array"},
      {"GENERATE_TIMESTAMP_ARRAY",
       "start_timestamp, end_timestamp, INTERVAL step_expression date_part",
       "",
       ""},
      {"GENERATE_UUID", "", "", ""},
      {"HLL_COUNT.EXTRACT", "sketch", "", ""},
      {"HLL_COUNT.INIT", "input [, precision]", "", ""},
      {"HLL_COUNT.MERGE", "sketch", "", ""},
      {"HLL_COUNT.MERGE_PARTIAL", "sketch", "", ""},
      {"IF", "expr, true_result, else_result", "", ""},
      {"INT64",
       "json_expr",
       "Takes a JSON expression, extracts a JSON number and returns that value "
       "as a SQL INT64. If the expression is SQL NULL, the function returns "
       "SQL NULL. If the extracted JSON number has a fractional part or is "
       "outside of the INT64 domain, an error is produced.",
       ""},
      {"JSON_KEYS",
       "json_expr [, max_depth] [, mode=> 'strict' | 'lax' | 'lax recursive']",
       "Extracts an array of JSON keys from a JSON value. max_depth specifies "
       "the maximum depth of nested fields to search for keys. mode specifies "
       "how keys are extracted from arrays.",
       ""},
      {"JSON_TYPE",
       "json_expr",
       "Takes a JSON expression and returns the type of the outermost JSON "
       "value as a SQL STRING. If the expression is SQL NULL, the function "
       "returns SQL NULL. If the extracted JSON value is not a valid JSON "
       "type, an error is produced.",
       ""},
      {"JUSTIFY_DAYS", "interval_expression", "", ""},
      {"JUSTIFY_HOURS", "interval_expression", "", ""},
      {"JUSTIFY_INTERVAL", "interval_expression", "", ""},
      {"LAST",
       "expression",
       "Returns the last row in a match inside MATCH_RECOGNIZE",
       ""},
      {"LPAD", "original_value, return_length[, pattern]", "", ""},
      {"MAKE_INTERVAL", "year, month, day, hour, minute, second", "", ""},
      {"MATCH_NUMBER",
       "",
       "Returns the 1-based index of a match inside MATCH_RECOGNIZE",
       ""},
      {"MATCH_ROW_NUMBER",
       "",
       "Returns the 1-based index of a row in a match inside MATCH_RECOGNIZE",
       ""},
      {"MD5", "input", "", ""},
      {"NET.HOST", "url", "", ""},
      {"NET.IPV4_FROM_INT64", "integer_value", "", ""},
      {"NET.IPV4_TO_INT64", "addr_bin", "", ""},
      {"NET.IP_FROM_STRING", "addr_str", "", ""},
      {"NET.IP_NET_MASK", "num_output_bytes, prefix_length", "", ""},
      {"NET.IP_TO_STRING", "addr_bin", "", ""},
      {"NET.IP_TRUNC", "addr_bin, prefix_length", "", ""},
      {"NET.PUBLIC_SUFFIX", "url", "", ""},
      {"NET.REG_DOMAIN", "url", "", ""},
      {"NET.SAFE_IP_FROM_STRING", "addr_str", "", ""},
      {"NORMALIZE", "value[, normalization_mode]", "", ""},
      {"NORMALIZE_AND_CASEFOLD", "value[, normalization_mode]", "", ""},
      {"NULLIF", "expr, expr_to_match", "", ""},
      {"PARSE_BIGNUMERIC", "string_expression", "", ""},
      {"PARSE_DATE", "format_string, date_string", "", ""},
      {"PARSE_DATETIME", "format_string, datetime_string", "", ""},
      {"PARSE_NUMERIC", "string_expression", "", ""},
      {"PARSE_TIME", "format_string, time_string", "", ""},
      {"RAND", "", "", ""},
      {"RANGE_BUCKET", "point, boundaries_array", "", ""},
      {"RANGE_END",
       "range_expression",
       "Gets the upper bound of a range.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "range-functions#range_end"},
      {"RANGE_INTERSECT",
       "range_expression, range_expression",
       "Gets a segment of two ranges that intersect.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "range-functions#range_intersect"},
      {"RANGE_OVERLAPS",
       "range_expression, range_expression",
       "Checks if two ranges overlap.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "range-functions#range_overlaps"},
      {"RANGE_START",
       "range_expression",
       "Gets the lower bound of a range.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "range-functions#range_start"},
      {"REPEAT", "original_value, repetitions", "", ""},
      {"REVERSE", "value", "", ""},
      {"RPAD", "original_value, return_length[, pattern]", "", ""},
      {"SAFE_ADD", "X, Y", "", ""},
      {"SAFE_DIVIDE", "X, Y", "", ""},
      {"SAFE_MULTIPLY", "X, Y", "", ""},
      {"SAFE_NEGATE", "X", "", ""},
      {"SAFE_OFFSET", "zero_based_offset", "", ""},
      {"SAFE_ORDINAL", "one_based_offset", "", ""},
      {"SAFE_SUBTRACT", "X, Y", "", ""},
      {"SESSION_USER", "", "", ""},
      {"SHA1", "input", "", ""},
      {"SHA256", "input", "", ""},
      {"SHA512", "input", "", ""},
      {"STDDEV", "[DISTINCT]  expression", "", ""},
      {"STDDEV_POP", "[DISTINCT]  expression", "", ""},
      {"STDDEV_SAMP", "[DISTINCT]  expression", "", ""},
      {"ST_AREA", "geography_expression[, use_spheroid]", "", ""},
      {"ST_ASBINARY", "geography_expression", "", ""},
      {"ST_ASGEOJSON", "geography_expression", "", ""},
      {"ST_ASTEXT", "geography_expression", "", ""},
      {"ST_BOUNDARY", "geography_expression", "", ""},
      {"ST_CENTROID", "geography_expression", "", ""},
      {"ST_CENTROID_AGG", "geography", "", ""},
      {"ST_CLOSESTPOINT", "geography_1, geography_2[, use_spheroid]", "", ""},
      {"ST_CLUSTERDBSCAN",
       "geography_column, epsilon, minimum_geographies) OVER(...",
       "",
       ""},
      {"ST_CONTAINS", "geography_1, geography_2", "", ""},
      {"ST_CONVEXHULL", "geography_expression", "", ""},
      {"ST_COVEREDBY", "geography_1, geography_2", "", ""},
      {"ST_COVERS", "geography_1, geography_2", "", ""},
      {"ST_DIFFERENCE", "geography_1, geography_2", "", ""},
      {"ST_DIMENSION", "geography_expression", "", ""},
      {"ST_DISJOINT", "geography_1, geography_2", "", ""},
      {"ST_DUMP", "geography[, dimension]", "", ""},
      {"ST_DWITHIN",
       "geography_1, geography_2, distance[, use_spheroid]",
       "",
       ""},
      {"ST_ENDPOINT", "linestring_geography", "", ""},
      {"ST_EQUALS", "geography_1, geography_2", "", ""},
      {"ST_EXTERIORRING", "polygon_geography", "", ""},
      {"ST_GEOGFROM", "expression", "", ""},
      {"ST_GEOGFROMGEOJSON",
       "geojson_string[, make_valid => constant_expression]",
       "",
       ""},
      {"ST_GEOGFROMTEXT", "wkt_string[, oriented]", "", ""},
      {"ST_GEOGFROMWKB", "wkb_bytes_expression", "", ""},
      {"ST_GEOGPOINT", "longitude, latitude", "", ""},
      {"ST_GEOGPOINTFROMGEOHASH", "geohash", "", ""},
      {"ST_GEOHASH", "geography_expression, maxchars", "", ""},
      {"ST_INTERSECTION", "geography_1, geography_2", "", ""},
      {"ST_INTERSECTS", "geography_1, geography_2", "", ""},
      {"ST_INTERSECTSBOX", "geography, lng1, lat1, lng2, lat2", "", ""},
      {"ST_ISCOLLECTION", "geography_expression", "", ""},
      {"ST_ISEMPTY", "geography_expression", "", ""},
      {"ST_LENGTH", "geography_expression[, use_spheroid]", "", ""},
      {"ST_MAKELINE", "geography_1, geography_2", "", ""},
      {"ST_MAKEPOLYGON", "geography_expression[, array_of_geography]", "", ""},
      {"ST_MAKEPOLYGONORIENTED", "array_of_geography", "", ""},
      {"ST_MAXDISTANCE", "geography_1, geography_2[, use_spheroid]", "", ""},
      {"ST_NUMGEOMETRIES", "geography_expression", "", ""},
      {"ST_NUMPOINTS", "geography_expression", "", ""},
      {"ST_PERIMETER", "geography_expression[, use_spheroid]", "", ""},
      {"ST_POINTN", "linestring_geography, index", "", ""},
      {"ST_REGIONSTATS",
       "geography, raster_id[, band => string_1][, include => string_2][, "
       "options => json_constant]",
       "",
       ""},
      {"ST_SIMPLIFY", "geography, tolerance_meters", "", ""},
      {"ST_SNAPTOGRID", "geography_expression, grid_size", "", ""},
      {"ST_STARTPOINT", "linestring_geography", "", ""},
      {"ST_TOUCHES", "geography_1, geography_2", "", ""},
      {"ST_UNION", "geography_1, geography_2", "", ""},
      {"ST_UNION_AGG", "geography", "", ""},
      {"ST_WITHIN", "geography_1, geography_2", "", ""},
      {"ST_X", "geography_expression", "", ""},
      {"ST_Y", "geography_expression", "", ""},
      {"TIMESTAMP_BUCKET",
       "timestamp_expression, bucket_width [, origin]",
       "Gets the lower bound of the timestamp bucket that contains a "
       "timestamp.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#timestamp_bucket"},
      {"TIME", "hour, minute, second", "", ""},
      {"TIME_ADD", "time_expression, INTERVAL int64_expression part", "", ""},
      {"TIME_DIFF", "time_expression_a, time_expression_b, part", "", ""},
      {"TIME_SUB", "time_expression, INTERVAL int64_expression part", "", ""},
      {"TIME_TRUNC", "time_expression, part", "", ""},
      {"TO_BASE32", "bytes_expr", "", ""},
      {"TO_BASE64", "bytes_expr", "", ""},
      {"TO_CODE_POINTS", "value", "", ""},
      {"TO_HEX", "bytes", "", ""},
      {"TO_JSON_STRING",
       "value[, pretty_print]",
       "Takes a SQL value and returns a JSON-formatted string representation "
       "of the value. The value must be a supported BigQuery data type.",
       ""},
      {"VARIANCE", "[DISTINCT]  expression", "", ""},
      {"VAR_POP", "[DISTINCT]  expression", "", ""},
      {"VAR_SAMP", "[DISTINCT]  expression", "", ""},
  };
  return *kEntries;
}

}  // namespace completion_data_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
