# ML pipelines overview

This document provides an overview of the services you can use to build an ML
pipeline to manage your BigQuery ML
[MLOps](https://docs.cloud.google.com/architecture/mlops-continuous-delivery-and-automation-pipelines-in-machine-learning)
workflow.

An ML pipeline is a representation of an MLOps workflow that is composed of a
series of *pipeline tasks*. Each pipeline task performs a specific step in the
MLOps workflow to train and deploy a model. Separating each step into a
standardized, reusable task lets you automate and monitor repeatable processes
in your ML practice.

You can use any of the following services to create BigQuery ML
ML pipelines:

- Use Vertex AI Pipelines to create portable, extensible ML pipelines.
- Use GoogleSQL queries to create less complex SQL-based ML pipelines.
- Use Dataform to create more complex SQL-based ML pipelines, or ML pipelines where you need to use version control.

## Vertex AI Pipelines

In [Vertex AI Pipelines](https://docs.cloud.google.com/vertex-ai/docs/pipelines/introduction),
an ML pipeline is structured as a directed acyclic graph (DAG) of containerized
pipeline tasks that are interconnected using input-output dependencies.
Each [pipeline task](https://docs.cloud.google.com/vertex-ai/docs/pipelines/introduction#pipeline-task)
is an instantiation of a
[pipeline component](https://docs.cloud.google.com/vertex-ai/docs/pipelines/introduction#pipeline-component)
with specific inputs. When defining your ML pipeline, you connect multiple
pipeline tasks to form a DAG by routing the outputs of one pipeline task to the
inputs for the next pipeline task in the ML workflow. You can also use the
original inputs to the ML pipeline as the inputs for a given pipeline task.

Use the
[BigQuery ML components](https://docs.cloud.google.com/vertex-ai/docs/pipelines/bigqueryml-component)
of the Google Cloud Pipeline Components SDK to compose ML pipelines
in Vertex AI Pipelines. To get started with
BigQuery ML components, see the following notebooks:

- [Get started with BigQuery ML pipeline components](https://github.com/GoogleCloudPlatform/vertex-ai-samples/blob/main/notebooks/community/ml_ops/stage3/get_started_with_bqml_pipeline_components.ipynb)
- [Train and evaluate a demand forecasting model](https://github.com/GoogleCloudPlatform/vertex-ai-samples/blob/main/notebooks/community/pipelines/google_cloud_pipeline_components_bqml_pipeline_demand_forecasting.ipynb)

## GoogleSQL queries

You can use
[GoogleSQL procedural language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language)
to execute multiple statements in a
[multi-statement query](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries). You can use a
multi-statement query to:

- Run multiple statements in a sequence, with shared state.
- Automate management tasks such as creating or dropping tables.
- Implement complex logic using programming constructs such as `IF` and `WHILE`.

After creating a multi-statement query, you can
[save](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction) and
[schedule](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) the query to automate model
training, inference, and monitoring.

If your ML pipeline includes use of the
[`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text),
see
[Handle quota errors by calling `ML.GENERATE_TEXT` iteratively](https://docs.cloud.google.com/bigquery/docs/iterate-generate-text-calls) for more information on how to use SQL to
iterate through calls to the function. Calling the function
iteratively lets you address any retryable errors that occur due to exceeding
the [quotas and limits](https://docs.cloud.google.com/bigquery/quotas#cloud_ai_service_functions).

## Dataform

You can use [Dataform](https://docs.cloud.google.com/dataform/docs/overview) to develop,
test, version control, and schedule complex SQL workflows for data
transformation in BigQuery. You can use Dataform for
such tasks as data transformation in the Extraction, Loading, and
Transformation (ELT) process for data integration. After raw data is extracted
from source systems and loaded into BigQuery,
Dataform helps you to transform it into a well-defined, tested,
and documented suite of data tables.

If your ML pipeline includes use of the
[`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text),
you can adapt the
[`structured_table_ml.js` example library](https://github.com/dataform-co/dataform-bqml/blob/main/modules/structured_table_ml.js)
to iterate through calls to the function. Calling the function
iteratively lets you address any retryable errors that occur due to exceeding
the quotas and limits that apply to the function.