# Remote functions and Translation API tutorial

This tutorial describes how to create a
[BigQuery remote function](https://docs.cloud.google.com/bigquery/docs/remote-functions),
invoke the [Cloud Translation API](https://docs.cloud.google.com/translate/docs/reference/api-overview), and
perform content translation from any language to Spanish using SQL and Python.

Use cases for this function include the following:

- Translate user comments on a website into a local language
- Translate support requests from many languages into one common language for support case workers

## Objectives

- Assign necessary roles to your account.
- Create a Cloud Run functions function.
- Create a BigQuery dataset.
- Create a BigQuery connection and service account.
- Grant permissions to the BigQuery service account.
- Create a BigQuery remote function.
- Call the BigQuery remote function.

## Costs


In this document, you use the following billable components of Google Cloud:


- [BigQuery](https://cloud.google.com/bigquery/pricing)
- [Cloud Run functions](https://cloud.google.com/functions/pricing)
- [Cloud Translation](https://cloud.google.com/translate/pricing)


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

## Before you begin

We recommend that you create a Google Cloud project for this tutorial.
Also, make sure that you have the required roles to complete this tutorial.

### Set up a Google Cloud project

To set up a Google Cloud project for this tutorial, complete these steps:

<br />

### Required roles for your account


To get the permissions that
you need to perform the tasks in this tutorial,

ask your administrator to grant you the
following IAM roles on your project:

- [BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`)
- [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`)
- [Cloud Functions Developer](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudfunctions#cloudfunctions.developer) (`roles/cloudfunctions.developer`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to perform the tasks in this tutorial. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to perform the tasks in this tutorial:

- `bigquery.datasets.create`
- `bigquery.connections.create`
- `bigquery.connections.get`
- `cloudfunctions.functions.create`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Required roles for the Compute Engine default service account

When you enabled the API for Cloud Run functions, a
[Compute Engine default service account](https://docs.cloud.google.com/compute/docs/access/service-accounts#default_service_account)
was created. To complete this tutorial, you must give this
default service account the Cloud Translation API User role.

1. [Get the ID assigned to the project](https://docs.cloud.google.com/resource-manager/docs/view-update-projects#identifying_projects).

2. Copy your Compute Engine default service account. Your default
   service account looks like this:

       PROJECT_NUMBER-compute@developer.gserviceaccount.com

   Replace `PROJECT_NUMBER` with your project ID.
3. In the Google Cloud console, go to the **IAM** page.

   [Go
   to IAM](https://console.cloud.google.com/projectselector/iam-admin/iam?supportedpurview=project,folder,organizationId)
4. Select your project.

5. Click
   **Grant access** , and then in the **New principals** field, paste the
   Compute Engine default service account that you copied earlier.

6. In the **Assign roles** list, search for and select
   **Cloud Translation API User**.

7. Click **Save**.

## Create a Cloud Run functions function

Using Cloud Run functions, create a function that translates input text into
Spanish.

1. [Create a Cloud Run functions function](https://docs.cloud.google.com/functions/docs/console-quickstart#create_a_function)
   with the following specifications:

   - For **Environment** , select **2nd gen**.
   - For **Function name** , enter `translation-handler`.
   - For **Region** , select **us-central1**.
   - For **Maximum number of instances** , enter `10`.

     This setting is in the **Runtime, build, connections and security
     settings** section.

     In this tutorial, we use a lower value than the default to control the
     request rate sent to Translation.
   - For **Runtime** , select **Python 3.10**.

   - For **Entry point** , enter `handle_translation`.

2. In the file list, select **main.py**, and then paste the following code.


   Before trying this sample, follow the Python setup instructions in the
   [BigQuery quickstart using
   client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


   For more information, see the
   [BigQuery Python API
   reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


   To authenticate to BigQuery, set up Application Default Credentials.
   For more information, see

   [Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

       from __future__ import annotations


       import flask
       import functions_framework
       from google.api_core.retry import Retry
       from google.cloud import translate

       # Construct a Translation Client object
       translate_client = translate.TranslationServiceClient()


       # Register an HTTP function with the Functions Framework
       @functions_framework.http
       def handle_translation(request: flask.Request) -> flask.Response:
           """BigQuery remote function to translate input text.

           Args:
               request: HTTP request from BigQuery
               https://cloud.google.com/bigquery/docs/reference/standard-sql/remote-functions#input_format

           Returns:
               HTTP response to BigQuery
               https://cloud.google.com/bigquery/docs/reference/standard-sql/remote-functions#output_format
           """
           try:
               # Parse request data as JSON
               request_json = request.get_json()
               # Get the project of the query
               caller = request_json["caller"]
               project = extract_project_from_caller(caller)
               if project is None:
                   return flask.make_response(
                       flask.jsonify(
                           {
                               "errorMessage": (
                                   'project can\'t be extracted from "caller":' f" {caller}."
                               )
                           }
                       ),
                       400,
                   )
               # Get the target language code, default is Spanish ("es")
               context = request_json.get("userDefinedContext", {})
               target = context.get("target_language", "es")

               calls = request_json["calls"]
               translated = translate_text([call[0] for call in calls], project, target)

               return flask.jsonify({"replies": translated})
           except Exception as err:
               return flask.make_response(
                   flask.jsonify({"errorMessage": f"Unexpected error {type(err)}:{err}"}),
                   400,
               )


       def extract_project_from_caller(job: str) -> str:
           """Extract project id from full resource name of a BigQuery job.

           Args:
               job: full resource name of a BigQuery job, like
                 "//bigquery.googleapi.com/projects/<project>/jobs/<job_id>"

           Returns:
               project id which is contained in the full resource name of the job.
           """
           path = job.split("/")
           return path[4] if len(path) > 4 else None


       def translate_text(
           calls: list[str], project: str, target_language_code: str
       ) -> list[str]:
           """Translates the input text to specified language using Translation API.

           Args:
               calls: a list of input text to translate.
               project: the project where the translate service will be used.
               target_language_code: The ISO-639 language code to use for translation
                 of the input text. See
                 https://cloud.google.com/translate/docs/advanced/discovering-supported-languages-v3#supported-target
                   for the supported language list.

           Returns:
               a list of translated text.
           """
           location = "<your location>"
           parent = f"projects/{project}/locations/{location}"
           # Call the Translation API, passing a list of values and the target language
           response = translate_client.translate_text(
               request={
                   "parent": parent,
                   "contents": calls,
                   "target_language_code": target_language_code,
                   "mime_type": "text/plain",
               },
               retry=Retry(),
           )
           # Convert the translated value to a list and return it
           return [translation.translated_text for translation in response.translations]

   <br />

   Update `<your location>` with `us-central1`.
3. In the file list, select **requirements.txt**, and then paste the following
   text:


       Flask==2.2.2
       functions-framework==3.9.2
       google-cloud-translate==3.18.0
       Werkzeug==2.3.8

   <br />

4. Click **Deploy** and wait for the function to deploy.

5. Click the **Trigger** tab.

6. Copy the **Trigger URL** value and save it for later. You must use this URL
   when you create a BigQuery remote function.

## Create a BigQuery dataset

[Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets#create-dataset)
that will contain the remote function. When you create the dataset, include
these specifications:

- For **Dataset ID** , enter `remote_function_test`.
- For **Location type** , select **Multi-region**.
- For **Multi-region** , select **US (multiple regions in United States)**.

## Create a BigQuery connection and service account

Create a BigQuery connection so that you can implement a
remote function with any supported languages in Cloud Run functions and
Cloud Run. When you create a connection, a service account is created
for that connection.

1. [Create a Google Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection)
   with the following specifications:

   - For **Connection type** , select **BigLake and remote functions (Cloud Resource)**
   - For **Connection ID** , enter `remote-function-connection`.
   - For **Location type** , select **Multi-region**.
   - For **Multi-region** , select **US (multiple regions in United States)**.
2. [Open the **Connections** list](https://docs.cloud.google.com/bigquery/docs/working-with-connections#list-connections)
   and select **`us.remote-function-connection`**.

3. Copy the service account ID and save it for later. You must grant
   permissions to this ID in the next step.

## Grant permissions to the BigQuery service account

The service account that you created in the previous step needs permission to use
Cloud Run so that the BigQuery remote function can use
the Cloud Run functions function. To grant permissions to the service account,
complete the following steps:

1. Go to the **Cloud Run** page.

   [Go to Cloud Run](https://console.cloud.google.com/project/_/run)
2. Select your project.

3. Select the checkbox next to **`translation-handler`**.

4. In the **Permissions** panel, click **Add principal**.

5. In the **New principals** field, enter the service account ID that you
   copied earlier.

6. In the **Assign roles** list, search for and select
   **Cloud Run Invoker**.

7. Click **Save**.

> [!NOTE]
> **Note:** It can take up to a minute before new permissions take effect.

## Create a BigQuery remote function

To use the Cloud Run functions function that translates text into Spanish
with a BigQuery remote function, complete the following steps.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following query:

       CREATE OR REPLACE FUNCTION `remote_function_test.translate_text`(x STRING)
       RETURNS
       STRING
           REMOTE WITH CONNECTION `us.remote-function-connection`
       OPTIONS (
           endpoint = 'TRIGGER_URL',
           max_batching_rows = 10);

   Replace `TRIGGER_URL` with the trigger URL that you
   saved earlier when you created a Cloud Run functions function.
3. Click **Run**. A message similar to the following is displayed:

   ```
   This statement created a new function named
   your_project.remote_function_test.translate_text.
   ```

> [!NOTE]
> **Note:** To limit how many rows are included in an HTTP request, the `max_batching_rows` option is set to `10`. When you do not specify the `max_batching_rows` option, BigQuery decides how many rows are included in an HTTP request.

## Call the BigQuery remote function

After you create your remote function, test it to make sure that it is linked
to the Cloud Run functions function and produces the expected results in Spanish.

1. In the BigQuery query editor, enter the following query, and
   then click **Run**.

       SELECT
         remote_function_test.translate_text('This new feature is fantastic!')
           AS translated_text;

   The results are similar to the following:

   ```
   +---+
   | translated_text                           |
   +---+
   | ¡Esta nueva característica es fantástica! |
   +---+
   ```
2. Optional: To test the remote function on a public dataset, enter the
   following query, and then click **Run** . To limit the results returned,
   use the `LIMIT` clause.

       SELECT
           text,
           remote_function_test.translate_text(text) AS translated_text
       FROM
           (SELECT text FROM `bigquery-public-data.hacker_news.full` LIMIT 3);

   The results are similar to the following:

   ```
   +---+
   | text                            | translated_text                         |
   +---+
   | These benchmarks look good.     | Estos puntos de referencia se ven bien. |
   | Who is using Java?              | ¿Quién está usando Java?                |
   | You need more database storage. | Necesitas más almacenamiento.           |
   +---+
   ```

## Delete the resources

If you don't plan to use these functions in this project, you can avoid
additional costs by deleting your project. This permanently deletes all
resources associated with the project.

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

<br />

## What's next

- Learn how to use [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions) in BigQuery.
- Learn about [Translation](https://docs.cloud.google.com/translate/docs/overview).
- Learn about [Cloud Run functions](https://docs.cloud.google.com/functions/docs/concepts/overview).
- Learn about [Cloud Run](https://docs.cloud.google.com/run/docs/overview/what-is-cloud-run).