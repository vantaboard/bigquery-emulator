# Create logical views

This document describes how to create logical views in BigQuery.

You can create a logical view in the following ways:

- Using the Google Cloud console.
- Using the bq command-line tool's `bq mk` command.
- Calling the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/insert) API method.
- Using the client libraries.
- Submitting a [`CREATE VIEW`](https://docs.cloud.google.com/bigquery/docs/data-definition-language#create_view_statement) data definition language (DDL) statement.

## View limitations

BigQuery views are subject to the following limitations:

- Views are read-only. For example, you can't run queries that insert, update, or delete data.
- If your view references tables from remote [locations](https://docs.cloud.google.com/bigquery/docs/locations), you must enable [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries) before you create the view.
- A reference inside of a view must be qualified with a dataset. The default dataset doesn't affect a view body.
- You cannot use the `TableDataList` JSON API method to retrieve data from a view. For more information, see [Tabledata: list](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list).
- You cannot mix GoogleSQL and legacy SQL queries when using views. A GoogleSQL query cannot reference a view defined using legacy SQL syntax.
- You cannot reference [query parameters](https://docs.cloud.google.com/bigquery/docs/parameterized-queries) in views.
- The schemas of the underlying tables are stored with the view when the view is created. If columns are added, deleted, or modified after the view is created, the view isn't automatically updated and the reported schema will remain inaccurate until the view SQL definition is changed or the view is recreated. Even though the reported schema may be inaccurate, all submitted queries produce accurate results.
- You cannot automatically update a legacy SQL view to GoogleSQL syntax. To modify the query used to define a view, you can use the following:
  - The [**Edit query**](https://docs.cloud.google.com/bigquery/docs/updating-views#update-sql) option in the Google Cloud console
  - The [`bq update --view`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update) command in the bq command-line tool
  - The [BigQuery Client libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries)
  - The [update](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/update) or [patch](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) API methods.
- You cannot include a temporary user-defined function or a temporary table in the SQL query that defines a view.
- You cannot reference a view in a [wildcard table](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables) query.
- Logical views cannot inherit or explicitly define [parameterized data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_data_types), such as `STRING(n)`, as parameterized data types are only supported for base table columns and script variables.

For information about quotas and limits that apply to views, see [View limits](https://docs.cloud.google.com/bigquery/quotas#view_limits).

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document.

### Required permissions

Views are treated as table resources in BigQuery, so creating a
view requires the same permissions as creating a table. You must also have
permissions to query any tables that are referenced by the view's SQL query.

- To create a dataset, you need `bigquery.datasets.create` IAM permission on the project.
- To create a view, you need the `bigquery.tables.create` IAM permission on the dataset. The `roles/bigquery.dataEditor` predefined IAM role includes the permissions that you need to create a view.
- To create a view that queries a table you don't have access to, you must be granted the `bigquery.tables.getData` permission on the table queried by the view.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and
permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

> [!NOTE]
> **Note:** To create or update an [authorized view](https://docs.cloud.google.com/bigquery/docs/authorized-views) or a view in an [authorized dataset](https://docs.cloud.google.com/bigquery/docs/authorized-datasets#create_or_update_view), you need additional permissions. For more information, see [required permissions for authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views#required_permissions) and [required permissions for views in authorized datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets#permissions_datasets).

## View naming

When you create a view in BigQuery, the view name must
be unique per dataset. The view name can:

- Contain characters with a total of up to 1,024 UTF-8 bytes.
- Contain Unicode characters in category L (letter), M (mark), N (number), Pc (connector, including underscore), Pd (dash), Zs (space). For more information, see [General Category](https://wikipedia.org/wiki/Unicode_character_property#General_Category).

The following are all examples of valid view names:
`view 01`, `ग्राहक`, `00_お客様`, `étudiant-01`.

Caveats:

- Table names are case-sensitive by default. `mytable` and `MyTable` can coexist in the same dataset, unless they are part of a [dataset with
  case-sensitivity turned off](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#creating_a_case-insensitive_dataset).
- Some view names and view name prefixes are reserved. If you receive an error saying that your view name or prefix is reserved, then select a different name and try again.
- If you include multiple dot operators (`.`) in a sequence, the duplicate
  operators are implicitly stripped.

  For example, this:
  `project_name....dataset_name..table_name`

  Becomes this:
  `project_name.dataset_name.table_name`

## Create a view

You can create a view by composing a SQL query that is used to define the data
accessible to the view. The SQL query must consist of a `SELECT` statement.
Other statement types (such as DML statements) and
[multi-statement queries](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries) aren't allowed
in view queries, with the exception of the `@@session_id`
[system variable](https://docs.cloud.google.com/bigquery/docs/reference/system-variables).

To create a view:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **SQL query**.

3. In the query editor, enter a valid SQL query.

   Alternatively, you can [open a saved query](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#open_a_saved_query_version_as_a_new_query).
4. Click ![](https://docs.cloud.google.com/static/bigquery/images/save-bigquery-console.png)
   **Save \> Save view**.

   ![Save view.](https://docs.cloud.google.com/static/bigquery/images/save-view-button.png "Save view button in Cloud console")
5. In the **Save view** dialog:

   - In the **Project** menu, select a project to store the view.
   - In the **Dataset** menu, select a dataset or create a new dataset to store the view. The destination dataset for a saved view must be in the same [region](https://docs.cloud.google.com/bigquery/docs/dataset-locations) as the source.
   - In the **Table** field, enter the name of the view.
   - Click **Save**.

> [!NOTE]
> **Note:** When you create a view using Google Cloud console, you cannot add a label, description, or expiration time. You can add these optional properties when you create a view using the API or bq command-line tool. After you create a view using the Google Cloud console, you can add an expiration, description, and labels. For more information, see [Updating views](https://docs.cloud.google.com/bigquery/docs/updating-views).

### SQL

Use the
[`CREATE VIEW` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement).
The following
example creates a view named `usa_male_names` from the USA names
public dataset:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE VIEW mydataset.usa_male_names(name, number) AS (
     SELECT
       name,
       number
     FROM
       `bigquery-public-data.usa_names.usa_1910_current`
     WHERE
       gender = 'M'
     ORDER BY
       number DESC
   );
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk)
with the `--view` flag. For GoogleSQL queries,
add the `--use_legacy_sql` flag and set it to `false`. Some optional
parameters include `--add_tags`, `--expiration`, `--description`, and
`--label`. For a full list of parameters, see the
[`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk)
reference.

If your query references external user-defined function (UDF) resources
stored in Cloud Storage or in local files, use the
`--view_udf_resource` flag to specify those resources. The
`--view_udf_resource` flag is not demonstrated here. For more information about
using UDFs, see
[UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions).

If you are creating a view in a project other than your default project,
specify the project ID using the `--project_id` flag.

> [!NOTE]
> **Note:** The dataset that contains your view and the dataset that contains the tables referenced by the view must be in the same [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

```bash
bq mk \
--use_legacy_sql=false \
--view_udf_resource=PATH_TO_FILE \
--expiration=INTEGER \
--description="DESCRIPTION" \
--label=KEY_1:VALUE_1 \
--add_tags=KEY_2:VALUE_2[,...] \
--view='QUERY' \
--project_id=PROJECT_ID \
DATASET.VIEW
```

Replace the following:

- `PATH_TO_FILE` is the URI or local file system path to a code file to be loaded and evaluated immediately as a UDF resource used by the view. Repeat the flag to specify multiple files.
- `INTEGER` sets the lifetime (in seconds) for the view. If `INTEGER` is `0`, the view doesn't expire. If you don't include the `--expiration` flag, BigQuery creates the view with the dataset's default table lifetime.
- `DESCRIPTION` is a description of the view in quotes.
- `KEY_1:VALUE_1` is the key-value pair that represents a [label](https://docs.cloud.google.com/bigquery/docs/labels). Repeat the `--label` flag to specify multiple labels.
- `KEY_2:VALUE_2` is the key-value pair that represents a [tag](https://docs.cloud.google.com/bigquery/docs/labels). Add multiple tags under the same flag with commas between key:value pairs.
- `QUERY` is a valid query.
- `PROJECT_ID` is your project ID (if you don't have a default project configured).
- `DATASET` is a dataset in your project.
- `VIEW` is the name of the view that you want to create.

Examples:

Enter the following command to create a view named `myview` in
`mydataset` in your default project. The expiration time is set to
3600 seconds (1 hour), the description is set to `This is my view`, and the
label is set to `organization:development`. The query used to create the view
queries data from the [USA Name Data public dataset](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=usa_names&page=dataset).

    bq mk \
    --use_legacy_sql=false \
    --expiration 3600 \
    --description "This is my view" \
    --label organization:development \
    --view \
    'SELECT
      name,
      number
    FROM
      `bigquery-public-data.usa_names.usa_1910_current`
    WHERE
      gender = "M"
    ORDER BY
      number DESC' \
    mydataset.myview

Enter the following command to create a view named `myview` in
`mydataset` in `myotherproject`. The description is set to
`This is my view`, the label is set to `organization:development`,
and the view's expiration is set to the dataset's default table
expiration.
The query used to create the view
queries data from the [USA Name Data public dataset](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=usa_names&page=dataset).

    bq mk \
    --use_legacy_sql=false \
    --description "This is my view" \
    --label organization:development \
    --project_id myotherproject \
    --view \
    'SELECT
      name,
      number
    FROM
      `bigquery-public-data.usa_names.usa_1910_current`
    WHERE
      gender = "M"
    ORDER BY
      number DESC' \
    mydataset.myview

After the view is created, you can update the view's
expiration, description, and labels. For more information, see
[Updating views](https://docs.cloud.google.com/bigquery/docs/updating-views).

### Terraform

Use the
[`google_bigquery_table`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the [Cloud Resource Manager API](https://docs.cloud.google.com/resource-manager/reference/rest).

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a view named `myview`:

    resource "google_bigquery_dataset" "default" {
      dataset_id                      = "mydataset"
      default_partition_expiration_ms = 2592000000  # 30 days
      default_table_expiration_ms     = 31536000000 # 365 days
      description                     = "dataset description"
      location                        = "US"
      max_time_travel_hours           = 96 # 4 days

      labels = {
        billing_group = "accounting",
        pii           = "sensitive"
      }
    }

    resource "google_bigquery_table" "default" {
      dataset_id = google_bigquery_dataset.default.dataset_id
      table_id   = "myview"

      view {
        query          = "SELECT global_id, faa_identifier, name, latitude, longitude FROM `bigquery-public-data.faa.us_airports`"
        use_legacy_sql = false
      }

    }

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

### API

Call the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/insert) method
with a [table resource](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables) that
contains a `view` property.

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

    // createView demonstrates creation of a BigQuery logical view.
    func createView(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydatasetid"
    	// tableID := "mytableid"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	meta := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		// This example shows how to create a view of the shakespeare sample dataset, which
    		// provides word frequency information.  This view restricts the results to only contain
    		// results for works that contain the "king" in the title, e.g. King Lear, King Henry V, etc.
    		ViewQuery: "SELECT word, word_count, corpus, corpus_date FROM `bigquery-public-data.samples.shakespeare` WHERE corpus LIKE '%king%'",
    	}
    	if err := client.Dataset(datasetID).Table(tableID).Create(ctx, meta); err != nil {
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ViewDefinition.html;

    // Sample to create a view
    public class CreateView {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String viewName = "MY_VIEW_NAME";
        String query =
            String.format(
                "SELECT TimestampField, StringField, BooleanField FROM %s.%s", datasetName, tableName);
        createView(datasetName, viewName, query);
      }

      public static void createView(String datasetName, String viewName, String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, viewName);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ViewDefinition.html viewDefinition =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ViewDefinition.html.newBuilder(query).setUseLegacySql(false).build();

          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(TableInfo.of(tableId, viewDefinition));
          System.out.println("View created successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("View was not created. \n" + e.toString());
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

    // Import the Google Cloud client library and create a client
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function createView() {
      // Creates a new view named "my_shared_view" in "my_dataset".

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const myDatasetId = "my_table"
      // const myTableId = "my_table"
      // const projectId = "bigquery-public-data";
      // const sourceDatasetId = "usa_names"
      // const sourceTableId = "usa_1910_current";
      const myDataset = await bigquery.dataset(myDatasetId);

      // For all options, see https://cloud.google.com/bigquery/docs/reference/v2/tables#resource
      const options = {
        view: `SELECT name 
        FROM \`${projectId}.${sourceDatasetId}.${sourceTableId}\`
        LIMIT 10`,
      };

      // Create a new view in the dataset
      const [view] = await myDataset.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/dataset.html(myTableId, options);

      console.log(`View ${view.id} created.`);
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

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    view_id = "my-project.my_dataset.my_view"
    source_id = "my-project.my_dataset.my_table"
    view = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html(view_id)

    # The source table in this example is created from a CSV file in Google
    # Cloud Storage located at
    # `gs://cloud-samples-data/bigquery/us-states/us-states.csv`. It contains
    # 50 US states, while the view returns only those states with names
    # starting with the letter 'W'.
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry.html#google_cloud_bigquery_dataset_AccessEntry_view.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_view_query = f"SELECT name, post_abbr FROM `{source_id}` WHERE name LIKE 'W%'"

    # Make an API request to create the view.
    view = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_create_table(view)
    print(f"Created {https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry.html#google_cloud_bigquery_dataset_AccessEntry_view.table_type}: {str(https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry.html#google_cloud_bigquery_dataset_AccessEntry_view.reference)}")

After you create the view, you [query](https://docs.cloud.google.com/bigquery/docs/running-queries) it like
you query a table.

## View security

To control access to views in BigQuery, see
[Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).

## What's next

- For information about creating an authorized view, see [Creating authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).
- For information about getting view metadata, see [Getting information about views](https://docs.cloud.google.com/bigquery/docs/view-metadata).
- For more information about managing views, see [Managing views](https://docs.cloud.google.com/bigquery/docs/managing-views).