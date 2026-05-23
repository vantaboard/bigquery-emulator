# Use Gemini Cloud Assist

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

This document describes how to use
[Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/overview), a
product of the
[Gemini for Google Cloud](https://cloud.google.com/products/gemini)
portfolio, to help you understand and work with your metadata, jobs, and queries
in BigQuery. It provides supported use cases and sample prompts that
you can use in Gemini Cloud Assist.

## Before you begin

Before you can use Gemini Cloud Assist, your administrator must
perform the steps to
[Set up Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/set-up-gemini) for the
project or folder that you're working in.

In order to support questions and requests about your Google Cloud resources,
Gemini Cloud Assist needs the appropriate Identity and Access Management (IAM)
permissions for those resources. Gemini Cloud Assist inherits
your permissions when you prompt it to query your BigQuery data,
so in many cases, the necessary IAM permissions are
already granted. For more information, see
[IAM requirements for using Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/iam-requirements).

## Use Gemini Cloud Assist

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the Google Cloud toolbar, click
   spark **Open or
   close Gemini AI chat** to open Gemini Cloud Assist chat.

   ![Gemini Cloud Assist button in the BigQuery toolbar.](https://docs.cloud.google.com/static/bigquery/images/gemini-spark.png)
3. In the **Enter a prompt** field, enter your prompt.

4. Click **Send prompt**.

The following sections provide examples of tasks that you can
perform with Gemini Cloud Assist, along with sample prompts.

## Analyze jobs

Learn more about jobs executed
in your project, including your personal job history and project job history,
to support the following use cases:

- **Debug long-running queries** . Learn about the current status of a job and
  reasons it might be taking longer than expected, such as slot contention,
  a large number of rows scanned, high data volume, and others. In the
  **Cloud Assist** panel, enter a prompt similar to the following:

      Why is this job taking so long? JOB_ID

- **Analyze the cause of a failed job** . Learn about why a specific
  query failed. In the
  **Cloud Assist** panel, enter a prompt similar to the following:

      Why did JOB_ID fail?

- **Find resource-intensive queries** . Learn about your most expensive queries
  based on the estimated number of bytes processed. In the
  **Cloud Assist** panel, enter a prompt similar to the following:

      What are the 3 most expensive queries that I ran in the last 2 days?

## Discover resources

Search for and learn about datasets and table resources in a single project or
across multiple projects. Gemini Cloud Assist uses
Knowledge Catalog to search your BigQuery
resources. Searches are performed using your permissions. For example, if you
don't have permission to view the metadata of a resource, then it won't
show up in the results. Supported use cases include the following:

- **Search for a resource by name** . In the
  **Cloud Assist** panel, enter a prompt similar to the following:

      Do I have any datasets named ecommerce?

- **Ask about a table's metadata** . You can ask about a table by name, or let
  Gemini Cloud Assist infer which table you mean based on your
  chat history or which table is referenced in your active query tab. If you
  specify a table by name, then you must use the fully qualified name. You can
  ask about a table's schema or other metadata, such as partitioning and
  clustering. In the
  **Cloud Assist** panel, enter a prompt similar to the following:

      What's the schema for `project_name.dataset_name.table_name`?

- **Ask where to find specific information** . In the **Cloud Assist** panel,
  enter a prompt similar to the following:

      Where can I find demographics, such as age and location, for new users from the last year?

## Generate SQL

Generate a SQL query by describing what you want the query to do. For best
results, include the name of the table that you want to query. For example,
in the **Cloud Assist** panel, enter a prompt similar to the following:

    Generate a SQL query to show me the duration and subscriber type for the ten longest trips.
    Use the `bigquery-public-data.san_francisco_bikeshare.bikeshare_trips` table.

## Generate Python code

Generate Python code by describing what you want it to do. For example, in the
**Cloud Assist** panel, you can enter the
following prompt to ask Gemini to query the `penguins` table
from a public dataset using the BigQuery magics syntax:

    Generate python code to query the `bigquery-public-data.ml_datasets.penguins`
    table using bigquery magics

## What's next

- Learn more about [Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/overview).
- Learn how [Gemini for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).