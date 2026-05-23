# Create pipelines

This document describes how to create
[pipelines in BigQuery](https://docs.cloud.google.com/bigquery/docs/pipelines-introduction).
Pipelines are powered by
[Dataform](https://docs.cloud.google.com/dataform/docs/overview).

## Before you begin

<br />

### Required roles for pipelines


To get the permissions that
you need to create pipelines,

ask your administrator to grant you the
following IAM roles on the project:

- To create pipelines: [Code Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeCreator) (`roles/dataform.codeCreator`)
- To edit and run pipelines: [Dataform Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.editor) (`roles/dataform.editor`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

<br />

For more information about Dataform IAM, see
[Control access with IAM](https://docs.cloud.google.com/dataform/docs/access-control).

> [!NOTE]
> **Note:** When you create a pipeline, BigQuery grants you the [Dataform Admin role](https://docs.cloud.google.com/dataform/docs/access-control#dataform.admin) (`roles/dataform.admin`) on that pipeline. All users with the Dataform Admin role granted on the Google Cloud project have owner access to all the pipelines created in the project. To override this behavior, see [Grant a specific role upon resource creation](https://docs.cloud.google.com/dataform/docs/access-control#grant-specific-role).

### Required roles for notebook options


To get the permissions that
you need to select a runtime template in notebook options,

ask your administrator to grant you the
[Notebook Runtime User](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.notebookRuntimeUser) (`roles/aiplatform.notebookRuntimeUser`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

If you don't have this role, you can select the default notebook runtime
specification.

### Set the default region for code assets

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

## Create a pipeline

You can also use the BigQuery **Pipelines \& Connections**
page in the Google Cloud console to create a Dataform pipeline that uses a
[streamlined, BigQuery-specific workflow](https://docs.cloud.google.com/bigquery/docs/pipeline-connection-page).
This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

To create a pipeline, follow these steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the tab bar of the editor pane, click the

   arrow next to the **+** sign, and then click
   **Pipeline**.

3. Optional: To rename the pipeline,
   click the pipeline name, and then type a new name.

4. Click **Get started** , and then go to the **Settings** tab.

5. In the **Authentication** section, choose to authorize the
   pipeline with the user credentials for your Google Account or a service
   account.

   - To use the user credentials for your Google Account
     ([Preview](https://cloud.google.com/products#product-launch-stages)), select
     **Run with my user credentials**.

     > [!NOTE]
     > **Note:** Authenticating API-based runs with user credentials isn't supported. To [run all the tasks in a pipeline](https://docs.cloud.google.com/bigquery/docs/create-pipelines#run-pipeline-all-tasks) using the Dataform API, you must configure the pipeline to use a service account.

   - To use a service account, select
     **Run with selected service account** , and then select a
     service account. If you need to create a service account, click
     **New service account**.

6. In the **Processing location** section, select a processing location for the
   pipeline.

   - To enable the automatic selection of a location, select
     **Automatic location selection**. This option selects a location
     based on the datasets referenced in the request. The selection process is
     as follows:

     - If your query references datasets from the same location, BigQuery uses that location.
     - If your query references datasets from two or more different locations, an error occurs. For details about this limitation, see [Cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication).
     - If your query doesn't reference any datasets, BigQuery defaults to the `US` multi-region.
   - To pick a specific region, select **Region** , then choose a
     region in the **Region** menu. Alternatively, you can use the
     [`@@location` system variable](https://docs.cloud.google.com/bigquery/docs/reference/system-variables)
     in your query. For more information, see
     [Specify locations](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations).

   - To pick a multi-region, select **Multi-region** , then choose a
     multi-region in the **Multi-region** menu.

   The pipeline processing location doesn't need to match your default
   storage location for code assets.

### SQLX options

To configure the SQLX settings for your pipeline, do the following in the
**SQLX options** section:

1. In the **Default project** field, enter the name of an existing
   Google Cloud project. This value is used for `defaultProject` in the
   `workflow_settings.yaml` file and for `defaultDatabase` in the
   `dataform.json` file. The default project is used by pipeline tasks during their
   execution.

   > [!NOTE]
   > **Note:** The project name isn't validated, so it's possible to enter any non-empty string. However, if the project doesn't exist, the pipeline execution fails.

2. Optional: In the **Default dataset** field, search for and select an existing
   dataset. The list of available datasets is filtered based on the selected
   project and processing location. This value is used for `defaultDataset` in
   the `workflow_settings.yaml` file. The default dataset is used by pipeline tasks
   during their execution.

   > [!NOTE]
   > **Note:** Setting the default dataset and then changing the pipeline's region invalidates the dataset selection. Changing the project can also invalidate the dataset selection. If a given dataset doesn't exist in the selected project, it is created.

### Notebook options

To add a notebook to your pipeline, do the following in the **Notebook options**
section:

1. In the **Runtime template field**, either accept the default notebook
   runtime, or search for and select an existing runtime.

   - To view specifications for the default runtime, click the adjacent arrow.
   - To create a new runtime, see [Create a runtime template](https://docs.cloud.google.com/colab/docs/create-runtime-template).

   > [!NOTE]
   > **Note:** A notebook runtime template must be located in the same region as the pipeline that specifies it.

   > [!NOTE]
   > **Note:** When you include a notebook in a BigQuery pipeline, you can't change the network of the Vertex AI runtime instance. The runtime is restricted to the default network, and selecting a different network isn't supported.

2. In the **Cloud Storage bucket** field, click **Browse**
   and select or create a Cloud Storage bucket for storing the output
   of notebooks in your pipeline.

3. Follow the steps in
   [Add a principal to a bucket-level policy](https://docs.cloud.google.com/storage/docs/access-control/using-iam-permissions#bucket-add)
   to add your custom Dataform service account as a principal to the
   Cloud Storage bucket that you plan to use for storing output of
   scheduled pipeline runs, and grant the
   [Storage Admin role](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.admin)
   (`roles/storage.admin`) to this principal.

   The selected custom Dataform service account must be granted the
   Storage Admin IAM role on the selected bucket.

## Add a pipeline task

To add a task to a pipeline, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. To add a code asset, select one of the following options:

   ### SQL query

   1. Click **Add task** , and then select **Query**.
      You can either create a new query or import an existing one.

   2. Optional: In the **Query task details** pane, in the
      **Run after** menu, select a task to precede your query.

   **Create a new query**
   1. Click the
      arrow menu next to **Edit Query** and
      select either **In context** or **In new tab**.

   2. Search for an existing query.

   3. Select a query name and then press **Enter**.

   4. Click **Save**.

   5. Optional: To rename the query,
      click the query name on the pipeline pane,
      click **Edit Query**, click the existing query name at the top of the
      screen, and then type a new name.

   **Import an existing query**
   1. Click the
      arrow menu next to **Edit Query** and
      click **Import a copy**.

   2. Search for an existing query to import or select an existing
      query from the search pane. When you import a
      query, the original remains unchanged because the query's source
      file is copied into the pipeline.

   3. Click **Edit** to open the imported query.

   4. Click **Save**.

   ### Notebook

   1. Click **Add task** , and then select **Notebook** .
      You can either create a new notebook or import an existing one.
      To change settings for notebook runtime templates, see
      [Notebook options](https://docs.cloud.google.com/bigquery/docs/create-pipelines#notebook_options).

   2. Optional: In the **Notebook task details** pane, in the
      **Run after** menu, select a task to precede your notebook.

   **Create a new notebook**
   1. Click the
      arrow menu next to **Edit Notebook** and
      select either **In context** or **In new tab**.

   2. Search for an existing notebook.

   3. Select a notebook name and then press **Enter**.

   4. Click **Save**.

   5. Optional: To rename the notebook,
      click the notebook name on the pipeline pane,
      click **Edit Notebook**, click the existing notebook name at the top of
      the screen, and then type a new name.

   **Import an existing notebook**
   1. Click the
      arrow menu next to **Edit Notebook** and
      click **Import a copy**.

   2. Search for an existing notebook to import or select an existing
      notebook from the search pane. When you import a
      notebook, the original remains unchanged because the notebook's
      source file is copied into the pipeline.

   3. To open the imported notebook, click **Edit**.

   4. Click **Save**.

   ### Data preparation

   1. Click **Add task** , and then select **Data preparation**.
      You can either create a new data preparation or import an existing one.

   2. Optional: In the **Data preparation task details** pane, in the
      **Run after** menu, select a task to precede your data preparation.

   **Create a new data preparation**
   1. Click the
      arrow menu adjacent to **Edit Data preparation** and
      select either **In context** or **In new tab**.

   2. Search for an existing data preparation.

   3. Select a data preparation name and press **Enter**.

   4. Click **Save**.

   5. Optional: To rename the data preparation, click the data preparation
      name on the pipeline pane, click **Edit Data preparation**, click the
      name at the top of the screen, and enter a new name.

   **Import an existing data preparation**
   1. Click the
      arrow drop-down menu next to **Edit Data preparation** and
      click **Import a copy**.

   2. Search for an existing data preparation to import or select an existing
      data preparation from the search pane. When you import a data
      preparation, the original remains unchanged because the data
      preparation's source file is copied into the pipeline.

   3. To open the imported data preparation, click **Edit**.

   4. Click **Save**.

   ### Table


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

   <br />

   > [!NOTE]
   > **Note:** To provide feedback or request support, contact [dataform-preview-support@google.com](mailto:dataform-preview-support@google.com).

   1. Click **Add task** , and then select **Table**.

   2. In the **Create new** pane, select **Table** or **Incremental table**.

   3. Verify the default project for the table, or select a new project.

   4. Verify the default dataset for the table, or select a new dataset.

   5. Enter a name for the table.

   6. In the **Table task details** pane, click **Open** to open the task.

   7. Configure the task using the settings in
      **Details \> Configuration** or in the `config` block of the
      code editor for the table.

      For metadata changes, use the **Configuration** tab. This tab lets
      you edit a specific value in the `config` block from the code editor,
      such as a string or an array, that is formatted like a JavaScript
      object. Using this tab helps you avoid syntax errors and verify that
      your settings are correct.

      Optional: In the **Run after** menu, select a task to precede your
      table.

      You can also define the metadata for your pipeline task in the
      `config` block in the editor. For more information, see
      [Creating tables](https://docs.cloud.google.com/dataform/docs/reference/sample-scripts#creating_tables).

      The editor validates your code and displays the validation status.

      > [!NOTE]
      > **Note:** When you use JavaScript functions as values in the `config` block, you can't edit the JavaScript functions on the **Configuration** tab.

   8. In **Details \> Compiled queries**, view the SQL compiled
      from the SQLX code.

   9. Click **Run** to run the SQL in your pipeline.

   10. In **Query results**, inspect the data preview.

   ### View


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

   <br />

   > [!NOTE]
   > **Note:** To provide feedback or request support, contact [dataform-preview-support@google.com](mailto:dataform-preview-support@google.com).

   1. Click **Add task** , and then select **View**.

   2. In the **Create new** pane, select **View** or **Materialized view**.

   3. Verify the default project for the view, or select a new project.

   4. Verify the default dataset for the view, or select a new dataset.

   5. Enter a name for the view.

   6. In the **View task details** pane, click **Open** to open the task.

   7. Configure the task using the settings in
      **Details \> Configuration** or in the `config` block of the
      code editor for the view.

      For metadata changes, use the **Configuration** tab. This tab lets
      you edit a specific value in the `config` block from the code editor,
      such as a string or an array, that is formatted like a JavaScript
      object. Using this tab helps you avoid syntax errors and verify that
      your settings are correct.

      Optional: In the **Run after** menu, select a task to precede your
      view.

      You can also define the metadata for your pipeline task in the
      `config` block in the editor. For more information, see
      [Creating a view with Dataform core](https://docs.cloud.google.com/dataform/docs/reference/sample-scripts#create-view).

      The editor validates your code and displays the validation status.

      > [!NOTE]
      > **Note:** When you use JavaScript functions as values in the `config` block, you can't edit the JavaScript functions on the **Configuration** tab.

   8. In **Details \> Compiled queries**, view the SQL compiled
      from the SQLX code.

   9. Click **Run** to run the SQL in your pipeline.

   10. In **Query results**, inspect the data preview.

## Edit a pipeline task

To edit a pipeline task, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click the selected task.

5. To change the preceding task, in the **Run after** menu, select a task that
   will precede your task.

6. To edit the contents of the selected task, click **Edit**.

7. In the new tab that opens, edit the task contents, and then save changes to
   the task.

## Delete a pipeline task

To delete a task from a pipeline, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click the selected task.

5. In the **Task details** pane, click
   Delete **Delete**.

## Share a pipeline

> [!IMPORTANT]
> **Important:** If you enhance security by setting the `enable_private_workspace` field [(Preview)](https://cloud.google.com/products#product-launch-stages) to `true` in the [`projects.locations.updateConfig` Dataform API method](https://docs.cloud.google.com/dataform/reference/rest/v1beta1/projects.locations/updateConfig), only the pipeline creator can read and write code in that pipeline. For more information, see [Enable private workspaces](https://docs.cloud.google.com/dataform/docs/access-control#enable-private-workspaces).

> [!NOTE]
> **Note:** You can share a pipeline but not a task within the pipeline.

To share a pipeline, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click **Share** , and then select **Manage permissions**.

5. Click **Add user/group**.

6. In the **New principals** field, enter the name of at least one user or group.

7. For **Assign Roles**, select a role.

8. Click **Save**.

## Share a link to a pipeline

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click **Share** , and then select **Share link**. The URL for your pipeline
   is copied to your computer's clipboard.

## Run a pipeline

When running a pipeline, you can choose to run all the tasks in the pipeline,
manually select specific tasks to run, or run tasks with selected tags.

### Run all the tasks in a pipeline

To manually run the current version of a pipeline, select one of the
following options:

### Console

<br />

To run all the tasks in a pipeline, do the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click **Run**
   \> **Run all tasks** . If you selected
   **Run with my user credentials** for your
   [authentication](https://docs.cloud.google.com/bigquery/docs/create-pipelines#create_a_pipeline), you must
   [authorize your Google Account](https://docs.cloud.google.com/bigquery/docs/create-pipelines#authorize-google-account)
   ([Preview](https://cloud.google.com/products#product-launch-stages)).

5. Optional: To inspect the run,
   [view past manual runs](https://docs.cloud.google.com/bigquery/docs/manage-pipelines#view-manual-runs).

### API

> [!NOTE]
> **Note:** The Dataform API doesn't support user credentials for pipeline runs. You must select a service account in the **Authentication** section of your pipeline settings to use the API.

To run a pipeline manually, compile the default workspace and use the
compilation result to create a workflow invocation.

1. To create a compilation result for the default workspace, use the
   [`projects.locations.repositories.compilationResults.create` method](https://docs.cloud.google.com/dataform/reference/rest/v1/projects.locations.repositories.compilationResults/create).

   Run the API request with the following information:

       curl -X POST \
          -H "Authorization: Bearer $(gcloud auth print-access-token)" \
          -H "Content-Type: application/json" \
          -d '{
             "workspace": "projects/PROJECT_ID/locations/LOCATION/repositories/REPOSITORY_ID/workspaces/default"
          }' \
          "https://dataform.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/repositories/REPOSITORY_ID/compilationResults"

   Replace the following:
   - `LOCATION`: the Google Cloud region for your repository, for example, `us-central1`. You can find the repository location in the Google Cloud console by navigating to the **Explorer** pane, selecting the pipeline, opening the **Settings** tab, and clicking **Open pipeline in Dataform** . The location is in the URL in the format of `/locations/LOCATION/`.
   - `PROJECT_ID`: the unique identifier of your Google Cloud project.
   - `REPOSITORY_ID`: the unique identifier for your Dataform repository, for example, `my-secure-repo`. You can find the repository ID in the Google Cloud console by navigating to the **Explorer** pane, selecting the pipeline, opening the **Settings** tab, and viewing the **Dataform repository ID** field.
2. In the response body, locate the `name` field and copy its value,
   for example,
   `projects/my-project/locations/us-central1/repositories/my-repo/compilationResults/12345-67890`.

3. Trigger the pipeline run using the
   [`projects.locations.repositories.workflowInvocations.create` method](https://docs.cloud.google.com/dataform/reference/rest/v1/projects.locations.repositories.workflowInvocations/create).

   Run the API request with the following information:

       curl -X POST \
          -H "Authorization: Bearer $(gcloud auth print-access-token)" \
          -H "Content-Type: application/json" \
          -d '{
             "compilationResult": "COMPILATION_RESULT"
          }' \
          "https://dataform.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/repositories/REPOSITORY_ID/workflowInvocations"

   Replace the following:
   - `COMPILATION_RESULT`: the full resource name of the compilation result that you copied in the previous step.
   - `LOCATION`: the Google Cloud region for your repository, for example, `us-central1`.
   - `PROJECT_ID`: the unique identifier of your Google Cloud project.
   - `REPOSITORY_ID`: the unique identifier for your Dataform repository, for example, `my-secure-repo`.

### Run selected tasks in a pipeline

To run selected tasks in a pipeline, do the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click **Run**
   \> **Select tasks to run**.

5. In the **Run** pane, in the **Authentication** section, authorize the
   execution with the user credentials for your Google Account or a service
   account.

   - To use the user credentials for your Google Account ([Preview](https://cloud.google.com/products#product-launch-stages)), select **Run with user credentials**.
   - To use a custom service account, select
     **Run with selected service account**, and then select a custom
     service account.

     > [!NOTE]
     > **Note:** To see service accounts in the menu, you must have the `iam.serviceAccounts.list` permission at the project level, which is available in the [View Service Accounts role](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountViewer) (`roles/iam.serviceAccountViewer`). If you don't have this permission, you can select the service account by clicking **Enter manually** and entering the service account ID.

     If you need to create a service account, click **New service account**.
6. Ensure **Selection of tasks** is selected.

7. In the **Select tasks to run** menu, search for specific tasks
   and select the tasks that you want to run.

   The **Tasks** table lists the tasks that you've selected. Click a task name
   to open it directly in the SQL editor.
8. Optional: Configure the following execution options:

   - **Include dependencies**: select this option to run the selected tasks and their dependencies.
   - **Include dependents**: select this option to run the selected tasks and their transitive downstream dependents.
   - **Run with full refresh**: select this option to rebuild all tables from scratch.
   - **Run as interactive job with high priority (default)** : select this option to set the BigQuery query job priority. By default, BigQuery runs queries as [interactive query jobs](https://docs.cloud.google.com/bigquery/docs/running-queries#interactive-batch), which are intended to start running as quickly as possible. Clearing this option runs the queries as [batch query jobs](https://docs.cloud.google.com/bigquery/docs/running-queries#interactive-batch), which have lower priority.
9. Click **Run** . If you selected **Run with user credentials**
   for your authentication method, you must
   [authorize your Google Account](https://docs.cloud.google.com/bigquery/docs/create-pipelines#authorize-google-account)
   ([Preview](https://cloud.google.com/products#product-launch-stages)).

10. Optional: To inspect the run,
    [view past manual runs](https://docs.cloud.google.com/bigquery/docs/manage-pipelines#view-manual-runs).

### Run tasks with selected tags in a pipeline

To run tasks with selected tags in a pipeline, do the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane**
   to open the pane.
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click
   **Run**
   \> **Run by tag**, then do either of the following:

   - Click a tag that you want to run.
   - Click **Select tags to run**.
5. In the **Run** pane, in the **Authentication** section, authorize the execution
   with the user credentials for your Google Account or a service account.

   - To use the user credentials for your Google Account ([Preview](https://cloud.google.com/products#product-launch-stages)), select **Run with user credentials**.
   - To use a custom service account, select
     **Run with selected service account**, and then select a custom
     service account.

     > [!NOTE]
     > **Note:** To see service accounts in the menu, you must have the `iam.serviceAccounts.list` permission at the project level, which is available in the [View Service Accounts role](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountViewer) (`roles/iam.serviceAccountViewer`). If you don't have this permission, you can select the service account by clicking **Enter manually** and entering the service account ID.

     If you need to create a service account, click **New service account**.
6. Ensure **Selection of tags** is selected.

7. In the **Select tags to run** menu, search for specific tags and select the
   tags that you want to run.

   The **Tasks** table lists the tasks that you've selected. Click a task name
   to open it directly in the SQL editor.
8. Optional: Configure the following execution options:

   - **Include dependencies**: select this option to run the selected tasks and their dependencies.
   - **Include dependents**: select this option to run the selected tasks and their transitive downstream dependents.
   - **Run with full refresh**: select this option to rebuild all tables from scratch.
   - **Run as interactive job with high priority (default)** : select this option to set the BigQuery query job priority. By default, BigQuery runs queries as [interactive query jobs](https://docs.cloud.google.com/bigquery/docs/running-queries#interactive-batch), which are intended to start running as quickly as possible. Clearing this option runs the queries as [batch query jobs](https://docs.cloud.google.com/bigquery/docs/running-queries#interactive-batch), which have lower priority.
9. Click **Run** . If you selected **Run with user credentials**
   for your authentication method, you must
   [authorize your Google Account](https://docs.cloud.google.com/bigquery/docs/create-pipelines#authorize-google-account)
   ([Preview](https://cloud.google.com/products#product-launch-stages)).

10. Optional: To inspect the run,
    [view past manual runs](https://docs.cloud.google.com/bigquery/docs/manage-pipelines#view-manual-runs).

### Authorize your Google Account

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
> **Note:** To request support or provide feedback for this feature, contact [dataform-preview-support@google.com](mailto:dataform-preview-support@google.com).

To authenticate the resource with your
[Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account)
user credentials, you must manually grant permission for BigQuery
pipelines to get the access token for your Google Account and access the source
data on your behalf. You can grant manual approval with the OAuth dialog
interface.

> [!NOTE]
> **Note:** Context-Aware Access (CAA) policies---including IP-based, geolocation-based, and device compliance policies---aren't supported when executing or scheduling BigQuery pipelines with user credentials for a Google Account, because the token requests originate from Google infrastructure. CAA policies block these executions unless the Dataform OAuth client ID is [exempted from the policies](https://docs.cloud.google.com/dataform/docs/troubleshooting#euc-permission-denied).

You only need to give permission to BigQuery pipelines once.

To revoke the permission that you granted, follow these steps:

1. Go to your [Google Account page](https://myaccount.google.com/).
2. Click **BigQuery Pipelines**.
3. Click **Remove access**.

> [!WARNING]
> **Warning:** Revoking access permissions prevents any future pipeline runs that this Google Account owns across all regions.

If your pipeline contains a notebook, you must also manually grant
permission for Colab Enterprise to get the access token for your
Google Account and access the source data on your behalf. You only need
to give permission once. You can revoke this permission on the
[Google Account page](https://myaccount.google.com/).

## What's next

- Learn more about [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/pipelines-introduction).
- Learn how to [manage pipelines](https://docs.cloud.google.com/bigquery/docs/manage-pipelines).
- Learn how to [schedule pipelines](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines).