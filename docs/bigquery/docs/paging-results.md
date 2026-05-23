# Read data with BigQuery API using pagination

This document describes how to read table data and query results with the
BigQuery API using pagination.

## Page through results using the API

All `*collection*.list` methods return paginated results
under certain circumstances. The `maxResults` property limits the number of results per page.

| Method | Pagination criteria | Default `maxResults` value | Maximum `maxResults` value | Maximum `maxFieldValues` value |
|---|---|---|---|---|
| `tabledata.list` | Returns paginated results if the response size is more than 10 MB^1^ of data or more than `maxResults` rows. | Unlimited | Unlimited | Unlimited |
| All other `*collection*.list` methods | Returns paginated results if the response is more than `maxResults` rows and also less than the maximum limits. | 10,000 | Unlimited | 300,000 |

If the result is larger than the byte or field limit, the result is
trimmed to fit the limit. If one row is greater than the byte or field limit,
`tabledata.list` can return up to 100 MB of data^1^,
which is consistent with the maximum row size limit for query results.
There is no minimum size per page, and some pages might return more rows than others.

^1^The row size is approximate, as the
size is based on the internal representation of row data.
The maximum row size limit is enforced during certain stages of query job
execution.

`jobs.getQueryResults` can return 20 MB of data unless explicitly
requested more through support.

A page is a subset of the total number of rows. If your results are more
than one page of data, the result data has a `pageToken`
property. To retrieve the next page of results, make another `list`
call and include the token value as a URL parameter named `pageToken`.

The [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list)
method, which is used to page through table data, uses a row offset value or a
page token. See [Browsing table data](https://docs.cloud.google.com/bigquery/docs/managing-table-data#browse-table)
for information.

## Iterate through client libraries results

The cloud client libraries handle the low-level details of API pagination and
provide a more iterator-like experience that simplifies interaction with the
individual elements in the page responses.

The following samples demonstrate paging through BigQuery table
data.

### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.html;
    using Google.Apis.Bigquery.v2.Data;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;
    using System.Linq;

    public class BigQueryBrowseTable
    {
        public void BrowseTable(
            string projectId = "your-project-id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            TableReference tableReference = new TableReference()
            {
                TableId = "shakespeare",
                DatasetId = "samples",
                ProjectId = "bigquery-public-data"
            };
            // Load all rows from a table
            PagedEnumerable<TableDataList, BigQueryRow> result = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_ListRows_Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableSchema_Google_Cloud_BigQuery_V2_ListRowsOptions_(
                tableReference: tableReference,
                schema: null
            );
            // Print the first 10 rows
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryRow.html row in result.Take(10))
            {
                Console.WriteLine($"{row["corpus"]}: {row["word_count"]}");
            }
        }
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.TableDataListOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to directly browse a table with optional paging
    public class BrowseTable {

      public static void runBrowseTable() {
        // TODO(developer): Replace these variables before running the sample.
        String table = "MY_TABLE_NAME";
        String dataset = "MY_DATASET_NAME";
        browseTable(dataset, table);
      }

      public static void browseTable(String dataset, String table) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Identify the table itself
          TableId tableId = TableId.of(dataset, table);

          // Page over 100 records. If you don't need pagination, remove the pageSize parameter.
          TableResult result = bigquery.listTableData(tableId, TableDataListOption.pageSize(100));

          // Print the records
          result
              .iterateAll()
              .forEach(
                  row -> {
                    row.forEach(fieldValue -> System.out.print(fieldValue.toString() + ", "));
                    System.out.println();
                  });

          System.out.println("Query ran successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Query failed to run \n" + e.toString());
        }
      }
    }

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


