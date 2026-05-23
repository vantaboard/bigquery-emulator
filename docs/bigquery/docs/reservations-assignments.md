# Manage workload assignments

The BigQuery Reservation API lets you purchase dedicated slots (called
[*commitments*](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments)), create pools
of slots (called [*reservations*](https://docs.cloud.google.com/bigquery/docs/reservations-intro#reservations)),
and assign projects, folders, and organizations to those reservations.

> [!CAUTION]
> **Caution:** The assignee and the reservation must be in the same organization and in the same location. If you move the assignee to a different organization after the assignment is created, [reservations monitoring](https://docs.cloud.google.com/bigquery/docs/reservations-monitoring) will be inaccurate.

## Create reservation assignments

To use the slots that you purchase, you create an *assignment* which assigns a
project, folder, or organization to a slot reservation. You can't assign or
allocate a specific number of slots at the assignment level; slots are managed
and assigned at the reservation level.

Projects use the single most specific reservation in the resource hierarchy to
which they are assigned. A folder assignment overrides an organization
assignment, and a project assignment overrides a folder assignment. Folder and
organization assignments are not available to [standard
edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) reservations.

In order to create an assignment on a reservation, the reservation must fulfill
at least one of the following criteria:

- It is configured with a non-zero amount of assigned baseline slots.

- It is configured with a non-zero amount of autoscaling slots.

- It is configured to use idle slots, and there are available idle slots within
  the project.

If you attempt to assign a resource to a reservation that doesn't meet at least
one of these criteria, you receive the following
message: `Assignment is pending, your project will be executed as on-demand.`

You can assign a resource to a [failover
reservation](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery), but the assignment pends
in the secondary location.

### Required permissions

To create a reservation assignment, you need the following
Identity and Access Management (IAM) permission:

- `bigquery.reservationAssignments.create` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) and the assignee.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Assign an organization to a reservation

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Reservations** tab.

4. Find the reservation in the table of reservations.

5. Expand the

   **Actions** option.

6. Click **Create assignment**.

7. In the **Create an assignment** section, click **Browse**.

8. Browse or search for the organization and select it.

9. In the **Job Type** section, select a job type to assign for
   this reservation. Options include the following:

   - `QUERY`
   - `CONTINUOUS`
   - `PIPELINE`
   - `BACKGROUND`
   - `ML_EXTERNAL`

   For more information about job types, see
   [Reservation assignments](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments).
   This default value is `QUERY`.

   To learn more about allowing users to use
   Gemini in BigQuery with Enterprise Plus edition
   assignments, see
   [Setup Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).
10. Click **Create**.

### SQL

To assign an organization to a reservation, use the
[`CREATE ASSIGNMENT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_assignment_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE ASSIGNMENT
     `ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME.ASSIGNMENT_ID`
   OPTIONS (
     assignee = 'organizations/ORGANIZATION_ID',
     job_type = 'JOB_TYPE');
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
   - `RESERVATION_NAME`: the name of the reservation
   - `ASSIGNMENT_ID`: the ID of the assignment

     The ID must be unique to the project and location,
     start and end with a lowercase letter or a number,
     and contain only lowercase letters, numbers, and dashes.
   - `ORGANIZATION_ID`: the [organization ID](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization#retrieving_your_organization_id)
   - `JOB_TYPE`: the [type of job](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) to assign to this reservation, such as `QUERY`, `CONTINUOUS`, `PIPELINE`, `BACKGROUND`, or `ML_EXTERNAL`

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To assign an organization's jobs to a reservation, use the `bq mk` command
with the `--reservation_assignment` flag:

```
bq mk \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation_assignment \
    --reservation_id=RESERVATION_NAME \
    --assignee_id=ORGANIZATION_ID \
    --job_type=JOB_TYPE \
    --assignee_type=ORGANIZATION
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
- `RESERVATION_NAME`: the name of the reservation
- `ORGANIZATION_ID`: the [organization ID](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization#retrieving_your_organization_id)
- `JOB_TYPE`: the [type of job](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) to assign to this reservation, such as `QUERY`, `CONTINUOUS`, `PIPELINE`, `BACKGROUND`, or `ML_EXTERNAL`

When you create a reservation assignment, wait at least 5 minutes before running
a query. Otherwise the query might be billed using on-demand pricing.

### Assign a project or folder to a reservation

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Reservations** tab.

4. Find the reservation in the table of reservations.

5. Expand the

   **Actions** option.

6. Click **Create assignment**.

7. In the **Create an assignment** section, click **Browse**.

8. Browse or search for the project or folder and select it.

9. In the **Job Type** section, select a job type to assign for
   this reservation. Options include the following:

   - `QUERY`
   - `CONTINUOUS`
   - `PIPELINE`
   - `BACKGROUND`
   - `ML_EXTERNAL`

   Creation and modification of more granular background job types such as
   `BACKGROUND_COLUMN_METADATA_INDEX` are not yet supported through the console.

   For more information about job types, see
   [reservation assignments](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments).
   This default value is `QUERY`.
10. Click **Create**.

### SQL

To assign a project to a reservation, use the
[`CREATE ASSIGNMENT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_assignment_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE ASSIGNMENT
     `ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME.ASSIGNMENT_ID`
   OPTIONS(
     assignee="projects/PROJECT_ID",
     job_type="JOB_TYPE");
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
   - `RESERVATION_NAME`: the name of the reservation
   - `ASSIGNMENT_ID`: the ID of the assignment

     The ID must be unique to the project and location,
     start and end with a lowercase letter or a number,
     and contain only lowercase letters, numbers, and dashes.
   - `PROJECT_ID`: the ID of the project to assign to the reservation
   - `JOB_TYPE`: the [type of job](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) to assign to this reservation, such as `QUERY`, `CONTINUOUS`, `PIPELINE`, `BACKGROUND_CHANGE_DATA_CAPTURE`, `BACKGROUND_COLUMN_METADATA_INDEX`, `BACKGROUND_SEARCH_INDEX_REFRESH`, `BACKGROUND`, or `ML_EXTERNAL`

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To assign jobs to a reservation, use the `bq mk` command with the
`--reservation_assignment` flag:

```
bq mk \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation_assignment \
    --reservation_id=RESERVATION_NAME \
    --assignee_id=PROJECT_ID \
    --job_type=JOB_TYPE \
    --assignee_type=PROJECT
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
- `RESERVATION_NAME`: the name of the reservation
- `PROJECT_ID`: the ID of the project to assign to this reservation
- `JOB_TYPE`: the [type of job](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) to assign to this reservation, such as `QUERY`, `CONTINUOUS`, `PIPELINE`, `BACKGROUND_CHANGE_DATA_CAPTURE`, `BACKGROUND_COLUMN_METADATA_INDEX`, `BACKGROUND_SEARCH_INDEX_REFRESH`, `BACKGROUND`, or `ML_EXTERNAL`

### Terraform

Use the
[`google_bigquery_reservation_assignment`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_reservation_assignment)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the [Cloud Resource Manager API](https://docs.cloud.google.com/resource-manager/reference/rest).

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example assigns a project to the reservation named
`my-reservation`:

    resource "google_bigquery_reservation" "default" {
      name              = "my-reservation"
      location          = "us-central1"
      slot_capacity     = 100
      edition           = "ENTERPRISE"
      ignore_idle_slots = false # Use idle slots from other reservations
      concurrency       = 0     # Automatically adjust query concurrency based on available resources
      autoscale {
        max_slots = 200 # Allow the reservation to scale up to 300 slots (slot_capacity + max_slots) if needed
      }
    }

    data "google_project" "project" {}

    resource "google_bigquery_reservation_assignment" "default" {
      assignee    = "projects/${data.google_project.project.project_id}"
      job_type    = "QUERY"
      reservation = google_bigquery_reservation.default.id
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

When you create a reservation assignment, wait at least 5 minutes before running
a query. Otherwise the query might be billed using on-demand pricing.

To make a project that only uses [idle
slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots), [create a
reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_reservations) with `0`
slots assigned to it, then follow the prior steps to assign the project to that
reservation.

> [!NOTE]
> **Note:** A project can be assigned to at most one reservation in a single region.

### Assign a project to `none`

Assignments to `none` represent the absence of an assignment. Projects assigned
to `none` use on-demand pricing.

> [!NOTE]
> **Note:** Assignments to `none` are supported for QUERY jobs only.

### SQL

To assign a project to `none`, use the
[`CREATE ASSIGNMENT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_assignment_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE ASSIGNMENT
     `ADMIN_PROJECT_ID.region-LOCATION.none.ASSIGNMENT_ID`
   OPTIONS(
     assignee="projects/PROJECT_ID",
     job_type="QUERY");
   ```


   Replace the following:
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of jobs that should use on-demand pricing
   - `ASSIGNMENT_ID`: the ID of the assignment

     The ID must be unique to the project and location,
     start and end with a lowercase letter or a number,
     and contain only lowercase letters, numbers, and dashes.
   - `PROJECT_ID`: the ID of the project to assign to the reservation

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To assign a project to `none`, use the `bq mk` command with the
`--reservation_assignment` flag:

```
bq mk \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation_assignment \
    --reservation_id=none \
    --job_type=QUERY \
    --assignee_id=PROJECT_ID \
    --assignee_type=PROJECT
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of jobs that should use on-demand pricing
- `PROJECT_ID`: the ID of the project to assign to `none`

### Terraform

Use the
[`google_bigquery_reservation_assignment`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_reservation_assignment)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the [Cloud Resource Manager API](https://docs.cloud.google.com/resource-manager/reference/rest).

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example assigns a project to `none`:

    data "google_project" "project" {}

    resource "google_bigquery_reservation_assignment" "default" {
      assignee    = "projects/${data.google_project.project.project_id}"
      job_type    = "QUERY"
      reservation = "projects/${data.google_project.project.project_id}/locations/us/reservations/none"
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

If there are no reservations in the administration project, then you
must use the bq command-line tool to view projects assigned to `none`.

### Override a reservation on a query

To use a specific reservation in a query, you need the following
Identity and Access Management (IAM) permission:

- `bigquery.reservations.use` on the reservation or its [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project).

To assign a query to run in a specific reservation, do one of the following:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **SQL query**.

3. In the query editor, enter a valid GoogleSQL query.

4. Click **More** , and then
   click **Query settings**.

5. Clear the **Automatic location setting** checkbox, and then select the
   region or multi-region the reservation is in.

6. In the **Reservation** list, select the reservation you want the query
   to run in.

7. Click **Save**.

8. [Write a query in the editor tab and run
   it](https://docs.cloud.google.com/bigquery/docs/running-queries). The query runs in the reservation
   you specified.

### SQL

Set the `@@reservation` system variable in the session to assign the
reservation your query runs in:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SET @@reservation='RESERVATION';
   SELECT QUERY;
   ```


   Replace the following:
   - `RESERVATION`: the reservation you want the
     query to run in.

   - `QUERY`: the query you want to run.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

For example, the following query uses the
[`SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#set)
statement to set the reservation to the `test-reservation` in the `US`
multi-region, then calls a basic query:

```sql
SET @@reservation='projects/project1/locations/US/reservations/test-reservation';
SELECT 42;
```

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. In Cloud Shell, run the query by using the
   [`bq query` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query)
   with the `--reservation_id` flag:

   ```bash
   bq query --use_legacy_sql=false --reservation_id=RESERVATION_ID
   'QUERY'
   ```

   Replace the following:
   - `RESERVATION_ID`: the reservation you
     want to run the query in.

   - `QUERY`: the SQL statement for the query.

   For example, the following query runs in the `test-reservation`
   reservation in the `US` multi-region:

   ```bash
   bq query --reservation_id=project1.US:test-reservation 'SELECT 42;'
   ```

### API

To specify a reservation using the API, [insert a new
job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) and populate the `query`
job configuration property. Specify your reservation in the `reservation`
field.

> [!CAUTION]
> **Caution:** A query can use a reservation declared in another project. However, the query and the reservation must be in the same organization and in the same location.

### Assign slots to BigQuery ML workloads

The following sections provide information on reservation
assignment requirements for BigQuery ML models.
You can create these reservation assignments by following the procedures in
[Assign an organization to a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign-organization)
or
[Assign a project or folder to a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign_my_prod_project_to_prod_reservation).

#### External models

The following BigQuery ML model types use external services:

- [Autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder)
- [AutoML](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl)
- [Boosted tree](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
- [Deep Neural Network (DNN)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models)
- [Random forest](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest)
- [Wide-and-Deep Network](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models)

You can assign reserved slots to queries using these services by creating a
reservation assignment that uses the `ML_EXTERNAL` job type. If no
reservation assignment with an `ML_EXTERNAL` job type is found, the query job
runs using [on-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing).

For external model training jobs, the slots in the reservation assignment are
used for preprocessing, training, and postprocessing steps. During training,
the slots aren't preemptible, but during preprocessing and postprocessing,
idle slots can be used.

#### Matrix factorization models

To create a matrix factorization model you must
[create a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_reservations)
that uses the BigQuery
[Enterprise or Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro),
and then create a reservation assignment that uses the `QUERY` job type.

#### Other model types

For BigQuery ML models that aren't external models or matrix
factorization models, you can assign reserved slots to queries using these
services by creating a reservation assignment that uses the `QUERY` job type.
If no reservation assignment with a `QUERY` job type is found, the query job
runs using [on-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing).

## Find reservation assignments

### Required permissions

To search for a reservation assignment for a given project, folder, or
organization, you need the following Identity and Access Management (IAM) permission:

- `bigquery.reservationAssignments.list` on the administration project.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`
- `BigQuery Resource Viewer`
- `BigQuery User`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Find a project's reservation assignment

You can find out if your project, folder, or organization is assigned to a
reservation by doing the following:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Reservations** tab.

4. In the table of reservations, expand a reservation to see what resources
   are assigned to that reservation, or use the **Filter** field to filter
   by resource name.

### SQL

To find which reservation your project's query jobs are assigned to, query
the [`INFORMATION_SCHEMA.ASSIGNMENTS_BY_PROJECT` view](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations#schema).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
     SELECT
       assignment_id
     FROM `region-LOCATION`.INFORMATION_SCHEMA.ASSIGNMENTS_BY_PROJECT
     WHERE
       assignee_id = 'PROJECT_ID'
       AND job_type = 'JOB_TYPE';
   ```


   Replace the following:
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of reservations to view
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `PROJECT_ID`: the ID of the project to assign to the reservation
   - `JOB_TYPE`: the [type of job](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) to assign to this reservation, such as `QUERY`, `CONTINUOUS`, `PIPELINE`, `BACKGROUND`, or `ML_EXTERNAL`

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

> [!CAUTION]
> **Caution:** This command does not work in Cloud Shell. To use this command, run it from a local command line.

To find which reservation your project's query jobs are assigned to, use the
`bq show` command with the `--reservation_assignment` flag:

```
bq show \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation_assignment \
    --job_type=JOB_TYPE \
    --assignee_id=PROJECT_ID \
    --assignee_type=PROJECT
```

Replace the following:

- `ADMIN_PROJECT_ID`: the ID of the project that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of reservations to view
- `JOB_TYPE`: the [type of job](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) to assign to this reservation, such as `QUERY`, `CONTINUOUS`, `PIPELINE`, `BACKGROUND`, or `ML_EXTERNAL`
- `PROJECT_ID`: the ID of the project

## Update reservation assignments

### Move an assignment to a different reservation

You can move an assignment from one reservation to another reservation.

To move a reservation assignment, you need the following
Identity and Access Management (IAM) permissions on the
[administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project)
and the assignee.

- `bigquery.reservationAssignments.create`
- `bigquery.reservationAssignments.delete`

Each of the following predefined IAM roles includes these
permissions:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

To move an assignment, use the `bq update` command:

```
bq update \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation_assignment \
    --destination_reservation_id=DESTINATION_RESERVATION \
    ADMIN_PROJECT_ID:LOCATION.RESERVATION_NAME.ASSIGNMENT_ID
```

Replace the following:

- `ADMIN_PROJECT_ID`: the ID of the project that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the new reservation
- `RESERVATION_NAME`: the reservation to move the assignment from
- `DESTINATION_RESERVATION`: the reservation to move the assignment to
- `ASSIGNMENT_ID`: the ID of the assignment

  To get the assignment ID, see
  [List a project's reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#list-assignment).

> [!NOTE]
> **Note:** Updated reservation assignments only apply to new jobs. Existing jobs continue to use their original reservation assignment.

## Delete reservation assignments

You can remove a project from a reservation by deleting the reservation
assignment. If a project is not assigned to any reservations, it inherits any
assignments in its parent folders or organizations, or else uses on-demand
pricing if there are no parent assignments.

When you delete a reservation assignment, the jobs executing with slots from
that reservation continue to run until completion.

### Required permissions

To delete a reservation assignment, you need the following
Identity and Access Management (IAM) permission:

- `bigquery.reservationAssignments.delete` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) and the assignee.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`

### Remove a project from a reservation

To remove a project from a reservation:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Reservations** tab.

4. In the table of reservations, expand the reservation to find the
   project.

5. Expand the

   **Actions** option.

6. Click **Delete**.

### SQL

Use the
[`DROP ASSIGNMENT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_assignment_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   DROP ASSIGNMENT
     `ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME.ASSIGNMENT_ID`;
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
   - `RESERVATION_NAME`: the name of the reservation
   - `ASSIGNMENT_ID`: the ID of the assignment

     To find the assignment ID, see
     [List a project's reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#list-assignment).

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To remove a project from a reservation, use the `bq rm` command with the
`--reservation_assignment` flag:

```
bq rm \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation_assignment RESERVATION_NAME.ASSIGNMENT_ID
```

Replace the following:

- `ADMIN_PROJECT_ID`: the ID of the project that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
- `RESERVATION_NAME`: the name of the reservation
- `ASSIGNMENT_ID`: the ID of the assignment

  To get the assignment ID, see
  [Find a project's reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#list-assignment).