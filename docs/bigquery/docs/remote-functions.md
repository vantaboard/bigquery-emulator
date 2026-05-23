# Work with remote functions

A BigQuery remote function allows you to implement your function
in other languages than SQL and JavaScript or with the libraries or services
which are not allowed in BigQuery
[user-defined functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions).

## Overview

A BigQuery remote function lets you incorporate
GoogleSQL functionality with software outside of
BigQuery by providing a direct integration with
[Cloud Run functions](https://docs.cloud.google.com/functions/docs/concepts/overview) and
[Cloud Run](https://docs.cloud.google.com/run/docs/overview/what-is-cloud-run). With
BigQuery remote functions, you can deploy your functions in
Cloud Run functions or Cloud Run implemented with any
supported language, and then invoke them from GoogleSQL
queries.

### Workflow

1. Create the HTTP endpoint in Cloud Run functions or Cloud Run.
2. Create a remote function in BigQuery.
   1. Create a connection of type `CLOUD_RESOURCE`.
   2. Create a remote function.
3. Use the remote function in a query just like any other user-defined functions.

## Limitations

- Remote functions only support one of the following
  [data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types) as argument
  type or return type:

  - Boolean
  - Bytes
  - Numeric
  - String
  - Date
  - Datetime
  - Time
  - Timestamp
  - JSON

  Remote functions do not support `ARRAY`, `STRUCT`, `INTERVAL`, or
  `GEOGRAPHY` types.
- You cannot create table-valued remote functions.

- You cannot use remote functions when creating materialized views.

- The return value of a remote function is always assumed to be
  non-deterministic so the result of a query calling a remote function is not
  cached.

- You might see repeated requests with the same data to your endpoint,
  even after successful responses, due to transient network errors or
  BigQuery internal errors.

- When a remote function evaluation is skipped for some rows due to
  short-circuiting, for example, in [conditional expressions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions)
  or a [`MERGE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement)
  with `WHEN [NOT] MATCHED`, batching is not used with the remote function.
  In this case, the `calls` field in the [HTTP request body](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/remote-functions#input_format)
  has exactly one element.

- If the dataset associated with the remote function is replicated to a
  destination region through [cross-region dataset
  replication](https://docs.cloud.google.com/bigquery/docs/data-replication), the remote function can only
  be queried in the region that it was created in.

## Create an endpoint

To create a remote function that can implement business logic, you must create
an HTTP endpoint by using either Cloud Run functions or
Cloud Run. The endpoint must be able to process a batch of rows
in a single HTTP POST request and return the results for the batch as an
HTTP response.

If you are creating the remote function by
using [BigQuery DataFrames](https://dataframes.bigquery.dev/),
you don't have to manually create the HTTP endpoint; the service does that for
you automatically.

See the
[Cloud Run functions tutorial](https://docs.cloud.google.com/functions/docs/tutorials/http) and other
[Cloud Run functions documentation](https://docs.cloud.google.com/functions/docs/writing/http) on how to
write, deploy, test and maintain a Cloud Run function.

See the
[Cloud Run quick start](https://docs.cloud.google.com/run/docs/quickstarts#build-and-deploy-a-web-service)
and other [Cloud Run documentation](https://docs.cloud.google.com/run/docs/how-to) on how to
write, deploy, test and maintain a Cloud Run service.

It's recommended that you keep the default authentication instead of allowing
unauthenticated invocation of your Cloud Run function or
Cloud Run service.

### Input format

BigQuery sends HTTP POST requests with JSON body in the following
format:

| Field name | Description | Field type |
|---|---|---|
| requestId | Id of the request. Unique over multiple requests sent to this endpoint in a GoogleSQL query. | Always provided. String. |
| caller | Job full resource name for the GoogleSQL query calling the remote function. | Always provided. String. |
| sessionUser | Email of the user executing the GoogleSQL query. | Always provided. String. |
| userDefinedContext | The user defined context that was used when creating the remote function in BigQuery. | Optional. A JSON object with key-value pairs. |
| calls | A batch of input data. | Always provided. A JSON array. Each element itself is a JSON array, which is a JSON encoded argument list of one remote function call. |

An example of a request:

    {
     "requestId": "124ab1c",
     "caller": "//bigquery.googleapis.com/projects/myproject/jobs/myproject:US.bquxjob_5b4c112c_17961fafeaf",
     "sessionUser": "test-user@test-company.com",
     "userDefinedContext": {
      "key1": "value1",
      "key2": "v2"
     },
     "calls": [
      [null, 1, "", "abc"],
      ["abc", "9007199254740993", null, null]
     ]
    }

### Output format

BigQuery expects the endpoint should return a HTTP response in
the following format, otherwise BigQuery can't consume it and
will fail the query calling the remote function.

|---|---|---|
| Field name | Description | Value Range |
| replies | A batch of return values. | Required for a successful response. A JSON array. Each element corresponds to a JSON encoded return value of the external function. Size of the array must match the size of the JSON array of `calls` in the HTTP request. For example, if the JSON array in `calls` has 4 elements, this JSON array needs to have 4 elements as well. |
| errorMessage | Error message when the HTTP response code other than 200 is returned. For non-retryable errors, we return this as part of the BigQuery job's error message to the user. | Optional. String. Size should be less than 1KB. |

An example of a successful response:

    {
      "replies": [
        1,
        0
      ]
    }

An example of a failed response:

    {
      "errorMessage": "Received but not expected that the argument 0 be null".
    }

#### HTTP response code

Your endpoint should return the HTTP response code 200 for a successful response.
When BigQuery receives any other value, BigQuery
considers the response as a failure, and retries when the HTTP response code is
408, 429, 500, 503 or 504 until some internal limit.

### JSON encoding of SQL data type

JSON encoding in HTTP request/response follows
[the existing BigQuery JSON encoding](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings)
for [TO_JSON_STRING function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json_string).

### Sample Cloud Run function code

The following sample Python code implements adding all the integer arguments of
the remote function. It handles a request with the arguments for batched
invocations and returns all the results in a response.

    import functions_framework

    from flask import jsonify

    # Max INT64 value encoded as a number in JSON by TO_JSON_STRING. Larger values are encoded as
    # strings.
    # See https://cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings
    _MAX_LOSSLESS=9007199254740992

    @functions_framework.http
    def batch_add(request):
      try:
        return_value = []
        request_json = request.get_json()
        calls = request_json['calls']
        for call in calls:
          return_value.append(sum([int(x) if isinstance(x, str) else x for x in call if x is not None]))
        replies = [str(x) if x > _MAX_LOSSLESS or x < -_MAX_LOSSLESS else x for x in return_value]
        return_json = jsonify( { "replies":  replies } )
        return return_json
      except Exception as e:
        return jsonify( { "errorMessage": str(e) } ), 400

Assuming that the function is deployed in the project `my_gcf_project` in region
`us-east1` as the function name `remote_add`, it can be accessed via the
endpoint `https://us-east1-my_gcf_project.cloudfunctions.net/remote_add`.

### Sample Cloud Run code

The following sample Python code implements a web service, which can be built
and deployed to Cloud Run for the same functionality.

    import os

    from flask import Flask, request, jsonify

    # Max INT64 value encoded as a number in JSON by TO_JSON_STRING. Larger values are encoded as
    # strings.
    # See https://cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings
    _MAX_LOSSLESS=9007199254740992

    app = Flask(__name__)

    @app.route("/", methods=['POST'])
    def batch_add():
      try:
        return_value = []
        request_json = request.get_json()
        calls = request_json['calls']
        for call in calls:
          return_value.append(sum([int(x) if isinstance(x, str) else x for x in call if x is not None]))
        replies = [str(x) if x > _MAX_LOSSLESS or x < -_MAX_LOSSLESS else x for x in return_value]
        return jsonify( { "replies" :  replies } )
      except Exception as e:
        return jsonify( { "errorMessage": str(e) } ), 400

    if __name__ == "__main__":
        app.run(debug=True, host="0.0.0.0", port=int(os.environ.get("PORT", 8080)))

See the [guide](https://docs.cloud.google.com/run/docs/quickstarts/build-and-deploy/deploy-python-service) on
how to build and deploy the code.

Assuming that the Cloud Run service is deployed in the project
`my_gcf_project` in region `us-east1` as the service name `remote_add`, it can
be accessed via the endpoint
`https://remote_add-<project_id_hash>-ue.a.run.app`.

## Create a remote function

BigQuery uses a `CLOUD_RESOURCE` connection to interact with your
Cloud Run function. In order to create a remote function, you must
create a `CLOUD_RESOURCE` connection. If you are creating the remote function by
using [BigQuery DataFrames](https://dataframes.bigquery.dev/)
and you have been granted the
Project IAM Admin (`roles/resourcemanager.projectIamAdmin`) role, then you
don't have to manually create the connection and grant it access; the service
does that for you automatically.

### Create a connection

You must have a Cloud resource connection to connect to Cloud Run function
and Cloud Run.

You can skip this step if you either have a default connection configured, or
you have the BigQuery Admin role.
Select one of the following options:

<br />

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project name, and then click
   **Connections**.

4. On the **Connections** page, click **Create connection**.

5. For **Connection type** , choose **Vertex AI remote models, remote
   functions, BigLake and Spanner (Cloud Resource)**.

6. In the **Connection ID** field, enter a name for your connection.

7. For **Location type**, select a location for your connection. The
   connection should be colocated with your other resources such as
   datasets.

8. Click **Create connection**.

9. Click **Go to connection**.

10. In the **Connection info** pane, copy the service account ID for use in
    a later step.

### SQL

Use the [`CREATE CONNECTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_connection_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE CONNECTION [IF NOT EXISTS] `CONNECTION_NAME`
   OPTIONS (
     connection_type = "CLOUD_RESOURCE",
     friendly_name = "FRIENDLY_NAME",
     description = "DESCRIPTION"
     );
   ```


   Replace the following:
   - `CONNECTION_NAME`: the name of the connection in either the `PROJECT_ID.LOCATION.CONNECTION_ID`, `LOCATION.CONNECTION_ID`, or `CONNECTION_ID` format. If the project or location are omitted, then they are inferred from the project and location where the statement is run.
   - `FRIENDLY_NAME` (optional): a descriptive name for the connection.
   - `DESCRIPTION` (optional): a description of the connection.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

1. In a command-line environment, create a connection:

   ```bash
   bq mk --connection --location=REGION --project_id=PROJECT_ID \
       --connection_type=CLOUD_RESOURCE CONNECTION_ID
   ```

   The `--project_id` parameter overrides the default project.

   Replace the following:
   - `REGION`: your [connection region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations)
   - `PROJECT_ID`: your Google Cloud project ID
   - `CONNECTION_ID`: an ID for your connection

   When you create a connection resource, BigQuery creates a
   unique system service account and associates it with the connection.

   **Troubleshooting** : If you get the following connection error,
   [update the Google Cloud SDK](https://docs.cloud.google.com/sdk/docs/quickstart):

   ```
   Flags parsing error: flag --connection_type=CLOUD_RESOURCE: value should be one of...
   ```
2. Retrieve and copy the service account ID for use in a later
   step:

   ```bash
   bq show --connection PROJECT_ID.REGION.CONNECTION_ID
   ```

   The output is similar to the following:

   ```
   name                          properties
   1234.REGION.CONNECTION_ID     {"serviceAccountId": "connection-1234-9u56h9@gcp-sa-bigquery-condel.iam.gserviceaccount.com"}
   ```

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

    import google.api_core.exceptions
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html()


    def create_connection(
        project_id: str,
        location: str,
        connection_id: str,
    ):
        """Creates a BigQuery connection to a Cloud Resource.

        Cloud Resource connection creates a service account which can then be
        granted access to other Google Cloud resources for federated queries.

        Args:
            project_id: The Google Cloud project ID.
            location: The location of the connection (for example, "us-central1").
            connection_id: The ID of the connection to create.
        """

        parent = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_common_location_path(project_id, location)

        connection = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.Connection.html(
            friendly_name="Example Connection",
            description="A sample connection for a Cloud Resource.",
            cloud_resource=https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.CloudResourceProperties.html(),
        )

        try:
            created_connection = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_create_connection(
                parent=parent, connection_id=connection_id, connection=connection
            )
            print(f"Successfully created connection: {created_connection.name}")
            print(f"Friendly name: {created_connection.friendly_name}")
            print(
                f"Service Account: {created_connection.cloud_resource.service_account_id}"
            )

        except google.api_core.exceptions.AlreadyExists:
            print(f"Connection with ID '{connection_id}' already exists.")
            print("Please use a different connection ID.")
        except Exception as e:
            print(f"An unexpected error occurred while creating the connection: {e}")

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

    const {ConnectionServiceClient} =
      require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html').v1;
    const {status} = require('@grpc/grpc-js');

    const client = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html();

    /**
     * Creates a new BigQuery connection to a Cloud Resource.
     *
     * A Cloud Resource connection creates a service account that can be granted access
     * to other Google Cloud resources.
     *
     * @param {string} projectId The Google Cloud project ID. for example, 'example-project-id'
     * @param {string} location The location of the project to create the connection in. for example, 'us-central1'
     * @param {string} connectionId The ID of the connection to create. for example, 'example-connection-id'
     */
    async function createConnection(projectId, location, connectionId) {
      const parent = client.locationPath(projectId, location);

      const connection = {
        friendlyName: 'Example Connection',
        description: 'A sample connection for a Cloud Resource',
        // The service account for this cloudResource will be created by the API.
        // Its ID will be available in the response.
        cloudResource: {},
      };

      const request = {
        parent,
        connectionId,
        connection,
      };

      try {
        const [response] = await client.createConnection(request);

        console.log(`Successfully created connection: ${response.name}`);
        console.log(`Friendly name: ${response.friendlyName}`);

        console.log(`Service Account: ${response.cloudResource.serviceAccountId}`);
      } catch (err) {
        if (err.code === status.ALREADY_EXISTS) {
          console.log(`Connection '${connectionId}' already exists.`);
        } else {
          console.error(`Error creating connection: ${err.message}`);
        }
      }
    }

### Terraform

Use the
[`google_bigquery_connection`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_connection)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the [Cloud Resource Manager API](https://docs.cloud.google.com/resource-manager/reference/rest).

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a Cloud resource connection named
`my_cloud_resource_connection` in the `US` region:


    # This queries the provider for project information.
    data "google_project" "default" {}

    # This creates a cloud resource connection in the US region named my_cloud_resource_connection.
    # Note: The cloud resource nested object has only one output field - serviceAccountId.
    resource "google_bigquery_connection" "default" {
      connection_id = "my_cloud_resource_connection"
      project       = data.google_project.default.project_id
      location      = "US"
      cloud_resource {}
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

### Set up access

You must give the new connection read-only access to your
Cloud Run function or Cloud Run service.
It is not recommended to allow unauthenticated invocation for your
Cloud Run function or Cloud Run service.

To grant roles, follow these steps:

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Add**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, select one of the following options:

   - If you are using a 1st-gen Cloud Run function, choose **Cloud Function** , and then select **Cloud Function Invoker role**.
   - If you are using a 2nd-gen Cloud Run function, choose **Cloud Run** , and then select **Cloud Run Invoker role**.
   - If you are using a Cloud Run service, choose **Cloud Run** , and then select **Cloud Run Invoker role**.
5. Click **Save**.

### Create a remote function

To create a remote function:

### SQL

Run the following
[`CREATE FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement)
statement in BigQuery:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       CREATE FUNCTION PROJECT_ID.DATASET_ID.remote_add(x INT64, y INT64) RETURNS INT64
       REMOTE WITH CONNECTION PROJECT_ID.LOCATION.CONNECTION_NAME
       OPTIONS (
         endpoint = 'ENDPOINT_URL'
       )


   Replace the following:
   - `DATASET_ID`: the ID of your BigQuery dataset.
   - `ENDPOINT_URL`: the URL of your Cloud Run function or Cloud Run remote function endpoint.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### BigQuery DataFrames

1. Enable the required APIs and make sure you have been granted the required roles, as described in the **Requirements** section of [Remote functions](https://docs.cloud.google.com/bigquery/docs/dataframes-custom-python-functions#remote-function-requirements).
2. Use the
   [`remote_function` decorator](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.remote_function.html#bigframes.pandas.remote_function):

       import bigframes.pandas as bpd

       # Set BigQuery DataFrames options
       bpd.options.bigquery.project = your_gcp_project_id
       bpd.options.bigquery.location = "US"

       # BigQuery DataFrames gives you the ability to turn your custom scalar
       # functions into a BigQuery remote function. It requires the GCP project to
       # be set up appropriately and the user having sufficient privileges to use
       # them. One can find more details about the usage and the requirements via
       # `help` command.
       help(bpd.remote_function)

       # Read a table and inspect the column of interest.
       df = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")
       df["body_mass_g"].head(10)

       # Define a custom function, and specify the intent to turn it into a remote
       # function. It requires a BigQuery connection. If the connection is not
       # already created, BigQuery DataFrames will attempt to create one assuming
       # the necessary APIs and IAM permissions are setup in the project. In our
       # examples we will be letting the default connection `bigframes-default-connection`
       # be used. We will also set `reuse=False` to make sure we don't
       # step over someone else creating remote function in the same project from
       # the exact same source code at the same time. Let's try a `pandas`-like use
       # case in which we want to apply a user defined scalar function to every
       # value in a `Series`, more specifically bucketize the `body_mass_g` value
       # of the penguins, which is a real number, into a category, which is a
       # string.
       @bpd.remote_function(
           reuse=False,
           cloud_function_service_account="default",
       )
       def get_bucket(num: float) -> str:
           if not num:
               return "NA"
           boundary = 4000
           return "at_or_above_4000" if num >= boundary else "below_4000"

       # Then we can apply the remote function on the `Series` of interest via
       # `apply` API and store the result in a new column in the DataFrame.
       df = df.assign(body_mass_bucket=df["body_mass_g"].apply(get_bucket))

       # This will add a new column `body_mass_bucket` in the DataFrame. You can
       # preview the original value and the bucketized value side by side.
       df[["body_mass_g", "body_mass_bucket"]].head(10)

       # The above operation was possible by doing all the computation on the
       # cloud. For that, there is a google cloud function deployed by serializing
       # the user code, and a BigQuery remote function created to call the cloud
       # function via the latter's http endpoint on the data in the DataFrame.

       # The BigQuery remote function created to support the BigQuery DataFrames
       # remote function can be located via a property `bigframes_remote_function`
       # set in the remote function object.
       print(f"Created BQ remote function: {get_bucket.bigframes_remote_function}")

       # The cloud function can be located via another property
       # `bigframes_cloud_function` set in the remote function object.
       print(f"Created cloud function: {get_bucket.bigframes_cloud_function}")

       # Warning: The deployed cloud function may be visible to other users with
       # sufficient privilege in the project, so the user should be careful about
       # having any sensitive data in the code that will be deployed as a remote
       # function.

       # Let's continue trying other potential use cases of remote functions. Let's
       # say we consider the `species`, `island` and `sex` of the penguins
       # sensitive information and want to redact that by replacing with their hash
       # code instead. Let's define another scalar custom function and decorate it
       # as a remote function. The custom function in this example has external
       # package dependency, which can be specified via `packages` parameter.
       @bpd.remote_function(
           reuse=False,
           packages=["cryptography"],
           cloud_function_service_account="default",
       )
       def get_hash(input: str) -> str:
           from cryptography.fernet import Fernet

           # handle missing value
           if input is None:
               input = ""

           key = Fernet.generate_key()
           f = Fernet(key)
           return f.encrypt(input.encode()).decode()

       # We can use this remote function in another `pandas`-like API `map` that
       # can be applied on a DataFrame
       df_redacted = df[["species", "island", "sex"]].map(get_hash)
       df_redacted.head(10)

You need to have the permission `bigquery.routines.create` on the dataset where
you create the remote function, and the `bigquery.connections.delegate`
permission (available from the BigQuery Connection Admin role) on the connection
that is used by the remote function.

#### Providing user defined context

You can specify `user_defined_context` in `OPTIONS` as a form of key-value
pairs, which will be part of every HTTP request to the endpoint. With user
defined context, you can create multiple remote functions but re-use a single
endpoint, that provides different behaviors based on the context passed to it.

The following examples create two remote functions to encrypt and decrypt
`BYTES` data using the same endpoint.

    CREATE FUNCTION `PROJECT_ID.DATASET_ID`.encrypt(x BYTES)
    RETURNS BYTES
    REMOTE WITH CONNECTION `PROJECT_ID.LOCATION.CONNECTION_NAME`
    OPTIONS (
      endpoint = 'ENDPOINT_URL',
      user_defined_context = [("mode", "encryption")]
    )

    CREATE FUNCTION `PROJECT_ID.DATASET_ID`.decrypt(x BYTES)
    RETURNS BYTES
    REMOTE WITH CONNECTION `PROJECT_ID.LOCATION.CONNECTION_NAME`
    OPTIONS (
      endpoint = 'ENDPOINT_URL',
      user_defined_context = [("mode", "decryption")]
    )

#### Limiting number of rows in a batch request

You can specify `max_batching_rows` in `OPTIONS` as the maximum number of rows
in each HTTP request, to avoid
[Cloud Run functions timeout](https://docs.cloud.google.com/functions/docs/concepts/exec#timeout). If you
specify `max_batching_rows`, BigQuery determines the number of
rows in a batch up to the `max_batching_rows` limit. If not specified,
BigQuery determines the number of rows to batch automatically.

## Use a remote function in a query

Make sure you have
[granted the permission on your Cloud Run function](https://docs.cloud.google.com/bigquery/docs/remote-functions#grant_permission_on_function),
so that it is accessible to BigQuery's service account associated
with the connection of the remote function.

You also need to have the permission `bigquery.routines.get` on the dataset
where the remote function is, and the `bigquery.connections.use` permission,
which you can get through the `BigQuery Connection User` role, on
the connection which is used by the remote function.

You can use a remote function in a query just like a
[user defined function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions).

For example, you can use the `remote_add` function in the example query:

    SELECT
      val,
      `PROJECT_ID.DATASET_ID`.remote_add(val, 2)
    FROM
      UNNEST([NULL,2,3,5,8]) AS val;

This example produces the following output:

    +---+---+
    |  val | f0_ |
    +---+---+
    | NULL |   2 |
    |    2 |   4 |
    |    3 |   5 |
    |    5 |   7 |
    |    8 |  10 |
    +---+---+

> [!NOTE]
> **Note:** For endpoints with `internal traffic` [ingress settings](https://docs.cloud.google.com/functions/docs/networking/network-settings#ingress_settings), you can either use the same Cloud Run functions endpoint project to run the BigQuery query or [setup a VPC-SC perimeter](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/remote-functions#using_vpc_service_controls).

## Supported regions

There are two types of locations in BigQuery:

- A *region* is a specific geographic place, such as London.

- A *multi-region* is a large geographic area, such as the United States, that
  contains two or more geographic places.

### Single regions

In a BigQuery single region dataset, you can only create a remote
function that uses a Cloud Run function deployed in the same region. For
example:

- A remote function in BigQuery single region `us-east4` can only use a Cloud Run function in `us-east4`.

So for single regions, remote functions are only supported in regions that
support both Cloud Run functions and BigQuery.

### Multi-regions

In a BigQuery multi-region (`US`, `EU`) dataset, you can only
create a remote function that uses a Cloud Run function deployed in a
region within the same large geographic area (US, EU). For example:

- A remote function in BigQuery `US` multi-region can only use a Cloud Run function deployed in any single region in the US geographic area, such as `us-central1`, `us-east4`, `us-west2`, etc.
- A remote function in BigQuery `EU` multi-region can only use a Cloud Run function deployed in any single region in [member states](https://europa.eu/european-union/about-eu/countries_en) of the European Union, such as `europe-north1`, `europe-west3`, etc.

For more information about BigQuery regions and multi-regions,
see the [Dataset Locations](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations) page.
For more information about Cloud Run functions regions, see the
[Cloud Run functions Locations](https://docs.cloud.google.com/functions/docs/locations) page.

### Connections

For either a single-region location or multi-region location, you can only
create a remote function in the same location as the connection you use. For
example, to create a remote function in the `US` multi-region, use a connection
located in the `US` multi-region.

## Pricing

- Standard [BigQuery pricing](https://cloud.google.com/bigquery/pricing) applies.

- In addition, costs may be incurred for Cloud Run functions and
  Cloud Run by using this feature. Please review the
  [Cloud Run functions](https://cloud.google.com/functions/pricing) and
  [Cloud Run](https://cloud.google.com/run/pricing) pricing pages for details.

## Using VPC Service Controls

[VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls) is a Google Cloud feature that allows
you to set up a secure perimeter to guard against data exfiltration. To use
VPC Service Controls with remote functions for additional security, or to use
endpoints with `internal traffic`
[ingress settings](https://docs.cloud.google.com/functions/docs/networking/network-settings#ingress_settings),
follow the
[VPC Service Controls guide](https://docs.cloud.google.com/vpc-service-controls/docs/create-service-perimeters#create_a_service_perimeter) to:

1. Create a service perimeter.

2. Add the BigQuery project of the query using the remote function into the perimeter.

3. Add the endpoint project into the perimeter and set `Cloud Functions API` or `Cloud Run API` in the restricted services based on your endpoint type. For more details, see [Cloud Run functions VPC Service Controls](https://docs.cloud.google.com/functions/docs/securing/using-vpc-service-controls) and [Cloud Run VPC Service Controls](https://docs.cloud.google.com/run/docs/securing/using-vpc-service-controls).

## Best practices for remote functions

- Prefilter your input: If your input can be easily filtered down before being passed to a remote
  function, your query will likely be faster and cheaper.

- Keep your Cloud Run function scalable. Scalability is a function of
  [minimum instances](https://docs.cloud.google.com/functions/docs/configuring/min-instances),
  [maximum instances](https://docs.cloud.google.com/functions/docs/configuring/max-instances), and
  [concurrency](https://docs.cloud.google.com/functions/docs/configuring/concurrency).

  - Where possible, use the default value for your Cloud Run function's maximum number of instances.
  - Note that there is no default limit for 1st gen HTTP Cloud Run functions. To avoid unbounded scaling events with 1st gen HTTP Cloud Run functions while testing or in production, we recommend [setting a limit](https://docs.cloud.google.com/functions/docs/configuring/max-instances#setting_maximum_instances_limits), for example, 3000.
- Follow other [Cloud Run function tips](https://docs.cloud.google.com/functions/docs/bestpractices/tips) for better performance. Remote function queries interacting with a high latency Cloud Run function might fail due to timeout.

- Implement your endpoint to return a right HTTP response code and payload for a
  failed response.

  - To minimize retries from BigQuery, use HTTP response codes
    other than 408, 429, 500, 503 and 504 for a failed response, and make sure to
    catch all exceptions in your function code. Otherwise, the HTTP service
    framework may automatically return 500 for any uncaught exceptions.
    You might still see retried HTTP requests when BigQuery retries
    a failed data partition or query.

  - Your endpoint should return a JSON payload in the defined format for a
    failed response. Even not strictly required, it helps
    BigQuery distinguish whether the failed response is from your
    function implementation or the infrastructure of
    Cloud Run functions/Cloud Run. For the latter,
    BigQuery may retry with a different internal limit.

## Quotas

Use the following information to troubleshoot quota issues with remote
functions.

### Maximum number of concurrent queries that contain remote functions

BigQuery returns this error when the number of concurrent
queries that contain remote functions exceeds the limit.

To learn more about remote functions limits, see
[Remote functions](https://docs.cloud.google.com/bigquery/quotas#remote_function_limits).

**Error message**

```
Exceeded rate limits: too many concurrent queries with remote functions for
this project
```

This limit can be increased. Try the workarounds and best practices first.

#### Diagnosis

To see limits for concurrent queries that contain [remote
functions](https://docs.cloud.google.com/bigquery/docs/remote-functions), see
[Remote function limits](https://docs.cloud.google.com/bigquery/quotas#remote_function_limits).

#### Resolution

- When using remote functions, adhere to [best practices for remote
  functions](https://docs.cloud.google.com/bigquery/docs/remote-functions#best_practices_for_remote_functions).
- You can request a quota increase by contacting [support](https://docs.cloud.google.com/bigquery/docs/getting-support) or [sales](https://cloud.google.com/contact). It might take several days to review and process the request. We recommend stating the priority, use case, and the project ID in the request.