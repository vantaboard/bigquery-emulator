# Teradata to BigQuery migration: Introduction

This document outlines the reasons you might migrate from Teradata to
BigQuery, compares features between Teradata and BigQuery,
and provides an outline of steps to begin your BigQuery migration.

## Why migrate from Teradata to BigQuery?

Teradata was an early innovator in managing and analyzing substantial
data volumes. However, as your cloud computing needs evolve, you might require a
more modern solution for your data analytics.

If you have previously used Teradata, consider migrating to BigQuery
for the following reasons:

- Overcome legacy platform constraints
  - Teradata's conventional architecture often struggles to meet the demands of modern analytics, particularly the need for unlimited concurrency and consistently high performance for diverse workloads. The serverless architecture in BigQuery is designed to handle these demands with minimal effort.
- Adopt a cloud-native strategy
  - Many organizations are strategically moving from on-premises infrastructure to the cloud. This shift necessitates a departure from conventional, hardware-bound solutions like Teradata towards a fully managed, scalable, and on-demand service like BigQuery to reduce operational overhead.
- Integrate with modern data sources and analytics
  - Key enterprise data increasingly resides in cloud-based sources. BigQuery is natively integrated with the Google Cloud ecosystem, providing seamless access to these sources and enabling advanced analytics, machine learning, and real-time data processing without the infrastructure limitations of Teradata.
- Optimize cost and scalability
  - Teradata often involves complex and costly scaling processes. BigQuery offers transparent and automatic scaling of both storage and compute independently, eliminating the need for manual reconfiguration and providing a more predictable and often lower total cost of ownership.

## Feature comparison

The following table compares the features and concepts in Teradata
to equivalent features in BigQuery:

| Teradata Concept | BigQuery Equivalent | Description |
|---|---|---|
| Teradata (On-premises, Cloud, Hybrid) | BigQuery (Unified, AI Data Platform). BigQuery provides a large set of additional capabilities relative to a conventional data warehouse. | BigQuery is a fully managed, cloud-native data warehouse on Google Cloud. Teradata offers on-premises, cloud, and hybrid options. BigQuery is serverless and available on all clouds as [BQ Omni.](https://docs.cloud.google.com/bigquery/docs/omni-introduction) |
| Teradata Tools (Teradata Studio, BTEQ) | Google Cloud console, BigQuery Studio, the bq command-line tool | Both offer interfaces for managing and interacting with the data warehouse. BigQuery Studio is web-based and integrated with Google Cloud and give ability to write SQL, Python and Apache Spark. |
| Databases/Schemas | Datasets | In Teradata, databases and schemas are used to organize tables and views, similar to BigQuery datasets. However, the way they're managed and used can differ. |
| Table | Table | Both platforms use tables to store data in rows and columns. |
| View | View | Views function similarly in both platforms, providing a way to create virtual tables based on queries. |
| Primary Key | Primary Key (unenforced in GoogleSQL) | BigQuery supports unenforced [primary keys](https://docs.cloud.google.com/bigquery/docs/primary-foreign-keys) in GoogleSQL. These are primarily for helping with query optimization. |
| Foreign Key | Foreign Key (unenforced in GoogleSQL) | BigQuery supports unenforced [foreign keys](https://docs.cloud.google.com/bigquery/docs/primary-foreign-keys) in GoogleSQL. These are primarily for helping with query optimization. |
| Index | Clustering, Search Indexes, Vector Indexes (automatic or managed) | Teradata allows for explicit index creation. <br /> We recommend [clustering in BigQuery](https://docs.cloud.google.com/bigquery/docs/clustered-tables). While not equivalent to Database indexes, clustering helps store the data ordered on disk and this helps optimize with data retrieval when clustered columns are used as predicates. BigQuery supports [Search Indexes](https://docs.cloud.google.com/bigquery/docs/search-index) and [Vector Indexes](https://docs.cloud.google.com/bigquery/docs/vector-index). |
| Partitioning | Partitioning | Both platforms support table partitioning for improved query performance on large tables. <br /> BigQuery only supports partitioning by dates and integers. For strings, use clustering instead. |
| Resource allocation (based on hardware and licensing) | Reservations (Capacity Based), On-demand pricing (Analysis Pricing) | BigQuery offers flexible pricing models. Reservations provide predictable costs for consistent as well as ad hoc workloads using autoscaling, while on-demand pricing focused on per query byte-scan charges. |
| BTEQ, SQL Assistant, other client tools | BigQuery Studio, the bq command-line tool, APIs | BigQuery provides various interfaces for running queries, including a web-based editor, a command-line tool, and APIs for programmatic access. |
| Query Logging/history | Query history, `INFORMATION_SCHEMA.JOBS` | BigQuery maintains a history of executed queries, allowing you to review past queries, analyze performance, and troubleshoot issues. `INFORMATION_SCHEMA.JOBS` maintains the history of all jobs submitted in the last 6 months. |
| Security features (Access control, Encryption) | Security features (IAM, ACLs, encryption) | Both offer robust security. BigQuery uses Google Cloud IAM for granular access control. |
| Network controls (Firewalls, VPNs) | VPC Service Controls, Private Google Access | BigQuery integrates with VPC Service Controls to restrict access to your BigQuery resources from specific networks. Private Google Access lets you access BigQuery without using public IPs. |
| User and Role Management | Identity and Access Management (IAM) | BigQuery uses IAM for fine-grained access control. You can grant specific permissions to users and service accounts at the project, dataset, and table levels. |
| Grants and Roles on Objects | Access Control Lists (ACLs) on datasets and tables | BigQuery lets you define ACLs on datasets and tables to control access at a granular level. |
| Encryption at rest and in transit | Encryption at rest and in transit, Customer-Managed Encryption Keys (CMEK), keys can be hosted in external EKM systems. | BigQuery encrypts data by default. You can also manage your own encryption keys for additional control. |
| Data governance and compliance features | Data governance policies, DLP (Data Loss Prevention) | BigQuery supports data governance policies and DLP to help you enforce data security and compliance requirements. |
| Teradata Load Utilities (e.g., FastLoad, MultiLoad), bteq | The BigQuery Data Transfer Service, the bq command-line tool, APIs | BigQuery provides various data loading methods. Teradata has specialized load utilities. BigQuery emphasizes scalability and speed for data ingestion. |
| Teradata Export Utilities, bteq | The bq command-line tool, APIs, Export to Cloud Storage | BigQuery offers data export to various destinations. Teradata has its own export tools. BigQuery's integration with Cloud Storage is a key advantage. <br /> The BigQuery Storage read API provides any external compute ability to read data in bulk. |
| External Tables | External Tables | Both support querying data in external storage. BigQuery integrates well with Cloud Storage, Spanner, Bigtable, Cloud SQL, AWS S3, Azure Blob Storage, Google Drive. |
| Materialized views | Materialized views | Both offer materialized views for query performance. <br /> BigQuery provides Smart Tuning materialized views that always return current data and also provide automatic query rewrite to materialized views even when the query refers to base table. |
| User-Defined Functions (UDFs) | User-Defined Functions (UDFs) (SQL, JavaScript) | BigQuery supports UDFs in SQL and JavaScript. |
| Teradata Scheduler, other scheduling tools | Scheduled Queries, Managed Service for Apache Airflow, Cloud Functions, BigQuery pipelines | BigQuery integrates with Google Cloud scheduling services and other external scheduling tools. |
| Viewpoint | BigQuery administration for monitoring, health check, explore jobs and manage capacity. | BigQuery offers a UI based comprehensive administration toolbox which contains several panes to monitor operational health and resource utilisation. |
| Backup and Recovery | Dataset cloning, time travel and fail safe, table snapshot and cloning, regional and multi-regional storage, cross-regional backup and recovery. | BigQuery offers snapshots and time travel for recovering data. Time travel is a feature that lets you access historical data within a certain timeframe. BigQuery also offers dataset cloning, regional and multi-regional storage, and cross-regional backup and recovery options. |
| Geospatial Functions | Geospatial Functions | Both platforms have support for geospatial data and functions. |

## Get started

The following sections summarize the Teradata to BigQuery
migration process:

### Run a migration assessment

In your Teradata to BigQuery migration, we
recommend that you start by running the [BigQuery migration
assessment tool](https://docs.cloud.google.com/bigquery/docs/migration-assessment) to assess the feasibility
and potential benefits of moving your
data warehouse from Teradata to BigQuery. This tool
provides a structured approach to understanding your current Teradata
environment and estimating the effort involved in a successful migration.

Running the BigQuery migration assessment tool produces an
assessment report that contains the following sections:

- Existing system report: a snapshot of the existing Teradata system and usage, including the number of databases, schemas, tables, and total size in TB. It also lists the schemas by size and points to potential suboptimal resource utilization, like tables with no writes or few reads.
- BigQuery steady state transformation suggestions: shows what the system will look like on BigQuery after migration. It includes suggestions for optimizing workloads on BigQuery and avoiding wastage.
- Migration plan: provides information about the migration effort itself. For example, getting from the existing system to the BigQuery steady state. This section includes the count of queries that were automatically translated and the expected time to move each table into BigQuery.

For more information about the results of a migration assessment, see [Review the Data Studio report](https://docs.cloud.google.com/bigquery/docs/migration-assessment#review_the_data_studio_report).

### Migrate schema and data from Teradata

Once you've reviewed the results of your migration assessment, you can start your Teradata migration by [preparing BigQuery for the migration](https://docs.cloud.google.com/bigquery/docs/migration/teradata#before_you_begin), then [setting up a data transfer job](https://docs.cloud.google.com/bigquery/docs/migration/teradata#set_up_a_transfer).

For more information about the Teradata migration process,
see [Migrate schema and data from Teradata](https://docs.cloud.google.com/bigquery/docs/migration/teradata).

### Validate your migration

Once you've migrated your Teradata data to BigQuery,
run the Data Validation Tool (DVT) to perform a data
validation on your newly migrated BigQuery data
The DVT validates various functions, from the table level to the row level, to
verify that your migrated data works as intended. For more information about
the DVT, see [Introducing the Data Validation Tool for EDW migrations](https://cloud.google.com/blog/products/databases/automate-data-validation-with-dvt).

You can access the DVT in the [DVT public GitHub repository](https://github.com/GoogleCloudPlatform/professional-services-data-validator).

## What's next

- Try a [test migration](https://docs.cloud.google.com/bigquery/docs/migration/teradata-tutorial) of Teradata to BigQuery.