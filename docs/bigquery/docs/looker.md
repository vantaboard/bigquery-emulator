# Analyze data with BI Engine and Looker

Looker is an enterprise platform for business intelligence, data
applications, and embedded analytics. Looker helps you explore, share,
and visualize your company's data so that you can make better business
decisions.

## How Looker works

Looker lets data experts at each organization describe their data using
a lightweight modeling language called LookML. LookML tells Looker
how to query data, so everyone in the organization can create easy-to-read
reports and dashboards to explore patterns of data. Looker offers
additional features for creating custom data applications and experiences.

Looker's platform works with transactional databases like Oracle and
MySQL as well as analytical datastores like BigQuery, Snowflake, Redshift,
and more. Looker lets you create consistent data models on
top of all your data with speed and accuracy. Looker offers a unified
surface to access all of an organization's data.

## Looker integration with BigQuery

Looker supports hosting in Google Cloud. Because Looker is
platform independent, it connects to data in BigQuery as well as other public
clouds.

You don't need Looker to use BigQuery. However, if your
BigQuery use case includes business intelligence, data applications, or
embedded analytics you might want to review Looker as a provider of
these services.

If you already have a Looker instance running, see the
[instructions for connecting Looker to BigQuery](https://docs.cloud.google.com/looker/docs/db-config-google-bigquery).

## Get started with Looker and BigQuery

The BI Engine seamlessly integrates with any
business intelligence (BI) tools, including Looker. For more
information, see
[BI Engine overview](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro).

## Create a BigQuery dataset

The first step is to create a BigQuery dataset to store your
BI Engine-managed table. To create your dataset, follow these
steps:

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click your project.

4. In the details pane, click
   **View actions** , and then click **Create dataset**.

5. On the **Create dataset** page, do the following:

   - For **Dataset ID** , enter `biengine_tutorial`.
   - For **Data location** , choose **us (multiple regions in United
     States)** , the [multi-region
     location](https://docs.cloud.google.com/bigquery/docs/locations#multi-regions) where public datasets
     are stored.

   - For this tutorial, you can select **Enable table expiration**, and then
     specify the number of days before the table expires.

     ![Create dataset page](https://docs.cloud.google.com/static/bigquery/images/create-dataset-biengine.png)
6. Leave all of the other default settings in place and click **Create dataset**.

## Create a table by copying data from a public dataset

This tutorial uses a dataset available through the
[Google Cloud Public Dataset Program](https://docs.cloud.google.com/bigquery/public-data). Public datasets
are datasets that BigQuery hosts for you to access and integrate
into your applications.

In this section, you create a table by copying data from the
[San Francisco 311 service requests](https://console.cloud.google.com/marketplace/details/san-francisco-public-data/sf-311?filter=solution-type:dataset)
dataset. You can explore the dataset by using the
[Google Cloud console](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=san_francisco_311&page=dataset).

### Create your table

To create your table, follow these steps:

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, search for the `san_francisco_311` dataset.

4. Click the dataset, and then click **Overview \> Tables**.

5. Click the `311_service_requests` table.

6. In the toolbar, click **Copy**.

   ![Highlight of the copy option.](https://docs.cloud.google.com/static/bigquery/images/looker-311-table-copy.png)
7. In the **Copy table** dialog, in the **Destination** section, do the
   following:

   - For **Project** , click **Browse**, and then select your project.
   - For **Dataset** , select **biengine_tutorial**.
   - For **Table** , enter `311_service_requests_copy`.

     ![The copy table window with destination options](https://docs.cloud.google.com/static/bigquery/images/copy-311-table.png)
8. Click **Copy**.

9. Optional: After the copy job is complete, verify the table contents by expanding
   **`PROJECT_NAME` \> biengine_tutorial** and
   clicking **311_service_requests_copy \> Preview** . Replace
   **`PROJECT_NAME`** with name of your Google Cloud project
   for this tutorial.

## Create your BI Engine reservation

1. In the Google Cloud console, under **Administration** go to the
   **BI Engine** page.

   [Go to the BI Engine page](https://console.cloud.google.com/bigquery/admin/bi-engine)

   > [!NOTE]
   > **Note:** If prompted to enable **BigQuery Reservation API** , click **Enable**.

2. Click **Create reservation**.

3. On the **Create Reservation** page, configure your BI Engine
   reservation:

   - In the **Project** list, verify your Google Cloud project.
   - In the **Location** list, select a location. The location should match the [location of the datasets](https://docs.cloud.google.com/bigquery/docs/locations) that you're querying.
   - Adjust the **GiB of Capacity** slider to the amount of memory capacity
     that you're reserving. The following example sets the capacity to
     2 GiB. The maximum is 250 GiB.

     ![BI Engine capacity location](https://docs.cloud.google.com/static/bigquery/images/step-1.png)
4. Click **Next**.

5. In the **Preferred Tables** section, optionally specify tables for
   acceleration with BI Engine. To find table names, do the
   following:

   1. In the **Table Id** field, type part of the name of the table that you want accelerated by BI Engine---for example, `311`.
   2. From the list of suggested names, select your table names.

      Only specified tables are eligible for acceleration. If no preferred
      tables are specified, all project queries are eligible for acceleration.
6. Click **Next**.

7. In the **Confirm and submit** section, review the agreement.

8. If you accept the terms of agreement, click **Create**.

After you confirm your reservation, the details are displayed on the
**Reservations** page.

![Confirmed reservation](https://docs.cloud.google.com/static/bigquery/images/reservation.png)

## Connect using Looker

> [!IMPORTANT]
> **Shortcut:** If you already have a Looker model using a BigQuery dataset with a service account, in a project that is BI Engine-enabled, then no additional configuration is required.

The following instructions show you how to set up Looker with
BigQuery.

1. Log in to Looker as an administrator.
2. In the Looker documentation about BigQuery,
   complete the following sections:

   1. [Creating a service account](https://docs.cloud.google.com/looker/docs/db-config-google-bigquery#creating_a_service_account_and_downloading_the_json_credentials_certificate).
   2. [Configure an OAuth for a BigQuery connection in Looker](https://docs.cloud.google.com/looker/docs/db-config-google-bigquery#configuring_oauth_for_a_bigquery_connection).

   > [!NOTE]
   > **Note:** Ensure that the service account you create uses the same billing project as the project for which you enabled a BI Engine reservation.

3. Click the **Develop** tab and select **Development Mode**.

4. Generate a LookML model and project for your dataset. For more
   information, see the
   [instructions for connecting Looker to your database](https://docs.cloud.google.com/looker/docs/connecting-to-your-db).

5. Using the **Explore** menu, navigate to an explore associated with the new
   model file name **Explore 311_service_requests_copy** (or whatever you named
   your explore).

You have successfully connected Looker to BigQuery.
You can use the System Activity feature in Looker to generate a
Looker usage report and analyze the performance of your queries
against BigQuery-specific performance metrics.
To explore various BigQuery BI Engine query performance metrics, see [BigQuery BI Engine metrics](https://docs.cloud.google.com/looker/docs/query-performance-metrics#bigquery_bi_engine_metrics).

## Clean up

To avoid incurring charges to your Google Cloud account for the resources used
in this quickstart, you can delete the project, delete the
BI Engine reservation, or both.

### Delete the project

The easiest way to eliminate billing is to delete the project that you created
for the tutorial.

To delete the project:

> [!CAUTION]
> **Caution:** Caution: Deleting a project has the following effects:

- **Everything in the project is deleted.** If you used an existing project for this tutorial, when you delete it, you also delete any other work you've done in the project.
- **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an **appspot.com** URL, delete selected resources inside the project instead of deleting the whole project. If you plan to explore multiple tutorials and quickstarts, reusing projects can help you avoid exceeding project quota limits.

1. In Google Cloud console, go to the **Manage resources** page.

   [Go to the BI Engine page](https://console.cloud.google.com/cloud-resource-manager)
2. In the project list, select the project that you want to delete, and then
   click **Delete**.

3. In the dialog, type the project ID, and then click **Shut down** to delete
   the project.

### Delete the reservation

Alternatively, if you intend to keep the project, then you can avoid additional
BI Engine costs by deleting your capacity reservation.

To delete your reservation, follow these steps:

1. In the Google Cloud console, under **Administration** go to the
   **BI Engine** page.

   [Go to the BI Engine page](https://console.cloud.google.com/bigquery/admin/bi-engine)

   > [!NOTE]
   > **Note:** If prompted to enable **BigQuery Reservation API** , click **Enable**.

2. In the **Reservations** section, locate your reservation.

3. In the **Actions** column, click the icon to the right of your
   reservation and choose **Delete**.

4. In the **Delete reservation?** dialog, enter **Delete** and then
   click **DELETE**.

## What's next

There are many additional options related to administering
Looker, customizing its data model, and exposing data to users.
For more information, see the following resources:

- [Looker documentation](https://docs.cloud.google.com/looker/docs)
- [Looker best practices](https://docs.cloud.google.com/looker/docs/best-practices/home)
- [Looker training](https://www.cloudskillsboost.google/journeys/28)