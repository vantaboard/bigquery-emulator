# Manage BigQuery DataFrames sessions and I/O

This document explains how to manage sessions and perform input and output (I/O)
operations when you use BigQuery DataFrames. You will learn how to create and
use sessions, work with in-memory data, and read from and write to files and
BigQuery tables.

## BigQuery sessions

BigQuery DataFrames uses a local session object internally to manage
metadata. Each `DataFrame` and `Series` object connects to a session, each
session connects to a [location](https://docs.cloud.google.com/bigquery/docs/locations), and each query in a
session runs in the location where you created the session. Use the following
code sample to manually create a session and use it for loading data:

    import https://docs.cloud.google.com/python/docs/reference/bigframes/latest
    import bigframes.pandas as bpd

    # Create session object
    context = https://docs.cloud.google.com/python/docs/reference/bigframes/latest.BigQueryOptions(
        project=YOUR_PROJECT_ID,
        location=YOUR_LOCATION,
    )
    session = https://docs.cloud.google.com/python/docs/reference/bigframes/latest.Session(context)

    # Load a BigQuery table into a dataframe
    df1 = session.https://docs.cloud.google.com/python/docs/reference/bigframes/latest/bigframes.pandas.html("bigquery-public-data.ml_datasets.penguins")

    # Create a dataframe with local data:
    df2 = bpd.DataFrame({"my_col": [1, 2, 3]}, session=session)

You can't combine data from multiple session instances, even if you initialize
them with the same settings. The following code sample shows that trying to
combine data from different session instances causes an error:

    import https://docs.cloud.google.com/python/docs/reference/bigframes/latest
    import bigframes.pandas as bpd

    context = https://docs.cloud.google.com/python/docs/reference/bigframes/latest.BigQueryOptions(location=YOUR_LOCATION, project=YOUR_PROJECT_ID)

    session1 = https://docs.cloud.google.com/python/docs/reference/bigframes/latest.Session(context)
    session2 = https://docs.cloud.google.com/python/docs/reference/bigframes/latest.Session(context)

    series1 = bpd.Series([1, 2, 3, 4, 5], session=session1)
    series2 = bpd.Series([1, 2, 3, 4, 5], session=session2)

    try:
        series1 + series2
    except ValueError as e:
        print(e)  # Error message: Cannot use combine sources from multiple sessions

### Global session

BigQuery DataFrames provides a default global session that you can
access with the `bigframes.pandas.get_global_session()` method. In
Colab, you must provide a project ID for the
`bigframes.pandas.options.bigquery.project` attribute before you use it. You
can also set a location with the
`bigframes.pandas.options.bigquery.location` attribute, which defaults to
the `US` multi-region.

The following code sample shows how to set options for the global session:

    import bigframes.pandas as bpd

    # Set project ID for the global session
    bpd.options.bigquery.project = YOUR_PROJECT_ID
    # Update the global default session location
    bpd.options.bigquery.location = YOUR_LOCATION

To reset the global session's location or project, close the current session by
running the `bigframes.pandas.close_session()` method.

Many BigQuery DataFrames built-in functions use the global session by
default. The following code sample shows how built-in functions use the global
session:

    # The following two statements are essentially the same
    df = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")
    df = bpd.get_global_session().read_gbq("bigquery-public-data.ml_datasets.penguins")

## In-memory data

You can create `DataFrames` and `Series` objects with built-in Python or NumPy
data structures, similar to how you create objects with pandas. Use the
following code sample to create an object:

    import numpy as np

    import bigframes.pandas as bpd

    s = bpd.Series([1, 2, 3])

    # Create a dataframe with Python dict
    df = bpd.DataFrame(
        {
            "col_1": [1, 2, 3],
            "col_2": [4, 5, 6],
        }
    )

    # Create a series with Numpy
    s = bpd.Series(np.arange(10))

To convert `pandas` objects to `DataFrames` objects using the `read_pandas()`
method or constructors, use the following code sample:

    import numpy as np
    import pandas as pd

    import bigframes.pandas as bpd

    pd_df = pd.DataFrame(np.random.randn(4, 2))

    # Convert Pandas dataframe to BigQuery DataFrame with read_pandas()
    df_1 = bpd.read_pandas(pd_df)
    # Convert Pandas dataframe to BigQuery DataFrame with the dataframe constructor
    df_2 = bpd.DataFrame(pd_df)

To use the `to_pandas()` method to load BigQuery DataFrames data into
your memory, use the following code sample:

    import bigframes.pandas as bpd

    bf_df = bpd.DataFrame({"my_col": [1, 2, 3]})
    # Returns a Pandas Dataframe
    bf_df.to_pandas()

    bf_s = bpd.Series([1, 2, 3])
    # Returns a Pandas Series
    bf_s.to_pandas()

### Cost estimation with the `dry_run` parameter

Loading a large amount of data can take a lot of time and resources. To see how
much data is being processed, use the `dry_run=True` parameter in the
`to_pandas()` call. Use the following code sample to perform a dry run:

    import bigframes.pandas as bpd

    df = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")

    # Returns a Pandas series with dry run stats
    df.to_pandas(dry_run=True)

## Read and write files

You can read data from compatible files into a BigQuery DataFrames. These
files can be on your local machine or in Cloud Storage. Use the following code
sample to read data from a CSV file:

    import bigframes.pandas as bpd

    # Read a CSV file from GCS
    df = bpd.read_csv("gs://cloud-samples-data/bigquery/us-states/us-states.csv")

To save your BigQuery DataFrames to local files or Cloud Storage files
using the `to_csv` method, use the following code sample:

    import bigframes.pandas as bpd

    df = bpd.DataFrame({"my_col": [1, 2, 3]})
    # Write a dataframe to a CSV file in GCS
    df.to_csv(f"gs://{YOUR_BUCKET}/myfile*.csv")

## Read and write BigQuery tables

To create BigQuery DataFrames using BigQuery table
references and the `bigframes.pandas.read_gbq` function, use the following code
sample:

    import bigframes.pandas as bpd

    df = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")

To use a SQL string with the `read_gbq()` function to read data into
BigQuery DataFrames, use the following code sample:

    import bigframes.pandas as bpd

    sql = """
    SELECT species, island, body_mass_g
    FROM bigquery-public-data.ml_datasets.penguins
    WHERE sex = 'MALE'
    """

    df = bpd.read_gbq(sql)

> [!NOTE]
> **Note:** If you specify a table when calling the `read_gbq()`, `read_gbq_table()`, or `read_gbq_query()` function, and you haven't set the `bigframes.pandas.options.bigquery.location` attribute before the function call, then BigQuery DataFrames automatically sets the `bigframes.pandas.options.bigquery.location` attribute to the table's location. For information on how to manually specify the location, see [Global session](https://docs.cloud.google.com/bigquery/docs/dataframes-sessions-io#global-session).

To save your `DataFrame` object to a BigQuery table, use the
`to_gbq()` method of your `DataFrame` object. The following code sample shows
how to do that:

    import bigframes.pandas as bpd

    df = bpd.DataFrame({"my_col": [1, 2, 3]})

    df.to_gbq(f"{YOUR_PROJECT_ID}.{YOUR_DATASET_ID}.{YOUR_TABLE_NAME}")

## What's next

- Learn about [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).
- Learn how to [work with data types in BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-data-types).
- Learn how to [visualize graphs using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-visualizations).
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).