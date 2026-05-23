# Specify ObjectRef columns in table schemas

This document describes how to define a BigQuery standard table schema
with columns that can store `ObjectRef` values.

`ObjectRef` values provide metadata and connection information for objects in
Cloud Storage. Use `ObjectRef` values when you need to
integrate unstructured data into a standard table. For example, in a products
table, you could store product images in the same row with the rest of the
product information by adding a column containing `ObjectRef` values. You can
store `ObjectRef` values in `STRUCT` columns that use the
[`ObjectRef` format](https://docs.cloud.google.com/bigquery/docs/work-with-objectref),
which is
`STRUCT<uri STRING, version STRING, authorizer STRING, details JSON>`.

For more information about working with multimodal data, see
[Analyze multimodal data](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data).
For a tutorial that shows how to work with `ObjectRef` data, see
[Analyze multimodal data with SQL](https://docs.cloud.google.com/bigquery/docs/multimodal-data-sql-tutorial).

> [!NOTE]
> **Note:** The examples in this document use the [`CREATE OR REPLACE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement) to create and populate an `ObjectRef` column in a single operation, but you can also use the [`ALTER TABLE ADD COLUMN` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_column_statement) to add a `STRUCT` column to an existing table and then use the [`UPDATE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_statement) to populate that column in a separate operation.

## Prerequisites

To populate and update `ObjectRef` values in a standard table, the table must
have a `STRING` column that contains URI information for the related
Cloud Storage objects.

You must have a Cloud Storage bucket that contains the same objects
that are identified in the URI data of the target standard table.

## Maintaining `ObjectRef` values

Any object tables that you create have a `ref` column that contains an
`ObjectRef` value for the given object. If you have an existing object table,
then you can join it with your standard table on the object URI column
to populate and update `ObjectRef` values. This is more efficient because
it avoids re-fetching metadata from Cloud Storage to create a
new `ObjectRef` value.

Similarly, if you already have a
[Storage Insights dataset for object metadata](https://docs.cloud.google.com/storage/docs/insights/dataset-tables-and-schemas#object-schema),
then you can use the `ref.uri` or `selfLink` column to join the standard table
with the Storage Insights dataset to populate and update `ObjectRef` values.
Any `ObjectRef` values created in Storage Insights datasets don't have an
authorizer. To query these objects, you must either have
[direct access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#direct-access) to the object
or add an authorizer to the `ObjectRef` to use
[delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access).

If you don't have an existing object table or Storage Insights dataset,
you can use the
[`OBJ.MAKE_REF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objmake_ref)
function to populate and update `ObjectRef` values by fetching object metadata
directly from Cloud Storage. This approach might be less scalable,
because it requires the retrieval of object metadata from Cloud Storage.

## Create an `ObjectRef` column

To create and populate an `ObjectRef` column in a standard table, select one of
the following options:

### SQL functions

Create and populate an `ObjectRef` column based on output from the
`OBJ.MAKE_REF` function:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE OR REPLACE TABLE PROJECT_ID.DATASET_ID.TABLE_NAME
   AS
   SELECT TABLE_NAME.*,
     OBJ.MAKE_REF(uri, 'CONNECTION_ID') AS objectrefcolumn
   FROM DATASET_ID.TABLE_NAME;
   ```


   Replace the following:
   - `PROJECT_ID`: your project ID. You can skip this argument if you are creating the table in your current project.
   - `DATASET_ID`: the ID of the dataset that you are creating.
   - `TABLE_NAME`: the name of the standard table that you are recreating.
   - `CONNECTION_ID`: A `STRING` value that contains a [Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) that the service can use to access the objects in Cloud Storage, in the format `location.connection_id`. For example, `us-west1.myconnection`. You can get the connection ID by [viewing the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections) in the Google Cloud console and copying the value in the last section of the fully qualified connection ID that is shown in **Connection ID** . For example, `projects/myproject/locations/connection_location/connections/*myconnection*`.

     You must grant the Storage Object User
     (`roles/storage.objectUser`) role to the connection's
     service account on any Cloud Storage bucket where you are using it to
     access objects.

     The connection must be in the same project and region as the query
     where you are calling the function.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### Object table

Create and populate an `ObjectRef` column based on data from an object
table `ref` column:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE OR REPLACE TABLE PROJECT_ID.DATASET_ID.TABLE_NAME
   AS
   SELECT TABLE_NAME.*, OBJECT_TABLE.ref AS objectrefcolumn
   FROM DATASET_ID.TABLE_NAME
   INNER JOIN DATASET_ID.OBJECT_TABLE
   ON OBJECT_TABLE.uri = TABLE_NAME.uri;
   ```


   Replace the following:
   - `PROJECT_ID`: your project ID. You can skip this argument if you are creating the table in your current project.
   - `DATASET_ID`: the ID of the dataset that you are creating.
   - `TABLE_NAME`: the name of the standard table that you are recreating.
   - `OBJECT_TABLE`: the name of the object table that contains the object data that you want to integrate into the standard table.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## Create an `ARRAY<ObjectRef>` column

You can create an
`ARRAY<STRUCT<uri STRING, version STRING, authorizer STRING, details JSON>>`
column to contain arrays of `ObjectRef`
values. For example, you could chunk a video into separate images, and
then store these images as an array of `ObjectRef` values.

You can use the
[`ARRAY_AGG` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg)
to aggregate arrays of `ObjectRef` values, including using the `ORDER BY` clause
preserve object order if necessary. You can use the
[`UNNEST` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator)
to parse an array of `ObjectRef` values into individual `ObjectRef` values,
including using the `WITH OFFSET` clause to preserve object order if necessary.
You can use object metadata, like the URI path and object filename, to
map `ObjectRef` values that represent object chunks to an `ObjectRef`
value that represents the original object.

To see an example of how to work with arrays of `ObjectRef` values, see the
[Process ordered multimodal data using `ARRAY<ObjectRef>` values](https://docs.cloud.google.com/bigquery/docs/multimodal-data-sql-tutorial#process_ordered_multimodal_data_using_arrays_of_objectref_values)
section of the
[Analyze multimodal data with SQL](https://docs.cloud.google.com/bigquery/docs/multimodal-data-sql-tutorial)
tutorial.

## Update an `ObjectRef` column

To update an `ObjectRef` column in a standard table, select one of the
following options:

### Object table

Update an `ObjectRef` column by using data from an object
table `ref` column:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   UPDATE PROJECT_ID.DATASET_ID.TABLE_NAME
   SET objectrefcolumn = (SELECT ref FROM DATASET_ID.OBJECT_TABLE WHERE OBJECT_TABLE.uri = TABLE_NAME.uri)
   WHERE uri != "";
   ```


   Replace the following:
   - `PROJECT_ID`: your project ID. You can skip this argument if you are creating the table in your current project.
   - `DATASET_ID`: the ID of the dataset that you are creating.
   - `TABLE_NAME`: the name of the standard table that you are recreating.
   - `OBJECT_TABLE`: the name of the object table that contains the same object data as the standard table `ObjectRef` column.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### SQL functions

Update an `ObjectRef` column by using output from the
`OBJ.FETCH_METADATA` and `OBJ.MAKE_REF` functions:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   UPDATE PROJECT_ID.DATASET_ID.TABLE_NAME
   SET objectrefcolumn = (SELECT OBJ.MAKE_REF(uri, 'CONNECTION_ID'))
   WHERE uri != "";
   ```


   Replace the following:
   - `PROJECT_ID`: your project ID. You can skip this argument if you are creating the table in your current project.
   - `DATASET_ID`: the ID of the dataset that you are creating.
   - `TABLE_NAME`: the name of the standard table that you are recreating.
   - `CONNECTION_ID`: A `STRING` value that contains a [Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) that the service can use to access the objects in Cloud Storage, in the format `location.connection_id`. For example, `us-west1.myconnection`. You can get the connection ID by [viewing the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections) in the Google Cloud console and copying the value in the last section of the fully qualified connection ID that is shown in **Connection ID** . For example, `projects/myproject/locations/connection_location/connections/*myconnection*`.

     You must grant the Storage Object User
     (`roles/storage.objectUser`) role to the connection's
     service account on any Cloud Storage bucket where you are using it to
     access objects.

     The connection must be in the same project and region as the query
     where you are calling the function.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## What's next

- [Analyze multimodal data](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data).
- [Analyze multimodal data with SQL](https://docs.cloud.google.com/bigquery/docs/multimodal-data-sql-tutorial).