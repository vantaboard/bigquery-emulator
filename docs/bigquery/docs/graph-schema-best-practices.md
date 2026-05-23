# Graph schema best practices

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

> [!NOTE]
> **Note:** To request support or provide feedback for this feature, send email to [bq-graph-preview-support@google.com](mailto:bq-graph-preview-support@google.com).

This document provides best practices for defining your graph schema to
improve your graph query performance.

## Scope your property definitions

Properties are key-value pairs that provide additional information attached to
nodes or edges. We recommend that you only include necessary properties in nodes
or edges, and avoid using the `PROPERTIES ALL COLUMNS` syntax or the default
syntax that attaches all columns from the node or edge tables to the property
list. Having many properties in nodes or edges might cause unnecessary column
scans in graph queries, which degrades performance.

To restrict the properties that you include in a node or edge definition, use
the `PROPERTIES` keyword when you
[define element properties](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#element_table_property_definition)
in your [`CREATE PROPERTY GRAPH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_create_graph).

The following node table definition restricts the properties for the `Person`
node table to `id` and `name`:

    NODE TABLES (
      graph_db.Person PROPERTIES (id, name)
    )

## Define primary and foreign key constraints on graph nodes and edges

BigQuery can use
[primary and foreign key constraints](https://docs.cloud.google.com/bigquery/docs/primary-foreign-keys)
on your node and edge tables to optimize your graph
queries by reducing unnecessary table scans.
However, BigQuery doesn't enforce primary or foreign key
constraints on tables. If your application can't guarantee
referential integrity or uniqueness on primary keys, then using primary or
foreign keys for query optimization might lead to incorrect query results.

The following example defines primary and foreign key constraints on the node
tables `Person` and `Account`, and the edge table `PersonOwnAccount`:

    CREATE OR REPLACE TABLE graph_db.Person (
      id               INT64,
      name             STRING,
      birthday         TIMESTAMP,
      country          STRING,
      city             STRING,
      PRIMARY KEY (id) NOT ENFORCED
    );

    CREATE OR REPLACE TABLE graph_db.Account (
      id               INT64,
      create_time      TIMESTAMP,
      is_blocked       BOOL,
      nick_name        STRING,
      PRIMARY KEY (id) NOT ENFORCED
    );

    CREATE OR REPLACE TABLE graph_db.PersonOwnAccount (
      id               INT64 NOT NULL,
      account_id       INT64 NOT NULL,
      create_time      TIMESTAMP,
      PRIMARY KEY (id, account_id) NOT ENFORCED,
      FOREIGN KEY (id) references graph_db.Person(id) NOT ENFORCED,
      FOREIGN KEY (account_id) references graph_db.Account(id) NOT ENFORCED
    );

## What's next

- For more information about graphs, see [Introduction to BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview).
- For more information about defining a schema, see [Schema overview](https://docs.cloud.google.com/bigquery/docs/graph-schema-overview).
- For more information about writing graph queries, see [Graph query overview](https://docs.cloud.google.com/bigquery/docs/graph-query-overview).