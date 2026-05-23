# Updating labels

This page explains how to update labels on BigQuery resources.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document. The permissions required to perform a task (if any) are listed in the "Required permissions" section of the task.

## Update dataset labels

A dataset label can be updated by:

- Using the Google Cloud console
- Using SQL [DDL statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)
- Using the bq command-line tool's `bq update` command
- Calling the [`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch) API method
- Using the client libraries

### Required permissions

To update a dataset label, you need the `bigquery.datasets.update` IAM permission.

Each of the following predefined IAM roles includes the permissions that you need in order to update a dataset label:

- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can update labels of the datasets that you create.

For more information on IAM roles and permissions in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Update a dataset label

To update labels on a dataset, select one of the following options:

### Console

1. In the Google Cloud console, select the dataset.

2. On the dataset details page, click the pencil icon to the right of
   **Labels**.

   ![Label pencil](https://docs.cloud.google.com/static/bigquery/images/label-pencil.png)
3. In the **Edit labels** dialog:

   - To apply additional labels, click **Add label**. Each key can be used only once per dataset, but you can use the same key in different datasets in the same project.
   - Modify the existing keys or values to update a label.
   - Click **Update** to save your changes.

### SQL

Use the
[`ALTER SCHEMA SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_set_options_statement)
to set the labels on an existing dataset. Setting labels overwrites any
existing labels on the dataset. The following example sets a single label on
the dataset `mydataset`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER SCHEMA mydataset
   SET OPTIONS (labels = [('sensitivity', 'high')]);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To add additional labels or to update a dataset label, issue the `bq update`
command with the `set_label` flag. Repeat the flag to add or update multiple
labels.

If the dataset is in a project other than your default project, add the
project ID to the dataset in the following format: `[PROJECT_ID]:[DATASET]`.

```bash
bq update \
--set_label key:value \
project_id:dataset
```

Where:

- <var translate="no">key:value</var> corresponds to a key:value pair for a label that you want to add or update. If you specify the same key as an existing label, the value for the existing label is updated. The key must be unique.
- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the dataset you're updating.

Example:

To update the `department` label on `mydataset`, enter the `bq update`
command and specify `department` as the label key. For example, to update
the `department:shipping` label to `department:logistics`, enter the
following command. `mydataset` is in `myotherproject`, not your default
project.

        bq update \
        --set_label department:logistics \
        myotherproject:mydataset

The output looks like the following.

```
Dataset 'myotherproject:mydataset' successfully updated.
```

<br />

### API

