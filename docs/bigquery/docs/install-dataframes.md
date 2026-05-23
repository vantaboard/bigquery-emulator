# Install BigQuery DataFrames

BigQuery DataFrames provides a Python DataFrame and machine
learning (ML) API powered by the BigQuery engine.
BigQuery DataFrames is an open-source package.

## Install BigQuery DataFrames

To install the latest version of BigQuery DataFrames, run `pip install
--upgrade bigframes`.

## Available libraries

BigQuery DataFrames provides three libraries:

- `bigframes.pandas` provides a [pandas API](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.html) that you can use to analyze and manipulate data in BigQuery. Many workloads can be migrated from pandas to bigframes by just changing a few imports. The `bigframes.pandas` API is scalable to support processing terabytes of BigQuery data, and the API uses the BigQuery query engine to perform calculations.
- `bigframes.bigquery` provides many BigQuery SQL functions that might not have a pandas equivalent.
- `bigframes.ml` provides an API similar to the scikit-learn API for ML. The ML capabilities in BigQuery DataFrames let you preprocess data, and then train models on that data. You can also chain these actions together to create data pipelines.

## Required roles


To get the permissions that
you need to complete the tasks in this document,

ask your administrator to grant you the
following IAM roles on your project:

- [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`)
- [BigQuery Read Session User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.readSessionUser) (`roles/bigquery.readSessionUser`)
- Use BigQuery DataFrames in a BigQuery notebook:
  - [BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`)
  - [Notebook Runtime User](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.notebookRuntimeUser) (`roles/aiplatform.notebookRuntimeUser`)
  - [Code Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeCreator) (`roles/dataform.codeCreator`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

When you're performing end user authentication in an interactive
environment like a notebook, Python REPL, or the command line,
BigQuery DataFrames prompts for authentication, if needed.
Otherwise, see
[how to set up application default credentials](https://docs.cloud.google.com/docs/authentication/provide-credentials-adc)
for various environments.

## Configure installation options

After you install BigQuery DataFrames, you can specify the following
options.

### Location and project

You need to specify the
[location](https://dataframes.bigquery.dev/reference/api/bigframes._config.BigQueryOptions.location.html#bigframes._config.BigQueryOptions.location)
and
[project](https://dataframes.bigquery.dev/reference/api/bigframes._config.BigQueryOptions.project.html)
in which you want to use BigQuery DataFrames.

You can define the location and project in your notebook in the following way:

    import bigframes.pandas as bpd

    PROJECT_ID = "bigframes-dev"  # @param {type:"string"}
    REGION = "US"  # @param {type:"string"}

    # Set BigQuery DataFrames options
    # Note: The project option is not required in all environments.
    # On BigQuery Studio, the project ID is automatically detected.
    bpd.options.bigquery.project = PROJECT_ID

    # Note: The location option is not required.
    # It defaults to the location of the first table or query
    # passed to read_gbq(). For APIs where a location can't be
    # auto-detected, the location defaults to the "US" location.
    bpd.options.bigquery.location = REGION

### Data processing location

BigQuery DataFrames is designed for scale, which it
achieves by keeping data and processing on the BigQuery
service. However, you can bring data into the memory of your client
machine by calling `.to_pandas()` on a DataFrame or`Series` object. If
you choose to do this, the memory limitation of your client machine
applies.

## What's next

- Learn about [manipulating data](https://docs.cloud.google.com/bigquery/docs/dataframes-data-manipulation) with BigQuery DataFrames.
- Learn how to [generate BigQuery DataFrames code with Gemini](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#dataframe).
- Learn how to [analyze package downloads from PyPI with BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/dataframes/pypi.ipynb).
- View BigQuery DataFrames [source code](https://github.com/googleapis/python-bigquery-dataframes), [sample notebooks](https://github.com/googleapis/python-bigquery-dataframes/tree/main/notebooks), and [samples](https://github.com/googleapis/python-bigquery-dataframes/tree/main/samples/snippets) on GitHub.
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).