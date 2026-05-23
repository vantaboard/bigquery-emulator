# The ML.NGRAMS function

This document describes the `ML.NGRAMS` function, which lets you create
[n-grams](https://wikipedia.org/wiki/N-gram) of the input values.

You can use this function with models that support
[manual feature preprocessing](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing). For more
information, see the following documents:

- [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)
- [Contribution analysis user journey](https://docs.cloud.google.com/bigquery/docs/contribution-analysis#contribution_analysis_user_journey)).

## Syntax

```sql
ML.NGRAMS(array_input, range [, separator])
```

### Arguments

`ML.NGRAMS` takes the following arguments:

- `array_input`: an `ARRAY<STRING>` value that represent the tokens to be merged.
- `range`: an `ARRAY` of two `INT64` elements or a single `INT64` value. If you specify an `ARRAY` value, the `INT64` elements provide the range of n-gram sizes to return. Provide the numerical values in order, lower to higher. If you specify a single `INT64` value of *x* , the range of n-gram sizes to return is `[x, x]`.
- `separator`: a `STRING` value that specifies the separator to connect two adjacent tokens in the output. The default value is whitespace .

## Output

`ML.NGRAMS` returns an `ARRAY<STRING>` value that contain the n-grams.

## Example

The following example outputs all possible 2-token and 3-token combinations
for a set of three input strings:

```sql
SELECT
  ML.NGRAMS(['a', 'b', 'c'], [2,3], '#') AS output;
```

The output looks similar to the following:

```
+---+
|        output         |
+---+
| ["a#b","a#b#c","b#c"] |
+---+
```

## What's next

- For information about feature preprocessing, see [Feature preprocessing overview](https://docs.cloud.google.com/bigquery/docs/preprocess-overview).