To add additional labels or to update a label for an existing dataset, call
the [`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch)
method and add to or update the `labels` property for the
[dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

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

    // addDatasetLabel demonstrates adding label metadata to an existing dataset.
    func addDatasetLabel(projectID, datasetID string) error {
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
    	update.SetLabel("color", "green")
    	if _, err := ds.Update(ctx, update, meta.ETag); err != nil {
    		return err
    	}
    	return nil
    }

### Java

This sample uses the [Google HTTP Client Library for
Java](https://developers.google.com/api-client-library/java/google-http-java-client/)
to send a request to the BigQuery API.


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

    // Sample to updates a label on dataset
    public class LabelDataset {

      public static void runLabelDataset() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        labelDataset(datasetName);
      }

      public static void labelDataset(String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // This example dataset starts with existing label { color: 'green' }
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html dataset = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getDataset_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetOption____(datasetName);
          // Add label to dataset
          Map<String, String> labels = new HashMap<>();
          labels.put("color", "green");

          dataset.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html#com_google_cloud_bigquery_Dataset_toBuilder__().setLabels(labels).build().update();
          System.out.println("Label added successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Label was not added. \n" + e.toString());
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

    async function labelDataset() {
      // Updates a label on a dataset.

      /**
       * TODO(developer): Uncomment the following lines before running the sample
       */
      // const datasetId = "my_dataset";

      // Retrieve current dataset metadata.
      const dataset = bigquery.dataset(datasetId);
      const [metadata] = await dataset.getMetadata();

      // Add label to dataset metadata
      metadata.labels = {color: 'green'};
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
    dataset.labels = {"color": "green"}
    dataset = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_dataset(dataset, ["labels"])  # Make an API request.

    print("Labels added to {}".format(dataset_id))

## Update table and view labels

A label can be updated after a table or view is created by:

- Using the Google Cloud console
- Using the bq command-line tool's `bq update` command
- Calling the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) API method
  - Because views are treated like table resources, you use the `tables.patch` method to modify both views and tables.
- Using the client libraries

### Required permissions

To update a table or view label, you need the `bigquery.tables.update` IAM permission.

Each of the following predefined IAM roles includes the permissions that you need in order to update a table or view label:

- `roles/bigquery.dataEditor`
- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can update labels of the tables and views in the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Update a table or view label

To update a table or view label:

### Console

1. In the Google Cloud console, select the table or view.

2. Click the **Details** tab, and then click the pencil icon to the right of
   **Labels**.

3. In the **Edit labels** dialog:

   - To apply additional labels, click **Add label**. Each key can be used only once per table or view, but you can use the same key in tables or views in different datasets.
   - Modify the existing keys or values to update a label.
   - Click **Update** to save your changes.

### SQL

Use the
[`ALTER TABLE SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement)
to set the labels on an existing table, or the
[`ALTER VIEW SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_view_set_options_statement)
to set the labels on an existing view. Setting labels overwrites any
existing labels on the table or view. The following example sets two labels
on the table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
   SET OPTIONS (
     labels = [('department', 'shipping'), ('cost_center', 'logistics')]);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To add additional labels or to update a table or view label, issue the `bq
update` command with the `set_label` flag. Repeat the flag to add or update
multiple labels.

If the table or view is in a project other than your default project, add
the project ID to the dataset in the following format:
`project_id:dataset`.

```bash
bq update \
--set_label key:value \
project_id:dataset.table_or_view
```

Where:

- <var translate="no">key:value</var> corresponds to a key:value pair for a label that you want to add or update. If you specify the same key as an existing label, the value for the existing label is updated. The key must be unique.
- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the dataset that contains the table or view you're updating.
- <var translate="no">table_or_view</var> is the name of the table or view you're updating.

Example:

To update the `department` label for `mytable`, enter the `bq update`
command and specify `department` as the label key. For example, to update
the `department:shipping` label to `department:logistics` for `mytable`,
enter the following command. `mytable` is in `myotherproject`, not your
default project.

        bq update \
        --set_label department:logistics \
        myotherproject:mydataset.mytable

The output looks like the following:

```
Table 'myotherproject:mydataset.mytable' successfully updated.
```

<br />

### API

To add labels or to update a label for an existing table or view,
call the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and add to or update the `labels`
property for the [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables).

Because views are treated like table resources, you use the `tables.patch`
method to modify both views and tables.

Because the `tables.update` method replaces the entire dataset resource, the
`tables.patch` method is preferred.

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

    // addTableLabel demonstrates adding Label metadata to a BigQuery table.
    func addTableLabel(projectID, datasetID, tableID string) error {
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
    	update.SetLabel("color", "green")
    	if _, err := tbl.Update(ctx, update, meta.ETag); err != nil {
    		return err
    	}
    	return nil
    }

### Java

This sample uses the [Google HTTP Client Library for Java](https://developers.google.com/api-client-library/java/google-http-java-client/)
to send a request to the BigQuery API.


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

    // Sample to adds a label to an existing table
    public class LabelTable {

      public static void runLabelTable() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        labelTable(datasetName, tableName);
      }

      public static void labelTable(String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // This example table starts with existing label { color: 'green' }
          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(TableId.of(datasetName, tableName));
          // Add label to table
          Map<String, String> labels = new HashMap<>();
          labels.put("color", "green");

          table.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html#com_google_cloud_bigquery_biglake_v1_Table_toBuilder__().setLabels(labels).build().update();
          System.out.println("Label added successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Label was not added. \n" + e.toString());
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

    async function labelTable() {
      // Adds a label to an existing table.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      const dataset = bigquery.dataset(datasetId);
      const [table] = await dataset.table(tableId).get();

      // Retrieve current table metadata
      const [metadata] = await table.getMetadata();

      // Add label to table metadata
      metadata.labels = {color: 'green'};
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

    # from google.cloud import bigquery
    # client = bigquery.Client()
    # project = client.project
    # dataset_ref = bigquery.DatasetReference(project, dataset_id)
    # table_ref = dataset_ref.table('my_table')
    # table = client.get_table(table_ref)  # API request

    assert table.labels == {}
    labels = {"color": "green"}
    table.labels = labels

    table = client.update_table(table, ["labels"])  # API request

    assert table.labels == labels

## Update job labels

Updating a job label is not supported. To update the label on a job,
resubmit the job with a new label specified.

## Update reservation labels

You can update a label on a reservation. Updating a label using SQL overwrites
any existing labels on the reservation.

### Required IAM roles


To get the permission that
you need to update a label to a reservation,

ask your administrator to grant you the
[BigQuery Resource Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.resourceEditor) (`roles/bigquery.resourceEditor`) IAM role on the administration project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`bigquery.reservations.update`
permission,
which is required to
update a label to a reservation.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Update a label on a reservation

To update a label to a reservation:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot reservations** tab.

4. Find the reservation you want to update.

5. Expand the

   **Actions** option.

6. Click **Edit**.

7. To expand the **Advanced settings** section, click the
   expander arrow.

8. Update the names of the key-value pair.

9. Click **Save**.

### bq

To update a label to a reservation, issue the `bq update` command with the `set_label` flag and `--reservation` flag. To update multiple labels, repeat the flag.

```bash
bq update --set_label KEY:VALUE  --reservation RESERVATION_NAME
```

Replace the following:

- `KEY:VALUE`: a key-value pair for a label that you want to update on the reservation. The key must be unique. Keys and values can contain only lowercase letters, numeric characters, underscores, and dashes. All characters must use UTF-8 encoding, and international characters are allowed. To update multiple labels on a reservation, repeat the `--set_label` flag and specify a unique key for each label.
- `RESERVATION_NAME`: the name of the reservation.

### SQL

To update a label to a reservation, use the [`ALTER RESERVATION SET OPTIONS` DDL
statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_reservation_set_options_statement)
to set the labels on an existing reservation. Setting labels overwrites any
existing labels on the reservation. The following example sets a label on the
reservation `myreservation`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER RESERVATION myreservation
   SET OPTIONS (
     labels = [('sensitivity', 'high')]);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## Convert labels to tags

A label that has a key with an empty value is used as a tag. You can create a
new label with no value, or you can turn an existing label into a tag on a
dataset, table, or view. You cannot convert a job label to a tag.

Tags can be useful in situations where you are labeling a resource, but you
don't need the `key:value` format. For example, if you have a table that
contains test data that is used by multiple groups (support, development, and so
on), you can add a `test_data` tag to the table to identify it.

### Required permissions

To convert a label to a tag, you need the following IAM permissions:

- `bigquery.datasets.update` (lets you convert a dataset label)
- `bigquery.tables.update` (lets you convert a table or view label)

Each of the following predefined IAM roles includes the permissions that you need in order to convert a dataset label:

- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

Each of the following predefined IAM roles includes the permissions that you need in order to convert a table or view label:

- `roles/bigquery.dataEditor`
- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can update labels of the datasets that you create and the tables and views in those datasets.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Convert a label to a tag

To convert a label to a tag:

### Console

1. In the Google Cloud console, select the dataset, table, or view.

2. For datasets, the dataset details page is automatically opened. For
   tables and views, click **Details** to open the details page.

   ![Table details](https://docs.cloud.google.com/static/bigquery/images/table-details.png)
3. On the details page, click the pencil icon to the right of **Labels**.

   ![Label pencil](https://docs.cloud.google.com/static/bigquery/images/label-pencil.png)
4. In the **Edit labels** dialog:

   - Delete the value for an existing label.
   - Click **Update**.

### bq

To convert a label to a tag, use the `bq update` command with the
`set_label` flag. Specify the key, followed by a colon, but leave the value
unspecified. This updates an existing label to a tag.

```bash
bq update \
--set_label key: \
resource_id
```

Where:

- <var translate="no">key:</var> is the label key that you want update to a tag.
- <var translate="no">resource_id</var> is a valid dataset, table, or view name. If the resource is in a project other than your default project, add the project ID in the following format: `project_id:dataset`.

Examples:

Enter the following command to change the existing `test_data:development`
label on `mydataset` to a tag. `mydataset` is in `myotherproject`, not your
default project.

    bq update --set_label test_data: myotherproject:mydataset

The output looks like the following:

```
Dataset 'myotherproject:mydataset' successfully updated.
```

<br />

### API

To turn an existing label into a tag, call the [`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch)
method or the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and replace the label values with the empty string (`""`) in the
[dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets)
or the [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables).

Because views are treated like table resources, you use the `tables.patch`
method to modify both views and tables. Also, because the `tables.update`
method replaces the entire dataset resource, the `tables.patch` method is
preferred.

## What's next

- Learn how to [add labels](https://docs.cloud.google.com/bigquery/docs/adding-labels) to BigQuery resources.
- Learn how to [view labels](https://docs.cloud.google.com/bigquery/docs/viewing-labels) on BigQuery resources.
- Learn how to [filter resources using labels](https://docs.cloud.google.com/bigquery/docs/filtering-labels).
- Learn how to [delete labels](https://docs.cloud.google.com/bigquery/docs/deleting-labels) on BigQuery resources.
- Read about [using labels](https://docs.cloud.google.com/resource-manager/docs/using-labels) in the Resource Manager documentation.