# Embed text with pretrained TensorFlow models

This tutorial shows you how to
generate NNLM, SWIVEL, and BERT text embeddings in BigQuery by using
pretrained TensorFlow models.
A text embedding is a dense vector representation of a piece of text such that
if two pieces of text are semantically similar, then their respective embeddings
are close together in the embedding vector space.

## The NNLM, SWIVEL, and BERT models

The NNLM, SWIVEL, and BERT models vary in size, accuracy, scalability, and cost.
Use the following table to help you determine which model to use:

| Model | Model size | Embedding dimension | Use case | Description |
|---|---|---|---|---|
| [NNLM](https://tfhub.dev/google/nnlm-en-dim50-with-normalization/2) | \<150MB | 50 | Short phrases, news, tweets, reviews | Neural Network Language Model |
| [SWIVEL](https://tfhub.dev/google/tf2-preview/gnews-swivel-20dim/1) | \<150MB | 20 | Short phrases, news, tweets, reviews | Submatrix-wise Vector Embedding Learner |
| [BERT](https://tfhub.dev/tensorflow/bert_en_cased_L-12_H-768_A-12/4) | \~200MB | 768 | Short phrases, news, tweets, reviews, short paragraphs | Bidirectional Encoder Representations from Transformers |

In this tutorial, the NNLM and SWIVEL models are
[imported TensorFlow models](https://docs.cloud.google.com/bigquery/docs/making-predictions-with-imported-tensorflow-models),
and the BERT model is a
[remote model on Vertex AI](https://docs.cloud.google.com/bigquery/docs/bigquery-ml-remote-model-tutorial).

## Required permissions

- To create the dataset, you need the `bigquery.datasets.create`
  Identity and Access Management (IAM) permission.

- To create the bucket, you need the `storage.buckets.create` IAM
  permission.

- To upload the model to Cloud Storage, you need the
  `storage.objects.create` and `storage.objects.get` IAM
  permissions.

- To create the connection resource, you need the following IAM
  permissions:

  - `bigquery.connections.create`
  - `bigquery.connections.get`
- To load the model into BigQuery ML, you need the following
  IAM permissions:

  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
- To run inference, you need the following IAM permissions:

  - `bigquery.tables.getData` on the object table
  - `bigquery.models.getData` on the model
  - `bigquery.jobs.create`

## Costs


In this document, you use the following billable components of Google Cloud:


- **BigQuery:** You incur costs for the queries that you run in BigQuery.
- **BigQuery ML:** You incur costs for the model that you create and the inference that you perform in BigQuery ML.
- **Cloud Storage:** You incur costs for the objects that you store in Cloud Storage.
- **Vertex AI:** If you follow the instructions for generating the BERT model, then you incur costs for deploying the model to an endpoint.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

For more information, see the following resources:

- [Storage pricing](https://cloud.google.com/bigquery/pricing#storage)
- [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml)
- [Cloud Storage pricing](https://cloud.google.com/storage/pricing)
- [Vertex AI pricing](https://cloud.google.com/vertex-ai/pricing)

## Before you begin

<br />

> [!NOTE]
> **Note:** The Vertex AI API and BigQuery Connection API are only required for the BERT model.

## Create a dataset

To create a dataset named `tf_models_tutorial` to store the models that
you create, select one of the following options:

### SQL

Use the
[`CREATE SCHEMA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_schema_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE SCHEMA `PROJECT_ID.tf_models_tutorial`;
   ```


   Replace `PROJECT_ID` with your project ID.
3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)
2. To create the dataset, run the
   [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset):

   ```bash
   bq mk --dataset --location=us PROJECT_ID:tf_models_tutorial
   ```

   Replace `PROJECT_ID` with your project ID.

## Generate and upload a model to Cloud Storage

For more detailed instructions on generating text embeddings using pretrained
TensorFlow models, see the
[Colab notebook](https://github.com/GoogleCloudPlatform/bigquery-ml-utils/blob/master/notebooks/bqml-generate-text-embedding-model.ipynb).
Otherwise, select one of the following models:

### NNLM

1. Install the
   [`bigquery-ml-utils` library](https://github.com/GoogleCloudPlatform/bigquery-ml-utils#installation)
   using pip:

       pip install bigquery-ml-utils

2. Generate an NNLM model. The following Python code loads an NNLM model
   from TensorFlow Hub and prepares it for
   BigQuery:

       from bigquery_ml_utils import model_generator
       import tensorflow_text

       # Establish an instance of TextEmbeddingModelGenerator.
       text_embedding_model_generator = model_generator.TextEmbeddingModelGenerator()

       # Generate an NNLM model.
       text_embedding_model_generator.generate_text_embedding_model('nnlm', OUTPUT_MODEL_PATH)

   Replace `OUTPUT_MODEL_PATH` with a path to a local
   folder where you can temporarily store the model.
3. Optional: Print the generated model's signature:

       import tensorflow as tf

       reload_embedding_model = tf.saved_model.load(OUTPUT_MODEL_PATH)
       print(reload_embedding_model.signatures["serving_default"])

4. To copy the generated model from your local folder to a
   Cloud Storage bucket, use the
   [Google Cloud CLI](https://docs.cloud.google.com/sdk/gcloud/reference/storage):

       gcloud storage cp OUTPUT_MODEL_PATH gs://BUCKET_PATH/nnlm_model --recursive

   Replace `BUCKET_PATH` with the name of the
   Cloud Storage bucket to which you are copying the model.

### SWIVEL

1. Install the
   [`bigquery-ml-utils` library](https://github.com/GoogleCloudPlatform/bigquery-ml-utils#installation)
   using pip:

       pip install bigquery-ml-utils

2. Generate a SWIVEL model. The following Python code loads a SWIVEL model
   from TensorFlow Hub and prepares it for
   BigQuery:

       from bigquery_ml_utils import model_generator
       import tensorflow_text

       # Establish an instance of TextEmbeddingModelGenerator.
       text_embedding_model_generator = model_generator.TextEmbeddingModelGenerator()

       # Generate a SWIVEL model.
       text_embedding_model_generator.generate_text_embedding_model('swivel', OUTPUT_MODEL_PATH)

   Replace `OUTPUT_MODEL_PATH` with a path to a local
   folder where you can temporarily store the model.
3. Optional: Print the generated model's signature:

       import tensorflow as tf

       reload_embedding_model = tf.saved_model.load(OUTPUT_MODEL_PATH)
       print(reload_embedding_model.signatures["serving_default"])

4. To copy the generated model from your local folder to a
   Cloud Storage bucket, use the
   [Google Cloud CLI](https://docs.cloud.google.com/sdk/gcloud/reference/storage):

       gcloud storage cp OUTPUT_MODEL_PATH gs://BUCKET_PATH/swivel_model --recursive

   Replace `BUCKET_PATH` with the name of the
   Cloud Storage bucket to which you are copying the model.

### BERT

1. Install the
   [`bigquery-ml-utils` library](https://github.com/GoogleCloudPlatform/bigquery-ml-utils#installation)
   using pip:

       pip install bigquery-ml-utils

2. Generate a BERT model. The following Python code loads a BERT model
   from TensorFlow Hub and prepares it for
   BigQuery:

       from bigquery_ml_utils import model_generator
       import tensorflow_text

       # Establish an instance of TextEmbeddingModelGenerator.
       text_embedding_model_generator = model_generator.TextEmbeddingModelGenerator()

       # Generate a BERT model.
       text_embedding_model_generator.generate_text_embedding_model('bert', OUTPUT_MODEL_PATH)

   Replace `OUTPUT_MODEL_PATH` with a path to a local
   folder where you can temporarily store the model.
3. Optional: Print the generated model's signature:

       import tensorflow as tf

       reload_embedding_model = tf.saved_model.load(OUTPUT_MODEL_PATH)
       print(reload_embedding_model.signatures["serving_default"])

4. To copy the generated model from your local folder to a
   Cloud Storage bucket, use the
   [Google Cloud CLI](https://docs.cloud.google.com/sdk/gcloud/reference/storage):

       gcloud storage cp OUTPUT_MODEL_PATH gs://BUCKET_PATH/bert_model --recursive

   Replace `BUCKET_PATH` with the name of the
   Cloud Storage bucket to which you are copying the model.

## Load the model into BigQuery

Select one of the following models:

### NNLM

Use the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       CREATE OR REPLACE MODEL tf_models_tutorial.nnlm_model
       OPTIONS (
         model_type = 'TENSORFLOW',
         model_path = 'gs://BUCKET_NAME/nnlm_model/*');


   Replace `BUCKET_NAME` with the name of the bucket
   that you previously created.
3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### SWIVEL

Use the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       CREATE OR REPLACE MODEL tf_models_tutorial.swivel_model
       OPTIONS (
         model_type = 'TENSORFLOW',
         model_path = 'gs://BUCKET_NAME/swivel_model/*');


   Replace `BUCKET_NAME` with the name of the bucket
   that you previously created.
3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### BERT

To load the BERT model into BigQuery, import the
BERT model to Vertex AI, deploy the model to a
Vertex AI endpoint, create a connection, and then
create a remote model in BigQuery.

To import the BERT model to Vertex AI, follow these steps:

1. In the Google Cloud console, go to the Vertex AI
   **Model registry** page.

   [Go to Model registry](https://console.cloud.google.com/vertex-ai/models)
2. Click **Import**, and then do the following:

   - For **Name** , enter `BERT`.
   - For **Region**, select a region that matches your Cloud Storage bucket's region.
3. Click **Continue**, and then do the following:

   - For **Model framework version** , select `2.8`.
   - For **Model artifact location** , enter the path to the Cloud Storage bucket where you stored the model file. For example, `gs://BUCKET_PATH/bert_model`.
4. Click **Import** . After the import is complete, your model appears
   on the **Model registry** page.

To deploy the BERT model to a Vertex AI endpoint and connect it to
BigQuery, follow these steps:

1. In the Google Cloud console, go to the Vertex AI
   **Model registry** page.

   [Go to Model registry](https://console.cloud.google.com/vertex-ai/models)
2. Click on the name of your model.

3. Click **Deploy \& test**.

4. Click **Deploy to endpoint**.

5. For **Endpoint name** , enter `bert_model_endpoint`.

6. Click **Continue**.

7. Select your compute resources.

8. Click **Deploy**.

9. [Create a BigQuery Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection)
   and [grant access](https://docs.cloud.google.com/bigquery/docs/bigquery-ml-remote-model-tutorial#set_up_connection_access)
   to the connection's service account.

To create a remote model based on the Vertex AI endpoint,
use the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       CREATE OR REPLACE MODEL tf_models_tutorial.bert_model
       INPUT(content STRING)
       OUTPUT(embedding ARRAY<FLOAT64>)
       REMOTE WITH CONNECTION `PROJECT_ID.CONNECTION_LOCATION.CONNECTION_ID`
       OPTIONS (
         ENDPOINT = "https://ENDPOINT_LOCATION-aiplatform.googleapis.com/v1/projects/PROJECT_ID/locations/ENDPOINT_LOCATION/endpoints/ENDPOINT_ID");


   Replace the following:
   - `PROJECT_ID`: the project ID
   - `CONNECTION_LOCATION`: the location of your BigQuery connection
   - `CONNECTION_ID`: the ID of your BigQuery connection

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, this is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`
   - `ENDPOINT_LOCATION`: the location of your Vertex AI endpoint. For example: "us-central1".
   - `ENDPOINT_ID`: the ID of your model endpoint

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## Generate text embeddings

In this section, you use the
[`ML.PREDICT()` inference function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
to generate text embeddings of the `review` column from the public dataset
`bigquery-public-data.imdb.reviews`. The
query limits the table to 500 rows to reduce the amount of data processed.

### NNLM

```googlesql
SELECT
  *
FROM
  ML.PREDICT(
    MODEL `tf_models_tutorial.nnlm_model`,
    (
    SELECT
      review AS content
    FROM
      `bigquery-public-data.imdb.reviews`
    LIMIT
      500)
  );
```

The result is similar to the following:

```
+---+---+
| embedding             | content                                |
+---+---+
|  0.08599445223808289  | Isabelle Huppert must be one of the... |
| -0.04862852394580841  |                                        |
| -0.017750458791851997 |                                        |
|  0.8658871650695801   |                                        |
| ...                   |                                        |
+---+---+
```

<br />

### SWIVEL

```googlesql
SELECT
  *
FROM
  ML.PREDICT(
    MODEL `tf_models_tutorial.swivel_model`,
    (
    SELECT
      review AS content
    FROM
      `bigquery-public-data.imdb.reviews`
    LIMIT
      500)
  );
```

The result is similar to the following:

```
+---+---+
| embedding            | content                                |
+---+---+
|  2.5952553749084473  | Isabelle Huppert must be one of the... |
| -4.015787601470947   |                                        |
|  3.6275434494018555  |                                        |
| -6.045154333114624   |                                        |
| ...                  |                                        |
+---+---+
```

<br />

### BERT

```googlesql
SELECT
  *
FROM
  ML.PREDICT(
    MODEL `tf_models_tutorial.bert_model`,
    (
    SELECT
      review AS content
    FROM
      `bigquery-public-data.imdb.reviews`
    LIMIT
      500)
  );
```

The result is similar to the following:

```
+---+---+---+
| embedding    | remote_model_status | content                                |
+---+---+---+
| -0.694072425 | null                | Isabelle Huppert must be one of the... |
|  0.439208865 |                     |                                        |
|  0.99988997  |                     |                                        |
| -0.993487895 |                     |                                        |
| ...          |                     |                                        |
+---+---+---+
```

<br />

## Clean up

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