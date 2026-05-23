# Use Data Apps in BigQuery and Data Studio

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
> **Note:** To provide feedback or to ask questions, contact [colab-enterprise-feedback@google.com](mailto:colab-enterprise-feedback@google.com).

Colab Data Apps let you transform your data analyses from
Colaboratory notebooks into polished, interactive applications. Instead
of sharing code or static reports, you can now build shareable experiences that
include interactive visualizations, data tables, and machine learning
inferences.

Data Apps provide the following:

- **Self-service insights**. Business users can adjust parameters such as date ranges or filters to see the data that they need without editing code.
- **No setup for app viewers**. Consumers access your app using a URL. They don't need to navigate the Google Cloud console or run notebooks.
- **Flexibility**. Integrate any Python visualization library or widget to build custom, complex Data Apps with ease. To save time, use the integrated agent to generate the code for you.
- **Managed life cycle**. Administrators and authors retain control over sharing, versions, and resource usage.

## Before you begin

1.


   Enable the BigQuery and Dataform APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,dataform.googleapis.com)

   For new projects, the BigQuery API is
   automatically enabled.

### Required roles


To get the permissions that
you need to create data apps,

ask your administrator to grant you the
following IAM roles on the project:

- [BigQuery Read Session User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.readSessionUser) (`roles/bigquery.readSessionUser`)
- [BigQuery Studio User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.studioUser) (`roles/bigquery.studioUser`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

If you're new to Colab Enterprise in BigQuery, see
the required permissions on the [Create notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks#required_permissions)
page.

## Limitations

Data Apps are subject to the following limitations:

- The first time you open an app, it can take from two to five minutes to
  load, depending on its complexity.

- Interactivity sessions last for 30 minutes. After 30 minutes, apps lose
  their connection to the kernel and become static. To initiate a new
  30-minute session, you can reload the page or refresh the data.

- Regardless of which cells are visible in the app, all cells are run in
  order from first to last. If non-visible cells are running and consuming
  kernel resources, the app might appear unresponsive until kernel resources
  are freed.

- You can't use a service account or End User Credentials (EUC) for data
  access or for viewing Data Apps.

## Components of Data Apps

The components you can add to your Data Apps are derived from the
underlying notebook. If you're working with an existing notebook, you can add
any of its existing cells to your Data Apps.

You can add to Data Apps any supported [cell types](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) that you
can create in a Colab notebook, including SQL cells, code
cells, text cells, and visualization cells. You can create cell types manually,
or you can use the [Colab Data Science Agent](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent) or [Gemini](https://docs.cloud.google.com/bigquery/docs/gemini-overview)
to generate cells for you.

For information on adding different cell types to notebooks, see the following
tutorials:

- [Visualize query results](https://docs.cloud.google.com/bigquery/docs/visualize-data-colab)
- [Try BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart)

### Controls created using third-party libraries

Data Apps support controls that are created using third-party
widget libraries. For example:

- You can use the `ipywidgets` package or the `anywidget` library to add interactive controls to your notebook. The widgets can be generated by using the [Colab Data Science Agent](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent).
- At the notebook level, you can chain widget cells. You define a widget at the start of the notebook, and its output can be consumed by other cells such as visualization cells. This setup lets you set up a global filter for your Data Apps.

## Create and publish Data Apps

You can create Data Apps using an existing notebook or by
creating a new one. In the following example, you use the *Getting started with
notebooks for Python users* template to generate a notebook and to create a Data
App from it.

After you create Data Apps, you publish them to
Data Studio to turn your data analyses into shareable, interactive
experiences.

> [!NOTE]
> **Note:** Data Apps don't require Data Studio Pro.

When you publish Data Apps, you can connect them to an existing
Colab runtime, or you can create a new runtime using a
[template](https://docs.cloud.google.com/vertex-ai/docs/colab/create-runtime-template). With templates, you can size the runtime according to your
workloads, and you can set the idle shutdown time to balance costs and app
startup time.

To create and publish Data Apps, follow these steps:

1. In the Google Cloud console, go to the BigQuery **Studio**
   page.

   [Go to Studio](https://console.cloud.google.com/bigquery)
2. To open the template gallery, do one of the following:

   - In the left pane, click
     **Explorer** , expand your
     project, and then select
     **Notebook \> View actions \> Create notebook \> All templates**.

     ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

     If you don't see the left pane, click
     **Expand left pane** to
     open the pane.
   - On the **Studio** page, click **View notebook gallery**:

     ![The View notebook gallery link on the BigQuery Studio page.](https://docs.cloud.google.com/bigquery/images/template-gallery.png)
3. Click the **Getting started with notebooks for Python users** card or search
   for it in the gallery.

4. After the template opens, click **Use this template** to convert the
   template into a runnable notebook.

5. Click dashboard **Data app**.

6. Optional: In the **Components** pane, add and remove cells from the app by
   using the checkboxes:

   ![The components pane where you can add and remove cells from the app.](https://docs.cloud.google.com/bigquery/images/components-pane.png)
7. To create new components, do the following:

   1. To go back to the notebook, click arrow_back **Notebook**.
   2. To add new cells, click the drop-down arrow next to **Code** or **Text**.
   3. Click **Save**.
   4. To return to the app, click dashboard **Data app**.
8. When you're satisfied with the layout of your app, click **Publish**.

9. On the **Publish** page, enter the following:

   1. In the **Name** field, enter a name for the app. The app name doesn't
      have to be the same as the name of the notebook used to create it.

   2. In the **Runtime** section, choose **Connect to an existing runtime** ,
      and then choose the runtime from the **Runtime** list, or choose
      **Create new Runtime** , and then choose the
      appropriate template from the **Runtime template** list.

      If you choose **Connect to an existing runtime**, you must connect
      your notebook to a runtime before you publish the app.

      ![The Publish page for your app.](https://docs.cloud.google.com/bigquery/images/publish-page.png)
   3. Click **Publish**.

   4. If you're prompted to give Data Studio access to your
      Google Account, click **Allow**.

10. If you make changes to your source notebooks and your app, click
    **Publish changes** to update the app.

    Alternatively, from Data Studio you can edit the app by
    clicking more_vert **More options
    \> Edit**.

    The source notebook opens in BigQuery Studio.

## View and share Data Apps

You use Data Studio to view your Colab Data Apps and to
share your apps with others.

When you view an app, interactivity depends on the components you included.
For example, a Matplotlib chart is static, and a chart created using
Colab visualization cells or using libraries such as Plotly is
interactive.

You can interact with widgets defined in your code and use them for tasks like
filtering or changing the output. These interactions are user-specific.

To view and share your app in Data Studio, follow these steps:

1. To view your app in Data Studio, click **View Data App**.

2. To share your app, in Data Studio, click
   person_add **Share**.

3. On the **Share with people and groups** page, do the following:

   1. In the **Add people and groups** field, enter the groups and individuals
      you're granting access.

   2. Select the [access permission](https://docs.cloud.google.com/looker/docs/studio/roles-and-permissions)
      for the users and groups you specified:

      - **Editor**: users can edit the app, create and edit schedules and alerts, and share the app with others.
      - **Viewer**: users can see the app but can't edit or share it with others.
   3. Click **Send**.

4. To invite others, get a report link, or share or download the app, beside
   the **Share** option, click arrow_drop_down,
   and then choose one of the following:

   - **Invite people**
   - **Get report link**
   - **Download report**

   ![The share menu in Data Studio.](https://docs.cloud.google.com/bigquery/images/share-menu.png)

For more information on sharing assets in Data Studio, see
[Invite others to your reports](https://docs.cloud.google.com/looker/docs/studio/invite-others-to-your-reports).

## Reconnecting and refreshing Data Apps

Interactivity sessions last for 30 minutes. After 30 minutes, apps lose their
connection to the kernel and become static. To initiate a new 30-minute session,
you can reload the page or refresh the data.

To refresh the data in Data Studio, do the following:

- With your app open, click more_vert **More report actions \> Refresh data**.

## Credentials used to run cells in Data Apps

By default, Data Apps use the app creator's credentials for data
access and to render visualizations. All viewers see the rendered
Data Apps based on the author's access. Consider this outcome
before you share Data Apps that have access to sensitive data.

## Delete Data Apps

You can delete Data Apps using Data Studio.
Deleting the notebook that you used to create the app doesn't delete the app or
prevent the app from functioning.

To delete Data Apps in Data Studio, follow these
steps:

1. [Sign in to Data Studio](https://lookerstudio.google.com).

2. On the **Recent** page, locate your app.

3. At the end of the row, click more_vert
   **More options \> Remove**:

   ![The More options menu that you use to delete an app.](https://docs.cloud.google.com/bigquery/images/delete-data-app.png)

## Pricing

You are charged for running code in the notebook's runtime and for any
BigQuery [slots](https://docs.cloud.google.com/bigquery/docs/slots) that you use. For more information, see
[Colab Enterprise pricing](https://cloud.google.com/colab/pricing).