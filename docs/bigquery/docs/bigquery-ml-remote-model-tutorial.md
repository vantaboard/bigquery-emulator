In this tutorial, you register a Vertex AI endpoint as a remote model
in BigQuery. Then, you use the [`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict) to make
predictions using the remote model.

You can use remote models when a model is too large to import into
BigQuery. They are also useful when you want to have a single
point of inference for online, batch, and micro-batch use cases.

> [!NOTE]
> **Note:** For a version of this tutorial that uses Python in a BigQuery notebook, see the [BQML Remote Model Tutorial](https://github.com/GoogleCloudPlatform/bigquery-ml-utils/blob/master/notebooks/bqml-inference-remote-model-tutorial.md) in GitHub.

## Objectives

- Import a pretrained TensorFlow model into the Vertex AI Model Registry.
- Deploy the model to a Vertex AI endpoint.
- Create a Cloud resource connection.
- Use the `CREATE MODEL` statement to create a remote model in BigQuery.
- Use the `ML.PREDICT` function to make predictions with the remote model.

## Costs


In this document, you use the following billable components of Google Cloud:


- [BigQuery](https://cloud.google.com/bigquery/pricing)
- [BigQuery ML](https://cloud.google.com/bigquery/pricing#bqml)
- [Vertex AI](https://docs.cloud.google.com/vertex-ai/pricing)


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

When you finish the tasks that are described in this document, you can avoid
continued billing by deleting the resources that you created. For more information, see
[Clean up](https://docs.cloud.google.com/bigquery/docs/bigquery-ml-remote-model-tutorial#clean-up).

## Before you begin

1.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

2. 
3.


   Enable the BigQuery, Vertex AI, Cloud Storage, and BigQuery Connection APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,aiplatform.googleapis.com,storage-component.googleapis.com,bigqueryconnection.googleapis.com)
4. Ensure that you have the [necessary permissions](https://docs.cloud.google.com/bigquery/docs/bigquery-ml-remote-model-tutorial#required_permissions) to perform the tasks in this document.

<br />

### Required roles

If you create a new project, you are the project owner, and you are granted all
of the required IAM permissions that you need to complete this
tutorial.

If you are using an existing project do the following.


Make sure that you have the following role or roles on the project:


- [BigQuery Studio Admin](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.studioAdmin) (`roles/bigquery.studioAdmin`)
- [Vertex AI User](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.user) (`roles/aiplatform.user`)
- [BigQuery Connection Admin](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`)

<br />

#### Check for the roles

