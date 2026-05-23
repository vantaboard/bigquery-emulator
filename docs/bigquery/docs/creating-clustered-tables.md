# Create clustered tables

You can reduce the amount of data processed by a query by using clustered tables
in BigQuery.

With clustered tables, table data is organized based on the values of specified
columns, also called the *clustering columns*. BigQuery sorts the
data by the clustered columns, then stores the rows that have similar values in
the same or nearby physical blocks. When a query filters on a clustered column,
BigQuery efficiently scans only the relevant blocks and skips the
data that doesn't match the filter.

For more information, see the following:

- To learn more about clustered tables in BigQuery, see [Introduction to clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables).
- To learn about working with and controlling access to clustered tables, see [Manage clustered tables](https://docs.cloud.google.com/bigquery/docs/manage-clustered-tables).

## Before you begin

### Required roles


To get the permissions that
you need to create a table,

ask your administrator to grant you the
following IAM roles:

- [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`) on the project if you're creating a table by loading data or by saving query results to a table.
- [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the dataset where you're creating the table.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to create a table. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create a table:

- `bigquery.tables.create` on the dataset where you're creating the table.
- `bigquery.tables.getData` on all tables and views that your query references if you're saving query results as a table.
- `bigquery.jobs.create` on the project if you're creating the table by loading data or by saving query results to a table.
- `bigquery.tables.updateData` on the table if you're appending to or overwriting a table with query results.


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Table naming requirements

When you create a table in BigQuery, the table name must
be unique per dataset. The table name can:

- Contain characters with a total of up to 1,024 UTF-8 bytes.
- Contain Unicode characters in category L (letter), M (mark), N (number), Pc (connector, including underscore), Pd (dash), Zs (space). For more information, see [General Category](https://wikipedia.org/wiki/Unicode_character_property#General_Category).

The following are all examples of valid table names:
`table 01`, `ग्राहक`, `00_お客様`, `étudiant-01`.

Caveats:

- Table names are case-sensitive by default. `mytable` and `MyTable` can coexist in the same dataset, unless they are part of a [dataset with
  case-sensitivity turned off](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#creating_a_case-insensitive_dataset).
- Some table names and table name prefixes are reserved. If you receive an error saying that your table name or prefix is reserved, then select a different name and try again.
- If you include multiple dot operators (`.`) in a sequence, the duplicate
  operators are implicitly stripped.

  For example, this:
  `project_name....dataset_name..table_name`

  Becomes this:
  `project_name.dataset_name.table_name`

### Clustered column requirements

You can specify the columns used to create the clustered table when you create a
table in BigQuery. After the table is created, you can modify the
columns used to create the clustered table. For details, see
[Modifying the clustering specification](https://docs.cloud.google.com/bigquery/docs/manage-clustered-tables#modifying-cluster-spec).

Clustering columns must be top-level, non-repeated columns, and they must be one
of the following data types:

- `BIGNUMERIC`
- `BOOL`
- `DATE`
- `DATETIME`
- `GEOGRAPHY`
- `INT64`
- `NUMERIC`
- `RANGE`
- `STRING`
- `TIMESTAMP`

You can specify up to four clustering columns. When you specify multiple
columns, the order of the columns determines how the data is sorted. For
example, if the table is clustered by columns a, b and c, the data is sorted in
the same order: first by column a, then by column b, and then by column
c. As a best practice, place the most frequently filtered or aggregated column
first.

The order of your clustering columns also affects query performance and pricing.
For more information about query best practices for clustered tables, see
[Querying clustered tables](https://docs.cloud.google.com/bigquery/docs/querying-clustered-tables).

## Create an empty clustered table with a schema definition

To create an empty clustered table with a schema definition:

### Console

1.
   In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then select a dataset.
4. In the **Dataset info** section, click **Create table**.
5. In the **Create table** pane, specify the following details:
   1. In the **Source** section, select **Empty table** in the **Create table from** list.
   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, enter the [schema](https://docs.cloud.google.com/bigquery/docs/schemas) definition. You can enter schema information manually by using one of the following methods:
      - Option 1: Click **Edit as text** and paste the schema in the form of a JSON array. When you use a JSON array, you generate the schema using the same process as [creating a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file). You can view the schema of an existing table in JSON format by entering the following command:

        ```bash
            bq show --format=prettyjson dataset.table
            
        ```
      - Option 2: Click **Add field** and enter the table schema. Specify each field's **Name** , [**Type**](https://docs.cloud.google.com/bigquery/docs/schemas#standard_sql_data_types), and [**Mode**](https://docs.cloud.google.com/bigquery/docs/schemas#modes).
   4. For **Clustering order**, enter between one and four comma-separated column names.
   5. Optional: In the **Advanced options** section, if you want to use a customer-managed encryption key, then select the **Use a customer-managed
      encryption key (CMEK)** option. By default, BigQuery [encrypts customer content stored at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption) by using a Google-owned and Google-managed encryption key.
   6. Click **Create table**.

### SQL

Use the [`CREATE TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
command with the `CLUSTER BY` option. The following example creates a
clustered table named `myclusteredtable` in `mydataset`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE TABLE mydataset.myclusteredtable
   (
     customer_id STRING,
     transaction_amount NUMERIC
   )
   CLUSTER BY
     customer_id
     OPTIONS (
       description = 'a table clustered by customer_id');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk) command
with the following flags:

- `--table` (or the `-t` shortcut).
- `--schema`. You can supply the table's schema definition inline or use a JSON schema file.
- `--clustering_fields`. You can specify up to four clustering columns.

Optional parameters include `--expiration`, `--description`,
`--time_partitioning_type`, `--time_partitioning_field`,
`--time_partitioning_expiration`, `--destination_kms_key`, and `--label`.

If you are creating a table in a project other than your default project,
add the project ID to the dataset in the following format:
`project_id:dataset`.

`--destination_kms_key` is not demonstrated here. For information about
using `--destination_kms_key`, see
[customer-managed encryption keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).

Enter the following command to create an empty clustered table with a
schema definition:

```bash
bq mk \
    --table \
    --expiration INTEGER1 \
    --schema SCHEMA \
    --clustering_fields CLUSTER_COLUMNS \
    --description "DESCRIPTION" \
    --label KEY:VALUE,KEY:VALUE \
    PROJECT_ID:DATASET.TABLE
```

Replace the following:

- `INTEGER1`: the default lifetime, in seconds, for the table. The minimum value is 3,600 seconds (one hour). The expiration time evaluates to the current UTC time plus the integer value. If you set the table's expiration time when you create a table, the dataset's default table expiration setting is ignored. Setting this value deletes the table after the specified time.
- `SCHEMA`: an inline schema definition in the format `COLUMN:DATA_TYPE,COLUMN:DATA_TYPE` or the path to the JSON schema file on your local machine.
- `CLUSTER_COLUMNS`: a comma-separated list of up to four clustering columns. The list cannot contain any spaces.
- `DESCRIPTION`: a description of the table, in quotes.
- `KEY:VALUE`: the key-value pair that represents a [label](https://docs.cloud.google.com/bigquery/docs/labels). You can enter multiple labels using a comma-separated list.
- `PROJECT_ID`: your project ID.
- `DATASET`: a dataset in your project.
- `TABLE`: the name of the table you're creating.

When you specify the schema on the command line, you cannot include a
`RECORD` ([`STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type))
type, you cannot include a column description, and you
cannot specify the column's mode. All modes default to `NULLABLE`. To
include descriptions, modes, and `RECORD` types, [supply a JSON schema
file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file) instead.

Examples:

Enter the following command to create a clustered table
named `myclusteredtable` in `mydataset` in your default project. The table's
expiration is set to 2,592,000 (1 30-day month), the description is set to
`This is my clustered table`, and the label is set to
`organization:development`. The command uses the `-t` shortcut instead of
`--table`.

The schema is specified inline as:
`timestamp:timestamp,customer_id:string,transaction_amount:float`. The
specified clustering field `customer_id` is used to cluster the table.

    bq mk \
        -t \
        --expiration 2592000 \
        --schema 'timestamp:timestamp,customer_id:string,transaction_amount:float' \
        --clustering_fields customer_id \
        --description "This is my clustered table" \
        --label org:dev \
        mydataset.myclusteredtable

Enter the following command to create a clustered table named
`myclusteredtable` in `myotherproject`, not your default project. The
description is set to `This is my clustered table`, and the label is set
to `organization:development`. The command uses the `-t` shortcut instead of
`--table`. This command does not specify a table expiration. If the dataset
has a default table expiration, it is applied. If the dataset has no default
table expiration, the table never expires.

The schema is specified in a local JSON file: `/tmp/myschema.json`. The
`customer_id` field is used to cluster the table.

    bq mk \
        -t \
        --expiration 2592000 \
        --schema /tmp/myschema.json \
        --clustering_fields=customer_id \
        --description "This is my clustered table" \
        --label org:dev \
        myotherproject:mydataset.myclusteredtable

After the table is created, you can update the table's
[description](https://docs.cloud.google.com/bigquery/docs/samples/bigquery-update-table-description)
and [labels](https://docs.cloud.google.com/bigquery/docs/labels#creating_and_updating_table_and_view_labels).

### Terraform

Use the
[`google_bigquery_table`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the Cloud Resource Manager API.

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a table named `mytable` that is clustered
on the `ID` and `Created` columns:

    resource "google_bigquery_dataset" "default" {
      dataset_id                      = "mydataset"
      default_partition_expiration_ms = 2592000000  # 30 days
      default_table_expiration_ms     = 31536000000 # 365 days
      description                     = "dataset description"
      location                        = "US"
      max_time_travel_hours           = 96 # 4 days

      labels = {
        billing_group = "accounting",
        pii           = "sensitive"
      }
    }

    resource "google_bigquery_table" "default" {
      dataset_id = google_bigquery_dataset.default.dataset_id
      table_id   = "mytable"

      clustering = ["ID", "Created"]

      schema = <<EOF
    [
      {
        "name": "ID",
        "type": "INT64",
        "description": "Item ID"
      },
      {
        "name": "Item",
        "type": "STRING",
        "mode": "NULLABLE"
      },
     {
       "name": "Created",
       "type": "TIMESTAMP"
     }
    ]
    EOF

    }

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

### API

Call the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert)
method with a defined [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables)
that specifies the `clustering.fields` property and the `schema` property.

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

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    schema = [
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("full_name", "STRING"),
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("city", "STRING"),
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("zipcode", "INTEGER"),
    ]

    table = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html(table_id, schema=schema)
    table.clustering_fields = ["city", "zipcode"]
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_create_table(table)  # Make an API request.
    print(
        "Created clustered table {}.{}.{}".format(
            table.project, table.dataset_id, table.table_id
        )
    )

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

    // createTableClustered demonstrates creating a BigQuery table with advanced properties like
    // partitioning and clustering features.
    func createTableClustered(projectID, datasetID, tableID string) error {
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
    		{Name: "timestamp", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "origin", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "destination", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "amount", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	}
    	metaData := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		Schema: sampleSchema,
    		TimePartitioning: &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TimePartitioning{
    			Field:      "timestamp",
    			Expiration: 90 * 24 * time.Hour,
    		},
    		Clustering: &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Clustering{
    			Fields: []string{"origin", "destination"},
    		},
    	}
    	tableRef := client.Dataset(datasetID).Table(tableID)
    	if err := tableRef.Create(ctx, metaData); err != nil {
    		return err
    	}
    	return nil
    }

<br />

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Clustering.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html;
    import com.google.common.collect.ImmutableList;

    public class CreateClusteredTable {
      public static void runCreateClusteredTable() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        createClusteredTable(datasetName, tableName);
      }

      public static void createClusteredTable(String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html partitioning = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html.of(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html.Type.DAY);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("date", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.DATE));

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Clustering.html clustering =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Clustering.html.newBuilder().setFields(ImmutableList.of("name", "post_abbr")).build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html tableDefinition =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html.newBuilder()
                  .setSchema(schema)
                  .setTimePartitioning(partitioning)
                  .setClustering(clustering)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html tableInfo = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html.newBuilder(tableId, tableDefinition).build();

          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(tableInfo);
          System.out.println("Clustered table created successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Clustered table was not created. \n" + e.toString());
        }
      }
    }

## Create a clustered table from a query result

There are two ways to create a clustered table from a query result:

- Write the results to a new destination table and specify the clustering columns.
- By using a DDL `CREATE TABLE AS SELECT` statement. For more information about this method, see [Creating a clustered table from the result of a query](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#creating_a_clustered_table_from_the_result_of_a_query) on the [Using data definition language statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language) page.

You can create a clustered table by querying either a partitioned table or a
non-partitioned table. You cannot change an existing table to a clustered table
by using query results.

When you create a clustered table from a query result, you must use standard
SQL. Legacy SQL is not supported for querying clustered tables or
for writing query results to clustered tables.

### SQL

To create a clustered table from a query result, use the
[`CREATE TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
with the `CLUSTER BY` option. The following example creates a new
table clustered by `customer_id` by querying an existing unclustered table:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE TABLE mydataset.clustered_table
   (
     customer_id STRING,
     transaction_amount NUMERIC
   )
   CLUSTER BY
     customer_id
   AS (
     SELECT * FROM mydataset.unclustered_table
   );
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Enter the following command to create a new, clustered destination
table from a query result:

```bash
bq --location=LOCATION query \
    --use_legacy_sql=false 'QUERY'
```

Replace the following:

- `LOCATION`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `QUERY`: a query in GoogleSQL syntax. You cannot use legacy SQL to query clustered tables or to write query results to clustered tables. The query can contain a `CREATE TABLE` [DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language) statement that specifies the options for creating your clustered table. You can use DDL rather than specifying the individual command-line flags.

Examples:

Enter the following command to write query results to a clustered
destination table named `myclusteredtable` in `mydataset`. `mydataset` is in
your default project. The query retrieves data from a non-partitioned table:
mytable. The table's `customer_id` column is used to cluster the
table. The table's `timestamp` column is used to create a partitioned table.

    bq query --use_legacy_sql=false \
        'CREATE TABLE
           mydataset.myclusteredtable
         PARTITION BY
           DATE(timestamp)
         CLUSTER BY
           customer_id
         AS (
           SELECT
             *
           FROM
             `mydataset.mytable`
         );'

### API

To save query results to a clustered table,
call the [`jobs.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert),
configure a
[`query` job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationQuery),
and include a `CREATE TABLE` [DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)
statement that creates your clustered table.

Specify your location in the `location` property in the
`jobReference` section of the
[job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

## Create a clustered table when you load data

You can create a clustered table by specifying clustering columns when you load
data into a new table. You do not need to create an empty table before loading
data into it. You can create the clustered table and load your data at the same
time.

For more information about loading data, see
[Introduction to loading data into BigQuery](https://docs.cloud.google.com/bigquery/docs/loading-data).

To define clustering when defining a load job:

### SQL

Use the [`LOAD DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example loads AVRO data to create a table that is partitioned
by the `transaction_date` field and clustered by the `customer_id` field.
It also configures the partitions to expire after three days.

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA INTO mydataset.mytable
   PARTITION BY transaction_date
   CLUSTER BY customer_id
     OPTIONS (
       partition_expiration_days = 3)
   FROM FILES(
     format = 'AVRO',
     uris = ['gs://bucket/path/file.avro']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### API

To define a clustering configuration when creating a table through a
load job, you can populate the
[`Clustering`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#clustering)
properties for the table.


<br />

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

    // importClusteredTable demonstrates creating a table from a load job and defining partitioning and clustering
    // properties.
    func importClusteredTable(projectID, destDatasetID, destTableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/sample-transactions/transactions.csv")
    	gcsRef.SkipLeadingRows = 1
    	gcsRef.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "timestamp", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "origin", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "destination", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "amount", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	}
    	loader := client.Dataset(destDatasetID).Table(destTableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)
    	loader.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TimePartitioning = &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TimePartitioning{
    		Field: "timestamp",
    	}
    	loader.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Clustering = &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Clustering{
    		Fields: []string{"origin", "destination"},
    	}
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

<br />

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Clustering.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html;
    import com.google.common.collect.ImmutableList;

    public class LoadTableClustered {

      public static void runLoadTableClustered() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "/path/to/file.csv";
        loadTableClustered(datasetName, tableName, sourceUri);
      }

      public static void loadTableClustered(String datasetName, String tableName, String sourceUri)
          throws Exception {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("date", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.DATE));

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html partitioning = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html.of(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html.Type.DAY);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Clustering.html clustering =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Clustering.html.newBuilder().setFields(ImmutableList.of("name", "post_abbr")).build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadJobConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.builder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_csv__())
                  .setSchema(schema)
                  .setTimePartitioning(partitioning)
                  .setClustering(clustering)
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html loadJob = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.newBuilder(loadJobConfig).build());

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
          System.out.println("Data successfully loaded into clustered table during load job");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Data not loaded into clustered table during load job \n" + e.toString());
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

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        skip_leading_rows=1,
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.CSV,
        schema=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("timestamp", https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SqlTypeNames.html.TIMESTAMP),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("origin", https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SqlTypeNames.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.DecimalTargetType.html#google_cloud_bigquery_enums_DecimalTargetType_STRING),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("destination", https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SqlTypeNames.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.DecimalTargetType.html#google_cloud_bigquery_enums_DecimalTargetType_STRING),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("amount", https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SqlTypeNames.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.DecimalTargetType.html#google_cloud_bigquery_enums_DecimalTargetType_NUMERIC),
        ],
        time_partitioning=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.TimePartitioning.html(field="timestamp"),
        clustering_fields=["origin", "destination"],
    )

    job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        ["gs://cloud-samples-data/bigquery/sample-transactions/transactions.csv"],
        table_id,
        job_config=job_config,
    )

    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()  # Waits for the job to complete.

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    print(
        "Loaded {} rows and {} columns to {}".format(
            table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows, len(table.schema), table_id
        )
    )

## What's next

- For information about working with clustered tables, see [Manage clustered tables](https://docs.cloud.google.com/bigquery/docs/manage-clustered-tables).
- For information about querying clustered tables, see [Querying clustered tables](https://docs.cloud.google.com/bigquery/docs/querying-clustered-tables).
- For an overview of partitioned table support in BigQuery, see [Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).
- To learn how to create partitioned tables, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables).
- To see an overview of `INFORMATION_SCHEMA`, see [Introduction to BigQuery `INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro).