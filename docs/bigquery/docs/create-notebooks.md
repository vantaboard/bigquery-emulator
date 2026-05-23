# Create notebooks

This document describes how to create
[Colab Enterprise notebooks in BigQuery](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction).
Notebooks are
[BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio)
code assets powered by
[Dataform](https://docs.cloud.google.com/dataform/docs/overview).

## Before you begin

<br />

### Required permissions

Set the appropriate permissions to create, edit, or view notebooks.

All users with the
[Dataform Admin role](https://docs.cloud.google.com/dataform/docs/access-control#dataform.admin)
(`roles/dataform.admin`) have owner access to all notebooks created in the
project.

For more information about BigQuery Identity and Access Management (IAM),
see [Access control with IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

#### Permissions to create notebooks


To get the permissions that
you need to create and run notebooks,

ask your administrator to grant you the
following IAM roles on the project:

- [BigQuery Read Session User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.readSessionUser) (`roles/bigquery.readSessionUser`)
- [BigQuery Studio User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.studioUser) (`roles/bigquery.studioUser`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

> [!WARNING]
> **Warning:** Visibility for code assets is governed by project-level Dataform permissions. Users with the `dataform.repositories.list` permission---which is included in standard BigQuery roles such as [BigQuery Job User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.jobUser), [BigQuery Studio User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.studioUser), and [BigQuery User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user)---can see all code assets in the **Explorer** panel of the Google Cloud project, regardless of whether they created these assets or these assets were shared with them. To restrict visibility, you can create [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) that exclude the `dataform.repositories.list` permission.

> [!NOTE]
> **Note:** Users assigned the Code Creator role in a project can list the names of code assets in that project by using the Dataform API or the Dataform command-line interface (CLI).

You might also be able to get the required permissions through
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined). To see the exact permissions
that are required to create and run notebooks, expand the
**Required permissions** section:

#### Required permissions

- `bigquery.config.get`
- `bigquery.jobs.create`
- `bigquery.readsessions.create`
- `bigquery.readsessions.getData`
- `bigquery.readsessions.update`
- `resourcemanager.projects.get`
- `resourcemanager.projects.list`
- `dataform.locations.get`
- `dataform.locations.list`
- `dataform.repositories.create`

> [!NOTE]
> **Note:** Users who have the `dataform.repositories.create` permission can execute code using the default Dataform service account and all permissions granted to that service account. For more information, see [Security considerations for Dataform permissions](https://docs.cloud.google.com/dataform/docs/access-control#security-considerations-permissions).

- `dataform.repositories.list`
- `dataform.collections.create`
- `dataform.collections.list`
- `aiplatform.notebookRuntimeTemplates.apply`
- `aiplatform.notebookRuntimeTemplates.get`
- `aiplatform.notebookRuntimeTemplates.list`
- `aiplatform.notebookRuntimeTemplates.getIamPolicy`
- `aiplatform.notebookRuntimes.assign`
- `aiplatform.notebookRuntimes.get`
- `aiplatform.notebookRuntimes.list`
- `aiplatform.operations.list`

> [!NOTE]
> **Note:** When you create a notebook, BigQuery grants you the [Dataform Admin role](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.admin) (`roles/dataform.admin`) on that notebook. All users with the Dataform Admin role granted on the Google Cloud project have owner access to all the notebooks created in the project. To override this behavior, see [Grant a specific role upon resource creation](https://docs.cloud.google.com/dataform/docs/access-control#grant-specific-role).

#### Roles to edit notebooks

To edit and run notebooks, you need the following IAM
roles:

- [BigQuery Job User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.jobUser) (`roles/bigquery.jobUser`)
- [BigQuery Read Session User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.readSessionUser) (`roles/bigquery.readSessionUser`)
- [Notebook Runtime User](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.notebookRuntimeUser) (`roles/aiplatform.notebookRuntimeUser`)
- [Code Editor](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeEditor) (`roles/dataform.codeEditor`)

#### Roles to view notebooks

To view and run notebooks, you need the following IAM
roles:

- [BigQuery Job User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.jobUser) (`roles/bigquery.jobUser`)
- [BigQuery Read Session User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.readSessionUser) (`roles/bigquery.readSessionUser`)
- [Notebook Runtime User](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.notebookRuntimeUser) (`roles/aiplatform.notebookRuntimeUser`)
- [Code Viewer](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeViewer) (`roles/dataform.codeViewer`)

## Create notebooks

Use the following sections to learn how to create a notebook.

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

### Create a notebook using the notebook gallery

The notebook gallery in the Google Cloud console for BigQuery
is your central hub for discovering and using prebuilt notebook templates.

To create a notebook from a template in the notebook gallery, follow these
steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. To open the gallery, in the tab bar of the editor pane, click the

   arrow next to

   **SQL query** , and then click **Notebook \> All templates**.

3. In the notebook gallery, select a template. For example, you can select
   **Getting started with BigQuery DataFrames**.

   The new notebook opens, containing cells that show example queries against
   the `bigquery-public-data.ml_datasets.penguins` public dataset.
4. Alternatively, you can click the

   arrow next to

   **SQL query** , and then click **Notebook \> Empty notebook** ,
   **Notebook \> BigQuery template** , or
   **Notebook \> Spark template** to open these specific templates.

5. To create a runnable notebook from the template, click **Use this
   template**.

6. Optional: To view notebook details or the
   [version history](https://docs.cloud.google.com/bigquery/docs/create-notebooks#create_a_notebook_from_an_existing_notebook), add new
   comments, or reply to or get a link to an existing comment, use the
   following toolbar:

   ![Toolbar adjacent to the notebook.](https://docs.cloud.google.com/static/bigquery/images/editor-toolbar.png)

   The **Comments** toolbar feature is in
   [Preview](https://cloud.google.com/products#product-launch-stages). To
   provide feedback or request support for this feature, send an email to
   [bqui-workspace-pod@google.com](mailto:bqui-workspace-pod@google.com).
7. Optional: In the toolbar, you can use the **Reference** panel to
   preview the schema details of tables, snapshots, views, or materialized
   views, or open them in a new tab. The panel also has a list of recent and
   starred resources.

### Create a notebook from a table

To create a notebook containing a default query for a specific table, follow
these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then
   click your dataset.

4. Click **Overview \> Tables**, and find the table that you want to query.

5. Next to the table, click **Actions** ,
   and then click **Open in \> Python notebook**.

   The new notebook opens, containing cells that show example queries against
   the selected table.

### Create a notebook to explore the result set of a query

To create a notebook to explore the result set of a query, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the editor pane, run a query that generates a query result.

3. In the **Query results** pane, click **Open in \> Notebook**.

   The new notebook opens, containing cells with code to return the query SQL
   and the query results.

### Create a notebook from an existing notebook

To open any version of an existing notebook as a new notebook, follow these
steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Select a notebook.

5. Click **Version history**.

6. Click
   **View actions** next to a notebook version and then click
   **Open as new Python notebook**.

   A copy of the notebook is opened as a new notebook.

## Upload notebooks

You can upload a local notebook to use it in
BigQuery Studio. The uploaded notebook is then visible in the
BigQuery page of the Google Cloud console.

To upload a notebook, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, and then do one of the following:

   - Next to **Notebooks** , click **View actions** \> **Upload to Notebooks**.
   - Next to the Google Cloud project name, click **View actions** \> **Upload to project** \> **Notebook**.
4. In the **Upload Notebook** dialog, in the **Notebook** field,
   click **Browse**, and then select the notebook that you want to
   upload.

5. Optional: In the **Notebook name** field, edit the name of the notebook.

6. In the **Region** field, select the region where you want to upload your notebook.

7. Click **Upload**.

Your notebook can be accessed through the **Explorer** pane.

## Connect to a runtime

Use the following sections to learn how to connect a notebook to a
[Vertex AI runtime](https://docs.cloud.google.com/colab/docs/create-runtime). A runtime is a
compute resource that runs the code in your notebook. The runtime must be in the
same region as your notebook.

For more information about runtimes, see
[Runtimes and runtime templates](https://docs.cloud.google.com/colab/docs/runtimes).

> [!NOTE]
> **Note:** If you use [VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/overview), make sure you have configured [Private Google Access with VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/private-connectivity) before connecting to a runtime. Otherwise, the service returns the error `Failed to connect to Runtime Network
> projects/*projectid*/global/networks/default' was not found.`

### Connect to the default runtime

The default runtime is a preset runtime that requires minimal setup.

To connect to the default runtime, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Click the name of a notebook to open it.

5. In the notebook, click **Connect**, or run any cell in the notebook.

   It might take several minutes to connect to the default runtime if you
   don't already have an active runtime.

### Connect to a non-default runtime

If you want to use a runtime other than the default runtime, you must first
[create that additional runtime in Vertex AI](https://docs.cloud.google.com/vertex-ai/docs/colab/create-runtime).

To connect to non-default runtime, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Click the name of a notebook to open it.

5. In the notebook, click the

   drop-down next to **Connect** and then click **Connect to a runtime**.

6. Click **Connect to an existing runtime**.

7. In **Runtimes**, select the runtime to use. The runtime must be in the same
   location as the notebook.

8. Click **Connect**.

### Connect to a new runtime

To connect to a new runtime, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Click the name of a notebook to open it.

5. In the notebook, click the

   drop-down next to **Connect** and then click **Connect to a runtime**.

6. Click **Create new runtime**.

7. In **Runtime Template** , select the
   [Vertex AI runtime template](https://docs.cloud.google.com/colab/docs/create-runtime-template)
   to use.

8. In **Runtime name**, type a name for the runtime.

9. Click **Connect**.

## Understand cells

A notebook is made up of cells that you can edit. The
following types of cells are supported:

- **Text cell**: Use a text cell to add explanations and images to your notebook
  in Markdown.

- **Code cell**: Use a code cell to add Python to your notebook. You can run
  each code cell individually. A code cell can reference any variables created
  in another cell that you've already run.

- **SQL cell** : Use a [SQL cell](https://docs.cloud.google.com/colab/docs/sql-cells) to run
  GoogleSQL queries. The output of the query is automatically saved as
  a DataFrame with the same name as the title of the cell. You can run multiple
  SQL statements in a single SQL cell, but only the results of the last
  statement are saved to a DataFrame.

  You can refer to Python variables in expressions or use
  BigQuery DataFrames as tables in your query by enclosing the variable
  name in braces (`{ }`):

      # Refer to the Python variable my_threshold in a SQL expression.
      SELECT * FROM my_dataset.my_table WHERE x > {my_threshold};

      # Reference previous query results to iterate on your queries.
      SELECT * FROM {df};

- **Visualization cell** : Use a
  [visualization cell](https://docs.cloud.google.com/colab/docs/visualization-cells) to automatically
  generate a visualization of any DataFrame in your notebook.
  You can modify which columns
  are displayed and select from various chart types and aggregations. You can
  also choose custom colors, data labels, and titles.

## Grant access to notebooks

To grant other users access to a notebook, add those users to an appropriate
IAM role.

> [!IMPORTANT]
> **Important:** Users who have access to the notebook can see all output generated by code in the notebook, even if this output contains data from tables they don't have access to. To prevent saved output from being shared, [disable notebook output saving](https://docs.cloud.google.com/bigquery/docs/create-notebooks#disable_output_saving).

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Find the notebook that you want to grant access to.

5. Click
   **Open actions** next to the notebook, and then click **Share**.

6. In the **Share permissions** pane, click **Add user/group**.

7. In the **New principals** field, enter a principal.

8. In the **Role** list, select one of
   the following roles:

   - [**Code Owner**](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeOwner): Can perform any action on the notebook, including deleting or sharing it.
   - [**Code Editor**](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeEditor): Can edit the notebook.
   - [**Code Viewer**](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeViewer): Can view the notebook.

   > [!NOTE]
   > **Note:** The principal must also have the [Notebook Runtime User (`roles/aiplatform.notebookRuntimeUser`)](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.notebookRuntimeUser) and [BigQuery User (`roles/bigquery.user`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user) roles to run the notebook.

9. Optional: To view a complete list of roles and advanced sharing settings,
   click **Advanced sharing**.

10. Click **Save**.

11. To return to the notebook information page, click **Close**.

## Share notebooks

To share a notebook with other users, you can generate and share a link to
the notebook. For other users to see the notebook you share, you must
first [grant access](https://docs.cloud.google.com/bigquery/docs/create-notebooks#grant_access_to_notebooks) to the notebook.

To run a notebook, users must have access to the data that the notebook
accesses. For more information, see
[Grant access to a dataset](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_dataset).

> [!IMPORTANT]
> **Important:** Users who have access to the notebook can see all output generated by code in the notebook, even if this output contains data from tables they don't have access to. To prevent saved output from being shared, [disable notebook output saving](https://docs.cloud.google.com/bigquery/docs/create-notebooks#disable_output_saving).

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Find the notebook that you want to share. You can use the
   search feature or filters to find your notebook.

5. Click
   **View actions** next to the notebook, and then click **Share** \> **Copy link**.

6. Share the link with other users.

## Disable notebook output saving

You can prevent sharing saved notebook output
with other users who have access to the notebook file
by disabling notebook output saving.

When you disable output saving for a selected notebook,
BigQuery deletes all output saved in the notebook file
and doesn't save the output of subsequent runs.

However, users who have [access to the notebook](https://docs.cloud.google.com/bigquery/docs/create-notebooks#grant_access_to_notebooks)
can still view its output in the following ways:

- Run the notebook to view its current output. This output is not saved.
- View an archival version of the notebook and its output in revision history.

To disable saving of output for a selected notebook, follow these steps:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Click the notebook for which you want to disable saving output.

5. To expand the menu bar, click keyboard_arrow_down **Toggle header visibility**.

6. Click **Edit \> Notebook settings**.

7. In the **Notebook settings** window, select
   **Omit code cell output when saving this notebook**.

8. Click **Save**.

9. Click **Reload**.

## Resolve conflicts

If you and another user make conflicting changes in a notebook, the service
raises the error `Automatic saving failed. This file was updated remotely or
in another tab.` and provides a `Show diff` link. To resolve the conflict,
follow these steps:

1. Click the `Show diff` link. The **Review remote changes** dialog opens.
2. Optional: To compare the notebook source code, select the **Raw source** checkbox.
3. Optional: To compare the versions inline instead of in separate panes, select the **Inline diff** checkbox.
4. Review the changes and decide which to keep, revising your input if necessary.
5. Click **Save your changes**.

## Rename notebooks

To rename a notebook, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Click the notebook that you want to rename.

5. Click keyboard_arrow_down **Toggle header visibility**
   to expand the menu bar.

6. Click **File \> Rename**.

7. In the **Rename notebook** dialog, type a name for the notebook, and then
   click **Rename**.

## Troubleshooting

For more information, see [Troubleshoot Colab Enterprise](https://docs.cloud.google.com/colab/docs/troubleshooting).

## What's next

- Learn how to [manage notebooks](https://docs.cloud.google.com/bigquery/docs/manage-notebooks).
- Learn how to [schedule notebooks](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks).