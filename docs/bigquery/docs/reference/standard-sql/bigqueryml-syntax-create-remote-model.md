# The CREATE MODEL statement for Vertex AI LLMs as MaaS

This document describes the `CREATE MODEL` statement for creating remote models
in BigQuery over LLMs in Vertex AI as a model as a service
(MaaS) by using SQL. When you use MaaS on Vertex AI, you don't
have to provision or manage serving infrastructure for your models. Choose
MaaS for rapid development and prototyping, when you want to minimize
operational overhead.
Vertex AI offers access to
[Google models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models),
[partner models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/use-partner-models)
and [open models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/open-models/use-maas)
using MaaS. For more information, see
[When to use MaaS](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/open-models/choose-serving-option#when_to_use_maas).

Alternatively, you can use the Google Cloud console user interface to
[create a model by using a UI](https://docs.cloud.google.com/bigquery/docs/create-machine-learning-model-console)
([Preview](https://cloud.google.com/products#product-launch-stages)) instead of constructing the SQL
statement yourself.

After you create the remote model, you can use one of the following functions
to perform generative AI with that model:

- [`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
- [`AI.GENERATE_TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table) (only for Gemini models)

## `CREATE MODEL` syntax

```sql
{CREATE MODEL | CREATE MODEL IF NOT EXISTS | CREATE OR REPLACE MODEL}
`project_id.dataset.model_name`
REMOTE WITH CONNECTION {DEFAULT | `project_id.region.connection_id`}
OPTIONS(ENDPOINT = 'vertex_ai_llm_endpoint');
```

### `CREATE MODEL`

Creates and trains a new model in the specified dataset. If the model name
exists, `CREATE MODEL` returns an error.

### `CREATE MODEL IF NOT EXISTS`

Creates and trains a new model only if the model doesn't exist in the
specified dataset.

### `CREATE OR REPLACE MODEL`

Creates and trains a model and replaces an existing model with the same name in
the specified dataset.

### `model_name`

The name of the model you're creating or replacing. The model
name must be unique in the dataset: no other model or table can have the same
name. The model name must follow the same naming rules as a
BigQuery table. A model name can:

- Contain up to 1,024 characters
- Contain letters (upper or lower case), numbers, and underscores

`model_name` is case-sensitive.

If you don't have a default project configured, then you must prepend the
project ID to the model name in the following format, including backticks:

\`\[PROJECT_ID\].\[DATASET\].\[MODEL\]\`

For example, \`myproject.mydataset.mymodel\`.

### `REMOTE WITH CONNECTION`

**Syntax**

    `[PROJECT_ID].[LOCATION].[CONNECTION_ID]`

BigQuery uses a
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
to interact with

the Vertex AI endpoint.


The connection elements are as follows:

- `PROJECT_ID`: the project ID of the project that contains the connection.
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) used by the connection. The connection must be in the same location as the dataset that contains the model.
- `CONNECTION_ID`: the connection ID---for example, `myconnection`.

  To find your connection ID,
  [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
  in the Google Cloud console. The connection ID is the value in the last
  section of the fully qualified connection ID that is shown in
  **Connection ID** ---for example
  `projects/myproject/locations/connection_location/connections/*myconnection*`.

  To use a [default
  connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify `DEFAULT` instead of the connection string
  containing <var translate="no">PROJECT_ID</var>.<var translate="no">LOCATION</var>.<var translate="no">CONNECTION_ID</var>.

<br />

If you are creating a remote model over a Vertex AI model that uses supervised tuning, you need to grant the [Vertex AI Service Agent role](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.serviceAgent) to the connection's service account in the project where you create the model. Otherwise, you need to grant the [Vertex AI User role](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.user) to the connection's service account in the project where you create the model.

If you are using the remote model to analyze unstructured data from an
[object table](https://docs.cloud.google.com/bigquery/docs/object-table-introduction), you must also grant the
[Vertex AI Service Agent role](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.serviceAgent)
to the service account of the connection associated with the object table.
You can find the object table's connection in the Google Cloud console, on the
**Details** pane for the object table.

**Example**

    `myproject.us.my_connection`

### `ENDPOINT`

**Syntax**

```
ENDPOINT = 'vertex_ai_llm_endpoint'
```

**Description**

The Vertex AI endpoint for the remote model to use. You can
specify the name of the Vertex AI model, for example
`gemini-2.5-flash`, or you can specify the Vertex AI model's
endpoint URL, for example
`https://europe-west6-aiplatform.googleapis.com/v1/projects/myproject/locations/europe-west6/publishers/google/models/gemini-2.5-flash`. If you specify the model name, BigQuery ML
automatically identifies and uses the full endpoint of the
Vertex AI model based on the location of the dataset in which
you create the model.

**Arguments**

A `STRING` value that contains the model name of the target
Vertex AI LLM. The following LLMs are supported:

### Pretrained Gemini models

All of the
[generally available](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#generally_available_models)
and
[preview](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#preview-gemini-models)
Gemini models are supported.

> [!NOTE]
> **Note:** To provide feedback or request support for the models in preview, send an email to [bqml-feedback@google.com](mailto:bqml-feedback@google.com).

For
[supported Gemini models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#supported_models),
you can specify the
[global endpoint](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#use_the_global_endpoint),
as shown in the following example:

      https://aiplatform.googleapis.com/v1/projects/test-project/locations/global/publishers/google/models/gemini-2.5-flash

Using the global endpoint for your requests can improve overall
availability while reducing resource exhausted (429) errors, which occur
when you exceed your quota for a regional endpoint.
If you want to use Gemini in a region where it isn't
available, you can avoid migrating your data to a different region by
using the global endpoint instead. You can only
use the global endpoint with the `AI.GENERATE_TEXT` function.

> [!NOTE]
> **Note:** Don't use the global endpoint if you have requirements for the data processing location, because when you use the global endpoint, you can't control or know the region where your processing requests are handled.

> [!NOTE]
> **Note:** Using Gemini 2.5 models with any of these functions incurs charges for the [thinking process](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/thinking). With some functions, you can set a budget for the thinking process for Gemini 2.5 Flash and Gemini 2.5 Flash-Lite models. You can't set a budget for Gemini 2.5 Pro models. See the documentation for a given function for details.

### Claude models

The following
[Anthropic Claude models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/use-claude)
are supported:

- `claude-haiku-4-5`
- `claude-sonnet-4-5`
- `claude-opus-4-1`
- `claude-opus-4`
- `claude-sonnet-4`
- `claude-3-7-sonnet`
- `claude-3-5-haiku`
- `claude-3-haiku`

You must enable Claude models in Vertex AI before you can use
them. For more information, see
[Enable a partner model](https://docs.cloud.google.com/bigquery/docs/generate-text#enable-model).

Although Claude models are multimodal, you can only use text input with
Claude models in BigQuery ML.

After you create a remote model based on a Claude model, you can use the
model with the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
to generate text based on a prompt you provide in a query or from a column in a
[standard table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables).

### Mistral AI models

The following
[Mistral AI models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/mistral)
are supported:

- `mistral-large-2411`
- `mistral-nemo`
- `mistral-small-2503`

Don't use a version suffix with any Mistral AI model.

You must enable Mistral AI models in Vertex AI before you can use
them. For more information, see
[Enable a partner model](https://docs.cloud.google.com/bigquery/docs/generate-text#enable-model).

After you create a remote model based on a Mistral AI model, you can use the
model with the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
to generate text based on a prompt you provide in a query or from a column in a
[standard table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables).

### Llama models as MaaS

To create a Llama model in BigQuery ML, you must specify it as
an [OpenAI API](https://platform.openai.com/docs/api-reference/introduction)
endpoint in the format `openapi/<publisher_name>/<model_name>`.

The following
[Llama models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/llama)
are supported:

- Llama 4 Scout 17B-16E, endpoint `meta/llama-4-scout-17b-16e-instruct-maas`
- Llama 4 Maverick 17B-128E, endpoint `meta/llama-4-maverick-17b-128e-instruct-maas`
- Llama 3.3 70B (Preview), endpoint `openapi/meta/llama-3.3-70b-instruct-maas`
- Llama 3.2 90B (Preview), endpoint `openapi/meta/llama-3.2-90b-vision-instruct-maas`
- Llama 3.1 405B (GA), endpoint `openapi/meta/llama-3.1-405b-instruct-maas`
- Llama 3.1 70B (Preview), endpoint `openapi/meta/llama-3.1-70b-instruct-maas`
- Llama 3.1 8B (Preview), endpoint `openapi/meta/llama-3.1-8b-instruct-maas`

> [!IMPORTANT]
> **Important:** For Llama 4.0 and greater models, you must create the dataset and connection for the remote model in the same region as the Llama model endpoint.

You must enable Llama models in Vertex AI before you can use
them. For more information, see
[Enable a partner model](https://docs.cloud.google.com/bigquery/docs/generate-text#enable-model).

After you create a remote model based on a Llama model, you can use the
model with the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
to generate text based on a prompt you provide in a query or from a column in a
[standard table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables).

For information that can help you choose between the supported models, see
[Model information](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/learn/models).

## Locations

For information about supported locations, see
[Locations for remote models](https://docs.cloud.google.com/bigquery/docs/locations#locations-for-remote-models).

## Examples

The following examples create BigQuery ML remote models.

### Create a Gemini model that uses the default connection

The following example creates a BigQuery ML remote model over a
Gemini model:

```
CREATE OR REPLACE MODEL `mydataset.gemini_model`
REMOTE WITH CONNECTION DEFAULT
OPTIONS(ENDPOINT = 'gemini-2.5-flash');
```

### Create a partner model that uses the default connection

The following example creates a BigQuery ML remote model over a
Mistral AI model:

```
CREATE OR REPLACE MODEL `mydataset.mistral_model`
REMOTE WITH CONNECTION DEFAULT
OPTIONS(ENDPOINT = 'mistral-large-2411');
```

## What's next

- For more information about using Vertex AI models with BigQuery ML, see [Generative AI overview](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).
- Try [generating text from BigQuery data](https://docs.cloud.google.com/bigquery/docs/generate-text).
- Try [generating structured text from BigQuery data](https://docs.cloud.google.com/bigquery/docs/generate-table).