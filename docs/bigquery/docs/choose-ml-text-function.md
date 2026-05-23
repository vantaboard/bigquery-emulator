# Choose a natural language processing function

This document provides a comparison of the natural language processing functions
available in BigQuery ML, which are
[`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text),
[`ML.TRANSLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-translate),
and
[`ML.UNDERSTAND_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-understand-text).
You can use the information in this document to help you decide which function
to use in cases where the functions have overlapping capabilities.

At a high level, the difference between these functions is as follows:

- `AI.GENERATE_TEXT` is a good choice for performing customized natural language processing (NLP) tasks at a lower cost. This function offers more language support, faster throughput, and model tuning capability, and also works with multimodal models.
- `ML.TRANSLATE` is a good choice for performing translation-specific NLP tasks where you need to support a high rate of queries per minute.
- `ML.UNDERSTAND_TEXT` is a good choice for performing NLP tasks that are supported by the Cloud Natural Language API.

## Function comparison

Use the following table to compare the `AI.GENERATE_TEXT`,
`ML.TRANSLATE`, and `ML.UNDERSTAND_TEXT` functions:

|   | `AI.GENERATE_TEXT` | `ML.TRANSLATE` | `ML.UNDERSTAND_TEXT` |
|---|---|---|---|
| Purpose | Perform any NLP task by passing a prompt to a [Gemini or partner model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) or to an [open model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open). For example, to perform a question answering task, you could provide a prompt similar to `CONCAT("What are the key concepts in the following article?: ", article_text)`. | Use the [Cloud Translation API](https://docs.cloud.google.com/translate) to perform the following tasks: - [`TRANSLATE_TEXT`](https://docs.cloud.google.com/translate/docs/advanced/translating-text-v3) - [`DETECT_LANGUAGE`](https://docs.cloud.google.com/translate/docs/advanced/detecting-language-v3) | Use the [Cloud Natural Language API](https://docs.cloud.google.com/natural-language) to perform the following tasks: - [`ANALYZE_ENTITIES`](https://docs.cloud.google.com/natural-language/docs/analyzing-entities) - [`ANALYZE_ENTITY_SENTIMENT`](https://docs.cloud.google.com/natural-language/docs/analyzing-entity-sentiment) - [`ANALYZE_SENTIMENT`](https://docs.cloud.google.com/natural-language/docs/analyzing-sentiment) - [`ANALYZE_SYNTAX`](https://docs.cloud.google.com/natural-language/docs/analyzing-syntax) - [`CLASSIFY_TEXT`](https://docs.cloud.google.com/natural-language/docs/classifying-text) |
| Billing | Incurs BigQuery ML charges for data processed. For more information, see [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing). Incurs Vertex AI charges for calls to the model. If you are using a Gemini 2.0 or greater model, the call is billed at the batch API rate. For more information, see [Cost of building and deploying AI models in Vertex AI](https://docs.cloud.google.com/vertex-ai/generative-ai/pricing). | Incurs BigQuery ML charges for data processed. For more information, see [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing). Incurs charges for calls to the Cloud Translation API. For more information, see [Cloud Translation API pricing](https://docs.cloud.google.com/translate/pricing). | Incurs BigQuery ML charges for data processed. For more information, see [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing). Incurs charges for calls to the Cloud Natural Language API. For more information, see [Cloud Natural Language API pricing](https://docs.cloud.google.com/translate/pricing). |
| Requests per minute | Not applicable for Gemini models. Between 25 and 60 for partner models. For more information, see [Requests per minute limits](https://docs.cloud.google.com/bigquery/quotas#requests_per_minute_limits). | 200. For more information, see [Cloud AI service functions](https://docs.cloud.google.com/bigquery/quotas#cloud_ai_service_functions). | 600. For more information, see [Cloud AI service functions](https://docs.cloud.google.com/bigquery/quotas#cloud_ai_service_functions). |
| Tokens per minute | Ranges from 8,192 to over 1 million, depending on the model used. | No token limit. However, `ML_TRANSLATE` does have a [30,000 bytes limit](https://docs.cloud.google.com/translate/quotas#content-limit). | [100,000](https://docs.cloud.google.com/natural-language/quotas#content). |
| Input data | Supports both text and unstructured data from BigQuery standard tables and object tables. | Supports text data from BigQuery standard tables. | Supports text data from BigQuery standard tables. |
| Function output | Output can vary for calls to the model, even with the same prompt. | Produces the same output for a given task type for each successful call to the API. The output includes information about the input language. | Produces the same output for a given task type for each successful call to the API. The output includes information about the magnitude of the sentiment for sentiment analysis tasks. |
| Data context | You can provide data context as part of the prompt you submit. | Not supported. | Not supported. |
| Supervised tuning | [Supervised tuning](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#languages-gemini) is supported for some models. | Not supported. | Not supported. |
| Supported languages | Support varies based on the LLM you choose. | Supports Cloud Translation API [languages](https://docs.cloud.google.com/translate/docs/languages). | Supports Cloud Natural Language API [languages](https://docs.cloud.google.com/natural-language/docs/languages). |
| Supported regions | Supported in all Generative AI for Vertex AI [regions](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#available-regions). | Supported in the `EU` and `US` multi-regions. | Supported in the `EU` and `US` multi-regions. |