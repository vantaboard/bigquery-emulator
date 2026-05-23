# Get data insights from a contribution analysis model using a summable ratio metric

In this tutorial, you use a
[contribution analysis](https://docs.cloud.google.com/bigquery/docs/contribution-analysis) model to analyze
the contribution of the cost of sales ratio in the Iowa liquor
sales dataset. This tutorial guides you through performing the following tasks:

- Create an input table based on publicly available Iowa liquor data.
- Create a [contribution analysis model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis) that uses a [summable ratio metric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_ratio_metric). This type of model summarizes the values of two numeric columns and determines the ratio differences across the control and test dataset for each segment of the data.
- Get the metric insights from the model by using the [`ML.GET_INSIGHTS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights).

Before starting this tutorial, you should be familiar with the
[contribution analysis](https://docs.cloud.google.com/bigquery/docs/contribution-analysis) use case.

## Required permissions

- To create the dataset, you need the `bigquery.datasets.create`
  Identity and Access Management (IAM) permission.

- To create the model, you need the following permissions:

  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
- To run inference, you need the following permissions:

  - `bigquery.models.getData`
  - `bigquery.jobs.create`

## Costs


In this document, you use the following billable components of Google Cloud:


- **BigQuery ML**: You incur costs for the data that you process in BigQuery.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

For more information about BigQuery pricing, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) in
the BigQuery documentation.

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


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com)

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

## Create a table of input data

Create a table that contains test and control data to analyze. The following
query creates two intermediate tables, a test table for liquor data from 2021
and a control table with liquor data from 2020, and then performs a union of
the intermediate tables to create a table with both test and control rows
and the same set of columns.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE TABLE bqml_tutorial.iowa_liquor_sales_data AS
   (SELECT
     store_name,
     city,
     vendor_name,
     category_name,
     item_description,
     SUM(sale_dollars) AS total_sales,
     SUM(state_bottle_cost) AS total_bottle_cost,
     FALSE AS is_test
   FROM `bigquery-public-data.iowa_liquor_sales.sales`
   WHERE EXTRACT(YEAR FROM date) = 2020
   GROUP BY store_name, city, vendor_name, category_name, item_description, is_test)
   UNION ALL
   (SELECT
     store_name,
     city,
     vendor_name,
     category_name,
     item_description,
     SUM(sale_dollars) AS total_sales,
     SUM(state_bottle_cost) AS total_bottle_cost,
     TRUE AS is_test
   FROM `bigquery-public-data.iowa_liquor_sales.sales`
   WHERE EXTRACT(YEAR FROM date) = 2021
   GROUP BY store_name, city, vendor_name, category_name, item_description, is_test);
   ```

## Create the model

Create a contribution analysis model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE MODEL bqml_tutorial.liquor_sales_model
   OPTIONS(
     model_type = 'CONTRIBUTION_ANALYSIS',
     contribution_metric = 'sum(total_bottle_cost)/sum(total_sales)',
     dimension_id_cols = ['store_name', 'city', 'vendor_name', 'category_name', 'item_description'],
     is_test_col = 'is_test',
     min_apriori_support = 0.05
   ) AS
   SELECT * FROM bqml_tutorial.iowa_liquor_sales_data;
   ```

The query takes approximately 35 seconds to complete, after which the model
`liquor_sales_model` appears in the `bqml_tutorial` dataset. Because the
query uses a `CREATE MODEL` statement to create a model, there are no query
results.

## Get insights from the model

Get insights generated by the contribution analysis model by using the
`ML.GET_INSIGHTS` function.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement to select columns from the
   [output for a summable ratio metric contribution analysis model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights#output_for_summable_ratio_metric_contribution_analysis_models):

   ```googlesql
   SELECT
   contributors,
   metric_test,
   metric_control,
   metric_test_over_metric_control,
   metric_test_over_complement,
   metric_control_over_complement,
   aumann_shapley_attribution,
   apriori_support
   contribution
   FROM
     ML.GET_INSIGHTS(
       MODEL `bqml_tutorial.liquor_sales_model`)
   ORDER BY aumann_shapley_attribution DESC;
   ```

The first several rows of the output should look similar to the following.
The values are truncated to improve readability.

| contributors | metric_test | metric_control | metric_test_over_metric_control | metric_test_over_complement | metric_control_over_complement | aumann_shapley_attribution | apriori_support | contribution |
|---|---|---|---|---|---|---|---|---|
| all | 0.069 | 0.071 | 0.969 | null | null | -0.00219 | 1.0 | 0.00219 |
| city=DES MOINES | 0.048 | 0.054 | 0.88 | 0.67 | 0.747 | -0.00108 | 0.08 | 0.00108 |
| vendor_name=DIAGEO AMERICAS | 0.064 | 0.068 | 0.937 | 0.917 | 0.956 | -0.0009 | 0.184 | 0.0009 |
| vendor_name=BACARDI USA INC | 0.071 | 0.082 | 0.857 | 1.025 | 1.167 | -0.00054 | 0.057 | 0.00054 |
| vendor_name=PERNOD RICARD USA | 0.068 | 0.077 | 0.89 | 0.988 | 1.082 | -0.0005 | 0.061 | 0.0005 |

In the output, you can see that the data segment `city=DES MOINES`
has the highest
contribution of change in the sales ratio. You can also see this difference
in the `metric_test` and `metric_control` columns, which show that the ratio
decreased in the test data compared to the control data. Other metrics,
such as `metric_test_over_metric_control`, `metric_test_over_complement`,
and `metric_control_over_complement`, compute additional statistics that
describe the relationship between the control and test ratios and how they
relate to the overall population. For more information, see
[Output for summable ratio metric contribution analysis models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights#output_for_summable_ratio_metric_contribution_analysis_models).

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