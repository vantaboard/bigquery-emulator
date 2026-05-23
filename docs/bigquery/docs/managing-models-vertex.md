# Manage BigQuery ML models in Vertex AI

You can register BigQuery ML models with
the Vertex AI Model Registry, in order to manage them alongside your
Vertex AI models without needing to export them. When you register
models with Model Registry, you can version, evaluate,
and deploy the models for online prediction by using a single interface, and
without needing a serving container. If you aren't familiar with
Vertex AI and how it integrates with BigQuery ML,
see [Vertex AI for BigQuery users](https://docs.cloud.google.com/vertex-ai/docs/beginner/bqml).

To learn more about Vertex AI prediction, see
[Overview of getting predictions on Vertex AI](https://docs.cloud.google.com/vertex-ai/docs/predictions/overview).

> [!NOTE]
> **Note:** Although all model types can be registered, there are limitations on deployment for certain model types. For more information, see [Model deployment](https://docs.cloud.google.com/bigquery/docs/exporting-models#model-deployment).

To learn how to manage your BigQuery ML models from Vertex AI Model Registry,
see [Introduction to Vertex AI Model Registry](https://docs.cloud.google.com/vertex-ai/docs/model-registry/introduction).

## Before you begin


Enable the Vertex AI API.


**Roles required to enable APIs**


To enable APIs, you need the Service Usage Admin IAM
role (`roles/serviceusage.serviceUsageAdmin`), which
contains the `serviceusage.services.enable` permission. [Learn how to grant
roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

[Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=aiplatform.googleapis.com)

## Required permissions


To get the permissions that
you need to register BigQuery ML models to the
Model Registry,

ask your administrator to grant you the
[Vertex AI Administrator](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.admin) (`roles/aiplatform.admin`) IAM role on your project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Register models

When you create a BigQuery ML model, you can register the model
to the Model Registry in the following ways:

- In the Google Cloud console, select the model in the **Explorer** pane and then click **Register** on the **Registry** tab. ([Preview](https://cloud.google.com/products#product-launch-stages))
- Use the
  [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create).
  In the `CREATE MODEL` statement, you can use the following options to
  register the model to the Model Registry:

  - `MODEL_REGISTRY`: register the model to the Model Registry.
  - `VERTEX_AI_MODEL_ID`: specify a model ID to use for the model in the Model Registry. The model ID is associated with your BigQuery ML model, and is visible from the Model Registry. Each BigQuery ML model can only be registered to one model ID in the Model Registry.
  - `VERTEX_AI_MODEL_VERSION_ALIASES`: specify one or more model version aliases, which you can use to streamline deployment, manage models, and enable [Vertex Explainable AI](https://docs.cloud.google.com/vertex-ai/docs/explainable-ai/overview) on models.

  If you set the `MODEL_REGISTRY` option when creating a model, the model is
  registered to the Model Registry, and automatically
  displays there once it has completed training in BigQuery ML.
  You can use the **Source** column in the **Model Registry** page of the
  Google Cloud console to see where a model is sourced from.

Once a BigQuery ML model is registered, you can use the following
Model Registry capabilities with your model:

- [Deploy the model to an endpoint](https://docs.cloud.google.com/vertex-ai/docs/predictions/deploy-model-console)
- [Compare model versions](https://docs.cloud.google.com/vertex-ai/docs/model-registry/versioning)
- [Get predictions](https://docs.cloud.google.com/vertex-ai/docs/predictions/get-predictions#get_predictions_from_custom_trained_models)
- [Monitor the model](https://docs.cloud.google.com/vertex-ai/docs/model-monitoring/overview)
- [View model evaluations](https://docs.cloud.google.com/vertex-ai/docs/evaluation/introduction)
- [Get feature-based explanations for the model](https://docs.cloud.google.com/vertex-ai/docs/explainable-ai/overview#feature-based)

All models created using BigQuery ML still display in the
BigQuery user interface, regardless of whether they are
registered to the Model Registry.

The following example shows how to create and register a k-means model:

```googlesql
CREATE OR REPLACE MODEL `mydataset.my_kmeans_model`
  MODEL_TYPE = 'KMEANS',
  MODEL_REGISTRY = 'VERTEX_AI',
  VERTEX_AI_MODEL_ID = 'customer_clustering';
```

### Register an existing BigQuery ML model to the Model Registry

If you don't register a model to Vertex AI when you create it,
you can use SQL, the bq command-line tool, or the BigQuery API to register it
afterwards.

The following examples show how to register an existing model:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Datasets** and then click a dataset
   that contains your model.

4. Click the **Models** tab, and then click the model that you want to register.

5. On the model details pane, select the **Registry** tab.

6. Click **Register**.

7. On the **Register model to Vertex model registry** pane, do one of the
   following:

   - Select **Register as a new model** . For **Model name**, type a model
     name.

   - Select **Register as a new version of an existing model**.

     1. For **Model name**, type a model name.
     2. Optional. If you want to use a version alias, select **Version alias** and then type a version alias name.
8. Click **Register**.

### SQL

Use the
[`ALTER MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-alter-model):

```googlesql
ALTER MODEL IF EXISTS mymodel SET OPTIONS (vertex_ai_model_id='my_vertex_ai_model_id');
```

### bq

Use the
[`bq update` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update)
with the `--model` flag:

```bash
  bq update --model --vertex_ai_model_id 'my_vertex_ai_model_id' myproject:mydataset.mymodel
```

### API

Use the [`models.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch).
Pass in an [`Model` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#Model)
that contains a
[`trainingRuns` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#TrainingRun)
with a populated `vertexAiModelId` field:

```bash
{
  "trainingRuns": [
    {
      "vertexAiModelId": my_vertex_ai_model_id
    }
}
```

### Register multiple versions of BigQuery ML models

The first BigQuery ML model that you register under a given model
ID displays as version 1 of that model in the
Model Registry. You can register additional
BigQuery ML models as different versions of that registered model
by specifying the same Vertex AI model ID when you create or
alter those BigQuery ML models.

For example, you could create `model1` in BigQuery ML and register
it in Model Registry as `regression_model`. `model1`
displays as version 1 of `regression_model` in
Model Registry. If you then create `model2` in
BigQuery ML and register it in
Model Registry as `regression_model`, `model2`
displays as version 2 of `regression_model` in
Model Registry.

If you create or replace a BigQuery ML model and use a
BigQuery ML model name that is already associated with a model
in the Model Registry, the existing
Model Registry model version is deleted and replaced with
the new model. Building on the prior example, if you create or replace
`model2` in BigQuery ML by using the
`CREATE OR REPLACE MODEL` statement with the `MODEL_REGISTRY` and
`VERTEX_AI_MODEL_ID` options, version 2 of `regression_model` in the
Model Registry is replaced, and
Model Registry displays version 1 and version 3 of
`regression_model` model.

### Change the model ID of a registered BigQuery ML model

Once a BigQuery ML model is registered to the
Model Registry, you can't change the `VERTEX_AI_MODEL_ID`
value. To register the model with a new `VERTEX_AI_MODEL_ID`, use one of the
following options:

- [Delete the model](https://docs.cloud.google.com/bigquery/docs/deleting-models#delete_a_model)
  and recreate it, specifying a new value for the `VERTEX_AI_MODEL_ID` option.
  This approach incurs re-training costs.

- [Copy the model](https://docs.cloud.google.com/bigquery/docs/managing-models#copy_a_model),
  and then use the `ALTER MODEL` statement to register the
  new model with a new `VERTEX_AI_MODEL_ID` value.

### Location considerations

If you register a multi-region BigQuery ML model to
Model Registry, the model becomes a regional model
in Vertex AI. A BigQuery ML US multi-region model
is synced to Vertex AI (us-central1) and a
BigQuery ML multi-region EU model is synced to
Vertex AI (europe-west4). For single region models, there are no
changes.

For information about how to update model locations, see
[Choosing your location](https://docs.cloud.google.com/vertex-ai/docs/general/locations#choosing_your_location).

## Deploy a model in Vertex AI

You can use a variety of methods to deploy a model to an endpoint in
Vertex AI. For more information, see
[Deploy a model to an endpoint](https://docs.cloud.google.com/vertex-ai/docs/general/deployment).

## Delete BigQuery ML models from the Model Registry

To delete a BigQuery ML model from the
Model Registry, delete the model in
BigQuery ML. The model is automatically removed from the
Model Registry.

There are multiple ways you can delete a BigQuery ML model. For
more information, see [Delete models](https://docs.cloud.google.com/bigquery/docs/deleting-models).

If you want to delete a model in BigQuery ML that has been
registered in the Model Registry and deployed to an
endpoint, you must first use Model Registry to undeploy
the model. You can then return to BigQuery ML and delete the
model. For more information on how to undeploy a model, see
[Delete an endpoint](https://docs.cloud.google.com/vertex-ai/docs/samples/aiplatform-delete-endpoint-sample).

## Limitations

- You can't register
  [remote models](https://docs.cloud.google.com/bigquery/docs/bqml-introduction#remote_models).

- The following models can be registered in
  Model Registry, but they can't be deployed in
  Vertex AI:

  - [Imported XGBoost models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-xgboost)
  - [`ARIMA_PLUS` models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
  - [`ARIMA_PLUS_XREG` models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series)