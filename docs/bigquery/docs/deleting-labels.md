# Deleting labels

You can delete a label from a dataset, table, or view by:

- Using the Google Cloud console
- Using SQL [DDL statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)
- Using the bq command-line tool's `bq update` command
- Calling the [`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch) or [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) API methods
- Using the client libraries

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document. The permissions required to perform a task (if any) are listed in the "Required permissions" section of the task.

## Delete a dataset label

The following sections specify the permissions and steps for deleting a dataset label.

### Required permissions

To delete a dataset label, you need the following IAM permissions:

- `bigquery.datasets.get`
- `bigquery.datasets.update`

Each of the following predefined IAM roles includes the permissions that you need in order to delete a dataset label:

- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can delete labels of the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Delete a dataset label

To delete a label from a dataset, choose one of the following options:

### Console

1. In the Google Cloud console, select the dataset.

2. On the dataset details page, click the pencil icon to the right of
   **Labels**.

   ![Label pencil](https://docs.cloud.google.com/static/bigquery/images/label-pencil.png)
3. In the **Edit labels** dialog:

   - For each label you want to delete, click delete (X).
   - To save your changes, click **Update**.

### SQL

Use the
[`ALTER SCHEMA SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_set_options_statement)
to set the labels on an existing dataset. Setting labels overwrites any
existing labels on the dataset. The following example deletes all labels on
the dataset `mydataset`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER SCHEMA mydataset
   SET OPTIONS (labels = []);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To delete a dataset label, issue the `bq update` command with the
`clear_label` flag. Repeat the flag to delete multiple labels.

If the dataset is in a project other than your default project, add the
project ID to the dataset in the following format:
`project_id:dataset`.

```bash
bq update \
--clear_label key \
project_id:dataset
```

Where:

- <var translate="no">key</var> is the key for a label that you want to delete.
- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the dataset you're updating.

Examples:

To delete the `department:shipping` label from `mydataset`, enter the
`bq update` command with the `--clear_label` flag. `mydataset` is in your
default project.

        bq update --clear_label department mydataset

To delete the `department:shipping` label from `mydataset` in
`myotherproject`, enter the `bq update` command with the `--clear_label`
flag.

        bq update --clear_label department myotherproject:mydataset

To delete multiple labels from a dataset, repeat the `clear_label` flag and
specify each label's key. For example, to delete the `department:shipping`
label and `cost_center:logistics` labels from `mydataset` in your default
project, enter:

        bq update \
        --clear_label department \
        --clear_label cost_center \
        mydataset

For each of these examples, the output looks like the following:

```
Dataset 'myproject:mydataset' successfully updated.
```

<br />

### API

To delete a particular label for an existing dataset, call the
[`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch)
method and update the `labels`
property for the [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets)
by setting the label's key value to `null`.

To delete all labels from a dataset, call the
[`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch)
method and delete the `labels` property.

