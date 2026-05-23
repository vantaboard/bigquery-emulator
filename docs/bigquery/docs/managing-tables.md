# Manage tables

This document describes how to manage tables in BigQuery.
You can manage your BigQuery tables in the following ways:

- [Update table properties](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_table_properties):
  - [Expiration time](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_expiration_time)
  - [Description](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_description)
  - [Schema definition](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_schema_definition)
  - [Labels](https://docs.cloud.google.com/bigquery/docs/adding-labels#adding_table_and_view_labels)
  - [Default rounding mode](https://docs.cloud.google.com/bigquery/docs/managing-tables#update_rounding_mode)
- [Rename (copy) a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#renaming-table)
- [Copy a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table)
- [Delete a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#deleting_tables)

For information about how to restore (or *undelete* ) a deleted table, see
[Restore deleted tables](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables).

For more information about creating and using tables including getting table
information, listing tables, and controlling access to table data, see
[Creating and using tables](https://docs.cloud.google.com/bigquery/docs/tables).

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document. The permissions required to perform a
task (if any) are listed in the "Required permissions" section of the task.

## Update table properties

You can update the following elements of a table:

- [Description](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_description)
- [Expiration time](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_expiration_time)
- [Schema definition](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas)
- [Labels](https://docs.cloud.google.com/bigquery/docs/labels#creating_or_updating_a_table_or_view_label)
- [Table name](https://docs.cloud.google.com/bigquery/docs/managing-tables#renaming-table)
- [Tags](https://docs.cloud.google.com/bigquery/docs/tags)

### Required permissions


To get the permissions that
you need to update table properties,

ask your administrator to grant you the
[Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) IAM role on a table.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to update table properties. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to update table properties:

- `bigquery.tables.update`
- `bigquery.tables.get`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

Additionally, if you have the `bigquery.datasets.create` permission, you can
update the properties of the tables of the datasets that you create.

### Update a table's description

You can update a table's description in the following ways:

- Using the Google Cloud console.
- Using a data definition language (DDL) `ALTER TABLE` statement.
- Using the bq command-line tool's `bq update` command.
- Calling the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) API method.
- Using the client libraries.
- Generating a description with Gemini in BigQuery.

To update a table's description:

### Console

You can't add a description when you create a table using the
Google Cloud console. After the table is created, you can add a
description on the **Details** page.

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

3. Click **Overview \> Tables**, and then select a table.

4. Click the **Details** tab, and then click **Edit details**.

5. In the **Description** section, add a new description or edit an existing
   description.

6. Click **Save**.

### SQL

Use the
[`ALTER TABLE SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement).
The following example updates the
description of a table named `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
     SET OPTIONS (
       description = 'Description of mytable');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Issue the `bq update` command with the `--description` flag. If you are
   updating a table in a project other than your default project, add the
   project ID to the dataset name in the following format:
   `project_id:dataset`.

   ```bash
   bq update \
   --description "description" \
   project_id:dataset.table
   ```

   Replace the following:
   - `description`: the text describing the table in quotes
   - `project_id`: your project ID
   - `dataset`: the name of the dataset that contains the table you're updating
   - `table`: the name of the table you're updating

   Examples:

   To change the description of the `mytable` table in the `mydataset` dataset to
   "Description of mytable", enter the following command. The `mydataset` dataset is in
   your default project.

   ```sh
   bq update --description "Description of mytable" mydataset.mytable
   ```

   To change the description of the `mytable` table in the `mydataset` dataset to
   "Description of mytable", enter the following command. The `mydataset` dataset is in the
   `myotherproject` project, not your default project.

   ```sh
   bq update \
   --description "Description of mytable" \
   myotherproject:mydataset.mytable
   ```

### API

Call the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and use the `description` property in the [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables)
to update the table's description. Because the `tables.update` method
replaces the entire table resource, the `tables.patch` method is preferred.

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

    // updateTableDescription demonstrates how to fetch a table's metadata and updates the Description metadata.
    func updateTableDescription(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	tableRef := client.Dataset(datasetID).Table(tableID)
    	meta, err := tableRef.Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	update := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadataToUpdate{
    		Description: "Updated description.",
    	}
    	if _, err = tableRef.Update(ctx, update, meta.ETag); err != nil {
    		return err
    	}
    	return nil
    }

<br />

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

    public class UpdateTableDescription {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String newDescription = "this is the new table description";
        updateTableDescription(datasetName, tableName, newDescription);
      }

      public static void updateTableDescription(
          String datasetName, String tableName, String newDescription) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(datasetName, tableName);
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_update_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(table.toBuilder().setDescription(newDescription).build());
          System.out.println("Table description updated successfully to " + newDescription);
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table description was not updated \n" + e.toString());
        }
      }
    }

<br />

### Python

<br />


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Configure the [Table.description](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_description) property and call [Client.update_table()](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_update_table) to send the update to the API.

    # from google.cloud import bigquery
    # client = bigquery.Client()
    # project = client.project
    # dataset_ref = bigquery.DatasetReference(project, dataset_id)
    # table_ref = dataset_ref.table('my_table')
    # table = client.get_table(table_ref)  # API request

    assert table.description == "Original description."
    table.description = "Updated description."

    table = client.update_table(table, ["description"])  # API request

    assert table.description == "Updated description."

<br />

### Gemini

You can generate a table description with Gemini in
BigQuery by using data insights. Data insights is an automated
way to explore, understand, and curate your data.

For more information about data insights, including setup steps, required
IAM roles, and best practices to improve the accuracy of the
generated insights, see
[Generate data insights in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-insights).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and dataset, then select
   the table.

4. In the details panel, click the **Schema** tab.

5. Click **Generate**.

   > [!NOTE]
   > **Note:** If you don't see the **Generate** button, click **Describe data**. You might need to scroll to see this button.

   Gemini generates a table description and insights about
   the table. It takes a few minutes for the information to be
   populated. You can view the generated insights on the table's
   **Insights** tab.
6. To edit and save the generated table description, do the following:

   1. Click **View column descriptions**.

      The current table description and the generated description are
      displayed.
   2. In the **Table description** section, click **Save to details**.

   3. To replace the current description with the generated description,
      click **Copy suggested description**.

   4. Edit the table description as necessary, and then click
      **Save to details**.

      The table description is updated immediately.
   5. To close the **Preview descriptions** panel, click
      **Close**.

### Update a table's expiration time

You can set a default table expiration time at the dataset level, or you can set
a table's expiration time when the table is created. A table's expiration time
is often referred to as "time to live" or TTL.

When a table expires, it is deleted along with all of the data it contains.
If necessary, you can undelete the expired table within the time travel window
specified for the dataset, see
[Restore deleted tables](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables) for more
information.

If you set the expiration when the table is created, the dataset's default table
expiration is ignored. If you do not set a default table expiration at the
dataset level, and you do not set a table expiration when the table is created,
the table never expires and you must [delete](https://docs.cloud.google.com/bigquery/docs/managing-tables#deleting_a_table) the table
manually.

At any point after the table is created, you can update the table's expiration
time in the following ways:

- Using the Google Cloud console.
- Using a data definition language (DDL) `ALTER TABLE` statement.
- Using the bq command-line tool's `bq update` command.
- Calling the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) API method.
- Using the client libraries.

> [!NOTE]
> **Note:** If you set an expiration time that has already passed, the table is deleted immediately.

To update a table's expiration time:

### Console

You can't add an expiration time when you create a table using the
Google Cloud console. After a table is created, you can add or update a
table expiration on the **Table Details** page.

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

3. Click **Overview \> Tables**, and then select a table.

4. Click the **Details** tab and the click **Edit details**.

5. For **Expiration time** , select **Specify date**. Then select the
   expiration date using the calendar widget.

6. Click **Save** . The updated expiration time appears in the
   **Table info** section.

### SQL

Use the
[`ALTER TABLE SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement).
The following example updates the
expiration time of a table named `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
     SET OPTIONS (
       -- Sets table expiration to timestamp 2025-02-03 12:34:56
       expiration_timestamp = TIMESTAMP '2025-02-03 12:34:56');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Issue the `bq update` command with the `--expiration` flag. If you are
   updating a table in a project other than your default project,
   add the project ID to the dataset name in the following format:
   `project_id:dataset`.

       bq update \
       --expiration integer \
       project_id:dataset.table

   Replace the following:
   - `integer`: the default lifetime (in seconds) for the table. The minimum value is 3600 seconds (one hour). The expiration time evaluates to the current time plus the integer value. If you specify `0`, the table expiration is removed, and the table never expires. Tables with no expiration must be manually deleted.
   - `project_id`: your project ID.
   - `dataset`: the name of the dataset that contains the table you're updating.
   - `table`: the name of the table you're updating.

   Examples:

   To update the expiration time of the `mytable` table in the `mydataset` dataset to 5 days
   (432000 seconds), enter the following command. The `mydataset` dataset is in your
   default project.

   ```sh
   bq update --expiration 432000 mydataset.mytable
   ```

   To update the expiration time of the `mytable` table in the `mydataset` dataset to 5 days
   (432000 seconds), enter the following command. The `mydataset` dataset is in the
   `myotherproject` project, not your default project.

   ```sh
   bq update --expiration 432000 myotherproject:mydataset.mytable
   ```

### API

Call the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and use the `expirationTime` property in the [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables)
to update the table expiration in milliseconds. Because the `tables.update`
method replaces the entire table resource, the `tables.patch` method is
preferred.

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
    	"time"

    	"cloud.google.com/go/bigquery"
    )

    // updateTableExpiration demonstrates setting the table expiration of a table to a specific point in time
    // in the future, at which time it will be deleted.
    func updateTableExpiration(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	tableRef := client.Dataset(datasetID).Table(tableID)
    	meta, err := tableRef.Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	update := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadataToUpdate{
    		ExpirationTime: time.Now().Add(time.Duration(5*24) * time.Hour), // table expiration in 5 days.
    	}
    	if _, err = tableRef.Update(ctx, update, meta.ETag); err != nil {
    		return err
    	}
    	return nil
    }

<br />

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
    import java.util.concurrent.TimeUnit;

    public class UpdateTableExpiration {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        // Update table expiration to one day.
        Long newExpiration =
            TimeUnit.MILLISECONDS.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.ProtoSchemaConverter.html#com_google_cloud_bigquery_storage_v1_ProtoSchemaConverter_convert_com_google_protobuf_Descriptors_Descriptor_(1, TimeUnit.DAYS) + System.currentTimeMillis();
        updateTableExpiration(datasetName, tableName, newExpiration);
      }

      public static void updateTableExpiration(
          String datasetName, String tableName, Long newExpiration) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(datasetName, tableName);
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_update_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(table.toBuilder().setExpirationTime(newExpiration).build());

          System.out.println("Table expiration updated successfully to " + newExpiration);
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table expiration was not updated \n" + e.toString());
        }
      }
    }

<br />

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

    async function updateTableExpiration() {
      // Updates a table's expiration.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset', // Existing dataset
      // const tableId = 'my_table', // Existing table
      // const expirationTime = Date.now() + 1000 * 60 * 60 * 24 * 5 // 5 days from current time in ms

      // Retreive current table metadata
      const table = bigquery.dataset(datasetId).table(tableId);
      const [metadata] = await table.getMetadata();

      // Set new table expiration to 5 days from current time
      metadata.expirationTime = expirationTime.toString();
      const [apiResponse] = await table.setMetadata(metadata);

      const newExpirationTime = apiResponse.expirationTime;
      console.log(`${tableId} expiration: ${newExpirationTime}`);
    }


### Python

<br />


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Configure [Table.expires](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_expires) property and call [Client.update_table()](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_update_table) to send the update to the API.

    # Copyright 2022 Google LLC
    #
    # Licensed under the Apache License, Version 2.0 (the "License");
    # you may not use this file except in compliance with the License.
    # You may obtain a copy of the License at
    #
    #     https://www.apache.org/licenses/LICENSE-2.0
    #
    # Unless required by applicable law or agreed to in writing, software
    # distributed under the License is distributed on an "AS IS" BASIS,
    # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    # See the License for the specific language governing permissions and
    # limitations under the License.

    import datetime


    def update_table_expiration(table_id, expiration):
        orig_table_id = table_id
        orig_expiration = expiration

        from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

        client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

        # TODO(dev): Change table_id to the full name of the table you want to update.
        table_id = "your-project.your_dataset.your_table_name"

        # TODO(dev): Set table to expire for desired days days from now.
        expiration = datetime.datetime.now(datetime.timezone.utc) + datetime.timedelta(
            days=5
        )

        table_id = orig_table_id
        expiration = orig_expiration

        table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
        table.expires = expiration
        table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_table(table, ["expires"])  # API request

        print(f"Updated {table_id}, expires {table.expires}.")

<br />

To update the default dataset partition expiration time:

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
    import java.util.concurrent.TimeUnit;

    // Sample to update partition expiration on a dataset.
    public class UpdateDatasetPartitionExpiration {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        // Set the default partition expiration (applies to new tables, only) in
        // milliseconds. This example sets the default expiration to 90 days.
        Long newExpiration = TimeUnit.MILLISECONDS.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.ProtoSchemaConverter.html#com_google_cloud_bigquery_storage_v1_ProtoSchemaConverter_convert_com_google_protobuf_Descriptors_Descriptor_(90, TimeUnit.DAYS);
        updateDatasetPartitionExpiration(datasetName, newExpiration);
      }

      public static void updateDatasetPartitionExpiration(String datasetName, Long newExpiration) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html dataset = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getDataset_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetOption____(datasetName);
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_update_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(dataset.toBuilder().setDefaultPartitionExpirationMs(newExpiration).build());
          System.out.println(
              "Dataset default partition expiration updated successfully to " + newExpiration);
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Dataset partition expiration was not updated \n" + e.toString());
        }
      }
    }

