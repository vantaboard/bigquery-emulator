# Discover and catalog Cloud Storage data

This document explains how to use Knowledge Catalog automatic discovery, a
feature in BigQuery that lets you scan data in Cloud Storage
buckets to extract and then catalog metadata. As part of the discovery scan,
automatic discovery creates BigLake or external tables for
structured data and object tables for unstructured data. This centralized table
data eases AI-powered data insights, data security, and governance.

To use automatic discovery of Cloud Storage data, you create and run a
discovery scan.

Automatic discovery is also called standalone discovery.

## Discovery scan overview

A discovery scan does the following:

- Scans the data in the Cloud Storage bucket or path.
- Groups structured and semi-structured data into tables.
- Collects metadata, such as the table name, schema, and partition definition.
- Creates and updates [BigLake external](https://docs.cloud.google.com/bigquery/docs/biglake-intro), [non-BigLake external](https://docs.cloud.google.com/bigquery/docs/external-tables), or [BigLake object](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) tables in BigQuery using the schema and partition definition.

For unstructured data, such as images and videos, the discovery scan detects and
registers groups of files that share the same data file format. Files must be
located in folders that contain the same file format. For example,
`gs://images/group1` must only contain GIF images, and `gs://images/group2` must
only contain JPEG images for the discovery scan to detect and register two
BigLake object tables.

For structured data, such as Avro, the discovery scan registers groups of files
as BigLake external tables and detects files only if they're
located in folders that contain the same data format and compatible schema.

The discovery scan supports the following formats:

**Structured and semi-structured**

- Parquet
- Avro
- ORC
- JSON (only the [newline-delimited format](https://github.com/ndjson/ndjson-spec))
- CSV (but not CSV files that have comment rows)

[**Unstructured**](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#supported_object_files)

- Image (such as JPEG, PNG, and BMP)
- Documents (such as PDF, slide presentations, and text reports)
- Audio or video (such as WAV, MP3, and MP4)

> [!NOTE]
> **Note:** Apache Iceberg and Delta Lake table formats aren't supported by the discovery scan.

The discovery scan supports the following compression formats:

**Structured and semi-structured data**

- Internal compression for the following formats:

  | Compression | File extension sample | Supported format |
  |---|---|---|
  | gzip | `.gz.parquet` | Parquet |
  | lz4 | `.lz4.parquet` | Parquet |
  | Snappy | `.snappy.parquet` | Parquet, ORC, Avro |
  | lzo | `.lzo.parquet` | Parquet, ORC |

- External compression for JSON and CSV files:

  - gzip
  - bzip2

**Unstructured data**

For object tables, compression is managed primarily through
[Cloud Storage object metadata](https://docs.cloud.google.com/storage/docs/metadata), rather than
BigQuery internal settings.

- Standard metadata compression: BigQuery automatically recognizes files compressed with gzip and bzip2 if they use the standard .gz or .bz2 extensions.
- Content-Encoding: you can use the [Content-Encoding gzip](https://docs.cloud.google.com/storage/docs/metadata#content-encoding) metadata in Cloud Storage to serve compressed files while maintaining their original content-type.
- Media-internal compression: formats that are inherently compressed (such as JPEG for images, MP3 for audio, MP4 for video) are natively supported.

To see the limit of how many tables a discovery scan supports,
see [Quotas and limits](https://docs.cloud.google.com/bigquery/quotas#dataplex-discovery).

The discovered tables are registered in BigQuery as
BigLake external tables, BigLake object tables,
or external tables. This makes their data available for analysis in
BigQuery. Metadata caching for BigLake tables and
object tables is also enabled. All the BigLake tables are
automatically ingested into Knowledge Catalog for search and
discovery.

## Before you begin


Enable the Dataplex API.


**Roles required to enable APIs**


To enable APIs, you need the Service Usage Admin IAM
role (`roles/serviceusage.serviceUsageAdmin`), which
contains the `serviceusage.services.enable` permission. [Learn how to grant
roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

[Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=dataplex.googleapis.com)

> [!NOTE]
> **Note:** The roles listed in the next section are for a standard discovery scan. If you want to use unstructured data semantic inference to extract AI-powered insights from your files, you must also apply the additional roles listed in [Use data insights for unstructured data](https://docs.cloud.google.com/dataplex/docs/use-data-insights-unstructured-data#roles-permissions).

### Required roles for the Knowledge Catalog service account

Before you begin, assign the IAM permissions to the
Knowledge Catalog service account in your project.

<br />

```
  service-PROJECT_NUMBER@gcp-sa-dataplex.iam.gserviceaccount.com
  
```

<br />

Replace `PROJECT_NUMBER` with the project in which the
Dataplex API is enabled.


To ensure that the Dataplex service account has the necessary
permissions to create and run a discovery scan,

ask your administrator to grant the
following IAM roles to the Dataplex service account:

**Important:** You must grant these roles to the Dataplex service account, *not* to your user account. Failure to grant the roles to the correct principal might result in permission errors.

- [Dataplex Discovery Service Agent](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.discoveryServiceAgent) (`roles/dataplex.discoveryServiceAgent`) on the storage bucket
- [Dataplex Discovery Publishing Service Agent](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.discoveryPublishingServiceAgent) (`roles/dataplex.discoveryPublishingServiceAgent`) on the user project
- Create BigLake tables: [Dataplex Discovery BigLake Publishing Service Agent](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.discoveryBigLakePublishingServiceAgent) (`roles/dataplex.discoveryBigLakePublishingServiceAgent`) on the BigQuery connection


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to create and run a discovery scan. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create and run a discovery scan:

- `bigquery.datasets.create` on the data source project
- `storage.buckets.get` on the data source bucket
- `storage.objects.get` on the data source bucket
- `storage.objects.list` on the data source bucket
- `bigquery.datasets.get` on the data source project
- Provide a connection:
  - `bigquery.connections.delegate` on the BigQuery connection
  - `bigquery.connections.use` on the BigQuery connection


Your administrator might also be able to give the Dataplex service account
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Required roles for the BigQuery connection service account


To ensure that the BigQuery Connection Service account has the necessary
permissions to create a discovery scan,

ask your administrator to grant the
[Dataplex Discovery Service Agent](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.discoveryServiceAgent) (`roles/dataplex.discoveryServiceAgent`) IAM role to the BigQuery Connection Service account on the Cloud Storage bucket.


**Important:** You must grant this role to the BigQuery Connection Service account, *not* to your user account. Failure to grant the role to the correct principal might result in permission errors. For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

<br />


This predefined role contains

the permissions required to create a discovery scan. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create a discovery scan:

- `bigquery.datasets.create` on the data source project
- `storage.buckets.get` on the data source bucket
- `storage.objects.get` on the data source bucket
- `storage.objects.list` on the data source bucket
- `bigquery.datasets.get` on the data source project
- Provide a connection:
  - `bigquery.connections.delegate` on the BigQuery connection
  - `bigquery.connections.use` on the BigQuery connection


Your administrator might also be able to give the BigQuery Connection Service account
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Required roles for end users


To get the permissions that
you need to create and manage data discovery scans,

ask your administrator to grant you the
following IAM roles on the Cloud Storage bucket:

- Full access to DataScan resources: Dataplex DataScan Administrator (`roles/dataplex.dataScanAdmin`) - your project
- Write access to DataScan resources: Dataplex DataScan Editor (`roles/dataplex.dataScanEditor`) - your project
- Read access to DataScan resources, excluding the results: Dataplex DataScan Viewer (`roles/dataplex.dataScanViewer`) - your project
- Read access to DataScan resources, including the results: Dataplex DataScan DataViewer (`roles/dataplex.dataScanDataViewer`) - your project


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to create and manage data discovery scans. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create and manage data discovery scans:

- Create a DataScan: `dataplex.datascans.create` on your project
- Delete a DataScan: `dataplex.datascans.delete` on your project or a DataScan resource
- View DataScan details excluding results: `dataplex.datascans.get` on your projector a DataScan resource
- View DataScan details including results: `dataplex.datascans.getData` on your project or a DataScan resource
- List DataScans: `dataplex.datascans.list` on your project or a DataScan resource
- Run a DataScan: `dataplex.datascans.run` on your project or a DataScan resource
- Update the description of a DataScan: `dataplex.datascans.update` on your projector a DataScan resource
- View the IAM permissions of the DataScan: `dataplex.datascans.getIamPolicy` on your project or a DataScan resource
- Set the IAM permissions on the DataScan: `dataplex.datascans.setIamPolicy` on your project or a DataScan resource


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create a discovery scan

To discover data, you must create and run a discovery scan. You can set a
schedule for the scan or run the scan on demand.

When the discovery scan runs, it creates a new dataset in
BigQuery that corresponds to the Cloud Storage
bucket that was scanned. The BigQuery dataset name is the same as
the Cloud Storage bucket name. Invalid characters in the bucket name
are replaced by an underscore. If the dataset name isn't available, a suffix is
appended (for example, `_discovered_001`). The dataset contains the
BigLake external or non-BigLake external tables
that were created by the discovery scan for further analysis.

### Console

1. In the Google Cloud console, go to the **Metadata curation** page.


   [Go to Metadata curation](https://console.cloud.google.com/bigquery/governance/metadata-curation/cloud-storage-discovery)

   <br />

2. In the **Cloud Storage discovery** tab, click **Create**.

3. In the **Create discovery scan** pane, configure the details about the data
   to scan.

4. Enter a name for the scan.

5. In the **Scan ID** field, enter a unique ID that follows the
   [resource naming conventions in Google Cloud](https://docs.cloud.google.com/compute/docs/naming-resources#resource-name-format).
   If you don't provide an ID, the discovery scan generates the scan ID.

6. Optional: Provide a description of the scan.

7. To specify the Cloud Storage bucket that contains the files to
   scan, in the **Bucket** field, browse to and select the bucket.

8. Optional: Define the data to include or exclude from the discovery scan
   by providing a list of
   [glob patterns for file filtering](https://en.wikipedia.org/wiki/Glob_(programming)).

   - **Include**: if only a subset of the data should be scanned, provide a list of glob patterns that match the objects to include.
   - **Exclude**: provide a list of glob patterns that match the objects to exclude.

   For example, if you want to exclude `gs://test_bucket/foo/..` from the
   discovery scan, enter `**/foo/**` as the exclude path. Quotation marks
   cause errors. Make sure to enter `**/foo/**` instead of `"**/foo/**"`.

   If you provide both include patterns and exclude patterns, the exclude
   patterns are applied first.
9. For **Unstructured data options** , select **Enable semantic inference**.

   This option is required if you want to view data insights for unstructured data in Knowledge Catalog. For more information, see [About data insights for unstructured data](https://docs.cloud.google.com/dataplex/docs/data-insights-unstructured-data).
10. Optional: In **Project**, select the BigQuery dataset project
    that contains the BigLake external or non-BigLake
    external tables created by the discovery scan. If not provided,
    the dataset is created in the project that contains the
    Cloud Storage bucket.

11. In **Location type** , select either **Region** or **Multi-region**
    (whichever is available) where the BigQuery publishing dataset
    is created.

12. To create BigLake tables from the scanned data, in the
    **Connection ID** field, provide your Google Cloud resource
    connection ID. For more information, see
    [Google Cloud resource connections in BigQuery](https://docs.cloud.google.com/bigquery/docs/connections-api-intro#cloud-resource-connections).

    You can create a new connection ID in the same location as the
    BigQuery dataset location, which is [compatible with the Cloud Storage bucket location](https://docs.cloud.google.com/bigquery/docs/external-tables#storage-location-considerations).

    If you don't provide a resource connection ID, the discovery scan
    creates [non-BigLake external tables](https://docs.cloud.google.com/bigquery/docs/external-data-sources#non-biglake-tables).
    To understand the differences between these external table types and why
    the discovery service might choose one over the other, see the
    [behavioral differences comparison](https://docs.cloud.google.com/bigquery/docs/external-data-sources#external_data_source_feature_comparison).
13. In the **Discovery frequency** section, configure when you want the
    discovery scan to run:

    - **Repeat**: the scan runs on a predefined schedule. Provide the
      start time, days to run the scan, and the frequency, such as
      hourly.

    - **On demand**: the scan runs on demand.

14. Optional: In the **JSON or CSV specifications** section, specify how
    the scan should process JSON and CSV files. Click **JSON or CSV
    specifications**.

    1. To configure JSON options, select **Enable JSON parsing options** .
       - **Disable type inference**: whether the discovery scan should infer data types when scanning data. If you disable type inference for JSON data, all columns are registered as their primitive types, such as string, number, or boolean.
       - **Encoding format**: the character encoding of the data, such as UTF-8, US-ASCII, or ISO-8859-1. If you don't specify a value, UTF-8 is used as the default.
    2. To configure CSV options, check **Enable CSV parsing options** .
       - **Disable type inference**: whether the discovery scan should infer data types when scanning data. If you disable type inference for CSV data, all columns are registered as strings.
       - **Header rows** : the number of header rows, either `0` or `1`. If you specify the value `0`, the discovery scan infers headings and extracts the column names from the file. The default is `0`.
       - **Column delimiter character** : the character that is used to separate values. Provide a single character, `\r` (carriage return), or `\n` (newline). The default is a comma (`,`).
       - **Encoding format** : the character encoding of the data, such as `UTF-8`, `US-ASCII`, or `ISO-8859-1`. If you don't specify a value, UTF-8 is used as the default.
15. Click **Create** (for a scheduled scan), **Run now** (for an on-demand scan),
    or **Create and run** (for a one-time scan).

    A scheduled scan is run according to the schedule that you set.

    An on-demand scan is run once initially when you create it, and you
    can run the scan at any time. It can take several minutes for the discovery
    scan to run.

    A one-time scan executes automatically, a single time. It's automatically
    deleted when it reaches its defined time to live (TTL) threshold, a value
    that determines the duration a discovery scan remains active after
    execution. The TTL value can range from 0 seconds (immediate deletion) to
    365 days. A discovery scan without a specified TTL is automatically
    deleted after 24 hours.

### gcloud

To create a discovery scan, use the [`gcloud dataplex datascans create data-discovery`](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/create/data-discovery) command.

```
gcloud dataplex datascans create data-discovery --location=LOCATION
--data-source-resource=BUCKET_PATH
```

Replace the following:

- `LOCATION`: the location in which you'd like to create your discovery scan
- `BUCKET_PATH`: the Cloud Storage path of the bucket you want to scan

### REST

To create a discovery scan, use the [`dataScans.create` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/create).

## Query published BigLake tables

After you run the discovery scan, BigLake
tables are published in a new dataset in BigQuery. The tables are
then available for analysis in BigQuery using SQL, or in
Managed Service for Apache Spark using Apache Spark or HiveQL.

### SQL

You can view or query tables in BigQuery. For more information
about how to run queries in BigQuery, see
[Run a query](https://docs.cloud.google.com/bigquery/docs/running-queries).

### Apache Spark

To query BigLake tables using Spark SQL on a
Managed Service for Apache Spark serverless job, follow these steps:

1. Create a PySpark script similar to the following sample script:

   ```python
   from pyspark.sql import SparkSession
   session = (
     SparkSession.builder.appName("testing")
       .config("viewsEnabled","true")
       .config("materializationDataset", "DATASET_ID")
       .config("spark.hive.metastore.bigquery.project.id", "PROJECT_ID")
       .config("spark.hive.metastore.client.factory.class", "com.google.cloud.bigquery.metastore.client.BigQueryMetastoreClientFactory")
       .enableHiveSupport()
       .getOrCreate()
   )

   session.sql("show databases").show()
   session.sql("use TABLE_NAME").show()
   session.sql("show tables").show()

   sql = "SELECT * FROM DATASET_ID.TABLE_ID LIMIT 10"
   df = session.read.format("bigquery").option("dataset", "DATASET_ID").load(sql)
   df.show()
   ```

   Replace the following:
   - `DATASET_ID`: ID of dataset for which users have create permission
   - `PROJECT_ID`: ID of project with BigLake table
   - `TABLE_NAME`: Name of BigLake table
   - `TABLE_ID`: ID of BigLake table
2. [Submit the batch job](https://docs.cloud.google.com/dataproc-serverless/docs/quickstarts/spark-batch#submit_a_spark_batch_workload).

## Manage published BigLake tables

Published BigLake tables are created and managed in
BigQuery by the discovery scan. By default, the discovery scan
handles new data discovery, schema inferences, and schema evolution every time
the scheduled or on-demand scans are run. To indicate that metadata is managed
by the scan, the scan publishes tables with the label `metadata-managed-mode`
set to `discovery-managed`.

If you want to manage the schema and other metadata like CSV or JSON options
yourself, set the `metadata-managed-mode` label to `user_managed`. This way, the
schema remains unchanged when the next discovery scan runs. This approach can
be useful in scenarios where the schema that's inferred by the discovery scan is
incorrect or different from what is expected for a given table. When the
`metadata-managed-mode` label is set to `user_managed`, it can reduce the cost.

To update the label, you can [edit the value of the label key](https://docs.cloud.google.com/bigquery/docs/updating-labels#updating_a_table_or_view_label)
`metadata-managed-mode` to `user_managed` instead of `discovery-managed`. In
this case, the discovery scan doesn't update the schema of the table as long as
the `user_managed` label is attached to the table.

> [!NOTE]
> **Note:** You can apply access policies, such as [column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) and [row-level security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro), even if your tables are managed by the discovery scan.

### Update published BigLake tables

For BigLake tables published using the discovery scan jobs
with the default configuration, the schema and other metadata are automatically
updated with every discovery scan job run at the scheduled frequency.

To update a published BigLake table, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. [Update one or more table properties](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_table_properties).

3. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
4. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

5. Click **Overview \> Tables**, and then select the table.

6. In the **Details** tab, in the **Labels** section, make sure that the
   **metadata-managed-mode** label is set to **user_managed**. If it is set to a
   different value, follow these steps:

   1. Click **Edit details**.

   2. Next to the **metadata-managed-mode** key, in the **value** field,
      enter `user_managed`.

> [!NOTE]
> **Note:** A table with an updated schema becomes available for SQL and Spark queries. When the next discovery scans run, the table metadata remains unchanged.

### Delete published BigLake tables

To delete a published BigLake table, follow these steps:

1. [Delete the data files for the table in the Cloud Storage bucket](https://docs.cloud.google.com/storage/docs/deleting-objects).

2. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

3. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
4. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

5. Click **Overview \> Tables**, and then select the table.

6. In the **Details** pane, in the **Labels** section, make sure the
   **metadata-managed-mode** label isn't set to `user_managed`. If it's set to
   `user_managed`, follow these steps:

   1. Click **Edit details** .

   2. Next to the **metadata-managed-mode** key, in the **value** field,
      enter `discovery-managed`.

      > [!NOTE]
      > **Note:** If the **metadata-managed-mode** label is set to `user_managed`, the discovery scan doesn't override the table metadata, which causes your table not to be deleted.

7. Click **Run**. The discovery scan runs on demand.

After the discovery scan runs, the BigLake table is
deleted in BigQuery and isn't available to list or query
through Spark.

## Run a discovery scan on demand

To run a discovery scan on demand, select one of the following options.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. In the navigation menu, click **Governance \> Metadata curation**.

3. In the **Cloud Storage discovery** pane, click the discovery scan you want
   to run.

4. Click **Run now**.

### gcloud

To run a discovery scan, use the
[`gcloud dataplex datascans run` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/run):

    gcloud dataplex datascans run DATASCAN \
      --location=LOCATION

Replace the following variables:

- `LOCATION`: the Google Cloud region in which the discovery scan was created.
- `DATASCAN`: the name of the discovery scan.

### REST

To run a discovery scan on demand, use the
[`dataScans.run` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/run)
in the Dataplex API.

## List discovery scans

To list your discovery scans, select one of the following options.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. In the navigation menu, click **Governance \> Metadata curation**.

3. In the **Cloud Storage discovery** pane, the discovery scans created in the
   project are listed.

### gcloud

```
gcloud dataplex datascans list --location=LOCATION --project=PROJECT_ID
```

Replace the following:

- `LOCATION`: the location of your project
- `PROJECT_ID`: your Google Cloud project ID

### REST

To retrieve the list of discovery scans in your project, use the
[`dataScans.list` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/list)
in the Dataplex API.

## View a discovery scan

To view a discovery scan, select one of the following options.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. In the navigation menu, click **Governance \> Metadata curation**.

3. In the **Cloud Storage discovery** pane, click the discovery scan you want
   to view details for.

   - The **Scan details** section shows details about the discovery scan.
   - The **Scan status** section shows the discovery results of the latest scan job.

### gcloud

```
gcloud dataplex datascans jobs describe JOB \
    --location=LOCATION \
    --datascan=DATASCAN \
    --view=FULL
```

Replace the following:

- `JOB`: the job ID of the discovery scan job.
- `LOCATION`: the Google Cloud region in which the discovery scan was created.
- `DATASCAN`: the name of the discovery scan the job belongs to.
- `--view=FULL`: see the discovery scan job result.

### REST

To view the results of a data discovery scan, use the
[`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/get)
in the Dataplex API.

### View historical discovery scan results

To view historical discovery scan results, select one of the following options.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. In the navigation menu, click **Governance \> Metadata curation**.

3. In the **Cloud Storage discovery** pane, click the discovery scan you want
   to view details for.

4. Click the **Scan history** pane. The **Scan history** pane provides
   information about past jobs, including the number of records scanned in
   each job, each job's status, and the time the jobs were run.

5. To view detailed information about a job, click the job in the **Job ID**
   column.

### gcloud

```
gcloud dataplex datascans jobs list \
    --location=LOCATION \
    --datascan=DATASCAN
```

Replace the following:

- `LOCATION`: the Google Cloud region in which the discovery scan was created.
- `DATASCAN`: the name of the discovery scan the job belongs to.

### REST

To view all the jobs of a discovery scan, use the
[`dataScans.job/list` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans.jobs/list)
in the Dataplex API.

## Update a discovery scan

To change the schedule of a discovery scan, for example, to change the schedule
from on-demand to recurring, update the discovery scan.

> [!NOTE]
> **Note:** Updating a discovery scan is not supported for one-time discovery scans.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. In the navigation menu, click **Governance \> Metadata curation**.

3. In the **Cloud Storage discovery** pane, for the discovery scan you want to
   update, click **Actions \> Edit**.

4. Edit the values.

5. Click **Save**.

### gcloud

To update a discovery scan, use the [`gcloud dataplex datascans update data-discovery`](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/update/data-discovery) command.

```
gcloud dataplex datascans update data-discovery SCAN_ID --location=LOCATION --description=DESCRIPTION
```

Replace the following:

- `SCAN_ID`: the ID of the discovery scan you want to update
- `LOCATION`: the Google Cloud region in which the discovery scan was created
- `DESCRIPTION`: the new description for the discovery scan

### REST

To update a discovery scan, use the
[`dataScans.patch` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/patch)
in the Dataplex API.

## Delete a discovery scan

To delete a discovery scan, select one of the following options.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. In the navigation menu, click **Governance \> Metadata curation**.

3. In the **Cloud Storage discovery** pane, for the discovery scan you want to
   delete, click **Actions \> Delete**.

4. Click **Delete**.

### gcloud

```
gcloud dataplex datascans delete SCAN_ID --location=LOCATION --async
```

Replace the following:

- `SCAN_ID`: the ID of the discovery scan you want to delete.
- `LOCATION`: the Google Cloud region in which the discovery scan was created.

### REST

To delete a discovery scan, use the
[`dataScans.delete` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/delete)
in the Dataplex API.