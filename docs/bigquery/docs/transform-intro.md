# Introduction to data transformation

This document describes the different ways you can transform data in your
BigQuery tables.

For more information about data integrations, see
[Introduction to loading, transforming, and exporting data](https://docs.cloud.google.com/bigquery/docs/load-transform-export-intro).

## Methods of transforming data

You can transform data in BigQuery in the following ways:

- Use [data manipulation language (DML)](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-dml) to transform data in your BigQuery tables.
- Use [materialized views](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-mvs) to automatically cache the results of a query for increased performance and efficiency.
- Use [continuous queries](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-continuous-queries) to analyze incoming data in real time and continuously insert the output rows into a BigQuery table or export to Pub/Sub or Bigtable.
- Use [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-bq-pipelines) or [Dataform](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-dataform) to develop, test, control versions, and schedule pipelines in BigQuery.
- Use [data preparations](https://docs.cloud.google.com/bigquery/docs/transform-intro#data-preparation) with context-aware, AI-generated transformation recommendations to cleanse data for analysis. Data preparations are powered by the [Dataform API](https://docs.cloud.google.com/dataform/reference/rest).

The following table shows the different characteristics of each transformation
method.

| Transform method | Transformation target | Definition method | Transformation frequency |
|---|---|---|---|
| [Data manipulation language (DML)](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-dml) | [Table (in place)](https://docs.cloud.google.com/bigquery/docs/tables-intro) | [SQL DML](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) | User-initiated or scheduled |
| [Materialized views](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-mvs) | [Materialized view](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) | [SQL query](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax) | Automatic or manual refresh |
| [Continuous queries](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-continuous-queries) | [Table](https://docs.cloud.google.com/bigquery/docs/tables-intro), [Pub/Sub topic](https://docs.cloud.google.com/bigquery/docs/continuous-queries#pubsub-example), [Bigtable table](https://docs.cloud.google.com/bigquery/docs/continuous-queries#bigtable-example) | [SQL query with EXPORT DATA](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements#export_data_statement) | Continuous |
| [Dataform](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-dataform) | [Table](https://docs.cloud.google.com/bigquery/docs/tables-intro) | [Dataform core (SQLX)](https://docs.cloud.google.com/dataform/docs/overview#dataform-core) | Scheduled (pipelines) |
| [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/transform-intro#transform-with-bq-pipelines) | [Table](https://docs.cloud.google.com/bigquery/docs/tables-intro) | [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/pipelines-introduction) | Scheduled (pipelines) |
| [Data preparation](https://docs.cloud.google.com/bigquery/docs/transform-intro#data-preparation) | [Table](https://docs.cloud.google.com/bigquery/docs/tables-intro) | [Visual editor](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#open-data-prep-editor) | Scheduled |

You can also [review the change history of a BigQuery table](https://docs.cloud.google.com/bigquery/docs/change-history)
to examine the transformations made to a table in a specified time range.

### Transform data with DML

You can use [data manipulation language (DML)](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
to transform data in your BigQuery tables. DML statements are
GoogleSQL queries that
manipulate existing table data to add or delete rows, modify data
in existing rows, or merge data with values from another table. DML
transformations are also supported in [partitioned tables](https://docs.cloud.google.com/bigquery/docs/using-dml-with-partitioned-tables).

You can run multiple DML statements concurrently, where BigQuery
queues several DML statements that transform your data one after the other.
BigQuery manages [how concurrent DML statements are run](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#concurrent_jobs),
based upon the transformation type.

### Transform data with materialized views

[Materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) views are
precomputed views that periodically cache the results of a SQL query for increased
performance and efficiency. BigQuery leverages precomputed
results from materialized views and whenever possible reads only changes from
the base tables to compute up-to-date results.

Materialized views are precomputed in the background when the base tables change.
Any incremental data changes from the base tables are automatically added to the
materialized views, with no user action required.

### Transform data with continuous queries

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

[Continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction)
are SQL statements that run continuously. Continuous queries let you analyze
incoming data in BigQuery in real time. You can insert the output
rows produced by a continuous query into a BigQuery table or
export them to Pub/Sub or Bigtable.

### Transform data with Dataform

Dataform lets you manage data transformation in the extract,
load, and transform (ELT) process for data integration. After extracting
raw data from source systems and loading it into BigQuery, you can
use Dataform to transform it into an organized, tested, and documented
suite of tables. While in DML you take an imperative approach by telling BigQuery
how exactly to transform your data, in Dataform you write
declarative statements where Dataform then determines the
transformation needed to achieve that state.

In Dataform, you can develop, test, and version control [SQL workflows for data transformation](https://docs.cloud.google.com/dataform/docs/sql-workflows)
from data source declarations to output tables, views, or materialized views.
You can develop SQL workflows with Dataform core or pure JavaScript.
[Dataform core](https://docs.cloud.google.com/dataform/docs/overview#dataform-core) is an open source
meta-language that extends SQL with SQLX and JavaScript. You can use
Dataform core to manage dependencies, set up automated data
quality testing, and document table or column descriptions within the code.

Dataform stores your SQL workflow code in [repositories](https://docs.cloud.google.com/dataform/docs/create-repository)
and uses Git to track file changes. Development workspaces in Dataform
let you work on the contents of the repository without affecting the work of
others who are working in the same repository. You can connect Dataform
repositories to third-party Git providers, including Azure DevOps Services,
Bitbucket, GitHub, and GitLab.

You can run or schedule SQL workflows
with Dataform release configurations and workflow configurations.
Alternatively, you can schedule executions either with Managed Service for Apache Airflow, or
with Workflows and Cloud Scheduler. During execution,
Dataform executes SQL queries in BigQuery in
order of object dependencies in your SQL workflow. After execution, you can use
your defined tables and views for analysis in BigQuery.

To learn more about creating data transformation SQL workflows in
Dataform, see [Dataform overview](https://docs.cloud.google.com/dataform/docs/overview) and
[Dataform features](https://docs.cloud.google.com/dataform/docs/overview#features).

### Transform data with BigQuery pipelines

BigQuery pipelines are powered by Dataform
and let you create and manage data transformation
in extract, load, transform (ELT) or extract, transform, load (ETL) processes.

You can create and manage BigQuery pipelines in a visual way
in BigQuery Studio.

To learn more about creating BigQuery pipelines,
see [Create pipelines](https://docs.cloud.google.com/bigquery/docs/create-pipelines).

### Prepare data in BigQuery

To reduce the toil of data preparation, BigQuery lets you clean
data with Gemini-generated transformation suggestions. Data preparation
in BigQuery offers the following assistance:

- Applying transformations and data quality rules
- Standardizing and enriching data
- Automating schema mapping

You can validate the results in a preview of your data before executing the
changes on all your data.

For more information, see [BigQuery data preparation overview](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction).

## What's next

- To learn more about DML, see [Transform data with data manipulation language (DML)](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language).
- To learn more about Dataform, see [Dataform overview](https://docs.cloud.google.com/dataform/docs/overview).