# Loading data from Datastore exports

BigQuery supports loading data from [Datastore](https://docs.cloud.google.com/datastore)
exports created using the Datastore managed import and export
service. You can use the managed import and export service to export
Datastore entities into a Cloud Storage bucket. You can then load
the export into BigQuery as a table.

To learn how to create a Datastore export file, see
[Exporting and importing entities](https://docs.cloud.google.com/datastore/docs/export-import-entities)
in the Datastore documentation. For information on
scheduling exports, see [Scheduling an export](https://docs.cloud.google.com/datastore/docs/schedule-export).

> [!NOTE]
> **Note:** If you intend to load a Datastore export into BigQuery, you must specify an entity filter in your [export command](https://docs.cloud.google.com/datastore/docs/export-import-entities#exporting_entities). Data exported without specifying an entity filter cannot be loaded into BigQuery.

You can control which properties BigQuery should load by setting
the [`projectionFields` property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.projection_fields)
in the API or by using the `--projection_fields` flag in the bq command-line tool.

If you prefer to skip the loading process, you can query the export directly by
setting it up as an external data source. For more information, see
[External data sources](https://docs.cloud.google.com/bigquery/external-data-sources).

When you load data from Cloud Storage into a BigQuery table,
the dataset that contains the table must be in the same region or multi-region
as the Cloud Storage bucket.

## Limitations

When you load data into BigQuery from a Datastore
export, note the following restrictions:

- You cannot use a wildcard in the Cloud Storage URI when you specify a Datastore export file.
- You can specify only one Cloud Storage URI when loading data from Datastore exports.
- You cannot append Datastore export data to an existing table with a defined schema.
- For a Datastore export to load correctly, entities in the export data must share a consistent schema with fewer than 10,000 unique property names.
- Data exported without specifying an entity filter cannot be loaded into BigQuery. The export request must include one or more kind names in the entity filter.
- The maximum field size for Datastore exports is 64 KB. When you load a Datastore export, any field larger than 64 KB is truncated.

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

## Loading Datastore export service data

To load data from a Datastore export metadata file:

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
         The URI for your Datastore export file must end with `KIND_NAME.export_metadata` or `export[NUM].export_metadata`. For example, in `default_namespace_kind_Book.export_metadata`, `Book` is the kind name, and `default_namespace_kind_Book` is the filename generated by Datastore. ![select source file to create a BigQuery table](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
      2. For **File format** , select **Cloud Datastore Backup**.
   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, no action is necessary. The schema is inferred for a Datastore export.
   4. Optional: Specify **Partition and cluster settings** . For more information, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
   5. Click **Advanced options** and do the following:
      - For **Write preference** , leave **Write if empty** selected. This option creates a new table and loads your data into it.
      - If you want to ignore values in a row that are not present in the table's schema, then select **Unknown values**.
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
      For information about the available options, see [Datastore options](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-datastore#cloud_datastore_options).
   6. Click **Create table**.

### bq

Use the `bq load` command with `source_format` set to `DATASTORE_BACKUP`.
Supply the `--location` flag and set the value to your [location](https://docs.cloud.google.com/bigquery/docs/locations).

    bq --location=LOCATION load \
    --source_format=FORMAT \
    DATASET.TABLE \
    PATH_TO_SOURCE

Replace the following:

- `LOCATION`: your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location by using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: `DATASTORE_BACKUP`.
- `DATASET`: the dataset that contains the table into which you're loading data.
- `TABLE`: the table into which you're loading data. If the table does not exist, it is created.
- `PATH_TO_SOURCE`: the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).

For example, the following command loads the
`gs://mybucket/20180228T1256/default_namespace/kind_Book/default_namespace_kind_Book.export_metadata`
Datastore export file into a table named `book_data`.
`mybucket` and `mydataset` were created in the `US` multi-region location.

    bq --location=US load \
    --source_format=DATASTORE_BACKUP \
    mydataset.book_data \
    gs://mybucket/20180228T1256/default_namespace/kind_Book/default_namespace_kind_Book.export_metadata

### API

Set the following properties to load Datastore export data
using the [API](https://docs.cloud.google.com/bigquery/docs/reference/v2).

1. Create a [load job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationload)
   that points to the source data in Cloud Storage.

2. Specify your [location](https://docs.cloud.google.com/bigquery/docs/locations) in the `location`
   property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The [source URIs](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.source_uris)
   must be fully qualified, in the format gs://\[BUCKET\]/\[OBJECT\]. The file
   (object) name must end in `[KIND_NAME].export_metadata`. Only one
   URI is allowed for Datastore exports, and you cannot use a
   wildcard.

4. Specify the data format by setting the
   [`JobConfigurationLoad.sourceFormat` property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.source_format)
   to `DATASTORE_BACKUP`.

## Appending to or overwriting a table with Datastore data

When you load Datastore export data into BigQuery, you
can create a new table to store the data, or you can overwrite an existing
table. You cannot append Datastore export data to an existing
table.

If you attempt to append Datastore export data to an existing
table, the following error results: `Cannot append a datastore backup to a table
that already has a schema. Try using the WRITE_TRUNCATE write disposition to
replace the existing table`.

To overwrite an existing table with Datastore export data:


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
         The URI for your Datastore export file must end with `KIND_NAME.export_metadata` or `export[NUM].export_metadata`. For example, in `default_namespace_kind_Book.export_metadata`, `Book` is the kind name, and `default_namespace_kind_Book` is the filename generated by Datastore. ![select source file to create a BigQuery table](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
      2. For **File format** , select **Cloud Datastore Backup**.

   > [!NOTE]
   > **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information about supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, no action is necessary. The schema is inferred for a Datastore export.
   **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information about supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).
   4. Optional: Specify **Partition and cluster settings** . For more information, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables). You cannot convert a table to a partitioned or clustered table by appending or overwriting it. The Google Cloud console does not support appending to or overwriting partitioned or clustered tables in a load job.
   5. Click **Advanced options** and do the following:
      - For **Write preference** , choose **Append to table** or **Overwrite
        table**.
      - If you want to ignore values in a row that are not present in the table's schema, then select **Unknown values**.
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
      For information about the available options, see [Datastore options](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-datastore#cloud_datastore_options).
   6. Click **Create table**.

### bq

Use the `bq load` command with the `--replace` flag and with `source_format`
set to `DATASTORE_BACKUP`. Supply the `--location` flag and set the value to
your [location](https://docs.cloud.google.com/bigquery/docs/locations).

    bq --location=LOCATION load \
    --source_format=FORMAT \
    --replace \
    DATASET.TABLE \
    PATH_TO_SOURCE

Replace the following:

- `LOCATION`: your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location by using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: `DATASTORE_BACKUP`.
- `DATASET`: the dataset containing the table into which you're loading data.
- `TABLE`: the table you're overwriting.
- `PATH_TO_SOURCE`: the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).

For example, the following command loads the
`gs://mybucket/20180228T1256/default_namespace/kind_Book/default_namespace_kind_Book.export_metadata`
Datastore export file and overwrites a table named
`book_data`:

    bq load --source_format=DATASTORE_BACKUP \
    --replace \
    mydataset.book_data \
    gs://mybucket/20180228T1256/default_namespace/kind_Book/default_namespace_kind_Book.export_metadata

### API

Set the following properties to load data from the [API](https://docs.cloud.google.com/bigquery/docs/reference/v2).

1. Create a [load job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationload)
   that points to the source data in Cloud Storage.

2. Specify your [location](https://docs.cloud.google.com/bigquery/docs/locations) in the `location`
   property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The [source URIs](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.source_uris)
   must be fully qualified, in the format gs://\[BUCKET\]/\[OBJECT\]. The file
   (object) name must end in `[KIND_NAME].export_metadata`. Only one
   URI is allowed for Datastore exports, and you cannot use a
   wildcard.

4. Specify the data format by setting the
   [`JobConfigurationLoad.sourceFormat` property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.source_format)
   to `DATASTORE_BACKUP`.

5. Specify the write disposition by setting the
   [`JobConfigurationLoad.writeDisposition` property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.write_disposition)
   to `WRITE_TRUNCATE`.

## Datastore options

To change how BigQuery parses Datastore export
data, specify the following option:

| Console option | bq tool flag | BigQuery API property | Description |
|---|---|---|---|
| Not available | `--projection_fields` | [projectionFields](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.projection_fields) | A comma-separated list that indicates which entity properties to load into BigQuery from a Datastore export. Property names are case-sensitive and must be top-level properties. If no properties are specified, BigQuery loads all properties. If any named property isn't found in the Datastore export, an invalid error is returned in the job result. The default value is ''. |

## Data type conversion

BigQuery converts data from each entity in
Datastore export files to BigQuery
[data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types).
The following table describes the conversion between data types.

| Datastore data type | BigQuery data type |
|---|---|
| Array | `ARRAY` |
| Blob | `BYTES` |
| Boolean | `BOOLEAN` |
| Date and time | `TIMESTAMP` |
| Embedded entity | `RECORD` |
| Floating-point number | `FLOAT` |
| Geographical point | `RECORD` ``` [{"lat","DOUBLE"}, {"long","DOUBLE"}] ``` |
| Integer | `INTEGER` |
| Key | `RECORD` |
| Null | `STRING` |
| Text string | `STRING` (truncated to 64 KB) |

## Datastore key properties

Each entity in Datastore has a unique key that contains
information such as the namespace and the path. BigQuery creates
a `RECORD` data type for the key, with nested fields for each piece of
information, as described in the following table.

| Key property | Description | BigQuery data type |
|---|---|---|
| `__key__.app` | The Datastore app name. | STRING |
| `__key__.id` | The entity's ID, or `null` if `__key__.name` is set. | INTEGER |
| `__key__.kind` | The entity's kind. | STRING |
| `__key__.name` | The entity's name, or `null` if `__key__.id` is set. | STRING |
| `__key__.namespace` | If the Datastore app uses a custom namespace, the entity's namespace. Else, the default namespace is represented by an empty string. | STRING |
| `__key__.path` | The flattened [ancestral path of the entity](https://docs.cloud.google.com/appengine/docs/java/datastore/entities#Java_Ancestor_paths), consisting of the sequence of kind-identifier pairs from the root entity to the entity itself. For example: `"Country", "USA", "PostalCode", 10011, "Route", 1234`. | STRING |