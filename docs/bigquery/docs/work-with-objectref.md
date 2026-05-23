# Work with ObjectRef values

This document describes `ObjectRef` values and how to create and use them in
BigQuery.

An `ObjectRef` value is a [`STRUCT` type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type)
with a predefined schema that references Cloud Storage objects for
[multimodal analysis](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data). It can be
processed by [`OBJ` functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions),
[AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview), or
[Python user-defined functions](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python).

## Schema

An `ObjectRef` value has the following fields:

| Name | Type | Mode | Description | Example |
|---|---|---|---|---|
| `uri` | `STRING` | `REQUIRED` | The Cloud Storage object's URI. | `"gs://cloud-samples-data/vision/demo-img.jpg"` |
| `version` | `STRING` | `NULLABLE` | The object generation. | `"1560286006357632"` |
| `authorizer` | `STRING` | `NULLABLE` | A BigQuery connection ID for [delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access) or `NULL` for [direct access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#direct-access). The ID can have the following formats: `"region.connection"` or `"project.region.connection"` | `"myproject.us.myconnection"` |
| `details` | `JSON` | `NULLABLE` | The object metadata or errors from processing the object. It can include the fields `content_type`, `md5_hash`, `size`, and `updated` for the [object](https://docs.cloud.google.com/storage/docs/json_api/v1/objects). | `{"gcs_metadata":{"content_type":"image/png","md5_hash":"dfbbb5cf034af026d89f2dc16930be15","size":915052,"updated":1560286006000000}}` |

The `content_type` field in the `gcs_metadata` field from the `details`
column is fetched from Cloud Storage. You can set an object's
[content type](https://docs.cloud.google.com/storage/docs/metadata#content-type) in
Cloud Storage. If you omit it in Cloud Storage, then
BigQuery infers the content type from the suffix of the URI.

## Create `ObjectRef` values

You can create `ObjectRef` values by using
[object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction), the
[`OBJ.MAKE_REF` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objmake_ref),
or [Cloud Storage Insights datasets](https://docs.cloud.google.com/storage/docs/insights/dataset-tables-and-schemas#object-schema).

### Use object tables

Use an object table if you don't have URIs stored in a table and want to list
all the objects from a Cloud Storage prefix. An object table stores
the reference to an object in each row, and has a `ref` column that contains
`ObjectRef` values. The following query uses the
[`CREATE EXTERNAL TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
to create an object table:

    CREATE EXTERNAL TABLE mydataset.images
    WITH CONNECTION `us.myconnection`
    OPTIONS (uris=["gs://mybucket/images/*"], object_metadata="SIMPLE");

    SELECT ref AS image_ref FROM mydataset.images;

`ObjectRef` values from an object table must have an authorizer for
[delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access). The authorizer connection is the same
connection that you use to create the object table.

### Use the `OBJ.MAKE_REF` function

Use the [`OBJ.MAKE_REF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objmake_ref)
function if you already have URIs stored in a table and want to create
`ObjectRef` values from those URIs. The following queries show how
to create `ObjectRef` values in the `image_ref` column from the `uri` column
that contains Cloud Storage URIs:

    -- Specify only the URI
    SELECT *, OBJ.MAKE_REF(uri) AS image_ref FROM mydataset.images;
    -- Specify the URI and the connection
    SELECT *, OBJ.MAKE_REF(uri, "us.myconnection") AS image_ref FROM mydataset.images;

To modify the authorizers of an existing `ObjectRef` value, you can
use the `OBJ.MAKE_REF` function:

    -- Remove the authorizer
    SELECT *, OBJ.MAKE_REF(ref, authorizer=>NULL) AS image_ref FROM mydataset.images;
    -- Change the authorizer
    SELECT *, OBJ.MAKE_REF(ref, authorizer=>"us.myconnection2") AS image_ref FROM mydataset.images;

The `OBJ.MAKE_REF` function accepts a nullable authorizer to support
[direct access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#direct-access) and [delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access).

### Use Cloud Storage Insights datasets

If you have a
[Storage Insights dataset configured](https://docs.cloud.google.com/storage/docs/insights/configure-datasets),
then the dataset already includes a
[`ref` column](https://docs.cloud.google.com/storage/docs/insights/dataset-tables-and-schemas#object-schema)
that contains `ObjectRef` values. Any `ObjectRef` values created in Storage
Insights datasets don't have an authorizer. To query these objects, you must
either have [direct access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#direct-access) to the object or add an authorizer
to the `ObjectRef` to use [delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access).

## Authorizer and permissions

When you pass an `ObjectRef` value to ObjectRef functions, AI functions, or
Python UDFs, those functions need to access the object stored in
Cloud Storage. You can authorize this access based
on the value of the `authorizer` field in two ways: direct access and delegated
access.

### Direct access

With *direct access* , the user who runs the query accesses the object directly
by using their own credentials. Direct access is used when the `ObjectRef`
value has no authorizer.

Direct access has the following restrictions:

- The user must have permission to access the objects.
- A query job using the [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate), [`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if), [`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score), or [`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify) functions without a connection requires the user to have [additional permissions](https://docs.cloud.google.com/bigquery/docs/permissions-for-ai-functions#run_generative_ai_queries_with_end-user_credentials). The query can only access Cloud Storage buckets and objects from the same project in which the job is executed.

For example, if you call the `AI.GENERATE` function on an `ObjectRef` value
that doesn't have an authorizer, then the function reads the object as you. If you
don't have permission to read the object, the function writes a
`"permission denied"` error to the `status` column in the result.

The following example shows a query that uses direct access:

    -- Requires that the end user can read the object "gs://cloud-samples-data/vision/demo-img.jpg" and use the Vertex AI model.
    SELECT AI.GENERATE(
      ("Describe this image:",
      OBJ.GET_ACCESS_URL(OBJ.MAKE_REF("gs://cloud-samples-data/vision/demo-img.jpg"), 'r')));

### Delegated access

With *delegated access* , the user who runs the query delegates object access
to a [BigQuery Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection),
which is specified in the `authorizer` field of the `ObjectRef` value.
Delegated access can enable cross-project data access.

To use delegated access, your data administrator must follow these steps to
set up the connection and permissions:

- **One-time setup** . The data administrator must [set up a Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) to manage the Cloud Storage bucket:
  1. Create a new BigQuery Cloud resource connection or reuse an existing one in the project.
  2. Look up the service account in the connection's metadata.
  3. Grant the service account the [`storage.objects.get`](https://docs.cloud.google.com/storage/docs/access-control/iam-permissions#objects) permission for reads, or the [`storage.objects.create`](https://docs.cloud.google.com/storage/docs/access-control/iam-permissions#objects) permission for writes, in either the project or the Cloud Storage buckets. You can grant these permissions with the [Storage Object Viewer](https://docs.cloud.google.com/storage/docs/access-control/iam-roles#storage.objectViewer) or [Storage Object User](https://docs.cloud.google.com/storage/docs/access-control/iam-roles#storage.objectUser) roles.
- **Per-user setup** . The data administrator must grant users the [`bigquery.objectRefs.read`](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.objectRefs.read) permission for reads, or the [`bigquery.objectRefs.write`](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.objectRefs.write) permission for writes, to the BigQuery connection. You can grant these permissions with the [BigQuery ObjectRef Reader](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.objectRefReader) or [BigQuery ObjectRef Admin](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.objectRefAdmin) roles.

For example, if a user passes `ObjectRef` values that have an authorizer to an
`AI.GENERATE` function, then the function verifies that the user has the
`bigquery.objectRefs.read` permission, and then reads the objects by using the
connection's service account. If the user or the service account has
insufficient permissions, then the function writes a `"permission denied"` error
to the `status` column in the result.

The following example shows a query that uses delegated access. It requires the
following:

- The user has the `bigquery.objectRefs.read` permission on `connection1`.
- The service account for `connection1` has the `storage.objects.get` permission on the object.
- The service account for `connection2` has the Vertex AI User role.

    SELECT AI.GENERATE(
      ("Describe this image:",
        OBJ.GET_ACCESS_URL(OBJ.MAKE_REF("gs://cloud-samples-data/vision/demo-img.jpg", "us.connection1"), 'r')),
      connection_id => "us.connection2");

### Best practices

Consider the following best practices when you decide whether to use direct or
delegated access:

- Use *direct access* for a small team operating in a single project for both data storage and analysis. The data administrator uses Identity and Access Management to grant users access to BigQuery data and Cloud Storage data. Users can create `ObjectRef` values on demand without an authorizer to analyze objects by using their own credentials.
- Use *delegated access* for a large team operating across multiple projects, especially when data storage and analysis are decoupled. The data administrator can set up connections and create `ObjectRef` values for analysis ahead of time with a connection as their authorizer. This approach works with object tables or by using `OBJ.MAKE_REF` on a list of URIs. Then, the data administrator can share the table storing the `ObjectRef` values with analysts. The analysts don't need to access the original bucket to analyze the objects.

## Errors

Functions that consume `ObjectRef` values report errors in two ways:

- Query failure: the query might fail with an error message and no result.
- Returned error values: the query succeeds, but the function might write errors as a part of the return value. For information about the format of the return value, see the reference page for the function you are using.

When a function returns an `ObjectRef` value, the `details` field of that
value might contain an `errors` field. If it does, the value of that field is
an array of errors. Each error has the following schema:

| Name | Type | Mode | Description | Example |
|---|---|---|---|---|
| `code` | `INT64` | `REQUIRED` | Standard HTTP error code. | `400` |
| `message` | `STRING` | `REQUIRED` | A descriptive, user-friendly error message. | `"Connection credential for myproject.us.nonexistent_connection cannot be used. Either the connection does not exist, or the user does not have sufficient permissions (bigquery.objectRefs.read)"` |
| `source` | `STRING` | `REQUIRED` | The name of the function that triggered the error. | `"OBJ.MAKE_REF"` |

These are two common types of errors:

- Object error: the object URI or version provided doesn't exist.
- Authorizer error: the connection doesn't exist or the user has no permission to use it for [delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access).

The following query shows how to select `ObjectRef` values that contain errors
from an [`Objectref` column](https://docs.cloud.google.com/bigquery/docs/objectref-columns):

    SELECT ref
    FROM mydataset.images
    WHERE ref.details.errors IS NOT NULL;

## What's next

- [Specify `ObjectRef` columns in table schemas](https://docs.cloud.google.com/bigquery/docs/objectref-columns).
- [Analyze multimodal data](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data).
- Learn more about [ObjectRef functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions).