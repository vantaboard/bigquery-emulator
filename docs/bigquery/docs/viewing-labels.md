# Viewing labels

This page explains how to view labels on your BigQuery resources.

You can view labels by:

- Using the Google Cloud console
- Querying `INFORMATION_SCHEMA` views
- Using the bq command-line tool's `bq show` command
- Calling the [`datasets.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get) or [`tables.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get) API methods
- Using the client libraries

Because views are treated like table resources, you use the `tables.get`
method to get label information for both views and tables.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document.

### Required permissions

The permissions required for viewing labels depend on the types of resources
you can access. To perform the tasks in this document, you need the following
permissions.

#### Permissions to view dataset details

To view dataset details, you need the `bigquery.datasets.get` IAM permission.

Each of the following predefined IAM roles includes the permissions that you need in order to view dataset details:

- `roles/bigquery.user`
- `roles/bigquery.metadataViewer`
- `roles/bigquery.dataViewer`
- `roles/bigquery.dataOwner`
- `roles/bigquery.dataEditor`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can view details of the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

#### Permissions to view table or view details

To view table or view details, you need the `bigquery.tables.get` IAM permission.

All predefined IAM roles include the permissions that you need in order to view table or view details **except for** `roles/bigquery.user` and `roles/bigquery.jobUser`.

Additionally, if you have the `bigquery.datasets.create` permission, you can view details of the tables and views in the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

#### Permissions to view job details

To view job details, you need the `bigquery.jobs.get` IAM
permission.

Each of the following predefined IAM roles includes the
permissions that you need in order to view job details:

- `roles/bigquery.admin` (lets you view details of all the jobs in the project)
- `roles/bigquery.user` (lets you view details of your jobs)
- `roles/bigquery.jobUser` (lets you view details of your jobs)

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## View dataset, table, and view labels

To view a resource's labels, select one of the following options:

### Console

