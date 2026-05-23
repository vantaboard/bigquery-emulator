# Introduction to loading, transforming, and exporting data

This document describes the data integration approaches to load and transform
data in BigQuery using the extract, load, and transform (ELT) or the
extract, transform, load (ETL) processes. It also describes exporting data from
BigQuery to apply insights in other systems, known as *reverse
ETL*.

## Deciding between ELT or ETL

It's common to transform your data before or after loading it into
BigQuery. A fundamental decision is whether to transform the
data before loading it into BigQuery (extract-transform-load or ETL
approach) or load the raw data into BigQuery and perform transformations using
BigQuery (extract-load-transform or ELT approach).

The following chart shows the various options for data integration into
BigQuery - either using ELT or ETL.

![A decision tree of products used in either ELT or ETL workflows for data integration into BigQuery](https://docs.cloud.google.com/static/bigquery/images/elt-or-etl.png "Diagram of the products involved in either an ELT or ETL workflow for data integration into BigQuery.")

In general, we recommend the ELT approach to most customers. The ELT workflow
splits the complex data integration in two manageable parts - extract \& load, then
transform. Users can choose from a variety data load methods
that suit their needs. Once their data is loaded into BigQuery,
users familiar with SQL can develop transformation pipelines with tools such as
Dataform.

The following sections describe each workflow in further detail.

## Loading and transforming data

It's common to transform your data before or after loading it into
BigQuery. The two common approaches to data integration, ETL and
ELT, are described in the following sections.

### ELT data integration approach

With the extract-load-transform (ELT) approach, you perform data integration in
two discrete steps:

- Extract and load data
- Transform data

For example, you can extract and load data from a JSON file source into a
BigQuery table. Then, you can use pipelines to extract and
transform fields into target tables.

The ELT approach can simplify your data integration workflow in the following
ways:

- Eliminates the need for other data processing tools
- Splits the often complex data integration process into two manageable parts
- Fully utilizes BigQuery's capabilities to prepare, transform, and optimize your data at scale

#### Extracting and loading data

In the ELT data integration approach, you extract data from a data source and
load it into BigQuery using any of the supported
[methods of loading or accessing external data](https://docs.cloud.google.com/bigquery/docs/loading-data#methods).

#### Transforming data in BigQuery

After loading the data into BigQuery, you can prepare and
transform the data with the following tools:

- To collaboratively build, test, document, and schedule advanced SQL data transformation pipelines, use [Dataform](https://docs.cloud.google.com/dataform/docs).
- For smaller data transformation workflows executing SQL code, Python notebooks, or data preparations on a schedule, use [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/pipelines-introduction).
- To clean your data for analysis, use AI-augmented [data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction).

Each of these tools is powered by the
[Dataform API](https://docs.cloud.google.com/dataform/reference/rest).

For more information, see
[Introduction to transformations](https://docs.cloud.google.com/bigquery/docs/transform-intro).

### ETL data integration approach

In the extract-transform-load (ETL) approach, you extract and
transform data before it reaches BigQuery. This approach is
beneficial if you have an existing process in place for data transformation, or
if you aim to reduce resource usage in BigQuery.

[Cloud Data Fusion](https://docs.cloud.google.com/data-fusion/docs/concepts/overview) can help facilitate your ETL process. BigQuery also works with
[3rd-party partners that transform and load data into BigQuery](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-partners#etl-data-integration).

## Exporting data

After you process and analyze data in BigQuery, you can export
the results to apply them in other systems. BigQuery supports the
following exports:

- Exporting query results to a local file, Google Drive, Google Sheets
- Exporting tables or query results to Cloud Storage, Bigtable, Spanner, AlloyDB for PostgreSQL, and Pub/Sub

This process is referred to as reverse ETL.

For more information, see [Introduction to data export in BigQuery](https://docs.cloud.google.com/bigquery/docs/export-intro).

## What's next

- Learn more about [loading data in BigQuery](https://docs.cloud.google.com/bigquery/docs/loading-data).
- Learn more about [transforming data in BigQuery](https://docs.cloud.google.com/bigquery/docs/transform-intro).
- Learn more about [exporting data in BigQuery](https://docs.cloud.google.com/bigquery/docs/export-intro).