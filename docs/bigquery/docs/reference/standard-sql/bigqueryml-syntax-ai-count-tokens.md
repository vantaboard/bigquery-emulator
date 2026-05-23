# The AI.COUNT_TOKENS function

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To request support or provide feedback for this feature, contact [bqml-feedback@google.com](mailto:bqml-feedback@google.com).

This document describes the `AI.COUNT_TOKENS` function, which estimates the
token count of text input that you provide.

    # The result is approximately 11.
    SELECT
      AI.COUNT_TOKENS("Token count isn't always equal to word count.").result AS num_tokens;

Tokens are units of text that a generative AI model uses to
process input. You can use the token count of a prompt to help you estimate
the cost of calling AI functions with that prompt.
Token counting happens in
BigQuery. This function doesn't incur
charges in Vertex AI.

The `AI.COUNT_TOKENS`
function estimates the input token count. The number of thinking or output
tokens isn't included in the result. To see the actual number of each type
of token processed by a query,
[view your token counts](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#token_usage)
in the **Job information** tab of the **Query results** pane.

## Syntax

```googlesql
AI.COUNT_TOKENS(
  INPUT,
  [, endpoint => ENDPOINT ]
)
```

### Arguments

`AI.COUNT_TOKENS` takes the following arguments:

- `INPUT`: a `STRING` value that contains the input text prompt for which to count tokens.
- `ENDPOINT`: a `STRING` literal that specifies the name of the generative AI model to use for tokenization rules. If you don't provide an endpoint, then the default model used by the [`AI.GENERATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate) is used.

## Output

`AI.COUNT_TOKENS` returns a `STRUCT` value that contains the following fields:

- `result`: An `INT64` value that contains the total token count for the input. This value is `NULL` if the input is `NULL` or an API error occurred.
- `full_response`: A `JSON` value that provides the modality and token count of the input. This value is `NULL` if the input is `NULL` or an API error occurred.

## Examples

The following example counts tokens in sample reviews from the
`bigquery-public-data.imdb.reviews` table:

    SELECT
      review,
      AI.COUNT_TOKENS(review, endpoint => 'gemini-2.5-flash').*
    FROM
      `bigquery-public-data.imdb.reviews`
    LIMIT 2;

The result is similar to the following:

    +---+---+---+
    | review                                 | result | full_response                               |
    +---+---+---+
    | I think the manuscript of the movie... | 180    | {"promptTokensDetails":[{"modality":"TEXT", |
    |                                        |        | "tokenCount":180}],"totalTokens":180}       |
    | Or that's what the filmmakers would... | 246    | {"promptTokensDetails":[{"modality":"TEXT", |
    |                                        |        | "tokenCount":246}],"totalTokens":246}       |
    +---+---+---+

## What's next

- Learn more about [generative AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).