# Use tuning and evaluation to improve model performance

This document shows you how to create a BigQuery ML
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
that references a
[Vertex AI `gemini-2.0-flash-001` model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models).
You then use
[supervised tuning](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-tuned#supervised_tuning)
to tune the model with new training data, followed by evaluating the model
with the
[`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).

Tuning can help you address scenarios where you need to customize the hosted
Vertex AI model, such as when the expected behavior of the model
is hard to concisely define in a prompt, or when prompts don't produce expected
results consistently enough. Supervised tuning also influences the model in the
following ways:

- Guides the model to return specific response styles---for example being more concise or more verbose.
- Teaches the model new behaviors---for example responding to prompts as a specific persona.
- Causes the model to update itself with new information.

In this tutorial, the goal is to have the model generate text whose style and
content conforms as closely as possible to provided ground truth content.

## Required roles

To run this tutorial, you need the following Identity and Access Management (IAM)
roles:

- Create and use BigQuery datasets, connections, and models: BigQuery Admin (`roles/bigquery.admin`).
- Grant permissions to the connection's service account: Project IAM Admin (`roles/resourcemanager.projectIamAdmin`).

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- Create a dataset: `bigquery.datasets.create`
- Create a table: `bigquery.tables.create`
- Create, delegate, and use a connection: `bigquery.connections.*`
- Set the default connection: `bigquery.config.*`
- Set service account permissions: `resourcemanager.projects.getIamPolicy` and `resourcemanager.projects.setIamPolicy`
- Create a model and run inference:
  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
  - `bigquery.models.updateMetadata`

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Before you begin

1. In the Google Cloud console, on the project selector page,
   select or create a Google Cloud project.

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
2.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

3.


   Enable the BigQuery, BigQuery Connection, Vertex AI, and Compute Engine APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,bigqueryconnection.googleapis.com,aiplatform.googleapis.com,compute.googleapis.com)

## Costs


In this document, you use the following billable components of Google Cloud:


- **BigQuery:** You incur costs for the queries that you run in BigQuery.
- **BigQuery ML:** You incur costs for the model that you create and the processing that you perform in BigQuery ML.
- **Vertex AI:** You incur costs for calls to and supervised tuning of the `gemini-2.0-flash-001` model.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

For more information, see the following resources:

- [BigQuery storage pricing](https://cloud.google.com/bigquery/pricing#storage)
- [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml)
- [Vertex AI pricing](https://cloud.google.com/vertex-ai/pricing)

# Create a dataset

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

## Create test tables

Create tables of training and evaluation data based on the public
[task955_wiki_auto_style_transfer](https://huggingface.co/datasets/Lots-of-LoRAs/task955_wiki_auto_style_transfer) dataset from Hugging Face.

1. Open the [Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true).

2. In the Cloud Shell, run the following commands to create tables of
   test and evaluation data:

       python3 -m pip install pandas pyarrow fsspec huggingface_hub

       python3 -c "import pandas as pd; df_train = pd.read_parquet('hf://datasets/Lots-of-LoRAs/task955_wiki_auto_style_transfer/data/train-00000-of-00001.parquet').drop('id', axis=1); df_train['output'] = [x[0] for x in df_train['output']]; df_train.to_json('wiki_auto_style_transfer_train.jsonl', orient='records', lines=True);"

       python3 -c "import pandas as pd; df_valid = pd.read_parquet('hf://datasets/Lots-of-LoRAs/task955_wiki_auto_style_transfer/data/valid-00000-of-00001.parquet').drop('id', axis=1); df_valid['output'] = [x[0] for x in df_valid['output']]; df_valid.to_json('wiki_auto_style_transfer_valid.jsonl', orient='records', lines=True);"

       bq rm -t bqml_tutorial.wiki_auto_style_transfer_train

       bq rm -t bqml_tutorial.wiki_auto_style_transfer_valid

       bq load --source_format=NEWLINE_DELIMITED_JSON bqml_tutorial.wiki_auto_style_transfer_train wiki_auto_style_transfer_train.jsonl input:STRING,output:STRING

       bq load --source_format=NEWLINE_DELIMITED_JSON bqml_tutorial.wiki_auto_style_transfer_valid wiki_auto_style_transfer_valid.jsonl input:STRING,output:STRING

## Create a baseline model

Create a
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
over the Vertex AI `gemini-2.0-flash-001` model.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement to create a remote model:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.gemini_baseline`
   REMOTE WITH CONNECTION DEFAULT
   OPTIONS (ENDPOINT ='gemini-2.0-flash-001');
   ```

   The query takes several seconds to complete, after which the
   `gemini_baseline` model appears in the `bqml_tutorial` dataset in the
   **Explorer** pane. Because the query uses a `CREATE MODEL` statement to
   create a model, there are no query results.

## Check baseline model performance

Run the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
with the remote model to see how it performs on the evaluation data without any
tuning.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   SELECT result, ground_truth
   FROM
     AI.GENERATE_TEXT(
       MODEL `bqml_tutorial.gemini_baseline`,
       (
         SELECT
           input AS prompt, output AS ground_truth
         FROM `bqml_tutorial.wiki_auto_style_transfer_valid`
         LIMIT 10
       ));
   ```

   If you examine the output data and compare the `result`
   and `ground_truth` values, you see that while the baseline model generates
   text that accurately reflects the facts provided in the ground
   truth content, the style of the text is fairly different.

## Evaluate the baseline model

To perform a more detailed evaluation of the model performance, use the
[`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
This function computes model metrics that measure the accuracy and quality of
the generated text, in order to see how the model's responses compare to ideal
responses.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   SELECT *
   FROM
     ML.EVALUATE(
       MODEL `bqml_tutorial.gemini_baseline`,
       (
         SELECT
           input AS input_text, output AS output_text
         FROM `bqml_tutorial.wiki_auto_style_transfer_valid`
       ),
       STRUCT('text_generation' AS task_type));
   ```

The output looks similar to the following:

<br />

```
   +---+---+---+---+
   | bleu4_score         | rouge-l_precision   | rouge-l_recall      | rouge-l_f1_score    | evaluation_status                          |
   +---+---+---+---+---+
   | 0.23317359667074181 | 0.37809145226740043 | 0.45902937167791508 | 0.40956844061733139 | {                                          |
   |                     |                     |                     |                     |  "num_successful_rows": 176,               |
   |                     |                     |                     |                     |  "num_total_rows": 176                     |
   |                     |                     |                     |                     | }                                          |
   +---+---+ ---+---+---+
   
```

<br />

You can see that the baseline model performance isn't bad, but the similarity of
the generated text to the ground truth is low, based on the evaluation metrics.
This indicates that it is worth performing supervised tuning to see if you can
improve model performance for this use case.

## Create a tuned model

Create a remote model very similar to the one you created in
[Create a model](https://docs.cloud.google.com/bigquery/docs/tune-evaluate#create_a_baseline_model), but this time specifying the
[`AS SELECT` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-tuned#as_select)
to provide the training data in order to tune the model.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement to create a
   [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-tuned):

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.gemini_tuned`
     REMOTE
       WITH CONNECTION DEFAULT
     OPTIONS (
       endpoint = 'gemini-2.0-flash-001',
       max_iterations = 500,
       data_split_method = 'no_split')
   AS
   SELECT
     input AS prompt, output AS label
   FROM `bqml_tutorial.wiki_auto_style_transfer_train`;
   ```

   The query takes a few minutes to complete, after which the
   `gemini_tuned` model appears in the
   `bqml_tutorial` dataset in the **Explorer** pane. Because the query uses a
   `CREATE MODEL` statement to create a model, there are no query results.

## Check tuned model performance

Run the `AI.GENERATE_TEXT` function to see how the tuned model performs on the
evaluation data.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   SELECT result, ground_truth
   FROM
     AI.GENERATE_TEXT(
       MODEL `bqml_tutorial.gemini_tuned`,
       (
         SELECT
           input AS prompt, output AS ground_truth
         FROM `bqml_tutorial.wiki_auto_style_transfer_valid`
         LIMIT 10
       ));
   ```

   If you examine the output data, you see that the tuned model produces text
   that is much more similar in style to the ground truth content.

## Evaluate the tuned model

Use the `ML.EVALUATE` function to see how the tuned model's responses compare
to ideal responses.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   SELECT *
   FROM
     ML.EVALUATE(
       MODEL `bqml_tutorial.gemini_tuned`,
       (
         SELECT
           input AS prompt, output AS label
         FROM `bqml_tutorial.wiki_auto_style_transfer_valid`
       ),
       STRUCT('text_generation' AS task_type));
   ```

The output looks similar to the following:

<br />

```
   +---+---+---+---+
   | bleu4_score         | rouge-l_precision   | rouge-l_recall      | rouge-l_f1_score    | evaluation_status                          |
   +---+---+---+---+---+
   | 0.416868792119966   | 0.642001000843349   | 0.55910008048151372 | 0.5907226262084847  | {                                          |
   |                     |                     |                     |                     |  "num_successful_rows": 176,               |
   |                     |                     |                     |                     |  "num_total_rows": 176                     |
   |                     |                     |                     |                     | }                                          |
   +---+---+ ---+---+---+
   
```

<br />

You can see that even though the training dataset used only 1,408 examples,
there is a marked improvement in performance as indicated by the higher
evaluation metrics.

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

<br />