# Customize Python functions for BigQuery DataFrames

BigQuery DataFrames lets you turn your custom Python functions into
BigQuery artifacts that you can run on
BigQuery DataFrames objects at scale. This extensibility support
lets you perform operations beyond what is possible with
BigQuery DataFrames and SQL APIs, so you can potentially take advantage
of open source libraries.

There are two variants of this extensibility mechanism: [user-defined
functions](https://docs.cloud.google.com/bigquery/docs/dataframes-custom-python-functions#udf) and [remote functions](https://docs.cloud.google.com/bigquery/docs/dataframes-custom-python-functions#remote-functions).

## Required roles


To get the permissions that
you need to complete the tasks in this document,

ask your administrator to grant you the
following IAM roles on your project:

- [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`)
- [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`)
- [Cloud Functions Developer](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudfunctions#cloudfunctions.developer) (`roles/cloudfunctions.developer`)
- [Service Account User](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountUser) (`roles/iam.serviceAccountUser`)
- [Storage Object Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.objectViewer) (`roles/storage.objectViewer`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## User-defined functions (UDFs)

With UDFs ([Preview](https://cloud.google.com/products/#product-launch-stages)),
you can turn your custom Python function into a
[Python UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python).
For an example usage, see
[Create a persistent Python UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python#bigquery-dataframes).

Creating a UDF in BigQuery DataFrames creates a BigQuery
routine as the Python UDF in the specified dataset. For a full set of
supported parameters, see
[bigframes.pandas.udf](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.udf.html#bigframes.pandas.udf).

### Requirements

To use a BigQuery DataFrames UDF, enable the
[BigQuery API](https://console.cloud.google.com/apis/library/bigquery.googleapis.com)
in your project. If you provide the `bigquery_connection` parameter in
your project, you must also enable the
[BigQuery Connection API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com).

### Clean up

In addition to cleaning up the cloud artifacts directly in the Google Cloud console
or with other tools, you can clean up the BigQuery DataFrames UDFs that
were created with an explicit name argument by using the
`bigframes.pandas.get_global_session().bqclient.delete_routine(routine_id)`
command.

### Limitations

- The code in the UDF must be self-contained, meaning, it must not contain any references to an import or variable defined outside of the function body.
- The code in the UDF must be compatible with Python 3.11, as that is the environment in which the code is executed in the cloud.
- Re-running the UDF definition code after trivial changes in the function code---for example, renaming a variable or inserting a new line---causes the UDF to be re-created, even if these changes are inconsequential to the behavior of the function.
- The user code is visible to users with read access on the BigQuery routines, so you should include sensitive content only with caution.
- A project can have up to 1,000 Cloud Run functions at a time in a BigQuery location.

The BigQuery DataFrames UDF deploys a user-defined
BigQuery Python function, and the related
[limitations](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python#python-udf-limitation)
apply.

## Remote functions

BigQuery DataFrames lets you turn your custom scalar functions into
[BigQuery remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions).
For an example usage, see
[Create a remote function](https://docs.cloud.google.com/bigquery/docs/remote-functions#bigquery-dataframes).
For a full set of supported parameters, see
[remote_function](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.remote_function.html#bigframes.pandas.remote_function).

Creating a remote function in BigQuery DataFrames creates the following:

- A [Cloud Run function](https://docs.cloud.google.com/run/docs/functions-with-run).
- A [BigQuery connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection).

  By default, a connection named `bigframes-default-connection` is used. You can
  use a pre-configured BigQuery connection if you prefer, in which
  case the connection creation is skipped. The service account for the default
  connection is granted the
  [Cloud Run role](https://docs.cloud.google.com/iam/docs/roles-permissions/run#run.invoker)
  (`roles/run.invoker`).
- A BigQuery remote function that uses the Cloud Run
  function that's been created with the BigQuery connection.

### Requirements

To use BigQuery DataFrames remote functions, you must enable the
following APIs:

- [BigQuery API (`bigquery.googleapis.com`)](https://docs.cloud.google.com/bigquery/docs/reference/rest)
- [BigQuery Connection API (`bigqueryconnection.googleapis.com`)](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest)
- [Cloud Functions API (`cloudfunctions.googleapis.com`)](https://docs.cloud.google.com/functions/docs/reference/rest)
- [Cloud Run Admin API (`run.googleapis.com`)](https://docs.cloud.google.com/run/docs/reference/rest)
- [Artifact Registry API (`artifactregistry.googleapis.com`)](https://docs.cloud.google.com/artifact-registry/docs/reference/rest)
- [Cloud Build API (`cloudbuild.googleapis.com`)](https://docs.cloud.google.com/build/docs/api/reference/rest)
- [Compute Engine API (`compute.googleapis.com`)](https://docs.cloud.google.com/compute/docs/reference/rest/v1)
- [Cloud Resource Manager API (`cloudresourcemanager.googleapis.com`)](https://docs.cloud.google.com/resource-manager/reference/rest)

When you use BigQuery DataFrames remote functions, you need the
[Project IAM Admin role](https://docs.cloud.google.com/iam/docs/roles-permissions/resourcemanager#resourcemanager.projectIamAdmin) (`roles/resourcemanager.projectIamAdmin`)
if you're using a default BigQuery connection, or the
[Browser role](https://docs.cloud.google.com/iam/docs/roles-permissions/browser#browser) (`roles/browser`)
if you're using a pre-configured connection. You can avoid this requirement by
setting the `bigframes.pandas.options.bigquery.skip_bq_connection_check` option
to `True`, in which case the connection (default or pre-configured) is used
as-is without any existence or permission check. If you're using the
pre-configured connection and skipping the connection check, verify the
following:

- The connection is created in the right location.
- If you're using BigQuery DataFrames remote functions, the service account has the [Cloud Run Invoker role](https://docs.cloud.google.com/iam/docs/roles-permissions/run#run.invoker) (`roles/run.invoker`) on the project.

### View and manage connections

BigQuery connections are created in the same location as the
BigQuery DataFrames session, using the name you provide in the custom
function definition. To view and manage connections, do the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Select the project in which you created the remote function.

3. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
4. In the **Explorer** pane, expand the project, and then click
   **Connections**.

BigQuery remote functions are created in the dataset you specify,
or they are created in an anonymous dataset, which is a type of
[hidden dataset](https://docs.cloud.google.com/bigquery/docs/datasets#hidden_datasets).
If you don't set a name for a remote function during its creation,
BigQuery DataFrames applies a default name that begins with the
`bigframes` prefix. To view and manage remote functions created in a
user-specified dataset, do the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Select the project in which you created the remote function.

3. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
4. In the **Explorer** pane, expand the project, and then click **Datasets**.

5. Click the dataset in which you created the remote function.

6. Click the **Routines** tab.

To view and manage Cloud Run functions, do the following:

1. Go to the **Cloud Run** page.

   [Go to Cloud Run](https://console.cloud.google.com/project/_/run)
2. Select the project in which you created the function.

3. In the list of available services, filter on **Function Deployment type**.

4. To identify functions created by BigQuery DataFrames, look for
   function names with the `bigframes` prefix.

### Clean up

In addition to cleaning up the cloud artifacts directly in the Google Cloud console
or with other tools, you can clean up the BigQuery remote
functions that were created without an explicit name argument and their
associated Cloud Run functions in the following ways:

- For a BigQuery DataFrames session, use the `session.close()` command.
- For the default BigQuery DataFrames session, use the `bigframes.pandas.close_session()` command.
- For a past session with `session_id`, use the `bigframes.pandas.clean_up_by_session_id(session_id)` command.

You can also clean up the BigQuery remote functions that were
created with an explicit name argument and their associated
Cloud Run functions by using the
`bigframes.pandas.get_global_session().bqclient.delete_routine(routine_id)`
command.

### Limitations

- Remote functions take about 90 seconds to become usable when you first create them. Additional package dependencies might add to the latency.
- Re-running the remote function definition code after trivial changes in and around the function code---for example, renaming a variable, inserting a new line, or inserting a new cell in the notebook---might cause the remote function to be re-created, even if these changes are inconsequential to the behavior of the function.
- The user code is visible to users with read access on the Cloud Run functions, so you should include sensitive content only with caution.
- A project can have up to 1,000 Cloud Run functions at a time in a region. For more information, see [Quotas](https://docs.cloud.google.com/functions/quotas).

## What's next

- Learn about [ML and AI capabilities](https://docs.cloud.google.com/bigquery/docs/dataframes-ml-ai) with BigQuery DataFrames.
- Learn how to [generate BigQuery DataFrames code with Gemini](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#dataframe).
- Learn how to [analyze package downloads from PyPI with BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/dataframes/pypi.ipynb).
- View BigQuery DataFrames [source code](https://github.com/googleapis/python-bigquery-dataframes), [sample notebooks](https://github.com/googleapis/python-bigquery-dataframes/tree/main/notebooks), and [samples](https://github.com/googleapis/python-bigquery-dataframes/tree/main/samples/snippets) on GitHub.
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).