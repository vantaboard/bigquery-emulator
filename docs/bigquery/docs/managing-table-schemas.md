# Modifying table schemas

This document describes how to modify the schema definitions for existing
BigQuery tables.

You can make most schema modifications described in this document by using SQL
[data definition language (DDL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language).
These statements don't incur charges.

You can modify a table schema in all the ways described on this page by
[exporting](https://docs.cloud.google.com/bigquery/docs/exporting-data) your table data to
Cloud Storage,
and then [loading](https://docs.cloud.google.com/bigquery/docs/loading-data)
the data into a new table with the modified schema definition.
BigQuery load and extract jobs are
free, but you incur costs for storing the exported data in
Cloud Storage.
The following sections describe other ways of performing various types of
schema modifications.

Schema updates in BigQuery don't cause data loss.

> [!NOTE]
> **Note:** When you update a schema, the changes might not be immediately reflected in the [`INFORMATION_SCHEMA.TABLES`](https://docs.cloud.google.com/bigquery/docs/information-schema-tables) and [`INFORMATION_SCHEMA.COLUMNS`](https://docs.cloud.google.com/bigquery/docs/information-schema-columns) views. To view immediate schema changes, call the [`tables.get` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get).

## Add a column

You can add columns to an existing table's schema definition by using one of the
following options:

- Add a new empty column.
- Overwrite a table with a load or query job.
- Append data to a table with a load or query job.

Any column you add must adhere to BigQuery's rules for
[column names](https://docs.cloud.google.com/bigquery/docs/schemas#column_names). For more information on
creating schema components, see [Specifying a schema](https://docs.cloud.google.com/bigquery/docs/schemas).

It isn't possible to add columns in the middle of a table schema. New columns
and nested fields are always added at the end of the table or field. The only
way to create a new column in the middle of a table schema is to create a new
table with the chosen schema and copy the data from the original table.

### Add an empty column

If you add new columns to an existing table schema, the columns must be
`NULLABLE` or `REPEATED`. You cannot add a `REQUIRED` column to an existing
table schema. Adding a `REQUIRED` column to an existing table
schema in the API or bq command-line tool causes an error. However, you can create a
nested `REQUIRED` column as part of a new `RECORD` field.
`REQUIRED` columns can be added only when you
create a table while loading data, or when you create an empty table with a
schema definition.

To add empty columns to a table's schema definition:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Overview \> Tables**, and then select the table.

5. In the details pane, click the **Schema** tab.

6. Click **Edit schema**. You might need to scroll to see this button.

7. In the **Current schema** page, under **New fields** , click **Add
   field**.

   - For **Name**, type the column name.
   - For **Type** , choose the [data type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types).
   - For [**Mode**](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableFieldSchema.FIELDS.mode), choose `NULLABLE` or `REPEATED`.
8. When you are done adding columns, click **Save**.

> [!NOTE]
> **Note:** You can't use the Google Cloud console to add a column to an [external table](https://docs.cloud.google.com/bigquery/docs/external-tables).

### SQL

Use the
[`ALTER TABLE ADD COLUMN` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_column_statement):

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
   ADD COLUMN new_column STRING;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

> [!NOTE]
> **Note:** You can't use the `ALTER TABLE ADD COLUMN` statement to add a column to an [external table](https://docs.cloud.google.com/bigquery/docs/external-tables).

### bq

Issue the `bq update` command and provide a JSON schema file. If the table
you're updating is in a project other than your default project, add the
project ID to the dataset name in the following format:
`PROJECT_ID:DATASET`.

```bash
bq update PROJECT_ID:DATASET.TABLE SCHEMA
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET`: the name of the dataset that contains the table you're updating.
- `TABLE`: the name of the table you're updating.
- `SCHEMA`: the path to the JSON schema file on your local machine.

When you specify an inline schema, you cannot specify the column
description, mode, and `RECORD` ([`STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type))
type. All column modes default to `NULLABLE`. As a result, if you are
adding a new nested column to a `RECORD`, you must
[supply a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).

If you attempt to add columns using an inline schema definition, you must
supply the entire schema definition including the new columns. Because you
cannot specify column modes using an inline schema definition, the update
changes any existing `REPEATED` column to `NULLABLE`, which
produces the following error: `BigQuery error in update
operation: Provided Schema does not match Table
PROJECT_ID:dataset.table. Field field has changed mode
from REPEATED to NULLABLE.`

The preferred method of adding columns to an existing table using the bq command-line tool is
to [supply a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).

To add empty columns to a table's schema using a JSON schema file:

1. First, issue the `bq show` command with the `--schema` flag and write the
   existing table schema to a file. If the table you're updating is in a
   project other than your default project, add the project ID to the
   dataset name in the following format: `PROJECT_ID:DATASET`.

   ```bash
   bq show \
   --schema \
   --format=prettyjson \
   PROJECT_ID:DATASET.TABLE > SCHEMA
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET`: the name of the dataset that contains the table you're updating.
   - `TABLE`: the name of the table you're updating.
   - `SCHEMA`: the schema definition file written to your local machine.

   For example, to write the schema definition of `mydataset.mytable` to a
   file, enter the following command. `mydataset.mytable` is in your
   default project.

          bq show \
          --schema \
          --format=prettyjson \
          mydataset.mytable > /tmp/myschema.json

2. Open the schema file in a text editor. The schema should look like the
   following:

   ```
   [
     {
       "mode": "REQUIRED",
       "name": "column1",
       "type": "STRING"
     },
     {
       "mode": "REQUIRED",
       "name": "column2",
       "type": "FLOAT"
     },
     {
       "mode": "REPEATED",
       "name": "column3",
       "type": "STRING"
     }
   ]
   ```
3. Add the new columns to the end of the schema definition. If you attempt
   to add new columns elsewhere in the array, the following error is
   returned: `BigQuery error in update operation: Precondition
   Failed`. Modifying schema order after table creation doesn't have an
   effect on column or nested field order.

   Using a JSON file, you can specify descriptions, `NULLABLE` or
   `REPEATED` modes, and `RECORD` types for new columns. For example,
   using the schema definition from the previous step, your new JSON array
   would look like the following. In this example, a new `NULLABLE` column
   is added named `column4`. `column4` includes a description.

   <br />

   ```
     [
       {
         "mode": "REQUIRED",
         "name": "column1",
         "type": "STRING"
       },
       {
         "mode": "REQUIRED",
         "name": "column2",
         "type": "FLOAT"
       },
       {
         "mode": "REPEATED",
         "name": "column3",
         "type": "STRING"
       },
       {
         "description": "my new column",
         "mode": "NULLABLE",
         "name": "column4",
         "type": "STRING"
       }
     ]
     
   ```

   <br />

   For more information on working with JSON schema files, see
   [Specifying a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).
4. After updating your schema file, issue the following command to update
   the table's schema. If the table you're updating is in a project other
   than your default project, add the project ID to the dataset name in the
   following format: `PROJECT_ID:DATASET`.

   ```bash
   bq update PROJECT_ID:DATASET.TABLE SCHEMA
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET`: the name of the dataset that contains the table you're updating.
   - `TABLE`: the name of the table you're updating.
   - `SCHEMA`: the schema definition file written to your local machine.

   For example, enter the following command to update the schema definition
   of `mydataset.mytable` in your default project. The path to the schema
   file on your local machine is `/tmp/myschema.json`.

       bq update mydataset.mytable /tmp/myschema.json

### API

Call the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and use the `schema` property to add empty columns to your schema
definition. Because the `tables.update` method replaces the entire table
resource, the `tables.patch` method is preferred.

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

    // updateTableAddColumn demonstrates modifying the schema of a table to append an additional column.
    func updateTableAddColumn(projectID, datasetID, tableID string) error {
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
    	newSchema := append(meta.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema,
    		&bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_FieldSchema{Name: "phone", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	)
    	update := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadataToUpdate{
    		Schema: newSchema,
    	}
    	if _, err := tableRef.Update(ctx, update, meta.ETag); err != nil {
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FieldList.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;
    import java.util.ArrayList;
    import java.util.List;

    public class AddEmptyColumn {

      public static void runAddEmptyColumn() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableId = "MY_TABLE_NAME";
        String newColumnName = "NEW_COLUMN_NAME";
        addEmptyColumn(newColumnName, datasetName, tableId);
      }

      public static void addEmptyColumn(String newColumnName, String datasetName, String tableId) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(datasetName, tableId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema = table.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html#com_google_cloud_bigquery_TableInfo__T_getDefinition__().getSchema();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FieldList.html fields = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.jdbc.BigQueryBaseResultSet.html#com_google_cloud_bigquery_jdbc_BigQueryBaseResultSet_schema.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html#com_google_cloud_bigquery_Schema_getFields__();

          // Create the new field/column
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html newField = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of(newColumnName, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html.STRING);

          // Create a new schema adding the current fields, plus the new one
          List<Field> fieldList = new ArrayList<Field>();
          fields.forEach(fieldList::add);
          fieldList.add(newField);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html newSchema = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(fieldList);

          // Update the table with the new schema
          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html updatedTable =
              table.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html#com_google_cloud_bigquery_biglake_v1_Table_toBuilder__().setDefinition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html.of(newSchema)).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Table.html#com_google_cloud_bigquery_Table_update_com_google_cloud_bigquery_BigQuery_TableOption____dTable.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Table.html#com_google_cloud_bigquery_Table_update_com_google_cloud_bigquery_BigQuery_TableOption____();
          System.out.println("Empty column successfully added to table");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Empty column was not added. \n" + e.toString());
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


    // Import the Google Cloud client library and create a client
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function addEmptyColumn() {
      // Adds an empty column to the schema.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';
      const column = {name: 'size', type: 'STRING'};

      // Retrieve current table metadata
      const table = bigquery.dataset(datasetId).table(tableId);
      const [metadata] = await table.getMetadata();

      // Update table schema
      const schema = metadata.schema;
      const new_schema = schema;
      new_schema.fields.push(column);
      metadata.schema = new_schema;

      const [result] = await table.setMetadata(metadata);
      console.log(https://docs.cloud.google.com/nodejs/docs/reference/bigquery-analyticshub/latest/bigquery-analyticshub/protos.google.longrunning.operation.html.schema.fields);
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

Append a new [SchemaField](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField) object to a copy of the [Table.schema](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_schema) and then replace the value of the [Table.schema](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_schema) property with the updated schema.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table
    #                  to add an empty column.
    # table_id = "your-project.your_dataset.your_table_name"

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.

    original_schema = table.schema
    new_schema = original_schema[:]  # Creates a copy of the schema.
    new_schema.append(https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("phone", "STRING"))

    table.schema = new_schema
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_table(table, ["schema"])  # Make an API request.

    if len(table.schema) == len(original_schema) + 1 == len(new_schema):
        print("A new column has been added.")
    else:
        print("The column has not been added.")

<br />

### Add a nested column to a `RECORD` column

In addition to adding new columns to a table's schema, you can also add new
nested columns to a `RECORD` column. The process for adding a new nested column is
similar to the process for adding a new column.

### Console

Adding a new nested field to an existing `RECORD` column is not
supported by the Google Cloud console.

### SQL

Adding a new nested field to an existing `RECORD` column by using a SQL DDL
statement is not supported.

### bq

Issue the `bq update` command and provide a JSON schema file that adds the
nested field to the existing `RECORD` column's schema definition. If the
table you're updating is in a project other than your default project, add
the project ID to the dataset name in the following format:
`PROJECT_ID:DATASET`.

```bash
bq update PROJECT_ID:DATASET.TABLE SCHEMA
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET`: the name of the dataset that contains the table you're updating.
- `TABLE`: the name of the table you're updating.
- `SCHEMA` : the path to the JSON schema file on your local machine.

When you specify an inline schema, you cannot specify the column
description, mode, and `RECORD` ([`STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type))
type. All column modes default to `NULLABLE`. As a result, if you are
adding a new nested column to a `RECORD`, you must
[supply a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).

To add a nested column to a `RECORD` using a JSON schema file:

1. First, issue the `bq show` command with the `--schema` flag and write the
   existing table schema to a file. If the table you're updating is in a
   project other than your default project, add the project ID to the
   dataset name in the following format:
   `PROJECT_ID:DATASET.TABLE`.

   ```bash
   bq show \
   --schema \
   --format=prettyjson \
   PROJECT_ID:DATASET.TABLE > SCHEMA
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET`: the name of the dataset that contains the table you're updating.
   - `TABLE`: the name of the table you're updating.
   - `SCHEMA`: the schema definition file written to your local machine.

   For example, to write the schema definition of `mydataset.mytable` to a
   file, enter the following command. `mydataset.mytable` is in your
   default project.

       bq show \
       --schema \
       --format=prettyjson \
       mydataset.mytable > /tmp/myschema.json

2. Open the schema file in a text editor. The schema should look like the
   following. In this example, `column3` is a nested repeated column. The
   nested columns are `nested1` and `nested2`. The `fields` array lists
   the fields nested within `column3`.

   ```
   [
     {
       "mode": "REQUIRED",
       "name": "column1",
       "type": "STRING"
     },
     {
       "mode": "REQUIRED",
       "name": "column2",
       "type": "FLOAT"
     },
     {
       "fields": [
         {
           "mode": "NULLABLE",
           "name": "nested1",
           "type": "STRING"
         },
         {
           "mode": "NULLABLE",
           "name": "nested2",
           "type": "STRING"
         }
       ],
       "mode": "REPEATED",
       "name": "column3",
       "type": "RECORD"
     }
   ]
   ```
3. Add the new nested column to the end of the `fields` array. Nested
   fields are always added at the end of the field. In this example,
   `nested3` is the new nested column.

   <br />

   ```
     [
       {
         "mode": "REQUIRED",
         "name": "column1",
         "type": "STRING"
       },
       {
         "mode": "REQUIRED",
         "name": "column2",
         "type": "FLOAT"
       },
       {
         "fields": [
           {
             "mode": "NULLABLE",
             "name": "nested1",
             "type": "STRING"
           },
           {
             "mode": "NULLABLE",
             "name": "nested2",
             "type": "STRING"
           },
           {
             "mode": "NULLABLE",
             "name": "nested3",
             "type": "STRING"
           }
         ],
         "mode": "REPEATED",
         "name": "column3",
         "type": "RECORD"
       }
     ]
     
   ```

   <br />

   For more information on working with JSON schema files, see
   [Specifying a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).
4. After updating your schema file, issue the following command to update
   the table's schema. If the table you're updating is in a project other
   than your default project, add the project ID to the dataset name in the
   following format:
   `PROJECT_ID:DATASET`.

   ```bash
   bq update PROJECT_ID:DATASET.TABLE SCHEMA
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET`: the name of the dataset that contains the table you're updating.
   - `TABLE`: the name of the table you're updating.
   - `SCHEMA`: the path to the JSON schema file on your local machine.

   For example, enter the following command to update the schema definition
   of `mydataset.mytable` in your default project. The path to the schema
   file on your local machine is `/tmp/myschema.json`.

       bq update mydataset.mytable /tmp/myschema.json

### API

Call the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and use the `schema` property to add the nested columns to your
schema definition. Because the `tables.update` method replaces the entire
table resource, the `tables.patch` method is preferred.

### Add columns when you overwrite or append data

You can add new columns to an existing table when you load data into it and
choose to overwrite the existing table. When you overwrite an existing table,
the schema of the data you're loading is used to overwrite the existing table's
schema. For information on overwriting a table using a load job, see the document
for your data's format:

- [Avro](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#appending_to_or_overwriting_a_table_with_avro_data)
- [Parquet](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#appending_to_or_overwriting_a_table_with_parquet_data)
- [ORC](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-orc#append_to_or_overwrite_a_table_with_orc_data)
- [CSV](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#appending_to_or_overwriting_a_table_with_csv_data)
- [JSON](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#appending_to_or_overwriting_a_table_with_json_data)

#### Add columns in a load append job

You can add columns to a table when you append data to it in a load job. The
new schema is determined by one of the following:

- Autodetection (for CSV and JSON files)
- A schema specified in a JSON schema file (for CSV and JSON files)
- The self-describing source data for Avro, ORC, Parquet and Datastore export files

If you specify the schema in a JSON file, the new columns must be defined in it.
If the new column definitions are missing, an error is returned when
you attempt to append the data.

When you add new columns during an append operation,
the values in the new columns are set to `NULL` for existing rows.

To add a new column when you append data to a table during a load job, use
one of the following options:

### bq

Use the `bq load` command to load your data and specify the `--noreplace`
flag to indicate that you are appending the data to an existing table.

If the data you're appending is in CSV or newline-delimited JSON format,
specify the `--autodetect` flag to use [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect)
or supply the schema in a JSON schema file. The added columns can be
automatically inferred from Avro or Datastore export files.

Set the `--schema_update_option` flag to `ALLOW_FIELD_ADDITION` to indicate
that the data you're appending contains new columns.

If the table you're appending is in a dataset in a project other than your
default project, add the project ID to the dataset name in the following
format: `PROJECT_ID:DATASET`.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

Enter the `load` command as follows:

```bash
bq --location=LOCATION load \
--noreplace \
--autodetect \
--schema_update_option=ALLOW_FIELD_ADDITION \
--source_format=FORMAT \
PROJECT_ID:DATASET.TABLE \
PATH_TO_SOURCE \
SCHEMA
```

Replace the following:

- `LOCATION`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: the format of the schema. `NEWLINE_DELIMITED_JSON`, `CSV`, `AVRO`, `PARQUET`, `ORC`, or `DATASTORE_BACKUP`.
- `PROJECT_ID`: your project ID.
- `DATASET`: the name of the dataset that contains the table you're updating.
- `TABLE`: the name of the table you're appending.
- `PATH_TO_SOURCE`: a fully-qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri), a comma-separated list of URIs, or the path to a data file on your local machine.
- `SCHEMA`: the path to a local JSON schema file. A schema file is required only for CSV and JSON files when `--autodetect` is unspecified. Avro and Datastore schemas are inferred from the source data.

Examples:

Enter the following command to append a local Avro data file,
`/tmp/mydata.avro`, to `mydataset.mytable` using a load job. Because schemas
can be automatically inferred from Avro data you don't need to use
the `--autodetect` flag. `mydataset` is in your default project.

    bq load \
    --noreplace \
    --schema_update_option=ALLOW_FIELD_ADDITION \
    --source_format=AVRO \
    mydataset.mytable \
    /tmp/mydata.avro

Enter the following command append a newline-delimited JSON data file in
Cloud Storage to `mydataset.mytable` using a load job. The `--autodetect`
flag is used to detect the new columns. `mydataset` is in your default
project.

    bq load \
    --noreplace \
    --autodetect \
    --schema_update_option=ALLOW_FIELD_ADDITION \
    --source_format=NEWLINE_DELIMITED_JSON \
    mydataset.mytable \
    gs://mybucket/mydata.json

Enter the following command append a newline-delimited JSON data file in
Cloud Storage to `mydataset.mytable` using a load job. The schema
containing the new columns is specified in a local JSON schema file,
`/tmp/myschema.json`. `mydataset` is in `myotherproject`, not your default
project.

    bq load \
    --noreplace \
    --schema_update_option=ALLOW_FIELD_ADDITION \
    --source_format=NEWLINE_DELIMITED_JSON \
    myotherproject:mydataset.mytable \
    gs://mybucket/mydata.json \
    /tmp/myschema.json

### API

Call the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method. Configure a `load` job and set the following properties:

- Reference your data in Cloud Storage using the `sourceUris` property.
- Specify the data format by setting the `sourceFormat` property.
- Specify the schema in the `schema` property.
- Specify the schema update option using the `schemaUpdateOptions` property.
- Set the write disposition of the destination table to `WRITE_APPEND` using the `writeDisposition` property.

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
    	"os"

    	"cloud.google.com/go/bigquery"
    )

    // createTableAndWidenLoad demonstrates augmenting a table's schema to add a new column via a load job.
    func createTableAndWidenLoad(projectID, datasetID, tableID, filename string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	sampleSchema := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "full_name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	}
    	meta := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		Schema: sampleSchema,
    	}
    	tableRef := client.Dataset(datasetID).Table(tableID)
    	if err := tableRef.Create(ctx, meta); err != nil {
    		return err
    	}
    	// Now, import data from a local file, but specify field additions are allowed.
    	// Because the data has a second column (age), the schema is amended as part of
    	// the load.
    	f, err := os.Open(filename)
    	if err != nil {
    		return err
    	}
    	source := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_ReaderSource_NewReaderSource(f)
    	source.AutoDetect = true   // Allow BigQuery to determine schema.
    	source.SkipLeadingRows = 1 // CSV has a single header line.

    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(source)
    	loader.SchemaUpdateOptions = []string{"ALLOW_FIELD_ADDITION"}
    	job, err := loader.Run(ctx)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.SchemaUpdateOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.WriteDisposition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.common.collect.ImmutableList;
    import java.util.UUID;

    public class AddColumnLoadAppend {

      public static void runAddColumnLoadAppend() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "/path/to/file.csv";
        addColumnLoadAppend(datasetName, tableName, sourceUri);
      }

      public static void addColumnLoadAppend(String datasetName, String tableName, String sourceUri)
          throws Exception {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);

          // Add a new column to a BigQuery table while appending rows via a load job.
          // 'REQUIRED' fields cannot  be added to an existing schema, so the additional column must be
          // 'NULLABLE'.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html newSchema =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html.STRING)
                      .setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.REQUIRED)
                      .build(),
                  // Adding below additional column during the load job
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html.STRING)
                      .setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.NULLABLE)
                      .build());

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadJobConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.builder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_csv__())
                  .setWriteDisposition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.WriteDisposition.html.WRITE_APPEND)
                  .setSchema(newSchema)
                  .setSchemaUpdateOptions(ImmutableList.of(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.SchemaUpdateOption.html.ALLOW_FIELD_ADDITION))
                  .build();

          // Create a job ID so that we can safely retry.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html jobId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html.of(UUID.randomUUID().toString());
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html loadJob = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.newBuilder(loadJobConfig).setJobId(jobId).build());

          // Load data from a GCS parquet file into the table
          // Blocks until this load table job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = loadJob.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();

          // Check for errors
          if (completedJob == null) {
            throw new Exception("Job not executed since it no longer exists.");
          } else if (completedJob.getStatus().getError() != null) {
            // You can also look at queryJob.getStatus().getExecutionErrors() for all
            // errors, not just the latest one.
            throw new Exception(
                "BigQuery was unable to load into the table due to an error: \n"
                    + loadJob.getStatus().getError());
          }
          System.out.println("Column successfully added during load append job");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Column not added during load append \n" + e.toString());
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

    // Import the Google Cloud client libraries
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');

    // Instantiate client
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function addColumnLoadAppend() {
      // Adds a new column to a BigQuery table while appending rows via a load job.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const fileName = '/path/to/file.csv';
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      // In this example, the existing table contains only the 'Name', 'Age',
      // & 'Weight' columns. 'REQUIRED' fields cannot  be added to an existing
      // schema, so the additional column must be 'NULLABLE'.
      const schema = 'Name:STRING, Age:INTEGER, Weight:FLOAT, IsMagic:BOOLEAN';

      // Retrieve destination table reference
      const [table] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .get();
      const destinationTableRef = table.metadata.tableReference;

      // Set load job options
      const options = {
        schema: schema,
        schemaUpdateOptions: ['ALLOW_FIELD_ADDITION'],
        writeDisposition: 'WRITE_APPEND',
        destinationTable: destinationTableRef,
      };

      // Load data from a local file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(fileName, options);

      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);
      console.log(`New Schema:`);
      console.log(https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.configuration.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html.schema.fields);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
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

    # from google.cloud import bigquery
    # client = bigquery.Client()
    # project = client.project
    # dataset_ref = bigquery.DatasetReference(project, 'my_dataset')
    # filepath = 'path/to/your_file.csv'

    # Retrieves the destination table and checks the length of the schema
    table_id = "my_table"
    table_ref = dataset_ref.table(table_id)
    table = client.get_table(table_ref)
    print("Table {} contains {} columns.".format(table_id, len(table.schema)))

    # Configures the load job to append the data to the destination table,
    # allowing field addition
    job_config = bigquery.LoadJobConfig()
    job_config.write_disposition = bigquery.WriteDisposition.WRITE_APPEND
    job_config.schema_update_options = [
        bigquery.SchemaUpdateOption.ALLOW_FIELD_ADDITION
    ]
    # In this example, the existing table contains only the 'full_name' column.
    # 'REQUIRED' fields cannot be added to an existing schema, so the
    # additional column must be 'NULLABLE'.
    job_config.schema = [
        bigquery.SchemaField("full_name", "STRING", mode="REQUIRED"),
        bigquery.SchemaField("age", "INTEGER", mode="NULLABLE"),
    ]
    job_config.source_format = bigquery.SourceFormat.CSV
    job_config.skip_leading_rows = 1

    with open(filepath, "rb") as source_file:
        job = client.load_table_from_file(
            source_file,
            table_ref,
            location="US",  # Must match the destination dataset location.
            job_config=job_config,
        )  # API request

    job.result()  # Waits for table load to complete.
    print(
        "Loaded {} rows into {}:{}.".format(
            job.output_rows, dataset_id, table_ref.table_id
        )
    )

    # Checks the updated length of the schema
    table = client.get_table(table)
    print("Table {} now contains {} columns.".format(table_id, len(table.schema)))

<br />

#### Add columns in a query append job

You can add columns to a table when you append query results to it.

When you add columns using an append operation in a query job, the schema of the
query results is used to update the schema of the destination table. Note that
you cannot query a table in one location and write the results to a table in
another location.

To add a new column when you append data to a table during a query job, select
one of the following options:

### bq

Use the `bq query` command to query your data and specify the
`--destination_table` flag to indicate which table you're appending.

To specify that you are appending query results to an existing destination
table, specify the `--append_table` flag.

Set the `--schema_update_option` flag to `ALLOW_FIELD_ADDITION` to indicate
that the query results you're appending contain new columns.

Specify the `use_legacy_sql=false` flag to use GoogleSQL syntax for the
query.

If the table you're appending is in a dataset in a project other than your
default project, add the project ID to the dataset name in the following
format: `PROJECT_ID:DATASET`. Note that the table
you're querying and the destination table must be in the same location.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

```bash
bq --location=LOCATION query \
--destination_table PROJECT_ID:DATASET.TABLE \
--append_table \
--schema_update_option=ALLOW_FIELD_ADDITION \
--use_legacy_sql=false \
'QUERY'
```

Replace the following:

- `LOCATION`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags). Note that you cannot append query results to a table in another location.
- `PROJECT_ID`: your project ID.
- `dataset`: the name of the dataset that contains the table you're appending.
- `TABLE`: the name of the table you're appending.
- `QUERY`: a query in GoogleSQL syntax.

Examples:

Enter the following command to query `mydataset.mytable` in your default
project and to append the query results to `mydataset.mytable2` (also in
your default project).

    bq query \
    --destination_table mydataset.mytable2 \
    --append_table \
    --schema_update_option=ALLOW_FIELD_ADDITION \
    --use_legacy_sql=false \
    'SELECT
       column1,column2
     FROM
       mydataset.mytable'

Enter the following command to query `mydataset.mytable` in your default
project and to append the query results to `mydataset.mytable2` in
`myotherproject`.

    bq query \
    --destination_table myotherproject:mydataset.mytable2 \
    --append_table \
    --schema_update_option=ALLOW_FIELD_ADDITION \
    --use_legacy_sql=false \
    'SELECT
       column1,column2
     FROM
       mydataset.mytable'

### API

Call the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method. Configure a `query` job and set the following properties:

- Specify the destination table using the `destinationTable` property.
- Set the write disposition of the destination table to `WRITE_APPEND` using the `writeDisposition` property.
- Specify the schema update option using the `schemaUpdateOptions` property.
- Specify the GoogleSQL query using the `query` property.

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

    // createTableAndWidenQuery demonstrates how the schema of a table can be modified to add columns by appending
    // query results that include the new columns.
    func createTableAndWidenQuery(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	// First, we create a sample table.
    	sampleSchema := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "full_name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    		{Name: "age", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    	}
    	original := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		Schema: sampleSchema,
    	}
    	tableRef := client.Dataset(datasetID).Table(tableID)
    	if err := tableRef.Create(ctx, original); err != nil {
    		return err
    	}
    	// Our table has two columns.  We'll introduce a new favorite_color column via
    	// a subsequent query that appends to the table.
    	q := client.Query("SELECT \"Timmy\" as full_name, 85 as age, \"Blue\" as favorite_color")
    	q.SchemaUpdateOptions = []string{"ALLOW_FIELD_ADDITION"}
    	q.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryConfig.Dst = client.Dataset(datasetID).Table(tableID)
    	q.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty
    	q.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_Location = "US"
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	_, err = job.Wait(ctx)
    	if err != nil {
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.SchemaUpdateOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.WriteDisposition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import com.google.common.collect.ImmutableList;

    public class RelaxTableQuery {

      public static void runRelaxTableQuery() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        relaxTableQuery(projectId, datasetName, tableName);
      }

      // To relax all columns in a destination table when you append data to it during a query job
      public static void relaxTableQuery(String projectId, String datasetName, String tableName)
          throws Exception {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          TableId tableId = TableId.of(datasetName, tableName);

          String sourceTable = "`" + projectId + "." + datasetName + "." + tableName + "`";
          String query = "SELECT word FROM " + sourceTable + " WHERE word like '%is%'";

          QueryJobConfiguration queryConfig =
              QueryJobConfiguration.newBuilder(query)
                  // Use standard SQL syntax for queries.
                  // See: https://cloud.google.com/bigquery/sql-reference/
                  .setUseLegacySql(false)
                  .setSchemaUpdateOptions(ImmutableList.of(SchemaUpdateOption.ALLOW_FIELD_RELAXATION))
                  .setWriteDisposition(WriteDisposition.WRITE_APPEND)
                  .setDestinationTable(tableId)
                  .build();

          Job queryJob = bigquery.create(JobInfo.newBuilder(queryConfig).build());

          queryJob = queryJob.waitFor();

          // Check for errors
          if (queryJob == null) {
            throw new Exception("Job no longer exists");
          } else if (queryJob.getStatus().getError() != null) {
            // You can also look at queryJob.getStatus().getExecutionErrors() for all
            // errors, not just the latest one.
            throw new Exception(queryJob.getStatus().getError().toString());
          }

          // Get the results.
          TableResult results = queryJob.getQueryResults();

          // Print all pages of the results.
          results
              .iterateAll()
              .forEach(
                  rows -> {
                    rows.forEach(row -> System.out.println("row: " + row.toString()));
                  });

          System.out.println("Successfully relaxed all columns in destination table during query job");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Columns not relaxed during query job \n" + e.toString());
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

    // Import the Google Cloud client libraries
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');

    // Instantiate client
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function addColumnQueryAppend() {
      // Adds a new column to a BigQuery table while appending rows via a query job.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      // Retrieve destination table reference
      const [table] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .get();
      const destinationTableRef = table.metadata.tableReference;

      // In this example, the existing table contains only the 'name' column.
      // 'REQUIRED' fields cannot  be added to an existing schema,
      // so the additional column must be 'NULLABLE'.
      const query = `SELECT name, year
        FROM \`bigquery-public-data.usa_names.usa_1910_2013\`
        WHERE state = 'TX'
        LIMIT 10`;

      // Set load job options
      const options = {
        query: query,
        schemaUpdateOptions: ['ALLOW_FIELD_ADDITION'],
        writeDisposition: 'WRITE_APPEND',
        destinationTable: destinationTableRef,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
      };

      const [job] = await bigquery.createQueryJob(options);
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} started.`);

      // Wait for the query to finish
      const [rows] = await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/job.html();
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Print the results
      console.log('Rows:');
      rows.forEach(row => console.log(row));
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

    # TODO(developer): Set table_id to the ID of the destination table.
    # table_id = "your-project.your_dataset.your_table_name"

    # Retrieves the destination table and checks the length of the schema.
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    print("Table {} contains {} columns".format(table_id, len(table.schema)))

    # Configures the query to append the results to a destination table,
    # allowing field addition.
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        destination=table_id,
        schema_update_options=[https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SchemaUpdateOption.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SchemaUpdateOption.html#google_cloud_bigquery_enums_SchemaUpdateOption_ALLOW_FIELD_ADDITION],
        write_disposition=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html#google_cloud_bigquery_enums_WriteDisposition_WRITE_APPEND,
    )

    # Start the query, passing in the extra configuration.
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        # In this example, the existing table contains only the 'full_name' and
        # 'age' columns, while the results of this query will contain an
        # additional 'favorite_color' column.
        'SELECT "Timmy" as full_name, 85 as age, "Blue" as favorite_color;',
        job_config=job_config,
    )  # Make an API request and wait for job to complete.

    # Checks the updated length of the schema.
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    print("Table {} now contains {} columns".format(table_id, len(table.schema)))

<br />

## Change a column's name

To rename a column on a table, use the
[`ALTER TABLE RENAME COLUMN` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_column_statement). The following example renames the column `old_name` to `new_name` on `mytable`:

```googlesql
ALTER TABLE mydataset.mytable
  RENAME COLUMN old_name TO new_name;
```

For more
information about `ALTER TABLE RENAME COLUMN` statements, see
[DDL details](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#details_15).

## Change a column's data type

Changing a column's data type isn't supported by the Google Cloud console, the
bq command-line tool, or the BigQuery API. If you attempt to update a table by
applying a schema
that specifies a new data type for a column, an error is returned.

### Change a column's data type with a DDL statement

You can use GoogleSQL to make certain changes to the data type of a
column. For more information and a complete list of supported data type
conversions, see the
[`ALTER COLUMN SET DATA TYPE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_data_type_statement).

The following example creates a table with a column of type `INT64`, then
updates the type to `NUMERIC`:

```googlesql
CREATE TABLE mydataset.mytable(c1 INT64);

ALTER TABLE mydataset.mytable
ALTER COLUMN c1 SET DATA TYPE NUMERIC;
```

The following example creates a table with a nested column with two fields, and
then updates the type of one of the columns from `INT` to `NUMERIC`:

```googlesql
CREATE TABLE mydataset.mytable(s1 STRUCT<a INT64, b STRING>);

ALTER TABLE mydataset.mytable ALTER COLUMN s1
SET DATA TYPE STRUCT<a NUMERIC, b STRING>;
```

### Modify nested column types

For complex nested schema changes, like altering a field within an array of
STRUCTs, the [`ALTER TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_data_type_statement) isn't supported.
As a workaround you can use the [`CREATE OR REPLACE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
with a `SELECT` statement
to transform your nested schema changes.

The following example demonstrates how to transform a column within an array of
`STRUCTS`:

Consider a table `samples.test` with the following schema and data:

```googlesql
CREATE OR REPLACE TABLE
  samples.test(D STRUCT <L ARRAY<STRUCT<R STRING, U STRING, V STRING>>, F STRING>);

INSERT INTO
  samples.test(D)
VALUES
  (STRUCT([STRUCT("r1", "u1", "v1"), STRUCT("r2", "u2", "v2")], "f1"));
```

The result looks similar to the following:

```
+---+
|                                     D                                      |
+---+
| {"L":[{"R":"r1","U":"u1","V":"v1"},{"R":"r2","U":"u2","V":"v2"}],"F":"f1"} |
+---+
```

Suppose you need to change the type of field `U` within the nested array of
`STRUCT`s to `STRUCT<W STRING>`. The following SQL statement demonstrates how to
accomplish this:

```googlesql
CREATE OR REPLACE TABLE
  samples.new_table AS
SELECT
  STRUCT(ARRAY(
    SELECT
      STRUCT(tmp.R,
        STRUCT(tmp.U AS W) AS U,
        tmp.V)
    FROM
      UNNEST(t.D.L) AS tmp) AS L,
    t.D.F) AS D
FROM
  samples.test AS t
```

This statement creates a new table, `samples.new_table`, with the target schema.
The `UNNEST` function expands the array of STRUCTs within `t.D.L`. The
expression `STRUCT(tmp.U AS W) AS U` constructs the new STRUCT with field W,
populated by the value from the original `U` field. The resulting table,
`samples.new_table`, has the following schema and data:

```
+---+
|                                           D                                            |
+---+
| {"L":[{"R":"r1","U":{"W":"u1"},"V":"v1"},{"R":"r2","U":{"W":"u2"},"V":"v2"}],"F":"f1"} |
+---+
```

### Cast a column's data type

To change a column's data type into a
[castable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules) type,
use a SQL query to select the table data,
[cast](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast)
the relevant column, and
[overwrite the table](https://docs.cloud.google.com/bigquery/docs/writing-results#permanent-table). Casting
and overwriting is not recommended for very large tables because it requires a
full table scan.

The following example shows a SQL query that selects all the data from
`column_two` and `column_three` in `mydataset.mytable` and casts `column_one`
from `DATE` to `STRING`. The query result is used to overwrite the existing
table. The overwritten table stores `column_one` as a `STRING` data type.

When using `CAST`, a query can fail if BigQuery is unable to
perform the cast. For details on casting rules in GoogleSQL, see
[Casting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast).

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Query editor** , enter the following query to select all of the
   data from `column_two` and `column_three` in `mydataset.mytable` and to
   cast `column_one` from `DATE` to `STRING`. The query uses an alias to
   cast `column_one` with the same name. `mydataset.mytable` is in
   your default project.

   ```googlesql
   SELECT
    column_two,
    column_three,
    CAST(column_one AS STRING) AS column_one
   FROM
    mydataset.mytable;
   ```
3. Click **More** and select **Query settings**.

4. In the **Destination** section, do the following:

   1. Select **Set a destination table for query results**.

   2. For **Project name** , leave the value set to your default project.
      This is the project that contains `mydataset.mytable`.

   3. For **Dataset** , choose `mydataset`.

   4. In the **Table Id** field, enter `mytable`.

   5. For **Destination table write preference** , select **Overwrite
      table** . This option overwrites `mytable` using the query results.

5. Optionally, choose
   your data's [location](https://docs.cloud.google.com/bigquery/docs/locations).

6. To update the settings, click **Save**.

7. Click **Run**.

   When the query job completes, the data type of `column_one` is `STRING`.

### bq

Enter the following `bq query` command to select all of the data from
`column_two` and `column_three` in `mydataset.mytable` and to cast
`column_one` from `DATE` to `STRING`. The query uses an alias to cast
`column_one` with the same name. `mydataset.mytable` is in your default
project.

The query results are written to `mydataset.mytable` using the
`--destination_table` flag, and the `--replace` flag is used to overwrite
`mytable`. Specify the `use_legacy_sql=false` flag to use
GoogleSQL syntax.

Optionally, supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

    bq query \
        --destination_table mydataset.mytable \
        --replace \
        --use_legacy_sql=false \
    'SELECT
      column_two,
      column_three,
      CAST(column_one AS STRING) AS column_one
    FROM
      mydataset.mytable'

### API

To select all of the data from `column_two` and `column_three` in
`mydataset.mytable` and to cast `column_one` from `DATE` to `STRING`, call
the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method and configure a `query` job. Optionally, specify your location in the
`location` property in the `jobReference` section.

The SQL query used in the query job would be `SELECT column_two,
column_three, CAST(column_one AS STRING) AS column_one FROM
mydataset.mytable`. The query uses an alias to cast `column_one` with the
same name.

To overwrite `mytable` with the query results, include `mydataset.mytable`
in the `configuration.query.destinationTable` property, and specify
`WRITE_TRUNCATE` in the `configuration.query.writeDisposition` property.

## Change a column's mode

The only supported modification you can make to a column's mode is
changing it from `REQUIRED` to `NULLABLE`. Changing a column's mode from
`REQUIRED` to `NULLABLE` is also called column relaxation. You can also relax a
column when you load data to overwrite an existing table,
or when you append data to an existing table. You can't change a column's mode
from `NULLABLE` to `REQUIRED` or from `REPEATED` to `NULLABLE`. To change a
column's mode from `NULLABLE` to `REQUIRED` or from `REPEATED` to `NULLABLE`,
you must recreate the table with the updated column modes.

### Make a column `NULLABLE` in an existing table

To change a column's mode from `REQUIRED` to `NULLABLE`, select one of
the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Overview \> Tables**, and then select the table.

5. In the details panel, click the **Schema** tab.

6. Click **Edit schema**. You might need to scroll to see this button.

7. In the **Current schema** page, locate the field that you want to change.

8. In the **Mode** drop-down list for that field, select `NULLABLE`.

9. To update the settings, click **Save**.

> [!NOTE]
> **Note:** You can't use the Google Cloud console to alter a column in an [external table](https://docs.cloud.google.com/bigquery/docs/external-tables).

### SQL

Use the
[`ALTER COLUMN DROP NOT NULL` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_drop_not_null_statement).
The following example changes the mode of the column `mycolumn` from
`REQUIRED` to `NULLABLE`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
   ALTER COLUMN mycolumn
   DROP NOT NULL;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

1. First, issue the `bq show` command with the `--schema` flag and write the
   existing table schema to a file. If the table you're updating is in a
   project other than your default project, add the project ID to the dataset
   name in the following format: `PROJECT_ID:DATASET`.

   ```bash
   bq show \
   --schema \
   --format=prettyjson \
   PROJECT_ID:DATASET.TABLE > SCHEMA_FILE
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET`: the name of the dataset that contains the table you're updating.
   - `TABLE`: the name of the table you're updating.
   - `SCHEMA_FILE`: the schema definition file written to your local machine.

   For example, to write the schema definition of `mydataset.mytable` to a
   file, enter the following command. `mydataset.mytable` is in your
   default project.

         bq show \
         --schema \
         --format=prettyjson \
         mydataset.mytable > /tmp/myschema.json

2. Open the schema file in a text editor. The schema should look like the
   following:

   ```
   [
     {
       "mode": "REQUIRED",
       "name": "column1",
       "type": "STRING"
     },
     {
       "mode": "REQUIRED",
       "name": "column2",
       "type": "FLOAT"
     },
     {
       "mode": "REPEATED",
       "name": "column3",
       "type": "STRING"
     }
   ]
   ```
3. Change an existing column's mode from `REQUIRED` to `NULLABLE`. In this
   example, the mode for `column1` is relaxed.

   ```
   [
     {
       "mode": "NULLABLE",
       "name": "column1",
       "type": "STRING"
     },
     {
       "mode": "REQUIRED",
       "name": "column2",
       "type": "FLOAT"
     },
     {
       "mode": "REPEATED",
       "name": "column3",
       "type": "STRING"
     }
   ]
   ```

   For more information on working with JSON schema files, see
   [Specifying a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).
4. After updating your schema file, issue the following command to update
   the table's schema. If the table you're updating is in a project other than
   your default project, add the project ID to the dataset name in the
   following format: `PROJECT_ID:DATASET`.

   ```bash
   bq update PROJECT_ID:DATASET.TABLE SCHEMA
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET`: the name of the dataset that contains the table you're updating.
   - `TABLE`: the name of the table you're updating.
   - `SCHEMA`: the path to the JSON schema file on your local machine.

   For example, enter the following command to update the schema definition
   of `mydataset.mytable` in your default project. The path to the schema
   file on your local machine is `/tmp/myschema.json`.

         bq update mydataset.mytable /tmp/myschema.json

### API

Call [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) and
use the `schema` property to change a `REQUIRED` column to `NULLABLE` in
your schema definition. Because the `tables.update` method replaces the
entire table resource, the `tables.patch` method is preferred.

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

    // relaxTableAPI demonstrates modifying the schema of a table to remove the requirement that columns allow
    // no NULL values.
    func relaxTableAPI(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydatasetid"
    	// tableID := "mytableid"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	// Setup: We first create a table with a schema that's restricts NULL values.
    	sampleSchema := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "full_name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    		{Name: "age", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    	}
    	original := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		Schema: sampleSchema,
    	}
    	if err := client.Dataset(datasetID).Table(tableID).Create(ctx, original); err != nil {
    		return err
    	}

    	tableRef := client.Dataset(datasetID).Table(tableID)
    	meta, err := tableRef.Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	// Iterate through the schema to set all Required fields to false (nullable).
    	var relaxed bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema
    	for _, v := range meta.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema {
    		v.Required = false
    		relaxed = append(relaxed, v)
    	}
    	newMeta := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadataToUpdate{
    		Schema: relaxed,
    	}
    	if _, err := tableRef.Update(ctx, newMeta, meta.ETag); err != nil {
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;

    public class RelaxColumnMode {

      public static void runRelaxColumnMode() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableId = "MY_TABLE_NAME";
        relaxColumnMode(datasetName, tableId);
      }

      public static void relaxColumnMode(String datasetName, String tableId) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(datasetName, tableId);

          // Create new relaxed schema based on the existing table schema
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html relaxedSchema =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                  // The only supported modification you can make to a column's mode is changing it from
                  // REQUIRED to NULLABLE
                  // Changing a column's mode from REQUIRED to NULLABLE is also called column relaxation
                  // INFO: LegacySQLTypeName will be updated to StandardSQLTypeName in release 1.103.0
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("word", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html.STRING)
                      .setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.NULLABLE)
                      .build(),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("word_count", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html.STRING)
                      .setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.NULLABLE)
                      .build(),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("corpus", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html.STRING)
                      .setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.NULLABLE)
                      .build(),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("corpus_date", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LegacySQLTypeName.html.STRING)
                      .setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.NULLABLE)
                      .build());

          // Update the table with the new schema
          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html updatedTable =
              table.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html#com_google_cloud_bigquery_biglake_v1_Table_toBuilder__().setDefinition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html.of(relaxedSchema)).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Table.html#com_google_cloud_bigquery_Table_update_com_google_cloud_bigquery_BigQuery_TableOption____dTable.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Table.html#com_google_cloud_bigquery_Table_update_com_google_cloud_bigquery_BigQuery_TableOption____();
          System.out.println("Table schema successfully relaxed.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table schema not relaxed \n" + e.toString());
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

    // Import the Google Cloud client library and create a client
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function relaxColumn() {
      /**
       * Changes columns from required to nullable.
       * Assumes existing table with the following schema:
       * [{name: 'Name', type: 'STRING', mode: 'REQUIRED'},
       * {name: 'Age', type: 'INTEGER'},
       * {name: 'Weight', type: 'FLOAT'},
       * {name: 'IsMagic', type: 'BOOLEAN'}];
       */

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      const newSchema = [
        {name: 'Name', type: 'STRING', mode: 'https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/bigquery-storage/protos.google.cloud.bigquery.storage.v1.tablefieldschema.mode.html'},
        {name: 'Age', type: 'https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest/bigquery-data-transfer/protos.google.cloud.bigquery.datatransfer.v1.datasourceparameter.type.html'},
        {name: 'Weight', type: 'FLOAT'},
        {name: 'IsMagic', type: 'https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest/bigquery-data-transfer/protos.google.cloud.bigquery.datatransfer.v1.datasourceparameter.type.html'},
      ];

      // Retrieve current table metadata
      const table = bigquery.dataset(datasetId).table(tableId);
      const [metadata] = await table.getMetadata();

      // Update schema
      metadata.schema = newSchema;
      const [apiResponse] = await table.setMetadata(metadata);

      console.log(apiResponse.schema.fields);
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

Overwrite the [Table.schema](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_schema) property with a list of [SchemaField](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField) objects with the [mode](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField#google_cloud_bigquery_schema_SchemaField_mode) property set to `'NULLABLE'`

<br />

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(dev): Change table_id to full name of the table you want to create.
    table_id = "your-project.your_dataset.your_table"

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    new_schema = []
    for field in table.schema:
        if field.mode != "REQUIRED":
            new_schema.append(field)
        else:
            # SchemaField properties cannot be edited after initialization.
            # To make changes, construct new SchemaField objects.
            new_field = field.to_api_repr()
            new_field["mode"] = "NULLABLE"
            relaxed_field = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html.from_api_repr(new_field)
            new_schema.append(relaxed_field)

    table.schema = new_schema
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_table(table, ["schema"])

    print(f"Updated {table_id} schema: {table.schema}.")


<br />

### Make a column `NULLABLE` with an appending load job

You can relax a column's mode when you append data to a table in a load job.
Select one of the following based on the type of file:

- When appending data from CSV and JSON files, relax the mode for individual columns by specifying a JSON schema file.
- When appending data from Avro, ORC, or Parquet files, relax columns to `NULL` in your schema and let schema inference detect the relaxed columns.

> [!NOTE]
> **Note:** Column relaxation does not apply to Datastore export appends. The columns in tables created by loading Datastore export files are always `NULLABLE`.

To relax a column from `REQUIRED` to `NULLABLE` when you append data to a table
during a load job, select one of the following options:

### Console

You cannot relax a column's mode using the Google Cloud console.

### bq

Use the `bq load` command to load your data and specify the `--noreplace`
flag to indicate that you are appending the data to an existing table.

If the data you're appending is in CSV or newline-delimited JSON format,
specify the relaxed columns in a local JSON schema file or use the
`--autodetect` flag to use [schema detection](https://docs.cloud.google.com/bigquery/docs/schema-detect)
to discover relaxed columns in the source data.

Relaxed columns can be automatically inferred from Avro, ORC, and Parquet
files. Column relaxation does not apply to Datastore export
appends. The columns in tables created by loading Datastore export
files are always `NULLABLE`.

Set the `--schema_update_option` flag to `ALLOW_FIELD_RELAXATION` to
indicate that the data you're appending contains relaxed columns.

If the table you're appending is in a dataset in a project other than your
default project, add the project ID to the dataset name in the following
format: `PROJECT_ID:DATASET`.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

Enter the `load` command as follows:

```bash
bq --location=LOCATION load \
--noreplace \
--schema_update_option=ALLOW_FIELD_RELAXATION \
--source_format=FORMAT \
PROJECT_ID:DATASET.TABLE \
PATH_TO_SOURCE \
SCHEMA
```

Replace the following:

- `LOCATION`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: `NEWLINE_DELIMITED_JSON`, `CSV`, `PARQUET`, `ORC`, or `AVRO`. `DATASTORE_BACKUP` files don't require column relaxation. The columns in tables created from Datastore export files are always `NULLABLE`.
- `PROJECT_ID`: your project ID.
- <var translate="no">dataset</var> is the name of the dataset that contains the table.
- `TABLE`: the name of the table you're appending.
- `PATH_TO_SOURCE`: a fully-qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri), a comma-separated list of URIs, or the path to a data file on your local machine.
- `SCHEMA`: the path to a local JSON schema file. This option is used only for CSV and JSON files. Relaxed columns are automatically inferred from Avro files.

Examples:

Enter the following command to append a local Avro data file,
`/tmp/mydata.avro`, to `mydataset.mytable` using a load job. Since relaxed
columns can be automatically inferred from Avro data you don't need to
specify a schema file. `mydataset` is in your default project.

    bq load \
        --noreplace \
        --schema_update_option=ALLOW_FIELD_RELAXATION \
        --source_format=AVRO \
        mydataset.mytable \
        /tmp/mydata.avro

Enter the following command to append data from a newline-delimited JSON
file in Cloud Storage to `mydataset.mytable` using a load job. The
schema containing the relaxed columns is in a local JSON schema file ---
`/tmp/myschema.json`. `mydataset` is in your default project.

    bq load \
    --noreplace \
    --schema_update_option=ALLOW_FIELD_RELAXATION \
    --source_format=NEWLINE_DELIMITED_JSON \
    mydataset.mytable \
    gs://mybucket/mydata.json \
    /tmp/myschema.json

Enter the following command to append data in a CSV file on your local
machine to `mydataset.mytable` using a load job. The command uses schema
auto-detection to discover relaxed columns in the source data. `mydataset`
is in `myotherproject`, not your default project.

    bq load \
    --noreplace \
    --schema_update_option=ALLOW_FIELD_RELAXATION \
    --source_format=CSV \
    --autodetect \
    myotherproject:mydataset.mytable \
    mydata.csv

### API

Call the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method. Configure a `load` job and set the following properties:

- Reference your data in Cloud Storage using the `sourceUris` property.
- Specify the data format by setting the `sourceFormat` property.
- Specify the schema in the `schema` property.
- Specify the schema update option using the `schemaUpdateOptions` property.
- Set the write disposition of the destination table to `WRITE_APPEND` using the `writeDisposition` property.

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
    	"os"

    	"cloud.google.com/go/bigquery"
    )

    // relaxTableImport demonstrates amending the schema of a table to relax columns from
    // not allowing NULL values to allowing them.
    func relaxTableImport(projectID, datasetID, tableID, filename string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	sampleSchema := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "full_name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    		{Name: "age", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    	}
    	meta := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		Schema: sampleSchema,
    	}
    	tableRef := client.Dataset(datasetID).Table(tableID)
    	if err := tableRef.Create(ctx, meta); err != nil {
    		return err
    	}
    	// Now, import data from a local file, but specify relaxation of required
    	// fields as a side effect while the data is appended.
    	f, err := os.Open(filename)
    	if err != nil {
    		return err
    	}
    	source := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_ReaderSource_NewReaderSource(f)
    	source.AutoDetect = true   // Allow BigQuery to determine schema.
    	source.SkipLeadingRows = 1 // CSV has a single header line.

    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(source)
    	loader.SchemaUpdateOptions = []string{"ALLOW_FIELD_RELAXATION"}
    	job, err := loader.Run(ctx)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.common.collect.ImmutableList;

    // Sample to append relax column in a table.
    public class RelaxColumnLoadAppend {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.csv";
        relaxColumnLoadAppend(datasetName, tableName, sourceUri);
      }

      public static void relaxColumnLoadAppend(String datasetName, String tableName, String sourceUri) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Retrieve destination table reference
          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(TableId.of(datasetName, tableName));

          // column as a 'REQUIRED' field.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html name =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING).setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.REQUIRED).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html postAbbr =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.newBuilder("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING)
                  .setMode(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.Mode.REQUIRED)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(name, postAbbr);

          // Skip header row in the file.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html csvOptions = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html.newBuilder().setSkipLeadingRows(1).build();

          // Set job options
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(table.getTableId(), sourceUri)
                  .setSchema(schema)
                  .setFormatOptions(csvOptions)
                  .setSchemaUpdateOptions(
                      ImmutableList.of(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.SchemaUpdateOption.ALLOW_FIELD_RELAXATION))
                  .setWriteDisposition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.WriteDisposition.WRITE_APPEND)
                  .build();

          // Create a load job and wait for it to complete.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          // Check the job's status for errors
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__() && job.getStatus().getError() == null) {
            System.out.println("Relax column append successfully loaded in a table");
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

    // Import the Google Cloud client libraries
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');

    // Instantiate client
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function relaxColumnLoadAppend() {
      // Changes required column to nullable in load append job.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const fileName = '/path/to/file.csv';
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      // In this example, the existing table contains the 'Name'
      // column as a 'REQUIRED' field.
      const schema = 'Age:INTEGER, Weight:FLOAT, IsMagic:BOOLEAN';

      // Retrieve destination table reference
      const [table] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .get();
      const destinationTableRef = table.metadata.tableReference;

      // Set load job options
      const options = {
        schema: schema,
        schemaUpdateOptions: ['ALLOW_FIELD_RELAXATION'],
        writeDisposition: 'WRITE_APPEND',
        destinationTable: destinationTableRef,
      };

      // Load data from a local file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(fileName, options);

      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
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

    # from google.cloud import bigquery
    # client = bigquery.Client()
    # project = client.project
    # dataset_ref = bigquery.DatasetReference(project, 'my_dataset')
    # filepath = 'path/to/your_file.csv'

    # Retrieves the destination table and checks the number of required fields
    table_id = "my_table"
    table_ref = dataset_ref.table(table_id)
    table = client.get_table(table_ref)
    original_required_fields = sum(field.mode == "REQUIRED" for field in table.schema)
    # In this example, the existing table has 3 required fields.
    print("{} fields in the schema are required.".format(original_required_fields))

    # Configures the load job to append the data to a destination table,
    # allowing field relaxation
    job_config = bigquery.LoadJobConfig()
    job_config.write_disposition = bigquery.WriteDisposition.WRITE_APPEND
    job_config.schema_update_options = [
        bigquery.SchemaUpdateOption.ALLOW_FIELD_RELAXATION
    ]
    # In this example, the existing table contains three required fields
    # ('full_name', 'age', and 'favorite_color'), while the data to load
    # contains only the first two fields.
    job_config.schema = [
        bigquery.SchemaField("full_name", "STRING", mode="REQUIRED"),
        bigquery.SchemaField("age", "INTEGER", mode="REQUIRED"),
    ]
    job_config.source_format = bigquery.SourceFormat.CSV
    job_config.skip_leading_rows = 1

    with open(filepath, "rb") as source_file:
        job = client.load_table_from_file(
            source_file,
            table_ref,
            location="US",  # Must match the destination dataset location.
            job_config=job_config,
        )  # API request

    job.result()  # Waits for table load to complete.
    print(
        "Loaded {} rows into {}:{}.".format(
            job.output_rows, dataset_id, table_ref.table_id
        )
    )

    # Checks the updated number of required fields
    table = client.get_table(table)
    current_required_fields = sum(field.mode == "REQUIRED" for field in table.schema)
    print("{} fields in the schema are now required.".format(current_required_fields))

<br />

### Make all columns `NULLABLE` with an append job

You can relax all columns in a table when you append query results to it. You
can relax all required fields in the destination table by setting the
`--schema_update_option` flag to `ALLOW_FIELD_RELAXATION`. You cannot relax
individual columns in a destination table by using a query append. To relax
individual columns with a load append job, see
[Make a column `NULLABLE` with an append job](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas#column_nullable_append).

To relax all columns when you append query results to a destination table,
select one of the following options:

### Console

You cannot relax a column's mode using the Google Cloud console.

### bq

Use the `bq query` command to query your data and specify the
`--destination_table` flag to indicate which table you're appending.

To specify that you are appending query results to an existing destination
table, specify the `--append_table` flag.

Set the `--schema_update_option` flag to `ALLOW_FIELD_RELAXATION` to
indicate that all `REQUIRED` columns in the table you're appending should be
changed to `NULLABLE`.

Specify the `use_legacy_sql=false` flag to use GoogleSQL syntax for the
query.

If the table you're appending is in a dataset in a project other than your
default project, add the project ID to the dataset name in the following
format: `PROJECT_ID:DATASET`.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

```bash
bq --location=LOCATION query \
--destination_table PROJECT_ID:DATASET.TABLE \
--append_table \
--schema_update_option=ALLOW_FIELD_RELAXATION \
--use_legacy_sql=false \
'QUERY'
```

Replace the following:

- `LOCATION`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `PROJECT_ID`: your project ID.
- `DATASET`: the name of the dataset that contains the table you're appending.
- `TABLE`: the name of the table you're appending.
- `QUERY`: a query in GoogleSQL syntax.

Examples:

Enter the following command query `mydataset.mytable` in your default
project to append the query results to `mydataset.mytable2` (also in
your default project). The command changes all `REQUIRED` columns in the
destination table to `NULLABLE`.

    bq query \
        --destination_table mydataset.mytable2 \
        --append_table \
        --schema_update_option=ALLOW_FIELD_RELAXATION \
        --use_legacy_sql=false \
        'SELECT
           column1,column2
         FROM
           mydataset.mytable'

Enter the following command query `mydataset.mytable` in your default
project and to append the query results to `mydataset.mytable2` in
`myotherproject`. The command changes all `REQUIRED` columns in the
destination table to `NULLABLE`.

    bq query \
    --destination_table myotherproject:mydataset.mytable2 \
    --append_table \
    --schema_update_option=ALLOW_FIELD_RELAXATION \
    --use_legacy_sql=false \
    'SELECT
       column1,column2
     FROM
       mydataset.mytable'

### API

Call the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method. Configure a `query` job and set the following properties:

- Specify the destination table using the `destinationTable` property.
- Set the write disposition of the destination table to `WRITE_APPEND` using the `writeDisposition` property.
- Specify the schema update option using the `schemaUpdateOptions` property.
- Specify the GoogleSQL query using the`query` property.

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

    // relaxTableQuery demonstrates relaxing the schema of a table by appending query results to
    // enable the table to allow NULL values.
    func relaxTableQuery(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	sampleSchema := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "full_name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    		{Name: "age", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType, Required: true},
    	}
    	meta := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		Schema: sampleSchema,
    	}
    	tableRef := client.Dataset(datasetID).Table(tableID)
    	if err := tableRef.Create(ctx, meta); err != nil {
    		return err
    	}
    	// Now, append a query result that includes nulls, but allow the job to relax
    	// all required columns.
    	q := client.Query("SELECT \"Beyonce\" as full_name")
    	q.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryConfig.Dst = client.Dataset(datasetID).Table(tableID)
    	q.SchemaUpdateOptions = []string{"ALLOW_FIELD_RELAXATION"}
    	q.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty
    	q.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_Location = "US"
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	_, err = job.Wait(ctx)
    	if err != nil {
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.SchemaUpdateOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.WriteDisposition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import com.google.common.collect.ImmutableList;

    public class RelaxTableQuery {

      public static void runRelaxTableQuery() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        relaxTableQuery(projectId, datasetName, tableName);
      }

      // To relax all columns in a destination table when you append data to it during a query job
      public static void relaxTableQuery(String projectId, String datasetName, String tableName)
          throws Exception {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          TableId tableId = TableId.of(datasetName, tableName);

          String sourceTable = "`" + projectId + "." + datasetName + "." + tableName + "`";
          String query = "SELECT word FROM " + sourceTable + " WHERE word like '%is%'";

          QueryJobConfiguration queryConfig =
              QueryJobConfiguration.newBuilder(query)
                  // Use standard SQL syntax for queries.
                  // See: https://cloud.google.com/bigquery/sql-reference/
                  .setUseLegacySql(false)
                  .setSchemaUpdateOptions(ImmutableList.of(SchemaUpdateOption.ALLOW_FIELD_RELAXATION))
                  .setWriteDisposition(WriteDisposition.WRITE_APPEND)
                  .setDestinationTable(tableId)
                  .build();

          Job queryJob = bigquery.create(JobInfo.newBuilder(queryConfig).build());

          queryJob = queryJob.waitFor();

          // Check for errors
          if (queryJob == null) {
            throw new Exception("Job no longer exists");
          } else if (queryJob.getStatus().getError() != null) {
            // You can also look at queryJob.getStatus().getExecutionErrors() for all
            // errors, not just the latest one.
            throw new Exception(queryJob.getStatus().getError().toString());
          }

          // Get the results.
          TableResult results = queryJob.getQueryResults();

          // Print all pages of the results.
          results
              .iterateAll()
              .forEach(
                  rows -> {
                    rows.forEach(row -> System.out.println("row: " + row.toString()));
                  });

          System.out.println("Successfully relaxed all columns in destination table during query job");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Columns not relaxed during query job \n" + e.toString());
        }
      }
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

    # TODO(developer): Set table_id to the ID of the destination table.
    # table_id = "your-project.your_dataset.your_table_name"

    # Retrieves the destination table and checks the number of required fields.
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    original_required_fields = sum(field.mode == "REQUIRED" for field in table.schema)

    # In this example, the existing table has 2 required fields.
    print("{} fields in the schema are required.".format(original_required_fields))

    # Configures the query to append the results to a destination table,
    # allowing field relaxation.
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        destination=table_id,
        schema_update_options=[https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SchemaUpdateOption.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SchemaUpdateOption.html#google_cloud_bigquery_enums_SchemaUpdateOption_ALLOW_FIELD_RELAXATION],
        write_disposition=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html#google_cloud_bigquery_enums_WriteDisposition_WRITE_APPEND,
    )

    # Start the query, passing in the extra configuration.
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        # In this example, the existing table contains 'full_name' and 'age' as
        # required columns, but the query results will omit the second column.
        'SELECT "Beyonce" as full_name;',
        job_config=job_config,
    )  # Make an API request and wait for job to complete

    # Checks the updated number of required fields.
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    current_required_fields = sum(field.mode == "REQUIRED" for field in table.schema)
    print("{} fields in the schema are now required.".format(current_required_fields))

<br />

## Change a column's default value

To change the default value for a column, select one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand the project, click **Datasets**, and
   then select the dataset.

4. Click **Overview \> Tables**, and then click a table.

5. Click the **Schema** tab.

6. Click **Edit schema**. You might need to scroll to see this button.

7. In the **Current schema** page, locate the top-level field that you want
   to change.

8. Enter the default value for that field.

9. Click **Save**.

### SQL

Use the
[`ALTER COLUMN SET DEFAULT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_default_statement).

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
   ALTER COLUMN column_name SET DEFAULT default_expression;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## Change a column description

To change the description for a column, select one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Overview \> Tables**, and then select the table.

5. In the details panel, click the **Schema** tab.

6. Click **Edit schema**. You might need to scroll to see this button.

7. In the **Current schema** page, locate the field that you want
   to change.

8. Enter the description for that field.

9. Click **Save**.

### SQL

Use the
[`ALTER COLUMN SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_options_statement).

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
   ALTER COLUMN column_name
   SET OPTIONS (description = 'This is a column description.');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### Gemini

You can generate column descriptions with Gemini in
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

   Gemini generates column descriptions and insights about
   the table. It takes a few minutes for the information to be
   populated. You can view the generated insights on the table's
   **Insights** tab.
6. To edit and save the generated column descriptions, do the following:

   1. Click **View column descriptions**.

   2. In the **Column descriptions** section, click **Save to schema**.

      The generated column descriptions are populated in the
      **New description** field for each column.
   3. Edit the column descriptions as necessary, and then click **Save**.

      The column descriptions are updated immediately.
   4. To close the **Preview descriptions** panel, click
      **Close**.

## Delete a column

You can delete a column from an existing table by using the
[`ALTER TABLE DROP COLUMN` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_drop_column_statement).

The statement does not immediately free up the storage that is associated with
the dropped column. To learn more about the impact on storage when you drop a
column on storage, see [`ALTER TABLE DROP COLUMN` statement
details](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#details_16).
There are two options for immediately reclaiming storage:

- [Overwrite a table](https://docs.cloud.google.com/bigquery/docs/tables#create_a_table_from_a_query_result)
  with a [`SELECT * EXCEPT` query](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_except):

      CREATE OR REPLACE TABLE mydataset.mytable AS (
        SELECT * EXCEPT (column_to_delete) FROM mydataset.mytable
      );

- Export the data to Cloud Storage, delete the unwanted columns, and then
  load the data into a new table with the correct schema.

Schema updates can't drop nested fields.