Because the `datasets.update` method replaces the entire dataset resource,
the `datasets.patch` method is preferred.

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

    // deleteDatasetLabel demonstrates removing a specific label from a dataset's metadata.
    func deleteDatasetLabel(projectID, datasetID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	ds := client.Dataset(datasetID)
    	meta, err := ds.Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	update := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_DatasetMetadataToUpdate{}
    	update.DeleteLabel("color")
    	if _, err := ds.Update(ctx, update, meta.ETag); err != nil {
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html;
    import java.util.HashMap;
    import java.util.Map;

    // Sample tp deletes a label on a dataset.
    public class DeleteLabelDataset {

      public static void runDeleteLabelDataset() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        deleteLabelDataset(datasetName);
      }

      public static void deleteLabelDataset(String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // This example dataset starts with existing label { color: 'green' }
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html dataset = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getDataset_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetOption____(datasetName);
          // Add label to dataset
          Map<String, String> labels = new HashMap<>();
          labels.put("color", null);

          dataset.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html#com_google_cloud_bigquery_Dataset_toBuilder__().setLabels(labels).build().update();
          System.out.println("Dataset label deleted successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Dataset label was not deleted. \n" + e.toString());
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

    async function deleteLabelDataset() {
      // Deletes a label on a dataset.
      // This example dataset starts with existing label { color: 'green' }

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';

      // Retrieve current dataset metadata.
      const dataset = bigquery.dataset(datasetId);
      const [metadata] = await dataset.getMetadata();

      // Add label to dataset metadata
      metadata.labels = {color: null};
      const [apiResponse] = await dataset.setMetadata(metadata);

      console.log(`${datasetId} labels:`);
      console.log(apiResponse.labels);
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

    # TODO(developer): Set dataset_id to the ID of the dataset to fetch.
    # dataset_id = "your-project.your_dataset"

    dataset = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_dataset(dataset_id)  # Make an API request.

    # To delete a label from a dataset, set its value to None.
    dataset.labels["color"] = None

    dataset = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_dataset(dataset, ["labels"])  # Make an API request.
    print("Labels deleted from {}".format(dataset_id))

## Delete a table or view label

You can delete a table or view label in the following ways:

- Using the Google Cloud console
- Using SQL [DDL statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)
- Using the bq command-line tool's `bq update` command
- Calling the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) API method
  - Because views are treated like table resources, `tables.patch` is used to modify both views and tables.
- Using the client libraries

### Required permissions

To delete a table or view label, you need the following IAM permissions:

- `bigquery.tables.get`
- `bigquery.tables.update`

Each of the following predefined IAM roles includes the permissions that you need in order to delete a table or view label:

- `roles/bigquery.dataEditor`
- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can delete labels of the tables and views in the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Delete a table or view label

To delete a label from a table or view, choose one of the following options:

### Console

1. In the Google Cloud console, select the dataset.

2. Click the **Details** tab, and then click the pencil icon to the
   right of **Labels**.

   ![Label pencil](https://docs.cloud.google.com/static/bigquery/images/label-pencil.png)
3. In the **Edit labels** dialog:

   - For each label you want to delete, click delete (X).

     ![Label delete](https://docs.cloud.google.com/static/bigquery/images/label-delete.png)
   - To save your changes, click **Update**.

### SQL

Use the
[`ALTER TABLE SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement) to set the label on an existing table, or the
[`ALTER VIEW SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_view_set_options_statement)
to set the label on an existing view. Setting labels overwrites any
existing labels on the table or view. The following example deletes all
labels from the table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
   SET OPTIONS (labels = []);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To delete a label from a table or view, issue the `bq update` command with
the `clear_label` flag. Repeat the flag to delete multiple labels.

If the table or view is in a project other than your default project, add
the project ID to the dataset in the following format:
`project_id:dataset`.

```bash
bq update \
--clear_label key \
project_id:dataset.table_or_view
```

Where:

- <var translate="no">key</var> is the key for a label that you want to delete.
- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the dataset you're updating.
- <var translate="no">table_or_view</var> is the name of the table or view you're updating.

Examples:

To delete the `department:shipping` label from `mydataset.mytable`, enter
the `bq update` command with the `--clear_label` flag. `mydataset` is in
your default project.

        bq update --clear_label department mydataset.mytable

To delete the `department:shipping` label from `mydataset.myview` in
`myotherproject`, enter the `bq update` command with the `--clear_label`
flag.

        bq update --clear_label department myotherproject:mydataset.myview

To delete multiple labels from a table or view, repeat the `clear_label`
flag and specify each label's key. For example, to delete the
`department:shipping` label and `cost_center:logistics` label from
`mydataset.mytable` in your default project, enter:

        bq update \
        --clear_label department \
        --clear_label cost_center \
        mydataset.mytable

For each of these examples, the output looks like the following:

```
Table 'myproject:mydataset.mytable' successfully updated.
```

<br />

### API

To delete a particular label for an existing table or view, call the
[`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and update the `labels`
property for the [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables)
by setting the label's key value to `null`.

To delete all labels from a table or view, call the
[`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and delete the `labels` property.

Because views are treated like table resources, you use the `tables.patch`
method to modify both views and tables. Also, because the `tables.update`
method replaces the entire dataset resource, the `tables.patch` method is
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

    	"cloud.google.com/go/bigquery"
    )

    // deleteTableLabel demonstrates how to remove a specific metadata Label from a BigQuery table.
    func deleteTableLabel(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	tbl := client.Dataset(datasetID).Table(tableID)
    	meta, err := tbl.Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	update := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadataToUpdate{}
    	update.DeleteLabel("color")
    	if _, err := tbl.Update(ctx, update, meta.ETag); err != nil {
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import java.util.HashMap;
    import java.util.Map;

    // Sample tp deletes a label on a table.
    public class DeleteLabelTable {

      public static void runDeleteLabelTable() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        deleteLabelTable(datasetName, tableName);
      }

      public static void deleteLabelTable(String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // This example table starts with existing label { color: 'green' }
          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(TableId.of(datasetName, tableName));
          // Add label to table
          Map<String, String> labels = new HashMap<>();
          labels.put("color", null);

          table.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html#com_google_cloud_bigquery_biglake_v1_Table_toBuilder__().setLabels(labels).build().update();
          System.out.println("Table label deleted successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table label was not deleted. \n" + e.toString());
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

    async function deleteLabelTable() {
      // Deletes a label from an existing table.
      // This example dataset starts with existing label { color: 'green' }

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";
      // const tableId = "my_table";

      const dataset = bigquery.dataset(datasetId);
      const [table] = await dataset.table(tableId).get();

      // Retrieve current table metadata
      const [metadata] = await table.getMetadata();

      // Add label to table metadata
      metadata.labels = {color: null};
      const [apiResponse] = await table.setMetadata(metadata);

      console.log(`${tableId} labels:`);
      console.log(apiResponse.labels);
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

    # TODO(dev): Change table_id to the full name of the table you wish to delete from.
    table_id = "your-project.your_dataset.your_table_name"
    # TODO(dev): Change label_key to the name of the label you want to remove.
    label_key = "color"
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # API request

    # To delete a label from a table, set its value to None
    table.labels[label_key] = None

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_table(table, ["labels"])  # API request

    print(f"Deleted label '{label_key}' from {table_id}.")

## Delete a reservation label

You can delete a reservation label.

### Required IAM roles


To get the permission that
you need to delete a reservation label,

ask your administrator to grant you the
[BigQuery Resource Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.resourceEditor) (`roles/bigquery.resourceEditor`) IAM role on the administration project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`bigquery.reservations.delete`
permission,
which is required to
delete a reservation label.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Delete a reservation label

To delete a label from a reservation, choose one of the following options:

### SQL

To delete a reservation label, use the [`ALTER RESERVATION SET OPTIONS` DDL
statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_reservation_set_options_statement).
To delete the labels on a reservation, set the labels to an empty array. The
following example deletes the label on the reservation `myreservation`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER RESERVATION myreservation
   SET OPTIONS (
     labels = []);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To delete a reservation label, issue the `bq update` command with the
`clear_label` flag and `--reservation` flag. To delete multiple labels, repeat
the flag.

```bash
bq update --clear_label KEY  --reservation RESERVATION_NAME
```

Replace the following:

- `KEY`: a key for a label that you want to delete to the reservation. The key must be unique. Keys and values can contain only lowercase letters, numeric characters, underscores, and dashes. All characters must use UTF-8 encoding, and international characters are allowed. To delete multiple labels to a reservation, repeat the `--clear_label` flag and specify a unique key for each label.
- `RESERVATION_NAME`: the name of the reservation.

## Delete job labels

Deleting a label from an existing job is not supported.

## What's next

- Learn how to [add labels](https://docs.cloud.google.com/bigquery/docs/adding-labels) to BigQuery resources.
- Learn how to [view labels](https://docs.cloud.google.com/bigquery/docs/viewing-labels) on BigQuery resources.
- Learn how to [update labels](https://docs.cloud.google.com/bigquery/docs/updating-labels) on BigQuery resources.
- Learn how to [filter resources using labels](https://docs.cloud.google.com/bigquery/docs/filtering-labels).
- Read about [using labels](https://docs.cloud.google.com/resource-manager/docs/using-labels) in the Resource Manager documentation.