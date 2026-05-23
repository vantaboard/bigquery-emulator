# Use third party transfers

Third party transfers for BigQuery Data Transfer Service allow you to automatically
schedule and manage recurring load jobs for external data sources such as
Salesforce CRM, Adobe Analytics, and Facebook Ads.

## Before you begin

Before you create a third party data transfer:

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the data.
- Consult the documentation for your third party data source to ensure you have configured any permissions necessary to enable the transfer.
- If you intend to set up transfer run notifications for Pub/Sub, you must have `pubsub.topics.setIamPolicy` permissions. Pub/Sub permissions are not required if you just set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

### Required BigQuery roles


To get the permissions that
you need to create a BigQuery Data Transfer Service data transfer,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role on your project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create a BigQuery Data Transfer Service data transfer. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create a BigQuery Data Transfer Service data transfer:

- BigQuery Data Transfer Service permissions:
  - `bigquery.transfers.update`
  - `bigquery.transfers.get`
- BigQuery permissions:
  - `bigquery.datasets.get`
  - `bigquery.datasets.getIamPolicy`
  - `bigquery.datasets.update`
  - `bigquery.datasets.setIamPolicy`
  - `bigquery.jobs.create`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information, see [Grant `bigquery.admin` access](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#grant_bigqueryadmin_access).

## Limitations

Third party transfers are subject to the following limitations:

- You must create or update a third party transfer using the Google Cloud console.
- You cannot configure or update a third party transfers using the bq command-line tool.

## Set up a third party data transfer

To create a third party data transfer by using the Google Cloud console:

1. Go to the Google Cloud Marketplace.

   [Go to the Google Cloud Marketplace](https://console.cloud.google.com/marketplace/browse?filter=category:data-transfer-services)
2. Click the appropriate third party provider.

3. On the documentation page for the third party provider, click
   **Enroll**. The enrollment process may take a moment.

4. After the enrollment is complete, click **Configure Transfer**.

5. On the **Create Transfer** page:

   - For **Source** , choose the appropriate third party data source. You can
     click **Explore Data Sources** to see the list of third party providers
     in the Google Cloud Marketplace.

     ![Transfer source](https://docs.cloud.google.com/static/bigquery/images/transfer-source.png)
   - For **Display name** , enter a name for the transfer such as `My Transfer`.
     The transfer name can be any value that allows you to easily identify the
     transfer if you need to modify it later.

     ![Transfer name](https://docs.cloud.google.com/static/bigquery/images/transfer-name.png)
   - For **Schedule** , leave the default value (**Start now** ) or click
     **Start at a set time**.

     - For **Repeats**, choose an option for how often to run the transfer.
       Options include:

       - Daily (default)
       - Weekly
       - Monthly
       - Custom
       - On-demand

       If you choose an option other than Daily, additional options are
       available. For example, if you choose Weekly, an option appears for
       you to select the day of the week.
     - For **Start date and run time** , enter the date and time to start the
       transfer. If you choose **Start now**, this option is disabled.

       ![Transfer schedule](https://docs.cloud.google.com/static/bigquery/images/transfer-schedule-daily.png)
   - For **Destination dataset**, choose the dataset you created to store your
     data.

     ![Transfer dataset](https://docs.cloud.google.com/static/bigquery/images/transfer-dataset.png)
   - (Optional) In the **Notification options** section:

     - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
     - For **Select a Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** to create one. This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
6. Click **Connect Source**.

   > [!NOTE]
   > **Note:** If you did not select a destination dataset, clicking **Connect
   > Source** produces the following error: `A selected destination dataset is
   > required before connecting to the source.`

7. When prompted, click **Accept** to give the BigQuery Data Transfer Service permission
   to connect to the data source and to manage your data in
   BigQuery.

8. Follow the instructions in the subsequent pages to configure the connection
   to your external data source.

9. After you complete the configuration steps, click **Save**.

## Troubleshoot third party transfer setup

If you are having issues setting up your transfer, consult the appropriate third
party vendor. Contact information is available on the transfer's documentation
page in the Google Cloud Marketplace.

## Query your data

When your data is transferred to BigQuery, the data is
written to ingestion-time partitioned tables. For more information, see
[Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

## What's next

- For an overview of BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Working with transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).