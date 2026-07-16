#include "backend/sqltools/sql_completion_data_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace completion_data_internal {
const std::vector<FunctionInfoEntry>& FunctionInfoEntriesA() {
  static const std::vector<FunctionInfoEntry>* kEntries = new std::vector<
      FunctionInfoEntry>{
      {"ABS",
       "numeric_expr",
       "Computes absolute value. Returns an error if theargument isan integer "
       "and the output value cannot be represented as the same type; this "
       "happens only for the largest negative input value, which has no "
       "positive representation. Returns +inf for a +/-inf argument.",
       ""},
      {"ACOS",
       "numeric_expr",
       "Computes the principal value of the arc cosine of <numeric_expr>. The "
       "return value is in the range [0,]. Generates an error if "
       "<numeric_expr> is a finite value outside of range [-1, 1].",
       ""},
      {"ACOSH",
       "numeric_expr",
       "Computes the inverse hyperbolic cosine of <numeric_expr>. Generates an "
       "error if <numeric_expr> is a finite value less than 1.",
       ""},
      {"ANY_VALUE",
       "expr",
       "Returns any value from the input or NULL if there are zero input rows. "
       "The value returned is non-deterministic, which means you might receive "
       "a different result each time you use this function.",
       ""},
      {"APPROX_COUNT_DISTINCT",
       "expr",
       "Returns the approximate result for COUNT(DISTINCT <expr>). The value "
       "returned is a statistical estimate\\u2014not necessarily the actual "
       "value.\\n\\nThis function is less accurate than COUNT(DISTINCT "
       "<expr>), but performs better on huge input.",
       ""},
      {"APPROX_QUANTILES",
       "expr, number",
       "Returns the approximate boundaries for a group of <expr> values, where "
       "<number> represents the number of quantiles to create. This function "
       "returns an array of <number> + 1 elements, where the first element is "
       "the approximate minimum and the last element is the approximate "
       "maximum.",
       ""},
      {"APPROX_TOP_COUNT",
       "expr, number",
       "Returns the approximate top elements of <expr>. The <number> parameter "
       "specifies the number of elements returned.",
       ""},
      {"ARRAY_AGG", "expr", "Returns an ARRAY of <expr> values.", ""},
      {"ARRAY_CONCAT",
       "array_expr1 [, array_expr_n]",
       "Concatenates one or more arrays with the same element type into a "
       "single array.",
       ""},
      {"ARRAY_CONCAT_AGG",
       "expr",
       "Concatenates elements from <expr> of type ARRAY, returning a single "
       "ARRAY as a result. This function ignores NULL input arrays, but "
       "respects the NULL elements in non-NULL input arrays (an error is "
       "raised, however, if an array in the final query result contains a NULL "
       "element).",
       ""},
      {"ARRAY_LENGTH",
       "array_expr",
       "Returns the size of the array. Returns 0 for an empty array. Returns "
       "NULL if the <array_expr> is NULL.",
       ""},
      {"ARRAY_TO_STRING",
       "array_expr, delimiter [, null_text]",
       "Returns a concatenation of the elements in <array_expr> as a STRING. "
       "The value for <array_expr> can either be an array of STRING or BYTES "
       "data types.",
       ""},
      {"ASCII",
       "value",
       "Returns the ASCII code for the first character of the input (STRING "
       "type), or for the first byte of the input (BYTES type). Function "
       "returns 0 if the string is empty, or the first character/byte's ASCII "
       "code is 0.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#ascii"},
      {"ASIN",
       "numeric_expr",
       "Computes the principal value of the arc sine of <numeric_expr>. The "
       "return value is in the range [-\\u03c0/2,\\u03c0/2]. Generates an "
       "error if X is a finite value outside of range [-1, 1].",
       ""},
      {"ASINH",
       "numeric_expr",
       "Computes the inverse hyperbolic sine of <numeric_expr>. Does not fail.",
       ""},
      {"ATAN",
       "numeric_expr",
       "Computes the principal value of the arc tangent of <numeric_expr>. The "
       "return value is in the range [-\\u03c0/2,\\u03c0/2]. Does not fail.",
       ""},
      {"ATANH",
       "numeric_expr",
       "Computes the inverse hyperbolic tangent of <numeric_expr>. Generates "
       "an error if the absolute value of <numeric_expr> is greater or equal "
       "1.",
       ""},
      {"ATAN2",
       "numeric_expr, numeric_expr",
       "Calculates the principal value of the arc tangent of Y/X using the "
       "signs of the two arguments to determine the quadrant. The return value "
       "is in the range [-\\u03c0,\\u03c0]. ",
       ""},
      {"AVG",
       "expr",
       "Returns the average of non-NULL input values, or NaN if the input "
       "contains a NaN.",
       ""},
      {"BIT_AND",
       "expr",
       "Performs a bitwise AND operation on expression and returns the result.",
       ""},
      {"BIT_OR",
       "expr",
       "Performs a bitwise OR operation on expression and returns the result.",
       ""},
      {"BIT_XOR",
       "expr",
       "Performs a bitwise XOR operation on <expr> and returns the result.",
       ""},
      {"BYTE_LENGTH",
       "value",
       "Returns the length of the <value> in bytes, regardless of whether the "
       "type of the value is STRING or BYTES.",
       ""},
      {"CAST",
       "expr AS typename",
       "Converts <expr> into a variable of type <typename>.",
       ""},
      {"CEIL",
       "numeric_expr",
       "Returns the smallest integral value (with FLOAT64 type) that is not "
       "less than <numeric_expr>.",
       ""},
      {"CEILING", "numeric_expr", "Synonym of CEIL.", ""},
      {"CHAR_LENGTH",
       "value",
       "Returns the length of the STRING in characters.",
       ""},
      {"CHARACTER_LENGTH", "value", "Synonym for CHAR_LENGTH.", ""},
      {"CHR",
       "value",
       "Returns the character that matches the input Unicode code point. Each "
       "valid code point should fall within the range of [0, 0xD7FF] and "
       "[0xE000, 0x10FFFF]. If an invalid Unicode code point is specified, an "
       "error is returned.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#chr"},
      {"CONCAT",
       "value1 [, ...]",
       "Concatenates one or more values into a single result.",
       ""},
      {"COS",
       "numeric_expr",
       "Computes cosine of <numeric_expr>. Never fails.",
       ""},
      {"COSH",
       "numeric_expr",
       "Computes the hyperbolic cosine of <numeric_expr>. Generates an error "
       "if an overflow occurs.",
       ""},
      {"COUNT",
       "expr",
       "Returns the total number of values (NULL and non-NULL for <*>, "
       "non-NULL for <field>) in the scope of the function. With DISTINCT "
       "returns the number of distinct values for the specified field.",
       ""},
      {"COUNTIF",
       "expr",
       "Returns the count of TRUE values for <expr>. Returns 0 if there are "
       "zero input rows or <expr> evaluates to FALSE for all rows.",
       ""},
      {"CUME_DIST",
       "",
       "Return the relative rank of a row defined as NP/NR. NP is defined to "
       "be the number of rows that either precede or are peers with the "
       "current row. NR is the number of rows in the partition.",
       ""},
      {"CURRENT_DATE",
       "[time_zone]",
       "Returns the current date as of the specified or default timezone.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#current_date"},
      {"CURRENT_TIMESTAMP",
       "",
       "Parentheses are optional. This function handles leap seconds by "
       "smearing them across a window of 20 hours around the inserted leap "
       "second. CURRENT_TIMESTAMP() produces a TIMESTAMP value that is "
       "continuous, non-ambiguous, has exactly 60 seconds per minute and does "
       "not repeat values over the leap second.",
       ""},
      {"DATE",
       "timestamp_expr [, time_zone]",
       "Converts a <timestamp_expr> to a DATE data type. It supports an "
       "optional parameter to specify a timezone. If no timezone is specified, "
       "the default timezone, UTC, is used.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#date"},
      {"DATE_ADD",
       "date_expr, INTERVAL int64_expr date_part",
       "Adds a specified time interval to a DATE.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#date_add"},
      {"DATE_DIFF",
       "date_expr, date_expr, date_part",
       "Returns the number of <date_part> boundaries between the two "
       "<date_expr>s. If the first date occurs before the second date, then "
       "the result is non-positive.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#date_diff"},
      {"DATE_FROM_UNIX_DATE",
       "INT64_expr",
       "Interprets <INT64_expr> as the number of days since 1970-01-01.",
       ""},
      {"DATE_SUB",
       "date_expr, INTERVAL int64_expr date_part",
       "Subtracts a specified time interval from a DATE.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#date_sub"},
      {"DATE_TRUNC",
       "date_expr, date_part",
       "Truncates the date to the specified granularity.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#date_trunc"},
      {"DENSE_RANK",
       "",
       "Returns the ordinal (1-based) rank of each row within the window "
       "partition. All peer rows receive the same rank value, and the "
       "subsequent rank value is incremented by one.",
       ""},
      {"DIV",
       "numeric_expr, numeric_expr",
       "Returns the result of integer division of X by Y. Division by zero "
       "returns an error. Division by -1 may overflow. See the table below for "
       "possible result types.",
       ""},
      {"ENDS_WITH",
       "value1, value2",
       "Takes two values. Returns TRUE if <value2> is a suffix of <value1>.",
       ""},
      {"EXP",
       "numeric_expr",
       "Computes e to the power of <numeric_expr>, also called the natural "
       "exponential function. If the result underflows, this function returns "
       "a zero. Generates an error if the result overflows. If <numeric_expr> "
       "is +/-inf, then this function returns +inf or 0.",
       ""},
      {"EXTRACT",
       "datetime_part FROM timestamp_expr [AT TIME ZONE tz_spec]",
       "Returns the value corresponding to the specified date part.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#extract"},
      {"FIRST_VALUE",
       "value_expr",
       "Returns the value of the <value_expr> for the first row in the current "
       "window frame.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#first_value"},
      {"FLOOR",
       "numeric_expr",
       "Returns the largest integral value (with FLOAT64 type) that is not "
       "greater than <numeric_expr>.",
       ""},
      {"FORMAT",
       "format_string, format_specifier1 [, ...]",
       "BigQuery supports a FORMAT() function for formatting strings. This "
       "function is similar to the C printf function. It produces a STRING "
       "from a format string that contains zero or more format specifiers, "
       "along with a variable length list of additional arguments that matches "
       "the format specifiers.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#format"},
      {"FORMAT_DATE",
       "format_string, date",
       "Formats the date_expr according to the specified <format_string>.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#format_date"},
      {"FORMAT_TIMESTAMP",
       "format_string, timestamp [, <time_zone>]",
       "Formats a timestamp according to the specified <format_string>.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#format_timestamp"},
      {"GENERATE_ARRAY",
       "start_expression, end_expression [, step_expression]",
       "Returns an array of values. The <start_expression> and "
       "<end_expression> parameters determine the inclusive start and end of "
       "the array.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#generate_array"},
      {"GENERATE_DATE_ARRAY",
       "start_date, end_date[, INTERVAL step_expression "
       "DAY|WEEK|MONTH|QUARTER|YEAR]",
       "Returns an array of dates. The <start_date> and <end_date> parameters "
       "determine the inclusive start and end of the array.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#generate_date_array"},
      {"GREATEST",
       "numeric_expr1, numeric_expr2, ...",
       "Returns NULL if any of the inputs is NULL. Otherwise, returns NaN if "
       "any of the inputs is NaN. Otherwise, returns the largest value among "
       "<numeric_expr1>, <numeric_expr2>... according to the < comparison.",
       ""},
      {"GROUPING",
       "group_by_expression",
       "Returns 0 or 1. Returns 1 if the group_by_expression argument is being "
       "aggregated, or 0 if it is not.",
       ""},
      {"IEEE_DIVIDE",
       "numeric_expr1, numeric_expr2",
       "Divides <numeric_expr1> by <numeric_expr2>; this function never fails. "
       "Returns FLOAT64. Unlike the division operator (/), this function does "
       "not generate errors for division by zero or overflow.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#ieee_divide"},
      {"IFNULL",
       "expr, null_result",
       "If expr is NULL, return null_result. Otherwise, return expr. If expr "
       "is not NULL, null_result is not evaluated. expr and null_result can be "
       "any type and must be implicitly coercible to a common supertype. "
       "Synonym for COALESCE(expr, null_result).",
       ""},
      {"INITCAP",
       "input[, delimiters]",
       "Transform the <input> string with the first character in each word in "
       "uppercase, and all other characters in lowercase. Non-alphabetic "
       "characters will remain the same . Mapping between lowercase and "
       "uppercase is done according to the Unicode Character Database without "
       "taking into account language-specific mappings.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#initcap"},
      {"INSTR",
       "source_value, search_value [, position[, occurrence ]]",
       "Searches the occurrences of <search_value> in the <source_value>, "
       "starting from <position>, returns the start position of the search "
       "hit.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "string_functions#instr"},
      {"IS_INF",
       "numeric_expr",
       "Returns TRUE if the value is positive or negative infinity. Returns "
       "NULL for NULL inputs.",
       ""},
      {"IS_NAN",
       "numeric_expr",
       "Returns TRUE if the value is a NaN value. Returns NULL for NULL "
       "inputs.",
       ""},
      {"JSON_ARRAY",
       "[value][, ...]",
       "Creates a JSON array from zero or more SQL values.",
       ""},
      {"JSON_REMOVE", "json_expr, json_path[, ...]", "Removes JSON data.", ""},
      {"JSON_SET",
       "json_expr, json_path, value[, json_path, value][...]",
       "Inserts or replaces existing JSON data.",
       ""},
      {"KLL_QUANTILES.EXTRACT_INT64",
       "sketch, num_quantiles",
       "Gets a selected number of quantiles from an INT64-initialized KLL "
       "sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesextract_int64"},
      {"KLL_QUANTILES.EXTRACT_FLOAT64",
       "sketch, num_quantiles",
       "Gets a selected number of quantiles from a FLOAT64-initialized KLL "
       "sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesextract_double"},
      {"KLL_QUANTILES.EXTRACT_POINT_INT64",
       "sketch, phi",
       "Gets a specific quantile from an INT64-initialized KLL sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesextract_point_int64"},
      {"KLL_QUANTILES.EXTRACT_POINT_FLOAT64",
       "sketch, phi",
       "Gets a specific quantile from a FLOAT64-initialized KLL sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesextract_point_double"},
      {"KLL_QUANTILES.INIT_INT64",
       "input[, precision [, weight => input_weight ]",
       "Aggregates values into an INT64-initialized KLL sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesinit_int64"},
      {"KLL_QUANTILES.INIT_FLOAT64",
       "input[, precision [, weight => input_weight ]",
       "Aggregates values into a FLOAT64-initialized KLL sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesinit_double"},
      {"KLL_QUANTILES.MERGE_INT64",
       "sketch, num_quantiles",
       "Merges INT64-initialized KLL sketches into a new sketch, and then gets "
       "the quantiles from the new sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesmerge_int64"},
      {"KLL_QUANTILES.MERGE_FLOAT64",
       "sketch, num_quantiles",
       "Merges FLOAT64-initialized KLL sketches into a new sketch, and then "
       "gets the quantiles from the new sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesmerge_double"},
      {"KLL_QUANTILES.MERGE_PARTIAL",
       "sketch",
       "Merges KLL sketches of the same underlying type into a new sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesmerge_partial"},
      {"KLL_QUANTILES.MERGE_POINT_INT64",
       "sketch, phi",
       "Merges INT64-initialized KLL sketches into a new sketch, and then gets "
       "a specific quantile from the new sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesmerge_point_int64"},
      {"KLL_QUANTILES.MERGE_POINT_FLOAT64",
       "sketch, phi",
       "Merges FLOAT64-initialized KLL sketches into a new sketch, and then "
       "gets a specific quantile from the new sketch.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "kll_functions#kll_quantilesmerge_point_double"},
      {"LAG",
       "value_expr [, offset [, default_expr] ]",
       "Returns the value of the <value_expr> on a preceding row. Changing the "
       "<offset> value changes which preceding row is returned; the default "
       "value is 1, indicating the previous row in the window frame. An error "
       "occurs if <offset> is NULL or a negative value.",
       "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
       "functions-and-operators#lag"},
  };
  return *kEntries;
}

}  // namespace completion_data_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
