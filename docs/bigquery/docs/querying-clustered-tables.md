# Querying clustered tables

When you create a clustered table in BigQuery, the table data is
automatically organized based on the contents of one or more columns in
the table's schema. The columns you specify are used to colocate related data.
When you cluster a table using multiple columns, the order of columns you
specify is important. The order of the specified columns determines the sort
order of the data.

To optimize performance when you run queries against clustered tables, use an
expression that filters on a clustered column or on multiple clustered columns
in the order the clustered columns are specified. Queries that filter on
clustered columns generally perform better than queries that filter only on
non-clustered columns.

BigQuery sorts the data in a clustered table based on the values
in the clustering columns and organizes them into blocks.

When you submit a query that contains a filter on a clustered column,
BigQuery uses the clustering information to efficiently determine
whether a block contains any data relevant to the query. This
allows BigQuery to only scan the relevant blocks --- a process
referred to as [block pruning](https://docs.cloud.google.com/bigquery/docs/clustered-tables#block-pruning).

You can query clustered tables by:

- Using the Google Cloud console
- Using the bq command-line tool's `bq query` command
- Calling the [`jobs.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) and configuring a [query job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationQuery)
- Using the client libraries

You can only use [GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql)
with clustered tables.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // queryClusteredTable demonstrates querying a table that has a clustering specification.
    func queryClusteredTable(w io.Writer, projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	q := client.Query(fmt.Sprintf(`
    	SELECT
    	  COUNT(1) as transactions,
    	  SUM(amount) as total_paid,
    	  COUNT(DISTINCT destination) as distinct_recipients
        FROM
    	  `+"`%s.%s`"+`
    	 WHERE
    	    timestamp > TIMESTAMP('2015-01-01')
    		AND origin = @wallet`, datasetID, tableID))
    	q.Parameters = []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryParameter{
    		{
    			Name:  "wallet",
    			Value: "wallet00001866cb7e0f09a890",
    		},
    	}
    	// Run the query and print results when the query job is completed.
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	it, err := job.Read(ctx)
    	for {
    		var row []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Value
    		err := it.Next(&row)
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		fmt.Fprintln(w, row)
    	}
    	return nil
    }

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

    public class QueryClusteredTable {

      public static void runQueryClusteredTable() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        queryClusteredTable(projectId, datasetName, tableName);
      }

      public static void queryClusteredTable(String projectId, String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          String sourceTable = "`" + projectId + "." + datasetName + "." + tableName + "`";
          String query =
              "SELECT word, word_count\n"
                  + "FROM "
                  + sourceTable
                  + "\n"
                  // Optimize query performance by filtering the clustered columns in sort order
                  + "WHERE corpus = 'romeoandjuliet'\n"
                  + "AND word_count >= 1";

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query).build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query clustered table performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the destination table.
    # table_id = "your-project.your_dataset.your_table_name"

    sql = "SELECT * FROM `bigquery-public-data.samples.shakespeare`"
    cluster_fields = ["corpus"]

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        clustering_fields=cluster_fields, destination=table_id
    )

    # Start the query, passing in the extra configuration.
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        sql, job_config=job_config
    )  # Make an API request and wait for job to complete.

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    if table.clustering_fields == cluster_fields:
        print(
            "The destination table is written using the cluster_fields configuration."
        )

## Required permissions

To run a query [job](https://docs.cloud.google.com/bigquery/docs/managing-jobs), you need the
`bigquery.jobs.create` Identity and Access Management (IAM) permission on the project that
runs the query job.

Each of the following predefined IAM roles includes the
permissions that you need to run a query job:

- `roles/bigquery.admin`
- `roles/bigquery.jobUser`
- `roles/bigquery.user`

You also need the `bigquery.tables.getData` permission
on all tables and views that your query references. In addition, when querying
a view you need this permission on all underlying tables and views.
However, if you are using [authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views)
or [authorized datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets), you don't need
access to the underlying source data.

Each of the following predefined IAM roles includes the
permission that you need on all tables and views that the query references:

- `roles/bigquery.admin`
- `roles/bigquery.dataOwner`
- `roles/bigquery.dataEditor`
- `roles/bigquery.dataViewer`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://cloud.google.com/bigquery/docs/access-control).

## Best practices

To get the best performance from queries against clustered tables, use the
following best practices.

For context, the sample table used in the best practice examples is a
clustered table that is created by using a DDL statement. The DDL statement
creates a table named `ClusteredSalesData`. The table is clustered by the
following columns: `customer_id`, `product_id`, `order_id`, in that sort order.

```googlesql
CREATE TABLE
  `mydataset.ClusteredSalesData`
PARTITION BY
  DATE(timestamp)
CLUSTER BY
  customer_id,
  product_id,
  order_id AS
SELECT
  *
FROM
  `mydataset.SalesData`
```

### Filter clustered columns by sort order

When you specify a filter, use expressions that filter on the clustered columns
in sort order. Sort order is the column order given in the `CLUSTER BY` clause.
To get the benefits of clustering, include one or more of the clustered
columns in left-to-right sort order, starting with the first column. In most
cases, the first clustering column is the most effective in block
pruning, then the second column, then the third. You can still use the second
or third column alone in the query, but block pruning probably
won't be as effective. The ordering of the column names inside the filter
expression doesn't affect performance.

The following example queries the `ClusteredSalesData` clustered table
that was created in the preceding example. The query includes a filter
expression that filters on `customer_id` and then on `product_id`. This query
optimizes performance by filtering the clustered columns in *sort
order* ---the column order given in the `CLUSTER BY` clause.

```googlesql
SELECT
  SUM(totalSale)
FROM
  `mydataset.ClusteredSalesData`
WHERE
  customer_id = 10000
  AND product_id LIKE 'gcp_analytics%'
```

The following query does not filter the clustered columns in sort order. As a
result, the performance of the query is not optimal. This query filters on
`product_id` then on `order_id` (skipping `customer_id`).

```googlesql
SELECT
  SUM(totalSale)
FROM
  `mydataset.ClusteredSalesData`
WHERE
  product_id LIKE 'gcp_analytics%'
  AND order_id = 20000
```

### Don't use clustered columns in complex filter expressions

If you use a clustered column in a complex filter expression, the performance of
the query is not optimized because block pruning cannot be applied.

For example, the following query won't prune blocks because a clustered
column---`customer_id`---is used in a function in the filter
expression.

```googlesql
SELECT
  SUM(totalSale)
FROM
  `mydataset.ClusteredSalesData`
WHERE
  CAST(customer_id AS STRING) = "10000"
```

To optimize query performance by pruning blocks, use simple filter expressions
like the following. In this example, a simple filter is applied to the
clustered column---`customer_id`.

```googlesql
SELECT
  SUM(totalSale)
FROM
  `mydataset.ClusteredSalesData`
WHERE
  customer_id = 10000
```

### Don't compare clustered columns to other columns

If a filter expression compares a clustered column to another column (either a
clustered column or a non-clustered column), the performance of the query is not
optimized because block pruning cannot be applied.

The following query does not prune blocks because the filter expression compares
a clustered column---`customer_id` to another column---`order_id`.

```googlesql
SELECT
  SUM(totalSale)
FROM
  `mydataset.ClusteredSalesData`
WHERE
  customer_id = order_id
```

## Table security

To control access to tables in BigQuery, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

## What's next

- For more information on running queries, see [Running interactive and batch queries](https://docs.cloud.google.com/bigquery/docs/running-queries).
- To learn how to create and use clustered tables, see [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
- For an overview of partitioned table support in BigQuery, see [Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).
- To learn how to create partitioned tables, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables).