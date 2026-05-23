You can explore query results by using [SQL cells](https://docs.cloud.google.com/colab/docs/sql-cells) or
code cells in [BigQuery Colab Enterprise notebooks](https://docs.cloud.google.com/colab/docs/introduction).

In this tutorial, you query data from a
[BigQuery public dataset](https://docs.cloud.google.com/bigquery/public-data)
and explore the query results in a notebook.

## Objectives

- Create and run a query in BigQuery.
- Explore query results in a notebook using SQL cells and code cells.

## Costs

This tutorial uses a dataset available through the
[Google Cloud Public Datasets Program](https://cloud.google.com/blog/products/data-analytics/big-data-analytics-in-the-cloud-with-free-public-datasets).
Google pays for the storage of these datasets and provides public access to the
data. You incur charges for the queries that you perform on the data. For
more information, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing).

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

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

   For new projects, BigQuery is automatically enabled.

## Set the default region for code assets

All new code assets in your Google Cloud project use a default region. After the
asset is created, you can't change its region.

> [!IMPORTANT]
> **Important:** If you change the region while creating a code asset, that region becomes the default for all subsequent code assets. Existing code assets are not affected.

To set the default region for new code assets, do the following:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Next to the project name, click

   **View files panel actions** \> **Switch code region**.

4. Select the code region that you want to use as a default.

5. Click **Save**.

For a list of supported regions, see
[BigQuery Studio locations](https://docs.cloud.google.com/bigquery/docs/locations#bqstudio-loc).

### Required permissions

To create and run notebooks, you need the following Identity and Access Management (IAM)
roles:

- [BigQuery User (`roles/bigquery.user`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user)
- [Notebook Runtime User (`roles/aiplatform.notebookRuntimeUser`)](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.notebookRuntimeUser)
- [Code Creator (`roles/dataform.codeCreator`)](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeCreator)

## Open query results in a notebook

You can run a SQL query and then use a notebook to explore the data. This
approach is useful if you want to modify the data in BigQuery
before working with it, or if you need only a subset of the fields in the table.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**.

3. Go to the `bigquery-public-data` project, click
   **Toggle node** to expand
   it, and then click **Datasets**. A new tab opens in the details pane that
   shows a list of all the datasets in the project.

4. In the **Filter** box,
   choose **Dataset ID** and enter **ml_datasets**.

   ![The Filter field on the Datasets page](https://docs.cloud.google.com/bigquery/images/public-dataset-filter.png)
5. On the **Datasets** page, click **ml_datasets \> penguins**.

6. Click **Query**.

7. Add an asterisk (`*`) for field selection to the generated query, so that
   it looks like the following example:

   ```googlesql
   SELECT * FROM `bigquery-public-data.ml_datasets.penguins` LIMIT 1000;
   ```
8. Click **Run**.

9. In the **Query results** section, click **Open in** , and then click
   **Notebook**.

## Prepare the notebook for use

Prepare the notebook for use by connecting to a runtime and setting application
default values.

1. In the notebook header, click **Connect** to
   [connect to the default runtime](https://docs.cloud.google.com/bigquery/docs/create-notebooks#connect_to_the_default_runtime).

2. In the **Setup** code block, click
   **Run cell**.

## Explore the data

1. Click **Insert code cell
   options \> Add SQL cell**.

   ![The Add SQL cell option in the Insert code cell menu](https://docs.cloud.google.com/bigquery/images/add-sql-cell-option.png)
2. Enter the following query in the SQL cell:

       SELECT * FROM `bigquery-public-data.ml_datasets.penguins` LIMIT 1000;

3. Click **Run cell**.

   The query results are displayed in a [BigQuery
   DataFrame](https://docs.cloud.google.com/bigquery/docs/reference/bigquery-dataframes).
4. Alternatively, to load the query results into a BigQuery
   DataFrame using the query job you previously ran in the query editor,
   follow these steps:

   1. Go to the **Result set loaded from BigQuery job as a DataFrame**
      section.

   2. In the code block, click
      **Run cell**.

      The query results are displayed in a BigQuery DataFrame.
5. To get descriptive metrics for the data, follow these steps:

   1. Go to the **Show descriptive statistics using describe()** section.

   2. In the code block, click
      **Run cell**.

      The results are displayed in a BigQuery DataFrame.
6. Optional: Use other Python functions or packages to explore and analyze
   the data.

The following code sample shows using
[`bigframes.pandas`](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction)
to analyze data, and [`bigframes.ml`](https://docs.cloud.google.com/bigquery/docs/dataframes-ml-ai)
to create a linear regression model from **penguins** data in a
BigQuery DataFrame:

    import bigframes.pandas as bpd

    # Load data from BigQuery
    query_or_table = "bigquery-public-data.ml_datasets.penguins"
    bq_df = bpd.read_gbq(query_or_table)

    # Inspect one of the columns (or series) of the DataFrame:
    bq_df["body_mass_g"]

    # Compute the mean of this series:
    average_body_mass = bq_df["body_mass_g"].mean()
    print(f"average_body_mass: {average_body_mass}")

    # Find the heaviest species using the groupby operation to calculate the
    # mean body_mass_g:
    (
        bq_df["body_mass_g"]
        .groupby(by=bq_df["species"])
        .mean()
        .sort_values(ascending=False)
        .head(10)
    )

    # Create the Linear Regression model
    from bigframes.ml.linear_model import LinearRegression

    # Filter down to the data we want to analyze
    adelie_data = bq_df[bq_df.species == "Adelie Penguin (Pygoscelis adeliae)"]

    # Drop the columns we don't care about
    adelie_data = adelie_data.drop(columns=["species"])

    # Drop rows with nulls to get our training data
    training_data = adelie_data.dropna()

    # Pick feature columns and label column
    X = training_data[
        [
            "island",
            "culmen_length_mm",
            "culmen_depth_mm",
            "flipper_length_mm",
            "sex",
        ]
    ]
    y = training_data[["body_mass_g"]]

    model = LinearRegression(fit_intercept=False)
    model.fit(X, y)
    model.score(X, y)

## Clean up


To avoid incurring charges to your Google Cloud account for the resources used in this
tutorial, either delete the project that contains the resources, or keep the project and
delete the individual resources.

The easiest way to eliminate billing is to delete the Google Cloud project
that you created for this tutorial.

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

## What's next

- Learn more about [creating notebooks in BigQuery](https://docs.cloud.google.com/bigquery/docs/create-notebooks).
- Learn more about [exploring data with BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).