1.
   In the Google Cloud console, go to the **IAM** page.

   [Go to IAM](https://console.cloud.google.com/projectselector/iam-admin/iam?supportedpurview=project)
2. Select the project.
3.
   In the **Principal** column, find all rows that identify you or a group that
   you're included in. To learn which groups you're included in, contact your
   administrator.

4. For all rows that specify or include you, check the **Role** column to see whether the list of roles includes the required roles.

#### Grant the roles

1.
   In the Google Cloud console, go to the **IAM** page.

   [Go to IAM](https://console.cloud.google.com/projectselector/iam-admin/iam?supportedpurview=project)
2. Select the project.
3. Click **Grant access**.
4.
   In the **New principals** field, enter your user identifier.

   This is typically the email address for a Google Account.

5. Click **Select a role**, then search for the role.
6. To grant additional roles, click **Add
   another role** and add each additional role.
7. Click **Save**.

For more information about IAM permissions in BigQuery,
see [BigQuery permissions](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions).

## Import the model to the Vertex AI Model Registry

In this tutorial you use a pretrained TensorFlow model that is
available in Cloud Storage at
`gs://cloud-samples-data/bigquery/ml/remote_model_tutorial/`. The
Cloud Storage bucket is in the `US` multi-region location.

The model is a TensorFlow model that's named `saved_model.pb`. It is a
customized sentiment analysis model that was created by fine-tuning a BERT model
with plain text IMDB movie reviews. The model uses text input from the movie
reviews and returns sentiment scores between zero and one. When you import the
model into the Model Registry, you use a prebuilt TensorFlow
container.

> [!NOTE]
> **Note:** For a tutorial on creating the sample model, see [Classify text with
> BERT](https://www.tensorflow.org/text/tutorials/classify_text_with_bert) in the TensorFlow documentation.

Follow these steps to import the model.

1. In the Google Cloud console, go to the Vertex AI **Model
   Registry** page.

   [Go to Model Registry](https://console.cloud.google.com/vertex-ai/models)
2. Click **Import**.

3. For **Step one: Name and region**, do the following:

   1. Select **Import as new model**.

   2. For **Name** , enter `bert_sentiment`.

   3. For **Description** , enter `BQML tutorial model`.

   4. For **Region** , select `us-central1`. You must choose a US-based region
      because the Cloud Storage bucket is in the `US` multi-region
      location.

   5. Click **Continue**.

4. For **Step two: Model settings**, do the following:

   1. Select **Import model artifacts into a new prebuilt container**.

   2. In the **Prebuilt container settings** section, do the following:

      1. For **Model framework** , choose **TensorFlow**.

      2. For **Model framework version** , choose **2.15**.

      3. For **Accelerator type** , choose **GPU**.

      4. For **Model artifact location** , enter
         `gs://cloud-samples-data/bigquery/ml/remote_model_tutorial/`.

      5. Leave the default values for all remaining options and click
         **Import**.

After the import is complete, your model appears on the **Model Registry** page.

## Deploy the model to a Vertex AI endpoint

Follow these steps to deploy the model to an endpoint.

1. In the Google Cloud console go to the Vertex AI **Model
   Registry** page.

   [Go to Model Registry](https://console.cloud.google.com/vertex-ai/models)
2. In the **Name** column, click **`bert_sentiment`**.

3. Click the **Deploy \& Test** tab.

4. Click **Deploy to endpoint**.

5. For step one, **Define your endpoint**, do the following:

   1. Click **Create new endpoint**.

   2. For **Endpoint name** , enter **`bert sentiment endpoint`**.

   3. Leave the remaining default values and click **Continue**.

6. For step two, **Model settings**, do the following:

   1. In the **Compute settings** section, for **Minimum number of compute
      nodes** , enter `1`. This is the number of nodes that need to be
      available to the model at all times.

      > [!NOTE]
      > **Note:** In production, you should set the maximum number of compute nodes. This option turns on the autoscaling capability in Vertex AI, and it allows the endpoint to process more requests when your BigQuery table has a large number of rows.

   2. In the **Advanced scaling options** section, for **Machine type** ,
      choose **Standard (n1-standard-2)**. Because you chose GPU as the
      accelerator type when you imported the model, after you choose the
      machine type, the accelerator type and accelerator count are set
      automatically.

   3. Leave the remaining default values and click **Deploy**.

      When the model is deployed to the endpoint, the status changes to
      `Active`.
   4. Copy the numeric endpoint ID in the **ID** column and the value in the
      **Region** column. You'll need them later.

## Create a dataset

Create a BigQuery dataset to store your ML model.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click your project name.

3. Click **View actions \> Create dataset**

4. On the **Create dataset** page, do the following:

   - For **Dataset ID** , enter `bqml_tutorial`.

   - For **Location type** , select **Multi-region** , and then select
     **US**.

   - Leave the remaining default settings as they are, and click
     **Create dataset**.

### bq

To create a new dataset, use the
[`bq mk --dataset` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset).

1. Create a dataset named `bqml_tutorial` with the data location set to `US`.

   ```
   bq mk --dataset \
     --location=US \
     --description "BigQuery ML tutorial dataset." \
     bqml_tutorial
   ```
2. Confirm that the dataset was created:

   ```bash
   bq ls
   ```

### API

Call the [`datasets.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert)
method with a defined [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

<br />

```json
{
  "datasetReference": {
     "datasetId": "bqml_tutorial"
  }
}
```

## Create a BigQuery Cloud resource connection

You must have a Cloud resource connection to connect to a Vertex AI
endpoint.

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Add data**.

   The **Add data** dialog opens.
4. In the **Filter By** pane, in the **Data Source Type** section, select **Databases**.

   Alternatively, in the **Search for data sources** field, you can enter
   `Vertex AI`.
5. In the **Featured data sources** section, click **Vertex AI**.

6. Click the **Vertex AI Models: BigQuery Federation** solution card.

7. In the **Connection type** list, select **Vertex AI remote models,
   remote functions and BigLake (Cloud Resource)**.

8. In the **Connection ID** field, enter `bqml_tutorial`.

9. Verify that **Multi-region---US** is selected.

10. Click **Create connection**.

11. At the bottom of the window, click **Go to connection** . Alternatively, in
    the **Explorer** pane, click **Connections** , and then click
    **`us.bqml_tutorial`**.

12. In the **Connection info** pane, copy the Service account ID. You need
    this ID when you configure permissions for the connection. When you create
    a connection resource, BigQuery creates a unique system
    service account and associates it with the connection.

### bq

1. Create a connection:

   ```bash
   bq mk --connection --location=US --project_id=PROJECT_ID \
       --connection_type=CLOUD_RESOURCE bqml_tutorial
   ```

   Replace `PROJECT_ID` with your
   Google Cloud project ID. The `--project_id` parameter overrides the
   default project.

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
   bq show --connection PROJECT_ID.us.bqml_tutorial
   ```

   The output is similar to the following:

   ```
   name                          properties
   1234.REGION.CONNECTION_ID {"serviceAccountId": "connection-1234-9u56h9@gcp-sa-bigquery-condel.iam.gserviceaccount.com"}
   ```

## Set up connection access

Grant the Vertex AI User role to the Cloud resource connection's service
account. You must grant this role in the same project where you created the
remote model endpoint.

> [!NOTE]
> **Note:** If the connection is in a different project, this error is returned: `bqcx-1234567890-xxxx@gcp-sa-bigquery-condel.iam.gserviceaccount.com does not have the permission to access
> resource`.

To grant the role, follow these steps:

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Grant Access**.

3. In the **New principals** field, enter the Cloud resource connection's
   service account ID that you copied previously.

4. In the **Select a role** field, choose **Vertex AI** , and then select
   **Vertex AI User**.

5. Click **Save**.

## Create a BigQuery ML remote model

You create a BigQuery ML remote model by using the `CREATE MODEL`
statement with the `REMOTE WITH CONNECTION` clause. For more information on
the `CREATE MODEL` statement, see [The CREATE MODEL statement for remote models
over custom models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https).

You create your model in the `US` multi-region location. In a
BigQuery multi-region (`US`, `EU`) dataset, you can only create a
remote model that connects to an endpoint deployed in a region within the same
multi-region location (`US`, `EU`).

When you create the remote model, you need the endpoint ID that was generated
when you [deployed the model](https://docs.cloud.google.com/bigquery/docs/bigquery-ml-remote-model-tutorial#deploy-model) to Vertex AI. Also, the input and
output field names and types need to be exactly same as the Vertex AI
model's input and output. In this example, the input is a text `STRING`, and the
output is an `ARRAY` of type `FLOAT64`.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. For **Create new** , click **SQL query**.

3. In the query editor, enter this `CREATE MODEL` statement, and then click
   **Run**:

   ```googlesql
   CREATE OR REPLACE MODEL `PROJECT_ID.bqml_tutorial.bert_sentiment`
   INPUT (text STRING)
   OUTPUT(scores ARRAY<FLOAT64>)
   REMOTE WITH CONNECTION `PROJECT_ID.us.bqml_tutorial`
   OPTIONS(ENDPOINT = 'https://us-central1-aiplatform.googleapis.com/v1/projects/PROJECT_ID/locations/us-central1/endpoints/ENDPOINT_ID')
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project name.
   - <var translate="no">ENDPOINT_ID</var>: the endpoint ID that you copied previously.

   When the operation is complete, you see a message similar to
   `Successfully created model named bert_sentiment`.

   Your new model appears in the **Resources** panel. Models are
   indicated by the model icon: ![model
   icon](https://docs.cloud.google.com/static/bigquery/images/model-icon.png).

   If you select the new model in the **Resources** panel, information
   about the model appears below the **Query editor**.

### bq

1. Create the remote model by entering the following `CREATE MODEL`
   statement:

   ```
   bq query --use_legacy_sql=false \
   "CREATE OR REPLACE MODEL `PROJECT_ID.bqml_tutorial.bert_sentiment`
   INPUT (text STRING)
   OUTPUT(scores ARRAY<FLOAT64>)
   REMOTE WITH CONNECTION `PROJECT_ID.us.bqml_tutorial`
   OPTIONS(ENDPOINT = 'https://us-central1-aiplatform.googleapis.com/v1/projects/PROJECT_ID/locations/us-central1/endpoints/ENDPOINT_ID')"
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project name.
   - <var translate="no">ENDPOINT_ID</var>: the endpoint ID that you copied previously.
2. After you create the model, verify that the model appears in the
   dataset:

   ```
   bq ls -m bqml_tutorial
   ```

   The output is similar to the following:

   ```bash
   Id               Model Type   Labels    Creation Time
   --- --- --- ---
   bert_sentiment                         28 Jan 17:39:43
   ```

## Get predictions using `ML.PREDICT`

You use the `ML.PREDICT` function to get sentiment predictions from the remote
model. The input is a text column (`review`) that contains reviews of movies
from the `bigquery-public-data.imdb.reviews` table.

In this example, 10,000 records are selected and sent for prediction. The remote
model defaults to a batch size of 128 instances for requests.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Create new** section, click **SQL query**.

3. In the query editor, enter this query that uses the `ML.PREDICT`
   function, and then click **Run**.

   ```googlesql
   SELECT *
   FROM ML.PREDICT (
       MODEL `PROJECT_ID.bqml_tutorial.bert_sentiment`,
       (
           SELECT review as text
           FROM `bigquery-public-data.imdb.reviews`
           LIMIT 10000
       )
   )
   ```

   The query results should look similar to the following:

   ![Query results](https://docs.cloud.google.com/static/bigquery/images/bert-sentiment-predict.png)

### bq

Enter this command to run the query that uses `ML.PREDICT`.

```
bq query --use_legacy_sql=false \
'SELECT *
FROM ML.PREDICT (
MODEL `PROJECT_ID.bqml_tutorial.bert_sentiment`,
  (
    SELECT review as text
    FROM `bigquery-public-data.imdb.reviews`
    LIMIT 10000
  )
)'
```

## Clean up


To avoid incurring charges to your Google Cloud account for the resources used in this
tutorial, either delete the project that contains the resources, or keep the project and
delete the individual resources.

### Delete the project

### Console


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

### gcloud


> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. Delete a Google Cloud project:

```
gcloud projects delete PROJECT_ID
```

<br />

### Delete individual resources

Alternatively, to remove the individual resources used in this tutorial:

1. [Delete the model](https://docs.cloud.google.com/bigquery/docs/deleting-models).

2. Optional: [Delete the dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#delete-datasets).

3. [Undeploy the model and delete the endpoint](https://docs.cloud.google.com/vertex-ai/docs/general/deployment#undeploy_a_model_and_delete_the_endpoint).

4. [Delete the model from the Model Registry](https://docs.cloud.google.com/vertex-ai/docs/model-registry/delete-model).

5. [Delete the Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/working-with-connections#delete-connections).

## What's next

- For an overview of BigQuery ML, see [Introduction to
  AI and ML in BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- For more information about using the `CREATE MODEL`statement for remote models, see [The CREATE MODEL statement for
  remote models over custom models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https).
- For more information on using a BigQuery notebook, see [Introduction to notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction).
- For more information about BigQuery regions and multi-regions, see the [Supported locations](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations) page.
- To learn more about importing models in Vertex AI Model Registry, see [Import models to Vertex AI](https://docs.cloud.google.com/vertex-ai/docs/model-registry/import-model).
- To learn more about model versioning in Vertex AI Model Registry, see [Model versioning with Model Registry](https://docs.cloud.google.com/vertex-ai/docs/model-registry/versioning).
- For information on using Vertex AI VPC Service Controls, see [VPC Service Controls with Vertex AI](https://docs.cloud.google.com/vertex-ai/docs/general/vpc-service-controls).