<br />

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

    # Copyright 2019 Google LLC
    #
    # Licensed under the Apache License, Version 2.0 (the "License");
    # you may not use this file except in compliance with the License.
    # You may obtain a copy of the License at
    #
    #     https://www.apache.org/licenses/LICENSE-2.0
    #
    # Unless required by applicable law or agreed to in writing, software
    # distributed under the License is distributed on an "AS IS" BASIS,
    # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    # See the License for the specific language governing permissions and
    # limitations under the License.


    def update_dataset_default_partition_expiration(dataset_id: str) -> None:

        from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

        # Construct a BigQuery client object.
        client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

        # TODO(developer): Set dataset_id to the ID of the dataset to fetch.
        # dataset_id = 'your-project.your_dataset'

        dataset = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_dataset(dataset_id)  # Make an API request.

        # Set the default partition expiration (applies to new tables, only) in
        # milliseconds. This example sets the default expiration to 90 days.
        dataset.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.Dataset.html#google_cloud_bigquery_dataset_Dataset_default_partition_expiration_ms = 90 * 24 * 60 * 60 * 1000

        dataset = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_dataset(
            dataset, ["default_partition_expiration_ms"]
        )  # Make an API request.

        print(
            "Updated dataset {}.{} with new default partition expiration {}".format(
                dataset.project, dataset.dataset_id, dataset.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.Dataset.html#google_cloud_bigquery_dataset_Dataset_default_partition_expiration_ms
            )
        )

