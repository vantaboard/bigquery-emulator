# Teradata to BigQuery migration: Overview

This document provides more information to help you understand the decisions you
need to make when using the BigQuery Data Transfer Service to migrate schema and data from
Teradata to [BigQuery](https://docs.cloud.google.com/bigquery/docs).
For an introduction to the Teradata migration process, see
[Introduction to a Teradata to BigQuery migration](https://docs.cloud.google.com/bigquery/docs/migration/teradata-migration-intro).

Migrating schema and data is typically one of several steps needed to move a
data warehouse from a different platform to BigQuery.
For a description of a general migration process, see
[Overview: Migrate data warehouses to BigQuery](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview).

You can also use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator)
to migrate your SQL scripts in bulk, or
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
to translate ad hoc queries. Teradata SQL is fully supported by both
SQL translation services.

## Overview

You can use the BigQuery Data Transfer Service in combination with a
special migration agent to copy your schema and data from Teradata to
BigQuery. The migration agent connects to your local data
warehouse communicates with the BigQuery Data Transfer Service
to copy tables from your data warehouse to BigQuery.

The following steps describe the workflow for the migration process:

1. Download the migration agent.
2. Configure a transfer in the BigQuery Data Transfer Service.
3. Run the transfer job to copy table schema and data from your data warehouse to BigQuery.
4. Optional. Monitor transfer jobs by using the Google Cloud console.

## Transfer job configuration

You can configure a transfer job to best suit your needs. Before
setting up a data transfer from Teradata to BigQuery, consider the
configuration options described in the following sections and decide what
settings to use. Depending on the settings you
choose, you might need to complete some prerequisites prior to starting
the transfer job.

For most systems, especially those with large tables, you can get the best
performance by following these steps:

1. Partition your Teradata tables.
2. Use [Teradata Parallel Transporter (TPT)](https://docs.teradata.com/r/Teradata-Parallel-Transporter-User-Guide/February-2022/Introduction-to-Teradata-PT/High-Level-Description) for [extraction](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#extraction_method).
3. Create a [custom schema file](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#custom_schema_file) and configure your target BigQuery clustering and partitioning columns.

This enables the migration agent to perform partition-by-partition extraction,
which is the most efficient.

### Extraction method

The BigQuery Data Transfer Service supports two extraction methods for
transferring data from Teradata to BigQuery:

- **Use the
  [Teradata Parallel Transporter (TPT)](https://docs.teradata.com/r/Teradata-Parallel-Transporter-User-Guide/February-2022/Introduction-to-Teradata-PT/High-Level-Description)
  *tbuild* utility**.
  This is the recommended approach. Using TPT typically results in
  faster data extraction.

  In this mode, the migration agent attempts to calculate extraction batches
  using rows distributed by partitions. For each batch, the agent emits
  and executes a TPT extraction
  script, producing a set of pipe delimited files.
  It then uploads these files to a Cloud Storage bucket,
  where they are used by the transfer job. Once the files are uploaded to
  Cloud Storage, the migration agent deleted them from the local file
  system.

  When you use TPT extraction **without** a partitioning column, your
  whole table is extracted. When you use TPT extraction **with** a
  partitioning column, the agent extracts sets of partitions.

  In this mode, the migration agent doesn't limit the amount of
  space that the extracted files take up on the local file system.
  Make sure the local file system has more space than the size of your largest
  partition or your largest table, depending on whether you are specifying
  a partitioning column or not.

  **Use
  [access module for Cloud Storage](https://docs.teradata.com/r/Enterprise_IntelliFlex_Lake_VMware/Teradata-Tools-and-Utilities-Access-Module-Reference-20.00/Teradata-Access-Module-for-GCS/Overview-of-the-Teradata-Access-Module-for-GCS)**.
  This approach eliminates the need for intermediate storage on your local file system,
  which provides better performance and lower resource utilization
  of the VM running the agent. This approach directly exports the data to Cloud Storage
  using the Teradata access module for Cloud Storage. To use this feature,
  the Teradata tools running on your VM must be newer than version 17.20.
  Teradata tools can be independently upgraded without any changes to
  Teradata instance version.
- **Extraction using a JDBC driver with FastExport connection.**
  If there are constraints on the local storage space available for extracted
  files, or if there is some reason you can't use TPT, then use this
  extraction method.

  In this mode,
  the migration agent extracts tables into a collection of AVRO files on the
  local file system. It then uploads these files to a Cloud Storage bucket,
  where they are used by the transfer job. Once the files are uploaded to
  Cloud Storage, the migration agent deletes them from the local file
  system.

  In this mode, you can limit the amount of space used by the AVRO files
  on the local file system. If this limit is exceeded,
  extraction is paused until space is freed up by the migration agent
  uploading and deleting existing AVRO files.

### Schema identification

You can define the schema in several ways. The BigQuery Data Transfer Service
provides automatic schema detection and [data type mapping](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#data_types)
during a data transfer from Teradata to BigQuery.
You can also use the translation engine to get the data type mapping, or you
can choose to specify a custom schema file instead.

#### Default schema detection

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To get support or provide feedback for this feature, contact [bq-dts-migration@google.com](mailto:bq-dts-migration@google.com).

If you don't specify any schema configuration, the BigQuery Data Transfer Service
automatically detects the schema of your Teradata source tables and
perform data type mapping to the corresponding BigQuery data types
during the data transfer. For more information on the default data type mapping,
see [Data types](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#data_types).

#### Using translation engine output for schema

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To get support or provide feedback for this feature, contact [bq-dts-migration@google.com](mailto:bq-dts-migration@google.com).

The BigQuery Data Transfer Service uses the output of the BigQuery
translation engine for schema mapping during the migration of Teradata
tables to BigQuery. To use this option, ensure the following
prerequisites are met:

1. Generate metadata for translation. Execute the dumper tool to generate metadata for translation, adhering to the Teradata source guidelines. For more information, see [Generate metadata for translation and assessment](https://docs.cloud.google.com/bigquery/docs/generate-metadata).
2. Upload the generated metadata file (For example, `metadata.zip`) to a Cloud Storage bucket. This bucket serves as the input location for the translation engine.
3. Initiate a batch translation job to create the BigQuery Data Transfer Service
   mapping, which defines the schema for the target BigQuery
   tables. For information on how to do this, see [Create a batch
   translation](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#create_a_batch_translation).
   The following example generates the BigQuery Data Transfer Service mapping by
   specifying `target_types = "dts_mapping"`:

       curl -d "{
       \"name\": \"teradata_2_bq_translation\",
        \"displayName\": \"Teradata to BigQuery Translation\",
        \"tasks\": {
            string: {
              \"type\": \"Teradata2BigQuery_Translation\",
              \"translation_details\": {
                  \"target_base_uri\": \"gs://your_translation_output_bucket/output\",
                  \"source_target_mapping\": {
                    \"source_spec\": {
                        \"base_uri\": \"gs://your_metadata_bucket/input\"
                    }
                  },
                  \"target_types\": \"metadata\",
              }
            }
        },
        }" \
        -H "Content-Type:application/json" \
        -H "Authorization: Bearer YOUR_ACCESS_TOKEN" -X POST https://bigquerymigration.googleapis.com/v2alpha/projects/your_project_id/locations/your_location/workflows

   You can check the status of the batch translation job in the Google Cloud console by navigating to **BigQuery** -\> **SQL Translation** . Once complete, the mapping file is stored in a Cloud Storage location specified in the `target_base_uri` flag.

   To generate a token, use the `gcloud auth print-access-token` command or the [OAuth 2.0 Playground](https://developers.google.com/oauthplayground/) with the scope `https://www.googleapis.com/auth/cloud-platform`.
4. In your Teradata data transfer configuration, specify the path to
   the Cloud Storage folder where the mapping file from the previous step is
   stored. The BigQuery Data Transfer Service uses this mapping to define the
   schema of your target BigQuery tables.

#### Custom schema file

We recommend specifying custom schema in the following situations:

- If you need to capture important information
  about a table, like partitioning, that would otherwise be lost in the
  migration.

  For example,
  [incremental transfers](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#incremental) should have a
  schema file specified so that data from subsequent transfers can be
  properly partitioned when loaded into BigQuery.
  Without a schema file, every
  time a transfer runs, the BigQuery Data Transfer Service automatically applies a
  table schema by using the source data being transferred, and all
  information about partitioning, clustering, primary keys and change
  tracking is lost.
- If you need to change column names or data types during the
  data transfer.

A custom schema file is a JSON file that describes database objects. The schema
contains a set of databases, each containing a set of tables, each of which
contains a set of columns. Each object has an
`originalName` field that indicates the object name in Teradata, and a `name`
field that indicates the target name for the object in BigQuery.

Columns have the following fields:

- `originalType`: indicates the column data type in Teradata
- `type`: indicates the target data type for the column in BigQuery.
- `usageType`: information about the way the
  column is used by the system. The following usage types are supported:

  - `DEFAULT`: You can annotate multiple columns in one target table with this usage type. This `usageType` indicates that the column has no special use in the source system. This is the default value.
  - `CLUSTERING`: You can annotate up to four columns in each target table with this usage type. The column order for clustering is determined based on the order in which they appear in the custom schema. The columns you select must meet the [constraints](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables#create_an_empty_clustered_table_with_a_schema_definition) for clustering in BigQuery. If a `PARTITIONING` field is specified for the same table, BigQuery uses these columns to create a clustered table.
  - `PARTITIONING`: You can annotate only one column in each target table with this usage type. This column is used in the [partitioned](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) table definition for the containing `tables` object. You can only use this usage type with column that has a `TIMESTAMP` or `DATE` data type.
  - `COMMIT_TIMESTAMP`: You can annotate only one column in each target table with this usage type. Use this `usageType` to identify an update timestamp column for [incremental updates](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#incremental). This column is used to extract rows that have been created or updated since the last transfer run. You can only use this usage type with columns that have a `TIMESTAMP` or `DATE` data type.
  - `PRIMARY_KEY`: You can annotate columns in each target table with this usage type. Use this usage type to identify just one column as the primary key, or in the case of a composite key, use the same usage type on multiple columns to identify the unique entities of a table. These columns work together with `COMMIT_TIMESTAMP` to extract rows created or updated since the last transfer run.

You can create a custom schema file manually, as seen in the following example,
or you can have the migration agent generate one for you when you initialize the agent.

In this example, a user is migrating a Teradata table called `orders` in the `tpch`
database with the following table definition:

```sql
  CREATE SET TABLE TPCH.orders ,FALLBACK ,
      NO BEFORE JOURNAL,
      NO AFTER JOURNAL,
      CHECKSUM = DEFAULT,
      DEFAULT MERGEBLOCKRATIO,
      MAP = TD_MAP1
      (
        O_ORDERKEY INTEGER NOT NULL,
        O_CUSTKEY INTEGER NOT NULL,
        O_ORDERSTATUS CHAR(1) CHARACTER SET LATIN CASESPECIFIC NOT NULL,
        O_TOTALPRICE DECIMAL(15,2) NOT NULL,
        O_ORDERDATE DATE FORMAT 'yyyy-mm-dd' NOT NULL,
        O_ORDERPRIORITY CHAR(15) CHARACTER SET LATIN CASESPECIFIC NOT NULL,
        O_CLERK CHAR(15) CHARACTER SET LATIN CASESPECIFIC NOT NULL,
        O_SHIPPRIORITY INTEGER NOT NULL,
        O_COMMENT VARCHAR(79) CHARACTER SET LATIN CASESPECIFIC NOT NULL)
  UNIQUE PRIMARY INDEX ( O_ORDERKEY );
```

While migrating to BigQuery, the user wants to configure
the schema with the following changes:

- Rename the `O_CUSTKEY` column to `O_CUSTOMERKEY`
- Identify `O_ORDERDATE` as the partitioning column

The following example is a custom schema to configure these settings:


    {
      "databases": [
        {
          "name": "tpch",
          "originalName": "e2e_db",
          "tables": [
            {
              "name": "orders",
              "originalName": "orders",
              "columns": [
                {
                  "name": "O_ORDERKEY",
                  "originalName": "O_ORDERKEY",
                  "type": "INT64",
                  "originalType": "integer",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 4
                },
                {
                  "name": "O_CUSTOMERKEY",
                  "originalName": "O_CUSTKEY",
                  "type": "INT64",
                  "originalType": "integer",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 4
                },
                {
                  "name": "O_ORDERSTATUS",
                  "originalName": "O_ORDERSTATUS",
                  "type": "STRING",
                  "originalType": "character",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 1
                },
                {
                  "name": "O_TOTALPRICE",
                  "originalName": "O_TOTALPRICE",
                  "type": "NUMERIC",
                  "originalType": "decimal",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 8
                },
                {
                  "name": "O_ORDERDATE",
                  "originalName": "O_ORDERDATE",
                  "type": "DATE",
                  "originalType": "date",
                  "usageType": [
                    "PARTITIONING"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 4
                },
                {
                  "name": "O_ORDERPRIORITY",
                  "originalName": "O_ORDERPRIORITY",
                  "type": "STRING",
                  "originalType": "character",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 15
                },
                {
                  "name": "O_CLERK",
                  "originalName": "O_CLERK",
                  "type": "STRING",
                  "originalType": "character",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 15
                },
                {
                  "name": "O_SHIPPRIORITY",
                  "originalName": "O_SHIPPRIORITY",
                  "type": "INT64",
                  "originalType": "integer",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 4
                },
                {
                  "name": "O_COMMENT",
                  "originalName": "O_COMMENT",
                  "type": "STRING",
                  "originalType": "varchar",
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true,
                  "originalColumnLength": 79
                }
              ]
            }
          ]
        }
      ]
    }

### On-demand or Incremental transfers

When migrating data from a Teradata database instance to
BigQuery, the BigQuery Data Transfer Service supports both full transfers
(on-demand transfer) and recurring transfers (incremental transfers). You
designate the transfer as on-demand or incremental in the scheduling options
when [setting up a transfer](https://docs.cloud.google.com/bigquery/docs/migration/teradata#set_up_a_transfer).

- On-demand transfer: Use this mode to perform the full snapshot migration
  of schema and data from Teradata to BigQuery.

- Scheduled transfer: Use this mode to perform the full snapshot and regularly
  migrate new and modified data (incremental data) from Teradata to
  BigQuery. Incremental transfers requires customizing your
  schema to annotate columns with either of the below use cases:

  - Annotate columns with only `COMMIT_TIMESTAMP` usage type: In this transfer, new or modified rows in Teradata are appended to data in BigQuery. Updated rows in BigQuery tables might potentially have duplicate rows with old and new values.
  - Annotate columns with both `COMMIT_TIMESTAMP` and `PRIMARY_KEY` usage type: In this transfer, new rows are appended and modified rows are updated to the corresponding row in BigQuery. The column defined in `PRIMARY_KEY` is used to maintain uniqueness of the data in BigQuery.
  - The `PRIMARY_KEY` column defined in the schema does not have to be the `PRIMARY_KEY` in the Teradata table. It can be any column, but must contain unique data.

#### Incremental transfers

In incremental transfers, the first transfer always creates a table
snapshot in BigQuery. All subsequent incremental transfers
will adhere to the annotations defined in the custom schema file explained
below.

For each transfer run, a timestamp of the transfer run is saved. For each
subsequent transfer run, an agent receives the timestamp of a previous transfer
run (T1) and a timestamp of when the current transfer run started (T2).

For transfers after initial run, the migration agent will extract data
using the following per-table logic:

- If a table object in a schema file does not have a column with a usage type of `COMMIT_TIMESTAMP`, then the table is skipped.
- If a table has a column with the usage type of `COMMIT_TIMESTAMP`, then all rows with a timestamp between T1 and T2 are extracted and appended to the existing table in BigQuery.
- If a table has a column with the usage type of `COMMIT_TIMESTAMP` and a column with usage type of `PRIMARY_KEY`, then all rows with a timestamp between T1 and T2 are extracted. Any new rows are appended and modified rows are updated in the existing table in BigQuery.

> [!NOTE]
> **Note:** Incremental migration from Teradata does not support the syncing of deleted rows with BigQuery.

The following are example schema files for incremental transfers.

Schema with only `COMMIT_TIMESTAMP`


    {
      "databases": [
        {
          "name": "abc_db",
          "originalName": "abc_db",
          "tables": [
            {
              "name": "abc_table",
              "originalName": "abc_table",
              "columns": [
                {
                  "name": "Id",
                  "originalName": "Id",
                  "type": "INT64",
                  "originalType": "integer",
                  "originalColumnLength": 4,
                  "usageType": [
                    "DEFAULT"
                  ],
                  "isRequired": true
                },
                {
                  "name": "timestamp",
                  "originalName": "timestamp",
                  "type": "TIMESTAMP",
                  "originalType": "timestamp",
                  "originalColumnLength": 26,
                  "usageType": [
                    "COMMIT_TIMESTAMP"
                  ],
                  "isRequired": false
                }
              ]
            }
          ]
        }
      ]
    }

Schema with `COMMIT_TIMESTAMP` and one column (Id) as `PRIMARY_KEY`


    {
      "databases": [
        {
          "name": "abc_db",
          "originalName": "abc_db",
          "tables": [
            {
              "name": "abc_table",
              "originalName": "abc_table",
              "columns": [
                {
                  "name": "Id",
                  "originalName": "Id",
                  "type": "INT64",
                  "originalType": "integer",
                  "originalColumnLength": 4,
                  "usageType": [
                    "PRIMARY_KEY"
                  ],
                  "isRequired": true
                },
                {
                  "name": "timestamp",
                  "originalName": "timestamp",
                  "type": "TIMESTAMP",
                  "originalType": "timestamp",
                  "originalColumnLength": 26,
                  "usageType": [
                    "COMMIT_TIMESTAMP"
                  ],
                  "isRequired": false
                }
              ]
            }
          ]
        }
      ]
    }

Schema with `COMMIT_TIMESTAMP` and Composite key (Id + Name) as `PRIMARY_KEY`


    {
      "databases": [
        {
          "name": "abc_db",
          "originalName": "abc_db",
          "tables": [
            {
              "name": "abc_table",
              "originalName": "abc_table",
              "columns": [
                {
                  "name": "Id",
                  "originalName": "Id",
                  "type": "INT64",
                  "originalType": "integer",
                  "originalColumnLength": 4,
                  "usageType": [
                    "PRIMARY_KEY"
                  ],
                  "isRequired": true
                },
                {
                  "name": "Name",
                  "originalName": "Name",
                  "type": "STRING",
                  "originalType": "character",
                  "originalColumnLength": 30,
                  "usageType": [
                    "PRIMARY_KEY"
                  ],
                  "isRequired": false
                },
                {
                  "name": "timestamp",
                  "originalName": "timestamp",
                  "type": "TIMESTAMP",
                  "originalType": "timestamp",
                  "originalColumnLength": 26,
                  "usageType": [
                    "COMMIT_TIMESTAMP"
                  ],
                  "isRequired": false
                }
              ]
            }
          ]
        }
      ]
    }

The following table describes how the migration agent handles data definition
language (DDL) and data manipulation language (DML) operations in incremental
transfers.

| Teradata operation | Type | Teradata-to-BigQuery support |
|---|---|---|
| `CREATE` | DDL | A new full snapshot for the table is created in BigQuery. |
| `DROP` | DDL | Not supported |
| `ALTER` (`RENAME`) | DDL | A new full snapshot for the renamed table is created in BigQuery. The previous snapshot is not deleted from BigQuery}. The user is not notified of the renamed table. |
| `INSERT` | DML | New rows are added to the BigQuery table. |
| `UPDATE` | DML | Rows are appended to the BigQuery table as new, similar to an `INSERT` operation if only `COMMIT_TIMESTAMP` is used. Rows are updated, similar to an `UPDATE` operation if both `COMMIT_TIMESTAMP` and `PRIMARY_KEY` are used. |
| `MERGE` | DML | Not supported. See instead `INSERT`, `UPDATE`, and `DELETE`. |
| `DELETE` | DML | Not supported |

## Location considerations

Your Cloud Storage bucket must be in a region or multi-region that is
compatible with the region or multi-region of the destination dataset in
BigQuery.

- If your BigQuery dataset is in a multi-region, the Cloud Storage bucket containing the data you're transferring must be in the same multi-region or in a location that is contained within the multi-region. For example, if your BigQuery dataset is in the `EU` multi-region, the Cloud Storage bucket can be located in the `europe-west1` Belgium region, which is within the EU.
- If your dataset is in a single region, your Cloud Storage bucket must be in the same region. For example, if your dataset is in the `asia-northeast1` Tokyo region, your Cloud Storage bucket cannot be in the `ASIA` multi-region.

For detailed information about transfers and regions, see
[Dataset locations and transfers](https://docs.cloud.google.com/bigquery/docs/dts-locations).

## Pricing

The data transfer with BigQuery is free of charge. However,
costs can be incurred outside of Google by using this service, such as
platform outbound data transfer charges.

- Extraction, uploading to a Cloud Storage bucket, and loading data into BigQuery is free.
- Data is ***not*** automatically deleted from your Cloud Storage bucket after it is uploaded to BigQuery. Consider deleting the data from your Cloud Storage bucket to avoid additional storage costs. See [Cloud Storage pricing](https://cloud.google.com/storage/pricing).
- Standard BigQuery [Quotas \& limits](https://docs.cloud.google.com/bigquery/quotas#load_jobs) on load jobs apply.
- Standard DML BigQuery [Quotas \& limits](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements) on incremental ingestion upserts will apply.
- After data is transferred to BigQuery, standard BigQuery [storage](https://cloud.google.com/bigquery/pricing#storage) and [compute](https://cloud.google.com/bigquery/pricing#analysis_pricing_models) pricing applies.
- See our transfers [Pricing page](https://cloud.google.com/bigquery/pricing#data-transfer-service-pricing) for details.

## Limitations

- One-time, on-demand transfers are fully supported. [DDL/DML operations in incremental transfers](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#ddldml_operations_in_incremental_transfers) are partially supported.
- During data transfer, data is extracted to a directory on the local file system. Make sure there is adequate free space.
  - When using the FastExport mode of extraction, you can set the maximum storage space to be used, and the limit strongly enforced by the migration agent. Set the `max-local-storage` setting in the [migration agent's configuration file](https://docs.cloud.google.com/bigquery/docs/migration/teradata#initialize_the_migration_agent) when [setting up a transfer from Teradata to BigQuery](https://docs.cloud.google.com/bigquery/docs/migration/teradata#set_up_a_transfer).
  - When using the TPT extraction method, make sure the file system has enough free space --- larger than the largest table partition in the Teradata instance.
- The BigQuery Data Transfer Service converts schema automatically (if you don't supply a custom schema file) and transfers Teradata data to BigQuery. Data is [mapped from Teradata to BigQuery types](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#teradata_mapping).
- Files are **not** automatically deleted from your Cloud Storage bucket after being loaded into BigQuery. Consider deleting the data from your Cloud Storage bucket after loading it into BigQuery, to avoid additional storage costs. See [Pricing](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#pricing).
- The speed of the extraction is bounded by your JDBC connection.
- The data extracted from Teradata is ***not*** encrypted. Take appropriate steps to restrict access to the extracted files in the local file system, and ensure the Cloud Storage bucket is properly secured.
- Other database resources, such as stored procedures, saved queries, views, and user-defined functions are ***not*** transferred and not in the scope of this service.
- Incremental transfers does not support hard deletes. Incremental transfers does not sync any deleted rows in Teradata with BigQuery.

## What's next

- Get step-by-step instructions to [Migrate Teradata to BigQuery](https://docs.cloud.google.com/bigquery/docs/migration/teradata).
- Try a [test migration](https://docs.cloud.google.com/bigquery/docs/migration/teradata-tutorial) of Teradata to BigQuery.