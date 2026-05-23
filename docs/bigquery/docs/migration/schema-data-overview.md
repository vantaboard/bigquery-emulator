# Schema and data transfer overview

This document discusses the concepts and tasks
for transferring the schema and data from your existing data warehouse to
BigQuery.

Migrating your data warehouse to the cloud is a complex process that requires
planning, resources, and time. To tame this complexity, you should approach data
warehouse migration in a staged and iterative manner. Doing several iterations
of schema and data migration can improve the result.

## Schema and data migration process

At the start of your migration journey, you have upstream systems that feed
your existing data warehouse, and downstream systems that use that data in
reports, dashboards, and as feeds to other processes.

This general flow of data supports many analytics
[use cases](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview#use-case),
as shown in the following diagram:

![Starting state before migration.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-original-state.svg)

The end state of your journey is to have as many use cases as possible running
on top of BigQuery. This state enables you to minimize the use of
your existing data warehouse and to eventually phase it out. You're in control of
which use cases are migrated and when, by prioritizing them during the
[prepare and discover](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview#prepare-and-discover)
phase of the migration.

### Transfer schema and data to BigQuery

In the
[planning](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview#plan)
phase of the migration, you identify the use cases that you want to migrate.
Then you start the migration iterations in the
[execute](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview#execute)
phase. To manage your iterations while running your analytics environment with
minimal disruption, follow this high-level process:

1. Transfer tables and configure and test downstream processes.

   - Transfer the group of tables for each use case to BigQuery without any changes, using BigQuery Data Transfer Service or another ETL tool. For information about tools, see the [initial data transfer](https://docs.cloud.google.com/bigquery/docs/migration/schema-data-overview#initial-data-transfer) section.
   - Configure test versions of your downstream processes to read from the BigQuery tables.

   This initial step divides the flow of data. The following diagram shows
   the resulting flow. Some downstream systems now read from
   BigQuery as shown in the flows labeled B. Others still read
   from the existing data warehouse, as shown in the flows labeled A.

   ![Upstream processes feed into the existing data warehouse. Some of those go to
   downstream processes, but others go to BigQuery by means
   of BigQuery Data Transfer Service, and from there to different downstream processes.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-offload.svg)
2. Configure some test upstream processes to write data to
   BigQuery tables instead of (or in addition to) the existing
   data warehouse.

   After testing, configure your production upstream and downstream
   processes to write and read to the BigQuery tables. These
   processes can connect to BigQuery using the
   [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference)
   and incorporate new cloud products such as
   [Data Studio](https://lookerstudio.google.com/)
   and
   [Dataflow](https://docs.cloud.google.com/dataflow/docs).

   At this point, you have three flows of data:
   1. Existing. The data and processes are unchanged and still centered on your existing data warehouse.
   2. [Offloaded](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview#offload). The upstream processes feed your existing data warehouse, the data is offloaded to BigQuery, and it then feeds downstream processes.
   3. [Fully migrated](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview#full-migration). The upstream and downstream processes don't write or read from the existing data warehouse anymore.

      The following diagram shows a system with all of these three flows:
      ![Flow of workloads through multiple paths.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-three-paths.svg)
3. Select additional use cases for migration, then go to step 1 to start a
   new
   [execution iteration](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview#migrating-using-an-iterative-approach).
   Continue iterating through these steps until all your use cases are fully
   migrated into BigQuery. When selecting use cases, you can
   revisit ones that remained in the offloaded state to move them to fully
   migrated. For the use cases that are fully migrated, consider continuing
   the evolution process by following the guidelines in
   [Evolve your schema in BigQuery](https://docs.cloud.google.com/bigquery/docs/migration/schema-data-overview#evolving_your_schema_in_bigquery).

   ![Final step of migrated use cases.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-target-state.svg)

### Evolve your schema in BigQuery

The data warehouse schema defines how your data is structured and defines the
relationships between your data entities. The schema is at the core of your data
design, and it influences many processes, both upstream and downstream.

A data warehouse migration presents a unique opportunity to evolve your schema
after it's moved to BigQuery. This section introduces guidelines
for evolving your schema using a series of steps. These guidelines help you keep
your data warehouse environment running during schema changes with minimal
disruption to upstream and downstream processes.

The steps in this section focus on the schema transformation for a single use
case.

Depending on how far you want to go with the evolution, you might stop at an
intermediate step, or you might continue until your system is fully evolved.

1. Transfer a use case as is to BigQuery.

   Before you continue with the next steps, make sure that the upstream and
   downstream processes of your use case are already writing and reading from
   BigQuery. However, it's also possible to start from an
   intermediate state where only the downstream process is reading from
   BigQuery. In this scenario, apply only the guidelines for
   the downstream part. The following diagram illustrates a use case where
   upstream and downstream processes write to and read from tables in
   BigQuery.

   ![Upstream processes feed into BigQuery tables and from
   there to downstream processes.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-evolve-target.svg)
2. Apply light optimizations.

   1. Re-create your tables, applying [partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) and [clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables). For this task, you can use the method of creating a table from a query result. For details, see the [discussion](https://docs.cloud.google.com/bigquery/docs/creating-column-partitions#creating_a_partitioned_table_from_a_query_result) and [example](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#creating_a_partitioned_table_from_the_result_of_a_query) for partitioned tables, and see the [discussion](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables#create_a_clustered_table_from_a_query_result) and [example](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#creating_a_clustered_table_from_the_result_of_a_query) for clustered tables.
   2. Redirect your upstream and downstream processes to the new tables.
3. Create facade views.

   If you want to further evolve your schema beyond light optimizations,
   create *facade views* for your tables. The
   [facade pattern](https://wikipedia.org/wiki/Facade_pattern)
   is a design method that masks the underlying code or structures to hide
   complexity. In this case, the facade views mask the underlying tables to
   hide the complexity caused by table changes from the downstream processes.

   The views can describe a new schema, free from technical debt, and
   modeled with new ingestion and consumption scenarios in mind.

   Under the hood, the tables and the view query definition itself can
   change. But the views abstract away these changes as an internal
   implementation detail of your data warehouse, and they always return the
   same results. This
   [abstraction layer](https://wikipedia.org/wiki/Abstraction_layer) made
   of facade views isolates your upstream and downstream systems from change
   for as long as needed, and only surfaces the changes when appropriate.
4. Transform downstream processes.

   You can transform some of your downstream processes to read from the
   facade views instead of from the actual tables. These processes will
   already benefit from the evolved schema. It's transparent to these
   processes that under the hood, the facade views still get their data from
   the source BigQuery schema, as shown in the following
   diagram:

   ![Upstream processes feed into BigQuery tables. Some feed into downstream processes. Others feed into facade views, which feed into evolved downstream processes.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-evolve-transform-downstream.svg)

   We've described the downstream process transformation first. This lets
   you show business value more quickly, in the form of migrated dashboards or
   reports, than if you transformed upstream processes that aren't visible to
   non-technical stakeholders. However, it's possible to start the
   transformation with your upstream processes instead. The priority of these
   tasks is entirely dependent on your needs.
5. Transform upstream processes.

   You can transform some of your upstream processes to write into the new
   schema. Because views are read only, you create tables based on the new
   schema, and you then modify the query definition of the facade views. Some
   views will still query the source schema, while others will query the newly
   created tables, or perform a SQL `UNION` operation on both, as shown in the
   following diagram:

   ![Upstream processes feed into BigQuery tables, but they no longer feed into downstream processes. Instead, the BigQuery tables feed into facade views, which in turn feed into evolved downstream processes.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-evolve-transform-upstream.svg)

   At this point, you can take advantage of
   [nested and repeated fields](https://docs.cloud.google.com/bigquery/docs/migration/schema-data-overview#denormalization)
   when you create the new tables. This lets you further denormalize your
   model and take direct advantage BigQuery underlying columnar
   representation of data.

   A benefit of facade views is that your downstream processes can continue
   their transformation independently from these underlying schema changes and
   independently from changes in the upstream processes.
6. Fully evolve your use case.

   Finally, you can transform the remaining upstream and downstream
   processes. When all of these are evolved to write into the new tables and
   to read from the new facade views, you modify the query definitions of the
   facade views to not read from the source schema anymore. You can then
   retire the tables in the source model from the data flow. The following
   diagram shows the state where source tables are no longer used.

   ![The original upstream processes are no longer in use. Only evolved upstream processes remain, which feed to evolved tables, which feed facade views, which feed all of the downstream processes.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-evolve-remove-tables.svg)

   If the facade views don't aggregate fields or filter columns, you can
   configure your downstream processes to read from the evolved tables and
   then retire the facade views to reduce complexity, as shown in the
   following diagram:

   ![In the final configuration, both BigQuery tables and evolved tables feed into facade views, which are the only source for downstream processes.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-evolve-end-views.svg)

## Perform an initial transfer of your schema and data

This section discusses practical considerations for migrating your schema and
data from an existing data warehouse to BigQuery.

We recommend that you transfer the schema without any changes during initial
iterations of the migration. This gives you the following advantages:

- The data pipelines that feed your data warehouse don't need to be adjusted for a new schema.
- You avoid adding a new schema to the list of training material for your staff.
- You can leverage automated tools to perform the schema and data transfer.

In addition, proofs of concept (PoCs) and other data exploration activities
that leverage cloud capabilities can proceed unhindered, even while your
migration occurs in parallel.

### Choose a transfer method

You can make the initial transfer using one of several approaches.

- For Amazon Redshift and Teradata data warehouses, you can use [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) to load schema and data directly from your existing system into BigQuery. Cloud Storage is still used to stage data as part of the migration process.
- For any data warehouse, you can extract files that contain your schema and data, upload those files to Cloud Storage, and then use one of the following options to load the schema and data from those files into BigQuery:
  - [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction)
  - [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api)
  - [Batch load](https://docs.cloud.google.com/bigquery/docs/batch-loading-data)

For further considerations when choosing a data transfer method, see
[Choosing a data ingestion method](https://docs.cloud.google.com/bigquery/docs/loading-data#methods).

### Consider data transformation

Depending on your data extraction format and whether you want to trim or
enrich your data before loading it into BigQuery,
you might include a step to transform your data. You can transform the
data in the existing environment or on Google Cloud:

- If you transform the data in the current environment, consider how the available compute capacity and tooling might limit throughput. In addition, if you are enriching the data during the transformation process, consider whether you need additional transfer time or network bandwidth.
- If you transform the data on Google Cloud, see [Load data using an ETL tool](https://docs.cloud.google.com/bigquery/docs/migration/schema-data-overview#load_data_using_an_etl_tool) for more information on your options.

### Extract the existing schema and data into files

Your existing platform probably provides a tool to export data to a
vendor-agnostic format like
[Apache AVRO](https://avro.apache.org/docs/current/) or CSV.
To reduce the transfer complexity, we recommend using AVRO,
[ORC](https://orc.apache.org/)
or
[Parquet](https://parquet.apache.org/),
where schema information is embedded with the data. If you choose CSV or a
similar simple, delimited data format, you must specify the schema
separately. How you do this depends on the data transfer method you select.
For example, for batch upload, you can either specify a schema at load time
or allow auto-detection of the schema based on the CSV file contents.

As you extract these files from your existing platform, copy them into
staging storage in your existing environment.

### Upload the files to Cloud Storage

Unless you are using BigQuery Data Transfer Service to load data directly from an existing
Amazon Redshift or Teradata data warehouse, you must upload the
extracted files to a bucket in Cloud Storage. Depending on the
amount of data you're transferring and the network bandwidth available, you
can choose from the following transfer options:

- If your extracted data is in another cloud provider, use [Storage Transfer Service](https://docs.cloud.google.com/storage/transfer).
- If the data is in an on-premises environment or in a colocation facility
  that has good network bandwidth, use the
  [Google Cloud CLI](https://docs.cloud.google.com/sdk/gcloud/reference/storage).
  The gcloud CLI supports multi-threaded parallel uploads, it
  resumes after errors, and it encrypts the traffic in transit for security.

  - If you can't use the gcloud CLI, you can try a [third-party](https://docs.cloud.google.com/solutions/transferring-big-data-sets-to-gcp#third-party_tools) tool from a Google partner.
  - If your network bandwidth is limited, you can use [compression techniques](https://docs.cloud.google.com/solutions/transferring-big-data-sets-to-gcp#decrease_data_size) to limit the size of the data, or you can modify your current connection to Google Cloud to [increase the bandwidth.](https://docs.cloud.google.com/solutions/transferring-big-data-sets-to-gcp#increase_network_bandwidth)
- If you cannot achieve sufficient network bandwidth, you can perform an
  offline transfer using a
  [Transfer Appliance](https://docs.cloud.google.com/transfer-appliance).

When you create the Cloud Storage bucket and are transferring data
through the network, minimize network latency by choosing the location closest
to your data center. If possible,
[choose the location of the bucket](https://docs.cloud.google.com/bigquery/docs/locations#data-locations)
to be the same as the location of the BigQuery dataset.

For detailed information on best practices when moving data into
Cloud Storage, including estimating costs, see
[Strategies for transferring big data sets](https://docs.cloud.google.com/solutions/transferring-big-data-sets-to-gcp).

### Load the schema and data into BigQuery

Load the schema and data into BigQuery, using one of the options
discussed in [Choose a transfer method](https://docs.cloud.google.com/bigquery/docs/migration/schema-data-overview#choose_a_transfer_method).

For more information on one-time loads, see
[Introduction to loading data from Cloud Storage](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage)
in the BigQuery documentation. For more information on loads
scheduled at regular intervals, see
[Overview of Cloud Storage transfers](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer-overview)
in the BigQuery Data Transfer Service documentation.

#### Load data using an ETL tool

If your data needs further transformation as it is loaded into
BigQuery, use one of the following options:

- [Cloud Data Fusion](https://docs.cloud.google.com/data-fusion/docs). This tool graphically builds fully managed ETL/ELT data pipelines using an open source library of preconfigured connectors and transformations.
- [Dataflow](https://docs.cloud.google.com/dataflow/docs). This tool defines and runs complex data transformations and enrichment graphs using the [Apache Beam](https://beam.apache.org/) model. Dataflow is serverless and fully managed by Google, giving you access to virtually limitless processing capacity.
- [Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc/docs). This runs Apache Spark and Apache Hadoop cluster on a fully managed cloud service.
- Third-party tools. Contact one of our [partners](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-partners). They can provide effective choices when you want to externalize the building of a data pipeline.

The following diagram shows the pattern described in this section. The data
transfer tool is the
[gcloud CLI](https://docs.cloud.google.com/sdk/gcloud/reference/storage),
and there's a transformation
step that leverages Dataflow and writes directly to
BigQuery, perhaps using the Apache Beam built-in
[Google BigQuery I/O connector.](https://beam.apache.org/documentation/io/built-in/google-bigquery/)

![The existing data warehouse copies data to temporary storage on-premises. The gcloud CLI copies the data to a Cloud Storage bucket. Dataflow reads from the staging bucket and moves the data to BigQuery.](https://docs.cloud.google.com/static/bigquery/images/dw-bq-schema-and-data-transfer-overview-transfer-data-etl.svg)

After you've loaded an initial set of your data into BigQuery,
you can start taking advantage of
[BigQuery's powerful features](https://docs.cloud.google.com/bigquery/docs/introduction).

However, as when you transferred your schema, uploading your data is part of an
iterative process. Subsequent iterations can start by expanding the footprint of
the data being transferred to BigQuery. Then you can reroute
your upstream data feeds to BigQuery to eliminate the need for
keeping your existing data warehouse running. These topics are explored
in the next section.

### Validate the data

Now that your data is in BigQuery, you can verify the success
of your data transfer with the
[Data Validation Tool](https://github.com/GoogleCloudPlatform/professional-services-data-validator)
(DVT). DVT is an open source, Python CLI tool that allows you to compare
data from various sources to your target in BigQuery. You can
specify what aggregations you would like to run (for example, count, sum,
average) and the columns that these should apply to. For more information, see
[Automate Data Validation with DVT](https://cloud.google.com/blog/products/databases/automate-data-validation-with-dvt).

## Iterate on the initial transfer

This section discusses how to proceed after your initial data transfer in order
to take best advantage of BigQuery.

A subset of your data is now in BigQuery. You're preparing to
increase the footprint of the data being used in BigQuery, and
therefore to reduce the dependency on your existing data warehouse.

The method you choose for subsequent iterations depends on how important it is
for your use case to have data updated to the current second. If your data
analysts can work with data that is incorporated into the data warehouse at
recurrent intervals, a scheduled option is the way to go. You can create
scheduled transfers in a manner similar to the initial transfer. You use the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer-overview),
other ETL tools such as Google's
[Storage Transfer Service](https://docs.cloud.google.com/storage/transfer),
or
[third-party](https://docs.cloud.google.com/solutions/transferring-big-data-sets-to-gcp#third-party_tools)
implementations.

If you use BigQuery Data Transfer Service, first you decide which tables to move. Then
you create a table name pattern to include those tables. Finally you set a
recurrent schedule for when to run the transfer.

On the other hand, if your use case requires near-instant access to new data,
you require a streaming approach. You have two options:

- Set up a load pipeline with Google Cloud products. Google provides a set of [streaming Dataflow templates](https://docs.cloud.google.com/dataflow/docs/guides/templates/provided-streaming) to facilitate this task.
- Use a solution from one of our [partners](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-partners).

In the short term, increasing the footprint of your BigQuery
data and of using it for downstream process should be focused on satisfying your
top-priority use cases, with the medium-term goal of moving off your existing data
warehouse. Use the initial iterations wisely and don't spend a lot of
resources perfecting the ingestion pipelines from your existing data warehouse
into BigQuery. Ultimately, you'll need to adapt those pipelines
not to use the existing data warehouse.

### Optimize the schema

Simply migrating your tables as-is to BigQuery allows you to
take advantage of its unique features. For instance, there is no need for
rebuilding indexes, reshuffling data blocks (*vacuuming*) or any downtime or
performance degradation because of version upgrades.

However, keeping the data warehouse model intact beyond the initial iterations
of the migration also has disadvantages:

- Existing issues and technical debt associated with the schema remain.
- Query optimizations are limited, and they might need to be redone if the schema is updated at a later stage.
- You don't take full advantage of other BigQuery features, such as nested and repeated fields, partitioning, and clustering.

As you move towards doing a final transfer, we recommend that you improve
system performance by applying partitioning and clustering to the tables
you create in your schema.

#### Partitioning

BigQuery lets you divide your data into segments, called
[*partitions*](https://docs.cloud.google.com/bigquery/docs/partitioned-tables),
that make it easier and more efficient to manage and query your data. You can
partition your tables based on a
[`TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type)
or
[`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type)
column, or BigQuery can add pseudocolumns to automatically
partition your data during ingestion. Queries that involve smaller partitions
can be more performant because they scan only a subset of the data, and can
reduce costs by minimizing the number of bytes being read.

Partitioning does not impact the existing structure of your tables. Therefore,
you should consider creating partitioned tables even if your schema is not
modified.

#### Clustering

In BigQuery, no indexes are used to query your data.
BigQuery's performance is optimized by its underlying
model, storage and query techniques, and massively parallel architecture.
When you run a query, the more data is being processed,
the more machines are added to scan data and aggregate results concurrently.
This technique scales well to huge datasets, whereas rebuilding indexes does
not.

Nevertheless, there is room for further query optimization with techniques like
[clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables).
With clustering, BigQuery automatically sorts your data based on
the values of a few columns that you specify and colocates them in optimally
sized blocks. Clustering improves query performance compared to not using
clustering. With clustering, BigQuery can better estimate the
cost of running the query than without clustering. With clustered columns,
queries also eliminate scans of unnecessary data, and can calculate aggregates
quicker because the blocks colocate records with similar values.

Examine your queries for columns frequently used for filtering and create your
tables with clustering on those columns. For more information about clustering,
see [Introduction to clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables).

### Denormalization

Data migration is an iterative process.
Therefore, once you've moved your initial schema to the cloud, performed light
optimizations, and tested a few key use cases, it might be time to explore
additional capabilities that benefit from the underlying design of
BigQuery. These include nested and repeated fields.

Data warehouse schemas have historically used the following models:

- [Star schema](https://wikipedia.org/wiki/Star_schema). This is a denormalized model, where a fact table collects metrics such as order amount, discount, and quantity, along with a group of keys. These keys belong to dimension tables such as customer, supplier, region, and so on. Graphically, the model resembles a star, with the fact table in the center surrounded by dimension tables.
- [Snowflake schema](https://wikipedia.org/wiki/Snowflake_schema). This is similar to the star schema, but with its dimension tables normalized, which gives the schema the appearance of a unique snowflake.

BigQuery supports both star and snowflake schemas, but its
native schema representation is neither of those two. It uses
[nested and repeated fields](https://docs.cloud.google.com/bigquery/docs/nested-repeated)
instead for a more natural representation of the data. For more information, see
the
[example schema](https://docs.cloud.google.com/bigquery/docs/nested-repeated#example_schema)
in the BigQuery documentation.

Changing your schema to use nested and repeated fields is an excellent
evolutionary choice. It reduces the number of joins required for your queries,
and it aligns your schema with the BigQuery internal data
representation. Internally, BigQuery organizes data using the
[Dremel model](https://research.google/pubs/dremel-interactive-analysis-of-web-scale-datasets/)
and stores it in a columnar storage format called
[Capacitor](https://cloud.google.com/blog/products/bigquery/inside-capacitor-bigquerys-next-generation-columnar-storage-format).

To decide the best denormalization approach for your case, consider
the
[using nested and repeated fields](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-nested)
in BigQuery as well as the techniques for
[handling schema changes](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

## What's next

Learn more about the following steps in data warehouse migration:

- [Migration overview](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview)
- [Migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment)
- [Data pipelines](https://docs.cloud.google.com/bigquery/docs/migration/pipelines)
- [Batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator)
- [Interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
- [Data security and governance](https://docs.cloud.google.com/bigquery/docs/data-governance)
- [Data validation tool](https://github.com/GoogleCloudPlatform/professional-services-data-validator#data-validation-tool)

You can also learn about moving from specific
data warehouse technologies to BigQuery:

- [Migrating from Netezza](https://docs.cloud.google.com/architecture/dw2bq/netezza/netezza-bq-migration-guide)
- [Migrating from Oracle](https://docs.cloud.google.com/bigquery/docs/migration/oracle-migration)
- [Migrating from Amazon Redshift](https://docs.cloud.google.com/bigquery/docs/migration/redshift)
- [Migrating from Teradata](https://docs.cloud.google.com/bigquery/docs/migration/teradata)
- [Migrating from Snowflake](https://docs.cloud.google.com/architecture/dw2bq/snowflake/snowflake-bq-migration-guide)