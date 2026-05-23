# Using Connected Sheets

Connected Sheets brings the scale of BigQuery to the
familiar Google Sheets interface. With Connected Sheets, you can
preview your BigQuery data and use it in pivot tables, formulas,
and charts built from the entire set of data.

You can also do the following:

- Collaborate with partners, analysts, or other stakeholders in a familiar
  spreadsheet interface.

- Ensure a single source of truth for data analysis without additional
  spreadsheet exports.

- Streamline your reporting and dashboard workflows.

Connected Sheets runs BigQuery queries on your
behalf either upon your request or on a defined schedule. Results of
those queries are saved in your spreadsheet for analysis and sharing.

## Example use cases

The following are just a few use cases that show how
Connected Sheets lets you analyze large amounts of data within a
sheet, without needing to know SQL.

- **Business planning:** Build and prepare datasets, then allow others to find
  insights from the data. For example, analyze sales data to determine which
  products sell better in different locations.

- **Customer service:** Find out which stores have the most complaints per
  10,000 customers.

- **Sales:** Create internal finance and sales reports, and share revenue
  reports with sales reps.

## Access control

Direct access to BigQuery datasets and tables is controlled
within BigQuery. If you want to give a user Google Sheets access
only, share a spreadsheet and don't grant BigQuery access.

A user with Google Sheets-only access can perform analysis in the sheet and
use other Google Sheets features, but the user won't be able to perform the
following actions:

- Manually refresh the BigQuery data in the sheet.
- Schedule a refresh of the data in the sheet.

When you filter data in Connected Sheets, it refreshes the query that you send to
BigQuery against the project that you selected.
You can view the executed query with the following log filter in the related
project:

```
resource.type="bigquery_resource"
protoPayload.metadata.firstPartyAppMetadata.sheetsMetadata.docId != NULL_VALUE
```

### VPC Service Controls

You can use [VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls) to restrict access to
Google Cloud resources. Because VPC Service Controls does not support
Sheets, you might not be able to access BigQuery
data that VPC Service Controls is protecting. If you have the required allow
permissions and meet the
VPC Service Controls access restrictions, you can configure the
VPC Service Controls perimeter to allow queries issued through
Connected Sheets. To do so, you must configure the perimeter using
the following:

- An access level or ingress rule to allow requests from trusted IP addresses, identities, and trusted client devices from outside of the perimeter.
- An egress rule to allow query results to be copied to users' spreadsheets.

