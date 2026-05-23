# Analyze object tables by using remote functions

This document describes how to analyze unstructured data in
[object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) by using
[remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions).

## Overview

You can analyze the unstructured data represented by an object table by using
a remote function. A remote function lets you call a function running on
Cloud Run functions or Cloud Run, which you can program to access
resources such as:

- Google's pre-trained AI models, including Cloud Vision API and Document AI.
- Open source libraries such as [Apache Tika](https://tika.apache.org/).
- Your own custom models.

To analyze object table data by using a remote function, you must
generate and pass in
[signed URLs](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#signed_urls) for the
objects in the object table when you call the remote function. These signed
URLs are what grant the remote function access to the objects.

## Required permissions

- To create the connection resource used by the remote function, you need the following permissions:

  - `bigquery.connections.create`
  - `bigquery.connections.get`
  - `bigquery.connections.list`
  - `bigquery.connections.update`
  - `bigquery.connections.use`
  - `bigquery.connections.delete`
- To create a remote function, you need the permissions associated with the
  [Cloud Functions Developer](https://docs.cloud.google.com/functions/docs/reference/iam/roles#cloudfunctions.developer)
  or [Cloud Run Developer](https://docs.cloud.google.com/iam/docs/roles-permissions/run#run.developer) roles.

- To invoke a remote function, you need the permissions described in
  [Remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions#grant_permission_on_function).

- To analyze an object table with a remote function, you need the
  `bigquery.tables.getData` permission on the object table.

## Before you begin

1. Ensure that your BigQuery administrator has [created a connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection) and [set up
   access to Cloud Storage](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#access-storage).

<br />

## Create a remote function

For general instructions on creating a remote function, see
[Working with remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions).

When you create a remote function to analyze object table data, you must
pass in [signed URLs](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#signed_urls)
that have been generated for the objects in the object table. You can do this
by using an input parameter with a `STRING` data type. The signed URLs are
made available to the remote function as input data in the
[`calls` field of the HTTP `POST` request](https://docs.cloud.google.com/bigquery/docs/remote-functions#input_format).
An example of a request is:

    {
      // Other fields omitted.
      "calls": [
        ["https://storage.googleapis.com/mybucket/1.pdf?X-Goog-SignedHeaders=abcd"],
        ["https://storage.googleapis.com/mybucket/2.pdf?X-Goog-SignedHeaders=wxyz"]
      ]
    }

You can read an object in your remote function by using a method that makes
an HTTP `GET` request to the signed URL. The remote function can access the
object because the signed URL contains authentication information in its
query string.

When you specify the
[`CREATE FUNCTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement)
for the remote function, we recommend that you set the `max_batching_rows`
option to 1 in order to
[avoid Cloud Run functions timeout](https://docs.cloud.google.com/functions/docs/concepts/exec#timeout)
and increase processing parallelism.

### Example

The following Cloud Run functions Python code example reads storage
objects and returns their content length to BigQuery:

    import functions_framework
    import json
    import urllib.request

    @functions_framework.http
    def object_length(request):
      calls = request.get_json()['calls']
      replies = []
      for call in calls:
        object_content = urllib.request.urlopen(call[0]).read()
        replies.append(len(object_content))
      return json.dumps({'replies': replies})

Deployed, this function would have an endpoint similar to
`https://us-central1-myproject.cloudfunctions.net/object_length`.

The following example shows how to create a BigQuery remote
function based on this Cloud Run functions function:

```googlesql
CREATE FUNCTION mydataset.object_length(signed_url STRING) RETURNS INT64
REMOTE WITH CONNECTION `us.myconnection`
OPTIONS(
  endpoint = "https://us-central1-myproject.cloudfunctions.net/object_length",
  max_batching_rows = 1
);
```

For step-by-step guidance, see
[Tutorial: Analyze an object table with a remote function](https://docs.cloud.google.com/bigquery/docs/remote-function-tutorial).

## Call a remote function

To call a remote function on object table data, reference the remote
function in the
[`select_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_list)
of the query, and then call the
[`EXTERNAL_OBJECT_TRANSFORM` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions-built-in#external_object_transform)
in the
[`FROM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#from_clause)
to generate the signed URLs for the objects.

> [!NOTE]
> **Note:** When using one of the [AI APIs](https://cloud.google.com/products/ai), be aware of the relevant quotas for the API you are targeting. Use a `LIMIT` clause to limit the results returned if necessary to stay within quota.

The following example shows typical statement syntax:

```googlesql
SELECT uri, function_name(signed_url) AS function_output
FROM EXTERNAL_OBJECT_TRANSFORM(TABLE my_dataset.object_table, ["SIGNED_URL"])
LIMIT 10000;
```

The following example shows how to process only a subset of the object table
contents with a remote function:

```googlesql
SELECT uri, function_name(signed_url) AS function_output
FROM EXTERNAL_OBJECT_TRANSFORM(TABLE my_dataset.object_table, ["SIGNED_URL"])
WHERE content_type = "application/pdf";
```

## What's next

Learn how to [run inference on image object tables](https://docs.cloud.google.com/bigquery/docs/object-table-inference).