1. For datasets, the dataset details page is automatically opened. For
   tables and views, click **Details** to open the details page. Label
   information appears in the information table for the resource.

   ![Table details](https://docs.cloud.google.com/static/bigquery/images/table-details.png)

### SQL

Query the
[`INFORMATION_SCHEMA.SCHEMATA_OPTIONS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets#schemata_options_view)
to see the labels on a dataset, or the
[`INFORMATION_SCHEMA.TABLE_OPTIONS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-options)
to see the labels on a table. For example, the following SQL query returns
the labels on the dataset named `mydataset`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     *
   FROM
     INFORMATION_SCHEMA.SCHEMATA_OPTIONS
   WHERE
     schema_name = 'mydataset'
     AND option_name = 'labels';
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq show` command with the resource ID. The `--format` flag can be
used to control the output. If the resource is in a project other than your
default project, add the project ID in the following format:
`[PROJECT_ID]:[DATASET]`. For readability, the output is controlled by
setting the `--format` flag to `pretty`.

    bq show --format=pretty [RESOURCE_ID]

Where `[RESOURCE_ID]` is a valid dataset, table, view, or job ID.

Examples:

Enter the following command to display labels for `mydataset` in your
default project.

    bq show --format=pretty mydataset

The output looks like the following:

```
+---+---+---+
|  Last modified  |                          ACLs                          |       Labels        |
+---+---+---+
| 11 Jul 19:34:34 | Owners:                                                | department:shipping |
|                 |   projectOwners,                                       |                     |
|                 | Writers:                                               |                     |
|                 |   projectWriters                                       |                     |
|                 | Readers:                                               |                     |
|                 |   projectReaders                                       |                     |
+---+---+---+
```

Enter the following command to display labels for `mydataset.mytable`.
`mydataset` is in `myotherproject`, not your default project.

    bq show --format=pretty myotherproject:mydataset.mytable

The output looks like the following for a clustered table:

```
+---+---+---+---+---+---+---+---+
|  Last modified  |            Schema            | Total Rows | Total Bytes |   Expiration    |               Time Partitioning                | Clustered Fields | Labels  |
+---+---+---+---+---+---+---+---+
| 25 Jun 19:28:14 | |- timestamp: timestamp      | 0          | 0           | 25 Jul 19:28:14 | DAY (field: timestamp, expirationMs: 86400000) | customer_id      | org:dev |
|                 | |- customer_id: string       |            |             |                 |                                                |                  |         |
|                 | |- transaction_amount: float |            |             |                 |                                                |                  |         |
+---+---+---+---+---+---+---+---+
```

### API

Call the [`datasets.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get)
method or the [`tables.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get)
method. The response includes all labels associated with that resource.

Alternatively, you can use [`datasets.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list)
to view the labels for multiple datasets or [`tables.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list)
to view the labels for multiple tables and views.

Because views are treated like table resources, you use the `tables.get`
and `tables.list` methods to view label information for both views and
tables.

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
    )

    // printDatasetLabels retrieves label metadata from a dataset and prints it to an io.Writer.
    func printDatasetLabels(w io.Writer, projectID, datasetID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	meta, err := client.Dataset(datasetID).Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	fmt.Fprintf(w, "Dataset %s labels:\n", datasetID)
    	if len(meta.Labels) == 0 {
    		fmt.Fprintln(w, "Dataset has no labels defined.")
    		return nil
    	}
    	for k, v := range meta.Labels {
    		fmt.Fprintf(w, "\t%s:%s\n", k, v)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html;

    // Sample to get dataset labels
    public class GetDatasetLabels {

      public static void runGetDatasetLabels() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        getDatasetLabels(datasetName);
      }

      public static void getDatasetLabels(String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html dataset = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getDataset_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetOption____(datasetName);
          dataset
              .getLabels()
              .forEach((key, value) -> System.out.println("Retrieved labels successfully"));
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Label was not found. \n" + e.toString());
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

    async function getDatasetLabels() {
      // Gets labels on a dataset.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";

      // Retrieve current dataset metadata.
      const dataset = bigquery.dataset(datasetId);
      const [metadata] = await dataset.getMetadata();
      const labels = metadata.labels;

      console.log(`${datasetId} Labels:`);
      for (const [key, value] of Object.entries(labels)) {
        console.log(`${key}: ${value}`);
      }
    }
    getDatasetLabels();

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

    # TODO(developer): Set dataset_id to the ID of the dataset to fetch.
    # dataset_id = "your-project.your_dataset"

    dataset = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_dataset(dataset_id)  # Make an API request.

    # View dataset labels.
    print("Dataset ID: {}".format(dataset_id))
    print("Labels:")
    if dataset.labels:
        for label, value in dataset.labels.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Row.html#google_cloud_bigquery_table_Row_items():
            print("\t{}: {}".format(label, value))
    else:
        print("\tDataset has no labels defined.")

## View table labels

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
    )

    // tableLabels demonstrates fetching metadata from a table and printing the Label metadata to an io.Writer.
    func tableLabels(w io.Writer, projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	meta, err := client.Dataset(datasetID).Table(tableID).Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	fmt.Fprintf(w, "Table %s labels:\n", datasetID)
    	if len(meta.Labels) == 0 {
    		fmt.Fprintln(w, "Table has no labels defined.")
    		return nil
    	}
    	for k, v := range meta.Labels {
    		fmt.Fprintf(w, "\t%s:%s\n", k, v)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to get table labels
    public class GetTableLabels {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        getTableLabels(datasetName, tableName);
      }

      public static void getTableLabels(String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // This example table starts with existing label { color: 'green' }
          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(TableId.of(datasetName, tableName));
          table
              .getLabels()
              .forEach((key, value) -> System.out.println("Retrieved labels successfully"));
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Label was not deleted. \n" + e.toString());
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

    async function getTableLabels() {
      // Gets labels on a dataset.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";
      // const tableId = "my_table";

      // Retrieve current dataset metadata.
      const table = bigquery.dataset(datasetId).table(tableId);
      const [metadata] = await table.getMetadata();
      const labels = metadata.labels;

      console.log(`${tableId} Labels:`);
      for (const [key, value] of Object.entries(labels)) {
        console.log(`${key}: ${value}`);
      }
    }
    getTableLabels();

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

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(dev): Change table_id to the full name of the table you want to create.
    table_id = "your-project.your_dataset.your_table_name"

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # API Request

    # View table labels
    print(f"Table ID: {table_id}.")
    if table.labels:
        for label, value in table.labels.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Row.html#google_cloud_bigquery_table_Row_items():
            print(f"\t{label}: {https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ConnectionProperty.html#google_cloud_bigquery_query_ConnectionProperty_value}")
    else:
        print("\tTable has no labels defined.")

## View job labels

To see the labels on a job, select one of the following options:

### SQL

Query the
[`INFORMATION_SCHEMA.JOB_BY_*` views](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs)
to see the labels on a job. For example, the following SQL query returns the
query text and labels on the jobs submitted by the current user in the
current project:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     query,
     labels
   FROM
     INFORMATION_SCHEMA.JOBS_BY_USER;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To see the labels for a query job using the bq command-line tool, enter
the `bq show -j` command with the query job's job ID. The `--format` flag
can be used to control the output. For example, if your query job has job ID
`bqjob_r1234d57f78901_000023746d4q12_1`, enter the following command:

    bq show -j --format=pretty bqjob_r1234d57f78901_000023746d4q12_1

The output should look like the following:

```
+---+---+---+---+---+---+---+---+
| Job Type |  State  |   Start Time    | Duration |    User Email     | Bytes Processed | Bytes Billed |        Labels        |
+---+---+---+---+---+---+---+---+
| query    | SUCCESS | 03 Dec 15:00:41 | 0:00:00  | email@example.com | 255             | 10485760     | department:shipping  |
|          |         |                 |          |                   |                 |              | costcenter:logistics |
+---+---+---+---+---+---+---+---+
```

### API

Call the [`jobs.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get)
method. The response includes all labels associated with that resource.

