# Restore deleted tables

This document describes how to restore (or *undelete* ) a deleted table in
BigQuery.
You can restore a deleted table within the time travel window specified for the
dataset, including explicit deletions and implicit deletions due to table
expiration. You can also
[configure the time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel#configure_the_time_travel_window).

For information about how to restore an entire deleted dataset or snapshot,
see the following resources:

- [Restore deleted datasets](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets)
- [Restore table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-restore)

The time travel window can have a duration between two and seven days. After the
time travel window has passed, BigQuery provides a
[fail-safe period](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe)
where the deleted data is automatically retained for an additional seven days.
Once the fail-safe period has passed, it isn't possible to restore a
table using any method, including opening a support ticket.

## Before you begin

Ensure that you have the necessary Identity and Access Management (IAM) permissions to
restore a deleted table.

### Required roles


To get the permissions that
you need to restore a deleted table,

ask your administrator to grant you the
[BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Restore a table

You can restore a table from historical data by copying the historical data into
a new table. Copying historical data works even if the table was deleted or has
expired, as long as you restore the table within the duration of the time travel
window.

When you restore a table from historical data,
[tags](https://docs.cloud.google.com/bigquery/docs/tags)
from the source table aren't copied to the destination table.
Table partitioning information also isn't copied to the destination table. To
recreate the partitioning scheme of the original table, you can view the initial
table creation request in
[Cloud Logging](https://docs.cloud.google.com/logging/docs/view/logs-explorer-interface)
and use that information to partition the restored table.

You can restore a table that was deleted but is still within the time travel
window by copying the table to a new table, using the `@<time>` time decorator.
You can't query a deleted table, even if you use a time decorator. You must
restore it first.

Use the following syntax with the `@<time>` time decorator:

- `tableid@TIME` where `TIME` is the number of milliseconds since the Unix epoch.
- `tableid@-TIME_OFFSET` where `TIME_OFFSET` is the relative offset from the current time, in milliseconds.
- `tableid@0`: Specifies the oldest available historical data.

To restore a table, select one of the following options:

### Console

You can't undelete a table by using the Google Cloud console.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To restore a table, first determine a UNIX timestamp of when the table
   existed (in milliseconds). You can use the Linux `date` command to
   generate the Unix timestamp from a regular timestamp value:

   ```sh
   date -d '2023-08-04 16:00:34.456789Z' +%s000
   ```
3. Then, use the `bq copy` command with the
   `@<time>` time travel decorator to perform the table copy operation.

   For example, enter the following command to copy
   the `mydataset.mytable` table at the time `1418864998000` into a new table
   `mydataset.newtable`.

   ```sh
   bq cp mydataset.mytable@1418864998000 mydataset.newtable
   ```

   (Optional) Supply the `--location` flag and set the value to your
   [location](https://docs.cloud.google.com/bigquery/docs/locations).

   You can also specify a relative offset. The following example copies the
   version of a table from one hour ago:

   ```sh
   bq cp mydataset.mytable@-3600000 mydataset.newtable
   ```

   > [!NOTE]
   > **Note:** If you attempt to recover data prior to the time travel window or from a time before the table was created, you'll receive an `Invalid time travel timestamp` error. For more information, see [Troubleshoot table recovery](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables#troubleshoot_table_recovery).

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

    // deleteAndUndeleteTable demonstrates how to recover a deleted table by copying it from a point in time
    // that predates the deletion event.
    func deleteAndUndeleteTable(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	ds := client.Dataset(datasetID)
    	if _, err := ds.Table(tableID).Metadata(ctx); err != nil {
    		return err
    	}
    	// Record the current time.  We'll use this as the snapshot time
    	// for recovering the table.
    	snapTime := time.Now()

    	// "Accidentally" delete the table.
    	if err := client.Dataset(datasetID).Table(tableID).Delete(ctx); err != nil {
    		return err
    	}

    	// Construct the restore-from tableID using a snapshot decorator.
    	snapshotTableID := fmt.Sprintf("%s@%d", tableID, snapTime.UnixNano()/1e6)
    	// Choose a new table ID for the recovered table data.
    	recoverTableID := fmt.Sprintf("%s_recovered", tableID)

    	// Construct and run a copy job.
    	copier := ds.Table(recoverTableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_CopierFrom(ds.Table(snapshotTableID))
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

    	ds.Table(recoverTableID).Delete(ctx)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to undeleting a table
    public class UndeleteTable {

      public static void runUndeleteTable() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_TABLE";
        String recoverTableName = "MY_RECOVER_TABLE_TABLE";
        undeleteTable(datasetName, tableName, recoverTableName);
      }

      public static void undeleteTable(String datasetName, String tableName, String recoverTableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // "Accidentally" delete the table.
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____(TableId.of(datasetName, tableName));

          // Record the current time.  We'll use this as the snapshot time
          // for recovering the table.
          long snapTime = System.currentTimeMillis();

          // Construct the restore-from tableID using a snapshot decorator.
          String snapshotTableId = String.format("%s@%d", tableName, snapTime);

          // Construct and run a copy job.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html.newBuilder(
                      // Choose a new table ID for the recovered table data.
                      https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, recoverTableName),
                      https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, snapshotTableId))
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__() && job.getStatus().getError() == null) {
            System.out.println("Undelete table recovered successfully.");
          } else {
            System.out.println(
                "BigQuery was unable to copy the table due to an error: \n"
                    + job.getStatus().getError());
            return;
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Table not found. \n" + e.toString());
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

    async function undeleteTable() {
      // Undeletes "my_table_to_undelete" from "my_dataset".

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";
      // const tableId = "my_table_to_undelete";
      // const recoveredTableId = "my_recovered_table";

      /**
       * TODO(developer): Choose an appropriate snapshot point as epoch milliseconds.
       * For this example, we choose the current time as we're about to delete the
       * table immediately afterwards.
       */
      const snapshotEpoch = Date.now();

      // Delete the table
      await bigquery
        .dataset(datasetId)
        .table(tableId)
        .delete();

      console.log(`Table ${tableId} deleted.`);

      // Construct the restore-from table ID using a snapshot decorator.
      const snapshotTableId = `${tableId}@${snapshotEpoch}`;

      // Construct and run a copy job.
      await bigquery
        .dataset(datasetId)
        .table(snapshotTableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(bigquery.dataset(datasetId).table(recoveredTableId));

      console.log(
        `Copied data from deleted table ${tableId} to ${recoveredTableId}`
      );
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

    import time

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Choose a table to recover.
    # table_id = "your-project.your_dataset.your_table"

    # TODO(developer): Choose a new table ID for the recovered table data.
    # recovered_table_id = "your-project.your_dataset.your_table_recovered"

    # TODO(developer): Choose an appropriate snapshot point as epoch
    # milliseconds. For this example, we choose the current time as we're about
    # to delete the table immediately afterwards.
    snapshot_epoch = int(time.time() * 1000)

    # ...

    # "Accidentally" delete the table.
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_delete_table(table_id)  # Make an API request.

    # Construct the restore-from table ID using a snapshot decorator.
    snapshot_table_id = "{}@{}".format(table_id, snapshot_epoch)

    # Construct and run a copy job.
    job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_copy_table(
        snapshot_table_id,
        recovered_table_id,
        # Must match the source and destination tables location.
        location="US",
    )  # Make an API request.

    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()  # Wait for the job to complete.

    print(
        "Copied data from deleted table {} to {}".format(table_id, recovered_table_id)
    )

<br />

If you anticipate that you might want to restore a table later than what is
allowed by the time travel window, then create a table snapshot of the table.
For more information, see
[Introduction to table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).

You cannot restore a logical view directly. For more information, see [Restore a
view](https://docs.cloud.google.com/bigquery/docs/managing-views#restore_a_view).

### Identify when a table was deleted

Use the following filter in Logs Explorer in the Google Cloud console to identify
the audit entry that shows the expiration or deletion for a specified table:

    resource.type="bigquery_resource"
    protoPayload.resourceName="projects/PROJECT_ID/datasets/DATASET_ID/tables/TABLE_ID"
    (protoPayload.methodName="google.cloud.bigquery.v2.TableService.DeleteTable" OR protoPayload.methodName="tableservice.delete" OR protoPayload.serviceData.jobCompletedEvent.job.jobConfiguration.query.statementType="DROP_TABLE" OR protoPayload.methodName="InternalTableExpired")

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contained the table.
- `TABLE_ID`: the ID of the deleted table.

Alternatively, use the following filter to find the expiration or deletion
for the dataset that contained the table:

    resource.type="bigquery_dataset"
    protoPayload.resourceName="projects/PROJECT_ID/datasets/DATASET_ID"
    (protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.DeleteDataset" OR protoPayload.methodName="datasetservice.delete")

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contained the table.

## Identify the cause of table deletion

You can use the
[`INFORMATION_SCHEMA.TABLE_STORAGE`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage)
view to determine how a table was deleted.

The `INFORMATION_SCHEMA.TABLE_STORAGE` view contains information about current
tables and tables deleted within the time travel window. If a table was deleted,
the `table_deletion_time` column contains the deletion timestamp, and the
`table_deletion_reason` column contains the deletion method.

To determine the reason a table was deleted, query the
`INFORMATION_SCHEMA.TABLE_STORAGE` view:

```sql
SELECT
  table_name,
  deleted,
  table_deletion_time,
  table_deletion_reason
FROM
  `PROJECT_ID`.`region-REGION`.INFORMATION_SCHEMA.TABLE_STORAGE
WHERE
  table_schema = "DATASET_ID"
  AND table_name = "TABLE_ID"
```

Replace the following variables:

- `PROJECT_ID`: your project ID.
- `REGION`: the region of the dataset that contained the table.
- `DATASET_ID`: the ID of the dataset that contained the table.
- `TABLE_ID`: the ID of the deleted table.

The `table_deletion_reason` column explains why the table was deleted:

- `TABLE_EXPIRATION`: The table was deleted after the set expiration time.
- `DATASET_DELETION`: The dataset the table belonged to was deleted by a user.
- `USER_DELETED`: The table was deleted by a user.

## Troubleshoot table recovery

### Querying the deleted table using a timestamp in the past

You cannot restore table data by querying a deleted table in the past using a
timestamp decorator or by using
[`FOR SYSTEM_TIME AS OF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of)
to save the result in a destination table. Using either of these methods
generates the following error:

```
Not found: Table myproject:mydataset.table was not found in location LOCATION
```

Instead, to copy the table, follow the steps in [Restore a table](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables#restore_a_table).

### Error: `VPC Service Controls: Request is prohibited by organization's policy`

When you attempt to run the copy command from Google Cloud Shell you may
encounter an error like the following:

```
BigQuery error in cp operation: VPC Service Controls: Request is prohibited by organization's policy
```

Using Cloud Shell from the Google Cloud console with VPC SC is
[not supported](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#shell),
because it gets treated as a request outside of the service perimeters and
access to data that VPC Service Controls protects is denied. To work around this issue,
launch and [connect to Cloud Shell locally](https://docs.cloud.google.com/shell/docs/launching-cloud-shell#launch_and_connect_locally_to_with_the) with the Google Cloud CLI.

### Error: `Latest categories are incompatible with schema`

If you run the copy command from Google Cloud Shell, you may receive an error
like the following:

```
Latest categories are incompatible with schema at TIMESTAMP
```

There are several possible causes for this error:

- The destination table schema is different from the schema of the original table (extra columns are allowed as long as they don't have any [column-level policy tags](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) attached).
- The destination table [column-level policy tags](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) are configured differently from the source table.

To resolve this error:

1. Ensure that the schema of the destination table is identical, and ensure that none of the columns in the original table are missing from the destination table.
2. Remove any column-level policy tags from the destination table that aren't in the original table's schema.

### Error: `BigQuery error in cp operation: Invalid time travel timestamp`

If you run the `bq copy` command from Google Cloud Shell, you may receive an
error like the following:

```
BigQuery error in cp operation: Invalid time travel timestamp 1744343690000 for
table PROJECT_ID:DATASET_ID.TABLE_ID@1744343690000.
Cannot read before 1744843691075
```

This error indicates that you are trying to recover data from the table state
prior to the time travel window or before the table was created. This is
not supported. The error message contains the latest timestamp that can be
used to read the table data. Use the timestamp in the error in the `bq copy`
command.

This error can also occur when you provide a negative timestamp value, for
example, `TABLE@-1744963620000`. Instead, use a time-offset that can be used
with the `-` sign.

```
BigQuery error in cp operation: Invalid time travel timestamp 584878816 for
table PROJECT_ID:DATASET_ID.TABLE_ID@584878816.
Cannot read before 1744843691075
```

This error message indicates that the `bq cp` command contains a negative
timestamp value as an offset, and that you attempted to read the table at
`CURRENT_TIMESTAMP - PROVIDED TIMESTAMP`. This value is normally a timestamp
in 1970. To work around this issue, verify the offset or timestamp values when
you set the table decorator value and use the `-` sign appropriately.

### Materialized views

You can't restore a deleted materialized view directly. If you delete a
materialized view, you must
[recreate it](https://docs.cloud.google.com/bigquery/docs/materialized-views-create).

If you delete a table that is a base table for a materialized view,
the materialized view can no longer be queried or refreshed. If you restore
the base table by following the steps in
[Restore a table](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables#restore_a_table),
you must also [recreate](https://docs.cloud.google.com/bigquery/docs/materialized-views-create)
any materialized views that use that table.

### External tables

You can't restore a deleted external table directly. If you delete an
external table, you must
[recreate it](https://docs.cloud.google.com/bigquery/docs/external-data-sources#external_tables).
The recreation process requires knowing the original table's definition, most
importantly the following:

- The schema of the table
- The source URI(s) pointing to the external data
- The format of the external data

You can obtain this information from [Cloud Logging](https://cloud.google.com/logging)
by looking for the table creation log entry.
You can also try to get the URI(s) by querying the
[`INFORMATION_SCHEMA.TABLE_OPTIONS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-options)
if the table was just deleted.

Deleting the external table doesn't delete the underlying data.

## What's next

- Learn how to [create and use tables](https://docs.cloud.google.com/bigquery/docs/tables).
- Learn how to [manage tables](https://docs.cloud.google.com/bigquery/docs/managing-tables).
- Learn how to [modify table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).
- Learn about [working with table data](https://docs.cloud.google.com/bigquery/docs/managing-table-data).