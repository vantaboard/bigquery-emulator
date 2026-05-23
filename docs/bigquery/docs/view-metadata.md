# Get information about views

This document describes how to list, get information about, and see metadata for
views in BigQuery.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document.

## List views

Listing views is identical to the process for listing tables.

### Required permissions

To list views in a dataset, you need the `bigquery.tables.list`
IAM permission.

Each of the following predefined IAM roles includes the
permissions that you need in order to list views in a dataset:

- `roles/bigquery.user`
- `roles/bigquery.metadataViewer`
- `roles/bigquery.dataViewer`
- `roles/bigquery.dataOwner`
- `roles/bigquery.dataEditor`
- `roles/bigquery.admin`

For more information on IAM roles and permissions in
BigQuery, see
[Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### List views in a dataset

To list the views in a dataset:

### Console

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

3. Click **Overview \> Tables** . Scroll through the list to see
   the view in the dataset. Tables and views are identified by the values
   in the **Type** column.

### SQL

Use the
[`INFORMATION_SCHEMA.VIEWS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-views):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT table_name
   FROM DATASET_ID.INFORMATION_SCHEMA.VIEWS;
   ```


   Replace `DATASET_ID` with the name of the dataset.
3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Issue the `bq ls` command. The `--format` flag can be used to control the
output. If you are listing views in a project other than your default
project, add the project ID to the dataset in the following format:
`project_id:dataset`.

```bash
bq ls --format=pretty project_id:dataset
```

Where:

- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the name of the dataset.

When you run the command, the `Type` field displays either `TABLE` or
`VIEW`. For example:

```
+---+---+---+---+
|         tableId         | Type  |        Labels        | Time Partitioning |
+---+---+---+---+
| mytable                 | TABLE | department:shipping  |                   |
| myview                  | VIEW  |                      |                   |
+---+---+---+---+
```

Examples:

Enter the following command to list views in dataset `mydataset` in your
default project.

    bq ls --format=pretty mydataset

Enter the following command to list views in dataset `mydataset` in
`myotherproject`.

    bq ls --format=pretty myotherproject:mydataset

### API

To list views using the API, call the [`tables.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list)
method.

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

    // listTables demonstrates iterating through the collection of tables in a given dataset.
    func listTables(w io.Writer, projectID, datasetID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	ts := client.Dataset(datasetID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Dataset_Tables(ctx)
    	for {
    		t, err := ts.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		fmt.Fprintf(w, "Table: %q\n", t.TableID)
    	}
    	return nil
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

    # TODO(developer): Set dataset_id to the ID of the dataset that contains
    #                  the tables you are listing.
    # dataset_id = 'your-project.your_dataset'

    tables = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_tables(dataset_id)  # Make an API request.

    print("Tables contained in '{}':".format(dataset_id))
    for table in tables:
        print("{}.{}.{}".format(table.project, table.dataset_id, table.table_id))

## Get information about views

Getting information about views is identical to the process for getting
information about tables.

### Required permissions

To get information about a view, you need the `bigquery.tables.get` IAM permission.

Each of the following predefined IAM roles includes the permissions that you need in order to get information about a view:

- `roles/bigquery.metadataViewer`
- `roles/bigquery.dataViewer`
- `roles/bigquery.dataOwner`
- `roles/bigquery.dataEditor`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can get information about views in the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

To get information about views:

### Console

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

3. Click **Overview \> Tables** . Scroll through the list to see
   the view in the dataset. Tables and views are identified by the values
   in the **Type** column.

4. Click the **Details** tab that displays the view's
   description, view information, and the SQL query that defines the view.

### SQL

Query the
[`INFORMATION_SCHEMA.VIEWS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-views).
The following example retrieves all columns except for `check_option`,
which is reserved for future use. The metadata returned is for all views in
<var translate="no">DATASET_ID</var> in your default project:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
     SELECT
       * EXCEPT (check_option)
     FROM
       DATASET_ID.INFORMATION_SCHEMA.VIEWS;
     
   ```


   Replace `DATASET_ID` with the name of the dataset.
3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Issue the `bq show` command. The `--format` flag can be used to control the
output. If you are getting information about a view in a project other than
your default project, add the project ID to the dataset in the following
format: `[PROJECT_ID]:[DATASET]`.

```bash
bq show \
--format=prettyjson \
project_id:dataset.view
```

Where:

- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the name of the dataset.
- <var translate="no">view</var> is the name of the view.

Examples:

Enter the following command to display information about `myview` in
dataset `mydataset` in your default project.

    bq show --format=prettyjson mydataset.myview

Enter the following command to display information about `myview` in
dataset `mydataset` in `myotherproject`.

    bq show --format=prettyjson myotherproject:mydataset.myview

### API

Call the [`tables.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get)
method and provide any relevant parameters.

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

    // getView demonstrates fetching the metadata from a BigQuery logical view and printing it to an io.Writer.
    func getView(w io.Writer, projectID, datasetID, viewID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// viewID := "myview"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	view := client.Dataset(datasetID).Table(viewID)
    	meta, err := view.Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	fmt.Fprintf(w, "View %s, query: %s\n", view.FullyQualifiedName(), meta.ViewQuery)
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

    // Sample to get a view
    public class GetView {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String viewName = "MY_VIEW_NAME";
        getView(datasetName, viewName);
      }

      public static void getView(String datasetName, String viewName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, viewName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html view = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(tableId);
          System.out.println("View retrieved successfully" + view.getDescription());
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("View not retrieved. \n" + e.toString());
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

    async function getView() {
      // Retrieves view properties.

      /**
       * TODO(developer): Uncomment the following lines before running the sample
       */
      // const datasetId = "my_dataset";
      // const tableId = "my_view";

      // Retrieve view
      const dataset = bigquery.dataset(datasetId);
      const [view] = await dataset.table(tableId).get();

      const fullTableId = view.metadata.id;
      const viewQuery = view.metadata.view.query;

      // Display view properties
      console.log(`View at ${fullTableId}`);
      console.log(`View query: ${viewQuery}`);
    }
    getView();

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

    view_id = "my-project.my_dataset.my_view"
    # Make an API request to get the table resource.
    view = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(view_id)

    # Display view properties
    print(f"Retrieved {https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry.html#google_cloud_bigquery_dataset_AccessEntry_view.table_type}: {str(https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry.html#google_cloud_bigquery_dataset_AccessEntry_view.reference)}")
    print(f"View Query:\n{https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry.html#google_cloud_bigquery_dataset_AccessEntry_view.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_view_query}")

## View security

To control access to views in BigQuery, see
[Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).

## What's next

- For information on creating views, see [Creating views](https://docs.cloud.google.com/bigquery/docs/views).
- For information on creating an authorized view, see [Creating authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).
- For more information on managing views, see [Managing views](https://docs.cloud.google.com/bigquery/docs/managing-views).
- To see an overview of `INFORMATION_SCHEMA`, go to [Introduction to BigQuery `INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro).