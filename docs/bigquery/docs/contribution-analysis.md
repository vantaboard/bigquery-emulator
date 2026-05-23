# Contribution analysis overview

Use this document to understand the contribution analysis use case,
and the options for performing contribution analysis in BigQuery ML.

## What is contribution analysis?

Contribution analysis, also called key driver analysis, is a method used to
generate insights about changes to key metrics in your multi-dimensional data.
For example, you can use contribution analysis to see what data contributed to a
change in revenue numbers across two quarters, or to compare two sets of
training data to understand changes in an ML model's performance.

Contribution analysis is a form of
[augmented analytics](https://en.wikipedia.org/wiki/Augmented_Analytics),
which is the use of artificial intelligence (AI) to enhance and automate the
analysis and understanding of data. Contribution analysis accomplishes one of
the key goals of augmented analytics, which is to help users find patterns in
their data.

## Contribution analysis with BigQuery ML

Contribution analysis detects segments of data that show changes in
a given metric by comparing a test set of data to a control set of data. For
example, you might use a [table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro)
of sales data taken at the end of 2023 as your test data and a table snapshot
taken at the end of 2022 as your control data, and compare them to see how
your sales changed over time. Contribution analysis can show you
which segment of data, such as online customers in a particular region, drove
the biggest change in sales from one year to the next.

A *metric* is the numerical value that contribution analysis models use
to measure and compare the changes between the test and control data. You can
specify the following types of metrics with a contribution analysis model:

- [*Summable*](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_metric): sums the values of a metric column that you specify, and then determines a total for each segment of the data.
- [*Summable ratio*](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_ratio_metric): sums the values of two numeric columns that you specify, and determines the ratio between them for each segment of the data.
- [*Summable by category*](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_by_category_metric): sums the value of a numeric column and divides it by the number of distinct values from a categorical column.

A *segment* is a slice of the data identified by a given combination of
dimension values. For example, for a contribution analysis model based on the
`store_number`, `customer_id`, and `day` dimensions, every unique combination of
those dimension values represents a segment. In the following table, each row
represents a different segment:

| **`store_number`** | **`customer_id`** | **`day`** |
|---|---|---|
| store 1 |   |   |
| store 1 | customer 1 |   |
| store 1 | customer 1 | Monday |
| store 1 | customer 1 | Tuesday |
| store 1 | customer 2 |   |
| store 2 |   |   |

### Analyze data without a model

If you have fewer than 12 dimensions and are using a summable metric, then you
can perform contribution analysis by using the
[`AI.KEY_DRIVERS` TVF](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-key-drivers).
For most applications, we recommend using the `AI.KEY_DRIVERS`
function over creating a model because it offers a simplified syntax, faster
results, and automatic pruning. The function output
consists of rows of insights, where each insight corresponds to a segment and
provides the segment's corresponding metrics.

### Use a contribution analysis model

If you require more than 12 dimensions or other types
of metrics, you can create a contribution analysis model with the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis).

To reduce model creation time, specify an
[apriori support threshold](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_an_apriori_support_threshold).
An apriori support threshold lets you prune small and less relevant segments
so that the model uses only the largest and most relevant segments.

After you have created a contribution analysis model, you can use the
[`ML.GET_INSIGHTS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights)
to retrieve the metric information calculated by the model. The function output
consists of rows of insights, where each insight corresponds to a segment and
provides the segment's corresponding metrics.

## Contribution analysis user journey

The following table describes the statements and functions you can use with
contribution analysis:

| Statement or function | [Feature preprocessing](https://docs.cloud.google.com/bigquery/docs/preprocess-overview) | Insights generation | Tutorials |
|---|---|---|---|
| [`AI.KEY_DRIVERS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-key-drivers) | [Manual preprocessing](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing) | N/A | [Example of contribution analysis on Iowa liquor sale data](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-key-drivers#example) |
| [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis) | [Manual preprocessing](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing) | [`ML.GET_INSIGHTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights) | - [Get data insights from a contribution analysis model using a summable metric](https://docs.cloud.google.com/bigquery/docs/get-contribution-analysis-insights) - [Get data insights from a contribution analysis model using a summable ratio metric](https://docs.cloud.google.com/bigquery/docs/get-contribution-analysis-insights-sum-ratio) |

## What's next

- [Create a contribution analysis model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis)
- [Get data insights from a contribution analysis model](https://docs.cloud.google.com/bigquery/docs/get-contribution-analysis-insights)