<br />

### Update a table's rounding mode

You can update a table's
[default rounding mode](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table.FIELDS.default_rounding_mode)
by using the
[`ALTER TABLE SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement).
The following example updates the default rounding mode for `mytable` to
`ROUND_HALF_EVEN`:

```googlesql
ALTER TABLE mydataset.mytable
SET OPTIONS (
  default_rounding_mode = "ROUND_HALF_EVEN");
```

When you add a `NUMERIC` or `BIGNUMERIC` field to a table and do not specify
a [rounding mode](https://docs.cloud.google.com/bigquery/docs/schemas#rounding_mode), then the rounding mode
is automatically set to the table's default rounding mode. Changing a table's
default rounding mode doesn't alter the rounding mode of existing fields.

### Update a table's schema definition

For more information about updating a table's schema definition, see
[Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

### Rename a table

You can rename a table after it has been created by using the
[`ALTER TABLE RENAME TO` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_to_statement).
The following example renames `mytable` to `mynewtable`:

```googlesql
ALTER TABLE mydataset.mytable
RENAME TO mynewtable;
```

The `ALTER TABLE RENAME TO` statement recreates the table in the destination
dataset with the creation timestamp of the original table. If you have
configured [dataset-level table
expiration](https://docs.cloud.google.com/bigquery/docs/updating-datasets#table-expiration), the renamed
table might be immediately deleted if its original creation timestamp falls
outside of the expiration window.

#### Limitations on renaming tables

- If you want to rename a table that has data streaming into it, you must stop the streaming, commit any pending streams, and wait for BigQuery to indicate that streaming is not in use.
- While a table can usually be renamed 5 hours after the last streaming operation, it might take longer.
- Existing table ACLs and row access policies are preserved, but table ACL and row access policy updates made during the table rename are not preserved.
- You can't concurrently rename a table and run a DML statement on that table.
- Renaming a table removes all [Data Catalog tags](https://docs.cloud.google.com/data-catalog/docs/tags-and-tag-templates) (deprecated) and [Knowledge Catalog aspects](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspects) on the table.
- Any search index or vector index created on the table is dropped when the table is renamed.
- You can't rename external tables.

## Copy a table

This section describes
how to create a full copy of a table. For information about other types of table
copies, see [table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro) and
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).

You can copy a table in the following ways:

- Use the Google Cloud console.
- Use the [`bq cp`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cp) command.
- Use a data definition language (DDL) [`CREATE TABLE COPY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_copy) statement.
- Call the [jobs.insert](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API method and configure a `copy` job.
- Use the client libraries.

### Limitations on copying tables

Table copy jobs are subject to the following limitations:

- You can't stop a table copy operation after you start it. A table copy operation runs asynchronously and doesn't stop even when you cancel the job. You are also charged for data transfer for a cross-region table copy and for storage in the destination region.
- When you copy a table, the name of the destination table must adhere to the same naming conventions as when you [create a table](https://docs.cloud.google.com/bigquery/docs/tables#create-table).
- Table copies are subject to BigQuery [limits](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) on copy jobs.
- The Google Cloud console supports copying only one table at a time. You can't overwrite an existing table in the destination dataset. The table must have a unique name in the destination dataset.
- Copying multiple source tables into a destination table is not supported by the Google Cloud console.
- When copying multiple source tables to a destination table using the API,
  bq command-line tool, or the client libraries, all source tables must have identical
  schemas, including any partitioning or clustering.

  Certain table schema updates, such as dropping or renaming
  columns, can cause tables to have apparently identical schemas but different
  internal representations. This might cause a table copy job to fail with the
  error `Maximum limit on diverging physical schemas reached`. In this case, you
  can use the
  [`CREATE TABLE LIKE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_like)
  to ensure that your source table's schema matches the destination table's
  schema exactly.
- The time that BigQuery takes to copy tables might vary
  significantly across different runs because the underlying storage is managed
  dynamically.

- You can't copy and append a source table to a destination table that has more
  columns than the source table, and the additional columns have
  [default values](https://docs.cloud.google.com/bigquery/docs/default-values). Instead, you can run
  `INSERT destination_table SELECT * FROM source_table` to copy over the data.

- If the copy operation overwrites an existing table, then the table-level
  access for the existing table is maintained. [Tags](https://docs.cloud.google.com/bigquery/docs/tags) from
  the source table aren't copied to the overwritten table, while tags on the
  existing table are retained. However, when you copy tables across regions,
  tags on the existing table are removed.

- If the copy operation creates a new table, then the table-level access for the
  new table is determined by the access policies of the dataset in which the new
  table is created. Additionally, [tags](https://docs.cloud.google.com/bigquery/docs/tags) are copied from
  the source table to the new table.

- When you copy multiple source tables to a destination table, all source tables
  must have identical tags.

### Required roles

To perform the tasks in this document, you need the following permissions.

#### Roles to copy tables and partitions


To get the permissions that
you need to copy tables and partitions,

ask your administrator to grant you the
[Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) IAM role on the source and destination datasets.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to copy tables and partitions. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to copy tables and partitions:

- `bigquery.tables.getData` on the source and destination datasets
- `bigquery.tables.get` on the source and destination datasets
- `bigquery.tables.create` on the destination dataset
- `bigquery.tables.update` on the destination dataset


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

#### Permission to run a copy job


To get the permission that
you need to run a copy job,

ask your administrator to grant you the
[Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`) IAM role on the source and destination datasets.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`bigquery.jobs.create`
permission,
which is required to
run a copy job.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Copy a single source table

You can copy a single table in the following ways:

- Using the Google Cloud console.
- Using the bq command-line tool's `bq cp` command.
- Using a data definition language (DDL) `CREATE TABLE COPY` statement.
- Calling the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API method, configuring a `copy` job, and specifying the `sourceTable` property.
- Using the client libraries.

The Google Cloud console and the `CREATE TABLE COPY` statement support only
one source table and one destination
table in a copy job. To [copy multiple source files](https://docs.cloud.google.com/bigquery/docs/managing-tables#copying_multiple_source_tables)
to a destination table, you must use the bq command-line tool or the API.

To copy a single source table:

### Console

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

3. Click **Overview \> Tables**, and then select a table.

4. In the details pane, click **Copy**.

5. In the **Copy table** dialog, under **Destination**:

   - For **Project**, choose the project that will store the copied table.
   - For **Dataset** , select the dataset where you want to store the copied table. The source and destination datasets must be in the same [location](https://docs.cloud.google.com/bigquery/docs/locations).
   - For **Table** , enter a name for the new table. The name must be unique in the destination dataset. You can't overwrite an existing table in the destination dataset using the Google Cloud console. For more information about table name requirements, see [Table naming](https://docs.cloud.google.com/bigquery/docs/tables#table_naming).
6. Click **Copy** to start the copy job.

### SQL

Use the
[`CREATE TABLE COPY` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_copy)
to copy a table named
`table1` to a new table named `table1copy`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       CREATE TABLE myproject.mydataset.table1copy
       COPY myproject.mydataset.table1;

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Issue the `bq cp` command. Optional flags can be used to control the write
   disposition of the destination table:

   - `-a` or `--append_table` appends the data from the source table to an existing table in the destination dataset.
   - `-f` or `--force` overwrites an existing table in the destination dataset and doesn't prompt you for confirmation.
   - `-n` or `--no_clobber` returns the following error message if the table exists in the destination dataset: `Table 'project_id:dataset.table' already exists, skipping.` If `-n` is not specified, the default behavior is to prompt you to choose whether to replace the destination table.
   - `--destination_kms_key` is the customer-managed Cloud KMS key used to encrypt the destination table.

   `--destination_kms_key` is not demonstrated here. See
   [Protecting data with Cloud Key Management Service keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)
   for more information.

   If the source or destination dataset is in a project other than your default
   project, add the project ID to the dataset names in the following format:
   `project_id:dataset`.

   (Optional) Supply the `--location` flag and set the value to your
   [location](https://docs.cloud.google.com/bigquery/docs/locations).

       bq --location=location cp \
       -a -f -n \
       project_id:dataset.source_table \
       project_id:dataset.destination_table

   Replace the following:
   - `location`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [`.bigqueryrc` file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
   - `project_id`: your project ID.
   - `dataset`: the name of the source or destination dataset.
   - `source_table`: the table you're copying.
   - `destination_table`: the name of the table in the destination dataset.

   Examples:

   To copy the `mydataset.mytable` table to the `mydataset2.mytable2` table,
   enter the following command. Both datasets are in your default project.

   ```sh
   bq cp mydataset.mytable mydataset2.mytable2
   ```

   To copy the `mydataset.mytable` table and to overwrite a destination table
   with the same name, enter the following command. The source dataset is in
   your default project. The destination dataset is in the `myotherproject`
   project. The `-f` shortcut is used to overwrite the destination table
   without a prompt.

   ```bash
   bq cp -f \
   mydataset.mytable \
   myotherproject:myotherdataset.mytable
   ```

   To copy the `mydataset.mytable` table and to return an error if the
   destination dataset contains a table with the same name, enter the following
   command. The source dataset is in your default project. The destination
   dataset is in the `myotherproject` project. The `-n` shortcut is used to
   prevent overwriting a table with the same name.

   ```bash
   bq cp -n \
   mydataset.mytable \
   myotherproject:myotherdataset.mytable
   ```

   To copy the `mydataset.mytable` table and to append the data to a
   destination table with the same name, enter the following command. The
   source dataset is in your default project. The destination dataset is in the
   `myotherproject` project. The `- a` shortcut is used to append to the
   destination table.

   ```sh
   bq cp -a mydataset.mytable myotherproject:myotherdataset.mytable
   ```

### API

You can copy an existing table through the API by calling the
[`bigquery.jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method, and configuring a `copy` job. Specify your location in
the `location` property in the `jobReference` section of the
[job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

You must specify the following values in your job configuration:

```
"copy": {
      "sourceTable": {       // Required
        "projectId": string, // Required
        "datasetId": string, // Required
        "tableId": string    // Required
      },
      "destinationTable": {  // Required
        "projectId": string, // Required
        "datasetId": string, // Required
        "tableId": string    // Required
      },
      "createDisposition": string,  // Optional
      "writeDisposition": string,   // Optional
    },
```

Where `sourceTable` provides information about the table to be
copied, `destinationTable` provides information about the new
table, `createDisposition` specifies whether to create the
table if it doesn't exist, and `writeDisposition` specifies
whether to overwrite or append to an existing table.

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


    using Google.Apis.Bigquery.v2.Data;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryCopyTable
    {
        public void CopyTable(
            string projectId = "your-project-id",
            string destinationDatasetId = "your_dataset_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            TableReference sourceTableRef = new TableReference()
            {
                TableId = "shakespeare",
                DatasetId = "samples",
                ProjectId = "bigquery-public-data"
            };
            TableReference destinationTableRef = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetTableReference_System_String_System_String_(
                destinationDatasetId, "destination_table");
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html job = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_CreateCopyJob_Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_CreateCopyJobOptions_(
                sourceTableRef, destinationTableRef)
                .https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_() // Wait for the job to complete.
                .ThrowOnAnyError();

            // Retrieve destination table
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html destinationTable = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetTable_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_GetTableOptions_(destinationTableRef);
            Console.WriteLine(
                $"Copied {destinationTable.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_Resource.NumRows} rows from table "
                + $"{sourceTableRef.DatasetId}.{sourceTableRef.TableId} "
                + $"to {destinationTable.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_FullyQualifiedId}."
            );
        }
    }

<br />

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

    // copyTable demonstrates copying a table from a source to a destination, and
    // allowing the copy to overwrite existing data by using truncation.
    func copyTable(projectID, datasetID, srcID, dstID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// srcID := "sourcetable"
    	// dstID := "destinationtable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	dataset := client.Dataset(datasetID)
    	copier := dataset.Table(dstID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_CopierFrom(dataset.Table(srcID))
    	copier.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty
    	job, err := copier.Run(ctx)
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
    	return nil
    }

<br />

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    public class CopyTable {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String destinationDatasetName = "MY_DESTINATION_DATASET_NAME";
        String destinationTableId = "MY_DESTINATION_TABLE_NAME";
        String sourceDatasetName = "MY_SOURCE_DATASET_NAME";
        String sourceTableId = "MY_SOURCE_TABLE_NAME";

        copyTable(sourceDatasetName, sourceTableId, destinationDatasetName, destinationTableId);
      }

      public static void copyTable(
          String sourceDatasetName,
          String sourceTableId,
          String destinationDatasetName,
          String destinationTableId) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html sourceTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(sourceDatasetName, sourceTableId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html destinationTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(destinationDatasetName, destinationTableId);

          // For more information on CopyJobConfiguration see:
          // https://googleapis.dev/java/google-cloud-clients/latest/com/google/cloud/bigquery/JobConfiguration.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html.newBuilder(destinationTable, sourceTable).build();

          // For more information on Job see:
          // https://googleapis.dev/java/google-cloud-clients/latest/index.html?com/google/cloud/bigquery/package-summary.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));

          // Blocks until this job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (completedJob == null) {
            System.out.println("Job not executed since it no longer exists.");
            return;
          } else if (completedJob.getStatus().getError() != null) {
            System.out.println(
                "BigQuery was unable to copy table due to an error: \n" + job.getStatus().getError());
            return;
          }
          System.out.println("Table copied successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Table copying job was interrupted. \n" + e.toString());
        }
      }
    }

<br />

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

    async function copyTable() {
      // Copies src_dataset:src_table to dest_dataset:dest_table.

      /**
       * TODO(developer): Uncomment the following lines before running the sample
       */
      // const srcDatasetId = "my_src_dataset";
      // const srcTableId = "my_src_table";
      // const destDatasetId = "my_dest_dataset";
      // const destTableId = "my_dest_table";

      // Copy the table contents into another table
      const [job] = await bigquery
        .dataset(srcDatasetId)
        .table(srcTableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(bigquery.dataset(destDatasetId).table(destTableId));

      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
      }
    }

<br />

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
    use Google\Cloud\Core\ExponentialBackoff;

    /** Uncomment and populate these variables in your code */
    // $projectId = 'The Google project ID';
    // $datasetId = 'The BigQuery dataset ID';
    // $sourceTableId   = 'The BigQuery table ID to copy from';
    // $destinationTableId = 'The BigQuery table ID to copy to';

    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $dataset = $bigQuery->dataset($datasetId);
    $sourceTable = $dataset->table($sourceTableId);
    $destinationTable = $dataset->table($destinationTableId);
    $copyConfig = $sourceTable->copy($destinationTable);
    $job = $sourceTable->runJob($copyConfig);

    // poll the job until it is complete
    $backoff = new ExponentialBackoff(10);
    $backoff->execute(function () use ($job) {
        print('Waiting for job to complete' . PHP_EOL);
        $job->reload();
        if (!$job->isComplete()) {
            throw new Exception('Job has not yet completed', 500);
        }
    });
    // check if the job has errors
    if (isset($job->info()['status']['errorResult'])) {
        $error = $job->info()['status']['errorResult']['message'];
        printf('Error running job: %s' . PHP_EOL, $error);
    } else {
        print('Table copied successfully' . PHP_EOL);
    }

<br />

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

    # TODO(developer): Set source_table_id to the ID of the original table.
    # source_table_id = "your-project.source_dataset.source_table"

    # TODO(developer): Set destination_table_id to the ID of the destination table.
    # destination_table_id = "your-project.destination_dataset.destination_table"

    job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_copy_table(source_table_id, destination_table_id)
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()  # Wait for the job to complete.

    print("A copy of the table created.")

<br />

### Copy multiple source tables

You can copy multiple source tables to a destination table in the following
ways:

- Using the bq command-line tool's `bq cp` command.
- Calling the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) method, configuring a `copy` job, and specifying the `sourceTables` property.
- Using the client libraries.

All source tables must have identical schemas and [tags](https://docs.cloud.google.com/bigquery/docs/tags),
and only one destination table is allowed.

Source tables must be specified as a comma-separated list. You can't use
wildcards when you copy multiple source tables.

To copy multiple source tables, select one of the following choices:

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Issue the `bq cp` command and include multiple source tables as a
   comma-separated list. Optional flags can be used to control the write
   disposition of the destination table:

   - `-a` or `--append_table` appends the data from the source tables to an existing table in the destination dataset.
   - `-f` or `--force` overwrites an existing destination table in the destination dataset and doesn't prompt you for confirmation.
   - `-n` or `--no_clobber` returns the following error message if the table exists in the destination dataset: `Table 'project_id:dataset.table'
     already exists, skipping.` If `-n` is not specified, the default behavior is to prompt you to choose whether to replace the destination table.
   - `--destination_kms_key` is the customer-managed Cloud Key Management Service key used to encrypt the destination table.

   `--destination_kms_key` is not demonstrated here. See
   [Protecting data with Cloud Key Management Service keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)
   for more information.

   If the source or destination dataset is in a project other than your default
   project, add the project ID to the dataset names in the following format:
   `project_id:dataset`.

   (Optional) Supply the `--location` flag and set the value to your
   [location](https://docs.cloud.google.com/bigquery/docs/locations).

       bq --location=location cp \
       -a -f -n \
       project_id:dataset.source_table,project_id:dataset.source_table \
       project_id:dataset.destination_table

   Replace the following:
   - `location`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [`.bigqueryrc` file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
   - `project_id`: your project ID.
   - `dataset`: the name of the source or destination dataset.
   - `source_table`: the table that you're copying.
   - `destination_table`: the name of the table in the destination dataset.

   Examples:

   To copy the `mydataset.mytable` table and the `mydataset.mytable2` table to
   `mydataset2.tablecopy` table, enter the following command . All datasets are
   in your default project.

   ```bash
   bq cp \
   mydataset.mytable,mydataset.mytable2 \
   mydataset2.tablecopy
   ```

   To copy the `mydataset.mytable` table and the `mydataset.mytable2` table to
   `myotherdataset.mytable` table and to overwrite a destination table with the
   same name, enter the following command. The destination dataset is in the
   `myotherproject` project, not your default project. The `-f` shortcut is
   used to overwrite the destination table without a prompt.

   ```bash
   bq cp -f \
   mydataset.mytable,mydataset.mytable2 \
   myotherproject:myotherdataset.mytable
   ```

   To copy the `myproject:mydataset.mytable` table and the
   `myproject:mydataset.mytable2` table and to return an error if the
   destination dataset contains a table with the same name, enter the following
   command. The destination dataset is in the `myotherproject` project. The
   `-n` shortcut is used to prevent overwriting a table with the same name.

   ```bash
   bq cp -n \
   myproject:mydataset.mytable,myproject:mydataset.mytable2 \
   myotherproject:myotherdataset.mytable
   ```

   To copy the `mydataset.mytable` table and the `mydataset.mytable2` table and
   to append the data to a destination table with the same name, enter the
   following command. The source dataset is in your default project. The
   destination dataset is in the `myotherproject` project. The `-a` shortcut is
   used to append to the destination table.

   ```bash
   bq cp -a \
   mydataset.mytable,mydataset.mytable2 \
   myotherproject:myotherdataset.mytable
   ```

### API

To copy multiple tables using the API, call the
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method, configure a table `copy` job, and specify the `sourceTables`
property.

Specify your region in the `location` property in the
`jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

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

    // copyMultiTable demonstrates using a copy job to copy multiple source tables into a single destination table.
    func copyMultiTable(projectID, srcDatasetID string, srcTableIDs []string, dstDatasetID, dstTableID string) error {
    	// projectID := "my-project-id"
    	// srcDatasetID := "sourcedataset"
    	// srcTableIDs := []string{"table1","table2"}
    	// dstDatasetID = "destinationdataset"
    	// dstTableID = "destinationtable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	srcDataset := client.Dataset(srcDatasetID)
    	dstDataset := client.Dataset(dstDatasetID)
    	var tableRefs []*bigquery.Table
    	for _, v := range srcTableIDs {
    		tableRefs = append(tableRefs, srcDataset.Table(v))
    	}
    	copier := dstDataset.Table(dstTableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_CopierFrom(tableRefs...)
    	copier.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty
    	job, err := copier.Run(ctx)
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
    	return nil
    }

<br />

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import java.util.Arrays;

    public class CopyMultipleTables {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String destinationDatasetName = "MY_DATASET_NAME";
        String destinationTableId = "MY_TABLE_NAME";
        String sourceTable1Id = "MY_SOURCE_TABLE_1";
        String sourceTable2Id = "MY_SOURCE_TABLE_2";
        copyMultipleTables(destinationDatasetName, destinationTableId, sourceTable1Id, sourceTable2Id);
      }

      public static void copyMultipleTables(
          String destinationDatasetName,
          String destinationTableId,
          String sourceTable1Id,
          String sourceTable2Id) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html destinationTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(destinationDatasetName, destinationTableId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html sourceTable1 = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(destinationDatasetName, sourceTable1Id);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html sourceTable2 = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(destinationDatasetName, sourceTable2Id);

          // For more information on CopyJobConfiguration see:
          // https://googleapis.dev/java/google-cloud-clients/latest/com/google/cloud/bigquery/JobConfiguration.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html.newBuilder(
                      destinationTable, Arrays.asList(sourceTable1, sourceTable2))
                  .build();

          // For more information on Job see:
          // https://googleapis.dev/java/google-cloud-clients/latest/index.html?com/google/cloud/bigquery/package-summary.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));

          // Blocks until this job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (completedJob == null) {
            System.out.println("Job not executed since it no longer exists.");
            return;
          } else if (completedJob.getStatus().getError() != null) {
            System.out.println(
                "BigQuery was unable to copy tables due to an error: \n" + job.getStatus().getError());
            return;
          }
          System.out.println("Table copied successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Table copying job was interrupted. \n" + e.toString());
        }
      }
    }

<br />

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

    async function copyTableMultipleSource() {
      // Copy multiple source tables to a given destination.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";
      // sourceTable = 'my_table';
      // destinationTable = 'testing';

      // Create a client
      const dataset = bigquery.dataset(datasetId);

      const metadata = {
        createDisposition: 'CREATE_NEVER',
        writeDisposition: 'WRITE_TRUNCATE',
      };

      // Create table references
      const table = dataset.table(sourceTable);
      const yourTable = dataset.table(destinationTable);

      // Copy table
      const [apiResponse] = await table.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(yourTable, metadata);
      console.log(apiResponse.configuration.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html);
    }

<br />

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

    # TODO(developer): Set dest_table_id to the ID of the destination table.
    # dest_table_id = "your-project.your_dataset.your_table_name"

    # TODO(developer): Set table_ids to the list of the IDs of the original tables.
    # table_ids = ["your-project.your_dataset.your_table_name", ...]

    job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_copy_table(table_ids, dest_table_id)  # Make an API request.
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()  # Wait for the job to complete.

    print("The tables {} have been appended to {}".format(table_ids, dest_table_id))

<br />

### Copy tables across regions

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

You can copy a table, [table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro), or
[table clone](https://docs.cloud.google.com/bigquery/docs/table-clones-intro) from one [BigQuery
region](https://docs.cloud.google.com/bigquery/docs/locations) or multi-region to another. This includes any
tables that have customer-managed Cloud KMS (CMEK) applied.

Copying a table across regions incurs additional data transfer charges according
to [BigQuery pricing](https://cloud.google.com/bigquery/pricing#data_replication).
Additional charges are incurred even if you cancel the cross-region table copy
job before it has been completed.

To copy a table across regions, select one of the following options:

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Run the [`bq cp` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cp):

       bq cp \
       -f -n \
       SOURCE_PROJECT:SOURCE_DATASET.SOURCE_TABLE \
       DESTINATION_PROJECT:DESTINATION_DATASET.DESTINATION_TABLE
       
Replace the following:

- `SOURCE_PROJECT`: source project ID. If the source dataset is in a project other than your default
  project, add the project ID to the source dataset name.

- `DESTINATION_PROJECT`: destination project ID. If the destination dataset is in a project other than your default
  project, add the project ID to the destination dataset name.

- `SOURCE_DATASET`: the name of the source dataset.

- `DESTINATION_DATASET`: the name of the destination dataset.

- `SOURCE_TABLE`: the table that you are copying.

- `DESTINATION_TABLE`: the name of the table in
  the destination dataset.

The following example is a command that copies the `mydataset_us.mytable` table from the `us` multi-region to
the `mydataset_eu.mytable2` table in the `eu` multi-region. Both datasets are in the default project.

```sh
bq cp --sync=false mydataset_us.mytable mydataset_eu.mytable2
```

To copy a table across regions into a CMEK-enabled destination dataset, you must [enable CMEK on the table](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#change_to_kms) with a key from the table's region. The CMEK on the table doesn't have to be the same CMEK in use by the destination dataset. The following example copies a CMEK-enabled table to a destination dataset using the `bq cp` command.

```sh
bq cp source-project-id:source-dataset-id.source-table-id destination-project-id:destination-dataset-id.destination-table-id
```

Conversely, to copy a CMEK-enabled table across regions into a destination dataset, you can [enable CMEK on the destination dataset](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#dataset_default_key) with a key from the destination dataset's region. You can also use the `destination_kms_keys` flag in the `bq cp` command, as shown in the following example:

```sh
bq cp --destination_kms_key=projects/project_id/locations/eu/keyRings/eu_key/cryptoKeys/eu_region mydataset_us.mytable mydataset_eu.mytable2
```

### API

To copy a table across regions using the API, call the
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method and configure a table `copy` job.

Specify your region in the `location` property in the
`jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

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


    using Google.Apis.Bigquery.v2.Data;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryCopyTable
    {
        public void CopyTable(
            string projectId = "your-project-id",
            string destinationDatasetId = "your_dataset_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            TableReference sourceTableRef = new TableReference()
            {
                TableId = "shakespeare",
                DatasetId = "samples",
                ProjectId = "bigquery-public-data"
            };
            TableReference destinationTableRef = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetTableReference_System_String_System_String_(
                destinationDatasetId, "destination_table");
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html job = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_CreateCopyJob_Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_CreateCopyJobOptions_(
                sourceTableRef, destinationTableRef)
                .https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_() // Wait for the job to complete.
                .ThrowOnAnyError();

            // Retrieve destination table
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html destinationTable = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetTable_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_GetTableOptions_(destinationTableRef);
            Console.WriteLine(
                $"Copied {destinationTable.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_Resource.NumRows} rows from table "
                + $"{sourceTableRef.DatasetId}.{sourceTableRef.TableId} "
                + $"to {destinationTable.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_FullyQualifiedId}."
            );
        }
    }

<br />

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

    // copyTable demonstrates copying a table from a source to a destination, and
    // allowing the copy to overwrite existing data by using truncation.
    func copyTable(projectID, datasetID, srcID, dstID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// srcID := "sourcetable"
    	// dstID := "destinationtable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	dataset := client.Dataset(datasetID)
    	copier := dataset.Table(dstID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_CopierFrom(dataset.Table(srcID))
    	copier.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty
    	job, err := copier.Run(ctx)
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
    	return nil
    }

<br />

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    public class CopyTable {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String destinationDatasetName = "MY_DESTINATION_DATASET_NAME";
        String destinationTableId = "MY_DESTINATION_TABLE_NAME";
        String sourceDatasetName = "MY_SOURCE_DATASET_NAME";
        String sourceTableId = "MY_SOURCE_TABLE_NAME";

        copyTable(sourceDatasetName, sourceTableId, destinationDatasetName, destinationTableId);
      }

      public static void copyTable(
          String sourceDatasetName,
          String sourceTableId,
          String destinationDatasetName,
          String destinationTableId) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html sourceTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(sourceDatasetName, sourceTableId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html destinationTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(destinationDatasetName, destinationTableId);

          // For more information on CopyJobConfiguration see:
          // https://googleapis.dev/java/google-cloud-clients/latest/com/google/cloud/bigquery/JobConfiguration.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html.newBuilder(destinationTable, sourceTable).build();

          // For more information on Job see:
          // https://googleapis.dev/java/google-cloud-clients/latest/index.html?com/google/cloud/bigquery/package-summary.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));

          // Blocks until this job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (completedJob == null) {
            System.out.println("Job not executed since it no longer exists.");
            return;
          } else if (completedJob.getStatus().getError() != null) {
            System.out.println(
                "BigQuery was unable to copy table due to an error: \n" + job.getStatus().getError());
            return;
          }
          System.out.println("Table copied successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Table copying job was interrupted. \n" + e.toString());
        }
      }
    }

<br />

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

    async function copyTable() {
      // Copies src_dataset:src_table to dest_dataset:dest_table.

      /**
       * TODO(developer): Uncomment the following lines before running the sample
       */
      // const srcDatasetId = "my_src_dataset";
      // const srcTableId = "my_src_table";
      // const destDatasetId = "my_dest_dataset";
      // const destTableId = "my_dest_table";

      // Copy the table contents into another table
      const [job] = await bigquery
        .dataset(srcDatasetId)
        .table(srcTableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(bigquery.dataset(destDatasetId).table(destTableId));

      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
      }
    }

<br />

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
    use Google\Cloud\Core\ExponentialBackoff;

    /** Uncomment and populate these variables in your code */
    // $projectId = 'The Google project ID';
    // $datasetId = 'The BigQuery dataset ID';
    // $sourceTableId   = 'The BigQuery table ID to copy from';
    // $destinationTableId = 'The BigQuery table ID to copy to';

    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $dataset = $bigQuery->dataset($datasetId);
    $sourceTable = $dataset->table($sourceTableId);
    $destinationTable = $dataset->table($destinationTableId);
    $copyConfig = $sourceTable->copy($destinationTable);
    $job = $sourceTable->runJob($copyConfig);

    // poll the job until it is complete
    $backoff = new ExponentialBackoff(10);
    $backoff->execute(function () use ($job) {
        print('Waiting for job to complete' . PHP_EOL);
        $job->reload();
        if (!$job->isComplete()) {
            throw new Exception('Job has not yet completed', 500);
        }
    });
    // check if the job has errors
    if (isset($job->info()['status']['errorResult'])) {
        $error = $job->info()['status']['errorResult']['message'];
        printf('Error running job: %s' . PHP_EOL, $error);
    } else {
        print('Table copied successfully' . PHP_EOL);
    }

<br />

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

    # TODO(developer): Set source_table_id to the ID of the original table.
    # source_table_id = "your-project.source_dataset.source_table"

    # TODO(developer): Set destination_table_id to the ID of the destination table.
    # destination_table_id = "your-project.destination_dataset.destination_table"

    job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_copy_table(source_table_id, destination_table_id)
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()  # Wait for the job to complete.

    print("A copy of the table created.")

<br />

#### Limitations

Copying a table across regions is subject to the following limitations:

- You can't copy a table using the Google Cloud console or the [`TABLE COPY
  DDL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_copy) statement.
- You can't copy a table if there are any policy tags on the source table.
- You can't copy a table if the source table is larger than 20 physical TiB. See [get information about tables](https://docs.cloud.google.com/bigquery/docs/tables#get_information_about_tables) for the source table physical size. Additionally, copying source tables that are larger than 1 physical TiB across regions may need multiple retries to successfully copy them.
- You can't copy IAM policies associated with the tables. You can apply the same policies to the destination after the copy is completed.
- If the copy operation overwrites an existing table, [tags](https://docs.cloud.google.com/bigquery/docs/tags) on the existing table are removed.
- You can't copy multiple source tables into a single destination table.
- You can't copy tables in append mode. If you use `write_empty` mode, the destination table must not exist.
- [Time travel](https://docs.cloud.google.com/bigquery/docs/time-travel) information is not copied to the destination region.
- When you copy a table clone or snapshot to a new region, a full copy of the table is created. This incurs additional storage costs.
- Expiration time from the source table is copied to the destination table.

### View current quota usage

You can view your current usage of query, load, extract, or copy jobs by running
an `INFORMATION_SCHEMA` query to view metadata about the jobs ran over a
specified time period. You can compare your current usage against the [quota
limit](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) to determine your quota usage for a
particular type of job. The following example query uses the
`INFORMATION_SCHEMA.JOBS` view to list the number of query, load, extract, and
copy jobs by project:

```googlesql
SELECT
  sum(case  when job_type="QUERY" then 1 else 0 end) as QRY_CNT,
  sum(case  when job_type="LOAD" then 1 else 0 end) as LOAD_CNT,
  sum(case  when job_type="EXTRACT" then 1 else 0 end) as EXT_CNT,
  sum(case  when job_type="COPY" then 1 else 0 end) as CPY_CNT
FROM `region-REGION_NAME`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
WHERE date(creation_time)= CURRENT_DATE()
```

> [!NOTE]
> **Note:** The `INFORMATION_SCHEMA` view does not display cross-region copy jobs.

### Maximum number of copy jobs per day per project quota errors

BigQuery returns this error when the number of copy jobs running
in a project has exceeded the daily limit.
To learn more about the limit for copy jobs per day, see
[Copy jobs](https://docs.cloud.google.com/bigquery/quotas#copy_jobs).

**Error message**

```
Your project exceeded quota for copies per project
```

#### Diagnosis

If you'd like to gather more data about where the copy jobs are coming from,
you can try the following:

- If your copy jobs are located in a single or only a few regions, you can try
  querying the [`INFORMATION_SCHEMA.JOBS`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs)
  table for specific regions. For example:

  ```googlesql
  SELECT
  creation_time, job_id, user_email, destination_table.project_id, destination_table.dataset_id, destination_table.table_id
  FROM `PROJECT_ID`.`region-REGION_NAME`.INFORMATION_SCHEMA.JOBS
  WHERE
  creation_time BETWEEN TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 2 DAY) AND CURRENT_TIMESTAMP()
  AND job_type = "COPY"
  order by creation_time DESC
  ```

  You can also adjust the time interval depending on the time range you're interested in.
- To see all copy jobs in all regions, you can use the following filter in
  Cloud Logging:

  ```
  resource.type="bigquery_resource"
  protoPayload.methodName="jobservice.insert"
  protoPayload.serviceData.jobInsertRequest.resource.jobConfiguration.tableCopy:*
  ```

  <br />

#### Resolution

- If the goal of the frequent copy operations is to create a snapshot of data, consider using [table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) instead. Table snapshots are a cheaper and faster alternative to copying full tables.
- You can request a quota increase by contacting [support](https://docs.cloud.google.com/bigquery/docs/getting-support) or [sales](https://cloud.google.com/contact). It might take several days to review and process the request. We recommend stating the priority, use case, and the project ID in the request.

## Delete tables

You can delete a table in the following ways:

- Using the Google Cloud console.
- Using a data definition language (DDL) `DROP TABLE` statement.
- Using the bq command-line tool `bq rm` command.
- Calling the [`tables.delete`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/delete) API method.
- Using the client libraries.

To delete all of the tables in the dataset,
[delete the dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#delete-datasets).

When you delete a table, any data in the table is also deleted. To automatically
delete tables after a specified period of time, set the
[default table expiration](https://docs.cloud.google.com/bigquery/docs/updating-datasets#table-expiration)
for the dataset or set the expiration time when you [create the table](https://docs.cloud.google.com/bigquery/docs/tables#create-table).

Deleting a table also deletes any permissions associated with this table. When
you recreate a deleted table, you must also manually [reconfigure any access permissions](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam)
previously associated with it.

### Required roles


To get the permissions that
you need to delete a table,

ask your administrator to grant you the
[Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) IAM role on the dataset.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to delete a table. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to delete a table:

- `bigquery.tables.delete`
- `bigquery.tables.get`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Delete a table

To delete a table:

### Console

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

3. Click **Overview \> Tables**, and then select a table.

4. In the details pane, click **Delete**.

5. Type `"delete"` in the dialog, then click **Delete** to
   confirm.

### SQL

Use the
[`DROP TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_table_statement).
The following example deletes a table named `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   DROP TABLE mydataset.mytable;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Use the `bq rm` command with the `--table` flag (or `-t` shortcut) to delete
   a table. When you use the bq command-line tool to remove a table, you must confirm the
   action. You can use the `--force` flag (or `-f` shortcut) to skip
   confirmation.

   If the table is in a dataset in a project other than your default
   project, add the project ID to the dataset name in the following format:
   `project_id:dataset`.

   ```bash
   bq rm \
   -f \
   -t \
   project_id:dataset.table
   ```

   Replace the following:
   - `project_id`: your project ID
   - `dataset`: the name of the dataset that contains the table
   - `table`: the name of the table that you're deleting

   Examples:

   To delete the `mytable` table from the `mydataset` dataset, enter the
   following command. The `mydataset` dataset is in your default project.

   ```sh
   bq rm -t mydataset.mytable
   ```

   To delete the `mytable` table from the `mydataset` dataset, enter the
   following command. The `mydataset` dataset is in the `myotherproject`
   project, not your default project.

   ```sh
   bq rm -t myotherproject:mydataset.mytable
   ```

   To delete the `mytable` table from the `mydataset` dataset, enter the
   following command. The `mydataset` dataset is in your default project. The
   command uses the `-f` shortcut to bypass confirmation.

   ```sh
   bq rm -f -t mydataset.mytable
   ```

   > [!NOTE]
   > **Note:** You can enter the [`bq ls dataset`](https://docs.cloud.google.com/bigquery/docs/tables#list_tables_in_a_dataset) command in the bq command-line tool to confirm that a table was removed from a dataset.

### API

Call the [`tables.delete`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/delete)
API method and specify the table to delete using the `tableId` parameter.

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
    using System;

    public class BigQueryDeleteTable
    {
        public void DeleteTable(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_id",
            string tableId = "your_table_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_DeleteTable_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_DeleteTableOptions_(datasetId, tableId);
            Console.WriteLine($"Table {tableId} deleted.");
        }
    }

<br />

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

    // deleteTable demonstrates deletion of a BigQuery table.
    func deleteTable(projectID, datasetID, tableID string) error {
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
    	if err := table.Delete(ctx); err != nil {
    		return err
    	}
    	return nil
    }

<br />

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

    public class DeleteTable {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        deleteTable(datasetName, tableName);
      }

      public static void deleteTable(String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();
          boolean success = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____(TableId.of(datasetName, tableName));
          if (success) {
            System.out.println("Table deleted successfully");
          } else {
            System.out.println("Table was not found");
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table was not deleted. \n" + e.toString());
        }
      }
    }

<br />

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

    async function deleteTable() {
      // Deletes "my_table" from "my_dataset".

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";
      // const tableId = "my_table";

      // Delete the table
      await bigquery
        .dataset(datasetId)
        .table(tableId)
        .delete();

      console.log(`Table ${tableId} deleted.`);
    }

<br />

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

    /** Uncomment and populate these variables in your code */
    // $projectId = 'The Google project ID';
    // $datasetId = 'The BigQuery dataset ID';
    // $tableId = 'The BigQuery table ID';

    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $dataset = $bigQuery->dataset($datasetId);
    $table = $dataset->table($tableId);
    $table->delete();
    printf('Deleted table %s.%s' . PHP_EOL, $datasetId, $tableId);

<br />

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

    # TODO(developer): Set table_id to the ID of the table to fetch.
    # table_id = 'your-project.your_dataset.your_table'

    # If the table does not exist, delete_table raises
    # google.api_core.exceptions.NotFound unless not_found_ok is True.
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_delete_table(table_id, not_found_ok=True)  # Make an API request.
    print("Deleted table '{}'.".format(table_id))

<br />

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

    def delete_table dataset_id = "my_dataset_id", table_id = "my_table_id"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      dataset  = bigquery.dataset dataset_id
      table    = dataset.table table_id

      table.delete

      puts "Table #{table_id} deleted."
    end

<br />

## Troubleshoot deleted tables

There are three main reasons for a deleted table: manual table deletion, expiration,
and dataset deletion.

### Table deletion

Check Cloud Audit Logs for a `google.cloud.bigquery.v2.TableService.DeleteTable` event.
To view deleted tables, select one of the following options:

### Console

1. In the Google Cloud console, go to the **Logs Explorer** page.

   [Go to Logs Explorer](https://console.cloud.google.com/logs)
2. In the **Filter** section, use the following Cloud Logging Filter and **Run Query**,

       resource.type="bigquery_dataset"
       protoPayload.methodName="google.cloud.bigquery.v2.TableService.DeleteTable"
       protoPayload.resourceName="projects/PROJECT_ID/datasets/DATASET_ID/tables/TABLE_ID"

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Use the `gcloud logging read` command with the following filters:

   ```sh
   gcloud logging read '
   resource.type="bigquery_dataset"
   protoPayload.methodName="google.cloud.bigquery.v2.TableService.DeleteTable"
   protoPayload.resourceName=~"projects/PROJECT_ID/datasets/DATASET_ID/tables/TABLE_ID"
   '
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID
   - `DATASET_ID`: the name of the dataset that contains the table
   - `TABLE_ID`: the name of the table that was deleted

### Table expiration

Tables can be created with an expiration time. Once this time is reached, BigQuery automatically deletes the table.
To view deleted tables, select one of the following options:

### Console

1. In the Google Cloud console, go to the **Logs Explorer** page.

   [Go to Logs Explorer](https://console.cloud.google.com/logs)
2. In the **Filter** section, use the following Cloud Logging Filter and click **Run Query**.

       protoPayload.methodName="InternalTableExpired"
         protoPayload.resourceName="projects/PROJECT_ID/datasets/DATASET_ID/tables/TABLE_ID"

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Use the `gcloud logging read` command with the following filters:

   ```sh
   gcloud logging read '
       protoPayload.methodName="InternalTableExpired"
       protoPayload.resourceName=~"projects/PROJECT_ID/datasets/DATASET_ID/tables/TABLE_ID"
       '
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID
   - `DATASET_ID`: the name of the dataset that contains the table
   - `TABLE_ID`: the name of the table that was deleted

You can query the [`INFORMATION_SCHEMA.TABLE_OPTIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-options) view to check the `expiration_timestamp` column for existing tables.

### Dataset deletion

If the dataset containing the table was deleted, the table will also be deleted.
To view deleted tables, select one of the following options:

### Console

1. In the Google Cloud console, go to the **Logs Explorer** page.

   [Go to Logs Explorer](https://console.cloud.google.com/logs)
2. In the **Filter** section, use the following Cloud Logging Filter and **Run Query**.

       resource.type="bigquery_dataset"
       protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.DeleteDataset"
       protoPayload.resourceName="projects/PROJECT_ID/datasets/DATASET_ID"

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Use the `gcloud logging read` command with the following filters:

   ```sh
   gcloud logging read '
   resource.type="bigquery_dataset"
   protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.DeleteDataset"
   protoPayload.resourceName=~"projects/PROJECT_ID/datasets/DATASET_ID"
   '
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID
   - `DATASET_ID`: the name of the dataset that contains the table

## Restore deleted tables

To learn how to restore or undelete deleted tables, see
[Restore deleted tables](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables).

## Table security

To control access to tables in BigQuery, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

## What's next

- For more information about creating and using tables, see [Creating and using tables](https://docs.cloud.google.com/bigquery/docs/tables).
- For more information about handling data, see [Working With table data](https://docs.cloud.google.com/bigquery/docs/managing-table-data).
- For more information about specifying table schemas, see [Specifying a schema](https://docs.cloud.google.com/bigquery/docs/schemas).
- For more information about modifying table schemas, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).
- For more information about datasets, see [Introduction to datasets](https://docs.cloud.google.com/bigquery/docs/datasets-intro).
- For more information about views, see [Introduction to views](https://docs.cloud.google.com/bigquery/docs/views-intro).