# Choose a document processing function

This document provides a comparison of the document processing functions
available in BigQuery ML, which are
[`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
and
[`ML.PROCESS_DOCUMENT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-process-document).
You can use the information in this document to help you decide which function
to use in cases where the functions have overlapping capabilities.

At a high level, the difference between these functions is as follows:

- `AI.GENERATE_TEXT` is a good choice for performing natural
  language processing (NLP) tasks where some of the content resides in
  documents. This function offers the following benefits:

  - Lower costs
  - More language support
  - Faster throughput
  - Model tuning capability
  - Availability of multimodal models

  For examples of document processing tasks that work best with this
  approach, see
  [Explore document processing capabilities with the Gemini API](https://ai.google.dev/gemini-api/docs/document-processing).
- `ML.PROCESS_DOCUMENT` is a good choice for performing document processing
  tasks that require document parsing and a predefined, structured response.

## Function comparison

Use the following table to compare the `AI.GENERATE_TEXT` and
`ML.PROCESS_DOCUMENT` functions:

|   | `AI.GENERATE_TEXT` | `ML.PROCESS_DOCUMENT` |
|---|---|---|
| Purpose | Perform any document-related NLP task by passing a prompt to a [Gemini or partner model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) or to an [open model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open). For example, given a financial document for a company, you can retrieve document information by providing a prompt such as `What is the quarterly revenue for each division?`. | Use the [Document AI API](https://docs.cloud.google.com/document-ai) to perform specialized document processing for different document types, such as invoices, tax forms, and financial statements. You can also perform document chunking. |
| Billing | Incurs BigQuery ML charges for data processed. For more information, see [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing). Incurs Vertex AI charges for calls to the model. If you are using a Gemini 2.0 or greater model, the call is billed at the batch API rate. For more information, see [Cost of building and deploying AI models in Vertex AI](https://docs.cloud.google.com/vertex-ai/generative-ai/pricing). | Incurs BigQuery ML charges for data processed. For more information, see [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing). <br /> Incurs charges for calls to the Document AI API. For more information, see [Document AI API pricing](https://docs.cloud.google.com/document-ai/pricing). |
| Requests per minute (RPM) | Not applicable for Gemini models. Between 25 and 60 for partner models. For more information, see [Requests per minute limits](https://docs.cloud.google.com/bigquery/quotas#requests_per_minute_limits). | 120 RPM per processor type, with an overall limit of 600 RPM per project. For more information, see [Quotas list](https://docs.cloud.google.com/document-ai/quotas#quotas_list). |
| Tokens per minute | Ranges from 8,192 to over 1 million, depending on the model used. | No token limit. However, this function does have different page limits depending on the processor you use. For more information, see [Limits](https://docs.cloud.google.com/document-ai/limits). |
| Supervised tuning | [Supervised tuning](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#languages-gemini) is supported for some models. | Not supported. |
| Supported languages | Support varies based on the LLM you choose. | Language support depends on the document processor type; most only support English. For more information, see [Processor list](https://docs.cloud.google.com/document-ai/docs/processors-list). |
| Supported regions | Supported in all Generative AI for Vertex AI [regions](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#available-regions). | Supported in the `EU` and `US` multi-regions for all processors. Some processors are also available in certain single regions. For more information, see [Regional and multi-regional support](https://docs.cloud.google.com/document-ai/docs/regions). |