# Work with SQL stored procedures

A *stored procedure* is a collection of statements that can be called from other
queries or other stored procedures. A procedure can take input arguments and
return values as output. You name and store a procedure in a
BigQuery dataset. A stored procedure can access or modify data
across multiple datasets by multiple users. It can also contain a
[multi-statement query](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries).

Some stored procedures are built into BigQuery and don't need to
be created. These are called *system procedures* and you can learn more about
them in the [System procedures reference](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures).

Stored procedures support *procedural language statements* , which let you do
things like define variables and implement control flow. You can learn more
about procedural language statements in the
[Procedural language reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language).

## Create a stored procedure

Choose one of the following options to create a stored procedure:

### SQL

To create a procedure, use the
[`CREATE PROCEDURE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure)
statement.

In the following conceptual example, `procedure_name` represents
the procedure and the body of the procedure appears between
[`BEGIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#begin) and
`END` statements:

    CREATE PROCEDURE dataset_name.procedure_name()
    BEGIN
    -- statements here
    END

The following example shows a procedure that contains a multi-statement query.
The multi-statement query sets a variable, runs an `INSERT` statement, and
displays the result as a formatted text string.

    CREATE OR REPLACE PROCEDURE mydataset.create_customer()
    BEGIN
      DECLARE id STRING;
      SET id = GENERATE_UUID();
      INSERT INTO mydataset.customers (customer_id)
        VALUES(id);
      SELECT FORMAT("Created customer %s", id);
    END

In the preceding example, the name of the procedure is
`mydataset.create_customer`, and the body of procedure appears between
[`BEGIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#begin) and
`END` statements.

To call the procedure, use the
[`CALL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#call)
statement:

    CALL mydataset.create_customer();

### Terraform

Use the
[`google_bigquery_routine` resource](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_routine).

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the Cloud Resource Manager API.

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a stored procedure named `my_stored_procedure`:

    # Creates a SQL stored procedure.

    # Create a dataset to contain the stored procedure.
    resource "google_bigquery_dataset" "my_dataset" {
      dataset_id = "my_dataset"
    }

    # Create a stored procedure.
    resource "google_bigquery_routine" "my_stored_procedure" {
      dataset_id      = google_bigquery_dataset.my_dataset.dataset_id
      routine_id      = "my_stored_procedure"
      routine_type    = "PROCEDURE"
      language        = "SQL"
      definition_body = "SELECT * FROM `bigquery-public-data.ml_datasets.penguins`;"
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

### Pass a value in with an input parameter

A procedure can have input parameters. An input parameter allows input for
a procedure, but does not allow output.

    CREATE OR REPLACE PROCEDURE mydataset.create_customer(name STRING)
    BEGIN
      DECLARE id STRING;
      SET id = GENERATE_UUID();
      INSERT INTO mydataset.customers (customer_id, name)
        VALUES(id, name);
      SELECT FORMAT("Created customer %s (%s)", id, name);
    END

### Pass a value out with an output parameter

A procedure can have output parameters. An output parameter returns a value
from the procedure, but does not allow input for the procedure. To create an
output parameter, use the `OUT` keyword before the name of the parameter.

For example, this version of the procedure returns the new customer ID through
the `id` parameter:

    CREATE OR REPLACE PROCEDURE mydataset.create_customer(name STRING, OUT id STRING)
    BEGIN
      SET id = GENERATE_UUID();
      INSERT INTO mydataset.customers (customer_id, name)
        VALUES(id, name);
      SELECT FORMAT("Created customer %s (%s)", id, name);
    END

To call this procedure, you must use a variable to receive the output value:

    --- Create a new customer record.
    DECLARE id STRING;
    CALL mydataset.create_customer("alice",id);

    --- Display the record.
    SELECT * FROM mydataset.customers
    WHERE customer_id = id;

### Pass a value in and out with an input/output parameter

A procedure can also have input/output parameters. An input/output parameter
returns a value from the procedure and also accepts input for the procedure. To
create an input/output parameter, use the `INOUT` keyword before the name of the
parameter. For more information, see
[Argument mode](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#argument_mode).

## Authorize routines

You can authorize stored procedures as *routines*.
Authorized routines let you share query results with specific users or groups
without giving them access to the underlying tables that generated the results.
For example, an authorized routine can compute an aggregation
over data or look up a table value and use that value in a computation.

Authorized routines can
[create](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement),
[drop](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_table_statement),
and [manipulate tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax),
as well as
[invoke other stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures#call_a_stored_procedure)
on the underlying table.

For more information, see [Authorized routines](https://docs.cloud.google.com/bigquery/docs/authorized-routines).

## Call a stored procedure

To call a stored procedure after it's been created, use the `CALL` statement.
For example, the following statement calls the stored procedure
`create_customer`:

    CALL mydataset.create_customer();

> [!NOTE]
> **Note:** Calling a stored procedure rather than including the procedure's SQL statements directly in your query introduces a small performance overhead.

## Call a system procedure

To call a built-in system procedure, use the `CALL` statement.
For example, the following statement calls the system procedure
`BQ.REFRESH_MATERIALIZED_VIEW`:

    CALL BQ.REFRESH_MATERIALIZED_VIEW;