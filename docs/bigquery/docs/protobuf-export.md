# Export data as Protobuf columns

This document describes how you can export BigQuery data as Protocol
Buffers (Protobuf) columns by using BigQuery user-defined
functions (UDFs).

## When to use Protobuf columns

BigQuery offers a number of built-in functions to format selected
data. One option is to merge multiple column values into a single Protobuf
value, which has the following benefits:

- Object type safety.
- Improved compression, data transfer time, and cost as compared with JSON.
- Flexibility, as most programming languages have libraries to handle Protobuf.
- Less overhead when reading from multiple columns and building a single object.

While other column types can also provide type safety, using Protobuf columns
provides a fully typed object, which can reduce the amount of work that needs to
be done on the application layer or on another part of the pipeline.

However, there are limitations to exporting BigQuery data as
Protobuf columns:

- Protobuf columns are not well indexed or filtered. Searching by the content of the Protobuf columns can be less effective.
- Sorting data in Protobuf format can be difficult.

If these limitations apply to the export workflow, you might consider other
methods of exporting BigQuery data:

- Use [scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) with [`EXPORT DATA` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#export_data_statement) to sort the exported BigQuery data by date or time, and to schedule exports on a recurring basis. BigQuery supports exporting data into Avro, CSV, JSON, and Parquet formats.
- Use [Dataflow](https://docs.cloud.google.com/dataflow/docs/overview) to export BigQuery data in either the Avro or CSV file format.

## Required roles


To get the permissions that
you need to export BigQuery data as Protobuf columns,

ask your administrator to grant you the
following IAM roles on your project:

- Create a user-defined function: [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`)
- Export data from a BigQuery table: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`)
- Read and upload files to Cloud Storage: [Storage Object Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.objectCreator) (`roles/storage.objectCreator`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create a UDF

Create a UDF that converts a BigQuery `STRUCT`
data type into a Protobuf column:

1. In a command line, clone the `bigquery-utils.git` repository:

       git clone https://github.com/GoogleCloudPlatform/bigquery-utils.git

2. Navigate to the Protobuf export folder:

       cd bigquery-utils/tools/protobuf_export

3. Use the [`cp` command](https://man7.org/linux/man-pages/man1/cp.1.html)
   or your operating system's file browser to copy your proto file to the
   `./protos` child folder.

   There is a sample proto file named `dummy.proto` already in the
   `./protos` folder.
4. Install the necessary packages from the GitHub repository:

       npm install

5. Bundle the package by using webpack:

       npx webpack --config webpack.config.js --stats-error-details

6. Locate the `pbwrapper.js` file in the `./dist` child folder, and then
   [upload the file to a Cloud Storage bucket](https://docs.cloud.google.com/storage/docs/uploading-objects).

7. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
8. Using the query editor, create a UDF named `toMyProtoMessage` that builds a
   Protobuf column from existing BigQuery table columns:

       CREATE FUNCTION
         DATASET_ID.toMyProtoMessage(input STRUCT<INPUT_FIELDS>)
         RETURNS BYTES
           LANGUAGE js OPTIONS ( library=["gs://BUCKET_NAME/pbwrapper.js"]
       ) AS r"""
       let message = pbwrapper.setup("PROTO_PACKAGE.PROTO_MESSAGE")
       return pbwrapper.parse(message, input)
         """;

   Replace the following:
   - <var translate="no">`DATASET_ID`</var>: the ID of the dataset to contain the UDF.
   - <var translate="no">`INPUT_FIELDS`</var>: the fields used in the
     [proto message type](https://protobuf.dev/programming-guides/proto3/#simple)
     for the proto file, in the
     format `field_name_1 field_type_1 [, field_name_2 field_type_2, ...]`.

     You must translate any message type fields that use underscores to use
     [camel case](https://en.wikipedia.org/wiki/Camel_case) instead.
     For example, if the message type looks like the following, then
     the input fields value must be `itemId int64, itemDescription string`:

     ```
     message ThisMessage {
       int64 item_id = 1;
       string item_description = 2;
     }
     ```
   - <var translate="no">`BUCKET_NAME`</var>: the name of the Cloud Storage bucket
     that contains the `pbwrapper.js` file.

   - <var translate="no">`PROTO_PACKAGE`</var>: the
     [package](https://protobuf.dev/programming-guides/proto3/#packages) for
     the proto file.

   - <var translate="no">`PROTO_MESSAGE`</var>: the message type for the proto file.

   For example, if you use the provided `dummy.proto` file, the
   `CREATE FUNCTION` statement looks as follows:

   ```
   CREATE OR REPLACE FUNCTION
     mydataset.toMyProtoMessage(input STRUCT<dummyField STRING>)
     RETURNS BYTES
       LANGUAGE js OPTIONS ( library=["gs://mybucket/pbwrapper.js"]
   ) AS r"""
   let message = pbwrapper.setup("dummypackage.DummyMessage")
   return pbwrapper.parse(message, input)
     """;
   ```

## Format columns as Protobuf values

Run the `toMyProtoMessage` UDF to format BigQuery
table columns as Protobuf values:

      SELECT
        UDF_DATASET_ID.toMyProtoMessage(STRUCT(INPUT_COLUMNS)) AS protoResult
      FROM
        `PROJECT_ID.DATASET_ID.TABLE_NAME`
      LIMIT
        100;

Replace the following:

- <var translate="no">`UDF_DATASET_ID`</var>: the ID of the dataset that contains the UDF.
- <var translate="no">`INPUT_COLUMNS`</var>: the names of the columns to format as a Protobuf value, in the format `column_name_1 [, column_name_2, ...]`. Columns can be of any supported [scalar value type](https://protobuf.dev/programming-guides/proto3/#scalar) or non-scalar type, including `ARRAY` and `STRUCT`. Input columns must match the type and number of the proto message type fields.
- <var translate="no">`PROJECT_ID`</var>: the ID of the project that contains the table. You can skip identifying the project if the dataset is in your current project.
- <var translate="no">`DATASET_ID`</var>: the ID of the dataset that contains the table.
- <var translate="no">`TABLE_NAME`</var>: the name of the table that contains the columns to format.

For example, if you use a `toMyProtoMessage` UDF based on `dummy.proto`,
the following `SELECT` statement works:

    SELECT
      mydataset.toMyProtoMessage(STRUCT(word)) AS protoResult
    FROM
      `bigquery-public-data.samples.shakespeare`
    LIMIT 100;

## Work with Protobuf values

With the BigQuery data exported in the Protobuf format, you can
now work with the data as a fully typed object or struct.

The following code samples provide several examples of ways that you can process
or work with the exported data:

### Go

    // package Main queries Google BigQuery.
    package main

    import (
    	"context"
    	"fmt"
    	"io"
    	"log"
    	"os"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    	"google.golang.org/Protobuf/proto"

    	pb "path/to/proto/file_proto"
    )

    const (
    	projectID = "your-project-id"
    )

    // Row contains returned row data from bigquery.
    type Row struct {
    	RowKey string `bigquery:"RowKey"`
    	Proto  []byte `bigquery:"ProtoResult"`
    }

    func main() {
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		log.Fatalf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	rows, err := query(ctx, client)
    	if err != nil {
    		log.Fatal(err)
    	}
    	if err := printResults(os.Stdout, rows); err != nil {
    		log.Fatal(err)
    	}
    }

    // query returns a row iterator suitable for reading query results.
    func query(ctx context.Context, client *bigquery.Client) (*bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_RowIterator, error) {

    	query := client.Query(
    		`SELECT 
      concat(word, ":", corpus) as RowKey, 
      <dataset-id>.toMyProtoMessage(
        STRUCT(
          word, 
          CAST(word_count AS BIGNUMERIC)
        )
      ) AS ProtoResult 
    FROM 
      ` + "` bigquery - public - data.samples.shakespeare `" + ` 
    LIMIT 
      100;
    `)
    	return query.Read(ctx)
    }

    // printResults prints results from a query.
    func printResults(w io.Writer, iter *bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_RowIterator) error {
    	for {
    		var row Row
    		err := iter.Next(&row)
    		if err == iterator.Done {
    			return nil
    		}
    		if err != nil {
    			return fmt.Errorf("error iterating through results: %w", err)
    		}
    		message := &pb.TestMessage{}
    		if err = proto.Unmarshal(row.Proto, message); err != nil {
    			return err
    		}
    		fmt.Fprintf(w, "rowKey: %s, message: %v\n", row.RowKey, message)
    	}
    }

### Java

    package proto;

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FieldValueList.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import path.to.proto.TestMessage;
    import java.util.UUID;

    /** Queries Google BigQuery */
    public final class Main {
      public static void main(String[] args) throws Exception {
        String projectId = "your-project-id";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.newBuilder().setProjectId(projectId).build().getService();

        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(
                    " SELECT "
                        + "concat(word , \":\",corpus) as RowKey,"
                        + "<dataset-id>.toMyProtoMessage(STRUCT(word, "
                        + "CAST(word_count AS BIGNUMERIC))) AS ProtoResult "
                        + "FROM "
                        + "`bigquery-public-data.samples.shakespeare` "
                        + "ORDER BY word_count DESC "
                        + "LIMIT 20")
                .setUseLegacySql(false)
                .build();

        // Create a job ID so that we can safely retry.
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html jobId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html.of(UUID.randomUUID().toString());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html queryJob = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.newBuilder(queryConfig).setJobId(jobId).build());

        // Wait for the query to complete.
        queryJob = queryJob.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();

        // Check for errors
        if (queryJob == null) {
          throw new RuntimeException("Job no longer exists");
        } else if (queryJob.getStatus().getError() != null) {
          // You can also look at queryJob.getStatus().getExecutionErrors() for all
          // errors, not just the latest one.
          throw new RuntimeException(queryJob.getStatus().getError().toString());
        }

        // Get the results.
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html result = queryJob.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_getQueryResults_com_google_cloud_bigquery_BigQuery_QueryResultsOption____();

        // Print all pages of the results.
        for (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FieldValueList.html row : result.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()) {
          String key = row.get("RowKey").getStringValue();
          byte[] message = row.get("ProtoResult").https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FieldValue.html#com_google_cloud_bigquery_FieldValue_getBytesValue__();
          TestMessage testMessage = TestMessage.parseFrom(message);
          System.out.printf("rowKey: %s, message: %s\n", key, testMessage);
        }
      }
    }

### Python

    """Queries Google BigQuery."""

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest
    from path.to.proto import awesome_pb2


    def main():
      project_id = "your-project-id"
      client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(project=project_id)
      query_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(query="""
                   SELECT
    			concat(word , ":",corpus) as RowKey,
    			<dataset-id>.toMyProtoMessage(
    			    STRUCT(
    			      word, 
    			      CAST(word_count AS BIGNUMERIC)
    			    )
    			  ) AS ProtoResult 
    		FROM
    				  `bigquery-public-data.samples.shakespeare`
    		ORDER BY word_count DESC
    		LIMIT 20
        """)
      rows = https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dbapi.Cursor.html#google_cloud_bigquery_dbapi_Cursor_query_job.result()
      for row in rows:
        message = awesome_pb2.TestMessage()
        message.ParseFromString(row.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Row.html#google_cloud_bigquery_table_Row_get("ProtoResult"))
        print(
            "rowKey: {}, message: {}".format(row.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Row.html#google_cloud_bigquery_table_Row_get("RowKey"), message)
        )