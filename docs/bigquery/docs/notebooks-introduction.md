# Introduction to notebooks

This document provides an introduction to
[Colab Enterprise notebooks](https://docs.cloud.google.com/colab/docs/introduction)
in BigQuery. You can use notebooks to complete
analysis and machine learning (ML) workflows by using SQL, Python, and other
common packages and APIs. Notebooks offer improved collaboration and management
with the following options:

- Share notebooks with specific users and groups by using Identity and Access Management (IAM).
- Review the notebook version history.
- Revert to or branch from previous versions of the notebook.

Notebooks are [BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio)
code assets powered by [Dataform](https://docs.cloud.google.com/dataform/docs/overview).
[Saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction) are also code assets.
All code assets are stored in a default
[region](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction#supported_regions). Updating the default region changes
the region for all code assets created after that point.

Notebook capabilities are available only in the Google Cloud console.

## Benefits

Notebooks in BigQuery offer the following benefits:

- [BigQuery DataFrames](https://docs.cloud.google.com/python/docs/reference/bigframes/latest) is integrated into notebooks, no setup required. BigQuery DataFrames is a Python API that you can use to analyze BigQuery data at scale by using the [pandas DataFrame](https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.html) and [scikit-learn](https://scikit-learn.org/stable/modules/classes.html) APIs.
- Assistive code development powered by [Gemini generative AI](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini).
- Auto-completion of SQL statements, the same as in the BigQuery editor.
- The ability to save, share, and manage versions of notebooks.
- The ability to use [matplotlib](https://matplotlib.org/), [seaborn](https://seaborn.pydata.org/), and other popular libraries to visualize data at any point in your workflow.
- The ability to write and [execute SQL](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) in a cell that can reference Python variables from your notebook.
- Interactive [DataFrame visualization](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) that supports aggregation and customization.

## Notebook gallery

The notebook gallery is a central hub for discovering and using prebuilt
notebook templates. These templates let you perform common tasks like data
preparation, data analysis, and visualization. Notebook templates also help you
explore BigQuery Studio features, manage workflows, and promote best
practices.

You can use notebook gallery templates to streamline your entire
intent-to-insights workflow across each stage of the data lifecycle-from
ingestion and exploration to advanced analytics and BigQuery ML.

The notebook gallery provides templates for every skill level. The gallery
includes fundamental templates for SQL, Python, Apache Spark, and
DataFrames. You can also explore topics like generative AI and multimodal data
analytics in BigQuery.

To get started with the notebook gallery, follow these steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. From the BigQuery Studio home page, click **View notebook
   gallery**.

   ![The View notebook gallery link on the BigQuery Studio home page.](https://docs.cloud.google.com/bigquery/images/template-gallery.png)

For more information on using notebook gallery templates, see
[Create a notebook using the notebook gallery](https://docs.cloud.google.com/bigquery/docs/create-notebooks#create-notebook-console).

## Runtime management

BigQuery uses
[Colab Enterprise runtimes](https://docs.cloud.google.com/colab/docs/create-runtime) to run
notebooks.

A notebook runtime is a Compute Engine virtual machine allocated to a
particular user to enable code execution in a notebook. Multiple notebooks can
share the same runtime. However, each runtime belongs to only one user and can't
be used by others. Notebook runtimes are created based on template, which are
typically defined by users with administrative privileges. You can change to a
runtime that uses a different template type at any time.

## Notebook security

You control access to notebooks by using Identity and Access Management (IAM) roles. For
more information, see
[Grant access to notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks#grant_access_to_notebooks).

To detect vulnerabilities in Python packages that you use in your notebooks,
install and use
[Notebook Security Scanner](https://docs.cloud.google.com/security-command-center/docs/enable-notebook-security-scanner)
([Preview](https://cloud.google.com/products#product-launch-stages)).

## Supported regions

BigQuery Studio lets you save, share, and manage versions of
notebooks. The following table lists the regions where BigQuery Studio is
available:

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

## Pricing

For pricing information about BigQuery Studio notebooks, see [Notebook runtime pricing](https://cloud.google.com/bigquery/pricing#external_services).

## Monitor slot usage

You can monitor your BigQuery Studio notebook slot usage by viewing your [Cloud Billing report](https://docs.cloud.google.com/billing/docs/reports) in the Google Cloud console. In the Cloud Billing report, apply a filter with the label **goog-bq-feature-type** with the value **BQ_STUDIO_NOTEBOOK** to view slot usage and costs from BigQuery Studio notebook.

![BigQuery Studio notebook slot usage report.](https://docs.cloud.google.com/static/bigquery/images/studio-notebook-slot-usage.png)

## Troubleshooting

For more information, see [Troubleshoot Colab Enterprise](https://docs.cloud.google.com/colab/docs/troubleshooting).

## What's next

- Learn how to [create notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks).
- Learn how to [manage notebooks](https://docs.cloud.google.com/bigquery/docs/manage-notebooks).