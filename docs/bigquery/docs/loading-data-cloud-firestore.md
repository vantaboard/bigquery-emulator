# Loading data from Firestore exports

BigQuery supports loading data from [Firestore](https://docs.cloud.google.com/firestore)
exports created using the Firestore
[managed import and export service](https://docs.cloud.google.com/firestore/docs/manage-data/export-import).
The managed import and export service exports Firestore documents
into a Cloud Storage bucket. You can then load the exported data into a
BigQuery table.

> [!NOTE]
> **Note:** Not all Firestore exports can be loaded. Reference the BigQuery [limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-firestore#limitations) to create a Firestore export which can be loaded into a BigQuery table.

## Limitations

When you load data into BigQuery from a Firestore
export, note the following restrictions:

- Your dataset must be in the same location as the Cloud Storage bucket containing your export files.
- You can specify only one Cloud Storage URI, and you cannot use a URI wildcard.
- For a Firestore export to load correctly, documents in the export data must share a consistent schema with fewer than 10,000 unique field names.
- You can create a new table to store the data, or you can overwrite an existing table. You cannot append Firestore export data to an existing table.
- Your [export command](https://docs.cloud.google.com/firestore/docs/manage-data/export-import#export_data) must specify a `collection-ids` filter. Data exported without specifying a collection ID filter cannot be loaded into BigQuery.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document.

### Required permissions

To load data into BigQuery, you need IAM permissions to run a load job and load data into BigQuery tables and partitions. If you are loading data from Cloud Storage, you also need IAM permissions to access the bucket that contains your data.

#### Permissions to load data into BigQuery

To load data into a new BigQuery table or partition or to append or overwrite an existing table or partition, you need the following IAM permissions:

- `bigquery.tables.create`
- `bigquery.tables.updateData`
- `bigquery.tables.update`
- `bigquery.jobs.create`

Each of the following predefined IAM roles includes the permissions that you need in order to load data into a BigQuery table or partition:

- `roles/bigquery.dataEditor`
- `roles/bigquery.dataOwner`
- `roles/bigquery.admin` (includes the `bigquery.jobs.create` permission)
- `bigquery.user` (includes the `bigquery.jobs.create` permission)
- `bigquery.jobUser` (includes the `bigquery.jobs.create` permission)

Additionally, if you have the `bigquery.datasets.create` permission, you can create and
update tables using a load job in the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

### Permissions to load data from Cloud Storage


To get the permissions that
you need to load data from a Cloud Storage bucket,

ask your administrator to grant you the
[Storage Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.admin) (`roles/storage.admin`) IAM role on the bucket.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to load data from a Cloud Storage bucket. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to load data from a Cloud Storage bucket:

- `storage.buckets.get`
- `storage.objects.get`
- `storage.objects.list (required if you are using a URI https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards)`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Loading Firestore export service data

You can load data from a Firestore export metadata file by using
the Google Cloud console, [bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool), or
[API](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2).

Sometimes Datastore terminology is used in the Google Cloud console
and the bq command-line tool, but the following procedures are compatible with
Firestore export files. Firestore and Datastore share
an export format.

> [!NOTE]
> **Note:** You can load specific fields by using the [--projection_fields flag](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-firestore#cloud_firestore_options) in the bq command-line tool or by setting the `projectionFields` property in the `load` job configuration.

### Console

1.
   In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then select a dataset.
4. In the **Dataset info** section, click **Create table**.
5. In the **Create table** pane, specify the following details:
   1. In the **Source** section, select **Google Cloud Storage** in the **Create table from** list. Then, do the following:
      1. Select a file from the Cloud Storage bucket, or enter the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri). You cannot include multiple URIs in the Google Cloud console, but [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are supported. The Cloud Storage bucket must be in the same location as the dataset that contains the table you want to create, append, or overwrite.   
         The URI for your Firestore export file must end with `KIND_COLLECTION_ID.export_metadata`. For example, in `default_namespace_kind_Book.export_metadata`, `Book` is the collection ID, and `default_namespace_kind_Book` is the file name generated by Firestore. If the URI doesn't end with `KIND_COLLECTION_ID.export_metadata`, you receive the following error message: **does not contain valid backup metadata. (error code: invalid).**

         > [!NOTE]
         > **Note:** Do not use the file ending in `overall_export_metadata`. This file is not usable by BigQuery.

         ![select source file to create a BigQuery table](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
      2. For **File format** , select **Cloud Datastore Backup**. Firestore and Datastore share the export format.
   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, no action is necessary. The schema is inferred for a Firestore export.
   4. Optional: Specify **Partition and cluster settings** . For more information, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
   5. Click **Advanced options** and do the following:
      - For **Write preference** , leave **Write if empty** selected. This option creates a new table and loads your data into it.
      - If you want to ignore values in a row that are not present in the table's schema, then select **Unknown values**.
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
   6. Click **Create table**.

### bq

Use the [`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load)
command with `source_format` set to `DATASTORE_BACKUP`.
Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations). If you are overwriting
an existing table, add the `--replace` flag.

To load only specific fields, use the [--projection_fields flag](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-firestore#cloud_firestore_options).

    bq --location=LOCATION load \
    --source_format=FORMAT \
    DATASET.TABLE \
    PATH_TO_SOURCE

Replace the following:

- `LOCATION`: your location. The `--location` flag is optional.
- `FORMAT`: `DATASTORE_BACKUP`. Datastore Backup is the correct option for Firestore. Firestore and Datastore share an export format.
- `DATASET`: the dataset that contains the table into which you're loading data.
- `TABLE`: the table into which you're loading data. If the table doesn't exist, it is created.
- `PATH_TO_SOURCE`: the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).

For example, the following command loads the
`gs://mybucket/20180228T1256/default_namespace/kind_Book/default_namespace_kind_Book.export_metadata`
Firestore export file into a table named `book_data`.
`mybucket` and `mydataset` were created in the `US` multi-region location.

    bq --location=US load \
    --source_format=DATASTORE_BACKUP \
    mydataset.book_data \
    gs://mybucket/20180228T1256/default_namespace/kind_Book/default_namespace_kind_Book.export_metadata

### API

Set the following properties to load Firestore export data
using the [API](https://docs.cloud.google.com/bigquery/docs/reference/v2).

1. Create a `load` job configuration that points to the source data in
   Cloud Storage.

2. Specify your [location](https://docs.cloud.google.com/bigquery/docs/locations) in the `location`
   property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `sourceUris` must be fully qualified, in the format
   `gs://BUCKET/OBJECT` in the
   load job configuration. The file (object) name
   must end in `KIND_NAME.export_metadata`. Only one URI
   is allowed for Firestore exports, and you cannot use a wildcard.

4. Specify the data format by setting the `sourceFormat` property to
   `DATASTORE_BACKUP` in the load job configuration. Datastore Backup
   is the correct option for Firestore. Firestore and
   Datastore share an export format.

5. To load only specific fields, set the `projectionFields` property.

6. If you are overwriting an existing table, specify the write disposition
   by setting the `writeDisposition` property to `WRITE_TRUNCATE`.

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

    # TODO(developer): Set table_id to the ID of the table to create.
    table_id = "your-project.your_dataset.your_table_name"

    # TODO(developer): Set uri to the path of the kind export metadata
    uri = (
        "gs://cloud-samples-data/bigquery/us-states"
        "/2021-07-02T16:04:48_70344/all_namespaces/kind_us-states"
        "/all_namespaces_kind_us-states.export_metadata"
    )

    # TODO(developer): Set projection_fields to a list of document properties
    #                  to import. Leave unset or set to `None` for all fields.
    projection_fields = ["name", "post_abbr"]

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.DATASTORE_BACKUP,
        projection_fields=projection_fields,
    )

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

> [!NOTE]
> **Note:** If you prefer to skip the loading process, you can query the export directly by setting it up as an external data source. For more information, see [External data sources](https://docs.cloud.google.com/bigquery/external-data-sources).

## Firestore options

To change how BigQuery parses Firestore export
data, specify the following option:

| Google Cloud console option | \`bq\` flag | BigQuery API property | Description |
|---|---|---|---|
| Not available | `--projection_fields` | `projectionFields` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatastoreBackupOptions.Builder#com_google_cloud_bigquery_DatastoreBackupOptions_Builder_setProjectionFields_java_util_List_java_lang_String__), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_projection_fields)) | (Optional) A comma-separated list that indicates which document fields to load from a Firestore export. By default, BigQuery loads all fields. Field names are case-sensitive and must be present in the export. You cannot specify field paths within a map field such as `map.foo`. |

## Data type conversion

BigQuery converts data from each document in
Firestore export files to BigQuery
[data types](https://docs.cloud.google.com/bigquery/docs/schemas#standard_sql_data_types).
The following table describes the conversion between supported data types.

| Firestore data type | BigQuery data type |
|---|---|
| Array | RECORD |
| Boolean | BOOLEAN |
| Reference | RECORD |
| Date and time | TIMESTAMP |
| Map | RECORD |
| Floating-point number | FLOAT |
| Geographical point | RECORD ``` [{"lat","FLOAT"}, {"long","FLOAT"}] ``` |
| Integer | INTEGER |
| String | STRING (truncated to 64 KB) |

## Firestore key properties

Each document in Firestore has a unique key that contains
information such as the document ID and the document path.
BigQuery creates a `RECORD` data type (also known as a
[`STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type))
for the key, with nested fields for each piece of information, as described in
the following table.

| Key property | Description | BigQuery data type |
|---|---|---|
| `__key__.app` | The Firestore app name. | STRING |
| `__key__.id` | The document's ID, or `null` if `__key__.name` is set. | INTEGER |
| `__key__.kind` | The document's collection ID. | STRING |
| `__key__.name` | The document's name, or `null` if `__key__.id` is set. | STRING |
| `__key__.namespace` | Firestore does not support custom namespaces. The default namespace is represented by an empty string. | STRING |
| `__key__.path` | The path of the document: the sequence of the document and the collection pairs from the root collection. For example: `"Country", "USA", "PostalCode", 10011, "Route", 1234`. | STRING |