# Introduction to BigQuery pipelines

You can use BigQuery pipelines to automate and streamline your
BigQuery data processes. With pipelines, you can schedule and
execute code assets in sequence to improve efficiency and reduce manual effort.

## Overview

Pipelines are powered by [Dataform](https://docs.cloud.google.com/dataform/docs/overview).

A pipeline consists of one or more of the following code assets:

- [Notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction)
- [SQL queries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax)
- [Data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction)

You can use pipelines to schedule the execution of code assets. For example,
you can schedule a SQL query to run daily and update a table with the most
recent source data, which can then power a dashboard.

In a pipeline with multiple code assets, you define the execution sequence.
For example, to train a machine learning model, you can create a workflow in
which a SQL query prepares data, and then a subsequent notebook trains the
model using that data.

## Capabilities

You can do the following in a pipeline:

- [Create new or import existing](https://docs.cloud.google.com/bigquery/docs/create-pipelines#add_a_pipeline_task) SQL queries or notebooks into a pipeline.
- [Schedule a pipeline](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines) to automatically run at a specified time and frequency.
- [Share a pipeline](https://docs.cloud.google.com/bigquery/docs/create-pipelines#share_a_pipeline) with users or groups you specify.
- [Share a link to a pipeline](https://docs.cloud.google.com/bigquery/docs/create-pipelines#share_a_link_to_a_pipeline).

## Limitations

Pipelines are subject to the following limitations:

- Pipelines are available only in the Google Cloud console.
- You can't change the region for storing a pipeline after it is created.
- You can grant users or groups access to a selected pipeline, but you can't grant them access to individual tasks within the pipeline.
- If a scheduled pipeline run doesn't finish before the start of the next scheduled run, the next scheduled run is skipped and marked with an error.

## Set the default region for code assets

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

## Supported regions

All code assets are stored in your
[default region for code assets](https://docs.cloud.google.com/bigquery/docs/pipelines-introduction#set_the_default_region_for_code_assets).
Updating the default region changes the region for all code assets created
after that point.

The following table lists the regions where pipelines are available:

|   | Region description | Region name | Details |
|---|---|---|---|
| **Africa** ||||
|   | Johannesburg | `africa-south1` |   |
| **Americas** ||||
|   | Columbus | `us-east5` |   |
|   | Dallas | `us-south1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Los Angeles | `us-west2` |   |
|   | Las Vegas | `us-west4` |   |
|   | Montréal | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | N. Virginia | `us-east4` |   |
|   | Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | South Carolina | `us-east1` |   |
| **Asia Pacific** ||||
|   | Hong Kong | `asia-east2` |   |
|   | Jakarta | `asia-southeast2` |   |
|   | Mumbai | `asia-south1` |   |
|   | Seoul | `asia-northeast3` |   |
|   | Singapore | `asia-southeast1` |   |
|   | Sydney | `australia-southeast1` |   |
|   | Taiwan | `asia-east1` |   |
|   | Tokyo | `asia-northeast1` |   |
| **Europe** ||||
|   | Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Frankfurt | `europe-west3` |   |
|   | London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Madrid | `europe-southwest1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Turin | `europe-west12` |   |
|   | Zürich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| **Middle East** ||||
|   | Doha | `me-central1` |   |
|   | Dammam | `me-central2` |   |

## Quotas and limits

BigQuery pipelines are subject to
[Dataform quotas and limits](https://docs.cloud.google.com/dataform/docs/quotas).

## Pricing

The execution of BigQuery pipeline tasks incurs compute and storage
charges in BigQuery. For more information, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing).

Pipelines containing notebooks incur Colab Enterprise runtime charges
based on the
[default machine type](https://docs.cloud.google.com/colab/docs/runtimes#default_runtime_specifications).
For pricing details, see [Colab Enterprise pricing](https://cloud.google.com/colab/pricing).

Each BigQuery pipeline run is logged using
[Cloud Logging](https://docs.cloud.google.com/logging/docs). Logging is automatically
enabled for BigQuery pipeline runs, which can incur
Cloud Logging billing charges. For more information, see
[Cloud Logging pricing](https://cloud.google.com/logging/pricing).

## What's next

- Learn how to [create pipelines](https://docs.cloud.google.com/bigquery/docs/create-pipelines).
- Learn how to [manage pipelines](https://docs.cloud.google.com/bigquery/docs/manage-pipelines).
- Learn how to [schedule pipelines](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines).