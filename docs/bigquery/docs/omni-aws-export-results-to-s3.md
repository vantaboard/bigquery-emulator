# Export query results to Amazon S3

This document describes how to export the result of a query that runs against a
[BigLake table](https://docs.cloud.google.com/bigquery/docs/biglake-intro) to your
Amazon Simple Storage Service (Amazon S3) bucket.

For information about how data flows between BigQuery and
Amazon S3,
see [Data flow when exporting data](https://docs.cloud.google.com/bigquery/docs/omni-introduction#export-data).

## Limitations

For a full list of limitations that apply to BigLake tables
based on Amazon S3 and Blob Storage, see [Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

## Before you begin

Ensure that you have the following resources:


- A [connection to access your Amazon S3 bucket](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection).
- An [Amazon S3 BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table).
- The correct Amazon Web Services (AWS) Identity and Access Management (IAM) policy:
  - You must have the `PutObject` permission to write data into the Amazon S3 bucket. For more information, see [Create an AWS IAM policy for BigQuery](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection#creating-aws-iam-policy).

<!-- -->

- If you are on the [capacity-based pricing model](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing), then ensure that you have enabled the [BigQuery Reservation API](https://console.cloud.google.com/apis/library/bigqueryreservation.googleapis.com) for your project. For information about pricing, see [BigQuery Omni pricing](https://cloud.google.com/bigquery/pricing#bqomni).

## Export query results

BigQuery Omni writes to the specified
Amazon S3 location regardless of any existing
content. The export query can overwrite existing data or mix the query result
with existing data. We recommend that you export the query result to an empty
Amazon S3 bucket.

To run a query, select one of the following options:

### SQL

In the **Query editor** field, enter a GoogleSQL export query.
GoogleSQL is the default syntax in the Google Cloud console.

<br />


> [!NOTE]
> **Note:** To override the default project, use the `--project_id=PROJECT_ID` parameter. Replace `PROJECT_ID` with the ID of your Google Cloud project.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
      EXPORT DATA WITH CONNECTION `CONNECTION_REGION.CONNECTION_NAME`
      OPTIONS(uri="s3://BUCKET_NAME/PATH", format="FORMAT", ...)
      AS QUERY
   ```


   Replace the following:
   - `CONNECTION_REGION`: the region where the connection was created.
   - `CONNECTION_NAME`: the connection name that you created with the necessary permission to write to the Amazon S3 bucket.
   - `BUCKET_NAME`: the Amazon S3 bucket where you want to write the data.
   - `PATH`: the path where you want to write the exported file to. It must contain exactly one wildcard `*` anywhere in the leaf directory of the path string, for example, `../aa/*`, `../aa/b*c`, `../aa/*bc`, and `../aa/bc*`. BigQuery replaces `*` with `0000..N` depending on the number of files exported. BigQuery determines the file count and sizes. If BigQuery decides to export two files, then `*` in the first file's filename is replaced by `000000000000`, and `*` in the second file's filename is replaced by `000000000001`.
   - `FORMAT`: supported formats are `JSON`, `AVRO`, `CSV`, and `PARQUET`.
   - `QUERY`: the query to analyze the data that is stored in a BigLake table. The dataset that contains the BigLake table used in the query must be located in the same [Amazon S3 region](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations) as the target Amazon S3 bucket.

     <br />

   - Click **Run**.

     <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to export query results to Amazon S3 bucket
    public class ExportQueryResultsToS3 {

      public static void main(String[] args) throws InterruptedException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String externalTableName = "MY_EXTERNAL_TABLE_NAME";
        // connectionName should be in the format of connection_region.connection_name. e.g.
        // aws-us-east-1.s3-write-conn
        String connectionName = "MY_CONNECTION_REGION.MY_CONNECTION_NAME";
        // destinationUri must contain exactly one * anywhere in the leaf directory of the path string
        // e.g. ../aa/*, ../aa/b*c, ../aa/*bc, and ../aa/bc*
        // BigQuery replaces * with 0000..N depending on the number of files exported.
        // BigQuery determines the file count and sizes.
        String destinationUri = "s3://your-bucket-name/*";
        String format = "EXPORT_FORMAT";
        // Export result of query to find states starting with 'W'
        String query =
            String.format(
                "EXPORT DATA WITH CONNECTION `%s` OPTIONS(uri='%s', format='%s') "
                  + "AS SELECT * FROM %s.%s.%s WHERE name LIKE 'W%%'",
                connectionName, destinationUri, format, projectId, datasetName, externalTableName);
        exportQueryResultsToS3(query);
      }

      public static void exportQueryResultsToS3(String query) throws InterruptedException {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.of(query));

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query results exported to Amazon S3 successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

## Troubleshooting

If you get an error related to `quota failure`, then check if you have reserved
capacity for your queries. For more information about slot reservations, see
[Before you begin](https://docs.cloud.google.com/bigquery/docs/omni-aws-export-results-to-s3#before_you_begin) in this document.

## What's next

- Learn about [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction).
- Learn how to [export table data](https://docs.cloud.google.com/bigquery/docs/exporting-data).
- Learn how to [query data stored in Amazon S3](https://docs.cloud.google.com/bigquery/docs/query-aws-data).
- Learn how to [set up VPC Service Controls for BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-vpc-sc).