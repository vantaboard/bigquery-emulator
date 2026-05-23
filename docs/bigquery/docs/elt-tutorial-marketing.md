# Build ELT pipeline for marketing analytics data

This tutorial shows you how to set up an ELT workflow that extracts, loads,
and transforms marketing analytics data in BigQuery.

A typical ELT workflow periodically extracts new customer data from your data
source and loads it into BigQuery. The unstructured data is then
processed into meaningful metrics. In this tutorial, you create an ELT
workflow by setting up a marketing analytics data transfer by using the
BigQuery Data Transfer Service. Then, you schedule Dataform to run periodic
transformations on the data.

In this tutorial, you use Google Ads as your data source, but you
can use any of the [data sources supported by the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction#supported_data_sources).

## Before you begin

<br />

### Required roles


To get the permissions that
you need to complete this tutorial,

ask your administrator to grant you the
following IAM roles on the project:

- [BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`)
- [Dataform Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.admin) (`roles/dataform.admin`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Schedule recurring data transfers

To keep BigQuery up to date with the latest marketing data from
your data source, set up recurring data transfers using the BigQuery Data Transfer Service
to extract and load data on a schedule.

In this tutorial, you use Google Ads as the example data source.
For a full list of data sources supported by the BigQuery Data Transfer Service, see
[Supported data
sources](https://docs.cloud.google.com/bigquery/docs/dts-introduction#supported_data_sources).

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , choose **Google Ads**.

4. In the **Data source details** section:

   1. For **Customer ID**, enter your Google Ads customer ID.
   2. For **Report type** , select **Standard** . The standard report includes the standard set of reports and fields as detailed in [Google Ads report
      transformation](https://docs.cloud.google.com/bigquery/docs/google-ads-transformation).
      - For **Refresh window** , enter `5`.
5. In the **Destination settings** section, for **Dataset**, select the
   dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name** , enter `Marketing tutorial`.

7. In the **Schedule options** section:

   - For **Repeat frequency** , select **Days**.
   - For **At** , enter `08:00`.
8. Click **Save**.

After you save the configuration, the BigQuery Data Transfer Service begins the data
transfer. Based on the settings in the transfer configuration, the data transfer
runs once every day at 8:00 AM UTC and extracts data from
Google Ads from the past five days.

You can [monitor ongoing transfer jobs](https://docs.cloud.google.com/bigquery/docs/dts-monitor) to check
the status of each data transfer.

## Query table data

When your data is transferred to BigQuery, the data is
written to ingestion-time partitioned tables. For more information, see
[Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

The following sections show sample queries that you can use to examine your
transferred data.

### Campaign performance

The following sample query analyzes Google Ads campaign performance for the past
30 days.

### Console

```googlesql
SELECT
  c.customer_id,
  c.campaign_name,
  c.campaign_status,
  SUM(cs.metrics_impressions) AS Impressions,
  SUM(cs.metrics_interactions) AS Interactions,
  (SUM(cs.metrics_cost_micros) / 1000000) AS Cost
FROM
  `DATASET.ads_Campaign_CUSTOMER_ID` c
LEFT JOIN
  `DATASET.ads_CampaignBasicStats_CUSTOMER_ID` cs
ON
  (c.campaign_id = cs.campaign_id
  AND cs._DATA_DATE BETWEEN
  DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY) AND DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY))
WHERE
  c._DATA_DATE = c._LATEST_DATE
GROUP BY
  1, 2, 3
ORDER BY
  Impressions DESC
```

### bq

```bash
  bq query --use_legacy_sql=false '
  SELECT
    c.customer_id,
    c.campaign_name,
    c.campaign_status,
    SUM(cs.metrics_impressions) AS Impressions,
    SUM(cs.metrics_interactions) AS Interactions,
    (SUM(cs.metrics_cost_micros) / 1000000) AS Cost
  FROM
    `DATASET.ads_Campaign_CUSTOMER_ID` c
  LEFT JOIN
    `DATASET.ads_CampaignBasicStats_CUSTOMER_ID` cs
  ON
    (c.campaign_id = cs.campaign_id
    AND cs._DATA_DATE BETWEEN
    DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY) AND DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY))
  WHERE
    c._DATA_DATE = c._LATEST_DATE
  GROUP BY
    1, 2, 3
  ORDER BY
    Impressions DESC'
```

Replace the following:

- <var translate="no">`DATASET`</var>: the name of the dataset that you created to store the transferred table
- <var translate="no">`CUSTOMER_ID`</var>: your Google Ads Customer ID.

### Count of keywords

The following sample query analyzes keywords by campaign, ad group, and keyword
status. This query uses the `KeywordMatchType` function. Keyword match types
help control which searches can trigger your ad. For more information about keyword
matching options, see
[About keyword matching options](https://support.google.com/google-ads/answer/2497836).

### Console

```googlesql
  SELECT
    c.campaign_status AS CampaignStatus,
    a.ad_group_status AS AdGroupStatus,
    k.ad_group_criterion_status AS KeywordStatus,
    k.ad_group_criterion_keyword_match_type AS KeywordMatchType,
    COUNT(*) AS count
  FROM
    `DATASET.ads_Keyword_CUSTOMER_ID` k
    JOIN
    `DATASET.ads_Campaign_CUSTOMER_ID` c
  ON
    (k.campaign_id = c.campaign_id AND k._DATA_DATE = c._DATA_DATE)
  JOIN
    `DATASET.ads_AdGroup_CUSTOMER_ID` a
  ON
    (k.ad_group_id = a.ad_group_id AND k._DATA_DATE = a._DATA_DATE)
  WHERE
    k._DATA_DATE = k._LATEST_DATE
  GROUP BY
    1, 2, 3, 4
