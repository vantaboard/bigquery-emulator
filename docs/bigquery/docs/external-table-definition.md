# Create a table definition file for an external data source

This page describes how to create a table definition file, which is a
prerequisite for creating an
[external data source](https://docs.cloud.google.com/bigquery/docs/external-data-sources) by using the
bq command-line tool. You create the external data source by running the
[`bg mk --table` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table)
and using the `--external_table_definition` flag to specify the table
definition file.

A table definition file contains an external table's schema definition and
metadata, such as the table's data format and related properties. You can set
the same properties in a table definition file that are documented for the
[`ExternalDataConfiguration` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
in the REST API.

You can create table definition files to describe a [permanent or temporary
external table](https://docs.cloud.google.com/bigquery/docs/external-tables#temporary_table_support) for the
following external data sources:

- Cloud Storage

  - Comma-separated values (CSV)
  - Newline-delimited JSON
  - Avro files
  - Datastore export files
  - ORC files
  - Parquet files
  - Firestore export files
- Google Drive

  - Comma-separated values (CSV)
  - Newline-delimited JSON
  - Avro files
  - Google Sheets
- Bigtable

## Before you begin

To create a table definition file, you need the URI for your data source:

- For a Drive data source, you need the [Drive URI](https://docs.cloud.google.com/bigquery/docs/external-data-drive#drive-uri)
- For a Cloud Storage data source, you need the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri)
- For a Bigtable data source, you need the [Bigtable URI](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table#bigtable-uri)

## Create a definition file for CSV, JSON, or Google Sheets files

When you create a table definition file for CSV, JSON, or Google Sheets files
in Cloud Storage or Drive, you can specify the table schema in
the following ways:

- [Use the `autodetect` flag](https://docs.cloud.google.com/bigquery/docs/external-table-definition#use-auto-detect-flag)
- [Use an inline schema](https://docs.cloud.google.com/bigquery/docs/external-table-definition#use-inline-schema)
- [Use a JSON schema file](https://docs.cloud.google.com/bigquery/docs/external-table-definition#use-json-schema)

### Use the `autodetect` flag

If you specify a CSV, JSON, or Google Sheets file without including an inline
schema description or a schema file, you can use the `--autodetect` flag
to set the `"autodetect"` option to `true` in the table definition file. When
auto-detect is enabled, BigQuery makes a best-effort attempt to
automatically infer the schema. For more information, see
[Schema auto-detection for external data sources](https://docs.cloud.google.com/bigquery/docs/schema-detect#schema_auto-detection_for_external_data_sources).

#### Use auto-detect with a Cloud Storage data source

Create a table definition file for a Cloud Storage data source:

1. Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
   with the `--autodetect` flag to create a table definition file. The `mkdef`
   command generates a table definition file in JSON format. The following
   example creates a table definition and writes the output to a file:
   `/tmp/file_name`.

   ```bash
   bq mkdef \
     --autodetect \
     --source_format=SOURCE_FORMAT \
     "URI" > /tmp/FILE_NAME
   ```

   Replace the following:
   - `SOURCE_FORMAT`: your file format
   - `FILE_NAME`: the name of your table definition file
   - `URI`: the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri)

     For example, `gs://mybucket/myfile`.
2. (Optional) Open the table definition file in a text editor. For example,
   the command `nano /tmp/file_name` opens the file in
   nano. The file should look like the following for a CSV external data
   source. Notice `"autodetect"` is set to `true`.

   ```json
   {
   "autodetect": true,
   "csvOptions": {
     "allowJaggedRows": false,
     "allowQuotedNewlines": false,
     "encoding": "UTF-8",
     "fieldDelimiter": ",",
     "quote": "\"",
     "skipLeadingRows": 0
   },
   "sourceFormat": "CSV",
   "sourceUris": [
     "URI"
   ]
   }
   ```
3. (Optional) Manually edit the table definition file to modify, add, or
   delete general settings such as `maxBadRecords` and `ignoreUnknownValues`. There
   are no configuration settings that are specific to JSON source files, but there
   are settings that apply to [CSV](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#CsvOptions)
   and [Google Sheets](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#GoogleSheetsOptions)
   files. For more information, see
   [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
   in the API reference.

#### Use auto-detect with a Drive data source

Create a table definition file for a Drive data source:

1. Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
   with the `--autodetect` flag to create a table definition. The `mkdef` command
   generates a table definition file in JSON format. The following example
   creates a table definition and writes the output to a file:
   `/tmp/file_name`.

   ```bash
   bq mkdef \
      --autodetect \
      --source_format=SOURCE_FORMAT \
      "URI" > /tmp/FILE_NAME
   ```

   Replace the following:
   - `SOURCE_FORMAT`: your file format
   - `FILE_NAME`: the name of your table definition file
   - `URI`: the [Drive URI](https://docs.cloud.google.com/bigquery/docs/external-data-drive#drive-uri)

     For example, `https://drive.google.com/open?id=123ABCD123AbcD123Abcd`.
2. Open the table definition file in a text editor. For example, the command
   `nano /tmp/file_name` opens the file in nano. The file
   should look like the following for a Google Sheets external data source. Notice
   `"autodetect"` is set to `true`.

   ```json
   {
   "autodetect": true,
   "sourceFormat": "GOOGLE_SHEETS",
   "sourceUris": [
     "URI"
   ]
   }
   ```
3. (Optional) Manually edit the table definition file to modify, add, or
   delete general settings such as `maxBadRecords` and `ignoreUnknownValues`. There
   are no configuration settings that are specific to JSON source files, but there
   are settings that apply to [CSV](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#CsvOptions)
   and [Google Sheets](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#GoogleSheetsOptions)
   files. For more information, see
   [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
   in the API reference.

4. To specify a particular sheet or a cell range in a Google Sheets file, add
   the `range` property to the [`GoogleSheetsOptions` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#googlesheetsoptions) in the table definition file. To query a particular sheet,
   specify the sheet name. To query a cell range, specify the range in the form:
   `sheet_name!top_left_cell_id:bottom_right_cell_id`,
   for example, `"Sheet1!A1:B20"`. If the `range` parameter is not specified, the
   first sheet in the file is used.

### Use an inline schema

If you don't want to use schema auto-detect, you can create a table definition
file by providing an inline schema definition. To provide an inline schema
definition, list the fields and data types on the command line in the following
format: `FIELD:DATA_TYPE,FIELD:DATA_TYPE`.

#### Use an inline schema with a Cloud Storage or Drive data source

Create a table definition for a Cloud Storage or Drive
data source by using an inline schema definition:

1. Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
   with the `--noautodetect` flag to create a table definition. The `mkdef`
   command generates a table definition file in JSON format. The following
   example creates a table definition and writes the output to a file:
   `/tmp/file_name`.

   ```bash
   bq mkdef \
     --noautodetect \
     --source_format=SOURCE_FORMAT \
     "URI" \
     FIELD:DATA_TYPE,FIELD:DATA_TYPE > /tmp/FILE_NAME
   ```

   Replace the following
   - `SOURCE_FORMAT`: the source file format
   - `URI`: the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri)
     or your [Drive URI](https://docs.cloud.google.com/bigquery/docs/external-data-drive#drive-uri)

     For example, `gs://mybucket/myfile` for Cloud Storage or `https://drive.google.com/open?id=123ABCD123AbcD123Abcd` for Drive.
   - `FIELD:DATA_TYPE,FIELD:DATA_TYPE`: the schema
     definition

     For example, `Name:STRING,Address:STRING, ...`.
   - `FILE_NAME`: the name of your table definition file

2. (Optional) Open the table definition file in a text editor. For example,
   the command `nano /tmp/file_name` opens the file in
   nano. The file should look like the following. Notice `"autodetect"` is not
   enabled, and the schema information is written to the table definition file.

   ```json
   {
   "schema": {
     "fields": [
       {
         "name": "FIELD",
         "type": "DATA_TYPE"
       },
       {
         "name": "FIELD",
         "type": "DATA_TYPE"
       }
       ...
     ]
   },
   "sourceFormat": "NEWLINE_DELIMITED_JSON",
   "sourceUris": [
     "URI"
   ]
   }
   ```
3. (Optional) Manually edit the table definition file to modify, add, or
   delete general settings such as `maxBadRecords` and `ignoreUnknownValues`. There
   are no configuration settings that are specific to JSON source files, but there
   are settings that apply to [CSV](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#CsvOptions)
   and [Google Sheets](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#GoogleSheetsOptions)
   files. For more information, see
   [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
   in the API reference.

### Use a JSON schema file

If you don't want to use auto-detect or provide an inline schema definition,
you can create a JSON schema file and reference it when creating your table
definition file. Create the JSON schema file manually on your local machine.
Referencing a JSON schema file stored in Cloud Storage
or in Drive is not supported.

#### Use a schema file with a Cloud Storage data source

Create a table definition for a Cloud Storage data source
using a JSON schema file:

1. Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
   with the `--noautodetect` flag to create a table definition. The `mkdef`
   command generates a table definition file in JSON format. The following
   example creates a table definition and writes the output to a file:
   `/tmp/file_name`.

   ```bash
   bq mkdef \
      --noautodetect \
      --source_format=SOURCE_FORMAT \
      "URI" \
     PATH_TO_SCHEMA_FILE > /tmp/FILE_NAME
   ```

   Replace the following:
   - `SOURCE_FORMAT`: your file format
   - `FILE_NAME`: the name of your table definition file
   - `URI`: the
     [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri)

     For example, `gs://mybucket/myfile`.
   - `PATH_TO_SCHEMA_FILE`: the location of the JSON schema
     file on your local machine

2. (Optional) Open the table definition file in a text editor. For example,
   the command `nano /tmp/file_name` opens the file in  

   nano. The file should look like the following. Notice `"autodetect"` is not
   enabled, and the schema information is written to the table definition file.

   ```json
   {
   "schema": {
     "fields": [
       {
         "name": "FIELD",
         "type": "DATA_TYPE"
       },
       {
         "name": "FIELD",
         "type": "DATA_TYPE"
       }
       ...
     ]
   },
   "sourceFormat": "NEWLINE_DELIMITED_JSON",
   "sourceUris": [
     "URI"
   ]
   }
   ```
3. (Optional) Manually edit the table definition file to modify, add, or
   delete general settings such as `maxBadRecords` and `ignoreUnknownValues`. There
   are no configuration settings that are specific to JSON source files, but there
   are settings that apply to [CSV](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#CsvOptions)
   and [Google Sheets](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#GoogleSheetsOptions)
   files. For more information, see
   [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
   in the API reference.

#### Use a schema file with a Drive data source

Create a table definition for a Drive data source
using a JSON schema file:

1. Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
   with the `--noautodetect` flag to create a table definition. The `mkdef`
   command generates a table definition file in JSON format. The following
   example creates a table definition and writes the output to a file:
   `/tmp/file_name`.

   ```bash
   bq mkdef \
      --noautodetect \
      --source_format=source_format \
      "URI" \
      PATH_TO_SCHEMA_FILE > /tmp/FILE_NAME
   ```

   Replace the following:
   - `SOURCE_FORMAT`: the source file format
   - `URI`: the [Drive URI](https://docs.cloud.google.com/bigquery/docs/external-data-drive#drive-uri)

     For example, `https://drive.google.com/open?id=123ABCD123AbcD123Abcd`.
   - `PATH_TO_SCHEMA_FILE`: the location of the JSON schema
     file on your local machine

   - `FILE_NAME`: the name of your table definition file

2. Open the table definition file in a text editor. For example, the command
   `nano /tmp/file_name` opens the file in nano. The file
   should look like the following. Notice `"autodetect"` is not enabled, and the
   schema information is written to the table definition file.

   ```json
   {
   "schema": {
     "fields": [
       {
         "name": "FIELD",
         "type": "DATA_TYPE"
       },
       {
         "name": "FIELD",
         "type": "DATA_TYPE"
       }
       ...
     ]
   },
   "sourceFormat": "GOOGLE_SHEETS",
   "sourceUris": [
     "URI"
   ]
   }
   ```
3. (Optional) Manually edit the table definition file to modify, add, or
   delete general settings such as `maxBadRecords` and `ignoreUnknownValues`. There
   are no configuration settings that are specific to JSON source files, but there
   are settings that apply to [CSV](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#CsvOptions)
   and [Google Sheets](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#GoogleSheetsOptions)
   files. For more information, see
   [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
   in the API reference.

4. To specify a particular sheet or a cell range in a Google Sheets file, add
   the `range` property to the [`GoogleSheetsOptions` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#googlesheetsoptions) in the table definition file. To query a particular sheet,
   specify the sheet name. To query a cell range, specify the range in the form:
   `sheet_name!top_left_cell_id:bottom_right_cell_id`,
   for example, `"Sheet1!A1:B20"`. If the `range` parameter is not specified, the
   first sheet in the file is used.

## Create a definition file for self-describing formats

Avro, Parquet, and ORC are *self-describing* formats. Data files in these
formats contain their own schema information. If you use one of these formats as
an external data source, then BigQuery automatically retrieves the
schema using the source data. When creating a table definition, you don't need
to use schema auto-detection, and you don't need to provide an inline schema
definition or schema file.

You can create a table definition file for Avro, Parquet, or ORC data stored in
Cloud Storage or Drive:

1. Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
   to create a table definition.

   ```bash
   bq mkdef \
       --source_format=FORMAT \
       "URI" > FILE_NAME
   ```

   Replace the following:
   - `FORMAT`: the source format

   - `URI`: the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri)
     or your [Drive URI](https://docs.cloud.google.com/bigquery/docs/external-data-drive#drive-uri)

     For example, `gs://mybucket/myfile` for Cloud Storage or
     `https://drive.google.com/open?id=123ABCD123AbcD123Abcd`
     for Drive.
   - `FILE_NAME`: the name of your table definition file

2. Optional: Open the table definition file in a text editor. The file looks
   similar to the following:

   ```json
   {
      "sourceFormat": "AVRO",
      "sourceUris": [
      "URI"
       ]
   }
   ```
3. Optional: Manually edit the table definition file to modify, add, or
   delete general settings such as `maxBadRecords` and `ignoreUnknownValues`.
   For more information, see
   [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
   in the API reference.

## Create a definition file for hive-partitioned data

Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
with the `hive_partitioning_mode` and the
`hive_partitioning_source_uri_prefix` flags to
[create a definition file for hive-partitioned data](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries) that's stored in
Cloud Storage, Amazon Simple Storage Service (Amazon S3), or Azure Blob Storage.

## Create a definition file for Datastore and Firestore

If you use a Datastore or Firestore export as an external data
source, BigQuery automatically retrieves the schema using the
self-describing source data. When creating a table definition, you don't need to
provide an inline schema definition or schema file.

You can create a table definition file for Datastore and
Firestore export data stored in Cloud Storage:

1. Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
   to create a table definition. You don't need to use the `--noautodetect`
   flag with Datastore or Firestore backup files. Schema
   auto-detect is disabled for these file types. The `mkdef` command generates a
   table definition file in JSON format. The following example creates a table
   definition and writes the output to a file: `/tmp/file_name`.

   ```bash
   bq mkdef \
   --source_format=DATASTORE_BACKUP \
   "URI" > /tmp/FILE_NAME
   ```

   Replace the following:
   - `URI`: the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri)
   - `FILE_NAME`: the name of your table definition file

   The `DATASTORE_BACKUP` source format is used for both
   Datastore and Firestore.
2. (Optional) Open the table definition file in a text editor. For example,
   the command `nano /tmp/file_name` opens the file in nano.
   The file should look like the following. Notice there is no need for the
   `"autodetect"` setting.

   ```json
   {
   "sourceFormat": "DATASTORE_BACKUP",
   "sourceUris": [
     "gs://URI"
   ]
   }
   ```
3. (Optional) Manually edit the table definition file to modify, add, or
   delete settings such as `maxBadRecords` and `ignoreUnknownValues`. There
   are no configuration settings that are specific to Datastore and
   Firestore export files. For more information, see
   [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
   in the API reference.

## Create a definition file for Bigtable

When you create a table definition file for Bigtable, you manually
generate the file in JSON format. Using the `mkdef` command to create a table
definition is not supported for Bigtable data sources.
Schema auto-detect is also unsupported for Bigtable. For a list of
Bigtable table definition options, see
[`BigtableOptions`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#bigtableoptions) in
the REST API reference.

A JSON table definition file for Bigtable looks like the following.
Using this table definition file, BigQuery reads data from a
single column family, interpreting the values as binary encoded integers.

```json
{
    "sourceFormat": "BIGTABLE",
    "sourceUris": [
        "https://googleapis.com/bigtable/projects/PROJECT_ID/instances/INSTANCE_ID[/appProfiles/APP_PROFILE_ID]/tables/TABLE_NAME"
    ],
    "bigtableOptions": {
        "columnFamilies" : [
            {
                "familyId": "FAMILY_ID",
                "type": "INTEGER",
                "encoding": "BINARY"
            }
        ]
    }
}
```

Replace the following:

- `PROJECT_ID`: the project containing your Bigtable cluster
- `INSTANCE_ID`: the Bigtable instance ID
- `APP_PROFILE_ID` (optional): the ID of the app profile that you want to use to read your Bigtable data. [App profile
  settings](https://docs.cloud.google.com/bigtable/docs/app-profiles) indicate whether the external table uses Data Boost or provisioned nodes for compute.
- `TABLE_NAME`: the name of the table you're querying
- `FAMILY_ID`: the column family identifier

For more information, see
[Retrieve the Bigtable URI](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table#bigtable-uri).

## Wildcard support for table definition files

If your data is separated into multiple files, you can use an asterisk (\*)
wildcard to select multiple files. Use of the asterisk wildcard must follow
these rules:

- The asterisk can appear inside the object name or at the end of the object name.
- Using multiple asterisks is unsupported. For example, the path `gs://mybucket/fed-*/temp/*.csv` is invalid.
- Using an asterisk with the bucket name is unsupported.

Examples:

- The following example shows how to select all of the files in all the
  folders which start with the prefix `gs://mybucket/fed-samples/fed-sample`:

      gs://mybucket/fed-samples/fed-sample*

- The following example shows how to select only files with a `.csv` extension
  in the folder named `fed-samples` and any subfolders of `fed-samples`:

      gs://mybucket/fed-samples/*.csv

- The following example shows how to select files with a naming pattern of
  `fed-sample*.csv` in the folder named `fed-samples`. This example doesn't
  select files in subfolders of `fed-samples`.

      gs://mybucket/fed-samples/fed-sample*.csv

When using the bq command-line tool, you might need to escape the asterisk on some
platforms.

If you use an asterisk wildcard, enclose the bucket and filename in quotes. For
example, if you have two files named `fed-sample000001.csv` and
`fed-sample000002.csv` and you want to use an asterisk to select both of them,
the bucket URI would be `"gs://mybucket/fed-sample*"`.

The `*` wildcard character is not allowed when creating table definition files
for the following data sources:

- **Bigtable**. For Bigtable data, only one data source can be specified. The URI value must be a valid HTTPS URL for a Bigtable table.
- **Datastore** or **Firestore** . Datastore or Firestore exports stored in Cloud Storage. For Datastore backups, only one data source can be specified. The URI value must end with `.backup_info` or `.export_metadata`.
- **Drive**. Data stored in Drive.

## What's next

- Learn how to query [Cloud Storage data](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake).
- Learn how to query [Drive data](https://docs.cloud.google.com/bigquery/docs/external-data-drive).
- Learn how to query [Bigtable data](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable).