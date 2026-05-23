# Visualize graphs using BigQuery DataFrames

This document demonstrates how to plot various types of graphs by using the
BigQuery DataFrames visualization library.

The [`bigframes.pandas` API](https://dataframes.bigquery.dev/reference/api/bigframes.pandas.html)
provides a full ecosystem of tools for Python. The API supports advanced
statistical operations, and you can visualize the aggregations generated from
BigQuery DataFrames. You can also switch from
BigQuery DataFrames to a `pandas` DataFrame with built-in sampling operations.

## Histogram

The following example reads data from the `bigquery-public-data.ml_datasets.penguins`
table to plot a histogram on the distribution of penguin culmen depths:

    import bigframes.pandas as bpd

    penguins = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")
    penguins["culmen_depth_mm"].plot.hist(bins=40)

![Example of a histogram in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-histogram.png)

## Line chart

The following example uses data from the `bigquery-public-data.noaa_gsod.gsod2021` table
to plot a line chart of median temperature changes throughout the year:

    import bigframes.pandas as bpd

    noaa_surface = bpd.read_gbq("bigquery-public-data.noaa_gsod.gsod2021")

    # Calculate median temperature for each day
    noaa_surface_median_temps = noaa_surface[["date", "temp"]].groupby("date").median()

    noaa_surface_median_temps.plot.line()

![Example of a line chart in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-line-chart.png)

## Area chart

The following example uses the `bigquery-public-data.usa_names.usa_1910_2013` table to
track name popularity in US history and focuses on the names `Mary`, `Emily`,
and `Lisa`:

    import bigframes.pandas as bpd

    usa_names = bpd.read_gbq("bigquery-public-data.usa_names.usa_1910_2013")

    # Count the occurences of the target names each year. The result is a dataframe with a multi-index.
    name_counts = (
        usa_names[usa_names["name"].isin(("Mary", "Emily", "Lisa"))]
        .groupby(("year", "name"))["number"]
        .sum()
    )

    # Flatten the index of the dataframe so that the counts for each name has their own columns.
    name_counts = name_counts.unstack(level=1).fillna(0)

    name_counts.plot.area(stacked=False, alpha=0.5)

![Example of an area chart in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-area-chart.png)

## Bar chart

The following example uses the `bigquery-public-data.ml_datasets.penguins` table to
visualize the distribution of penguin sexes:

    import bigframes.pandas as bpd

    penguins = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")

    penguin_count_by_sex = (
        penguins[penguins["sex"].isin(("MALE", "FEMALE"))]
        .groupby("sex")["species"]
        .count()
    )
    penguin_count_by_sex.plot.bar()

![Example of a bar chart in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-bar-chart.png)

## Scatter plot

The following example uses the
`bigquery-public-data.new_york_taxi_trips.tlc_yellow_trips_2021` table to
explore the relationship between taxi fare amounts and trip distances:

    import bigframes.pandas as bpd

    taxi_trips = bpd.read_gbq(
        "bigquery-public-data.new_york_taxi_trips.tlc_yellow_trips_2021"
    ).dropna()

    # Data Cleaning
    taxi_trips = taxi_trips[
        taxi_trips["trip_distance"].between(0, 10, inclusive="right")
    ]
    taxi_trips = taxi_trips[taxi_trips["fare_amount"].between(0, 50, inclusive="right")]

    # If you are using partial ordering mode, you will also need to assign an order to your dataset.
    # Otherwise, the next line can be skipped.
    taxi_trips = taxi_trips.sort_values("pickup_datetime")

    taxi_trips.plot.scatter(x="trip_distance", y="fare_amount", alpha=0.5)

![Example of a scatter plot in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-scatter-plot.png)

## Visualizing a large dataset

BigQuery DataFrames downloads data to your local machine for
visualization. The number of data points to be downloaded is capped at 1,000 by
default. If the number of data points exceeds the cap, BigQuery DataFrames
randomly samples the number of data points equal to the cap.

You can override this cap by setting the `sampling_n` parameter when plotting
a graph, as shown in the following example:

    import bigframes.pandas as bpd

    noaa_surface = bpd.read_gbq("bigquery-public-data.noaa_gsod.gsod2021")

    # Calculate median temperature for each day
    noaa_surface_median_temps = noaa_surface[["date", "temp"]].groupby("date").median()

    noaa_surface_median_temps.plot.line(sampling_n=40)

![Example of a line chart visualizing a large dataset in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-visualizing-large-dataset.png)

> [!NOTE]
> **Note:** The `sampling_n` parameter has no effect on histograms because BigQuery DataFrames bucketizes the data on the server side for histograms.

## Advanced plotting with pandas and Matplotlib parameters

You can pass in more parameters to fine tune your graph like you can with
pandas, because the plotting library of BigQuery DataFrames is powered
by pandas and Matplotlib. The following sections describe examples.

### Name popularity trend with subplots

Using the name history data from the [area chart example](https://docs.cloud.google.com/bigquery/docs/dataframes-visualizations#area-chart), the
following example creates individual graphs for each name by setting
`subplots=True` in the `plot.area()` function call:

    import bigframes.pandas as bpd

    usa_names = bpd.read_gbq("bigquery-public-data.usa_names.usa_1910_2013")

    # Count the occurences of the target names each year. The result is a dataframe with a multi-index.
    name_counts = (
        usa_names[usa_names["name"].isin(("Mary", "Emily", "Lisa"))]
        .groupby(("year", "name"))["number"]
        .sum()
    )

    # Flatten the index of the dataframe so that the counts for each name has their own columns.
    name_counts = name_counts.unstack(level=1).fillna(0)

    name_counts.plot.area(subplots=True, alpha=0.5)

![Example of individual charts with subplots in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-name-popularity-with-subplots.png)

### Taxi trip scatter plot with multiple dimensions

Using data from the [scatter plot example](https://docs.cloud.google.com/bigquery/docs/dataframes-visualizations#scatter-plot), the following example
renames the labels for the x-axis and y-axis, uses the `passenger_count`
parameter for point sizes, uses color points with the `tip_amount` parameter,
and resizes the figure:

    import bigframes.pandas as bpd

    taxi_trips = bpd.read_gbq(
        "bigquery-public-data.new_york_taxi_trips.tlc_yellow_trips_2021"
    ).dropna()

    # Data Cleaning
    taxi_trips = taxi_trips[
        taxi_trips["trip_distance"].between(0, 10, inclusive="right")
    ]
    taxi_trips = taxi_trips[taxi_trips["fare_amount"].between(0, 50, inclusive="right")]

    # If you are using partial ordering mode, you also need to assign an order to your dataset.
    # Otherwise, the next line can be skipped.
    taxi_trips = taxi_trips.sort_values("pickup_datetime")

    taxi_trips["passenger_count_scaled"] = taxi_trips["passenger_count"] * 30

    taxi_trips.plot.scatter(
        x="trip_distance",
        xlabel="trip distance (miles)",
        y="fare_amount",
        ylabel="fare amount (usd)",
        alpha=0.5,
        s="passenger_count_scaled",
        label="passenger_count",
        c="tip_amount",
        cmap="jet",
        colorbar=True,
        legend=True,
        figsize=(15, 7),
        sampling_n=1000,
    )

![Example of scatter plot with multiple dimensions in BigQuery DataFrames.](https://docs.cloud.google.com/static/bigquery/images/dataframes-scatter-plot-multiple-dimensions.png)

## What's next

- Learn about [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).
- Learn how to [use BigQuery DataFrames in dbt](https://docs.cloud.google.com/bigquery/docs/dataframes-dbt).
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).