# Use the Colab Enterprise Data Science Agent with BigQuery

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback, to ask questions, or to request to opt out of this Preview feature, contact [vertex-notebooks-previews-external@google.com](mailto:vertex-notebooks-previews-external@google.com) or fill out the [Data Science Agent Public Preview Opt-out form](https://forms.gle/KuTAunuLT2YmFAcs8).

The Data Science Agent (DSA) for Colab Enterprise and
BigQuery lets you automate exploratory data analysis, perform
machine learning tasks, and deliver insights all within a
Colab Enterprise notebook.

## Before you begin

1.


   Enable the BigQuery, Vertex AI, Dataform, and Compute Engine APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,aiplatform.googleapis.com,dataform.googleapis.com,compute.googleapis.com)

   For new projects, the BigQuery API is
   automatically enabled.

If you're new to Colab Enterprise in BigQuery, see
the setup steps on the [Create notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks#required_permissions)
page.

## Limitations

- The Data Science Agent is only available within the Colab Enterprise environment.
- The Data Science Agent supports the following data sources:
  - CSV files
  - BigQuery tables
- The code produced by the Data Science Agent only runs in your notebook's runtime.
- The Data Science Agent isn't supported in projects that have enabled VPC Service Controls.
- Searching for BigQuery tables using the `@mention` function is limited to your current project. Use the table selector to search across projects.
- The `@mention` function only searches for BigQuery tables. To search for data files that you can upload, use the `+` symbol.
- PySpark in the Data Science Agent only generates Managed Service for Apache Spark 4.0 code. The DSA can help you upgrade to Managed Service for Apache Spark 4.0, but users who require earlier versions shouldn't use the Data Science Agent.

## When to use the Data Science Agent

The Data Science Agent helps you with tasks ranging from exploratory data
analysis to generating machine learning predictions and forecasts. You can use
the DSA for:

- **Large-scale data processing**: Use BigQuery ML, BigQuery DataFrames, or Managed Service for Apache Spark to perform distributed data processing on large datasets. This lets you efficiently clean, transform, and analyze data that's too large to fit into memory on a single machine.
- **Generating a plan**: Generate and modify a plan to complete a particular task using common tools such as Python, SQL, Managed Service for Apache Spark, and BigQuery DataFrames.
- **Data exploration**: Explore a dataset to understand its structure, identify potential issues like missing values and outliers, and examine the distribution of key variables using Python or SQL.
- **Data cleaning**: Clean your data. For example, remove data points that are outliers.
- **Data wrangling** : Convert categorical features into numerical representations using techniques like one-hot encoding or label encoding or by using BigQuery ML [feature transformation tools](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-transform). Create new features for analysis.
- **Data analysis**: Analyze the relationships between different variables. Calculate correlations between numerical features and explore distributions of categorical features. Look for patterns and trends in the data.
- **Data visualization**: Create visualizations such as histograms, box plots, scatter plots, and bar charts that represent the distributions of individual variables and the relationships between them. You can also create visualizations in Python for tables stored in BigQuery.
- **Feature engineering**: Engineer new features from a cleaned dataset.
- **Data splitting**: Split an engineered dataset into training, validation, and testing datasets.
- **Model training** : Train a model by using the training data in a pandas DataFrame (`X_train`, `y_train`), [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-ml-ai#train-models), a [PySpark DataFrame](https://spark.apache.org/docs/latest/api/python/reference/pyspark.sql/api/pyspark.sql.DataFrame.html), or by using the BigQuery ML [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create) with BigQuery tables.
- **Model optimization** : Optimize a model by using the validation set. Explore alternative models like `DecisionTreeRegressor` and `RandomForestRegressor` and compare their performance.
- **Model evaluation** : Evaluate model performance on a test dataset using a pandas DataFrame, BigQuery DataFrames, or a PySpark DataFrame. You can also assess model quality and compare models by using BigQuery ML [model evaluation functions](https://docs.cloud.google.com/bigquery/docs/evaluate-overview) for models trained using BigQuery ML.
- **Model inference** : Perform inference with BigQuery ML trained models, imported models, and remote models using BigQuery ML [inference functions](https://docs.cloud.google.com/bigquery/docs/inference-overview). You can also use the BigFrames `model.predict()` method or PySpark [transformers](https://spark.apache.org/docs/latest/ml-pipeline.html#transformers) to make predictions.

## Use the Data Science Agent in BigQuery

The following steps show you how to use the Data Science Agent in
BigQuery.

1. Create or open a Colab Enterprise notebook.

2. Optional: Reference your data in one of the following ways:

   - Upload a CSV file or use the `+` symbol in your prompt to search for available files.
   - Choose one or more BigQuery tables in the table selector from your current project or from other projects you have access to.
   - Reference a BigQuery table name in your prompt in this format: `project_id:dataset.table`.
   - Type the `@` symbol to search for a BigQuery table name using the `@mention` function.
3. Enter a prompt that describes the data analysis you want to perform or the
   prototype you want to build. The Data Science Agent's default behavior is to
   generate Python code using open source libraries such as sklearn to
   accomplish complex machine learning tasks. To use a specific tool, include
   the following keywords in your prompt:

   - If you want to use BigQuery ML, include the "SQL" keyword.
   - If you want to use "BigQuery DataFrames", specify the "BigFrames" or "BigQuery DataFrames" keywords.
   - If you want to use PySpark, include the "Apache Spark" or "PySpark" keywords.

   For help, see the [sample prompts](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent#sample-prompts).
4. Examine the results.

### Analyze a CSV file

To analyze a CSV using the Data Science Agent in BigQuery,
follow these steps.

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. On the BigQuery Studio welcome page, under **Create new** ,
   click **Notebook**.

   Alternatively, in the tab bar, click the

   drop-down arrow next to the **+** icon, and then click
   **Notebook \> Empty notebook**.
3. Click the
   ![](https://docs.cloud.google.com/static/colab/images/icon-gemini.png)
   **Toggle Gemini in Colab** button to open the chat dialog.

   > [!NOTE]
   > **Note:** You can move the chat dialog into a separate panel outside the notebook by clicking the **Move to panel** icon.

4. Upload your CSV file.

   1. In the chat dialog, click

      **Add to Gemini \> Upload**.

   2. If necessary, authorize your Google Account.

   3. Browse to the location of the CSV file, and then click **Open**.

5. Alternatively, type the `+` symbol in your prompt to search for available
   files to upload.

6. Enter your prompt in the chat window. For example: `Identify trends and
   anomalies in this file.`

7. Click
   **Send**. The results appear in the chat window.

8. You can ask the agent to change the plan, or you can run it by clicking
   **Accept \& run** . As the plan runs, generated code and text appear in the
   notebook. Click **Cancel** to stop.

### Analyze BigQuery tables

To analyze a BigQuery table, choose one or more tables in the
table selector, provide a reference to the table in your prompt, or search for a
table by using the `@` symbol.

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. On the BigQuery Studio welcome page, under **Create new** ,
   click **Notebook**.

   Alternatively, in the tab bar, click the

   drop-down arrow next to the **+** icon, and then click
   **Notebook \> Empty notebook**.
3. Click the
   ![](https://docs.cloud.google.com/static/colab/images/icon-gemini.png)
   **Toggle Gemini in Colab** button to open the chat dialog.

   > [!NOTE]
   > **Note:** You can move the chat dialog into a separate panel outside the notebook by clicking the **Move to panel** icon.

4. Enter your prompt in the chat window.

5. Reference your data in one of the following ways:

   1. Choose one or more tables using the table selector:

      1. Click
         **Add to Gemini \> BigQuery tables**.

      2. In the **BigQuery tables** window, select
         one or more tables in your project. You can search for tables across
         projects and filter tables by using the search bar.

   2. Include a BigQuery table name directly in your prompt.
      For example: "Help me perform exploratory data analysis and get
      insights about the data in this table:
      `project_id:dataset.table`."

      Replace the following:
      - `project_id`: your project ID
      - `dataset`: the name of the dataset that contains the table you're analyzing
      - `table`: the name of the table you're analyzing
   3. Type `@` to search for a BigQuery table in your current
      project.

6. Click
   **Send**.

   The results appear in the chat window.
7. You can ask the agent to change the plan, or you can run it by clicking
   **Accept \& run** . As the plan runs, generated code and text appear in the
   notebook. For additional steps in the plan, you may be required to click
   **Accept \& run** again. Click **Cancel** to stop.

## Sample prompts

Regardless of the complexity of the prompt that you use, the Data Science Agent
generates a plan that you can refine to meet your needs.

The following examples show the types of prompts that you can use with the DSA.

### Python prompts

Python code is generated by default unless you use a specific keyword in the
prompt such as "BigQuery ML" or "SQL".

- Investigate and fill missing values by using the k-Nearest Neighbors (KNN) machine learning algorithm.
- Create a plot of salary by experience level. Use the `experience_level` column to group the salaries, and create a box plot for each group showing the values from the `salary_in_usd` column.
- Use the XGBoost algorithm to make a model for determining the `class` variable of a particular fruit. Split the data into training and testing datasets to generate a model and to determine the model's accuracy. Create a confusion matrix to show the predictions amongst each class, including all predictions that are correct and incorrect.
- Forecast `target_variable` from `filename.csv` for the next six months.

### SQL and BigQuery ML prompts

- Create and evaluate a classification model on `bigquery-public-data.ml_datasets.census_adult_income` using BigQuery SQL.
- Using SQL, forecast the future traffic of my website for the next month based on `bigquery-public-data.google_analytics_sample.ga_sessions_*`. Then, plot the historical and forecasted values.
- Group similar customers together to create targeting market campaigns using a KMeans model and BigQuery ML SQL functions. Use three features for clustering. Then visualize the results by creating a series of 2D scatter plots. Use the table `bigquery-public-data.ml_datasets.census_adult_income`.
- Generate text embeddings in BigQuery ML using the review content in `bigquery-public-data.imdb.reviews`.

For a list of supported models and machine learning tasks, see the
[BigQuery ML documentation](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).

### DataFrame prompts

- Create a pandas DataFrame for the data in `project_id:dataset.table`. Analyze the data for null values, and then graph the distribution of each column using the graph type. Use violin plots for measured values and bar plots for categories.
- Read `filename.csv` and construct a DataFrame. Run analysis on the DataFrame to determine what needs to be done with values. For example, are there missing values that need to be replaced or removed, or are there duplicate rows that need to be addressed. Use the data file to determine the distribution of the money invested in USD per city location. Graph the top 20 results using a bar graph that shows the results in descending order as Location versus Avg Amount Invested (USD).
- Create and evaluate a classification model on `project_id:dataset.table` using BigQuery DataFrames.
- Create a time series forecasting model on `project_id:dataset.table` using BigQuery DataFrames, and visualize the model evaluations.
- Visualize the sales figures in the past year in BigQuery table `project_id:dataset.table` using BigQuery DataFrames.
- Find the features that can best predict the penguin species from the table `bigquery-public_data.ml_datasets.penguins` using BigQuery DataFrames.

### PySpark prompts

- Create and evaluate a classification model on `project_id:dataset.table` using Managed Service for Apache Spark.
- Group similar customers together to create targeting market campaigns, but first do dimensionality reduction using a PCA model. Use PySpark to do this on table `project_id:dataset.table`.

## Turn off Gemini in BigQuery


To turn off Gemini in BigQuery for a
Google Cloud project, an administrator must turn off the
Gemini for Google Cloud API. See
[Disabling services](https://docs.cloud.google.com/service-usage/docs/enable-disable#disabling).


To turn off Gemini in BigQuery for a specific user, an
administrator needs to revoke the
[Gemini for
Google Cloud User](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudaicompanion#cloudaicompanion.user) (`roles/cloudaicompanion.user`) role for that user. See
[Revoke
a single IAM role](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access#revoke-single-role).

> [!NOTE]
> **Note:** To opt out of using the Data Science Agent Preview without turning off other Gemini features, contact [vertex-notebooks-previews-external@google.com](mailto:vertex-notebooks-previews-external@google.com).

## Pricing

During Preview, you are charged for running code in the notebook's runtime and
for any BigQuery [slots](https://docs.cloud.google.com/bigquery/docs/slots) you used. For more
information, see [Colab Enterprise pricing](https://cloud.google.com/colab/pricing).

## Supported regions

To view the supported regions for Colab Enterprise's Data Science
Agent, see [Locations](https://docs.cloud.google.com/colab/docs/locations).