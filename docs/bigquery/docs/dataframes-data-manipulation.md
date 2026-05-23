# Manipulate data with BigQuery DataFrames

This document describes the data manipulation capabilities available with
BigQuery DataFrames. You can find the functions that are described in the
`bigframes.bigquery` library.

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

When you perform end user authentication in an interactive
environment like a notebook, Python REPL, or the command line,
BigQuery DataFrames prompts for authentication, if needed.
Otherwise, see
[how to set up application default credentials](https://docs.cloud.google.com/docs/authentication/provide-credentials-adc)
for various environments.

## pandas API

A notable feature of BigQuery DataFrames is that the
[`bigframes.pandas` API](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.html#module-bigframes.pandas)
is designed to be similar to APIs in the pandas library. This design lets you
employ
familiar syntax patterns for data manipulation tasks. Operations defined through
the BigQuery DataFrames API are executed server-side, operating directly
on data stored within BigQuery and eliminating the need to
transfer datasets out of BigQuery.

To check which pandas APIs are supported by BigQuery DataFrames, see
[Supported pandas APIs](https://dataframes.bigquery.dev/supported_pandas_apis.html).

## Inspect and manipulate data

You can use the `bigframes.pandas` API to perform data inspection and
calculation operations. The following code sample uses the `bigframes.pandas`
library to inspect the `body_mass_g` column, calculate the mean `body_mass`, and
calculate the mean `body_mass` by `species`:

    import bigframes.pandas as bpd

    # Load data from BigQuery
    query_or_table = "bigquery-public-data.ml_datasets.penguins"
    bq_df = bpd.read_gbq(query_or_table)

    # Inspect one of the columns (or series) of the DataFrame:
    bq_df["body_mass_g"]

    # Compute the mean of this series:
    average_body_mass = bq_df["body_mass_g"].mean()
    print(f"average_body_mass: {average_body_mass}")

    # Find the heaviest species using the groupby operation to calculate the
    # mean body_mass_g:
    (
        bq_df["body_mass_g"]
        .groupby(by=bq_df["species"])
        .mean()
        .sort_values(ascending=False)
        .head(10)
    )

## BigQuery library

The BigQuery library provides BigQuery SQL
functions that might not have a pandas equivalent. The following sections
present some examples.

### Process array values

You can use the `bigframes.bigquery.array_agg()` function in the
`bigframes.bigquery` library to aggregate values after a `groupby` operation:

    import bigframes.bigquery as bbq
    import bigframes.pandas as bpd

    s = bpd.Series([0, 1, 2, 3, 4, 5])

    # Group values by whether they are divisble by 2 and aggregate them into arrays
    bbq.array_agg(s.groupby(s % 2 == 0))
    # False    [1 3 5]
    # True     [0 2 4]
    # dtype: list<item: int64>[pyarrow]

You can also use the `array_length()` and `array_to_string()` array functions.

### Create a struct `Series` object

You can use the `bigframes.bigquery.struct()` function in the
`bigframes.bigquery` library to create a new struct `Series` object with
subfields for each column in a DataFrame:

    import bigframes.bigquery as bbq
    import bigframes.pandas as bpd

    # Load data from BigQuery
    query_or_table = "bigquery-public-data.ml_datasets.penguins"
    bq_df = bpd.read_gbq(query_or_table)

    # Create a new STRUCT Series with subfields for each column in a DataFrames.
    lengths = bbq.struct(
        bq_df[["culmen_length_mm", "culmen_depth_mm", "flipper_length_mm"]]
    )

    lengths.peek()
    # 146	{'culmen_length_mm': 51.1, 'culmen_depth_mm': ...
    # 278	{'culmen_length_mm': 48.2, 'culmen_depth_mm': ...
    # 337	{'culmen_length_mm': 36.4, 'culmen_depth_mm': ...
    # 154	{'culmen_length_mm': 46.5, 'culmen_depth_mm': ...
    # 185	{'culmen_length_mm': 50.1, 'culmen_depth_mm': ...
    # dtype: struct[pyarrow]

### Convert timestamps to Unix epochs

You can use the `bigframes.bigquery.unix_micros()` function in the
`bigframes.bigquery` library to convert timestamps into Unix microseconds:

    import pandas as pd

    import bigframes.bigquery as bbq
    import bigframes.pandas as bpd

    # Create a series that consists of three timestamps: [1970-01-01, 1970-01-02, 1970-01-03]
    s = bpd.Series(pd.date_range("1970-01-01", periods=3, freq="d", tz="UTC"))

    bbq.unix_micros(s)
    # 0               0
    # 1     86400000000
    # 2    172800000000
    # dtype: Int64

You can also use the `unix_seconds()` and `unix_millis()` time functions.

### Use the SQL scalar function

You can use the `bigframes.bigquery.sql_scalar()` function in the
`bigframes.bigquery` library to access arbitrary SQL syntax representing a
single-column expression:

    import bigframes.bigquery as bbq
    import bigframes.pandas as bpd

    # Load data from BigQuery
    query_or_table = "bigquery-public-data.ml_datasets.penguins"

    # The sql_scalar function can be used to inject SQL syntax that is not supported
    # or difficult to express with the bigframes.pandas APIs.
    bq_df = bpd.read_gbq(query_or_table)
    shortest = bbq.sql_scalar(
        "LEAST({0}, {1}, {2})",
        columns=[
            bq_df["culmen_depth_mm"],
            bq_df["culmen_length_mm"],
            bq_df["flipper_length_mm"],
        ],
    )

    shortest.peek()
    #         0
    # 149	18.9
    # 33	16.3
    # 296	17.2
    # 287	17.0
    # 307	15.0
    # dtype: Float64

## What's next

- Learn about [custom Python functions](https://docs.cloud.google.com/bigquery/docs/dataframes-custom-python-functions) for BigQuery DataFrames.
- Learn how to [generate BigQuery DataFrames code with Gemini](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#dataframe).
- Learn how to [analyze package downloads from PyPI with BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/dataframes/pypi.ipynb).
- View BigQuery DataFrames [source code](https://github.com/googleapis/python-bigquery-dataframes), [sample notebooks](https://github.com/googleapis/python-bigquery-dataframes/tree/main/notebooks), and [samples](https://github.com/googleapis/python-bigquery-dataframes/tree/main/samples/snippets) on GitHub.
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).