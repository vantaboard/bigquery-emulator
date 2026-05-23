This document describes how to stream data into BigQuery by using the
legacy [`tabledata.insertAll`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tabledata/insertAll)
method.

For new projects, we recommend using the
[BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) instead of the
`tabledata.insertAll` method. The Storage Write API has lower
pricing and more robust features, including exactly-once delivery semantics. If
you are migrating an existing project from the `tabledata.insertAll` method
to the Storage Write API, we recommend selecting the
[default stream](https://docs.cloud.google.com/bigquery/docs/write-api-streaming#at-least-once). The
`tabledata.insertAll` method is still fully supported.

## Before you begin

1. Ensure that you have write access to the dataset that contains your
   destination table. The table must exist before you begin writing data to it
   unless you are using template tables. For more information on template tables,
   see [Creating tables automatically using template tables](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#template-tables).

2. Check the
   [quota policy for streaming data](https://docs.cloud.google.com/bigquery/quotas#streaming_inserts).


3.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

4. Streaming is not available through the [free tier](https://cloud.google.com/bigquery/pricing#free-tier). If you attempt to use streaming without enabling billing, you receive the following error: `BigQuery: Streaming insert is not allowed in the free tier.`
5. Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document.

### Required permissions

To stream data into BigQuery, you need the following IAM permissions:

- `bigquery.tables.updateData` (lets you insert data into the table)
- `bigquery.tables.get` (lets you obtain table metadata)
- `bigquery.datasets.get` (lets you obtain dataset metadata)
- `bigquery.tables.create` (required if you use a [template table](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#template-tables) to create the table automatically)

Each of the following predefined IAM roles includes the permissions that you need in order to stream data into BigQuery:

- `roles/bigquery.dataEditor`
- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

For more information about IAM roles and permissions in
BigQuery, see
[Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Stream data into BigQuery

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


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;

    public class BigQueryTableInsertRows
    {
        public void TableInsertRows(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_id",
            string tableId = "your_table_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryInsertRow.html[] rows = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryInsertRow.html[]
            {
                // The insert ID is optional, but can avoid duplicate data
                // when retrying inserts.
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryInsertRow.html(insertId: "row1") {
                    { "name", "Washington" },
                    { "post_abbr", "WA" }
                },
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryInsertRow.html(insertId: "row2") {
                    { "name", "Colorado" },
                    { "post_abbr", "CO" }
                }
            };
            client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_InsertRows_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_BigQueryInsertRow___(datasetId, tableId, rows);
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

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // Item represents a row item.
    type Item struct {
    	Name string
    	Age  int
    }

    // Save implements the ValueSaver interface.
    // This example disables best-effort de-duplication, which allows for higher throughput.
    func (i *Item) Save() (map[string]bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Value, string, error) {
    	return map[string]bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Value{
    		"full_name": i.Name,
    		"age":       i.Age,
    	}, bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_NoDedupeID, nil
    }

    // insertRows demonstrates inserting data into a table using the streaming insert mechanism.
    func insertRows(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	inserter := client.Dataset(datasetID).Table(tableID).Inserter()
    	items := []*Item{
    		// Item implements the ValueSaver interface.
    		{Name: "Phred Phlyntstone", Age: 32},
    		{Name: "Wylma Phlyntstone", Age: 29},
    	}
    	if err := inserter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Inserter_Put(ctx, items); err != nil {
    		return err
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryError.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.InsertAllRequest.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.InsertAllResponse.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import java.util.HashMap;
    import java.util.List;
    import java.util.Map;

    // Sample to inserting rows into a table without running a load job.
    public class TableInsertRows {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        // Create a row to insert
        Map<String, Object> rowContent = new HashMap<>();
        rowContent.put("booleanField", true);
        rowContent.put("numericField", "3.14");
        // TODO(developer): Replace the row id with a unique value for each row.
        String rowId = "ROW_ID";
        tableInsertRows(datasetName, tableName, rowId, rowContent);
      }

      public static void tableInsertRows(
          String datasetName, String tableName, String rowId, Map<String, Object> rowContent) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Get table
          TableId tableId = TableId.of(datasetName, tableName);

          // Inserts rowContent into datasetName:tableId.
          InsertAllResponse response =
              bigquery.insertAll(
                  InsertAllRequest.newBuilder(tableId)
                      // More rows can be added in the same RPC by invoking .addRow() on the builder.
                      // You can omit the unique row ids to disable de-duplication.
                      .addRow(rowId, rowContent)
                      .build());

          if (response.hasErrors()) {
            // If any of the insertions failed, this lets you inspect the errors
            for (Map.Entry<Long, List<BigQueryError>> entry : response.getInsertErrors().entrySet()) {
              System.out.println("Response error: \n" + entry.getValue());
            }
          }
          System.out.println("Rows successfully inserted into table");
        } catch (BigQueryException e) {
          System.out.println("Insert operation not performed \n" + e.toString());
        }
      }
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

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function insertRowsAsStream() {
      // Inserts the JSON objects into my_dataset:my_table.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';
      const rows = [
        {name: 'Tom', age: 30},
        {name: 'Jane', age: 32},
      ];

      // Insert data into a table
      await bigquery.dataset(datasetId).table(tableId).insert(rows);
      console.log(`Inserted ${rows.length} rows`);
    }

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

    use Google\Cloud\BigQuery\BigQueryClient;

    /**
     * Stream data into bigquery
     *
     * @param string $projectId The project Id of your Google Cloud Project.
     * @param string $datasetId The BigQuery dataset ID.
     * @param string $tableId The BigQuery table ID.
     * @param string $data Json encoded data For eg,
     *    $data = json_encode([
     *       "field1" => "value1",
     *       "field2" => "value2",
     *    ]);
     */
    function stream_row(
        string $projectId,
        string $datasetId,
        string $tableId,
        string $data
    ): void {
        // instantiate the bigquery table service
        $bigQuery = new BigQueryClient([
          'projectId' => $projectId,
        ]);
        $dataset = $bigQuery->dataset($datasetId);
        $table = $dataset->table($tableId);

        $data = json_decode($data, true);
        $insertResponse = $table->insertRows([
          ['data' => $data],
          // additional rows can go here
        ]);
        if ($insertResponse->isSuccessful()) {
            print('Data streamed into BigQuery successfully' . PHP_EOL);
        } else {
            foreach ($insertResponse->failedRows() as $row) {
                foreach ($row['errors'] as $error) {
                    printf('%s: %s' . PHP_EOL, $error['reason'], $error['message']);
                }
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

    # TODO(developer): Set table_id to the ID of table to append to.
    # table_id = "your-project.your_dataset.your_table"

    rows_to_insert = [
        {"full_name": "Phred Phlyntstone", "age": 32},
        {"full_name": "Wylma Phlyntstone", "age": 29},
    ]

    errors = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_insert_rows_json(table_id, rows_to_insert)  # Make an API request.
    if errors == []:
        print("New rows have been added.")
    else:
        print("Encountered errors while inserting rows: {}".format(errors))

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

    require "google/cloud/bigquery"

    def table_insert_rows dataset_id = "your_dataset_id", table_id = "your_table_id"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      dataset  = bigquery.dataset dataset_id
      table    = dataset.table table_id

      row_data = [
        { name: "Alice", value: 5  },
        { name: "Bob",   value: 10 }
      ]
      response = table.insert row_data

      if response.success?
        puts "Inserted rows successfully"
      else
        puts "Failed to insert #{response.error_rows.count} rows"
      end
    end

<br />

> [!NOTE]
> **Note:** To specify a `NUMERIC` or `BIGNUMERIC` value in a row, you must surround the value with double quotation marks, such as `"big_numeric_col":"0.123456789123"`.

You don't need to populate the `insertID` field when you insert rows.
The following example shows how to avoid sending an `insertID` for each row
when streaming.

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryError.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.InsertAllRequest.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.InsertAllResponse.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.common.collect.ImmutableList;
    import java.util.HashMap;
    import java.util.List;
    import java.util.Map;

    // Sample to insert rows without row ids in a table
    public class TableInsertRowsWithoutRowIds {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        tableInsertRowsWithoutRowIds(datasetName, tableName);
      }

      public static void tableInsertRowsWithoutRowIds(String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          final BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();
          // Create rows to insert
          Map<String, Object> rowContent1 = new HashMap<>();
          rowContent1.put("stringField", "Phred Phlyntstone");
          rowContent1.put("numericField", 32);
          Map<String, Object> rowContent2 = new HashMap<>();
          rowContent2.put("stringField", "Wylma Phlyntstone");
          rowContent2.put("numericField", 29);
          InsertAllResponse response =
              bigquery.insertAll(
                  InsertAllRequest.newBuilder(TableId.of(datasetName, tableName))
                      // No row ids disable de-duplication, and also disable the retries in the Java
                      // library.
                      .setRows(
                          ImmutableList.of(
                              InsertAllRequest.RowToInsert.of(rowContent1),
                              InsertAllRequest.RowToInsert.of(rowContent2)))
                      .build());

          if (response.hasErrors()) {
            // If any of the insertions failed, this lets you inspect the errors
            for (Map.Entry<Long, List<BigQueryError>> entry : response.getInsertErrors().entrySet()) {
              System.out.println("Response error: \n" + entry.getValue());
            }
          }
          System.out.println("Rows successfully inserted into table without row ids");
        } catch (BigQueryException e) {
          System.out.println("Insert operation not performed \n" + e.toString());
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

    # TODO(developer): Set table_id to the ID of table to append to.
    # table_id = "your-project.your_dataset.your_table"

    rows_to_insert = [
        {"full_name": "Phred Phlyntstone", "age": 32},
        {"full_name": "Wylma Phlyntstone", "age": 29},
    ]

    errors = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_insert_rows_json(
        table_id, rows_to_insert, row_ids=[None] * len(rows_to_insert)
    )  # Make an API request.
    if errors == []:
        print("New rows have been added.")
    else:
        print("Encountered errors while inserting rows: {}".format(errors))

<br />

### Send date and time data

For date and time fields, format the data in the `tabledata.insertAll` method as
follows:

| Type | Format |
|---|---|
| `DATE` | A string in the form `"YYYY-MM-DD"` |
| `DATETIME` | A string in the form `"YYYY-MM-DD [HH:MM:SS]"` |
| `TIME` | A string in the form `"HH:MM:SS"` |
| `TIMESTAMP` | The number of seconds since 1970-01-01 (the Unix epoch), or a string in the form `"YYYY-MM-DD HH:MM[:SS]"` |

### Send range data

For fields with type `RANGE<T>`, format the data in the `tabledata.insertAll`
method as a JSON object with two fields, `start` and `end`.
Missing or NULL values for the `start` and `end` fields represent unbounded boundaries.
These fields must have the same supported JSON format of type `T`, where
`T` can be one of `DATE`, `DATETIME`, and `TIMESTAMP`.

In the following example, the `f_range_date` field represents a `RANGE<DATE>`
column in a table. A row is inserted into this column using the
`tabledata.insertAll` API.

    {
        "f_range_date": {
            "start": "1970-01-02",
            "end": null
        }
    }

## Stream data availability

Data is available for real-time analysis using [GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql)
queries immediately after BigQuery successfully acknowledges a
`tabledata.insertAll` request. When you query data in the streaming buffer,
you aren't charged for bytes processed from streaming buffer if you use
on-demand compute pricing. If you use capacity-based pricing, your
[reservations](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management)
consume slots for processing data in streaming buffer.

Recently streamed rows to an ingestion time partitioned table temporarily have a
NULL value for the [`_PARTITIONTIME`](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table)
pseudocolumn. For such rows, BigQuery assigns the final non-NULL
value of the `PARTITIONTIME` column in the background, typically within a few
minutes. In rare cases, this can take up to 90 minutes.

Some recently streamed rows might not be available for table copy typically for a
few minutes. In rare cases, this can take up to 90 minutes. To see whether data
is available for table copy, check the `tables.get` response for a section named
[`streamingBuffer`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#streamingbuffer).
If the `streamingBuffer` section is absent, your data is available for copy.
You can also use the `streamingBuffer.oldestEntryTime` field to identify the
age of records in the streaming buffer.

## Best effort de-duplication

When you supply `insertId` for an inserted row, BigQuery uses this
ID to support best effort de-duplication for up to one minute. That is, if
you stream the same row with the same `insertId` more than once within
that time period into the same table, BigQuery *might* de-duplicate
the multiple occurrences of that row, retaining only one of those occurrences.

The system expects that rows provided with identical `insertId`s are also
identical. If two rows have identical `insertId`s, it is nondeterministic
which row BigQuery preserves.

De-duplication is generally meant for retry scenarios in a distributed system where there's
no way to determine the state of a streaming insert under certain error
conditions, such as network errors between your system and BigQuery
or internal errors within BigQuery.
If you retry an insert, use the same `insertId` for the same set of rows so that
BigQuery can attempt to de-duplicate your data. For more
information, see [troubleshooting streaming inserts](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#troubleshooting).

De-duplication offered by BigQuery is best effort, and it should
not be relied upon as a mechanism to guarantee the absence of duplicates in your
data. Additionally, BigQuery might degrade the quality of best
effort de-duplication at any time in order to guarantee higher
reliability and availability for your data.

If you have strict de-duplication requirements for your data,
[Google Cloud Datastore](https://docs.cloud.google.com/datastore) is an alternative service that supports
[transactions](https://docs.cloud.google.com/datastore/docs/concepts/transactions).

### Disabling best effort de-duplication

You can disable best effort de-duplication by not populating the `insertId`
field for each row inserted. This is the recommended way to insert data.

#### Apache Beam and Dataflow

To disable best effort de-duplication when you use Apache Beam's
[BigQuery I/O connector](https://beam.apache.org/documentation/io/built-in/google-bigquery)
for Java, use the
[`ignoreInsertIds()` method](https://beam.apache.org/releases/javadoc/current/org/apache/beam/sdk/io/gcp/bigquery/BigQueryIO.Write.html).

### Manually removing duplicates

To ensure that no duplicate rows exist after you are done streaming, use the following manual process:

1. Add the `insertId` as a column in your table schema and include the `insertId` value in the data for each row.
2. After streaming has stopped, perform the following query to check for duplicates: <br />

   ```googlesql
   #standardSQL
   SELECT
     MAX(count) FROM(
     SELECT
       ID_COLUMN,
       count(*) as count
     FROM
       `TABLE_NAME`
     GROUP BY
       ID_COLUMN)
   ```

   <br />

   If the result is greater than 1, duplicates exist.
3. To remove duplicates, run the following query. Specify a destination table, allow large results, and disable result flattening. <br />

   ```googlesql
   #standardSQL
   SELECT
     * EXCEPT(row_number)
   FROM (
     SELECT
       *,
       ROW_NUMBER()
             OVER (PARTITION BY ID_COLUMN) row_number
     FROM
       `TABLE_NAME`)
   WHERE
     row_number = 1
   ```

   <br />

Notes about the duplicate removal query:

- The safer strategy for the duplicate removal query is to target a new table. Alternatively, you can target the source table with write disposition `WRITE_TRUNCATE`.
- The duplicate removal query adds a `row_number` column with the value `1` to the end of the table schema. The query uses a [`SELECT * EXCEPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_except) statement from [GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql) to exclude the `row_number` column from the destination table. The `#standardSQL` prefix [enables](https://docs.cloud.google.com/bigquery/docs/introduction-sql) GoogleSQL for this query. Alternatively, you can select by specific column names to omit this column.
- For querying live data with duplicates removed, you can also create a view over your table using the duplicate removal query. Be aware that query costs against the view are calculated based on the columns selected in your view, which can result in large bytes scanned sizes.

## Stream into time-partitioned tables

When you stream data to a time-partitioned table, each partition has a streaming
buffer. The streaming buffer is retained when you perform a load, query, or copy
job that overwrites a partition by setting the `writeDisposition` property to
`WRITE_TRUNCATE`. If you want to remove the streaming buffer, verify that the
streaming buffer is empty by calling
[`tables.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get) on the partition.

### Ingestion-time partitioning

When you stream to an ingestion-time partitioned table, BigQuery
infers the destination partition from the current UTC time.

Newly arriving data is temporarily placed in the `__UNPARTITIONED__` partition
while in the streaming buffer. When there's enough unpartitioned data,
BigQuery partitions the data into the correct partition. However,
there is no SLA for how long it takes for data to move out of the
`__UNPARTITIONED__` partition. A query
can exclude data in the streaming buffer from a query by filtering out the
`NULL` values from the `__UNPARTITIONED__` partition by using one of the
pseudocolumns ([`_PARTITIONTIME`
or `_PARTITIONDATE`](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table)
depending on your preferred data type).

If you are streaming data into a daily partitioned table, then you can override
the date inference by supplying a partition decorator as part of the `insertAll`
request. Include the decorator in the `tableId` parameter. For example, you can
stream to the partition corresponding to 2021-03-01 for table `table1` using the
partition decorator:

    table1$20210301

When streaming using a partition decorator, you can stream to partitions within
the last 31 days in the past and 16 days in the future relative to the current
date, based on current UTC time. To write to partitions for dates outside these
allowed bounds, use a load or query job instead, as described in
[Appending to and overwriting partitioned table data](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-table-data#append-overwrite).

Streaming using a partition decorator is only supported for daily partitioned
tables. It is not supported for hourly, monthly, or yearly partitioned tables.

For testing, you can use the bq command-line tool
[`bq insert`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_insert) CLI command.
For example, the following command streams a single row to a partition for the
date January 1, 2017 (`$20170101`) into a partitioned table named
`mydataset.mytable`:

    echo '{"a":1, "b":2}' | bq insert 'mydataset.mytable$20170101'

> [!CAUTION]
> **Caution:** The `bq insert` command is intended for testing only.

### Time-unit column partitioning

You can stream data into a table partitioned on a `DATE`, `DATETIME`, or
`TIMESTAMP` column that is between 10 years in the past and 1 year in the future.
Data outside this range is rejected.

When the data is streamed, it is initially placed in the `__UNPARTITIONED__`
partition. When there's enough unpartitioned data, BigQuery
automatically repartitions the data, placing it into the appropriate partition.
However, there is no SLA for how long it takes for data to move out of the
`__UNPARTITIONED__` partition.

- Note: Daily partitions are processed differently than hourly, monthly and yearly partitions. Only data outside of the date range (last 7 days to future 3 days) is extracted to the UNPARTITIONED partition, waiting to be repartitioned. On the other hand, for hourly partitioned table, data is always extracted to the UNPARTITIONED partition, and later repartitioned.

## Create tables automatically using template tables

*Template tables* provide a mechanism to split a logical table into many smaller
tables to create smaller sets of data (for example, by user ID). Template tables
have a number of limitations described below. Instead,
[partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) and
[clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables) are the recommended ways to
achieve this behavior.

To use a template table through the BigQuery API, add a `templateSuffix` parameter
to your `insertAll` request. For the bq command-line tool, add the `template_suffix` flag
to your `insert` command. If BigQuery detects a `templateSuffix`
parameter or the `template_suffix` flag, it treats the targeted table as a base
template. It creates a new table that shares the same schema as the targeted
table and has a name that includes the specified suffix:

```
<targeted_table_name> + <templateSuffix>
```

By using a template table, you avoid the overhead of creating each table
individually and specifying the schema for each table. You need only create
a single template, and supply different suffixes so that BigQuery can create
the new tables for you. BigQuery places the tables in the same project
and dataset.

Tables created by using template tables are usually available within a few seconds.
On rare occasions, they may take longer to become available.

### Change the template table schema

If you change a template table schema, all tables that are generated subsequently
use the updated schema. Previously generated tables are not affected,
unless the existing table still has a streaming buffer.

For existing tables that still have a streaming buffer, if you modify the
template table schema in a backward compatible way, the schema of those
actively streamed generated tables is also updated. However,
if you modify the template table schema in a non-backward compatible way,
any buffered data that uses the old schema is lost. Also, you
cannot stream new data to existing generated tables that use
the old, but now incompatible, schema.

After you change a template table schema, wait until the changes have propagated
before you try to insert new data or query the generated tables. Requests to insert
new fields should succeed within a few minutes. Attempts to query the new
fields might require a longer wait of up to 90 minutes.

If you want to change a generated table's schema, do not change the
schema until streaming through the template table has ceased and the generated
table's streaming statistics section is absent from the `tables.get()` response,
which indicates that no data is buffered on the table.

[Partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) and
[clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables) do not suffer from the
preceding limitations and are the recommended mechanism.

### Template table details

Template suffix value
:   The `templateSuffix` (or `--template_suffix`) value must contain only letters
    (a-z, A-Z), numbers (0-9), or underscores (_). The maximum combined length
    of the table name and the table suffix is 1024 characters.

Quota

:   Template tables are subject to [streaming quota](https://docs.cloud.google.com/bigquery/quotas#streaming_inserts)
    limitations. Your project can make up to 10 tables per second with template tables, similar
    to the [`tables.insert`](https://docs.cloud.google.com/bigquery/quotas#tables.insert_calls_per_second) API. This quota only applies
    to tables being created, not to tables being modified.

:   If your application needs to create more than 10 tables per second, we recommend using
    [clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables).
    For example, you can put the high cardinality table ID into the key column of a single clustering table.

Time to live

:   The generated table inherits its expiration time from the dataset. As with
    normal streaming data, generated tables cannot be copied immediately.

Deduplication

:   Deduplication only happens between uniform references to a destination table.
    For example, if you simultaneously stream to a generated table using both
    template tables and a regular `insertAll` command, no deduplication occurs
    between rows inserted by template tables and a regular `insertAll` command.

Views

:   The template table and the generated tables shouldn't be views.

## Troubleshoot streaming inserts

The following sections discuss how to troubleshoot errors that occur
when you [stream data into
BigQuery using the legacy streaming API](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery). For more
information on how to resolve quota errors for streaming inserts, see
[Streaming insert quota
errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-streaming-insert-quota).

### Failure HTTP response codes

If you receive a failure HTTP response code such as a network error, there's
no way to tell whether the streaming insert succeeded. If you try to re-send
the request, you might end up with duplicated rows in your table. To help
protect your table against duplication, set the `insertId` property when
sending your request. BigQuery uses the `insertId` property
for de-duplication.

If you receive a permission error, an invalid table name error, or an exceeded
quota error, no rows are inserted and the entire request fails.

### Success HTTP response codes

Even if you receive a [success HTTP response code](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#response-body), you'll need to check the
`insertErrors` property of the response to determine whether the row insertions
were successful because it's possible that BigQuery was only partially
successful at inserting the rows. You might encounter one of the following scenarios:

- **All rows inserted successfully.** If the `insertErrors` property is an empty list, all of the rows were inserted successfully.
- **Some rows inserted successfully.** Except in cases where there is a schema mismatch in any of the rows, rows indicated in the `insertErrors` property are not inserted, and all other rows are inserted successfully. The `errors` property contains detailed information about why each unsuccessful row failed. The `index` property indicates the 0-based row index of the request that the error applies to.
- **None of the rows inserted successfully.** If BigQuery encounters a schema mismatch on individual rows in the request, none of the rows are inserted and an `insertErrors` entry is returned for each row, even the rows that did not have a schema mismatch. Rows that did not have a schema mismatch have an error with the `reason` property set to `stopped`, and can be re-sent as-is. Rows that failed include detailed information about the schema mismatch. To learn about the supported protocol buffer types for each BigQuery data type, see [Supported protocol buffer and Arrow data types](https://docs.cloud.google.com/bigquery/docs/supported-data-types).

### Metadata errors for streaming inserts

Because BigQuery's streaming API is designed for high insertion rates,
modifications to the underlying table metadata exhibit are eventually consistent when interacting
with the streaming system. Most of the time, metadata changes are propagated within minutes, but
during this period API responses might reflect the inconsistent state of the table.

Some scenarios include:

- **Schema Changes**. Modifying the schema of a table that has recently received streaming inserts can cause responses with schema mismatch errors because the streaming system might not immediately pick up the schema change.
- **Table Creation/Deletion** . Streaming to a nonexistent table returns a variation of a `notFound` response. A table created in response might not immediately be recognized by subsequent streaming inserts. Similarly, deleting or recreating a table can create a period of time where streaming inserts are effectively delivered to the old table. The streaming inserts might not be present in the new table.
- **Table Truncation**. Truncating a table's data (by using a query job that uses writeDisposition of WRITE_TRUNCATE) can similarly cause subsequent inserts during the consistency period to be dropped.

### Missing/Unavailable data

Streaming inserts reside temporarily in the write-optimized storage, which has different availability
characteristics than managed storage. Certain operations in BigQuery don't interact
with the write-optimized storage, such as table copy jobs and API methods like `tabledata.list`.
Recent streaming data won't be present in the destination table or output.

### Streaming insert quota errors

This section provides tips for troubleshooting quota errors related to
streaming data into BigQuery.

In certain regions, streaming inserts have a higher quota if you don't populate
the `insertId` field for each row. For more information about quotas for
streaming inserts, see [Streaming inserts](https://docs.cloud.google.com/bigquery/quotas#streaming_inserts).
The quota-related errors for BigQuery streaming depend on the
presence or absence of `insertId`.

**Error message**

If the `insertId` field is empty, the following quota error is possible:

| Quota limit | Error message |
|---|---|
| Bytes per second per project | Your entity with gaia_id: <var translate="no">GAIA_ID</var>, project: <var translate="no">PROJECT_ID</var> in region: <var translate="no">REGION</var> exceeded quota for insert bytes per second. |

If the `insertId` field is populated, the following quota errors are possible:

| Quota limit | Error message |
|---|---|
| Rows per second per project | Your project: <var translate="no">PROJECT_ID</var> in <var translate="no">REGION</var> exceeded quota for streaming insert rows per second. |
| Rows per second per table | Your table: <var translate="no">TABLE_ID</var> exceeded quota for streaming insert rows per second. |
| Bytes per second per table | Your table: <var translate="no">TABLE_ID</var> exceeded quota for streaming insert bytes per second. |

The purpose of the `insertId` field is to deduplicate inserted rows. If multiple
inserts with the same `insertId` arrive within a few minutes' window,
BigQuery writes a single version of the record. However, this
automatic deduplication is not guaranteed. For maximum streaming throughput, we
recommend that you don't include `insertId` and instead use
[manual deduplication](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#manually_removing_duplicates).
For more information, see
[Ensuring data consistency](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#dataconsistency).

When you encounter this error, [diagnose the issue](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#ts-streaming-insert-quota-diagnose)
the issue and then [follow the recommended steps](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#ts-streaming-insert-quota-resolution) to resolve it.

#### Diagnosis

Use the [`STREAMING_TIMELINE_BY_*`](https://docs.cloud.google.com/bigquery/docs/information-schema-streaming)
views to analyze the streaming traffic. These views aggregate streaming
statistics over one-minute intervals, grouped by `error_code`. Quota errors appear
in the results with `error_code` equal to `RATE_LIMIT_EXCEEDED` or
`QUOTA_EXCEEDED`.

Depending on the specific quota limit that was reached, look at `total_rows` or
`total_input_bytes`. If the error is a table-level quota, filter by `table_id`.

For example, the following query shows total bytes ingested per minute, and the
total number of quota errors:

```googlesql
SELECT
 start_timestamp,
 error_code,
 SUM(total_input_bytes) as sum_input_bytes,
 SUM(IF(error_code IN ('QUOTA_EXCEEDED', 'RATE_LIMIT_EXCEEDED'),
     total_requests, 0)) AS quota_error
FROM
 `region-REGION_NAME`.INFORMATION_SCHEMA.STREAMING_TIMELINE_BY_PROJECT
WHERE
  start_timestamp > TIMESTAMP_SUB(CURRENT_TIMESTAMP, INTERVAL 1 DAY)
GROUP BY
 start_timestamp,
 error_code
ORDER BY 1 DESC
```

#### Resolution

To resolve this quota error, do the following:

- If you are using the `insertId` field for deduplication, and your project is
  in a region that supports the higher streaming quota, we recommend removing the
  `insertId` field. This solution might require some additional steps to manually
  deduplicate the data. For more information, see
  [Manually removing duplicates](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#manually_removing_duplicates).

- If you are not using `insertId`, or if it's not feasible to remove it, monitor
  your streaming traffic over a 24-hour period and analyze the quota errors:

  - If you see mostly `RATE_LIMIT_EXCEEDED` errors rather than `QUOTA_EXCEEDED`
    errors, and your overall traffic is less than 80% of quota, the errors probably
    indicate temporary spikes. You can address these errors by retrying the
    operation using exponential backoff between retries.

  - If you are using a Dataflow job to insert data, consider using
    load jobs instead of streaming
    inserts. For more information, see [Setting the insertion
    method](https://beam.apache.org/documentation/io/built-in/google-bigquery/#setting-the-insertion-method).
    If you are using Dataflow with a custom I/O connector, consider
    using a built-in I/O connector instead. For more information, see [Custom
    I/O patterns](https://beam.apache.org/documentation/patterns/custom-io/).

  - If you see `QUOTA_EXCEEDED` errors or the overall traffic consistently
    exceeds 80% of the quota, submit a request for a quota increase. For more
    information, see
    [Request a quota adjustment](https://docs.cloud.google.com/docs/quotas/help/request_increase).

  - You might also want to consider replacing streaming inserts with the newer
    [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api), which has higher throughput,
    lower price, and many useful features.