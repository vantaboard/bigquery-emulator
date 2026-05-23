# Schedule data preparations

This document describes how to schedule data preparation pipelines
and perform manual runs.

Data preparations are powered by [Dataform](https://docs.cloud.google.com/dataform/docs/overview).
Each data preparation schedule is run using your Google Account user credentials
or a
[custom service account](https://docs.cloud.google.com/dataform/docs/access-control#about-service-accounts)
that you select when you configure the schedule or test run.

Changes you make to the data preparation steps aren't automatically saved. You
must save and deploy the changes before they can be executed with a schedule.
Schedules always run the latest deployed version of your data preparation and
exclude any undeployed changes you might be developing.

## Before you begin

Before you begin, [create a data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions).

### Required roles

To authorize a data preparation with a service account when
[manually running the data preparation in development](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#run-undeployed-manually)
or
[scheduling the data preparation](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#create-schedule),
you must grant roles to the service account that you plan to use for
executing the data preparation runs. For more information, see
[Give access to the Dataform service account](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations#dataform-service-account-iam).

To schedule data preparations, do the following:

- Ask your administrator to grant you the [Service Account User role](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountUser) (`roles/iam.serviceAccountUser`) on the custom service account.
- Grant the [Service Account User role](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountUser) (`roles/iam.serviceAccountUser`) and the [Service Account Token Creator role](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountTokenCreator) (`roles/iam.serviceAccountTokenCreator`) to the default Dataform service agent on the custom service account.

To enhance security for scheduling, see
[Implement enhanced scheduling permissions](https://docs.cloud.google.com/dataform/docs/access-control#enhanced-scheduling-permissions).

## Develop a data preparation

As you develop a data preparation, you can manually run the steps and inspect
the output before you deploy the changes to production. You can test the current
version you're developing on your data, while
BigQuery continues to run the latest deployed version, according to a
[schedule](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#create-schedule). Before you can perform the run, you must
[configure the destination](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#add-or-change-destination),
and fix any validation errors.

### Manually run a data preparation in development

To test your data preparation steps and validate the results in your destination
table, run the data preparation manually from the data preparation editor:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click
   **Data preparations**.

4. Click the name of the data preparation that you want to run.

5. In the data preparation editor toolbar, click **More \> Configure
   run now experience**.

6. In the **Authentication** section, authorize the data preparation with
   your Google Account user credentials or a service account.

   - To use your Google Account user credentials ([Preview](https://cloud.google.com/products#product-launch-stages)), select **Execute with my user credentials**. This is the default option.
   - To use a service account, select **Execute with selected service account** , and then select a service account. If the service account needs additional permissions, grant it the required roles by clicking **Grant all**.

   > [!NOTE]
   > **Note:** If your data preparation uses Google Drive as a data source, you must select **Execute with selected service account**. End-user credentials are not supported for this operation. You must also share the Google Drive file with the service account.

7. Click **Save**.

8. Fix any validation errors that appear.

9. From the data preparation editor toolbar, click **Run**.

10. In the **Run now** dialog, click **Confirm** to acknowledge that this
    manual run writes data to a destination table, which you might also be
    using for scheduled runs.

    If you selected **Execute with my user credentials**
    for your authentication method, you must
    [authorize your Google Account](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#authorize-google-account)
    ([Preview](https://cloud.google.com/products#product-launch-stages)).

    The run then executes your steps and loads the output to the
    destination.
11. Optional: After the run is complete, you can view the details about the
    execution in the **Executions** pane.

## Deploy a data preparation

To schedule runs for a version of your data preparation, you must first deploy
it. Schedules run the most recently deployed version.

To deploy a data preparation, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click
   **Data preparations**.

4. Click the name of the chosen data preparation.

   The data preparation editor opens.
5. In the data preparation editor toolbar, click **Deploy**.

## Create a schedule

> [!TIP]
> **Tip:** You can also use the **Pipelines \& Connections** page to schedule a data preparation using a [streamlined, BigQuery-specific
> workflow](https://docs.cloud.google.com/bigquery/docs/pipeline-connection-page). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

To create a schedule that executes the deployed data preparation steps and
loads the prepared data into the destination table, you must first schedule a
data preparation run. To schedule the run, you must
[configure the destination](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#add-or-change-destination),
and fix any validation errors.

To create a data preparation schedule, follow these steps:

### **Explorer** pane

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click
   **Data preparations**.

4. Click the name of the data preparation that you want to schedule.

5. From the data preparation editor toolbar, click **Schedule**.

6. Enter a schedule name.

7. In the **Authentication** section, authorize the
   data preparation with your Google Account user credentials or a service
   account.

   - To use your Google Account user credentials ([Preview](https://cloud.google.com/products#product-launch-stages)), select **Execute with my user credentials**.
   - To use a service account, select **Execute with selected service account**, and then select a service account.

   > [!NOTE]
   > **Note:** If your data preparation uses Google Drive as a data source, you must select **Execute with selected service account**. End-user credentials are not supported for this operation. You must also share the Google Drive file with the service account.

8. Schedule a frequency.

9. Click **Create schedule** . If you selected **Execute with my user credentials**
   for your authentication method, you must
   [authorize your Google Account](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#authorize-google-account)
   ([Preview](https://cloud.google.com/products#product-launch-stages)).

### **Scheduling** page

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to Scheduling](https://console.cloud.google.com/bigquery/orchestration)
2. Click **Create** , and then select **Data Preparation schedule** from the menu.

3. In the **Schedule data preparation** pane, in the **Data preparation** field,
   select the data preparation that you want to schedule.

4. In the **Schedule name** field, enter a name for the schedule.

5. In the **Authentication** section, authorize the
   data preparation with your Google Account user credentials or a service
   account.

   - To use your Google Account user credentials ([Preview](https://cloud.google.com/products#product-launch-stages)), select **Execute with my user credentials**.
   - To use a service account, select **Execute with selected service account**, and then select a service account.

   > [!NOTE]
   > **Note:** If your data preparation uses Google Drive as a data source, you must select **Execute with selected service account**. End-user credentials are not supported for this operation. You must also share the Google Drive file with the service account.

6. In the **Schedule frequency** section, do the following:

   1. In the **Repeats** menu, select the frequency of data preparation runs.
   2. In the **At time** field, enter the time for scheduled data preparation runs.
   3. In the **Timezone** menu, select the timezone for the schedule.
7. Click **Create schedule** . If you selected **Execute with my user credentials**
   for your authentication method, you must
   [authorize your Google Account](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#authorize-google-account)
   ([Preview](https://cloud.google.com/products#product-launch-stages)).

## Authorize your Google Account

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

Changing the data preparation schedule owner by updating credentials
also requires manual approval if the new Google Account owner has never
created a schedule before.

## Manually run a scheduled data preparation

When you manually run a data preparation in a selected schedule,
BigQuery executes the data preparation once,
independently from the schedule.

To manually run a scheduled data preparation, follow these steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to Scheduling](https://console.cloud.google.com/bigquery/orchestration)
2. Click the name of the selected data preparation schedule.

3. On the **Schedule details** page, click **Run**.

## View schedules

You can view data preparation schedules from the data preparation editor or the
**Scheduling** page.

### Data preparation editor

To view the schedule for a data preparation, follow these steps:

1. In the data preparation editor toolbar, click schedule **View schedule**.
2. Optional: To view the schedule history, click **View past executions**.

### **Scheduling** page

To view all data preparation schedules in your project, follow these steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to Scheduling](https://console.cloud.google.com/bigquery/orchestration)
2. Optional: To view the run history and details of a selected schedule,
   click the name of the schedule. History of manual runs is not shown.

## Edit a schedule

You can edit a schedule from the data preparation editor or the
**Scheduling** page.

### Data preparation editor

To edit a schedule, follow these steps:

1. In the data preparation editor toolbar, click schedule **View schedule**.
2. In the **Schedule data preparation** dialog, click **Edit** and then update the schedule.
3. Click **Update schedule**.

### **Scheduling** page

To edit a schedule, follow these steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to Scheduling](https://console.cloud.google.com/bigquery/orchestration)
2. Click the name of the selected data preparation schedule.

3. On the **Schedule details** page, click **Edit**.

4. Click **View schedule**.

5. In the **Schedule data preparation** dialog, click **Edit** and then
   update the schedule.

6. Click **Update schedule**.

## Delete a schedule

To permanently delete a schedule for a selected data preparation, follow these
steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to Scheduling](https://console.cloud.google.com/bigquery/orchestration)
2. In the row that contains the schedule, click
   more_vert
   **Actions \> Delete**.

## What's next

- Learn how to [create data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions).
- Learn more about [managing data preparations](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations).