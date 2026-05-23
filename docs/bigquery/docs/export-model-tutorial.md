> [!NOTE]
> **Note:** Exporting your models is not required for deployment on Vertex AI if you are using the Vertex AI Model Registry. To learn more about the registry, see [Manage BigQuery ML models in the Model Registry](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex).

This tutorial shows how to [export a BigQuery ML model](https://docs.cloud.google.com/bigquery/docs/exporting-models)
and then deploy the model either on Vertex AI or on a local machine. You will
use the [`iris` table](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=ml_datasets&t=iris&page=table)
from the BigQuery public datasets and work through the following three end-to-end scenarios:

- Train and deploy a logistic regression model - also applies to DNN classifier, DNN regressor, k-means, linear regression, and matrix factorization models.
- Train and deploy a boosted tree classifier model - also applies to boosted tree regressor model.
- Train and deploy an AutoML classifier model - also applies to AutoML regressor model.

## Costs

This tutorial uses billable components of Google Cloud,
including:

- BigQuery ML
- Cloud Storage
- Vertex AI (optional, used for online prediction)

For more information about BigQuery ML costs, see
[BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml).

For more information about Cloud Storage costs, see the
[Cloud Storage pricing](https://cloud.google.com/storage/pricing) page.

For more information about Vertex AI costs, see
[Custom-trained models](https://cloud.google.com/vertex-ai/pricing#custom-trained_models).

## Before you begin

1. BigQuery is automatically enabled in new projects. To activate BigQuery in a pre-existing project, go to


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)
2.


   Enable the AI Platform Training and Prediction API and Compute Engine APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=ml.googleapis.com,compute_component)
3. Install the [Google Cloud CLI](https://docs.cloud.google.com/sdk/install) and the [Google Cloud CLI](https://docs.cloud.google.com/sdk/downloads#interactive).

## Create your dataset

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

## Train and deploy a logistic regression model

Use the following sections to learn how to train and deploy a logistic
regression model.

### Train the model

Train a logistic regression model that predicts iris type using the
BigQuery ML [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#create_model_syntax)
statement. This training job should take approximately 1 minute to complete.

```googlesql
bq query --use_legacy_sql=false \
  'CREATE MODEL `bqml_tutorial.iris_model`
  OPTIONS (model_type="logistic_reg",
      max_iterations=10, input_label_cols=["species"])
  AS SELECT
    *
  FROM
    `bigquery-public-data.ml_datasets.iris`;'
```

### Export the model

Export the model to a Cloud Storage bucket using the [bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool). For additional ways to export models, see [Export BigQuery ML models](https://docs.cloud.google.com/bigquery/docs/exporting-models). This extract job should take less than 1 minute to complete.

    bq extract -m bqml_tutorial.iris_model gs://some/gcs/path/iris_model

### Local deployment and serving

You can deploy exported TensorFlow models using the TensorFlow Serving Docker
container. The following steps require you to install [Docker](https://hub.docker.com/search/?type=edition&offering=community).

#### Download the exported model files to a temporary directory

    mkdir tmp_dir
    gcloud storage cp gs://some/gcs/path/iris_model tmp_dir --recursive

#### Create a version subdirectory

This step sets a version number (1 in this case) for the model.

    mkdir -p serving_dir/iris_model/1
    cp -r tmp_dir/iris_model/* serving_dir/iris_model/1
    rm -r tmp_dir

#### Pull the Docker image

    docker pull tensorflow/serving

#### Run the Docker container

    docker run -p 8500:8500 --network="host" --mount type=bind,source=`pwd`/serving_dir/iris_model,target=/models/iris_model -e MODEL_NAME=iris_model -t tensorflow/serving &

#### Run the prediction

    curl -d '{"instances": [{"sepal_length":5.0, "sepal_width":2.0, "petal_length":3.5, "petal_width":1.0}]}' -X POST http://localhost:8501/v1/models/iris_model:predict

### Online deployment and serving

This section uses the [Google Cloud CLI](https://docs.cloud.google.com/sdk/gcloud) to deploy and
run predictions against the exported model.

For more information about deploying a model to Vertex AI for
online or batch predictions, see
[Deploy a model to an endpoint](https://docs.cloud.google.com/vertex-ai/docs/general/deployment).

#### Create a model resource

    MODEL_NAME="IRIS_MODEL"
    gcloud ai-platform models create $MODEL_NAME

#### Create a model version

1) Set the environment variables:

    MODEL_DIR="gs://some/gcs/path/iris_model"
    // Select a suitable version for this model
    VERSION_NAME="v1"
    FRAMEWORK="TENSORFLOW"

2) Create the version:

    gcloud ai-platform versions create $VERSION_NAME --model=$MODEL_NAME --origin=$MODEL_DIR --runtime-version=1.15 --framework=$FRAMEWORK

This step might take a few minutes to complete. You should see the message
`Creating version (this might take a few minutes)......`.

3) (optional) Get information about your new version:

    gcloud ai-platform versions describe $VERSION_NAME --model $MODEL_NAME

You should see output similar to this:

    createTime: '2020-02-28T16:30:45Z'
    deploymentUri: gs://your_bucket_name
    framework: TENSORFLOW
    machineType: mls1-c1-m2
    name: projects/[YOUR-PROJECT-ID]/models/IRIS_MODEL/versions/v1
    pythonVersion: '2.7'
    runtimeVersion: '1.15'
    state: READY

#### Online prediction

For more information about running online predictions against a
deployed model, see
[Get online inferences from a custom trained model](https://docs.cloud.google.com/vertex-ai/docs/predictions/get-online-predictions).

1) Create a newline-delimited JSON file for inputs, for example `instances.json`
file with the following content:

    {"sepal_length":5.0, "sepal_width":2.0, "petal_length":3.5, "petal_width":1.0}
    {"sepal_length":5.3, "sepal_width":3.7, "petal_length":1.5, "petal_width":0.2}

2) Setup env variables for predict:

    INPUT_DATA_FILE="instances.json"

3) Run predict:

    gcloud ai-platform predict --model $MODEL_NAME --version $VERSION_NAME --json-instances $INPUT_DATA_FILE

## Train and deploy a boosted tree classifier model

Use the following sections to learn how to train and deploy a boosted tree
classifier model.

### Train the model

Train a boosted tree classifier model that predicts iris type using the
[`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#create_model)
statement. This training job should take approximately 7 minutes to complete.

```googlesql
bq query --use_legacy_sql=false \
  'CREATE MODEL `bqml_tutorial.boosted_tree_iris_model`
  OPTIONS (model_type="boosted_tree_classifier",
      max_iterations=10, input_label_cols=["species"])
  AS SELECT
    *
  FROM
    `bigquery-public-data.ml_datasets.iris`;'
```

### Export the model

Export the model to a Cloud Storage bucket using the
[bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool). For additional ways
to export models, see [Export BigQuery ML models](https://docs.cloud.google.com/bigquery/docs/exporting-models).

    bq extract --destination_format ML_XGBOOST_BOOSTER -m bqml_tutorial.boosted_tree_iris_model gs://some/gcs/path/boosted_tree_iris_model

### Local deployment and serving

In the exported files, there is a `main.py` file for local run.

#### Download the exported model files to a local directory

    mkdir serving_dir
    gcloud storage cp gs://some/gcs/path/boosted_tree_iris_model serving_dir --recursive

#### Extract predictor

    tar -xvf serving_dir/boosted_tree_iris_model/xgboost_predictor-0.1.tar.gz -C serving_dir/boosted_tree_iris_model/

#### Install XGBoost library

Install the [XGBoost library](https://xgboost.readthedocs.io/en/latest/build.html) - version 0.82 or later.

#### Run the prediction

    cd serving_dir/boosted_tree_iris_model/
    python main.py '[{"sepal_length":5.0, "sepal_width":2.0, "petal_length":3.5, "petal_width":1.0}]'

### Online deployment and serving

This section uses the [Google Cloud CLI](https://docs.cloud.google.com/sdk/gcloud) to deploy and
run predictions against the exported model. For more information, see
[Get online inferences from a custom trained model](https://docs.cloud.google.com/vertex-ai/docs/predictions/get-online-predictions).

> [!NOTE]
> **Note:** For serving on [Vertex AI](https://docs.cloud.google.com/vertex-ai/docs) Prediction, follow [Request Predictions](https://docs.cloud.google.com/vertex-ai/docs/predictions/getting-predictions) and use the following containers for your region respectively: 1) us-docker.pkg.dev/vertex-ai/bigquery-ml/xgboost-cpu.1-0:latest 2) europe-docker.pkg.dev/vertex-ai/bigquery-ml/xgboost-cpu.1-0:latest 3) asia-docker.pkg.dev/vertex-ai/bigquery-ml/xgboost-cpu.1-0:latest

For more information about deploying a model to Vertex AI for
online or batch predictions using custom routines, see
[Deploy a model to an endpoint](https://docs.cloud.google.com/vertex-ai/docs/general/deployment).

#### Create a model resource

    MODEL_NAME="BOOSTED_TREE_IRIS_MODEL"
    gcloud ai-platform models create $MODEL_NAME

#### Create a model version

1) Set the environment variables:

    MODEL_DIR="gs://some/gcs/path/boosted_tree_iris_model"
    VERSION_NAME="v1"

2) Create the version:

    gcloud beta ai-platform versions create $VERSION_NAME --model=$MODEL_NAME --origin=$MODEL_DIR --package-uris=${MODEL_DIR}/xgboost_predictor-0.1.tar.gz --prediction-class=predictor.Predictor --runtime-version=1.15

This step might take a few minutes to complete. You should see the message
`Creating version (this might take a few minutes)......`.

3) (optional) Get information about your new version:

    gcloud ai-platform versions describe $VERSION_NAME --model $MODEL_NAME

You should see output similar to this:

    createTime: '2020-02-07T00:35:42Z'
    deploymentUri: gs://some/gcs/path/boosted_tree_iris_model
    etag: rp090ebEnQk=
    machineType: mls1-c1-m2
    name: projects/[YOUR-PROJECT-ID]/models/BOOSTED_TREE_IRIS_MODEL/versions/v1
    packageUris:
    - gs://some/gcs/path/boosted_tree_iris_model/xgboost_predictor-0.1.tar.gz
    predictionClass: predictor.Predictor
    pythonVersion: '2.7'
    runtimeVersion: '1.15'
    state: READY

#### Online prediction

For more information about running online predictions against a deployed
model, see
[Get online inferences from a custom trained model](https://docs.cloud.google.com/vertex-ai/docs/predictions/get-online-predictions).

1) Create a newline-delimited JSON file for inputs. For example, `instances.json`
file with the following content:

    {"sepal_length":5.0, "sepal_width":2.0, "petal_length":3.5, "petal_width":1.0}
    {"sepal_length":5.3, "sepal_width":3.7, "petal_length":1.5, "petal_width":0.2}

2) Set up environment variables for predict:

    INPUT_DATA_FILE="instances.json"

3) Run predict:

    gcloud ai-platform predict --model $MODEL_NAME --version $VERSION_NAME --json-instances $INPUT_DATA_FILE

## Train and deploy an AutoML classifier model

Use the following sections to learn how to train and deploy an AutoML
classifier model.

### Train the model

Train an AutoML classifier model that predicts iris type using the
[`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create)
statement. AutoML models need at least 1000 rows of input data. Because
`ml_datasets.iris` only has 150 rows, we duplicate the data 10 times. This
training job should take around **2 hours** to complete.

```googlesql
bq query --use_legacy_sql=false \
  'CREATE MODEL `bqml_tutorial.automl_iris_model`
  OPTIONS (model_type="automl_classifier",
      budget_hours=1, input_label_cols=["species"])
  AS SELECT
    * EXCEPT(multiplier)
  FROM
    `bigquery-public-data.ml_datasets.iris`, unnest(GENERATE_ARRAY(1, 10)) as multiplier;'
```

### Export the model

Export the model to a Cloud Storage bucket using the
[bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool). For additional ways
to export models, see [Exporting BigQuery ML models](https://docs.cloud.google.com/bigquery/docs/exporting-models).

    bq extract -m bqml_tutorial.automl_iris_model gs://some/gcs/path/automl_iris_model

### Local deployment and serving

For details about building AutoML containers, see
[Export AutoML tabular models](https://docs.cloud.google.com/vertex-ai/docs/export/export-model-tabular).
The following steps require you to install
[Docker](https://hub.docker.com/search/?type=edition&offering=community).

#### Copy exported model files to a local directory

    mkdir automl_serving_dir
    gcloud storage cp gs://some/gcs/path/automl_iris_model/* automl_serving_dir/ --recursive

#### Pull AutoML Docker image

    docker pull gcr.io/cloud-automl-tables-public/model_server

#### Start Docker container

    docker run -v `pwd`/automl_serving_dir:/models/default/0000001 -p 8080:8080 -it gcr.io/cloud-automl-tables-public/model_server

#### Run the prediction

1) Create a newline-delimited JSON file for inputs. For example, `input.json`
file with the following contents:

    {"instances": [{"sepal_length":5.0, "sepal_width":2.0, "petal_length":3.5, "petal_width":1.0},
    {"sepal_length":5.3, "sepal_width":3.7, "petal_length":1.5, "petal_width":0.2}]}

2) Make the predict call:

    curl -X POST --data @input.json http://localhost:8080/predict

### Online deployment and serving

Online prediction for AutoML regressor and AutoML classifier models is not supported in Vertex AI.

## Clean up


To avoid incurring charges to your Google Cloud account for the resources used in this
tutorial, either delete the project that contains the resources, or keep the project and
delete the individual resources.

- You can delete the project you created.
- Or you can keep the project and delete the dataset and Cloud Storage bucket.

### Stop Docker container

1) List all running Docker containers.

    docker ps

2) Stop the container with the applicable container ID from the container list.

    docker stop container_id

### Delete Vertex AI resources

1) Delete the model version.

    gcloud ai-platform versions delete $VERSION_NAME --model=$MODEL_NAME

2) Delete the model.

    gcloud ai-platform models delete $MODEL_NAME

### Delete your dataset

Deleting your project removes all datasets and all tables in the project. If you
prefer to reuse the project, you can delete the dataset you created in this
tutorial:

1. If necessary, open the BigQuery page in the
   Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the navigation, click the **bqml_tutorial** dataset you created.

3. Click **Delete dataset** on the right side of the window.
   This action deletes the dataset, the table, and all the data.

4. In the **Delete dataset** dialog, confirm the delete command by typing
   the name of your dataset (`bqml_tutorial`) and then click **Delete**.

### Delete your Cloud Storage bucket

Deleting your project removes all Cloud Storage buckets in the project. If
you prefer to reuse the project, you can delete the bucket you created in this
tutorial

1. In the Google Cloud console, go to the Cloud Storage **Buckets** page.  

   [Go to Buckets](https://console.cloud.google.com/storage/browser)
2. Select the checkbox of the bucket you want to delete.

3. Click **Delete**.

4. In the overlay window that appears, confirm you want to delete the
   bucket and its contents by clicking **Delete**.

### Delete your project

To delete the project:

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

- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- For information on exporting models, see [Export models](https://docs.cloud.google.com/bigquery/docs/exporting-models).
- For information on creating models, see the [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create) syntax page.