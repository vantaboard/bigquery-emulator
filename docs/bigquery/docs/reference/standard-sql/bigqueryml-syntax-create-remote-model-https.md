# The CREATE MODEL statement for remote models over custom models

This document describes the `CREATE MODEL` statement for creating remote models
in BigQuery over custom models deployed to
[Vertex AI](https://docs.cloud.google.com/vertex-ai/docs) by using SQL.
Alternatively, you can use the Google Cloud console user interface to
[create a model by using a UI](https://docs.cloud.google.com/bigquery/docs/create-machine-learning-model-console)
([Preview](https://cloud.google.com/products#product-launch-stages)) instead of constructing the SQL
statement yourself.

After you create a remote model, you can use it with the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
to get predictions from the custom model deployed to Vertex AI.

## `CREATE MODEL` syntax

```sql
{CREATE MODEL | CREATE MODEL IF NOT EXISTS | CREATE OR REPLACE MODEL}
`project_id.dataset.model_name`
[INPUT (field_name field_type)]
[OUTPUT (field_name field_type)]
REMOTE WITH CONNECTION `project_id.region.connection_id`
OPTIONS(ENDPOINT = vertex_ai_https_endpoint);
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

### `INPUT` and `OUTPUT` clauses

You must specify the `INPUT` and `OUTPUT` clauses when you create a remote
model with an HTTPS endpoint over a custom model deployed to
Vertex AI. The `INPUT` clause must contain the fields needed
for the Vertex AI endpoint request, and the `OUTPUT` clause must
contain the fields needed for the Vertex AI endpoint response.

#### Supported data types

You can use the following BigQuery data types in the `INPUT` and
`OUTPUT` clauses:

- [`BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#boolean_type)
- [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types)
- [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types)
- [`NUMERIC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types)
- [`BIGNUMERIC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types)
- [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type)
- An [`ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type) of any of the aforementioned types.

#### Field name format

The `INPUT` and `OUTPUT` field names must be identical as the field names of
the Vertex AI endpoint request and response. For a Vertex AI
endpoint with a single `OUTPUT`, there is no field name in the response, and
therefore you can specify any field name in the `OUTPUT` statement.

**Example**

If the Vertex AI request looks like the following example:

    {
      "instances": [
        { "f1": 10, "f2": 12.3, "f3": "abc", "f4": [1, 2, 3, 4] },
        { "f1": 40, "f2": 32.5, "f3": "def", "f4": [11, 12, 13, 14] },
      ]
    }

The `INPUT` statement must be:

    INPUT(f1 INT64, f2 FLOAT64, f3 STRING, f4 ARRAY<INT64>)

If the Vertex AI response looks like the following example:

    {
      "predictions": [
        {
          "out1": 300,
          "out2": 40
        },
        {
          "out1": 200,
          "out2": 30
        }
      ]
    }

The `OUTPUT` statement must be:

    OUTPUT(out1 INT64, out2 INT64)

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

    ENDPOINT = vertex_ai_https_endpoint

**Description**

For `vertex_ai_https_endpoint`, specify the
[shared public endpoint](https://docs.cloud.google.com/vertex-ai/docs/predictions/choose-endpoint-type)
of a model deployed to Vertex AI, in the format
`https://location-aiplatform.googleapis.com/v1/projects/project/locations/location/endpoints/endpoint_id`.
Dedicated public endpoints, Private Service Connect endpoints, and
private endpoints aren't supported.

To learn more about deploying a model to a shared public endpoint, see
[Create a shared public endpoint](https://docs.cloud.google.com/vertex-ai/docs/predictions/create-public-endpoint#create_a_shared_public_endpoint).

The following example shows how to create a remote model that uses a shared
public endpoint:

    ENDPOINT = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/endpoints/1234'

## Locations

For information about supported locations, see
[Locations for remote models](https://docs.cloud.google.com/bigquery/docs/locations#locations-for-remote-models).

## Example

The following example creates a BigQuery ML remote model
over a model deployed to a Vertex AI endpoint:

```
CREATE MODEL `project_id.mydataset.mymodel`
 INPUT(f1 INT64, f2 FLOAT64, f3 STRING, f4 ARRAY)
 OUTPUT(out1 INT64, out2 INT64)
 REMOTE WITH CONNECTION `myproject.us.test_connection`
 OPTIONS(ENDPOINT = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/endpoints/1234')
```

## What's next

- Learn how to [make predictions with remote models on Vertex AI](https://docs.cloud.google.com/bigquery/docs/bigquery-ml-remote-model-tutorial#create-remote-model).
- For more information about the supported SQL statements and functions for remote models that use HTTPS endpoints, see [End-to-end user journey for each model](https://docs.cloud.google.com/bigquery/docs/e2e-journey).