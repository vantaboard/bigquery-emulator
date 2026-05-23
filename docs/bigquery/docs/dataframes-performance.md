# Optimize BigQuery DataFrames performance

BigQuery DataFrames helps you analyze and transform data in BigQuery
using a pandas-compatible API. To make your data processing faster and
more cost-effective, you can use several techniques to improve performance.

This document describes the following ways to optimize performance:

- [Use partial ordering mode](https://docs.cloud.google.com/bigquery/docs/dataframes-performance#partial-ordering-mode).
- [Cache results after expensive operations](https://docs.cloud.google.com/bigquery/docs/dataframes-performance#cache).
- [Preview data by using the `peek()` method](https://docs.cloud.google.com/bigquery/docs/dataframes-performance#preview-peek).
- [Defer the `repr()` data retrieval](https://docs.cloud.google.com/bigquery/docs/dataframes-performance#defer).

## Use partial ordering mode

BigQuery DataFrames has an ordering mode feature, which enforces a
specific row order for operations like window functions and joins. You can
specify the ordering mode by setting the `ordering_mode` property to either
`strict` (known as *strict ordering mode* , which is the default) or
`partial` (known as *partial ordering mode* ). Using the `partial` setting can make your queries more efficient.

Partial ordering mode is different from strict ordering mode. Strict ordering
mode arranges all rows in a specific order. This total ordering makes
BigQuery DataFrames work better with pandas, letting you access rows by
their order using the `DataFrame.iloc` property. However, total ordering and
its default sequential index prevent filters on columns or rows from reducing
the amount of data scanned. This prevention occurs unless you apply those
filters as parameters to the `read_gbq` and `read_gbq_table` functions. To order
all the rows in the DataFrame, BigQuery DataFrames creates a hash of all
the rows. This operation can cause a full data scan that ignores row and column
filters.

Partial ordering mode stops BigQuery DataFrames from creating a total
order for all rows and turns off features that need a total order, like
the `DataFrame.iloc` property. Partial ordering mode also sets the
[`DefaultIndexKind` class](https://dataframes.bigquery.dev/reference/api/bigframes.enums.DefaultIndexKind.html)
to a null index, instead of to a sequential index.

When you filter a `DataFrame` object using partial ordering mode,
BigQuery DataFrames doesn't calculate which rows are missing in the
sequential index. Partial ordering mode also doesn't automatically combine data
based on index. These approaches can increase the efficiency of your queries.
However, whether you use the default strict ordering mode or partial ordering
mode, the BigQuery DataFrames API works like the familiar pandas API.

With both partial and strict ordering modes, you pay for the
BigQuery resources you use. However, using partial ordering
mode can lower costs when working with large clustered and partitioned tables.
This cost reduction occurs because row filters on cluster and partition columns
reduce the amount of data processed.

> [!NOTE]
> **Note:** Partial ordering mode doesn't apply to the BigQuery API, the bq command-line tool, or Terraform, because BigQuery DataFrames is a client-side library.

### Enable partial ordering mode

To use partial ordering, set the `ordering_mode` property to `partial` before
performing any other operation with BigQuery DataFrames, as
shown in the following code sample:

    import bigframes.pandas as bpd

    bpd.options.bigquery.ordering_mode = "partial"

Partial ordering mode prevents implicit joins of unrelated
BigQuery DataFrames objects because it lacks a sequential index.
Instead, you must explicitly call the `DataFrame.merge` method to join
two BigQuery DataFrames objects that derive from different table
expressions.

The `Series.unique()` and `Series.drop_duplicates()` features don't work with
partial ordering mode. Instead, use the `groupby` method to find unique
values, as shown in the following example:

    # Avoid order dependency by using groupby instead of drop_duplicates.
    unique_col = df.groupby(["column"], as_index=False).size().drop(columns="size")

With partial ordering mode, the output of the `DataFrame.head(n)` and
`Series.head(n)` functions might not be the same every time you run them. To
download a small, random sample of the data, use the
[`DataFrame.peek()` or `Series.peek()` methods](https://docs.cloud.google.com/bigquery/docs/dataframes-performance#preview-peek).

For a detailed tutorial in which you use the `ordering_mode = "partial"`
property, see
[Analyzing package downloads from PyPI with BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/dataframes/pypi.ipynb).

### Troubleshooting

Because BigQuery DataFrames in partial ordering mode sometimes lacks an
ordering or index, you might encounter the following issues when using some
pandas-compatible methods.

#### Order required error

Some features, like the `DataFrame.head()` and `DataFrame.iloc` functions, need
an ordering. For a list of features that require ordering, see the **Requires
ordering** column in
[Supported pandas APIs](https://dataframes.bigquery.dev/supported_pandas_apis.html).

When an object has no ordering, the operation fails with an
`OrderRequiredError` message like the following: `OrderRequiredError: Op iloc
requires an ordering. Use .sort_values or .sort_index to provide an ordering.`

As the error message states, you can provide an ordering using the
[`DataFrame.sort_values()` method](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.DataFrame.sort_values.html)
to sort by one or more columns. Other methods, such as
[`DataFrame.groupby()`](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.DataFrame.groupby.html),
implicitly provide a total ordering based on the group by keys.

Unlike pandas, if you are in partial ordering mode, running the same code may
produce different results each time you run it. To ensure consistent results,
use a stable total ordering for all rows.

#### Null index error

Some features, like the `DataFrame.unstack()` and `Series.interpolate()`
properties, need an index. For a list of features that require an index, see
the **Requires index** column in
[Supported pandas APIs](https://dataframes.bigquery.dev/supported_pandas_apis.html).

When you use an operation that requires an index with partial ordering mode,
the operation raises a `NullIndexError` message like the following:
`NullIndexError: DataFrame cannot perform interpolate as it has no index.
Set an index using set_index.`

As the error message states, you can provide an index using the
[`DataFrame.set_index()` method](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.DataFrame.reset_index.html)
to sort by one or more columns. Other methods, such as
[`DataFrame.groupby()`](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.DataFrame.groupby.html),
implicitly provide an index based on the group by keys, unless the
`as_index=False` parameter is set.

## Cache results after expensive operations

BigQuery DataFrames stores operations locally and defers running queries
until certain conditions are met. This can cause the same operations to run
multiple times across different queries.

To avoid repeating costly operations, save intermediate results with the
`cache()` method, as shown in the following example:

    # Assume you have 3 large dataframes "users", "group" and "transactions"

    # Expensive join operations
    final_df = users.join(groups).join(transactions)
    final_df.cache()
    # Subsequent derived results will reuse the cached join
    print(final_df.peek())
    print(len(final_df[final_df["completed"]]))
    print(final_df.groupby("group_id")["amount"].mean().peek(30))

This method creates a temporary BigQuery table to store your
results. You are charged for the storage of this temporary table in
BigQuery.

## Preview your data with the `peek()` method

BigQuery DataFrames offers two API methods to preview data:

- `peek(n)` returns `n` rows of data, where `n` is the number of rows.
- `head(n)` returns the first `n` rows of data, depending on the context, where `n` is the number of rows.

Use the `head()` method only when the order of data is important, for example,
when you want the five largest values in a column. In other cases, use the
`peek()` method for more efficient data retrieval, as shown in the following
code sample:

    import bigframes.pandas as bpd

    # Read the "Penguins" table into a dataframe
    df = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")

    # Preview 3 random rows
    df.peek(3)

You can also use the `peek()` method to download a small, random sample of data
while using [partial ordering mode](https://docs.cloud.google.com/bigquery/docs/dataframes-performance#partial-enable).

## Defer the `repr()` data retrieval

You can call the `repr()` method in BigQuery DataFrames with
[notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction) or your IDE debugger. This
call triggers the `head()` call that retrieves the actual data. This retrieval
can slow down your iterative coding and debugging process and also incur costs.

To prevent the `repr()` method from retrieving data, set the `repr_mode`
attribute to `"deferred"`, as shown in the following example:

    import bigframes.pandas as bpd

    bpd.options.display.repr_mode = "deferred"

In the deferred mode, you can only preview your data with explicit
[`peek()` and `head()` calls](https://docs.cloud.google.com/bigquery/docs/dataframes-performance#preview-peek).

## What's next

- Learn about [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).
- Learn how to [visualize BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-visualizations).
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).
- View BigQuery DataFrames [source code](https://github.com/googleapis/python-bigquery-dataframes), [sample notebooks](https://github.com/googleapis/python-bigquery-dataframes/tree/main/notebooks), and [samples](https://github.com/googleapis/python-bigquery-dataframes/tree/main/samples/snippets) on GitHub.