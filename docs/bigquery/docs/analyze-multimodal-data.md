# Analyze multimodal data in BigQuery

This document describes the BigQuery features that you can use to
create and analyze multimodal data. These features can be used in
BigQuery with GoogleSQL and with Python by using
[BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).

BigQuery's multimodal data features let you perform the
following tasks:

- Integrate unstructured data stored in Cloud Storage as another column alongside structured data in standard BigQuery tables by representing unstructured data as [`ObjectRef`](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#objectref_values) values.
- Generate annotations, embeddings, and scalar values from multimodal data by using BigQuery ML [generative AI functions](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#generative_ai_functions) with Gemini models.
- Generate annotations, embeddings, and scalar values from multimodal data by [creating multimodal DataFrames](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#multimodal_dataframes) in BigQuery DataFrames and using Python libraries.

For a step-by-step tutorial that uses the Google Cloud console, see
[Analyze multimodal data with SQL and Python](https://docs.cloud.google.com/bigquery/docs/multimodal-data-sql-tutorial).

## Benefits

BigQuery's multimodal data features offer the following benefits:

- **Composability** : you can store and manage structured and unstructured data in the same standard table row by using `ObjectRef` values. For example, you could store images for a product in the same row as the rest of the product information. You can use standard SQL functions to create and update columns that contain `ObjectRef` values, and you can create `ObjectRef` values as the output of a transformation action on an object.
- **Add multiple pieces of unstructured data in generative AI prompts** : add multiple `ObjectRef` values as input to generative AI functions. For example, you can generate embeddings of image and text data from the same table. For text and scalar value generation, you can also refer to multiple objects within the prompt that you send to a model. For example, you could create a prompt that asks the model to generate a product description based on images and attributes of the product.
- **Persisting chunk ordering** : you can chunk objects and then store the chunks as an array of `ObjectRef` values in a standard table column, in order to persist their order. For example, you could parse images from a video, and then store these images as an array of `ObjectRef` values, so that the images stay in the same order that they appear in the original video.

## `ObjectRef` values

An `ObjectRef` value is a `STRUCT` value that uses the
[`ObjectRef` schema](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#schema).
You can store Cloud Storage object metadata and an
associated authorizer in a
[BigQuery standard table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables)
by creating a `STRUCT` or `ARRAY<STRUCT>` column that uses this format.
The authorizer value identifies the
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
that BigQuery uses to access the Cloud Storage object.
If you set the authorizer to `NULL`, then BigQuery uses your
end-user credentials.

Use `ObjectRef` values when you need to integrate unstructured data into a
standard table. For example, in a products table, you could store product images
in the same row with the rest of the product information by adding a column
containing an `ObjectRef` value.

Create and update `ObjectRef` values by using the following
GoogleSQL functions:

- [`OBJ.MAKE_REF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objmake_ref): create an `ObjectRef` value that contains metadata for a Cloud Storage object.
- [`OBJ.FETCH_METADATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objfetch_metadata): fetch Cloud Storage metadata for an `ObjectRef` value that is partially populated with `uri` and `authorizer` values.

For more information, see
[Work with ObjectRef values](https://docs.cloud.google.com/bigquery/docs/work-with-objectref).

## `ObjectRefRuntime` values

An `ObjectRefRuntime` value is a `JSON` value that uses the
[`ObjectRefRuntime` schema](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objectrefruntime).
An `ObjectRefRuntime` value contains the Cloud Storage object metadata
from the `ObjectRef` value that was used to create it, an associated authorizer,
and access URLs. You can use the access URLs to read or modify the object in
Cloud Storage.

Use `ObjectRefRuntime` values to work with object data in analysis and
transformation workflows. The access URLs in `ObjectRefRuntime` values expire
after 6 hours at most, although you can configure shorter expiration time. If
you persist `ObjectRefRuntime` values anywhere as part of your workflow, you
should refresh this data regularly. To persist object metadata, store
`ObjectRef` values instead, and then use them to generate `ObjectRefRuntime`
values when you need them. `ObjectRef` values don't need to be refreshed unless
the underlying objects in Cloud Storage are modified.

Create `ObjectRefRuntime` values by using the
[`OBJ.GET_ACCESS_URL` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objget_access_url).

## Generative AI functions

Generate text, embeddings, and scalar values based on `ObjectRef` or
`ObjectRefRuntime` input by using the following generative AI functions with
Gemini models:

- [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)
- [`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
- [`AI.GENERATE_TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table)
- [`AI.GENERATE_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool)
- [`AI.GENERATE_DOUBLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double)
- [`AI.GENERATE_INT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int)
- [`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
- [`AI.EMBED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed)
- [`AI.SIMILARITY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity)
- [`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify)
- [`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if)
- [`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score)

## Work with multimodal data in Python

You can analyze multimodal data in Python by using BigQuery DataFrames
classes and methods.

### Multimodal DataFrames

Create a multimodal DataFrame that integrates structured and unstructured data
by using the
[`read_gbq` method](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.read_gbq.html):
create a multimodal DataFrame from an object table.

To try it out, see the
[Multimodal Dataframes tutorial](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/multimodal/multimodal_dataframe.ipynb)
on GitHub.

### Generative AI methods

Use the following methods to perform generative AI tasks on multimodal data:

- [`predict` method](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.GeminiTextGenerator.html#bigframes.ml.llm.GeminiTextGenerator.predict) of the [`GeminiTextGenerator` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.GeminiTextGenerator.html#bigframes.ml.llm.GeminiTextGenerator): generate text based on multimodal data.
- [`predict` method](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.MultimodalEmbeddingGenerator.html#bigframes.ml.llm.MultimodalEmbeddingGenerator.predict) of the [`MultimodalEmbeddingGenerator` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.MultimodalEmbeddingGenerator.html#bigframes.ml.llm.MultimodalEmbeddingGenerator): generate embeddings based on multimodal data.

## Object tables

Any new
[object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) that
you create have a `ref` column that contains an `ObjectRef` value for the
given object. The connection that is used to create the object table is used
to populate the `authorizer` values in the `ref` column. You can use the
`ref` column to populate and refresh `ObjectRef` values in standard tables.

## Storage Insights datasets

A [Storage Insights dataset](https://docs.cloud.google.com/storage/docs/insights/datasets) is a linked
BigQuery dataset that
you can query to analyze and visualize your Cloud Storage data. The
[`ref` column](https://docs.cloud.google.com/storage/docs/insights/dataset-tables-and-schemas#object-schema)
contains `ObjectRef` values that you can use to
[analyze object data and metadata](https://docs.cloud.google.com/storage/docs/insights/configure-datasets#analyze-object-data).

## Limitations

The following limitations apply to BigQuery multimodal data
features:

- If you don't explicitly specify the name of the project that contains the connection in the `authorizer` field of an `ObjectRef` value, then the connection must exist either in the project that contains the object table that stores it, or in the project in which the query that references the `ObjectRef` value is run.
- You can't have more than 20 Cloud resource connections in the project and region where your query accesses object data as `ObjectRef` values.
- You can reference `ObjectRef` values from at most five projects besides the project in which you run the query.

## Costs

The following costs are applicable when using multimodal data:

- Storage of object metadata as `ObjectRef` values in standard tables contributes to the BigQuery storage cost for the table.
- Queries run on `ObjectRef` values incur BigQuery compute costs.
- New objects that you create from object transformations incur Cloud Storage costs.
- New data that you create and persist in BigQuery incurs BigQuery storage costs.
- Use of generative AI functions incurs Vertex AI costs.
- Use of BigQuery Python UDFs, and of multimodal DataFrames and object transformations methods in BigQuery DataFrames, incurs Python UDF costs.

For more information, see the following pricing pages:

- [BigQuery pricing](https://cloud.google.com/bigquery/pricing)
- [BigQuery Python UDFs pricing](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python#pricing)
- [Vertex AI pricing](https://docs.cloud.google.com/vertex-ai/generative-ai/pricing)
- [Cloud Storage pricing](https://cloud.google.com/storage/pricing)

## What's next

- [Specify `ObjectRef` columns in table schemas](https://docs.cloud.google.com/bigquery/docs/objectref-columns).
- [Analyze multimodal data with SQL](https://docs.cloud.google.com/bigquery/docs/multimodal-data-sql-tutorial).
- Learn more about [Generative AI in BigQuery ML](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).
- Learn more about [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).