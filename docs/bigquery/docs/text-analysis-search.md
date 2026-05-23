# Work with text analyzers

The [`CREATE SEARCH INDEX` DDL
statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement),
[`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions), and
[`TEXT_ANALYZE`
function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#text_analyze)
support advanced text analyzer configuration options. Understanding
BigQuery's text analyzers and their options lets you refine your search
experience.

This document provides an overview of the different text analyzers available in
BigQuery and their configuration options, as well as examples of
how text analyzers work with [search](https://docs.cloud.google.com/bigquery/docs/search) in
BigQuery. For more information about text analyzer syntax, see
[Text analysis](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis).

## Text analyzers

BigQuery supports the following text analyzers:

- `NO_OP_ANALYZER`
- `LOG_ANALYZER`
- `PATTERN_ANALYZER`

### `NO_OP_ANALYZER`

Use the `NO_OP_ANALYZER` when you have pre-processed data that you want to match
exactly. There is no tokenization or normalization applied to the text. Since
this analyzer does not perform tokenization or normalization, it accepts no
configuration. For more information about
`NO_OP_ANALYZER`, see
[`NO_OP_ANALYZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#no_op_analyzer).

### `LOG_ANALYZER`

The `LOG_ANALYZER` modifies data in the following ways:

- Text is made lowercase.
- ASCII values greater than 127 are kept as is.

- Text is split into individual terms called *tokens* by the following
  delimiters:

      [ ] < > ( ) { } | ! ; , ' " * & ? + / : = @ . - $ % \ _ \n \r \s \t %21 %26
      %2526 %3B %3b %7C %7c %20 %2B %2b %3D %3d %2520 %5D %5d %5B %5b %3A %3a %0A
      %0a %2C %2c %28 %29

  If you don't want to use the default delimiters, you can specify the
  delimiters you want to use as text analyzer options. `LOG_ANALYZER` lets you
  configure specific delimiters and token filters for more control over your
  search results. For more information about the
  specific configuration options available when using the `LOG_ANALYZER`, see
  [`delimiters` analyzer
  option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#log_analyzer_options)
  and [`token_filters` analyzer
  option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#token_filters_option).

### `PATTERN_ANALYZER`

The `PATTERN_ANALYZER` text analyzer extracts tokens from text using a regular
expression. The regular expression engine and syntax used with
`PATTERN_ANALYZER` is [RE2](https://github.com/google/re2/). `PATTERN_ANALYZER`
tokenizes patterns in the following order:

1. It finds the first substring that matches the pattern (from the left) in the string. This is a token to be included in the output.
2. It removes everything from the input string until the end of the substring found in step 1.
3. It repeats the process until the string is empty.

The following table provides examples of `PATTERN_ANALYZER` token extraction:

| Pattern | Input text | Output tokens |
|---|---|---|
| ab | ababab | - ab |
| ab | abacad | - ab |
| \[a-z\]{2} | abacad | - ab - ac - ad |
| aaa | aaaaa | - aaa |
| \[a-z\]/ | a/b/c/d/e | - a/ - b/ - c/ - d/ |
| /\[\^/\]+/ | aa/bb/cc | - /bb/ |
| \[0-9\]+ | abc |   |
| (?:/?)\[a-z\] | /abc | - /abc |
| (?:/)\[a-z\] | /abc | - /abc |
| (?:\[0-9\]abc){3}(?:\[a-z\]000){2} | 7abc7abc7abcx000y000 | - 7abc7abc7abcx000y000 |
| ".+" | "cats" and "dogs" | - "cats" and "dogs" <br /> <br /> Note the use of [greedy quantifiers +](https://stackoverflow.com/questions/2301285/what-do-lazy-and-greedy-mean-in-the-context-of-regular-expressions) makes the match to match the longest string possible in the text, causing '"cats" and "dogs"' to be extracted as a token in the text. |
| ".+?" | "cats" and "dogs" | - "cats" - "dogs" <br /> <br /> Note the use of [lazy quantifiers +?](https://stackoverflow.com/questions/2301285/what-do-lazy-and-greedy-mean-in-the-context-of-regular-expressions) makes the regular expression match the shortest string possible in the text, causing '"cats"', '"dogs"' to be extracted as 2 separate tokens in the text. |

Using the `PATTERN_ANALYZER` text analyzer gives you more control over the
tokens extracted from a text when used with the [`SEARCH`
function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions). The following
table shows how different patterns and results result in different `SEARCH`
results:

| Pattern | Query | Text | Tokens from text | SEARCH(text, query) | Explanation |
|---|---|---|---|---|---|
| abc | abcdef | abcghi | - abcghi | TRUE | 'abc' in \['abcghi'\] |
| cd\[a-z\] | abcdef | abcghi | - abcghi | FALSE | 'cde' in \['abcghi'\] |
| \[a-z\]/ | a/b/ | a/b/c/d/ | - a/ - b/ - c/ - d/ | TRUE | 'a/' in \['a/', 'b/', 'c/', 'd/'\] AND 'b/' in \['a/', 'b/', 'c/', 'd/'\] |
| /\[\^/\]+/ | aa/bb/ | aa/bb/cc/ | - /bb/ | TRUE | '/bb/' in \['/bb/'\] |
| /\[\^/\]+/ | bb | aa/bb/cc/ | - /bb/ | ERROR | No match found in query term |
| \[0-9\]+ | abc | abc123 |   | ERROR | No match found in query term |
| \[0-9\]+ | \`abc\` | abc123 |   | ERROR | No match found in query term <br /> Matching backtick as backtick, not a special character. |
| \[a-z\]\[a-z0-9\]\*@google\\.com | This is my email: test@google.com | test@google.com | - test@google.com | TRUE | 'test@google.com' in 'test@google.com' |
| abc | abc\\ abc | abc | - abc | TRUE | 'abc' in \['abc'\] <br /> Note that 'abc abc' is a single subquery(ie) after being parsed by the search query parser since the space is escaped. |
| (?i)(?:Abc) (no normalization) | aBcd | Abc | - Abc | FALSE | 'aBc' in \['Abc'\] |
| (?i)(?:Abc) <br /> normalization: lower_case = true | aBcd | Abc | - abc | TRUE | 'abc' in \['abc'\] |
| (?:/?)abc | bc/abc | /abc/abc/ | - /abc | TRUE | '/abc' in \['/abc'\] |
| (?:/?)abc | abc | d/abc | - /abc | FALSE | 'abc' in \['/abc'\] |
| ".+" | "cats" | "cats" and "dogs" | - "cats" and "dogs" | FALSE | '"cats"' in \['"cats" and "dogs"\] <br /> Note the use of [greedy quantifiers +](https://stackoverflow.com/questions/2301285/what-do-lazy-and-greedy-mean-in-the-context-of-regular-expressions) makes the regular expression match the longest string possible in the text, causing '"cats" and "dogs"' to be extracted as a token in the text. |
| ".+?" | "cats" | "cats" and "dogs" | - "cats" - "dogs" | TRUE | '"cats"' in \['"cats"', '"dogs"\] <br /> Note the use of [lazy quantifiers +?](https://stackoverflow.com/questions/2301285/what-do-lazy-and-greedy-mean-in-the-context-of-regular-expressions) makes the regular expression match the shortest string possible in the text, causing '"cats"', '"dogs"' to be extracted as 2 separate tokens in the text. |

## Examples

The following examples demonstrates the use of text analysis
with customization options to create search indexes, extract tokens, and return
search results.

### `LOG_ANALYZER` with NFKC ICU normalization and stop words

The following example configures `LOG_ANALYZER` options with [NFKC ICU](https://en.wikipedia.org/wiki/Unicode_equivalence)
normalization and stop words. The example assumes the following data table with
data already populated:

```googlesql
CREATE TABLE dataset.data_table(
  text_data STRING
);
```

To create a search index with NFKC ICU normalization and a list of stop words,
create a JSON-formatted string in the `analyzer_options` option of the [`CREATE
SEARCH INDEX` DDL
statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement).
For a complete list of options available in when creating a search index with
the `LOG_ANALYZER`, see
[`LOG_ANALYZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#log_analyzer).
For this example, our stop words are `"the", "of", "and", "for"`.

```googlesql
CREATE OR REPLACE SEARCH INDEX `my_index` ON `dataset.data_table`(ALL COLUMNS) OPTIONS(
  analyzer='PATTERN_ANALYZER',
  analyzer_options= '''{
    "token_filters": [
      {
        "normalizer": {
          "mode": "ICU_NORMALIZE",
          "icu_normalize_mode": "NFKC",
          "icu_case_folding": true
        }
      },
      { "stop_words": ["the", "of", "and", "for"] }
    ]
  }''');
```

Given the previous example, the following table describes the token extraction
for various values of `text_data`. Note that in this document the double
question mark character (*⁇*) has been italicized to differentiate between
two question marks (??):

| Data Text | Tokens for index | Explanation |
|---|---|---|
| The Quick Brown Fox | \["quick", "brown", "fox"\] | LOG_ANALYZER tokenization produces the tokens \["The", "Quick", "Brown", "Fox"\]. <br /> Next, ICU normalization with `icu_case_folding = true` lower cases the tokens to produce \["the", "quick", "brown", "fox"\] <br /> Finally, the stop words filter removes "the" from the list. |
| The Ⓠuick Ⓑrown Ⓕox | \["quick", "brown", "fox"\] | LOG_ANALYZER tokenization produces the tokens \["The", "Ⓠuick", "Ⓑrown", "Ⓕox"\]. <br /> Next, NFKC ICU normalization with `icu_case_folding = true` lower cases the tokens to produce \["the", "quick", "brown", "fox"\] <br /> Finally, the stop words filter removes "the" from the list. |
| Ⓠuick*⁇*Ⓕox | \["quick??fox"\] | LOG_ANALYZER tokenization produces the tokens \["The", "Ⓠuick*⁇*Ⓕox"\]. <br /> Next, NFKC ICU normalization with `icu_case_folding = true` lower cases the tokens to produce \["quick??fox"\]. Notice that the double question mark unicode has been normalized into 2 question mark ASCII characters. <br /> Finally, the stop words filter does nothing because none of the tokens are in the filter list. |

Now that the search index has been created, you can use the [`SEARCH`
function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions) to search the
table using the same analyzer configurations specified in the search index. Note
that if the analyzer configurations in the `SEARCH` function don't match those
of the search index, the search index won't be used. Use the following query:

```googlesql
SELECT
  SEARCH(
  analyzer => 'LOG_ANALYZER',
  analyzer_options => '''{
    "token_filters": [
      {
        "normalizer": {
          "mode": "ICU_NORMALIZE",
          "icu_normalize_mode": "NFKC",
          "icu_case_folding": true
        }
      },
      {
        "stop_words": ["the", "of", "and", "for"]
      }
    ]
  }''')
```

Replace the following:

- <var translate="no">`search_query`</var>: The text you want to search for.

The following
table demonstrates various results based on different search text and different
values of `search_query`:

| text_data | `search_query` | Result | Explanation |
|---|---|---|---|
| The Quick Brown Fox | `"Ⓠuick"` | `TRUE` | The final list of tokens extracted from the text is \["quick", "brown", "fox"\]. The final list of tokens extracted from the text query is \["quick"\]. <br /> The list query tokens can all be found in the text tokens. |
| The Ⓠuick Ⓑrown Ⓕox | `"quick"` | `TRUE` | The final list of tokens extracted from the text is \["quick", "brown", "fox"\]. The final list of tokens extracted from the text query is \["quick"\]. <br /> The list query tokens can all be found in the text tokens. |
| Ⓠuick*⁇*Ⓕox | `"quick"` | `FALSE` | The final list of tokens extracted from the text is \["quick??fox"\]. <br /> The final list of tokens extracted from the text query is \["quick"\]. <br /> "quick" is not in the list of tokens from the text. |
| Ⓠuick*⁇*Ⓕox | `"quick*⁇*fox"` | `TRUE` | The final list of tokens extracted from the text is \["quick??fox"\]. <br /> The final list of tokens extracted from the text query is \["quick??fox"\]. <br /> "quick??fox" is in the list of tokens from the text. |
| Ⓠuick*⁇*Ⓕox | ``"`quick*⁇*fox`"`` | `FALSE` | In `LOG_ANALYZER`, backtick requires exact text match. |

### `PATTERN_ANALYZER` for IPv4 search with stop words

The following example configures the `PATTERN_ANALYZER` text analyzer to search for a specific pattern while filtering certain stop words. In this example, the pattern matches an IPv4 address and ignores the localhost value (`127.0.0.1`).

This example assumes that the following table is populated with data:

```googlesql
CREATE TABLE dataset.data_table(
  text_data STRING
);
```

To create a search index the `pattern` option and a list of stop words, create a
JSON-formatted string in the `analyzer_options` option of the [`CREATE SEARCH
INDEX` DDL
statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement).
For a complete list of options available in when creating a search index with
the `PATTERN_ANALYZER`, see
[`PATTERN_ANALYZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#pattern_analyzer).
For this example, our stop words are the localhost address,
`127.0.0.1`.

```googlesql
CREATE SEARCH INDEX my_index
ON dataset.data_table(text_data)
OPTIONS (analyzer = 'PATTERN_ANALYZER', analyzer_options = '''{
  "patterns": [
    "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)[.]){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
  ],
  "token_filters": [
    {
      "stop_words": [
        "127.0.0.1"
      ]
    }
  ]
}'''
);
```

When using regular expressions with `analyzer_options`, include three
leading `\` symbols to properly escape regular expressions that include a
`\` symbol, such as `\d` or `\b`.

The following table describes the tokenization options for various values of `text_data`

| Data Text | Tokens for index | Explanation |
|---|---|---|
| abc192.168.1.1def 172.217.20.142 | \["192.168.1.1", "172.217.20.142"\] | The IPv4 patterns capture the IPv4 addresses even if there's no space between the address and the text. |
| 104.24.12.10abc 127.0.0.1 | \["104.24.12.10"\] | "127.0.0.1" is filtered out since it's in the list of stop words. |

Now that the search index has been created, you can use the [`SEARCH`
function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions) to search the
table based on the tokenization specified in `analyzer_options`. Use the
following query:

```googlesql
SELECT
  SEARCH(dataset.data_table.text_data
  "search_data",
  analyzer => 'PATTERN_ANALYZER',
  analyzer_options => '''{
    "patterns": [
      "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)[.]){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
      ],
    "token_filters": [
      {
        "stop_words": [
          "127.0.0.1"
        ]
      }
    ]
  }'''
);
```

Replace the following:

- <var translate="no">`search_query`</var>: The text you want to search for.

The following
table demonstrates various results based on different search text and different
values of `search_query`:

| text_data | `search_query` | Result | Explanation |
|---|---|---|---|
| 128.0.0.2 | "127.0.0.1" | ERROR | No search token in query. <br /> The query goes through the text analyzer, which filters out the "127.0.0.1" token. |
| abc192.168.1.1def 172.217.20.142 | "192.168.1.1abc" | TRUE | The list of tokens extracted from the query is \["192.168.1.1"\]. <br /> The list of tokens extracted from text is \["192.168.1.1", "172.217.20.142"\]. |
| abc192.168.1.1def 172.217.20.142 | "\`192.168.1.1\`" | TRUE | The list of tokens extracted from the query is \["192.168.1.1"\]. <br /> The list of tokens extracted from text is \["192.168.1.1", "172.217.20.142"\]. <br /> Note that backticks are treated as regular characters for PATTERN_ANALYZER. |

## What's next

- For an overview of search index use cases, pricing, required permissions, and limitations, see the [Introduction to search in
  BigQuery](https://docs.cloud.google.com/bigquery/docs/search-intro).
- For information about efficient searching of indexed columns, see [Search with an index](https://docs.cloud.google.com/bigquery/docs/search).