## View reservation labels

To see the labels on a reservation, select one of the following options:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot Reservations** tab.

4. The labels for each reservation are listed in the **Labels** column.

### SQL

Query the [`INFORMATION_SCHEMA.RESERVATIONS`
views](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations) to see the labels on
a reservation. For example, the following SQL query returns the reservation
name and labels:


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     reservation_name,
     labels
   FROM
     INFORMATION_SCHEMA.RESERVATIONS
   WHERE reservation_name = RESERVATION_NAME;
   ```


   Replace the following:
   - `RESERVATION_NAME`: the name of the reservation.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq show`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show)
command to view the reservation labels.

```bash
bq show --format=prettyjson --reservation=true --location=LOCATION RESERVATION_NAME
```

Replace the following:

- `LOCATION`: the location of the reservation.
- `RESERVATION_NAME`: the name of the reservation.

The output looks similar to the following:

```
{
  "autoscale": {
    "maxSlots": "100"
  },
  "creationTime": "2023-10-26T15:16:28.196940Z",
  "edition": "ENTERPRISE",
  "labels": {
    "department": "shipping"
  },
  "name": "projects/myproject/locations/US/reservations/myreservation",
  "updateTime": "2025-06-05T19:37:28.125914Z"
}
```

## What's next

- Learn how to [add labels](https://docs.cloud.google.com/bigquery/docs/adding-labels) to BigQuery resources.
- Learn how to [update labels](https://docs.cloud.google.com/bigquery/docs/updating-labels) on BigQuery resources.
- Learn how to [filter resources using labels](https://docs.cloud.google.com/bigquery/docs/filtering-labels).
- Learn how to [delete labels](https://docs.cloud.google.com/bigquery/docs/deleting-labels) on BigQuery resources.
- Read about [using labels](https://docs.cloud.google.com/resource-manager/docs/using-labels) in the Resource Manager documentation.