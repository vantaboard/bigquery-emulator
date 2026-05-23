# Get data insights from a contribution analysis model
using a summable metric

In this tutorial, you use a
[contribution analysis](https://docs.cloud.google.com/bigquery/docs/contribution-analysis) model to analyze
sales changes between 2020 and 2021 in the Iowa liquor sales dataset. This
tutorial guides you through performing the following tasks:

- Create an input table based on publicly available Iowa liquor data.
- Create a [contribution analysis model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis) that uses a [summable metric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_metric). This type of model summarizes a given metric for a combination of one or more dimensions in the data, to determine how those dimensions contribute to the metric value.
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

Create a table that contains test and control data to analyze. The test table
contains liquor data from 2021 and the control table contains liquor data from
2020. The following query combines the test and control data into a single
input table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE TABLE bqml_tutorial.iowa_liquor_sales_sum_data AS (
     (SELECT
       store_name,
       city,
       vendor_name,
       category_name,
       item_description,
       SUM(sale_dollars) AS total_sales,
       FALSE AS is_test
     FROM `bigquery-public-data.iowa_liquor_sales.sales`
     WHERE EXTRACT(YEAR from date) = 2020
     GROUP BY store_name, city, vendor_name, category_name, item_description, is_test)
     UNION ALL
     (SELECT
       store_name,
       city,
       vendor_name,
       category_name,
       item_description,
       SUM(sale_dollars) AS total_sales,
       TRUE AS is_test
     FROM `bigquery-public-data.iowa_liquor_sales.sales`
     WHERE EXTRACT (YEAR FROM date) = 2021
     GROUP BY store_name, city, vendor_name, category_name, item_description, is_test)
   );
   ```

## Create the model

Create a contribution analysis model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE MODEL bqml_tutorial.iowa_liquor_sales_sum_model
     OPTIONS(
       model_type='CONTRIBUTION_ANALYSIS',
       contribution_metric = 'sum(total_sales)',
       dimension_id_cols = ['store_name', 'city', 'vendor_name', 'category_name',
         'item_description'],
       is_test_col = 'is_test',
       min_apriori_support=0.05
     ) AS
   SELECT * FROM bqml_tutorial.iowa_liquor_sales_sum_data;
   ```

The query takes approximately 60 seconds to complete, after which the model
`iowa_liquor_sales_sum_model` appears in the `bqml_tutorial` dataset. Because
the query uses a `CREATE MODEL` statement to create a model, there are no
query results.

## Get insights from the model

Get insights generated by the contribution analysis model by using the
`ML.GET_INSIGHTS` function.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement to select columns from the
   [output for a summable metric contribution analysis model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights#output_for_summable_metric_contribution_analysis_models):

   ```googlesql
   SELECT
     contributors,
     metric_test,
     metric_control,
     difference,
     relative_difference,
     unexpected_difference,
     relative_unexpected_difference,
     apriori_support,
     contribution
   FROM
     ML.GET_INSIGHTS(
       MODEL `bqml_tutorial.iowa_liquor_sales_sum_model`);
   ```

The first several rows of the output should look similar to the following.
The values are truncated to improve readability.

| contributors | metric_test | metric_control | difference | relative_difference | unexpected_difference | relative_unexpected_difference | apriori_support | contribution |
|---|---|---|---|---|---|---|---|---|
| all | 428068179 | 396472956 | 31595222 | 0.079 | 31595222 | 0.079 | 1.0 | 31595222 |
| vendor_name=SAZERAC COMPANY INC | 52327307 | 38864734 | 13462573 | 0.346 | 11491923 | 0.281 | 0.122 | 13462573 |
| city=DES MOINES | 49521322 | 41746773 | 7774549 | 0.186 | 4971158 | 0.111 | 0.115 | 7774549 |
| vendor_name=DIAGEO AMERICAS | 84681073 | 77259259 | 7421814 | 0.096 | 1571126 | 0.018 | 0.197 | 7421814 |
| category_name=100% AGAVE TEQUILA | 23915100 | 17252174 | 6662926 | 0.386 | 5528662 | 0.3 | 0.055 | 6662926 |

The output is automatically sorted by contribution, or `ABS(difference)`, in
descending order. In the `all` row, the `difference` column shows there was a
$31,595,222 increase in total sales from 2020 to 2021, a 7.9% increase as
indicated by the `relative_difference` column. In the second row, with
`vendor_name=SAZERAC COMPANY INC`, there was an `unexpected_difference` of
$11,491,923, meaning this segment of data grew 28% more than the growth rate of
the data as a whole, as seen from the `relative_unexpected_difference` column.
For more information, see the
[summable metric output columns](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights#output_for_summable_metric_contribution_analysis_models).

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