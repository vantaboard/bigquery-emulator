#include "backend/sqltools/sql_completion_data_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace completion_data_internal {
const std::vector<FunctionInfoEntry>& FunctionInfoEntriesB() {
  static const std::vector<FunctionInfoEntry>* kEntries = new std::vector<FunctionInfoEntry>{
      {"LAST_DAY",
       "datetime_expression [, date_part]",
       "Returns the last day from the <datetime_expression> that contains the "
       "date. This is commonly used to return the last day of the month. You "
       "can optionally specify the <date_part> for which the last day is "
       "returned. If this parameter is not used, the default value is MONTH.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "datetime_functions#last_day"},
      {"LAST_VALUE",
       "value_expr",
       "Returns the value of the <value_expr> for the last row in the current "
       "window frame.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#last_value"},
      {"LAX_BOOL",
       "json_expr",
       "Attempts to convert a JSON value to a SQL BOOL value.",
       ""},
      {"LAX_FLOAT64",
       "json_expr",
       "Attempts to convert a JSON expression to a SQL FLOAT64 value.",
       ""},
      {"LAX_INT64",
       "json_expr",
       "Attempts to convert a JSON expression to a SQL INT64 value.",
       ""},
      {"LAX_STRING",
       "json_expr",
       "Attempts to convert a JSON expression to a SQL STRING value.",
       ""},
      {"LEAD",
       "value_expr [, offset [, default_expr] ]",
       "Returns the value of the <value_expr> on a subsequent row. Changing "
       "the <offset> value changes which subsequent row is returned; the "
       "default value is 1, indicating the next row in the window frame. An "
       "error occurs if <offset> is NULL or a negative value.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#lead"},
      {"LEAST",
       "numeric_expr1, numeric_expr2, ...",
       "Returns NULL if any of the inputs is NULL. Returns NaN if any of the "
       "inputs is NaN. Otherwise, returns the smallest value among "
       "<numeric_expr1>, <numeric_expr2>, ... according to the > comparison.",
       ""},
      {"LEFT",
       "value, length",
       "Returns the specified number of leftmost characters from the input "
       "STRING, or the specified number of leftmost bytes from the input "
       "BYTES.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#left"},
      {"LENGTH",
       "value",
       "Returns the length of the value. The returned value is in characters "
       "for STRING arguments and in bytes for the BYTES argument.",
       ""},
      {"LN",
       "numeric_expr",
       "Computes the natural logarithm of <numeric_expr>. Generates an error "
       "if <numeric_expr> is less than or equal to zero. If <numeric_expr> is "
       "+inf, then this function returns +inf.",
       ""},
      {"LOG",
       "numeric_expr [, base]",
       "If only <numeric_expr> is present, LOG is a synonym of LN. If <base> "
       "is also present, LOG computes the logarithm of <numeric_expr> to "
       "<base>.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#log"},
      {"LOG10",
       "numeric_expr",
       "Similar to LOG, but computes logarithm to base 10.",
       ""},
      {"LOGICAL_AND",
       "expr",
       "Returns the logical AND of all non-NULL expressions. Returns NULL if "
       "there are zero input rows or <expr> evaluates to NULL for all rows.",
       ""},
      {"LOGICAL_OR",
       "expr",
       "Returns the logical OR of all non-NULL expressions. Returns NULL if "
       "there are zero input rows or <expr> evaluates to NULL for all rows.",
       ""},
      {"LOWER",
       "value",
       "For STRING arguments, returns the original string with all alphabetic "
       "characters in lowercase. Mapping between lowercase and uppercase is "
       "done according to the Unicode Character Database without taking into "
       "account language-specific mappings.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#lower"},
      {"LTRIM",
       "value1 [, value2]",
       "Identical to TRIM, but only removes leading characters.",
       ""},
      {"MAX",
       "expr",
       "Returns the maximum value of non-NULL expressions. Returns NULL if "
       "there are zero input rows or <expr> evaluates to NULL for all rows. "
       "Returns NaN if the input contains a NaN.",
       ""},
      {"MAX_BY",
       "expr1, expr2",
       "Returns the value of expr1 for which the value of expr2 is maximized. "
       "Returns an arbitrary value of expr1 if multiple rows match the "
       "maximized expr2. Returns NULL if expr1 evaluates to NULL for all "
       "matched rows or expr2 contains a NaN.",
       ""},
      {"MIN",
       "expr",
       "Returns the minimum value of non-NULL expressions. Returns NULL if "
       "there are zero input rows or <expr> evaluates to NULL for all rows. "
       "Returns NaN if the input contains a NaN.",
       ""},
      {"MIN_BY",
       "expr1, expr2",
       "Returns the value of expr1 for which the value of expr2 is minimized. "
       "Returns an arbitrary value of expr1 if multiple rows match the "
       "minimized expr2. Returns NULL if expr1 evaluates to NULL for all "
       "matched rows or expr2 contains a NaN.",
       ""},
      {"MOD",
       "numeric_expr1, numeric_expr2",
       "Modulo function: returns the remainder of the division of "
       "<numeric_expr1> by <numeric_expr2>. Returned value has the same sign "
       "as <numeric_expr1>. An error is generated if <numeric_expr2> is 0. See "
       "the table below for possible result types.",
       ""},
      {"NTH_VALUE",
       "value_expr, constant_integer_expr",
       "Returns the value of <value_expr> at the Nth row of the current window "
       "frame, where Nth is defined by <constant_integer_expr>. Returns NULL "
       "if there is no such row.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#nth_value"},
      {"NTILE",
       "constant_integer_expr",
       "This function divides the rows into <constant_integer_expr> buckets "
       "based on row ordering and returns the 1-based bucket number that is "
       "assigned to each row. The number of rows in the buckets can differ by "
       "at most 1. The remainder values (the remainder of number of rows "
       "divided by buckets) are distributed one for each bucket, starting with "
       "bucket 1. If <constant_integer_expr> evaluates to NULL, 0 or negative, "
       "an error is provided.",
       ""},
      {"OCTET_LENGTH",
       "input",
       "An alias of BYTE_LENGTH. Returns the length of the <value> in bytes, "
       "regardless of whether the type of the <value> is STRING or BYTES.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#octet_length"},
      {"OFFSET",
       "zero_based_offset",
       "Accesses an ARRAY element by position and returns the element. OFFSET "
       "means that the numbering starts at zero, ORDINAL means that the "
       "numbering starts at one.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#offset-and-ordinal"},
      {"ORDINAL",
       "one_based_offset",
       "Accesses an ARRAY element by position and returns the element. OFFSET "
       "means that the numbering starts at zero, ORDINAL means that the "
       "numbering starts at one.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#offset-and-ordinal"},
      {"PARSE_TIMESTAMP",
       "format_string, string [, time_zone]",
       "Uses a <format_string> and a string representation of a timestamp to "
       "return a TIMESTAMP object.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#parse_timestamp"},
      {"PERCENT_RANK",
       "",
       "Return the percentile rank of a row defined as (RK-1)/(NR-1), where RK "
       "is the RANK of the row and NR is the number of rows in the partition. "
       "Returns 0 if NR=1.",
       ""},
      {"POW",
       "numeric_expr1, numeric_expr2",
       "Returns the value of <numeric_expr1> raised to the power of "
       "<numeric_expr1>. If the result underflows and is not representable, "
       "then the function returns a value of zero.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#pow"},
      {"POWER", "numeric_expr, numeric_expr", "Synonym of POW().", ""},
      {"RANK",
       "",
       "Returns the ordinal (1-based) rank of each row within the ordered "
       "partition. All peer rows receive the same rank value. The next row or "
       "set of peer rows receives a rank value which increments by the number "
       "of peers with the previous rank value, instead of DENSE_RANK, which "
       "always increments by 1.",
       ""},
      {"REGEXP_CONTAINS",
       "value, regex",
       "Returns TRUE if <value> is a partial match for the regular expression, "
       "<regex>. You can search for a full match by using ^ (beginning of "
       "text) and $ (end of text).",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#regexp_contains"},
      {"REGEXP_EXTRACT",
       "value, regex [, position[, occurrence]]",
       "Returns the first substring in <value> that matches the regular "
       "expression, <regex>. Returns NULL if there is no match. If <position> "
       "is specified, the search starts at this position in value, otherwise "
       "it starts at the beginning of value. If <occurrence> is specified, the "
       "search returns a specific occurrence of the regexp in value, otherwise "
       "returns the first match.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#regexp_extract"},
      {"REGEXP_EXTRACT_ALL",
       "value, regex",
       "Returns an array of all substrings of <value> that match the regular "
       "expression, <regex>.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#regexp_extract_all"},
      {"REGEXP_INSTR",
       "value, regex [, position[, occurrence, [return_pos_after_match]]]",
       "Returns the position of the specified <occurrence> of the <regex> "
       "pattern in <value>. <return_pos_after_match> can be 0 (default, "
       "returns the beginning position of the match) or 1 (returns the first "
       "position following the end of the match)",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#regexp_instr"},
      {"REGEXP_REPLACE",
       "value, regex, replacement",
       "Returns a STRING where all substrings of <value> that match regular "
       "expression <regex> are replaced with replacement.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#regexp_replace"},
      {"REGEXP_SUBSTR",
       "value, regexp [,position[, occurrence]]",
       "Extracts a substring from <value> that matches a regular expression "
       "specified by <regexp>. Optionally, search starts at <position> and "
       "looks for the specified <occurrence>.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#regexp_substr"},
      {"REPLACE",
       "original_value, from_value, to_value",
       "Replaces all occurrences of <from_value> with <to_value> in "
       "<original_value>. If <from_value> is empty, no replacement is made.",
       ""},
      {"RIGHT",
       "value, length",
       "Returns the specified number of rightmost characters from the input "
       "STRING, or the specified number of rightmost bytes from the input "
       "BYTES.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#right"},
      {"ROUND",
       "numeric_expr [, decimal_places]",
       "If only <numeric_expr> is present, ROUND rounds <numeric_expr> to the "
       "nearest integer. If <decimal_places> is present, ROUND rounds "
       "<numeric_expr> to <decimal_places> decimal places after the decimal "
       "point. If <decimal_places> is negative, ROUND will round off digits to "
       "the left of the decimal point. Rounds halfway cases away from zero. "
       "Generates an error if overflow occurs.",
       ""},
      {"ROW_NUMBER",
       "",
       "Does not require the ORDER BY clause. Returns the sequential row "
       "ordinal (1-based) of each row for each ordered partition. If the ORDER "
       "BY clause is unspecified then the result is non-deterministic.",
       ""},
      {"RTRIM",
       "value1 [, value2]",
       "Identical to TRIM, but only removes trailing characters.",
       ""},
      {"SAFE_CAST",
       "expr AS typename",
       "SAFE_CAST is identical to CAST, except it returns NULL instead of "
       "raising an error.",
       ""},
      {"SAFE_CONVERT_BYTES_TO_STRING",
       "value",
       "Converts a sequence of bytes to a string. Any invalid UTF-8 characters "
       "are replaced with the Unicode replacement character, U+FFFD.",
       ""},
      {"SIGN",
       "numeric_expr",
       "Returns -1, 0, or +1 for negative, zero and positive arguments "
       "respectively. For floating point arguments, this function does not "
       "distinguish between positive and negative zero. Returns NaN for a NaN "
       "argument.",
       ""},
      {"SIN",
       "numeric_expr",
       "Computes the sine of <numeric_expr>. Never fails.",
       ""},
      {"SINH",
       "numeric_expr",
       "Computes the hyperbolic sine of <numeric_expr>. Generates an error if "
       "an overflow occurs.",
       ""},
      {"SOUNDEX",
       "input",
       "Returns a string that represents the Soundex code for the <input>. "
       "Soundex is a phonetic algorithm for indexing names by sound, as "
       "pronounced in English.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#soundex"},
      {"SPLIT",
       "value [, delimiter]",
       "Splits <value> using the <delimiter> argument.",
       ""},
      {"SQRT",
       "numeric_expr",
       "Computes the square root of <numeric_expr>. Generates an error if "
       "<numeric_expr> is less than 0. Returns +inf if <numeric_expr> is +inf.",
       ""},
      {"STARTS_WITH",
       "value1, value2",
       "Returns TRUE if <value2> is a prefix of <value1>.",
       ""},
      {"STRING",
       "timestamp_expr [, time_zone]",
       "Converts a <timestamp_expr> to a STRING data type. Supports an "
       "optional parameter to specify a timezone. ",
       ""},
      {"STRING_AGG",
       "expr, [, delimiter]",
       "Returns a value (either STRING or BYTES) obtained by concatenating "
       "non-null values.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#string_agg"},
      {"STRPOS",
       "value1, value2",
       "Returns the 1-based index of the first occurrence of <value2> inside "
       "<value1>. Returns 0 if <value2> is not found.",
       ""},
      {"ST_DISTANCE",
       "geography_1, geography_2[, use_spheroid]",
       "Returns the shortest distance in meters between two non-empty "
       "GEOGRAPHYs.  If either of the input GEOGRAPHYs is empty, ST_DISTANCE "
       "returns NULL. The optional use_spheroid parameter determines how this "
       "function measures distance. If use_spheroid is FALSE, the function "
       "measures distance on the surface of a perfect sphere.  The "
       "use_spheroid parameter currently only supportsthe value FALSE. The "
       "default value of use_spheroid is FALSE.",
       ""},
      {"SUBSTR",
       "value, position [, length]",
       "Returns a substring of the supplied value. The <position> argument is "
       "an integer specifying the starting position of the substring, with "
       "position = 1 indicating the first character or byte. The <length> "
       "argument is the maximum number of characters for STRING arguments, or "
       "bytes for BYTES arguments.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#substr"},
      {"SUBSTRING",
       "value, position [, length]",
       "An alias of substr. Returns a substring of the supplied <value> "
       "(STRING or BYTES). The <position> argument is an integer specifying "
       "the starting position of the substring, with position = 1 indicating "
       "the first character or byte. The <length> argument is the maximum "
       "number of characters for STRING arguments, or bytes for BYTES "
       "arguments.",
       ""},
      {"SUM",
       "expr",
       "Returns the sum of non-null values.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#sum"},
  };
  return *kEntries;
}

}  // namespace completion_data_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