```

### bq

```bash
  bq query --use_legacy_sql=false '
  SELECT
    c.campaign_status AS CampaignStatus,
    a.ad_group_status AS AdGroupStatus,
    k.ad_group_criterion_status AS KeywordStatus,
    k.ad_group_criterion_keyword_match_type AS KeywordMatchType,
    COUNT(*) AS count
  FROM
    `DATASET.ads_Keyword_CUSTOMER_ID` k
  JOIN
    `DATASET.ads_Campaign_CUSTOMER_ID` c
  ON
    (k.campaign_id = c.campaign_id AND k._DATA_DATE = c._DATA_DATE)
  JOIN
    `DATASET.ads_AdGroup_CUSTOMER_ID` a
  ON
    (k.ad_group_id = a.ad_group_id AND k._DATA_DATE = a._DATA_DATE)
  WHERE
    k._DATA_DATE = k._LATEST_DATE
  GROUP BY
    1, 2, 3, 4'
```

Replace the following:

- <var translate="no">`DATASET`</var>: the name of the dataset that you created to store the transferred table
- <var translate="no">`CUSTOMER_ID`</var>: your Google Ads Customer ID.

## Create a Dataform repository

After you create the data transfer configuration to transfer the latest data
from Google Ads, set up Dataform to regularly
transform your marketing analytics data. Dataform lets you
schedule regular data transformations, and it lets you define these
transformations with SQL while collaborating with other data analysts.

Create a Dataform repository to store the [SQLX queries](https://docs.cloud.google.com/dataform/docs/overview#dataform-core)
that make up your transformation code.

1. In the Google Cloud console, go to the **Dataform** page.

   [Go to Dataform](https://console.cloud.google.com/bigquery/dataform)
2. Click **Create repository**.

3. On the **Create repository** page, do the following:

   1. In the **Repository ID** field, enter `marketing-tutorial-repository`.
   2. In the **Region** list, select a region.
   3. Click **Create**.

The `marketing-tutorial-repository` repository now appears in your
Dataform repositories list.

For more information about Dataform repositories,
see [About Dataform repositories](https://docs.cloud.google.com/dataform/docs/create-repository#about-repositories).

## Create and initialize a Dataform development workspace

Create a Dataform development workspace so that you can work on
the transformation code within your repository before you commit and push your
changes to your repository.

1. In the Google Cloud console, go to the **Dataform** page.

   [Go to Dataform](https://console.cloud.google.com/bigquery/dataform)
2. Click `marketing-tutorial-repository`.

3. Click **Create development
   workspace**.

4. In the **Create development workspace** window, do the following:

   1. In the **Workspace ID** field, enter `marketing-tutorial-workspace`.
   2. Click **Create**.

   The development workspace page appears.
5. Click **Initialize workspace**.

The `marketing-tutorial-workspace` development workspace now appears in your
`marketing-tutorial-repository` repository under the **Development Workspaces**
tab, along with two example files in the `definitions` directory called
`*first_view.sqlx` and `*second_view.sqlx`.

For more information about Dataform development workspaces,
see [Overview of development workspaces](https://docs.cloud.google.com/dataform/docs/create-workspace#overview-workspaces).

## Declare your Google Ads table as table source

Connect your newly-transferred Google Ads table to Dataform
by declaring it as a data source by following these following steps:

### Create a SQLX file for data source declaration

In Dataform, you declare a data source destination by creating
a SQLX file in the `definitions/` directory:

1. In the Google Cloud console, go to the **Dataform** page.

   [Go to the Dataform page](https://console.cloud.google.com/bigquery/dataform)
2. Select `marketing-tutorial-repository`.

3. Select `marketing-tutorial-workspace`.

4. In the **Files** pane, next to `definitions/`, click the **More** menu.

5. Click **Create file**.

6. In the **Create new file** pane, do the following:

   1. In the **Add a file path** field, after `definitions/`, enter the name `definitions/googleads-declaration.sqlx`.
   2. Click **Create file**.

### Declare a data source

Edit the `definitions/googleads-declaration.sqlx` to declare a transferred
Google Ads table as a data source. This example declares the
`ads_Campaign` table as a data source:

1. In your development workspace, in the **Files** pane, click your SQLX file for data source declaration.
2. In the file, enter the following code snippet:

   ```
       config {
           type: "declaration",
           database: "PROJECT_ID",
           schema: "DATASET",
           name: "ads_Campaign_CUSTOMER_ID",
       }
   ```

## Define your transformation

Define your data transformations by create a SQLX file in the `definitions/`
directory. In this tutorial, you create a daily transformation that aggregates
metrics like clicks, impressions, costs, and conversions using a file named
`daily_performance.sqlx`.

### Create the transformation SQLX file

1. In the **Files** pane, next to `definitions/`, click the **More** menu, and then select **Create file**.
2. In the **Add a file path** field, enter `definitions/daily_performance.sqlx`.
3. Click **Create file**.

### Define the transformation SQLX file

1. In the **Files** pane, expand the `definitions/` directory.
2. Select `daily_performance.sqlx`, then enter the following query:

   ```
       config {
           type: "table",
           schema: "reporting",
           tags: ["daily", "google_ads"]
       }
       SELECT
           date,
           campaign_id,
           campaign_name,
       SUM(clicks) AS total_clicks
       FROM
           `ads_Campaign_CUSTOMER_ID`
       GROUP BY
           date,
           campaign_id,
           campaign_name
           ORDER BY
           date DESC
   ```

## Commit and push your changes

After you have made your changes in your development workspace, you can commit
and push these changes to your repository by following these steps:

1. In the `marketing-tutorial-workspace` workspace, click **Commit 1 change**.
2. In the **New commit** pane, enter a commit description in the **Add a commit
   message** field.
3. Click **Commit all changes**.
4. In the `marketing-tutorial-workspace` workspace, click **Push to default
   branch**.

After your changes are successfully pushed to your repository, the **Workspace is up to date** message appears.

## Schedule your data transformation

After you have defined your data transformation file, schedule the data
transformations.

### Create a production release

A production release in Dataform ensures that your environment is
consistently updated with the results of your data transformations. The following
steps show you how to specify the `main` branch of the `marketing-tutorial-repository`
repository to store your data transformations:

1. In the Google Cloud console, go to the **Dataform** page.

   [Go to the Dataform page](https://console.cloud.google.com/bigquery/dataform)
2. Select `marketing-tutorial-repository`.

3. Click the **Releases \& scheduling** tab.

4. Click **Create production release**.

5. In the **Create release configuration** pane, configure the following
   settings:

   1. In the **Release ID** field, enter `transformations`.
   2. In the **Git commitish** field, leave the default value `main` .
   3. In the **Schedule frequency** section, select **On-demand**.
6. Click **Create**.

### Create a workflow configuration

Once you have created a production release, you can then create a workflow
configuration that runs your data transformations on a specified schedule
in your repository. The following steps show you how to schedule daily
transformations from the `transformations` file:

1. In the Google Cloud console, go to the **Dataform** page.

   [Go to the Dataform page](https://console.cloud.google.com/bigquery/dataform)
2. Select `marketing-tutorial-repository`.

3. Click the **Releases \& scheduling** tab.

4. In the **Workflow configurations** section, click **Create**.

5. In the **Create workflow configuration** pane, in the **Configuration ID**
   field, enter `transformations`.

6. In the **Release configuration** menu, select `transformations`.

7. Under **Authentication** , select **Execute with user credentials**

8. In the **Schedule frequency** section, do the following:

       1. Select **Repeat**.
       1. For **Repeats**, select `Daily`.
       1. For **At time**, enter `10:00 AM`.
       1. For **Timezone**, select `Coordinated Universal Time (UTC)`.

9. Click **Selection of tags**.

10. In the **Select tags to execute** field, select **Daily**.

11. Click **Create**.

The workflow configuration that you have created runs the entire latest
compilation result created by the `transformations` release configuration.

## Clean up

To avoid incurring charges to your Google Cloud account for the resources used on this page, follow these steps.

### Delete the dataset created in BigQuery

To avoid incurring charges for BigQuery assets, delete the
dataset called `dataform`.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** panel, expand your project and select `dataform`.

3. Click the **Actions** menu, and then select
   **Delete**.

4. In the **Delete dataset** dialog, enter `delete` into the field, and then
   click **Delete**.

### Delete the Dataform development workspace and configurations

Dataform development workspace creation incurs no costs, but to delete
the development workspace you can follow these steps:

1. In the Google Cloud console, go to the **Dataform** page.

   [Go to Dataform](https://console.cloud.google.com/bigquery/dataform)
2. Click `quickstart-repository`.

3. Click the **Release \& scheduling** tab.

4. Under the **Release configurations** section, click the
   **More** menu next to the `production`
   configuration, and then click **Delete**.

5. Under the **Workflow configurations** section, click the
   **More** menu next to the `transformations`
   configuration, and then click **Delete**.

6. In the **Development workspaces** tab, click the
   **More** menu by `quickstart-workspace`, and then select **Delete**.

7. To confirm, click **Delete**.

### Delete the Dataform repository

Dataform repository creation incurs no costs, but to delete the
repository you can follow these steps:

1. In the Google Cloud console, go to the **Dataform** page.

   [Go to Dataform](https://console.cloud.google.com/bigquery/dataform)
2. By `quickstart-repository`, click the **More** menu,
   and then select **Delete**.

3. In the **Delete repository** window, enter the name of the repository to
   confirm deletion.

4. To confirm, click **Delete**.