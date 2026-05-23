# Specifying a schema

BigQuery lets you specify a table's schema when you load
data into a table, and when you create an empty table. Alternatively, you can
use schema [auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect#auto-detect) for
supported data formats.

When you load Avro, Parquet, ORC, Firestore export files, or
Datastore export files, the schema is automatically retrieved from the
self-describing source data.

You can specify a table's schema in the following ways:

- Use the Google Cloud console.
- Use the `CREATE TABLE` SQL statement.
- Inline using the bq command-line tool.
- Create a schema file in JSON format.
- Call the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) method and configure the `schema` property in the `load` job configuration.
- Call the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert) method and configure the schema in the [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables) using the `schema` property.

After loading data or creating an empty table, you can
[modify the table's schema definition](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

## Schema components

When you specify a table schema, you must supply each column's name and data
type. You can also supply a column's description, mode, and default value.

### Column names

A column name can contain letters (a-z, A-Z), numbers (0-9), or underscores
(_), and it must start with a letter or underscore. If you use flexible column
names, BigQuery supports starting a column name with a number.
Exercise caution when starting columns with a number, since using flexible
column names with the BigQuery Storage Read API or
BigQuery Storage Write API requires special handling. For more information about
flexible column name support, see
[flexible column names](https://docs.cloud.google.com/bigquery/docs/schemas#flexible-column-names).

Column names have a maximum length of 300 characters. Column names can't use any
of the following prefixes:

- `_TABLE_`
- `_FILE_`
- `_PARTITION`
- `_ROW_TIMESTAMP`
- `__ROOT__`
- `_COLIDENTIFIER`
- `_CHANGE_SEQUENCE_NUMBER`
- `_CHANGE_TYPE`
- `_CHANGE_TIMESTAMP`

Duplicate column names are not allowed even if the case differs. For example, a
column named `Column1` is considered identical to a column named `column1`. To
learn more about column naming rules, see [Column
names](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#column_names) in the
GoogleSQL reference.

If a table name (for example, `test`) is the same as one of its column names
(for example, `test`), the `SELECT` expression interprets the `test` column as
a `STRUCT` containing all other table columns. To avoid this collision, use
one of the following methods:

- Avoid using the same name for a table and its columns.

- Avoid using `_field_` as a column name prefix. System-reserved prefixes
  cause automatic renaming during queries. For example, the
  `SELECT _field_ FROM project1.dataset.test` query returns a column named
  `_field_1`. If you must query a column with this name, use an alias to
  control the output.

- Assign the table a different alias. For example, the following query assigns
  a table alias `t` to the table `project1.dataset.test`:

      SELECT test FROM project1.dataset.test AS t;

- Include the table name when referencing a column. For example:

      SELECT test.test FROM project1.dataset.test;

### Flexible column names

You have more flexibility in what you name columns, including expanded access
to characters in languages other than English as well as additional symbols.
Make sure to use backtick (`` ` ``) characters to enclose flexible column names if they are
[Quoted Identifiers](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#quoted_identifiers).

Flexible column names support the following characters:

- Any letter in any language, as represented by the Unicode regular expression [`\p{L}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- Any numeric character in any language as represented by the Unicode regular expression [`\p{N}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- Any connector punctuation character, including underscores, as represented by the Unicode regular expression [`\p{Pc}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- A hyphen or dash as represented by the Unicode regular expression [`\p{Pd}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- Any mark intended to accompany another character as represented by the Unicode regular expression [`\p{M}`](https://www.unicode.org/reports/tr44/#General_Category_Values). For example, accents, umlauts, or enclosing boxes.
- The following special characters:
  - An ampersand (`&`) as represented by the Unicode regular expression `\u0026`.
  - A percent sign (`%`) as represented by the Unicode regular expression `\u0025`.
  - An equals sign (`=`) as represented by the Unicode regular expression `\u003D`.
  - A plus sign (`+`) as represented by the Unicode regular expression `\u002B`.
  - A colon (`:`) as represented by the Unicode regular expression `\u003A`.
  - An apostrophe (`'`) as represented by the Unicode regular expression `\u0027`.
  - A less-than sign (`<`) as represented by the Unicode regular expression `\u003C`.
  - A greater-than sign (`>`) as represented by the Unicode regular expression `\u003E`.
  - A number sign (`#`) as represented by the Unicode regular expression `\u0023`.
  - A vertical line (`|`) as represented by the Unicode regular expression `\u007c`.
  - Whitespace.

Flexible column names don't support the following special characters:

- An exclamation mark (`!`) as represented by the Unicode regular expression `\u0021`.
- A quotation mark (`"`) as represented by the Unicode regular expression `\u0022`.
- A dollar sign (`$`) as represented by the Unicode regular expression `\u0024`.
- A left parenthesis (`(`) as represented by the Unicode regular expression `\u0028`.
- A right parenthesis (`)`) as represented by the Unicode regular expression `\u0029`.
- An asterisk (`*`) as represented by the Unicode regular expression `\u002A`.
- A comma (`,`) as represented by the Unicode regular expression `\u002C`.
- A period (`.`) as represented by the Unicode regular expression `\u002E`. Periods are *not* replaced by underscores in Parquet file column names when a column name character map is used. For more information, see [flexible column limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#limitations_2).
- A slash (`/`) as represented by the Unicode regular expression `\u002F`.
- A semicolon (`;`) as represented by the Unicode regular expression `\u003B`.
- A question mark (`?`) as represented by the Unicode regular expression `\u003F`.
- An at sign (`@`) as represented by the Unicode regular expression `\u0040`.
- A left square bracket (`[`) as represented by the Unicode regular expression `\u005B`.
- A backslash (`\`) as represented by the Unicode regular expression `\u005C`.
- A right square bracket (`]`) as represented by the Unicode regular expression `\u005D`.
- A circumflex accent (`^`) as represented by the Unicode regular expression `\u005E`.
- A grave accent (`` ` ``) as represented by the Unicode regular expression `\u0060`.
- A left curly bracket {`{`) as represented by the Unicode regular expression `\u007B`.
- A right curly bracket (`}`) as represented by the Unicode regular expression `\u007D`.
- A tilde (`~`) as represented by the Unicode regular expression `\u007E`.

For additional guidelines, see
[Column names](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#column_names).

The expanded column characters are supported by both the BigQuery Storage Read API
and the BigQuery Storage Write API. To use the expanded list of Unicode characters
with the BigQuery Storage Read API, you must set a flag. You can use the
`displayName` attribute to retrieve the column name. The following example
shows how to set a flag with the Python client:

    from google.cloud.bigquery_storage import types
    requested_session = types.ReadSession()

    #set avro serialization options for flexible column.
    options = types.AvroSerializationOptions()
    options.enable_display_name_attribute = True
    requested_session.read_options.avro_serialization_options = options

To use the expanded list of Unicode characters with the BigQuery Storage Write API,
you must provide the schema with `column_name` notation, unless you are using
the `JsonStreamWriter` writer object. The following example shows how to
provide the schema:

    syntax = "proto2";
    package mypackage;
    // Source protos located in github.com/googleapis/googleapis
    import "google/cloud/bigquery/storage/v1/annotations.proto";

    message FlexibleSchema {
      optional string item_name_column = 1
      [(.google.cloud.bigquery.storage.v1.column_name) = "name-列"];
      optional string item_description_column = 2
      [(.google.cloud.bigquery.storage.v1.column_name) = "description-列"];
    }

In this example, `item_name_column` and `item_description_column` are
placeholder names which need to be compliant with the
[protocol buffer](https://protobuf.dev/) naming
convention. Note that `column_name` annotations always take precedence over
placeholder names.

#### Limitations

- Flexible column names are not supported with [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables).

### Column descriptions

Each column can include an optional description. The description is a string
with a maximum length of 1,024 characters.

### Default values

The [default value](https://docs.cloud.google.com/bigquery/docs/default-values) for a column must be a
[literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#literals) or one of the
following functions:

- [`CURRENT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date)
- [`CURRENT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime)
- [`CURRENT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time)
- [`CURRENT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp)
- [`GENERATE_UUID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/utility-functions#generate_uuid)
- [`RAND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand)
- [`SESSION_USER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/security_functions#session_user)
- [`ST_GEOGPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)

### GoogleSQL data types

GoogleSQL lets you specify the following data
types in your schema. Data type is required.

| Name | Data type | Description |
|---|---|---|
| [Integer](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types) | `INT64` | Numeric values without fractional components |
| [Floating point](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types) | `FLOAT64` | Approximate numeric values with fractional components |
| [Numeric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type) | `NUMERIC` | Exact numeric values with fractional components |
| [BigNumeric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bignumeric_type) | `BIGNUMERIC` | Exact numeric values with fractional components |
| [Boolean](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#boolean_type) | `BOOL` | TRUE or FALSE (case-insensitive) |
| [String](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type) | `STRING` | Variable-length character (Unicode) data |
| [Bytes](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type) | `BYTES` | Variable-length binary data |
| [Date](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type) | `DATE` | A logical calendar date |
| [Date/Time](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type) | `DATETIME` | A year, month, day, hour, minute, second, and subsecond |
| [Time](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type) | `TIME` | A time, independent of a specific date |
| [Timestamp](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type) | `TIMESTAMP` | An absolute point in time, with microsecond precision |
| [Struct (Record)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type) | `STRUCT` | Container of ordered fields each with a type (required) and field name (optional) |
| [Geography](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#geography_type) | `GEOGRAPHY` | A pointset on the Earth's surface (a set of points, lines and polygons on the [WGS84](http://earth-info.nga.mil/GandG/update/index.php) reference spheroid, with geodesic edges) |
| [JSON](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type) | `JSON` | Represents JSON, a lightweight data-interchange format |
| [RANGE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range) | `RANGE` | A range of `DATE`, `DATETIME`, or `TIMESTAMP` values |

For more information about data types in GoogleSQL, see
[GoogleSQL data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types).

You can also declare an array type when you query data. For more information,
see [Work with arrays](https://docs.cloud.google.com/bigquery/docs/arrays).

### Modes

BigQuery supports the following modes for your columns. Mode is
optional. If the mode is unspecified, the column defaults to `NULLABLE`.

| Mode | Description |
|---|---|
| Nullable | Column allows `NULL` values (default) |
| Required | `NULL` values are not allowed |
| Repeated | Column contains an array of values of the specified type |

For more information about modes, see `mode` in the [`TableFieldSchema`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableFieldSchema).

### Rounding mode

When a column is a `NUMERIC` or `BIGNUMERIC` type, you can set the
[`rounding_mode` column option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#column_option_list),
which determines how values in that column are rounded when written to the
table. You can set the `rounding_mode` option on a top-level column or a `STRUCT`
field. The following rounding modes are supported:

- `"ROUND_HALF_AWAY_FROM_ZERO"`: This mode (default) rounds halfway cases away from zero.
- `"ROUND_HALF_EVEN"`: This mode rounds halfway cases towards the nearest even digit.

You cannot set the `rounding_mode` option for a column that is not a `NUMERIC`
or `BIGNUMERIC` type. To learn more about these types, see
[decimal types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types).

The following example creates a table and inserts values that are rounded
based on the rounding mode of the column:

```googlesql
CREATE TABLE mydataset.mytable (
  x NUMERIC(5,2) OPTIONS (rounding_mode='ROUND_HALF_EVEN'),
  y NUMERIC(5,2) OPTIONS (rounding_mode='ROUND_HALF_AWAY_FROM_ZERO')
);
INSERT mydataset.mytable (x, y)
VALUES (NUMERIC "1.025", NUMERIC "1.025"),
       (NUMERIC "1.0251", NUMERIC "1.0251"),
       (NUMERIC "1.035", NUMERIC "1.035"),
       (NUMERIC "-1.025", NUMERIC "-1.025");
```

The table `mytable` looks like the following:

```
+---+---+
| x     | y     |
+---+---+
| 1.02  | 1.03  |
| 1.03  | 1.03  |
| 1.04  | 1.04  |
| -1.02 | -1.03 |
+---+---+
```

For more information, see `roundingMode` in the
[`TableFieldSchema`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableFieldSchema).

## Specify schemas

When you load data or create an empty table, you can specify the
table's schema using the Google Cloud console or the bq command-line tool. Specifying a
schema is supported when you load CSV and JSON (newline delimited)
files. When you load Avro, Parquet, ORC, Firestore export data, or
Datastore export data, the schema is automatically retrieved from the
self-describing source data.

To specify a table schema:

### Console

In the Google Cloud console, you can specify a schema using the **Add
field** option or the **Edit as text** option.

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Datasets**, and then click your dataset.

4. In the details pane, click **Create table**
   .

5. On the **Create table** page, in the **Source** section, select
   **Empty table**.

6. On the **Create table** page, in the **Destination** section:

   - For **Dataset name**, choose the appropriate dataset

     ![Select dataset.](https://docs.cloud.google.com/static/bigquery/images/create-table-select-dataset.png)
   - In the **Table name** field, enter the name of the table you're
     creating.

   - Verify that **Table type** is set to **Native table**.

7. In the **Schema** section, enter the [schema](https://docs.cloud.google.com/bigquery/docs/schemas)
   definition.

   - Option 1: Use **Add field** and specify each field's name, [type](https://docs.cloud.google.com/bigquery/docs/schemas#standard_sql_data_types), and [mode](https://docs.cloud.google.com/bigquery/docs/schemas#modes).
   - Option 2: Click **Edit as text** and paste the schema in the form of a JSON array. When you use a JSON array, you generate the schema using the same process as [creating a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).
8. Click **Create table**.

### SQL

Use the
[`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement).
Specify the schema using the
[column](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#column_name_and_column_schema)
option.
The following example creates a new table named `newtable` with columns
x, y, z of types integer, string, and boolean:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE TABLE IF NOT EXISTS mydataset.newtable (x INT64, y STRING, z BOOL)
     OPTIONS(
       description = 'My example table');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Supply the schema inline in the format
`field:data_type,field:data_type` using one of the
following commands:

- If you're loading data, use the `bq load` command.
- If you're creating an empty table, use the `bq mk` command.

When you specify the schema on the command line, you can't include
`RECORD` ([`STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type))
or [`RANGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions)
types, you can't include a column description, and you
can't specify the column's mode. All modes default to `NULLABLE`. To
include descriptions, modes, `RECORD` types, and `RANGE` types, supply a
[JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file) instead.

To load data into a table using an inline schema definition, enter the
`load` command and specify the data format using the `--source_format` flag.
If you are loading data into a table in a project other than your default
project, include the project ID in the following format:
`project_id:dataset.table_name`.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

```bash
bq --location=location load \
--source_format=format \
project_id:dataset.table_name \
path_to_source \
schema
```

Replace the following:

- `location`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `format`: `NEWLINE_DELIMITED_JSON` or `CSV`.
- `project_id`: your project ID.
- `dataset`: the dataset that contains the table into which you're loading data.
- `table_name`: the name of the table into which you're loading data.
- `path_to_source`: the location of the CSV or JSON data file on your local machine or in Cloud Storage.
- `schema`: the inline schema definition.

Example:

Enter the following command to load data from a local CSV file named
`myfile.csv` into `mydataset.mytable` in your default project. The schema is
specified inline.

    bq load \
    --source_format=CSV \
    mydataset.mytable \
    ./myfile.csv \
    qtr:STRING,sales:FLOAT,year:STRING

For more information about loading data into BigQuery, see
[Introduction to loading data](https://docs.cloud.google.com/bigquery/docs/loading-data).

To specify an inline schema definition when you create an empty table, enter
the `bq mk` command with the `--table` or `-t` flag. If you are creating
a table in a project other than your default project, add the project ID to
the command in the following format:
`project_id:dataset.table`.

```bash
bq mk --table project_id:dataset.table schema
```

Replace the following:

- `project_id`: your project ID.
- `dataset`: a dataset in your project.
- `table`: the name of the table you're creating.
- `schema`: an inline schema definition.

For example, the following command creates an empty table named `mytable` in
your default project. The schema is specified inline.

    bq mk --table mydataset.mytable qtr:STRING,sales:FLOAT,year:STRING

For more information about creating an empty table, see
[Creating an empty table with a schema definition](https://docs.cloud.google.com/bigquery/docs/tables#create_an_empty_table_with_a_schema_definition).

### C#

To specify a table's schema when you load data into a table:


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

    public class BigQueryLoadTableGcsJson
    {
        public void LoadTableGcsJson(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            var gcsURI = "gs://cloud-samples-data/bigquery/us-states/us-states.json";
            var dataset = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetDataset_Google_Apis_Bigquery_v2_Data_DatasetReference_Google_Cloud_BigQuery_V2_GetDatasetOptions_(datasetId);
            var schema = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.TableSchemaBuilder.html {
                { "name", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String },
                { "post_abbr", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String }
            }.Build();
            TableReference destinationTableRef = dataset.GetTableReference(
                tableId: "us_states");
            // Create job configuration
            var jobOptions = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.CreateLoadJobOptions.html()
            {
                SourceFormat = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.FileFormat.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.FileFormat.html#Google_Cloud_BigQuery_V2_FileFormat_NewlineDelimitedJson
            };
            // Create and run job
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html loadJob = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_CreateLoadJob_System_Collections_Generic_IEnumerable_System_String__Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableSchema_Google_Cloud_BigQuery_V2_CreateLoadJobOptions_(
                sourceUri: gcsURI, destination: destinationTableRef,
                schema: schema, options: jobOptions);
            loadJob = loadJob.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();  // Waits for the job to complete.
            // Display the number of rows uploaded
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html table = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetTable_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_GetTableOptions_(destinationTableRef);
            Console.WriteLine(
                $"Loaded {table.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_Resource.NumRows} rows to {table.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_FullyQualifiedId}");
        }
    }

To specify a schema when you create an empty table:


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;

    public class BigQueryCreateTable
    {
        public BigQueryTable CreateTable(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            var dataset = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetDataset_Google_Apis_Bigquery_v2_Data_DatasetReference_Google_Cloud_BigQuery_V2_GetDatasetOptions_(datasetId);
            // Create schema for new table.
            var schema = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.TableSchemaBuilder.html
            {
                { "full_name", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String },
                { "age", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_Int64 }
            }.Build();
            // Create the table
            return dataset.CreateTable(tableId: "your_table_id", schema: schema);
        }
    }

### Go

To specify a table's schema when you load data into a table:


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

    // importJSONExplicitSchema demonstrates loading newline-delimited JSON data from Cloud Storage
    // into a BigQuery table and providing an explicit schema for the data.
    func importJSONExplicitSchema(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.json")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
    	gcsRef.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "post_abbr", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	}
    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)
    	loader.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty

    	job, err := loader.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}

    	if status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err() != nil {
    		return fmt.Errorf("job completed with error: %v", status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err())
    	}
    	return nil
    }

To specify a schema when you create an empty table:

    import (
    	"context"
    	"fmt"
    	"time"

    	"cloud.google.com/go/bigquery"
    )

    // createTableExplicitSchema demonstrates creating a new BigQuery table and specifying a schema.
    func createTableExplicitSchema(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydatasetid"
    	// tableID := "mytableid"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	sampleSchema := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "full_name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "age", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	}

    	metaData := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		Schema:         sampleSchema,
    		ExpirationTime: time.Now().AddDate(1, 0, 0), // Table will be automatically deleted in 1 year.
    	}
    	tableRef := client.Dataset(datasetID).Table(tableID)
    	if err := tableRef.Create(ctx, metaData); err != nil {
    		return err
    	}
    	return nil
    }

### Java

To specify a table's schema when you load data into a table:


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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to load JSON data from Cloud Storage into a new BigQuery table
    public class LoadJsonFromGCS {

      public static void runLoadJsonFromGCS() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.json";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING));
        loadJsonFromGCS(datasetName, tableName, sourceUri, schema);
      }

      public static void loadJsonFromGCS(
          String datasetName, String tableName, String sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.json())
                  .setSchema(schema)
                  .build();

          // Load data from a GCS JSON file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("Json from GCS successfully loaded in a table");
          } else {
            System.out.println(
                "BigQuery was unable to load into the table due to an error:"
                    + job.getStatus().getError());
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Column not added during load append \n" + e.toString());
        }
      }
    }

To specify a schema when you create an empty table:

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;

    public class CreateTable {

      public static void runCreateTable() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("stringField", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("booleanField", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.BOOL));
        createTable(datasetName, tableName, schema);
      }

      public static void createTable(String datasetName, String tableName, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableDefinition.html tableDefinition = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html.of(schema);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html tableInfo = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html.newBuilder(tableId, tableDefinition).build();

          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(tableInfo);
          System.out.println("Table created successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table was not created. \n" + e.toString());
        }
      }
    }

### Python

To specify a table's schema when you load data into a table, configure the
[LoadJobConfig.schema](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_schema)
property.


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

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        schema=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("name", "STRING"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("post_abbr", "STRING"),
        ],
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.NEWLINE_DELIMITED_JSON,
    )
    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.json"

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri,
        table_id,
        location="US",  # Must match the destination dataset location.
        job_config=job_config,
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

To specify a schema when you create an empty table, configure the
[Table.schema](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_schema)
property.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    schema = [
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("full_name", "STRING", mode="REQUIRED"),
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("age", "INTEGER", mode="REQUIRED"),
    ]

    table = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html(table_id, schema=schema)
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_create_table(table)  # Make an API request.
    print(
        "Created table {}.{}.{}".format(table.project, table.dataset_id, table.table_id)
    )

## Specifying a JSON schema file

If you prefer, you can specify the schema using a JSON schema
file instead of using an inline schema definition. A JSON schema file consists
of a JSON array that contains the following:

- The column's [name](https://docs.cloud.google.com/bigquery/docs/schemas#column_names)
- The column's [data type](https://docs.cloud.google.com/bigquery/docs/schemas#standard_sql_data_types)
- Optional: The column's [mode](https://docs.cloud.google.com/bigquery/docs/schemas#modes) (if unspecified, mode defaults to `NULLABLE`)
- Optional: The column's fields if it is a [`STRUCT` type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type)
- Optional: The column's [description](https://docs.cloud.google.com/bigquery/docs/schemas#column_descriptions)
- Optional: The column's [policy tags](https://docs.cloud.google.com/data-catalog/docs/policy-tags), used for field-level access control
- Optional: The column's maximum length of values for `STRING` or `BYTES` types
- Optional: The column's [precision](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types) for `NUMERIC` or `BIGNUMERIC` types
- Optional: The column's [scale](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types) for `NUMERIC` or `BIGNUMERIC` types
- Optional: The column's [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts) for `STRING` types
- Optional: The column's [default value](https://docs.cloud.google.com/bigquery/docs/default-values)
- Optional: The column's [rounding mode](https://docs.cloud.google.com/bigquery/docs/schemas#rounding_mode), if the column is a `NUMERIC` or `BIGNUMERIC` type

> [!NOTE]
> **Note:** You can also specify the JSON array that you create in your schema file by using the Google Cloud console **Edit as Text** option. It can also be used as a starting point for configuring the [`schema`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#tableschema) property in the API.

### Creating a JSON schema file

To create a JSON schema file, enter a
[`TableFieldSchema`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableFieldSchema)
for each column. The `name` and `type` fields are required. All other fields are
optional.

```json
[
  {
    "name": string,
    "type": string,
    "mode": string,
    "fields": [
      {
        object (TableFieldSchema)
      }
    ],
    "description": string,
    "policyTags": {
      "names": [
        string
      ]
    },
    "maxLength": string,
    "precision": string,
    "scale": string,
    "collation": string,
    "defaultValueExpression": string,
    "roundingMode": string
  },
  {
    "name": string,
    "type": string,
    ...
  }
]
```

If the column is a `RANGE<T>` type, use the `rangeElementType` field to
describe `T`, where `T` must be one of `DATE`, `DATETIME`, or `TIMESTAMP`.

```json
[
  {
    "name": "duration",
    "type": "RANGE",
    "mode": "NULLABLE",
    "rangeElementType": {
      "type": "DATE"
    }
  }
]
```

The JSON array is indicated by the beginning and ending brackets `[]`. Each
column entry must be separated by a comma: `},`.

To write an existing table schema to a local file, do the following:

### bq

```bash
bq show \
--schema \
--format=prettyjson \
project_id:dataset.table > path_to_file
```

Replace the following:

- `project_id`: your project ID.
- `dataset`: a dataset in your project.
- `table`: the name of an existing table schema.
- `path_to_file`: the location of the local file into which you are writing table schema.

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

To write a schema JSON file from a table using the Python client library, call the [Client.schema_to_json](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_schema_to_json) method.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(dev): Change the table_id variable to the full name of the
    # table you want to get schema from.
    table_id = "your-project.your_dataset.your_table_name"

    # TODO(dev): Change schema_path variable to the path
    # of your schema file.
    schema_path = "path/to/schema.json"
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.

    # Write a schema file to schema_path with the schema_to_json method.
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_schema_to_json(table.schema, schema_path)

    with open(schema_path, "r", encoding="utf-8") as schema_file:
        schema_contents = schema_file.read()

    # View table properties
    print(f"Got table '{table.project}.{table.dataset_id}.{table.table_id}'.")
    print(f"Table schema: {schema_contents}")

You can use the output file as a starting point for your own JSON schema file.
If you use this approach, ensure the file contains only the JSON array
that represents the table's schema.

For example, the following JSON array represents a basic table schema. This
schema has three columns: `qtr` (`REQUIRED` `STRING`), `rep` (`NULLABLE` `STRING`),
and `sales` (`NULLABLE` `FLOAT`).

```json
[
  {
    "name": "qtr",
    "type": "STRING",
    "mode": "REQUIRED",
    "description": "quarter"
  },
  {
    "name": "rep",
    "type": "STRING",
    "mode": "NULLABLE",
    "description": "sales representative"
  },
  {
    "name": "sales",
    "type": "FLOAT",
    "mode": "NULLABLE",
    "defaultValueExpression": "2.55"
  }
]
```

### Using a JSON schema file

After you create your JSON schema file, you can specify it using the bq command-line tool.
You can't use a schema file with the Google Cloud console or the API.

Supply the schema file:

- If you're loading data, use the `bq load` command.
- If you're creating an empty table, use the `bq mk` command.

When you supply a JSON schema file, it must be stored in a locally readable
location. You cannot specify a JSON schema file stored in Cloud Storage or
Google Drive.

#### Specifying a schema file when you load data

To load data into a table using a JSON schema definition, do the following:

### bq

```bash
bq --location=location load \
--source_format=format \
project_id:dataset.table \
path_to_data_file \
path_to_schema_file
```

Replace the following:

- `location`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `format`: `NEWLINE_DELIMITED_JSON` or `CSV`.
- `project_id`: your project ID.
- `dataset`: the dataset that contains the table into which you're loading data.
- `table`: the name of the table into which you're loading data.
- `path_to_data_file`: the location of the CSV or JSON data file on your local machine or in Cloud Storage.
- `path_to_schema_file`: the path to the schema file on your local machine.

Example:

Enter the following command to load data from a local CSV file named
`myfile.csv` into `mydataset.mytable` in your default project. The schema is
specified in `myschema.json` in the current directory.

```bash
bq load --source_format=CSV mydataset.mytable ./myfile.csv ./myschema.json
```

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

To load a table schema from a JSON file using the Python client library, call the [schema_from_json](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_schema_from_json) method.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(dev): Change uri variable to the path of your data file.
    uri = "gs://your-bucket/path/to/your-file.csv"
    # TODO(dev): Change table_id to the full name of the table you want to create.
    table_id = "your-project.your_dataset.your_table"
    # TODO(dev): Change schema_path variable to the path of your schema file.
    schema_path = "path/to/schema.json"
    # To load a schema file use the schema_from_json method.
    schema = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_schema_from_json(schema_path)

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        # To use the schema you loaded pass it into the
        # LoadJobConfig constructor.
        schema=schema,
        skip_leading_rows=1,
    )

    # Pass the job_config object to the load_table_from_file,
    # load_table_from_json, or load_table_from_uri method
    # to use the schema on a new table.
    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    print(f"Loaded {destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows} rows to {table_id}.")

#### Specifying a schema file when you create a table

To create an empty table in an existing dataset using a JSON schema file, do the following:

### bq

```bash
bq mk --table project_id:dataset.table path_to_schema_file
```

Replace the following:

- `project_id`: your project ID.
- `dataset`: a dataset in your project.
- `table`: the name of the table you're creating.
- `path_to_schema_file`: the path to the schema file on your local machine.

For example, the following command creates a table named `mytable` in
`mydataset` in your default project. The schema is specified in `myschema.json` in the current directory:

```bash
bq mk --table mydataset.mytable ./myschema.json
```

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

To load a table schema from a JSON file using the Python client library, call the [schema_from_json](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_schema_from_json) method.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(dev): Change table_id to the full name of the table you want to create.
    table_id = "your-project.your_dataset.your_table_name"
    # TODO(dev): Change schema_path variable to the path of your schema file.
    schema_path = "path/to/schema.json"
    # To load a schema file use the schema_from_json method.
    schema = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_schema_from_json(schema_path)

    table = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html(table_id, schema=schema)
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_create_table(table)  # API request
    print(f"Created table {table_id}.")

## Specifying a schema in the API

Specify a table schema using the API:

- To specify a schema when you load data, call the
  [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) method and
  configure the
  [`schema`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.schema)
  property in the
  [`JobConfigurationLoad`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationload)
  resource.

- To specify a schema when you create a table, call the
  [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert) method and
  configure the
  [`schema`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table.FIELDS.schema)
  property in the [`Table`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#resource:-table)
  resource.

Specifying a schema using the API is similar to the process for
[Creating a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#creating_a_JSON_schema_file).

## Table security

To control access to tables in BigQuery, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

## What's next

- Learn how to specify [nested and repeated columns](https://docs.cloud.google.com/bigquery/docs/nested-repeated) in a schema definition.
- Learn about [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect).
- Learn about [loading data](https://docs.cloud.google.com/bigquery/docs/loading-data) into BigQuery.
- Learn about [creating and using tables](https://docs.cloud.google.com/bigquery/docs/tables).