The [Cloud Client Libraries for Go](https://docs.cloud.google.com/bigquery/docs/reference/libraries)
automatically paginates by default, so you do not need to implement pagination
yourself, for example:

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // browseTable demonstrates reading data from a BigQuery table directly without the use of a query.
    // For large tables, we also recommend the BigQuery Storage API.
    func browseTable(w io.Writer, projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	table := client.Dataset(datasetID).Table(tableID)
    	it := table.Read(ctx)
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

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


The [Cloud Client Libraries for Node.js](https://googleapis.dev/nodejs/bigquery/latest/index.html)
automatically paginates by default, so you do not need to implement pagination
yourself, for example:

    // Import the Google Cloud client library using default credentials
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function browseTable() {
      // Retrieve a table's rows using manual pagination.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset'; // Existing dataset
      // const tableId = 'my_table'; // Table to create

      const query = `SELECT name, SUM(number) as total_people
        FROM \`bigquery-public-data.usa_names.usa_1910_2013\`
        GROUP BY name 
        ORDER BY total_people 
        DESC LIMIT 100`;

      // Create table reference.
      const dataset = bigquery.dataset(datasetId);
      const destinationTable = dataset.table(tableId);

      // For all options, see https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationquery
      const queryOptions = {
        query: query,
        destination: destinationTable,
      };

      // Run the query as a job
      const [job] = await bigquery.createQueryJob(queryOptions);

      // For all options, see https://cloud.google.com/bigquery/docs/reference/v2/jobs/getQueryResults
      const queryResultsOptions = {
        // Retrieve zero resulting rows.
        maxResults: 0,
      };

      // Wait for the job to finish.
      await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/job.html(queryResultsOptions);

      function manualPaginationCallback(err, rows, nextQuery) {
        rows.forEach(row => {
          console.log(`name: ${row.name}, ${row.total_people} total people`);
        });

        if (nextQuery) {
          // More results exist.
          destinationTable.getRows(nextQuery, manualPaginationCallback);
        }
      }

      // For all options, see https://cloud.google.com/bigquery/docs/reference/v2/tabledata/list
      const getRowsOptions = {
        autoPaginate: false,
        maxResults: 20,
      };

      // Retrieve all rows.
      destinationTable.getRows(getRowsOptions, manualPaginationCallback);
    }
    browseTable();

### PHP


Before trying this sample, follow the PHP setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery PHP API
reference documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


Pagination happens automatically in the [Cloud Client Libraries for PHP](https://docs.cloud.google.com/bigquery/docs/reference/libraries)
using the generator function `rows`, which fetches the next page of
results during iteration.

    use Google\Cloud\BigQuery\BigQueryClient;

    /** Uncomment and populate these variables in your code */
    // $projectId = 'The Google project ID';
    // $datasetId = 'The BigQuery dataset ID';
    // $tableId   = 'The BigQuery table ID';
    // $maxResults = 10;

    $maxResults = 10;
    $startIndex = 0;

    $options = [
        'maxResults' => $maxResults,
        'startIndex' => $startIndex
    ];
    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $dataset = $bigQuery->dataset($datasetId);
    $table = $dataset->table($tableId);
    $numRows = 0;
    foreach ($table->rows($options) as $row) {
        print('---');
        foreach ($row as $column => $value) {
            printf('%s: %s' . PHP_EOL, $column, $value);
        }
        $numRows++;
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


The [Cloud Client Libraries for Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest)
automatically paginates by default, so you do not need to implement pagination
yourself, for example:


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to browse data rows.
    # table_id = "your-project.your_dataset.your_table_name"

    # Download all rows from a table.
    rows_iter = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_rows(table_id)  # Make an API request.

    # Iterate over rows to make the API requests to fetch row data.
    rows = list(rows_iter)
    print("Downloaded {} rows from table {}".format(len(rows), table_id))

    # Download at most 10 rows.
    rows_iter = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_rows(table_id, max_results=10)
    rows = list(rows_iter)
    print("Downloaded {} rows from table {}".format(len(rows), table_id))

    # Specify selected fields to limit the results to certain columns.
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    fields = table.schema[:2]  # First two columns.
    rows_iter = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_rows(table_id, selected_fields=fields, max_results=10)
    rows = list(rows_iter)
    print("Selected {} columns from table {}.".format(len(rows_iter.schema), table_id))
    print("Downloaded {} rows from table {}".format(len(rows), table_id))

    # Print row data in tabular format.
    rows = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_rows(table, max_results=10)
    format_string = "{!s:<16} " * len(https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.reader.ReadRowsStream.html#google_cloud_bigquery_storage_v1_reader_ReadRowsStream_rows.schema)
    field_names = [field.name for field in https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.reader.ReadRowsStream.html#google_cloud_bigquery_storage_v1_reader_ReadRowsStream_rows.schema]
    print(format_string.format(*field_names))  # Prints column headers.
    for row in rows:
        print(format_string.format(*row))  # Prints row data.

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://googleapis.dev/ruby/google-cloud-bigquery/latest/Google/Cloud/Bigquery.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


Pagination happens automatically in the [Cloud Client Libraries for Ruby](https://docs.cloud.google.com/bigquery/docs/reference/libraries)
using `Table#data` and `Data#next`.

    require "google/cloud/bigquery"

    def browse_table
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html project_id: "bigquery-public-data"
      dataset  = bigquery.dataset "samples"
      table    = dataset.table "shakespeare"

      # Load all rows from a table
      rows = table.data

      # Load the first 10 rows
      rows = table.data max: 10

      # Print row data
      rows.each { |row| puts row }
    end

<br />

## Request arbitrary pages and avoid redundant list calls

When you page backwards or jump to arbitrary pages using cached
`pageToken` values, it is possible that the data in your pages might
have changed since it was last viewed but there is no clear indication that
the data might have changed. To mitigate this, you can use the `etag`
property.

Every `collection.list` method (except for Tabledata) returns an
`etag` property in the result. This property is a hash of the page
results that can be used to verify whether the page has changed since the last
request. When you make a request to BigQuery with an ETag value,
BigQuery compares the ETag value to the ETag value returned by
the API and responds based on whether the ETag values match. You can use ETags
to avoid redundant list calls as follows:

- To return list values if the values have changed.

  If you only want to return a page of list values if the values have changed,
  you can make a list call with a previously-stored ETag using the
  [HTTP "if-none-match" header](http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.26).
  If the ETag you provide doesn't match the ETag on the server,
  BigQuery returns a page of new list values. If the ETags do
  match, BigQuery returns an `HTTP 304 Not Modified` status
  code and no values. An example of this might be a web page where users might
  periodically fill in information that is stored in BigQuery. If there are
  no changes to your data, you can avoid making redundant list calls to BigQuery by using the if-none-match header with ETags.
- To return list values if the values have not changed.

  If you only want to return a page of list values if the list values have
  not changed, you can use the
  [HTTP "if-match" header](http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.24).
  BigQuery matches the ETag values and returns the page of
  results if the results have not changed or returns a 412 "Precondition
  Failed" result if the page has changed.

**Note:** Although ETags are a great way to avoid making redundant list calls, you can apply the same methods to identifying if any objects have changed. For example, you can perform a Get request for a specific table and use ETags to determine if the table has changed before returning the full response.

## Page through query results

Each query writes to a destination table. If no destination table is provided,
the BigQuery API automatically populates the destination table
property with a reference to a [temporary anonymous
table](https://docs.cloud.google.com/bigquery/docs/writing-results#temporary_and_permanent_tables).

### API

Read the
[`jobs.config.query.destinationTable`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationQuery.FIELDS.destination_table)
field to determine the table that query results have been written to.
Call the [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list)
to read the query results.

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to run query with pagination.
    public class QueryPagination {

      public static void main(String[] args) {
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String query =
            "SELECT name, SUM(number) as total_people"
                + " FROM `bigquery-public-data.usa_names.usa_1910_2013`"
                + " GROUP BY name"
                + " ORDER BY total_people DESC"
                + " LIMIT 100";
        queryPagination(datasetName, tableName, query);
      }

      public static void queryPagination(String datasetName, String tableName, String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  // save results into a table.
                  .setDestinationTable(tableId)
                  .build();

          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results =
              bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_listTableData_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableDataListOption____(tableId, BigQuery.TableDataListOption.pageSize(20));

          // First Page
          results
              .getValues()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,\n", val.toString())));

          while (results.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_hasNextPage__()) {
            // Remaining Pages
            results = results.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_getNextPage__();
            results
                .getValues()
                .forEach(row -> row.forEach(val -> System.out.printf("%s,\n", val.toString())));
          }

          System.out.println("Query pagination performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

To set the number of rows returned on each page, use a
[`GetQueryResults` job](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job#com_google_cloud_bigquery_Job_getQueryResults_com_google_cloud_bigquery_BigQuery_QueryResultsOption____) and set the
[`pageSize` option](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.QueryResultsOption#com_google_cloud_bigquery_BigQuery_QueryResultsOption_pageSize_long_)
of the [`QueryResultsOption` object](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.QueryResultsOption) that you pass in, as shown in the
following example:

    TableResult result = job.getQueryResults();
    QueryResultsOption queryResultsOption = QueryResultsOption.pageSize(20);

    TableResult result = job.getQueryResults(queryResultsOption);

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    // Import the Google Cloud client library using default credentials
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryPagination() {
      // Run a query and get rows using automatic pagination.

      const query = `SELECT name, SUM(number) as total_people
      FROM \`bigquery-public-data.usa_names.usa_1910_2013\`
      GROUP BY name
      ORDER BY total_people DESC
      LIMIT 100`;

      // Run the query as a job.
      const [job] = await bigquery.createQueryJob(query);

      // Wait for job to complete and get rows.
      const [rows] = await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/job.html();

      console.log('Query results:');
      rows.forEach(row => {
        console.log(`name: ${row.name}, ${row.total_people} total people`);
      });
    }
    queryPagination();

### Python

The
[`QueryJob.result`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJob#google_cloud_bigquery_job_QueryJob_result)
method returns an iterable of the query results. Alternatively,

1. Read the [`QueryJob.destination`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJob#google_cloud_bigquery_job_QueryJob_destination) property. If this property is not configured, it is set by the API to a reference to a [temporary anonymous
   table](https://docs.cloud.google.com/bigquery/docs/writing-results#temporary_and_permanent_tables).
2. Get the table schema with the [`Client.get_table`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_get_table) method.
3. Create an iterable over all rows in the destination table with the [`Client.list_rows`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_list_rows) method.


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

    query = """
        SELECT name, SUM(number) as total_people
        FROM `bigquery-public-data.usa_names.usa_1910_2013`
        GROUP BY name
        ORDER BY total_people DESC
    """
    query_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(query)  # Make an API request.
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dbapi.Cursor.html#google_cloud_bigquery_dbapi_Cursor_query_job.result()  # Wait for the query to complete.

    # Get the destination table for the query results.
    #
    # All queries write to a destination table. If a destination table is not
    # specified, the BigQuery populates it with a reference to a temporary
    # anonymous table after the query completes.
    destination = https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dbapi.Cursor.html#google_cloud_bigquery_dbapi_Cursor_query_job.destination

    # Get the schema (and other properties) for the destination table.
    #
    # A schema is useful for converting from BigQuery types to Python types.
    destination = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(destination)

    # Download rows.
    #
    # The client library automatically handles pagination.
    print("The query data:")
    rows = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_rows(destination, max_results=20)
    for row in rows:
        print("name={}, count={}".format(row["name"], row["total_people"]))