Learn about [configuring ingress and egress
policies](https://docs.cloud.google.com/vpc-service-controls/docs/configuring-ingress-egress-policies) and
[configuring access levels](https://docs.cloud.google.com/vpc-service-controls/docs/use-access-levels) to
properly configure the [rules](https://docs.cloud.google.com/vpc-service-controls/docs/ingress-egress-rules).
To configure a perimeter to allow the required data copying, use the following
YAML file:

```
# Allows egress to Sheets through the Connected Sheets feature
- egressTo:
    operations:
    - serviceName: 'bigquery.googleapis.com'
      methodSelectors:
      - permission: 'bigquery.vpcsc.importData'
    resources:
    - projects/628550087766 # Sheets-owned Google Cloud project
  egressFrom:
    identityType: ANY_USER_ACCOUNT
```

> [!NOTE]
> **Note:** Scheduled refreshes of Connected Sheets don't propagate any end-user context such as IP address or device information. VPC Service Controls perimeters that use end-user context to restrict access cause scheduled refreshes to fail.

## Before you begin

First, make sure that you meet the requirements for accessing BigQuery
data in Sheets, as described in the "What you need" section of
the Google Workspace topic
[Get started with BigQuery data in Google Sheets](https://support.google.com/docs/answer/9702507).

If you don't have a Google Cloud project that is set up for billing, follow
these steps:

1. BigQuery is automatically enabled in new projects. To activate BigQuery in a preexisting project, go to


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

To avoid continued billing, you can delete the
resources that you created. See [Cleaning up](https://docs.cloud.google.com/bigquery/docs/connected-sheets#clean-up) for more detail.

## Open BigQuery datasets from Connected Sheets

The following example uses a public dataset to show you how to connect to
BigQuery from Google Sheets:

1. Create or open a Google Sheets spreadsheet.

2. Click **Data** , click **Data connectors** , and then click **Connect to
   BigQuery**.

   > [!NOTE]
   > **Note:** If you don't see the **Data connectors** option, see [Before you begin](https://docs.cloud.google.com/bigquery/docs/connected-sheets#before_you_begin).

3. Select a Google Cloud project that has billing enabled.

4. Click **Public datasets**.

5. In the search box, type **chicago** and then select the
   **chicago_taxi_trips** dataset.

6. Select the **taxi_trips** table and then click **Connect**.

   ![Connect to a table](https://docs.cloud.google.com/static/bigquery/images/connect-to-table.png)

   Your spreadsheet should look similar to the following:

   ![Taxi trips data](https://docs.cloud.google.com/static/bigquery/images/taxi-trips-data.png)

Start using the spreadsheet. You can create pivot tables, formulas, charts,
calculated columns, and scheduled queries
using familiar Google Sheets techniques. For more information, see the
[Connected Sheets tutorial](https://www.youtube.com/watch?v=rkimIhnLKGI).

Although the spreadsheet shows a preview of only 500 rows, any pivot tables,
formulas, and charts use the entire set of data. The maximum number of rows for
results returned for pivot tables is 200,000.

You can also extract the data to a Google Sheets. The maximum number of rows
and cells for results returned for data extracts depends on the following
conditions:

- If the number of rows is less than or equal to 50,000, then there's no cell limit.
- If the number of rows is greater than 50,000 but less than or equal to 500,000, then the number of cells must be less than or equal to 5 million.
- If the number of rows is greater than 500,000, then the data pull isn't supported.

When you use Connected Sheets to create a chart, pivot table, formula,
or other computed cell from your data, Connected Sheets runs a query
in BigQuery on your behalf. To view this query, do the following:

1. Select the cell or chart that you created.
2. Hold the pointer over **Refresh**.
3. Optional: To refresh the query results in Connected Sheets, click **Refresh**.
4. To view the query in BigQuery,
   click
   **Query details on BigQuery**.

   The query opens in the Google Cloud console.

## Open tables in Connected Sheets

To open a table in Connected Sheets, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset that contains the table that you want to open in
   Google Sheets.

4. Click **Overview \> Tables** , and next to your table name, click
   **View actions** ,
   and then select **Open in \> Connected Sheets**.

## Open saved queries in Connected Sheets

Ensure you have a [saved query](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#view_all_saved_queries).
For more information, see [Create saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries).

To open a saved query in Connected Sheets, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Queries**.
   Find the saved query that you want to open in Connected Sheets.

4. Click **Open actions**
   next to the saved query, and then click
   **Open in \> Connected Sheets**.

   Alternatively, click the name of the saved query to open it in the details
   pane, and then click **Open in \> Connected Sheets**.

## Monitor BigQuery usage from Connected Sheets

As a BigQuery administrator, you can monitor and audit resource
consumption from Connected Sheets to understand usage patterns,
manage costs, and identify frequently-used reports. The following sections
provide example SQL queries to help you monitor this usage at both the
organization and project levels. For more information, see
[`JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs).

All queries that originate from Connected Sheets are assigned a unique
job ID prefix: `sheets_dataconnector`. You can use this prefix to filter jobs in
the `INFORMATION_SCHEMA.JOBS` views.

### Aggregate Connected Sheets usage by user at the organization level

The following query provides a summary of the top Connected Sheets
users in your organization over the last 30 days, ranked by their total
billed data. The query aggregates the total number of queries, total
bytes billed, and total slot milliseconds for each user. This information is
useful for understanding adoption and for identifying top consumers of
resources.

    SELECT
      user_email,
      COUNT(*) AS total_queries,
      SUM(total_bytes_billed) AS total_bytes_billed,
      SUM(total_slot_ms) AS total_slot_ms
    FROM
      `region-REGION_NAME.INFORMATION_SCHEMA.JOBS_BY_ORGANIZATION`
    WHERE
      -- Filter for jobs created in the last 30 days
      creation_time >= TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 30 DAY)
      -- Filter for jobs originating from Connected Sheets
      AND job_id LIKE 'sheets_dataconnector%'
      -- Filter for completed jobs
      AND state = 'DONE'
      AND (statement_type IS NULL OR statement_type <> 'SCRIPT')
    GROUP BY
      1
    ORDER BY
      total_bytes_billed DESC;

Replace `REGION_NAME` with the region for your project.
For example, `region-us`.

> [!NOTE]
> **Note:** You must use a region qualifier to query `INFORMATION_SCHEMA` views. The location of the query execution must match the region of the `INFORMATION_SCHEMA` view.

The result looks similar to the following:

```
+---+---+---+---+
| user_email          | total_queries | total_bytes_billed | total_slot_ms   |
+---+---+---+---+
| alice@example.com   | 152           | 12000000000        | 3500000         |
| bob@example.com     | 45            | 8500000000         | 2100000         |
| charles@example.com | 210           | 1100000000         | 1800000         |
+---+---+---+---+
```

### Find job logs of Connected Sheets queries at the organization-level

The following query provides a detailed log of every individual job run by
Connected Sheets. This information is useful for auditing and
identifying specific high-cost queries.

    SELECT
      job_id,
      creation_time,
      user_email,
      project_id,
      total_bytes_billed,
      total_slot_ms
    FROM
      `region-REGION_NAME.INFORMATION_SCHEMA.JOBS_BY_ORGANIZATION`
    WHERE
      creation_time >= TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 30 DAY)
      AND job_id LIKE 'sheets_dataconnector%'
      AND state = 'DONE'
      AND (statement_type IS NULL OR statement_type <> 'SCRIPT')
    ORDER BY
      creation_time DESC;

Replace `REGION_NAME` with the region for your project.
For example, `region-us`.

> [!NOTE]
> **Note:** You must use a region qualifier to query `INFORMATION_SCHEMA` views. The location of the query execution must match the region of the `INFORMATION_SCHEMA` view.

The result looks similar to the following:

```
+---+---+---+---+---+---+
| job_id                          | creation_time                   | user_email      | project_id | total_bytes_billed | total_slot_ms |
+---+---+---+---+---+---+
| sheets_dataconnector_bquxjob_1  | 2025-11-06 00:26:53.077000 UTC  | abc@example.com | my_project | 12000000000        | 3500000       |
| sheets_dataconnector_bquxjob_2  | 2025-11-06 00:24:04.294000 UTC  | xyz@example.com | my_project | 8500000000         | 2100000       |
| sheets_dataconnector_bquxjob_3  | 2025-11-03 23:17:25.975000 UTC  | bob@example.com | my_project | 1100000000         | 1800000       |
+---+---+---+---+---+---+
```

### Aggregate Connected Sheets usage by user at the project level

If you don't have organization-level permissions or only need to monitor a
specific project, run the following query to identify the top
Connected Sheets users within a project over the last 30 days. The
query aggregates the total number of queries, total bytes billed, and total slot
milliseconds for each user. This information is useful for understanding
adoption and for identifying top consumers of resources.

    SELECT
      user_email,
      COUNT(*) AS total_queries,
      SUM(total_bytes_billed) AS total_bytes_billed,
      SUM(total_slot_ms) AS total_slot_ms
    FROM
      -- This view queries the project you are currently running the query in.
      `region-REGION_NAME`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
    WHERE
      -- Filter for jobs created in the last 30 days
      creation_time >= TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 30 DAY)
      -- Filter for jobs originating from Connected Sheets
      AND job_id LIKE 'sheets_dataconnector%'
      -- Filter for completed jobs
      AND state = 'DONE'
      AND (statement_type IS NULL OR statement_type <> 'SCRIPT')
    GROUP BY
      user_email
    ORDER BY
      total_bytes_billed DESC
    LIMIT
      10;

Replace `REGION_NAME` with the region for your project.
For example, `region-us`.

> [!NOTE]
> **Note:** You must use a region qualifier to query `INFORMATION_SCHEMA` views. The location of the query execution must match the region of the `INFORMATION_SCHEMA` view.

The result looks similar to the following:

```
+---+---+---+---+
| user_email          | total_queries | total_bytes_billed | total_slot_ms   |
+---+---+---+---+
| alice@example.com   | 152           | 12000000000        | 3500000         |
| bob@example.com     | 45            | 8500000000         | 2100000         |
| charles@example.com | 210           | 1100000000         | 1800000         |
+---+---+---+---+
```

### Find job logs of Connected Sheets queries at the project-level

If you don't have organization-level permissions or only need to monitor a
specific project, run the following query to see a detailed log of all
Connected Sheets queries for the current project:

    SELECT
      job_id,
      creation_time,
      user_email,
      total_bytes_billed,
      total_slot_ms,
      query
    FROM
      -- This view queries the project you are currently running the query in.
      `region-REGION_NAME.INFORMATION_SCHEMA.JOBS_BY_PROJECT`
    WHERE
      creation_time >= TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 30 DAY)
      AND job_id LIKE 'sheets_dataconnector%'
      AND state = 'DONE'
      AND (statement_type IS NULL OR statement_type <> 'SCRIPT')
    ORDER BY
      creation_time DESC;

Replace `REGION_NAME` with the region for your project.
For example, `region-us`.

> [!NOTE]
> **Note:** You must use a region qualifier to query `INFORMATION_SCHEMA` views. The location of the query execution must match the region of the `INFORMATION_SCHEMA` view.

The result looks similar to the following:

```
+---+---+---+---+---+---+
| job_id                          | creation_time                   | user_email       | total_bytes_billed | total_slot_ms   |  query                          |
+---+---+---+---+---+---+
| sheets_dataconnector_bquxjob_1  | 2025-11-06 00:26:53.077000 UTC  | abc@example.com  | 12000000000        | 3500000         |  SELECT ... FROM dataset.table1 |
| sheets_dataconnector_bquxjob_2  | 2025-11-06 00:24:04.294000 UTC  | xyz@example.com  | 8500000000         | 2100000         |  SELECT ... FROM dataset.table2 |
| sheets_dataconnector_bquxjob_3  | 2025-11-03 23:17:25.975000 UTC  | bob@example.com  | 1100000000         | 1800000         |  SELECT ... FROM dataset.table3 |
+---+---+---+---+---+---+
```

## Cleaning up

To avoid incurring charges to your Google Cloud account for the resources
used in this tutorial:

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

- Get more information from the Google Workspace
  [Get started with BigQuery data in Google Sheets](https://support.google.com/docs/answer/9702507) topic.

- View videos from the [Using Connected Sheets playlist](https://www.youtube.com/playlist?list=PLU8ezI8GYqs74i8hy_qln3FvkAuSpE-r1) on YouTube.