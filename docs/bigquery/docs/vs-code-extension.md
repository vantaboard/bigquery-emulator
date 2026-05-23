# Use the Google Cloud for Visual Studio Code extension

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or ask questions that are related to this Preview feature, contact [bigquery-ide-plugin@google.com](mailto:bigquery-ide-plugin@google.com).

The Google Cloud [Visual Studio Code (VS Code)](https://code.visualstudio.com/)
extension lets you do the following in VS Code:

- Develop and execute BigQuery notebooks.
- Browse, inspect, and preview BigQuery datasets.

## Before you begin

1. In your local terminal, check to make sure you have
   [Python 3.11](https://www.python.org/downloads/) or later installed on your
   system:

   ```bash
   python3 --version
   ```
2. [Install the Google Cloud CLI](https://docs.cloud.google.com/sdk/docs/install).

3. In your local terminal,
   [initialize the gcloud CLI](https://docs.cloud.google.com/sdk/docs/initializing):

   ```bash
   gcloud init
   ```
4. Configure a default project:

   ```bash
   gcloud config set project PROJECT_ID
   ```

   Replace <var translate="no">`PROJECT_ID`</var> with your default project.
5. Set up [Application Default Credentials](https://docs.cloud.google.com/bigquery/docs/authentication):

   ```bash
   gcloud auth application-default login
   ```
6. [Download and install VS Code](https://code.visualstudio.com/download).

7. Open VS Code, and then in the activity bar, click **Extensions**.

8. Using the search bar, find the **Jupyter** extension, and then click
   **Install**. The BigQuery features in VS Code require the
   Jupyter extension by Microsoft as a dependency.

   ![A list of Jupyter extensions in the VS Code console.](https://docs.cloud.google.com/static/bigquery/images/vs-code-jupyter.png)

## Install the Google Cloud extension

1. Open VS Code, and then in the activity bar, click **Extensions**.
2. Using the search bar, find the **Google Cloud Code** extension, and then
   click **Install**.

   ![The Google Cloud Code extension in the VS Code console.](https://docs.cloud.google.com/static/bigquery/images/vs-code-google-cloud.png)
3. If prompted, restart VS Code.

The **Google Cloud Code** icon is now visible in the activity bar.

## Configure the extension

1. Open VS Code, and then in the activity bar, click **Google Cloud Code**.
2. Open the **BigQuery Notebooks** section.
3. Click **Login to Google Cloud**. You are redirected to sign in with your credentials.
4. Use the top-level application taskbar to navigate to **Code \> Settings \> Settings \> Extensions**.
5. Find **Google Cloud Code** , and click the **Manage** icon to open the menu.
6. Select **Settings**.
7. For the **Cloud Code: Project** setting, enter the name of the Google Cloud project that you want to use to execute notebooks and display BigQuery datasets.
8. For the **Cloud Code \> Beta: BigQuery Region** setting, enter a [BigQuery location](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations). The extension displays datasets from this location.

## Develop BigQuery notebooks

1. Open VS Code, and then in the activity bar, click **Google Cloud Code**.
2. Open the **BigQuery Notebooks** section, and click **BigQuery Notebook** . A new `.ipynb` file containing sample code is created and opened in the editor.
3. In the new notebook, click **Select Kernel**, and select a Python kernel.
   BigQuery notebooks require a local Python kernel for
   execution. You can create a new virtual environment or use one of the
   existing ones.

   ![The select kernel interface in the VS Code console.](https://docs.cloud.google.com/static/bigquery/images/vs-code-python-kernel.png)
4. If it hasn't already been installed in your virtual environment, install the
   `bigframes` client library:

   1. Open the **Terminal** window.
   2. Run the `pip install bigframes` command.

You can now write and execute code in your BigQuery notebook.

## Explore and preview BigQuery datasets

1. Open VS Code, and then in the activity bar, click **Google Cloud Code**.
2. To see datasets and tables from your specified project and region, open the **BigQuery Datasets** section. BigQuery public datasets are also visible.
3. To open a new tab in the editor, click any table name. This tab contains the table details, schema, and preview.

## Pricing

The Visual Studio Code extension is free, but you are charged for any
Google Cloud services (BigQuery, Managed Service for Apache Spark,
Cloud Storage) that you use.

## What's next

- Learn more about [notebooks in BigQuery](https://docs.cloud.google.com/bigquery/docs/programmatic-analysis).
- Learn more about [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).