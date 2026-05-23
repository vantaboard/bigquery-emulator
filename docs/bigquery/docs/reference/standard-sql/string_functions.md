GoogleSQL for BigQuery supports string functions.
These string functions work on two different values:
`STRING` and `BYTES` data types. `STRING` values must be well-formed UTF-8.

Functions that return position values, such as [STRPOS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos),
encode those positions as `INT64`. The value `1`
refers to the first character (or byte), `2` refers to the second, and so on.
The value `0` indicates an invalid position. When working on `STRING` types, the
returned positions refer to character positions.

All string comparisons are done byte-by-byte, without regard to Unicode
canonical equivalence.

## Function list

| Name | Summary |
|---|---|
| [`ARRAY_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_to_string) | Produces a concatenation of the elements in an array as a `STRING` value. For more information, see [Array functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions). |
| [`ASCII`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ascii) | Gets the ASCII code for the first character or byte in a `STRING` or `BYTES` value. |
| [`BYTE_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length) | Gets the number of `BYTES` in a `STRING` or `BYTES` value. |
| [`CHAR_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length) | Gets the number of characters in a `STRING` value. |
| [`CHARACTER_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#character_length) | Synonym for `CHAR_LENGTH`. |
| [`CHR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#chr) | Converts a Unicode code point to a character. |
| [`CODE_POINTS_TO_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_bytes) | Converts an array of extended ASCII code points to a `BYTES` value. |
| [`CODE_POINTS_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string) | Converts an array of extended ASCII code points to a `STRING` value. |
| [`COLLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#collate) | Combines a `STRING` value and a collation specification into a collation specification-supported `STRING` value. |
| [`CONCAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat) | Concatenates one or more `STRING` or `BYTES` values into a single result. |
| [`CONTAINS_SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#contains_substr) | Performs a normalized, case-insensitive search to see if a value exists as a substring in an expression. |
| [`EDIT_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#edit_distance) | Computes the Levenshtein distance between two `STRING` or `BYTES` values. |
| [`ENDS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ends_with) | Checks if a `STRING` or `BYTES` value is the suffix of another value. |
| [`FORMAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_string) | Formats data and produces the results as a `STRING` value. |
| [`FROM_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base32) | Converts a base32-encoded `STRING` value into a `BYTES` value. |
| [`FROM_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base64) | Converts a base64-encoded `STRING` value into a `BYTES` value. |
| [`FROM_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_hex) | Converts a hexadecimal-encoded `STRING` value into a `BYTES` value. |
| [`INITCAP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#initcap) | Formats a `STRING` as proper case, which means that the first character in each word is uppercase and all other characters are lowercase. |
| [`INSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#instr) | Finds the position of a subvalue inside another value, optionally starting the search at a given offset or occurrence. |
| [`LAX_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_string) | Attempts to convert a JSON value to a SQL `STRING` value. For more information, see [JSON functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions). |
| [`LEFT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#left) | Gets the specified leftmost portion from a `STRING` or `BYTES` value. |
| [`LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length) | Gets the length of a `STRING` or `BYTES` value. |
| [`LOWER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower) | Formats alphabetic characters in a `STRING` value as lowercase. <br /> Formats ASCII characters in a `BYTES` value as lowercase. |
| [`LPAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lpad) | Prepends a `STRING` or `BYTES` value with a pattern. |
| [`LTRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim) | Identical to the `TRIM` function, but only removes leading characters. |
| [`NORMALIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize) | Case-sensitively normalizes the characters in a `STRING` value. |
| [`NORMALIZE_AND_CASEFOLD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize_and_casefold) | Case-insensitively normalizes the characters in a `STRING` value. |
| [`OCTET_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#octet_length) | Alias for `BYTE_LENGTH`. |
| [`REGEXP_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains) | Checks if a value is a partial match for a regular expression. |
| [`REGEXP_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract) | Produces a substring that matches a regular expression. |
| [`REGEXP_EXTRACT_ALL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all) | Produces an array of all substrings that match a regular expression. |
| [`REGEXP_INSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_instr) | Finds the position of a regular expression match in a value, optionally starting the search at a given offset or occurrence. |
| [`REGEXP_REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace) | Produces a `STRING` value where all substrings that match a regular expression are replaced with a specified value. |
| [`REGEXP_SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_substr) | Synonym for `REGEXP_EXTRACT`. |
| [`REPEAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat) | Produces a `STRING` or `BYTES` value that consists of an original value, repeated. |
| [`REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace) | Replaces all occurrences of a pattern with another pattern in a `STRING` or `BYTES` value. |
| [`REVERSE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#reverse) | Reverses a `STRING` or `BYTES` value. |
| [`RIGHT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#right) | Gets the specified rightmost portion from a `STRING` or `BYTES` value. |
| [`RPAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rpad) | Appends a `STRING` or `BYTES` value with a pattern. |
| [`RTRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim) | Identical to the `TRIM` function, but only removes trailing characters. |
| [`SAFE_CONVERT_BYTES_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string) | Converts a `BYTES` value to a `STRING` value and replace any invalid UTF-8 characters with the Unicode replacement character, `U+FFFD`. |
| [`SOUNDEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#soundex) | Gets the Soundex codes for words in a `STRING` value. |
| [`SPLIT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split) | Splits a `STRING` or `BYTES` value, using a delimiter. |
| [`STARTS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#starts_with) | Checks if a `STRING` or `BYTES` value is a prefix of another value. |
| [`STRING` (JSON)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#string_for_json) | Converts a JSON string to a SQL `STRING` value. For more information, see [JSON functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions). |
| [`STRING` (Timestamp)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string) | Converts a `TIMESTAMP` value to a `STRING` value. For more information, see [Timestamp functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions). |
| [`STRING_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg) | Concatenates non-`NULL` `STRING` or `BYTES` values. For more information, see [Aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions). |
| [`STRPOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos) | Finds the position of the first occurrence of a subvalue inside another value. |
| [`SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr) | Gets a portion of a `STRING` or `BYTES` value. |
| [`SUBSTRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substring) | Alias for `SUBSTR` |
| [`TO_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base32) | Converts a `BYTES` value to a base32-encoded `STRING` value. |
| [`TO_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base64) | Converts a `BYTES` value to a base64-encoded `STRING` value. |
| [`TO_CODE_POINTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points) | Converts a `STRING` or `BYTES` value into an array of extended ASCII code points. |
| [`TO_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex) | Converts a `BYTES` value to a hexadecimal `STRING` value. |
| [`TRANSLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#translate) | Within a value, replaces each source character with the corresponding target character. |
| [`TRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim) | Removes the specified leading and trailing Unicode code points or bytes from a `STRING` or `BYTES` value. |
| [`UNICODE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#unicode) | Gets the Unicode code point for the first character in a value. |
| [`UPPER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper) | Formats alphabetic characters in a `STRING` value as uppercase. <br /> Formats ASCII characters in a `BYTES` value as uppercase. |

## `ASCII`

    ASCII(value)

**Description**

Returns the ASCII code for the first character or byte in `value`. Returns
`0` if `value` is empty or the ASCII code is `0` for the first character
or byte.

**Return type**

`INT64`

**Examples**

    SELECT ASCII('abcd') as A, ASCII('a') as B, ASCII('') as C, ASCII(NULL) as D;

    /*---+---+---+---+
     | A     | B     | C     | D     |
     +---+---+---+---+
     | 97    | 97    | 0     | NULL  |
     +---+---+---+---*/

## `BYTE_LENGTH`

    BYTE_LENGTH(value)

**Description**

Gets the number of `BYTES` in a `STRING` or `BYTES` value,
regardless of whether the value is a `STRING` or `BYTES` type.

**Return type**

`INT64`

**Examples**

    SELECT BYTE_LENGTH('абвгд') AS string_example;

    /*---+
     | string_example |
     +---+
     | 10             |
     +---*/

    SELECT BYTE_LENGTH(b'абвгд') AS bytes_example;

    /*---+
     | bytes_example  |
     +---+
     | 10             |
     +---*/

## `CHAR_LENGTH`

    CHAR_LENGTH(value)

**Description**

Gets the number of characters in a `STRING` value.

**Return type**

`INT64`

**Examples**

    SELECT CHAR_LENGTH('абвгд') AS char_length;

    /*---+
     | char_length |
     +---+
     | 5           |
     +--- */

## `CHARACTER_LENGTH`

    CHARACTER_LENGTH(value)

**Description**

Synonym for [CHAR_LENGTH](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length).

**Return type**

`INT64`

**Examples**

    SELECT
      'абвгд' AS characters,
      CHARACTER_LENGTH('абвгд') AS char_length_example

    /*---+---+
     | characters | char_length_example |
     +---+---+
     | абвгд      |                   5 |
     +---+---*/

## `CHR`

    CHR(value)

**Description**

Takes a Unicode [code point](https://en.wikipedia.org/wiki/Code_point) and returns
the character that matches the code point. Each valid code point should fall
within the range of \[0, 0xD7FF\] and \[0xE000, 0x10FFFF\]. Returns an empty string
if the code point is `0`. If an invalid Unicode code point is specified, an
error is returned.

To work with an array of Unicode code points, see
[`CODE_POINTS_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string)

**Return type**

`STRING`

**Examples**

    SELECT CHR(65) AS A, CHR(255) AS B, CHR(513) AS C, CHR(1024)  AS D;

    /*---+---+---+---+
     | A     | B     | C     | D     |
     +---+---+---+---+
     | A     | ÿ     | ȁ     | Ѐ     |
     +---+---+---+---*/

    SELECT CHR(97) AS A, CHR(0xF9B5) AS B, CHR(0) AS C, CHR(NULL) AS D;

    /*---+---+---+---+
     | A     | B     | C     | D     |
     +---+---+---+---+
     | a     | 例    |       | NULL  |
     +---+---+---+---*/

## `CODE_POINTS_TO_BYTES`

    CODE_POINTS_TO_BYTES(ascii_code_points)

**Description**

Takes an array of extended ASCII
[code points](https://en.wikipedia.org/wiki/Code_point)
as `ARRAY<INT64>` and returns `BYTES`.

To convert from `BYTES` to an array of code points, see
[TO_CODE_POINTS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points).

**Return type**

`BYTES`

**Examples**

The following is a basic example using `CODE_POINTS_TO_BYTES`.

    SELECT CODE_POINTS_TO_BYTES([65, 98, 67, 100]) AS bytes;

    -- Note that the result of CODE_POINTS_TO_BYTES is of type BYTES, displayed as a base64-encoded string.
    -- In BYTES format, b'AbCd' is the result.
    /*---+
     | bytes    |
     +---+
     | QWJDZA== |
     +---*/

The following example uses a rotate-by-13 places (ROT13) algorithm to encode a
string.

    SELECT CODE_POINTS_TO_BYTES(ARRAY_AGG(
      (SELECT
          CASE
            WHEN chr BETWEEN b'a' and b'z'
              THEN TO_CODE_POINTS(b'a')[offset(0)] +
                MOD(code+13-TO_CODE_POINTS(b'a')[offset(0)],26)
            WHEN chr BETWEEN b'A' and b'Z'
              THEN TO_CODE_POINTS(b'A')[offset(0)] +
                MOD(code+13-TO_CODE_POINTS(b'A')[offset(0)],26)
            ELSE code
          END
       FROM
         (SELECT code, CODE_POINTS_TO_BYTES([code]) chr)
      ) ORDER BY OFFSET)) AS encoded_string
    FROM UNNEST(TO_CODE_POINTS(b'Test String!')) code WITH OFFSET;

    -- Note that the result of CODE_POINTS_TO_BYTES is of type BYTES, displayed as a base64-encoded string.
    -- In BYTES format, b'Grfg Fgevat!' is the result.
    /*---+
     | encoded_string   |
     +---+
     | R3JmZyBGZ2V2YXQh |
     +---*/

## `CODE_POINTS_TO_STRING`

    CODE_POINTS_TO_STRING(unicode_code_points)

**Description**

Takes an array of Unicode [code points](https://en.wikipedia.org/wiki/Code_point)
as `ARRAY<INT64>` and returns a `STRING`.

To convert from a string to an array of code points, see
[TO_CODE_POINTS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points).

**Return type**

`STRING`

**Examples**

The following are basic examples using `CODE_POINTS_TO_STRING`.

    SELECT CODE_POINTS_TO_STRING([65, 255, 513, 1024]) AS string;

    /*---+
     | string |
     +---+
     | AÿȁЀ   |
     +---*/

    SELECT CODE_POINTS_TO_STRING([97, 0, 0xF9B5]) AS string;

    /*---+
     | string |
     +---+
     | a例    |
     +---*/

    SELECT CODE_POINTS_TO_STRING([65, 255, NULL, 1024]) AS string;

    /*---+
     | string |
     +---+
     | NULL   |
     +---*/

The following example computes the frequency of letters in a set of words.

    WITH Words AS (
      SELECT word
      FROM UNNEST(['foo', 'bar', 'baz', 'giraffe', 'llama']) AS word
    )
    SELECT
      CODE_POINTS_TO_STRING([code_point]) AS letter,
      COUNT(*) AS letter_count
    FROM Words,
      UNNEST(TO_CODE_POINTS(word)) AS code_point
    GROUP BY 1
    ORDER BY 2 DESC;

    /*---+---+
     | letter | letter_count |
     +---+---+
     | a      | 5            |
     | f      | 3            |
     | r      | 2            |
     | b      | 2            |
     | l      | 2            |
     | o      | 2            |
     | g      | 1            |
     | z      | 1            |
     | e      | 1            |
     | m      | 1            |
     | i      | 1            |
     +---+---*/

## `COLLATE`

    COLLATE(value, collate_specification)

Takes a `STRING` and a [collation specification](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#collate_spec_details). Returns
a `STRING` with a collation specification. If `collate_specification` is empty,
returns a value with collation removed from the `STRING`.

The collation specification defines how the resulting `STRING` can be compared
and sorted. To learn more, see
[Collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

- `collation_specification` must be a string literal, otherwise an error is thrown.
- Returns `NULL` if `value` is `NULL`.

**Return type**

`STRING`

**Examples**

In this example, the weight of `a` is less than the weight of `Z`. This
is because the collate specification, `und:ci` assigns more weight to `Z`.

    WITH Words AS (
      SELECT
        COLLATE('a', 'und:ci') AS char1,
        COLLATE('Z', 'und:ci') AS char2
    )
    SELECT ( Words.char1 < Words.char2 ) AS a_less_than_Z
    FROM Words;

    /*---+
     | a_less_than_Z  |
     +---+
     | TRUE           |
     +---*/

In this example, the weight of `a` is greater than the weight of `Z`. This
is because the default collate specification assigns more weight to `a`.

    WITH Words AS (
      SELECT
        'a' AS char1,
        'Z' AS char2
    )
    SELECT ( Words.char1 < Words.char2 ) AS a_less_than_Z
    FROM Words;

    /*---+
     | a_less_than_Z  |
     +---+
     | FALSE          |
     +---*/

## `CONCAT`

    CONCAT(value1[, ...])

**Description**

Concatenates one or more values into a single result. All values must be
`BYTES` or data types that can be cast to `STRING`.

The function returns `NULL` if any input argument is `NULL`.

> [!NOTE]
> **Note:** You can also use the [\|\| concatenation operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators) to concatenate values into a string.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT CONCAT('T.P.', ' ', 'Bar') as author;

    /*---+
     | author              |
     +---+
     | T.P. Bar            |
     +---*/

    SELECT CONCAT('Summer', ' ', 1923) as release_date;

    /*---+
     | release_date        |
     +---+
     | Summer 1923         |
     +---*/


    With Employees AS
      (SELECT
        'John' AS first_name,
        'Doe' AS last_name
      UNION ALL
      SELECT
        'Jane' AS first_name,
        'Smith' AS last_name
      UNION ALL
      SELECT
        'Joe' AS first_name,
        'Jackson' AS last_name)

    SELECT
      CONCAT(first_name, ' ', last_name)
      AS full_name
    FROM Employees;

    /*---+
     | full_name           |
     +---+
     | John Doe            |
     | Jane Smith          |
     | Joe Jackson         |
     +---*/

## `CONTAINS_SUBSTR`

    CONTAINS_SUBSTR(
      expression,
      search_value_literal
      [, json_scope => json_scope_value ]
    )

**Description**

Performs a normalized, case-insensitive search to see if a value exists as a
substring in an expression. Returns `TRUE` if the value exists, otherwise
returns `FALSE`.

Before values are compared, they are [normalized and case folded with `NFKC`
normalization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize_and_casefold). Wildcard searches aren't
supported.

**Arguments**

- `search_value_literal`: The value to search for. It must be a `STRING` literal or a `STRING` constant expression.
- `expression`: The data to search over. The expression can be a column or
  table reference. A table reference is evaluated as a `STRUCT` whose fields
  are the columns of the table. A column reference is evaluated as one the
  following data types:

  - `STRING`
  - `INT64`
  - `BOOL`
  - `NUMERIC`
  - `BIGNUMERIC`
  - `TIMESTAMP`
  - `TIME`
  - `DATE`
  - `DATETIME`
  - `ARRAY`
  - `STRUCT`

  When the expression is evaluated, the result is cast to a `STRING`, and then
  the function looks for the search value in the result.

  You can perform a cross-field search on an expression that evaluates to a
  `STRUCT` or `ARRAY`. If the expression evaluates to a `STRUCT`, the
  cross-field search is recursive and includes all subfields inside the
  `STRUCT`.

  In a cross-field search, each field and subfield is individually converted
  to a string and searched for the value. The function returns `TRUE` if at
  least one field includes the search value; otherwise, if at least one field
  is `NULL`, it returns `NULL`; otherwise, if the search value isn't found
  and all fields are non-`NULL`, it returns `FALSE`.

  If the expression is `NULL`, the return value is `NULL`.
- `json_scope`: A named argument with a `STRING` value.
  Takes one of the following values to indicate the scope of `JSON` data to be
  searched. It has no effect if `expression` isn't `JSON` or doesn't
  contain a `JSON` field.

  - `'JSON_VALUES'`: Only the `JSON` values are searched. If `json_scope` is not provided, this is used by default.
  - `'JSON_KEYS'`: Only the `JSON` keys are searched.
  - `'JSON_KEYS_AND_VALUES'`: The `JSON` keys and values are searched.

**Return type**

`BOOL`

**Examples**

The following query returns `TRUE` because this case-insensitive match
was found: `blue house` and `Blue house`.

    SELECT CONTAINS_SUBSTR('the blue house', 'Blue house') AS result;

    /*---+
     | result |
     +---+
     | true   |
     +---*/

The following query returns `TRUE` similar to the above example, but in this
case the search value is a constant expression with CONCAT function.

    SELECT CONTAINS_SUBSTR('the blue house', CONCAT('Blue ', 'house')) AS result;

    /*---+
     | result |
     +---+
     | true   |
     +---*/

The following query returns `FALSE` because `blue` wasn't found
in `the red house`.

    SELECT CONTAINS_SUBSTR('the red house', 'blue') AS result;

    /*---+
     | result |
     +---+
     | false  |
     +---*/

The following query returns `TRUE` because `Ⅸ` and `IX` represent the same
normalized value.

    SELECT '\u2168 day' AS a, 'IX' AS b, CONTAINS_SUBSTR('\u2168', 'IX') AS result;

    /*---+
     | a      | b  | result |
     +---+
     | Ⅸ day | IX | true   |
     +---*/

The following query returns `TRUE` because `35` was found inside a
`STRUCT` field.

    SELECT CONTAINS_SUBSTR((23, 35, 41), '35') AS result;

    /*---+
     | result |
     +---+
     | true   |
     +---*/

The following query returns `TRUE` because `jk` was found during a
recursive search inside a `STRUCT`.

    SELECT CONTAINS_SUBSTR(('abc', ['def', 'ghi', 'jkl'], 'mno'), 'jk');

    /*---+
     | result |
     +---+
     | true   |
     +---*/

The following query returns `TRUE` because `NULL`s are ignored when
a match is found found inside a `STRUCT` or `ARRAY`.

    SELECT CONTAINS_SUBSTR((23, NULL, 41), '41') AS result;

    /*---+
     | result |
     +---+
     | true   |
     +---*/

The following query returns `NULL` because a `NULL` existed in a `STRUCT` that
didn't result in a match.

    SELECT CONTAINS_SUBSTR((23, NULL, 41), '35') AS result;

    /*---+
     | result |
     +---+
     | null   |
     +---*/

In the following query, an error is thrown because the search value can't be
a literal `NULL`.

    SELECT CONTAINS_SUBSTR('hello', NULL) AS result;
    -- Throws an error

The following examples reference a table called `Recipes` that you can emulate
with a `WITH` clause like this:

    WITH Recipes AS
     (SELECT 'Blueberry pancakes' as Breakfast, 'Egg salad sandwich' as Lunch, 'Potato dumplings' as Dinner UNION ALL
      SELECT 'Potato pancakes', 'Toasted cheese sandwich', 'Beef stroganoff' UNION ALL
      SELECT 'Ham scramble', 'Steak avocado salad', 'Tomato pasta' UNION ALL
      SELECT 'Avocado toast', 'Tomato soup', 'Blueberry salmon' UNION ALL
      SELECT 'Corned beef hash', 'Lentil potato soup', 'Glazed ham')
    SELECT * FROM Recipes;

    /*---+---+---+
     | Breakfast         | Lunch                   | Dinner           |
     +---+---+---+
     | Bluberry pancakes | Egg salad sandwich      | Potato dumplings |
     | Potato pancakes   | Toasted cheese sandwich | Beef stroganoff  |
     | Ham scramble      | Steak avocado salad     | Tomato pasta     |
     | Avocado toast     | Tomato soup             | Blueberry samon  |
     | Corned beef hash  | Lentil potato soup      | Glazed ham       |
     +---+---+---*/

The following query searches across all columns of the `Recipes` table for the
value `toast` and returns the rows that contain this value.

    SELECT * FROM Recipes WHERE CONTAINS_SUBSTR(Recipes, 'toast');

    /*---+---+---+
     | Breakfast         | Lunch                   | Dinner           |
     +---+---+---+
     | Potato pancakes   | Toasted cheese sandwich | Beef stroganoff  |
     | Avocado toast     | Tomato soup             | Blueberry samon  |
     +---+---+---*/

The following query searches the `Lunch` and `Dinner` columns of the
`Recipe` table for the value `potato` and returns the row if either column
contains this value.

    SELECT * FROM Recipes WHERE CONTAINS_SUBSTR((Lunch, Dinner), 'potato');

    /*---+---+---+
     | Breakfast         | Lunch                   | Dinner           |
     +---+---+---+
     | Bluberry pancakes | Egg salad sandwich      | Potato dumplings |
     | Corned beef hash  | Lentil potato soup      | Glazed ham       |
     +---+---+---*/

The following query searches across all columns of the `Recipes` table
except for the `Lunch` and `Dinner` columns. It returns the rows of any
columns other than `Lunch` or `Dinner` that contain the value `potato`.

    SELECT *
    FROM Recipes
    WHERE CONTAINS_SUBSTR(
      (SELECT AS STRUCT Recipes.* EXCEPT (Lunch, Dinner)),
      'potato'
    );

    /*---+---+---+
     | Breakfast         | Lunch                   | Dinner           |
     +---+---+---+
     | Potato pancakes   | Toasted cheese sandwich | Beef stroganoff  |
     +---+---+---*/

The following query searches for the value `lunch` in the JSON
`{"lunch":"soup"}` and returns `FALSE` because the default `json_scope` is
`"JSON_VALUES"`, and `lunch` is a `JSON` key, not a `JSON` value.

    SELECT CONTAINS_SUBSTR(JSON '{"lunch":"soup"}',"lunch") AS result;

    /*---+
     | result |
     +---+
     | FALSE  |
     +---*/

The following query searches for the value `lunch` in the values of the JSON
`{"lunch":"soup"}` and returns `FALSE` because `lunch` is a `JSON` key, not a
`JSON` value.

    SELECT CONTAINS_SUBSTR(JSON '{"lunch":"soup"}',
                           "lunch",
                           json_scope=>"JSON_VALUES") AS result;

    /*---+
     | result |
     +---+
     | FALSE  |
     +---*/

The following query searches for the value `lunch` in the keys and values of the
JSON `{"lunch":"soup"}` and returns `TRUE` because `lunch` is a `JSON` key.

    SELECT CONTAINS_SUBSTR(JSON '{"lunch":"soup"}',
                           "lunch",
                           json_scope=>"JSON_KEYS_AND_VALUES") AS result;

    /*---+
     | result |
     +---+
     | TRUE   |
     +---*/

The following query searches for the value `lunch` in the keys of the JSON
`{"lunch":"soup"}` and returns `TRUE` because `lunch` is a `JSON` key.

    SELECT CONTAINS_SUBSTR(JSON '{"lunch":"soup"}',
                           "lunch",
                           json_scope=>"JSON_KEYS") AS result;

    /*---+
     | result |
     +---+
     | TRUE   |
     +---*/

## `EDIT_DISTANCE`

    EDIT_DISTANCE(
      value1,
      value2,
      [ max_distance => max_distance_value ]
    )

**Description**

Computes the [Levenshtein distance](https://en.wikipedia.org/wiki/Levenshtein_distance) between two `STRING` or
`BYTES` values.

**Definitions**

- `value1`: The first `STRING` or `BYTES` value to compare.
- `value2`: The second `STRING` or `BYTES` value to compare.
- `max_distance`: A named argument with a `INT64` value that's greater than
  or equal to zero. Represents the maximum distance between the two values
  to compute.

  If this distance is exceeded, the function returns this value.
  The default value for this argument is the maximum size of
  `value1` and `value2`.

**Details**

If `value1` or `value2` is `NULL`, `NULL` is returned.

You can only compare values of the same type. Otherwise, an error is produced.

**Return type**

`INT64`

**Examples**

In the following example, the first character in both strings is different:

    SELECT EDIT_DISTANCE('a', 'b') AS results;

    /*---+
     | results |
     +---+
     | 1       |
     +---*/

In the following example, the first and second characters in both strings are
different:

    SELECT EDIT_DISTANCE('aa', 'b') AS results;

    /*---+
     | results |
     +---+
     | 2       |
     +---*/

In the following example, only the first character in both strings is
different:

    SELECT EDIT_DISTANCE('aa', 'ba') AS results;

    /*---+
     | results |
     +---+
     | 1       |
     +---*/

In the following example, the last six characters are different, but because
the maximum distance is `2`, this function exits early and returns `2`, the
maximum distance:

    SELECT EDIT_DISTANCE('abcdefg', 'a', max_distance => 2) AS results;

    /*---+
     | results |
     +---+
     | 2       |
     +---*/

## `ENDS_WITH`

    ENDS_WITH(value, suffix)

**Description**

Takes two `STRING` or `BYTES` values. Returns `TRUE` if `suffix`
is a suffix of `value`.

This function supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return type**

`BOOL`

**Examples**

    SELECT ENDS_WITH('apple', 'e') as example

    /*---+
     | example |
     +---+
     |    True |
     +---*/

## `FORMAT`

    FORMAT(format_string_expression, data_type_expression[, ...])

**Description**

`FORMAT` formats a data type expression as a string.

- `format_string_expression`: Can contain zero or more [format specifiers](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_specifiers). Each format specifier is introduced by the `%` symbol, and must map to one or more of the remaining arguments. In general, this is a one-to-one mapping, except when the `*` specifier is present. For example, `%.*i` maps to two arguments---a length argument and a signed integer argument. If the number of arguments related to the format specifiers isn't the same as the number of arguments, an error occurs.
- `data_type_expression`: The value to format as a string. This can be any GoogleSQL data type.

**Return type**

`STRING`

**Examples**

| Description | Statement | Result |
|---|---|---|
| Simple integer | FORMAT('%d', 10) | 10 |
| Integer with left blank padding | FORMAT('\|%10d\|', 11) | \| 11\| |
| Integer with left zero padding | FORMAT('+%010d+', 12) | +0000000012+ |
| Integer with commas | FORMAT("%'d", 123456789) | 123,456,789 |
| STRING | FORMAT('-%s-', 'abcd efg') | -abcd efg- |
| FLOAT64 | FORMAT('%f %E', 1.1, 2.2) | 1.100000 2.200000E+00 |
| DATE | FORMAT('%t', date '2015-09-01') | 2015-09-01 |
| TIMESTAMP | FORMAT('%t', timestamp '2015-09-01 12:34:56 America/Los_Angeles') | 2015‑09‑01 19:34:56+00 |

The `FORMAT()` function doesn't provide fully customizable formatting for all
types and values, nor formatting that's sensitive to locale.

If custom formatting is necessary for a type, you must first format it using
type-specific format functions, such as `FORMAT_DATE()` or `FORMAT_TIMESTAMP()`.
For example:

    SELECT FORMAT('date: %s!', FORMAT_DATE('%B %d, %Y', date '2015-01-02'));

Returns

    date: January 02, 2015!

#### Supported format specifiers

    %[flags][width][.precision]specifier

A [format specifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_specifier_list) adds formatting when casting a
value to a string. It can optionally contain these sub-specifiers:

- [Flags](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#flags)
- [Width](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#width)
- [Precision](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#precision)

Additional information about format specifiers:

- [%g and %G behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#g_and_g_behavior)
- [%p and %P behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#p_and_p_behavior)
- [%t and %T behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#t_and_t_behavior)
- [Error conditions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#error_format_specifiers)
- [NULL argument handling](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#null_format_specifiers)
- [Additional semantic rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rules_format_specifiers)

##### Format specifiers

|---|---|---|---|
| Specifier | Description | Examples | Types |
| `d` or `i` | Decimal integer | 392 | `INT64` |
| `o` | Octal <br /> Note: If an `INT64` value is negative, an error is produced. | 610 | `INT64` |
| `x` | Hexadecimal integer <br /> Note: If an `INT64` value is negative, an error is produced. | 7fa | `INT64` |
| `X` | Hexadecimal integer (uppercase) <br /> Note: If an `INT64` value is negative, an error is produced. | 7FA | `INT64` |
| `f` | Decimal notation, in \[-\](integer part).(fractional part) for finite values, and in lowercase for non-finite values | 392.650000 inf nan | `NUMERIC` `BIGNUMERIC` `FLOAT64` |
| `F` | Decimal notation, in \[-\](integer part).(fractional part) for finite values, and in uppercase for non-finite values | 392.650000 INF NAN | `NUMERIC` `BIGNUMERIC` `FLOAT64` |
| `e` | Scientific notation (mantissa/exponent), lowercase | 3.926500e+02 inf nan | `NUMERIC` `BIGNUMERIC` `FLOAT64` |
| `E` | Scientific notation (mantissa/exponent), uppercase | 3.926500E+02 INF NAN | `NUMERIC` `BIGNUMERIC` `FLOAT64` |
| `g` | Either decimal notation or scientific notation, depending on the input value's exponent and the specified precision. Lowercase. See [%g and %G behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#g_and_g_behavior) for details. | 392.65 3.9265e+07 inf nan | `NUMERIC` `BIGNUMERIC` `FLOAT64` |
| `G` | Either decimal notation or scientific notation, depending on the input value's exponent and the specified precision. Uppercase. See [%g and %G behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#g_and_g_behavior) for details. | 392.65 3.9265E+07 INF NAN | `NUMERIC` `BIGNUMERIC` `FLOAT64` |
| `p` | Produces a one-line printable string representing JSON. See [%p and %P behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#p_and_p_behavior). | ``` {"month":10,"year":2019} ``` | `JSON` |
| `P` | Produces a multi-line printable string representing JSON. See [%p and %P behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#p_and_p_behavior). | ``` { "month": 10, "year": 2019 } ``` | `JSON` |
| `s` | String of characters | sample | `STRING` |
| `t` | Returns a printable string representing the value. Often looks similar to casting the argument to `STRING`. See [%t and %T behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#t_and_t_behavior). | sample 2014‑01‑01 | Any type |
| `T` | Produces a string that's a valid GoogleSQL constant with a similar type to the value's type (maybe wider, or maybe string). See [%t and %T behavior](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#t_and_t_behavior). | 'sample' b'bytes sample' 1234 2.3 date '2014‑01‑01' | Any type |
| `%` | '%%' produces a single '%' | % | n/a |

The format specifier can optionally contain the sub-specifiers identified above
in the specifier prototype.

These sub-specifiers must comply with the following specifications.

##### Flags

|---|---|
| Flags | Description |
| `-` | Left-justify within the given field width; Right justification is the default (see width sub-specifier) |
| `+` | Forces to precede the result with a plus or minus sign (`+` or `-`) even for positive numbers. By default, only negative numbers are preceded with a `-` sign |
| \<space\> | If no sign is going to be written, a blank space is inserted before the value |
| `#` | - For \`%o\`, \`%x\`, and \`%X\`, this flag means to precede the value with 0, 0x or 0X respectively for values different than zero. - For \`%f\`, \`%F\`, \`%e\`, and \`%E\`, this flag means to add the decimal point even when there is no fractional part, unless the value is non-finite. - For \`%g\` and \`%G\`, this flag means to add the decimal point even when there is no fractional part unless the value is non-finite, and never remove the trailing zeros after the decimal point. |
| `0` | Left-pads the number with zeroes (0) instead of spaces when padding is specified (see width sub-specifier) |
| `'` | Formats integers using the appropriating grouping character. For example: - `FORMAT("%'d", 12345678)` returns `12,345,678` - `FORMAT("%'x", 12345678)` returns `bc:614e` - `FORMAT("%'o", 55555)` returns `15,4403` - This flag is only relevant for decimal, hex, and octal values. |

Flags may be specified in any order. Duplicate flags aren't an error. When
flags aren't relevant for some element type, they are ignored.

##### Width

|---|---|
| Width | Description |
| \<number\> | Minimum number of characters to be printed. If the value to be printed is shorter than this number, the result is padded with blank spaces. The value isn't truncated even if the result is larger |
| `*` | The width isn't specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted |

##### Precision

|---|---|
| Precision | Description |
| `.`\<number\> | - For integer specifiers \`%d\`, \`%i\`, \`%o\`, \`%u\`, \`%x\`, and \`%X\`: precision specifies the minimum number of digits to be written. If the value to be written is shorter than this number, the result is padded with trailing zeros. The value isn't truncated even if the result is longer. A precision of 0 means that no character is written for the value 0. - For specifiers \`%a\`, \`%A\`, \`%e\`, \`%E\`, \`%f\`, and \`%F\`: this is the number of digits to be printed after the decimal point. The default value is 6. - For specifiers \`%g\` and \`%G\`: this is the number of significant digits to be printed, before the removal of the trailing zeros after the decimal point. The default value is 6. |
| `.*` | The precision isn't specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted |

##### %g and %G behavior

The `%g` and `%G` format specifiers choose either the decimal notation (like
the `%f` and `%F` specifiers) or the scientific notation (like the `%e` and `%E`
specifiers), depending on the input value's exponent and the specified
[precision](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#precision).

Let p stand for the specified [precision](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#precision) (defaults to 6; 1 if the
specified precision is less than 1). The input value is first converted to
scientific notation with precision = (p - 1). If the resulting exponent part x
is less than -4 or no less than p, the scientific notation with precision =
(p - 1) is used; otherwise the decimal notation with precision = (p - 1 - x) is
used.

Unless [`#` flag](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#flags) is present, the trailing zeros after the decimal point
are removed, and the decimal point is also removed if there is no digit after
it.

##### %p and %P behavior

The `%p` format specifier produces a one-line printable string. The `%P`
format specifier produces a multi-line printable string. You can use these
format specifiers with the following data types:

|---|---|---|
| **Type** | **%p** | **%P** |
| JSON | JSON input: ``` JSON ' { "month": 10, "year": 2019 } ' ``` Produces a one-line printable string representing JSON: ``` {"month":10,"year":2019} ``` | JSON input: ``` JSON ' { "month": 10, "year": 2019 } ' ``` Produces a multi-line printable string representing JSON: ``` { "month": 10, "year": 2019 } ``` |

##### %t and %T behavior

The `%t` and `%T` format specifiers are defined for all types. The
[width](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#width), [precision](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#precision), and [flags](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#flags) act as they do
for `%s`: the [width](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#width) is the minimum width and the `STRING` will be
padded to that size, and [precision](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#precision) is the maximum width
of content to show and the `STRING` will be truncated to that size, prior to
padding to width.

The `%t` specifier is always meant to be a readable form of the value.

The `%T` specifier is always a valid SQL literal of a similar type, such as a
wider numeric type. The literal will not include casts or a type name,
except for the special case of non-finite floating point values.

The `STRING` is formatted as follows:

|---|---|---|
| **Type** | **%t** | **%T** |
| `NULL` of any type | NULL | NULL |
| `INT64` | 123 | 123 |
| NUMERIC | 123.0 *(always with .0)* | NUMERIC "123.0" |
| FLOAT64 | 123.0 *(always with .0)* 123e+10 `inf` `-inf` `NaN` | 123.0 *(always with .0)* 123e+10 CAST("inf" AS \<type\>) CAST("-inf" AS \<type\>) CAST("nan" AS \<type\>) |
| STRING | unquoted string value | quoted string literal |
| BYTES | unquoted escaped bytes e.g., abc\\x01\\x02 | quoted bytes literal e.g., b"abc\\x01\\x02" |
| BOOL | boolean value | boolean value |
| DATE | 2011-02-03 | DATE "2011-02-03" |
| TIMESTAMP | 2011-02-03 04:05:06+00 | TIMESTAMP "2011-02-03 04:05:06+00" |
| INTERVAL | 1-2 3 4:5:6.789 | INTERVAL "1-2 3 4:5:6.789" YEAR TO SECOND |
| ARRAY | \[value, value, ...\] where values are formatted with %t | \[value, value, ...\] where values are formatted with %T |
| STRUCT | (value, value, ...) where fields are formatted with %t | (value, value, ...) where fields are formatted with %T <br /> Special cases: Zero fields: STRUCT() One field: STRUCT(value) |
| JSON | one-line printable string representing JSON. ```json {"name":"apple","stock":3} ``` | one-line printable string representing a JSON literal. ```sql JSON '{"name":"apple","stock":3}' ``` |

##### Error conditions

If a format specifier is invalid, or isn't compatible with the related
argument type, or the wrong number or arguments are provided, then an error is
produced. For example, the following `<format_string>` expressions are invalid:

    FORMAT('%s', 1)

    FORMAT('%')

##### NULL argument handling

A `NULL` format string results in a `NULL` output `STRING`. Any other arguments
are ignored in this case.

The function generally produces a `NULL` value if a `NULL` argument is present.
For example, `FORMAT('%i', NULL_expression)` produces a `NULL STRING` as
output.

However, there are some exceptions: if the format specifier is %t or %T
(both of which produce `STRING`s that effectively match CAST and literal value
semantics), a `NULL` value produces 'NULL' (without the quotes) in the result
`STRING`. For example, the function:

    FORMAT('00-%t-00', NULL_expression);

Returns

    00-NULL-00

##### Additional semantic rules

`FLOAT64` values can be `+/-inf` or `NaN`.
When an argument has one of those values, the result of the format specifiers
`%f`, `%F`, `%e`, `%E`, `%g`, `%G`, and `%t` are `inf`, `-inf`, or `nan`
(or the same in uppercase) as appropriate. This is consistent with how
GoogleSQL casts these values to `STRING`. For `%T`,
GoogleSQL returns quoted strings for
`FLOAT64` values that don't have non-string literal
representations.

## `FROM_BASE32`

    FROM_BASE32(string_expr)

**Description**

Converts the base32-encoded input `string_expr` into `BYTES` format. To convert
`BYTES` to a base32-encoded `STRING`, use [TO_BASE32](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base32).

**Return type**

`BYTES`

**Example**

    SELECT FROM_BASE32('MFRGGZDF74======') AS byte_data;

    -- Note that the result of FROM_BASE32 is of type BYTES, displayed as a base64-encoded string.
    /*---+
     | byte_data |
     +---+
     | YWJjZGX/  |
     +---*/

## `FROM_BASE64`

    FROM_BASE64(string_expr)

**Description**

Converts the base64-encoded input `string_expr` into
`BYTES` format. To convert
`BYTES` to a base64-encoded `STRING`,
use [TO_BASE64](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base64).

There are several base64 encodings in common use that vary in exactly which
alphabet of 65 ASCII characters are used to encode the 64 digits and padding.
See [RFC 4648](https://tools.ietf.org/html/rfc4648#section-4) for details. This
function expects the alphabet `[A-Za-z0-9+/=]`.

**Return type**

`BYTES`

**Example**

    SELECT FROM_BASE64('/+A=') AS byte_data;

    -- Note that the result of FROM_BASE64 is of type BYTES, displayed as a base64-encoded string.
    /*---+
     | byte_data |
     +---+
     | /+A=      |
     +---*/

To work with an encoding using a different base64 alphabet, you might need to
compose `FROM_BASE64` with the `REPLACE` function. For instance, the
`base64url` url-safe and filename-safe encoding commonly used in web programming
uses `-_=` as the last characters rather than `+/=`. To decode a
`base64url`-encoded string, replace `-` and `_` with `+` and `/` respectively.

    SELECT FROM_BASE64(REPLACE(REPLACE('_-A=', '-', '+'), '_', '/')) AS binary;

    -- Note that the result of FROM_BASE64 is of type BYTES, displayed as a base64-encoded string.
    /*---+
     | binary |
     +---+
     | /+A=   |
     +---*/

## `FROM_HEX`

    FROM_HEX(string)

**Description**

Converts a hexadecimal-encoded `STRING` into `BYTES` format. Returns an error
if the input `STRING` contains characters outside the range
`(0..9, A..F, a..f)`. The lettercase of the characters doesn't matter. If the
input `STRING` has an odd number of characters, the function acts as if the
input has an additional leading `0`. To convert `BYTES` to a hexadecimal-encoded
`STRING`, use [TO_HEX](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex).

**Return type**

`BYTES`

**Example**

    WITH Input AS (
      SELECT '00010203aaeeefff' AS hex_str UNION ALL
      SELECT '0AF' UNION ALL
      SELECT '666f6f626172'
    )
    SELECT hex_str, FROM_HEX(hex_str) AS bytes_str
    FROM Input;

    -- Note that the result of FROM_HEX is of type BYTES, displayed as a base64-encoded string.
    /*---+---+
     | hex_str          | bytes_str    |
     +---+---+
     | 0AF              | AK8=         |
     | 00010203aaeeefff | AAECA6ru7/8= |
     | 666f6f626172     | Zm9vYmFy     |
     +---+---*/

## `INITCAP`

    INITCAP(value[, delimiters])

**Description**

Takes a `STRING` and returns it with the first character in each word in
uppercase and all other characters in lowercase. Non-alphabetic characters
remain the same.

`delimiters` is an optional string argument that's used to override the default
set of characters used to separate words. If `delimiters` isn't specified, it
defaults to the following characters:   

`<whitespace> [ ] ( ) { } / | \ < > ! ? @ " ^ # $ & ~ _ , . : ; * % + -`

If `value` or `delimiters` is `NULL`, the function returns `NULL`.

**Return type**

`STRING`

**Examples**

    SELECT
      'Hello World-everyone!' AS value,
      INITCAP('Hello World-everyone!') AS initcap_value

    /*---+---+
     | value                         | initcap_value                 |
     +---+---+
     | Hello World-everyone!         | Hello World-Everyone!         |
     +---+---*/

    SELECT
      'Apples1oranges2pears' as value,
      '12' AS delimiters,
      INITCAP('Apples1oranges2pears' , '12') AS initcap_value

    /*---+---+---+
     | value                | delimiters | initcap_value        |
     +---+---+---+
     | Apples1oranges2pears | 12         | Apples1Oranges2Pears |
     +---+---+---*/

## `INSTR`

    INSTR(value, subvalue[, position[, occurrence]])

**Description**

Returns the lowest 1-based position of `subvalue` in `value`.
`value` and `subvalue` must be the same type, either
`STRING` or `BYTES`.

If `position` is specified, the search starts at this position in
`value`, otherwise it starts at `1`, which is the beginning of
`value`. If `position` is negative, the function searches backwards
from the end of `value`, with `-1` indicating the last character.
`position` is of type `INT64` and can't be `0`.

If `occurrence` is specified, the search returns the position of a specific
instance of `subvalue` in `value`. If not specified, `occurrence`
defaults to `1` and returns the position of the first occurrence.
For `occurrence` \> `1`, the function includes overlapping occurrences.
`occurrence` is of type `INT64` and must be positive.

This function supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

Returns `0` if:

- No match is found.
- If `occurrence` is greater than the number of matches found.
- If `position` is greater than the length of `value`.

Returns `NULL` if:

- Any input argument is `NULL`.

Returns an error if:

- `position` is `0`.
- `occurrence` is `0` or negative.

**Return type**

`INT64`

**Examples**

    SELECT
      'banana' AS value, 'an' AS subvalue, 1 AS position, 1 AS occurrence,
      INSTR('banana', 'an', 1, 1) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | banana       | an           | 1        | 1          | 2     |
     +---+---+---+---+---*/

    SELECT
      'banana' AS value, 'an' AS subvalue, 1 AS position, 2 AS occurrence,
      INSTR('banana', 'an', 1, 2) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | banana       | an           | 1        | 2          | 4     |
     +---+---+---+---+---*/

    SELECT
      'banana' AS value, 'an' AS subvalue, 1 AS position, 3 AS occurrence,
      INSTR('banana', 'an', 1, 3) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | banana       | an           | 1        | 3          | 0     |
     +---+---+---+---+---*/

    SELECT
      'banana' AS value, 'an' AS subvalue, 3 AS position, 1 AS occurrence,
      INSTR('banana', 'an', 3, 1) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | banana       | an           | 3        | 1          | 4     |
     +---+---+---+---+---*/

    SELECT
      'banana' AS value, 'an' AS subvalue, -1 AS position, 1 AS occurrence,
      INSTR('banana', 'an', -1, 1) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | banana       | an           | -1       | 1          | 4     |
     +---+---+---+---+---*/

    SELECT
      'banana' AS value, 'an' AS subvalue, -3 AS position, 1 AS occurrence,
      INSTR('banana', 'an', -3, 1) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | banana       | an           | -3       | 1          | 4     |
     +---+---+---+---+---*/

    SELECT
      'banana' AS value, 'ann' AS subvalue, 1 AS position, 1 AS occurrence,
      INSTR('banana', 'ann', 1, 1) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | banana       | ann          | 1        | 1          | 0     |
     +---+---+---+---+---*/

    SELECT
      'helloooo' AS value, 'oo' AS subvalue, 1 AS position, 1 AS occurrence,
      INSTR('helloooo', 'oo', 1, 1) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | helloooo     | oo           | 1        | 1          | 5     |
     +---+---+---+---+---*/

    SELECT
      'helloooo' AS value, 'oo' AS subvalue, 1 AS position, 2 AS occurrence,
      INSTR('helloooo', 'oo', 1, 2) AS instr;

    /*---+---+---+---+---+
     | value        | subvalue     | position | occurrence | instr |
     +---+---+---+---+---+
     | helloooo     | oo           | 1        | 2          | 6     |
     +---+---+---+---+---*/

## `LEFT`

    LEFT(value, length)

**Description**

Returns a `STRING` or `BYTES` value that consists of the specified
number of leftmost characters or bytes from `value`. The `length` is an
`INT64` that specifies the length of the returned
value. If `value` is of type `BYTES`, `length` is the number of leftmost bytes
to return. If `value` is `STRING`, `length` is the number of leftmost characters
to return.

If `length` is 0, an empty `STRING` or `BYTES` value will be
returned. If `length` is negative, an error will be returned. If `length`
exceeds the number of characters or bytes from `value`, the original `value`
will be returned.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT LEFT('banana', 3) AS results

    /*---+
     | results |
      +---+
     | ban     |
     +---*/

    SELECT LEFT(b'\xab\xcd\xef\xaa\xbb', 3) AS results

    -- Note that the result of LEFT is of type BYTES, displayed as a base64-encoded string.
    /*---+
     | results |
     +---+
     | q83v    |
     +---*/

## `LENGTH`

    LENGTH(value)

**Description**

Returns the length of the `STRING` or `BYTES` value. The returned
value is in characters for `STRING` arguments and in bytes for the `BYTES`
argument.

**Return type**

`INT64`

**Examples**

    SELECT
      LENGTH('абвгд') AS string_example,
      LENGTH(CAST('абвгд' AS BYTES)) AS bytes_example;

    /*---+---+
     | string_example | bytes_example |
     +---+---+
     | 5              | 10            |
     +---+---*/

## `LOWER`

    LOWER(value)

**Description**

For `STRING` arguments, returns the original string with all alphabetic
characters in lowercase. Mapping between lowercase and uppercase is done
according to the
[Unicode Character Database](http://unicode.org/ucd/)
without taking into account language-specific mappings.

For `BYTES` arguments, the argument is treated as ASCII text, with all bytes
greater than 127 left intact.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT
      LOWER('FOO BAR BAZ') AS example
    FROM items;

    /*---+
     | example     |
     +---+
     | foo bar baz |
     +---*/

## `LPAD`

    LPAD(original_value, return_length[, pattern])

**Description**

Returns a `STRING` or `BYTES` value that consists of `original_value` prepended
with `pattern`. The `return_length` is an `INT64` that
specifies the length of the returned value. If `original_value` is of type
`BYTES`, `return_length` is the number of bytes. If `original_value` is
of type `STRING`, `return_length` is the number of characters.

The default value of `pattern` is a blank space.

Both `original_value` and `pattern` must be the same data type.

If `return_length` is less than or equal to the `original_value` length, this
function returns the `original_value` value, truncated to the value of
`return_length`. For example, `LPAD('hello world', 7);` returns `'hello w'`.

If `original_value`, `return_length`, or `pattern` is `NULL`, this function
returns `NULL`.

This function returns an error if:

- `return_length` is negative
- `pattern` is empty

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT FORMAT('%T', LPAD('c', 5)) AS results

    /*---+
     | results |
     +---+
     | "    c" |
     +---*/

    SELECT LPAD('b', 5, 'a') AS results

    /*---+
     | results |
     +---+
     | aaaab   |
     +---*/

    SELECT LPAD('abc', 10, 'ghd') AS results

    /*---+
     | results    |
     +---+
     | ghdghdgabc |
     +---*/

    SELECT LPAD('abc', 2, 'd') AS results

    /*---+
     | results |
     +---+
     | ab      |
     +---*/

    SELECT FORMAT('%T', LPAD(b'abc', 10, b'ghd')) AS results

    /*---+
     | results       |
     +---+
     | b"ghdghdgabc" |
     +---*/

## `LTRIM`

    LTRIM(value1[, value2])

**Description**

Identical to [TRIM](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim), but only removes leading characters.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT CONCAT('#', LTRIM('   apple   '), '#') AS example

    /*---+
     | example     |
     +---+
     | #apple   #  |
     +---*/

    SELECT LTRIM('***apple***', '*') AS example

    /*---+
     | example   |
     +---+
     | apple***  |
     +---*/

    SELECT LTRIM('xxxapplexxx', 'xyz') AS example

    /*---+
     | example   |
     +---+
     | applexxx  |
     +---*/

## `NORMALIZE`

    NORMALIZE(value[, normalization_mode])

**Description**

Takes a string value and returns it as a normalized string. If you don't
provide a normalization mode, `NFC` is used.

[Normalization](https://en.wikipedia.org/wiki/Unicode_equivalence#Normalization) is used to ensure that
two strings are equivalent. Normalization is often used in situations in which
two strings render the same on the screen but have different Unicode code
points.

`NORMALIZE` supports four optional normalization modes:

| Value | Name | Description |
|---|---|---|
| `NFC` | Normalization Form Canonical Composition | Decomposes and recomposes characters by canonical equivalence. |
| `NFKC` | Normalization Form Compatibility Composition | Decomposes characters by compatibility, then recomposes them by canonical equivalence. |
| `NFD` | Normalization Form Canonical Decomposition | Decomposes characters by canonical equivalence, and multiple combining characters are arranged in a specific order. |
| `NFKD` | Normalization Form Compatibility Decomposition | Decomposes characters by compatibility, and multiple combining characters are arranged in a specific order. |

**Return type**

`STRING`

**Examples**

The following example normalizes different language characters:

    SELECT
      NORMALIZE('\u00ea') as a,
      NORMALIZE('\u0065\u0302') as b,
      NORMALIZE('\u00ea') = NORMALIZE('\u0065\u0302') as normalized;

    /*---+---+---+
     | a | b | normalized |
     +---+---+---+
     | ê | ê | TRUE       |
     +---+---+---*/

The following examples normalize different space characters:

    SELECT NORMALIZE('Raha\u2004Mahan', NFKC) AS normalized_name

    /*---+
     | normalized_name |
     +---+
     | Raha Mahan      |
     +---*/

    SELECT NORMALIZE('Raha\u2005Mahan', NFKC) AS normalized_name

    /*---+
     | normalized_name |
     +---+
     | Raha Mahan      |
     +---*/

    SELECT NORMALIZE('Raha\u2006Mahan', NFKC) AS normalized_name

    /*---+
     | normalized_name |
     +---+
     | Raha Mahan      |
     +---*/

    SELECT NORMALIZE('Raha Mahan', NFKC) AS normalized_name

    /*---+
     | normalized_name |
     +---+
     | Raha Mahan      |
     +---*/

## `NORMALIZE_AND_CASEFOLD`

    NORMALIZE_AND_CASEFOLD(value[, normalization_mode])

**Description**

Takes a string value and returns it as a normalized string. If you don't
provide a normalization mode, `NFC` is used.

[Normalization](https://en.wikipedia.org/wiki/Unicode_equivalence#Normalization) is used to ensure that
two strings are equivalent. Normalization is often used in situations in which
two strings render the same on the screen but have different Unicode code
points.

[Case folding](https://en.wikipedia.org/wiki/Letter_case#Case_folding) is used for the caseless
comparison of strings. If you need to compare strings and case shouldn't be
considered, use `NORMALIZE_AND_CASEFOLD`, otherwise use
[`NORMALIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize).

`NORMALIZE_AND_CASEFOLD` supports four optional normalization modes:

| Value | Name | Description |
|---|---|---|
| `NFC` | Normalization Form Canonical Composition | Decomposes and recomposes characters by canonical equivalence. |
| `NFKC` | Normalization Form Compatibility Composition | Decomposes characters by compatibility, then recomposes them by canonical equivalence. |
| `NFD` | Normalization Form Canonical Decomposition | Decomposes characters by canonical equivalence, and multiple combining characters are arranged in a specific order. |
| `NFKD` | Normalization Form Compatibility Decomposition | Decomposes characters by compatibility, and multiple combining characters are arranged in a specific order. |

**Return type**

`STRING`

**Examples**

    SELECT
      NORMALIZE('The red barn') = NORMALIZE('The Red Barn') AS normalized,
      NORMALIZE_AND_CASEFOLD('The red barn')
        = NORMALIZE_AND_CASEFOLD('The Red Barn') AS normalized_with_case_folding;

    /*---+---+
     | normalized | normalized_with_case_folding |
     +---+---+
     | FALSE      | TRUE                         |
     +---+---*/

    SELECT
      '\u2168' AS a,
      'IX' AS b,
      NORMALIZE_AND_CASEFOLD('\u2168', NFD)=NORMALIZE_AND_CASEFOLD('IX', NFD) AS nfd,
      NORMALIZE_AND_CASEFOLD('\u2168', NFC)=NORMALIZE_AND_CASEFOLD('IX', NFC) AS nfc,
      NORMALIZE_AND_CASEFOLD('\u2168', NFKD)=NORMALIZE_AND_CASEFOLD('IX', NFKD) AS nfkd,
      NORMALIZE_AND_CASEFOLD('\u2168', NFKC)=NORMALIZE_AND_CASEFOLD('IX', NFKC) AS nfkc;

    /*---+---+---+---+---+---+
     | a | b  | nfd   | nfc   | nfkd | nfkc |
     +---+---+---+---+---+---+
     | Ⅸ | IX | false | false | true | true |
     +---+---+---+---+---+---*/

    SELECT
      '\u0041\u030A' AS a,
      '\u00C5' AS b,
      NORMALIZE_AND_CASEFOLD('\u0041\u030A', NFD)=NORMALIZE_AND_CASEFOLD('\u00C5', NFD) AS nfd,
      NORMALIZE_AND_CASEFOLD('\u0041\u030A', NFC)=NORMALIZE_AND_CASEFOLD('\u00C5', NFC) AS nfc,
      NORMALIZE_AND_CASEFOLD('\u0041\u030A', NFKD)=NORMALIZE_AND_CASEFOLD('\u00C5', NFKD) AS nkfd,
      NORMALIZE_AND_CASEFOLD('\u0041\u030A', NFKC)=NORMALIZE_AND_CASEFOLD('\u00C5', NFKC) AS nkfc;

    /*---+---+---+---+---+---+
     | a | b  | nfd   | nfc   | nkfd | nkfc |
     +---+---+---+---+---+---+
     | Å | Å  | true  | true  | true | true |
     +---+---+---+---+---+---*/

## `OCTET_LENGTH`

    OCTET_LENGTH(value)

Alias for [`BYTE_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length).

## `REGEXP_CONTAINS`

    REGEXP_CONTAINS(value, regexp)

**Description**

Returns `TRUE` if `value` is a partial match for the regular expression,
`regexp`.

If the `regexp` argument is invalid, the function returns an error.

You can search for a full match by using `^` (beginning of text) and `$` (end of
text). Due to regular expression operator precedence, it's good practice to use
parentheses around everything between `^` and `$`.

> [!NOTE]
> **Note:** GoogleSQL provides regular expression support using the [re2](https://github.com/google/re2/wiki/Syntax) library; see that documentation for its regular expression syntax.

**Return type**

`BOOL`

**Examples**

The following queries check to see if an email is valid:

    SELECT
      'foo@example.com' AS email,
      REGEXP_CONTAINS('foo@example.com', r'@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+') AS is_valid

    /*---+---+
     | email           | is_valid |
     +---+---+
     | foo@example.com | TRUE     |
     +---+---*/

    SELECT
      'www.example.net' AS email,
      REGEXP_CONTAINS('www.example.net', r'@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+') AS is_valid

    /*---+---+
     | email           | is_valid |
     +---+---+
     | www.example.net | FALSE    |
     +---+---*/

The following queries check to see if an email is valid. They
perform a full match, using `^` and `$`. Due to regular expression operator
precedence, it's good practice to use parentheses around everything between `^`
and `$`.

    SELECT
      'a@foo.com' AS email,
      REGEXP_CONTAINS('a@foo.com', r'^([\w.+-]+@foo\.com|[\w.+-]+@bar\.org)$') AS valid_email_address,
      REGEXP_CONTAINS('a@foo.com', r'^[\w.+-]+@foo\.com|[\w.+-]+@bar\.org$') AS without_parentheses;

    /*---+---+---+
     | email          | valid_email_address | without_parentheses |
     +---+---+---+
     | a@foo.com      | true                | true                |
     +---+---+---*/

    SELECT
      'a@foo.computer' AS email,
      REGEXP_CONTAINS('a@foo.computer', r'^([\w.+-]+@foo\.com|[\w.+-]+@bar\.org)$') AS valid_email_address,
      REGEXP_CONTAINS('a@foo.computer', r'^[\w.+-]+@foo\.com|[\w.+-]+@bar\.org$') AS without_parentheses;

    /*---+---+---+
     | email          | valid_email_address | without_parentheses |
     +---+---+---+
     | a@foo.computer | false               | true                |
     +---+---+---*/

    SELECT
      'b@bar.org' AS email,
      REGEXP_CONTAINS('b@bar.org', r'^([\w.+-]+@foo\.com|[\w.+-]+@bar\.org)$') AS valid_email_address,
      REGEXP_CONTAINS('b@bar.org', r'^[\w.+-]+@foo\.com|[\w.+-]+@bar\.org$') AS without_parentheses;

    /*---+---+---+
     | email          | valid_email_address | without_parentheses |
     +---+---+---+
     | b@bar.org      | true                | true                |
     +---+---+---*/

    SELECT
      '!b@bar.org' AS email,
      REGEXP_CONTAINS('!b@bar.org', r'^([\w.+-]+@foo\.com|[\w.+-]+@bar\.org)$') AS valid_email_address,
      REGEXP_CONTAINS('!b@bar.org', r'^[\w.+-]+@foo\.com|[\w.+-]+@bar\.org$') AS without_parentheses;

    /*---+---+---+
     | email          | valid_email_address | without_parentheses |
     +---+---+---+
     | !b@bar.org     | false               | true                |
     +---+---+---*/

    SELECT
      'c@buz.net' AS email,
      REGEXP_CONTAINS('c@buz.net', r'^([\w.+-]+@foo\.com|[\w.+-]+@bar\.org)$') AS valid_email_address,
      REGEXP_CONTAINS('c@buz.net', r'^[\w.+-]+@foo\.com|[\w.+-]+@bar\.org$') AS without_parentheses;

    /*---+---+---+
     | email          | valid_email_address | without_parentheses |
     +---+---+---+
     | c@buz.net      | false               | false               |
     +---+---+---*/

## `REGEXP_EXTRACT`

    REGEXP_EXTRACT(value, regexp[, position[, occurrence]])

**Description**

Returns the substring in `value` that matches the
[re2 regular expression](https://github.com/google/re2/wiki/Syntax), `regexp`.
Returns `NULL` if there is no match.

If the regular expression contains a capturing group (`(...)`), and there is a
match for that capturing group, that match is returned. If there
are multiple matches for a capturing group, the first match is returned.

If `position` is specified, the search starts at this
position in `value`, otherwise it starts at the beginning of `value`. The
`position` must be a positive integer and can't be 0. If `position` is greater
than the length of `value`, `NULL` is returned.

If `occurrence` is specified, the search returns a specific occurrence of the
`regexp` in `value`, otherwise returns the first match. If `occurrence` is
greater than the number of matches found, `NULL` is returned. For
`occurrence` \> 1, the function searches for additional occurrences beginning
with the character following the previous occurrence.

Returns an error if:

- The regular expression is invalid
- The regular expression has more than one capturing group
- The `position` isn't a positive integer
- The `occurrence` isn't a positive integer

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT REGEXP_EXTRACT('foo@example.com', r'^[a-zA-Z0-9_.+-]+') AS user_name

    /*---+
     | user_name |
     +---+
     | foo       |
     +---*/

    SELECT REGEXP_EXTRACT('foo@example.com', r'^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.([a-zA-Z0-9-.]+$)')

    /*---+
     | top_level_domain |
     +---+
     | com              |
     +---*/

    SELECT
      REGEXP_EXTRACT('ab', '.b') AS result_a,
      REGEXP_EXTRACT('ab', '(.)b') AS result_b,
      REGEXP_EXTRACT('xyztb', '(.)+b') AS result_c,
      REGEXP_EXTRACT('ab', '(z)?b') AS result_d

    /*---+
     | result_a | result_b | result_c | result_d |
     +---+
     | ab       | a        | t        | NULL     |
     +---*/

    WITH example AS
    (SELECT 'Hello Helloo and Hellooo' AS value, 'H?ello+' AS regex, 1 as position,
    1 AS occurrence UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 1, 2 UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 1, 3 UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 1, 4 UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 2, 1 UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 3, 1 UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 3, 2 UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 3, 3 UNION ALL
    SELECT 'Hello Helloo and Hellooo', 'H?ello+', 20, 1 UNION ALL
    SELECT 'cats&dogs&rabbits' ,'\\w+&', 1, 2 UNION ALL
    SELECT 'cats&dogs&rabbits', '\\w+&', 2, 3
    )
    SELECT value, regex, position, occurrence, REGEXP_EXTRACT(value, regex,
    position, occurrence) AS regexp_value FROM example;

    /*---+---+---+---+---+
     | value                    | regex   | position | occurrence | regexp_value |
     +---+---+---+---+---+
     | Hello Helloo and Hellooo | H?ello+ | 1        | 1          | Hello        |
     | Hello Helloo and Hellooo | H?ello+ | 1        | 2          | Helloo       |
     | Hello Helloo and Hellooo | H?ello+ | 1        | 3          | Hellooo      |
     | Hello Helloo and Hellooo | H?ello+ | 1        | 4          | NULL         |
     | Hello Helloo and Hellooo | H?ello+ | 2        | 1          | ello         |
     | Hello Helloo and Hellooo | H?ello+ | 3        | 1          | Helloo       |
     | Hello Helloo and Hellooo | H?ello+ | 3        | 2          | Hellooo      |
     | Hello Helloo and Hellooo | H?ello+ | 3        | 3          | NULL         |
     | Hello Helloo and Hellooo | H?ello+ | 20       | 1          | NULL         |
     | cats&dogs&rabbits        | \w+&    | 1        | 2          | dogs&        |
     | cats&dogs&rabbits        | \w+&    | 2        | 3          | NULL         |
     +---+---+---+---+---*/

## `REGEXP_EXTRACT_ALL`

    REGEXP_EXTRACT_ALL(value, regexp)

**Description**

Returns an array of all substrings of `value` that match the
[re2 regular expression](https://github.com/google/re2/wiki/Syntax), `regexp`. Returns an empty array
if there is no match.

If the regular expression contains a capturing group (`(...)`), the function
returns an array of substrings that are matched by the capturing group.

The `REGEXP_EXTRACT_ALL` function only returns non-overlapping matches. For
example, using this function to extract `ana` from `banana` returns only one
substring, not two.

When a capturing group is present, this non-overlapping rule applies to the
*entire substring* matched by the whole regular expression, not just the part
within the capturing group. The search for any subsequent match begins
*after* the end of the entire substring that satisfied the previous match. In
the examples that follow, the second example illustrates this behavior with the
pattern `r'\d(\d)\d'`.

Returns an error if:

- The regular expression is invalid
- The regular expression has more than one capturing group

**Return type**

`ARRAY<STRING>` or `ARRAY<BYTES>`

**Examples**

    SELECT REGEXP_EXTRACT_ALL('Try `func(x)` or `func(y)`', '`(.+?)`') AS example

    /*---+
     | example            |
     +---+
     | [func(x), func(y)] |
     +---*/

The following example demonstrates non-overlapping matches with a capturing
group:

    SELECT REGEXP_EXTRACT_ALL('123456', r'\d(\d)\d') AS example;

    /*---+
     | example   |
     +---+
     | ['2', '5'] |
     +---*/

The pattern `r'\d(\d)\d'` matches `'123'` and captures `'2'`. The next
search starts after `'3'`, and then it matches `'456'` and captures `'5'`.

## `REGEXP_INSTR`

    REGEXP_INSTR(source_value, regexp [, position[, occurrence, [occurrence_position]]])

**Description**

Returns the lowest 1-based position of a regular expression, `regexp`, in
`source_value`. `source_value` and `regexp` must be the same type, either
`STRING` or `BYTES`.

If `position` is specified, the search starts at this position in
`source_value`, otherwise it starts at `1`, which is the beginning of
`source_value`. `position` is of type `INT64` and must be positive.

If `occurrence` is specified, the search returns the position of a specific
instance of `regexp` in `source_value`. If not specified, `occurrence` defaults
to `1` and returns the position of the first occurrence. For `occurrence` \> 1,
the function searches for the next, non-overlapping occurrence.
`occurrence` is of type `INT64` and must be positive.

You can optionally use `occurrence_position` to specify where a position
in relation to an `occurrence` starts. Your choices are:

- `0`: Returns the start position of `occurrence`.
- `1`: Returns the end position of `occurrence` + `1`. If the end of the occurrence is at the end of `source_value`, `LENGTH(source_value) + 1` is returned.

Returns `0` if:

- No match is found.
- If `occurrence` is greater than the number of matches found.
- If `position` is greater than the length of `source_value`.
- The regular expression is empty.

Returns `NULL` if:

- `position` is `NULL`.
- `occurrence` is `NULL`.

Returns an error if:

- `position` is `0` or negative.
- `occurrence` is `0` or negative.
- `occurrence_position` is neither `0` nor `1`.
- The regular expression is invalid.
- The regular expression has more than one capturing group.

**Return type**

`INT64`

**Examples**

    SELECT
      REGEXP_INSTR('ab@cd-ef',  '@[^-]*') AS instr_a,
      REGEXP_INSTR('ab@d-ef',   '@[^-]*') AS instr_b,
      REGEXP_INSTR('abc@cd-ef', '@[^-]*') AS instr_c,
      REGEXP_INSTR('abc-ef',    '@[^-]*') AS instr_d,

    /*---+
     | instr_a | instr_b | instr_c | instr_d |
     +---+
     | 3       | 3       | 4       | 0       |
     +---*/

    SELECT
      REGEXP_INSTR('a@cd-ef b@cd-ef', '@[^-]*', 1) AS instr_a,
      REGEXP_INSTR('a@cd-ef b@cd-ef', '@[^-]*', 2) AS instr_b,
      REGEXP_INSTR('a@cd-ef b@cd-ef', '@[^-]*', 3) AS instr_c,
      REGEXP_INSTR('a@cd-ef b@cd-ef', '@[^-]*', 4) AS instr_d,

    /*---+
     | instr_a | instr_b | instr_c | instr_d |
     +---+
     | 2       | 2       | 10      | 10      |
     +---*/

    SELECT
      REGEXP_INSTR('a@cd-ef b@cd-ef c@cd-ef', '@[^-]*', 1, 1) AS instr_a,
      REGEXP_INSTR('a@cd-ef b@cd-ef c@cd-ef', '@[^-]*', 1, 2) AS instr_b,
      REGEXP_INSTR('a@cd-ef b@cd-ef c@cd-ef', '@[^-]*', 1, 3) AS instr_c

    /*---+
     | instr_a | instr_b | instr_c |
     +---+
     | 2       | 10      | 18      |
     +---*/

    SELECT
      REGEXP_INSTR('a@cd-ef', '@[^-]*', 1, 1, 0) AS instr_a,
      REGEXP_INSTR('a@cd-ef', '@[^-]*', 1, 1, 1) AS instr_b

    /*---+
     | instr_a | instr_b |
     +---+
     | 2       | 5       |
     +---*/

## `REGEXP_REPLACE`

    REGEXP_REPLACE(value, regexp, replacement)

**Description**

Returns a `STRING` where all substrings of `value` that
match regular expression `regexp` are replaced with `replacement`.

You can use backslashed-escaped digits (\\1 to \\9) within the `replacement`
argument to insert text matching the corresponding parenthesized group in the
`regexp` pattern. Use \\0 to refer to the entire matching text.

To add a backslash in your regular expression, you must first escape it. For
example, `SELECT REGEXP_REPLACE('abc', 'b(.)', 'X\\1');` returns `aXc`. You can
also use [raw strings](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#string_and_bytes_literals) to remove one layer of
escaping, for example `SELECT REGEXP_REPLACE('abc', 'b(.)', r'X\1');`.

The `REGEXP_REPLACE` function only replaces non-overlapping matches. For
example, replacing `ana` within `banana` results in only one replacement, not
two.

If the `regexp` argument isn't a valid regular expression, this function
returns an error.

> [!NOTE]
> **Note:** GoogleSQL provides regular expression support using the [re2](https://github.com/google/re2/wiki/Syntax) library; see that documentation for its regular expression syntax.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT REGEXP_REPLACE('# Heading', r'^# ([a-zA-Z0-9\s]+$)', '<h1>\\1</h1>') AS html

    /*---+
     | html                     |
     +---+
     | <h1>Heading</h1>         |
     +---*/

## `REGEXP_SUBSTR`

    REGEXP_SUBSTR(value, regexp[, position[, occurrence]])

**Description**

Synonym for [REGEXP_EXTRACT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract).

**Return type**

`STRING` or `BYTES`

**Examples**

    WITH example AS
    (SELECT 'Hello World Helloo' AS value, 'H?ello+' AS regex, 1 AS position, 1 AS
    occurrence
    )
    SELECT value, regex, position, occurrence, REGEXP_SUBSTR(value, regex,
    position, occurrence) AS regexp_value FROM example;

    /*---+---+---+---+---+
     | value              | regex   | position | occurrence | regexp_value |
     +---+---+---+---+---+
     | Hello World Helloo | H?ello+ | 1        | 1          | Hello        |
     +---+---+---+---+---*/

## `REPEAT`

    REPEAT(original_value, repetitions)

**Description**

Returns a `STRING` or `BYTES` value that consists of `original_value`, repeated.
The `repetitions` parameter specifies the number of times to repeat
`original_value`. Returns `NULL` if either `original_value` or `repetitions`
are `NULL`.

This function returns an error if the `repetitions` value is negative.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT REPEAT('abc', 3) AS results

    /*---+
     | results   |
     |---|
     | abcabcabc |
     +---*/

    SELECT REPEAT('abc', NULL) AS results

    /*---+
     | results |
     |---|
     | NULL    |
     +---*/

    SELECT REPEAT(NULL, 3) AS results

    /*---+
     | results |
     |---|
     | NULL    |
     +---*/

## `REPLACE`

    REPLACE(original_value, from_pattern, to_pattern)

**Description**

Replaces all occurrences of `from_pattern` with `to_pattern` in
`original_value`. If `from_pattern` is empty, no replacement is made.

This function supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return type**

`STRING` or `BYTES`

**Examples**

    WITH desserts AS
      (SELECT 'apple pie' as dessert
      UNION ALL
      SELECT 'blackberry pie' as dessert
      UNION ALL
      SELECT 'cherry pie' as dessert)

    SELECT
      REPLACE (dessert, 'pie', 'cobbler') as example
    FROM desserts;

    /*---+
     | example            |
     +---+
     | apple cobbler      |
     | blackberry cobbler |
     | cherry cobbler     |
     +---*/

## `REVERSE`

    REVERSE(value)

**Description**

Returns the reverse of the input `STRING` or `BYTES`.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT REVERSE('abc') AS results

    /*---+
     | results |
     +---+
     | cba     |
     +---*/

    SELECT FORMAT('%T', REVERSE(b'1a3')) AS results

    /*---+
     | results |
     +---+
     | b"3a1"  |
     +---*/

## `RIGHT`

    RIGHT(value, length)

**Description**

Returns a `STRING` or `BYTES` value that consists of the specified
number of rightmost characters or bytes from `value`. The `length` is an
`INT64` that specifies the length of the returned
value. If `value` is `BYTES`, `length` is the number of rightmost bytes to
return. If `value` is `STRING`, `length` is the number of rightmost characters
to return.

If `length` is 0, an empty `STRING` or `BYTES` value will be
returned. If `length` is negative, an error will be returned. If `length`
exceeds the number of characters or bytes from `value`, the original `value`
will be returned.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT 'apple' AS example, RIGHT('apple', 3) AS right_example

    /*---+---+
     | example | right_example |
     +---+---+
     | apple   | ple           |
     +---+---*/

    SELECT b'apple' AS example, RIGHT(b'apple', 3) AS right_example

    -- Note that the result of RIGHT is of type BYTES, displayed as a base64-encoded string.
    /*---+---+
     | example  | right_example |
     +---+---+
     | YXBwbGU= | cGxl          |
     +---+---*/

## `RPAD`

    RPAD(original_value, return_length[, pattern])

**Description**

Returns a `STRING` or `BYTES` value that consists of `original_value` appended
with `pattern`. The `return_length` parameter is an
`INT64` that specifies the length of the
returned value. If `original_value` is `BYTES`,
`return_length` is the number of bytes. If `original_value` is `STRING`,
`return_length` is the number of characters.

The default value of `pattern` is a blank space.

Both `original_value` and `pattern` must be the same data type.

If `return_length` is less than or equal to the `original_value` length, this
function returns the `original_value` value, truncated to the value of
`return_length`. For example, `RPAD('hello world', 7);` returns `'hello w'`.

If `original_value`, `return_length`, or `pattern` is `NULL`, this function
returns `NULL`.

This function returns an error if:

- `return_length` is negative
- `pattern` is empty

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT FORMAT('%T', RPAD('c', 5)) AS results

    /*---+
     | results |
     +---+
     | "c    " |
     +---*/

    SELECT RPAD('b', 5, 'a') AS results

    /*---+
     | results |
     +---+
     | baaaa   |
     +---*/

    SELECT RPAD('abc', 10, 'ghd') AS results

    /*---+
     | results    |
     +---+
     | abcghdghdg |
     +---*/

    SELECT RPAD('abc', 2, 'd') AS results

    /*---+
     | results |
     +---+
     | ab      |
     +---*/

    SELECT FORMAT('%T', RPAD(b'abc', 10, b'ghd')) AS results

    /*---+
     | results       |
     +---+
     | b"abcghdghdg" |
     +---*/

## `RTRIM`

    RTRIM(value1[, value2])

**Description**

Identical to [TRIM](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim), but only removes trailing characters.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT RTRIM('***apple***', '*') AS example

    /*---+
     | example   |
     +---+
     | ***apple  |
     +---*/

    SELECT RTRIM('applexxz', 'xyz') AS example

    /*---+
     | example |
     +---+
     | apple   |
     +---*/

## `SAFE_CONVERT_BYTES_TO_STRING`

    SAFE_CONVERT_BYTES_TO_STRING(value)

**Description**

Converts a sequence of `BYTES` to a `STRING`. Any invalid UTF-8 characters are
replaced with the Unicode replacement character, `U+FFFD`.

**Return type**

`STRING`

**Examples**

The following statement returns the Unicode replacement character, �.

    SELECT SAFE_CONVERT_BYTES_TO_STRING(b'\xc2') as safe_convert;

## `SOUNDEX`

    SOUNDEX(value)

**Description**

Returns a `STRING` that represents the
[Soundex](https://en.wikipedia.org/wiki/Soundex) code for `value`.

SOUNDEX produces a phonetic representation of a string. It indexes words by
sound, as pronounced in English. It's typically used to help determine whether
two strings, such as the family names *Levine* and *Lavine* , or the words *to*
and *too*, have similar English-language pronunciation.

The result of the SOUNDEX consists of a letter followed by 3 digits. Non-latin
characters are ignored. If the remaining string is empty after removing
non-Latin characters, an empty `STRING` is returned.

**Return type**

`STRING`

**Examples**

    SELECT 'Ashcraft' AS value, SOUNDEX('Ashcraft') AS soundex

    /*---+---+
     | value                | soundex |
     +---+---+
     | Ashcraft             | A261    |
     +---+---*/

## `SPLIT`

    SPLIT(value[, delimiter])

**Description**

Splits a `STRING` or `BYTES` value, using a delimiter. The `delimiter` argument
must be a literal character or sequence of characters. You can't split with a
regular expression.

For `STRING`, the default delimiter is the comma `,`.

For `BYTES`, you must specify a delimiter.

Splitting on an empty delimiter produces an array of UTF-8 characters for
`STRING` values, and an array of `BYTES` for `BYTES` values.

Splitting an empty `STRING` returns an
`ARRAY` with a single empty
`STRING`.

This function supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return type**

`ARRAY<STRING>` or `ARRAY<BYTES>`

**Examples**

    WITH letters AS
      (SELECT '' as letter_group
      UNION ALL
      SELECT 'a' as letter_group
      UNION ALL
      SELECT 'b c d' as letter_group)

    SELECT SPLIT(letter_group, ' ') as example
    FROM letters;

    /*---+
     | example              |
     +---+
     | []                   |
     | [a]                  |
     | [b, c, d]            |
     +---*/

## `STARTS_WITH`

    STARTS_WITH(value, prefix)

**Description**

Takes two `STRING` or `BYTES` values. Returns `TRUE` if `prefix` is a
prefix of `value`.

This function supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return type**

`BOOL`

**Examples**

    SELECT STARTS_WITH('bar', 'b') AS example

    /*---+
     | example |
     +---+
     |    True |
     +---*/

## `STRPOS`

    STRPOS(value, subvalue)

**Description**

Takes two `STRING` or `BYTES` values. Returns the 1-based position of the first
occurrence of `subvalue` inside `value`. Returns `0` if `subvalue` isn't found.

This function supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return type**

`INT64`

**Examples**

    SELECT STRPOS('foo@example.com', '@') AS example

    /*---+
     | example |
     +---+
     |       4 |
     +---*/

## `SUBSTR`

    SUBSTR(value, position[, length])

**Description**

Gets a portion (substring) of the supplied `STRING` or `BYTES` value.

The `position` argument is an integer specifying the starting position of the
substring.

- If `position` is `1`, the substring starts from the first character or byte.
- If `position` is `0` or less than `-LENGTH(value)`, `position` is set to `1`, and the substring starts from the first character or byte.
- If `position` is greater than the length of `value`, the function produces an empty substring.
- If `position` is negative, the function counts from the end of `value`, with `-1` indicating the last character or byte.

The `length` argument specifies the maximum number of characters or bytes to
return.

- If `length` isn't specified, the function produces a substring that starts at the specified position and ends at the last character or byte of `value`.
- If `length` is `0`, the function produces an empty substring.
- If `length` is negative, the function produces an error.
- The returned substring may be shorter than `length`, for example, when `length` exceeds the length of `value`, or when the starting position of the substring plus `length` is greater than the length of `value`.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT SUBSTR('apple', 2) AS example

    /*---+
     | example |
     +---+
     | pple    |
     +---*/

    SELECT SUBSTR('apple', 2, 2) AS example

    /*---+
     | example |
     +---+
     | pp      |
     +---*/

    SELECT SUBSTR('apple', -2) AS example

    /*---+
     | example |
     +---+
     | le      |
     +---*/

    SELECT SUBSTR('apple', 1, 123) AS example

    /*---+
     | example |
     +---+
     | apple   |
     +---*/

    SELECT SUBSTR('apple', 123) AS example

    /*---+
     | example |
     +---+
     |         |
     +---*/

    SELECT SUBSTR('apple', 123, 5) AS example

    /*---+
     | example |
     +---+
     |         |
     +---*/

## `SUBSTRING`

    SUBSTRING(value, position[, length])

Alias for [`SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr).

## `TO_BASE32`

    TO_BASE32(bytes_expr)

**Description**

Converts a sequence of `BYTES` into a base32-encoded `STRING`. To convert a
base32-encoded `STRING` into `BYTES`, use [FROM_BASE32](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base32).

**Return type**

`STRING`

**Example**

    SELECT TO_BASE32(b'abcde\xFF') AS base32_string;

    /*---+
     | base32_string    |
     +---+
     | MFRGGZDF74====== |
     +---*/

## `TO_BASE64`

    TO_BASE64(bytes_expr)

**Description**

Converts a sequence of `BYTES` into a base64-encoded `STRING`. To convert a
base64-encoded `STRING` into `BYTES`, use [FROM_BASE64](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base64).

There are several base64 encodings in common use that vary in exactly which
alphabet of 65 ASCII characters are used to encode the 64 digits and padding.
See [RFC 4648](https://tools.ietf.org/html/rfc4648#section-4) for details. This
function adds padding and uses the alphabet `[A-Za-z0-9+/=]`.

**Return type**

`STRING`

**Example**

    SELECT TO_BASE64(b'\377\340') AS base64_string;

    /*---+
     | base64_string |
     +---+
     | /+A=          |
     +---*/

To work with an encoding using a different base64 alphabet, you might need to
compose `TO_BASE64` with the `REPLACE` function. For instance, the
`base64url` url-safe and filename-safe encoding commonly used in web programming
uses `-_=` as the last characters rather than `+/=`. To encode a
`base64url`-encoded string, replace `+` and `/` with `-` and `_` respectively.

    SELECT REPLACE(REPLACE(TO_BASE64(b'\377\340'), '+', '-'), '/', '_') as websafe_base64;

    /*---+
     | websafe_base64 |
     +---+
     | _-A=           |
     +---*/

## `TO_CODE_POINTS`

    TO_CODE_POINTS(value)

**Description**

Takes a `STRING` or `BYTES` value and returns an array of `INT64` values that
represent code points or extended ASCII character values.

- If `value` is a `STRING`, each element in the returned array represents a [code point](https://en.wikipedia.org/wiki/Code_point). Each code point falls within the range of \[0, 0xD7FF\] and \[0xE000, 0x10FFFF\].
- If `value` is `BYTES`, each element in the array is an extended ASCII character value in the range of \[0, 255\].

To convert from an array of code points to a `STRING` or `BYTES`, see
[CODE_POINTS_TO_STRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string) or
[CODE_POINTS_TO_BYTES](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_bytes).

**Return type**

`ARRAY<INT64>`

**Examples**

The following examples get the code points for each element in an array of
words.

    SELECT
      'foo' AS word,
      TO_CODE_POINTS('foo') AS code_points

    /*---+---+
     | word    | code_points                        |
     +---+---+
     | foo     | [102, 111, 111]                    |
     +---+---*/

    SELECT
      'bar' AS word,
      TO_CODE_POINTS('bar') AS code_points

    /*---+---+
     | word    | code_points                        |
     +---+---+
     | bar     | [98, 97, 114]                      |
     +---+---*/

    SELECT
      'baz' AS word,
      TO_CODE_POINTS('baz') AS code_points

    /*---+---+
     | word    | code_points                        |
     +---+---+
     | baz     | [98, 97, 122]                      |
     +---+---*/

    SELECT
      'giraffe' AS word,
      TO_CODE_POINTS('giraffe') AS code_points

    /*---+---+
     | word    | code_points                        |
     +---+---+
     | giraffe | [103, 105, 114, 97, 102, 102, 101] |
     +---+---*/

    SELECT
      'llama' AS word,
      TO_CODE_POINTS('llama') AS code_points

    /*---+---+
     | word    | code_points                        |
     +---+---+
     | llama   | [108, 108, 97, 109, 97]            |
     +---+---*/

The following examples convert integer representations of `BYTES` to their
corresponding ASCII character values.

    SELECT
      b'\x66\x6f\x6f' AS bytes_value,
      TO_CODE_POINTS(b'\x66\x6f\x6f') AS bytes_value_as_integer

    /*---+---+
     | bytes_value      | bytes_value_as_integer |
     +---+---+
     | foo              | [102, 111, 111]        |
     +---+---*/

    SELECT
      b'\x00\x01\x10\xff' AS bytes_value,
      TO_CODE_POINTS(b'\x00\x01\x10\xff') AS bytes_value_as_integer

    /*---+---+
     | bytes_value      | bytes_value_as_integer |
     +---+---+
     | \x00\x01\x10\xff | [0, 1, 16, 255]        |
     +---+---*/

The following example demonstrates the difference between a `BYTES` result and a
`STRING` result. Notice that the character `Ā` is represented as a two-byte
Unicode sequence. As a result, the `BYTES` version of `TO_CODE_POINTS` returns
an array with two elements, while the `STRING` version returns an array with a
single element.

    SELECT TO_CODE_POINTS(b'Ā') AS b_result, TO_CODE_POINTS('Ā') AS s_result;

    /*---+---+
     | b_result   | s_result |
     +---+---+
     | [196, 128] | [256]    |
     +---+---*/

## `TO_HEX`

    TO_HEX(bytes)

**Description**

Converts a sequence of `BYTES` into a hexadecimal `STRING`. Converts each byte
in the `STRING` as two hexadecimal characters in the range
`(0..9, a..f)`. To convert a hexadecimal-encoded
`STRING` to `BYTES`, use [FROM_HEX](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_hex).

**Return type**

`STRING`

**Example**

    SELECT
      b'\x00\x01\x02\x03\xAA\xEE\xEF\xFF' AS byte_string,
      TO_HEX(b'\x00\x01\x02\x03\xAA\xEE\xEF\xFF') AS hex_string

    /*---+---+
     | byte_string                      | hex_string       |
     +---+---+
     | \x00\x01\x02\x03\xaa\xee\xef\xff | 00010203aaeeefff |
     +---+---*/

## `TRANSLATE`

    TRANSLATE(expression, source_characters, target_characters)

**Description**

In `expression`, replaces each character in `source_characters` with the
corresponding character in `target_characters`. All inputs must be the same
type, either `STRING` or `BYTES`.

- Each character in `expression` is translated at most once.
- A character in `expression` that isn't present in `source_characters` is left unchanged in `expression`.
- A character in `source_characters` without a corresponding character in `target_characters` is omitted from the result.
- A duplicate character in `source_characters` results in an error.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT TRANSLATE('This is a cookie', 'sco', 'zku') AS translate

    /*---+
     | translate        |
     +---+
     | Thiz iz a kuukie |
     +---*/

## `TRIM`

    TRIM(value_to_trim[, set_of_characters_to_remove])

**Description**

Takes a `STRING` or `BYTES` value to trim.

If the value to trim is a `STRING`, removes from this value all leading and
trailing Unicode code points in `set_of_characters_to_remove`.
The set of code points is optional. If it isn't specified, all
whitespace characters are removed from the beginning and end of the
value to trim.

If the value to trim is `BYTES`, removes from this value all leading and
trailing bytes in `set_of_characters_to_remove`. The set of bytes is required.

**Return type**

- `STRING` if `value_to_trim` is a `STRING` value.
- `BYTES` if `value_to_trim` is a `BYTES` value.

**Examples**

In the following example, all leading and trailing whitespace characters are
removed from `item` because `set_of_characters_to_remove` isn't specified.

    SELECT CONCAT('#', TRIM( '   apple   '), '#') AS example

    /*---+
     | example  |
     +---+
     | #apple#  |
     +---*/

In the following example, all leading and trailing `*` characters are removed
from '***apple***'.

    SELECT TRIM('***apple***', '*') AS example

    /*---+
     | example |
     +---+
     | apple   |
     +---*/

In the following example, all leading and trailing `x`, `y`, and `z` characters
are removed from 'xzxapplexxy'.

    SELECT TRIM('xzxapplexxy', 'xyz') as example

    /*---+
     | example |
     +---+
     | apple   |
     +---*/

In the following example, examine how `TRIM` interprets characters as
Unicode code-points. If your trailing character set contains a combining
diacritic mark over a particular letter, `TRIM` might strip the
same diacritic mark from a different letter.

    SELECT
      TRIM('abaW̊', 'Y̊') AS a,
      TRIM('W̊aba', 'Y̊') AS b,
      TRIM('abaŪ̊', 'Y̊') AS c,
      TRIM('Ū̊aba', 'Y̊') AS d

    /*---+---+---+---+
     | a    | b    | c    | d    |
     +---+---+---+---+
     | abaW | W̊aba | abaŪ | Ūaba |
     +---+---+---+---*/

In the following example, all leading and trailing `b'n'`, `b'a'`, `b'\xab'`
bytes are removed from `item`.

    SELECT b'apple', TRIM(b'apple', b'na\xab') AS example

    -- Note that the result of TRIM is of type BYTES, displayed as a base64-encoded string.
    /*---+---+
     | item                 | example          |
     +---+---+
     | YXBwbGU=             | cHBsZQ==         |
     +---+---*/

## `UNICODE`

    UNICODE(value)

**Description**

Returns the Unicode [code point](https://en.wikipedia.org/wiki/Code_point) for the first character in
`value`. Returns `0` if `value` is empty, or if the resulting Unicode code
point is `0`.

**Return type**

`INT64`

**Examples**

    SELECT UNICODE('âbcd') as A, UNICODE('â') as B, UNICODE('') as C, UNICODE(NULL) as D;

    /*---+---+---+---+
     | A     | B     | C     | D     |
     +---+---+---+---+
     | 226   | 226   | 0     | NULL  |
     +---+---+---+---*/

## `UPPER`

    UPPER(value)

**Description**

For `STRING` arguments, returns the original string with all alphabetic
characters in uppercase. Mapping between uppercase and lowercase is done
according to the
[Unicode Character Database](http://unicode.org/ucd/)
without taking into account language-specific mappings.

For `BYTES` arguments, the argument is treated as ASCII text, with all bytes
greater than 127 left intact.

**Return type**

`STRING` or `BYTES`

**Examples**

    SELECT UPPER('foo') AS example

    /*---+
     | example |
     +---+
     | FOO     |
     +---*/