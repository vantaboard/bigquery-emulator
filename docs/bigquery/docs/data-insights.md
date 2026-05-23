# Data insights overview

This document provides an overview of data insights, a Gemini in
BigQuery feature that helps accelerate initial exploration and analysis
when facing new or unfamiliar data. Data insights automatically generates
descriptions, relationship graphs, and SQL queries, along with suggested
questions in natural language, from your table and dataset metadata. This
information helps you quickly understand data structure, content, and
relationships without extensive manual setup.

> [!NOTE]
> **Note:** To give feedback for this feature, contact [dataplex-data-insights-help@google.com](mailto:dataplex-data-insights-help@google.com).

## Before you begin

Data insights are generated using
[Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/overview).
To start generating insights, you must first
[set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini).

> [!NOTE]
> **Note** : Gemini in BigQuery is part of Gemini for Google Cloud and doesn't support the same compliance and security offerings as BigQuery. You should only set up Gemini in BigQuery for BigQuery projects that don't require [compliance offerings that aren't supported by Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/discover/certifications). For information about how to turn off or prevent access to Gemini in BigQuery, see [Turn off Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#turn-off).

## Types of data insights

You can generate data insights at the table or dataset level:

- **For tables:** Gemini generates natural language questions
  and their SQL equivalents to help you understand data within a single table.
  Through table insights, you can detect data patterns, anomalies, outliers,
  or quality issues within a table. Gemini also generates table
  and column descriptions.

- **For datasets:**
  ([Preview](https://cloud.google.com/products#product-launch-stages))
  Gemini generates an interactive relationship graph showing
  cross-table relationships and cross-table SQL queries to help you understand
  how tables are related in a dataset. Through relationship graphs, you can
  discover how data is derived, which can help with quality, consistency, or
  redundancy issues. Through cross-table queries, you can find broader
  relationships. For example, you can calculate revenue by customer segment by
  leveraging data in a sales table and a customer table.

To investigate further, you can ask follow-up questions in [data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas).

### Table insights

Table insights help you understand the content, quality, and patterns within a
single BigQuery table. For example, by generating queries that
perform statistical analysis, you can use table insights to detect data
patterns, anomalies, and outliers. Table insights can also help you detect
quality issues, especially when
[data profile scans](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview) are available for
a table. When you generate insights for a table, Gemini provides
table description, column descriptions, and profile scan output based on the
table's metadata. The following options are available:

- **Generate queries:** suggests natural language questions and provides the corresponding SQL queries to answer them. This helps you uncover patterns, assess data quality, and perform statistical analysis without writing SQL from scratch.
- **Generate descriptions:** generates descriptions for the table and its columns. Gemini uses profile scan output (if available) to ground the generated descriptions. You can review, edit, and publish these descriptions to Knowledge Catalog to improve data discoverability and documentation.

### Dataset insights

Dataset insights help you understand the relationships and join paths across
multiple tables within a BigQuery dataset, which provides a
holistic view of the dataset's contents. When you generate insights for a
dataset, Gemini provides the following:

- **Dataset description:** provides an AI-generated summary of the dataset.
- **Relationships:** displays a visual, interactive map showing relationships between tables within the dataset. You can hover over connections to see relationship details, such as join keys.
- **Relationship table:** presents a tabular view of relationships between tables, including foreign keys and inferred joins. Relationships can be schema-defined (from primary and foreign key constraints), usage-based (from query logs), or Gemini infers them based on table and column names and descriptions.
- **Query recommendations:** offers sample SQL queries that demonstrate how to join data across different tables, based on the identified relationships.

## Example of table data insights

Consider a table called `telco_churn` with columns such as `CustomerID`,
`Tenure`, `InternetService`, `Contract`, `MonthlyCharges`, and `Churn`.
The following table describes the table's metadata.

| Field name | Type |
|---|---|
| `CustomerID` | `STRING` |
| `Gender` | `STRING` |
| `Tenure` | `INT64` |
| `InternetService` | `STRING` |
| `StreamingTV` | `STRING` |
| `OnlineBackup` | `STRING` |
| `Contract` | `STRING` |
| `TechSupport` | `STRING` |
| `PaymentMethod` | `STRING` |
| `MonthlyCharges` | `FLOAT64` |
| `Churn` | `BOOL` |

Data insights generates the following sample queries for this table:

- Identify customers who have subscribed to all premium services and have been customers for more than 50 months.

      SELECT
        CustomerID,
        Contract,
        Tenure
      FROM
        agentville_datasets.telco_churn
      WHERE
        OnlineBackup = 'Yes'
        AND TechSupport = 'Yes'
        AND StreamingTV = 'Yes'
        AND Tenure > 50;

- Identify which internet service has the most churned customers.

      SELECT
        InternetService,
        COUNT(DISTINCT CustomerID) AS customers
      FROM
        agentville_datasets.telco_churn
      WHERE
        Churn = TRUE
      GROUP BY
        InternetService
      ORDER BY
        customers DESC
      LIMIT 1;

## Example of dataset data insights

Consider a dataset containing `order_items` and `inventory_items` tables. Dataset
insights can infer that `order_items.inventory_item_id` relates to
`inventory_items.id`.

Based on these relationships, Gemini might generate
the following cross-table query:

Identify the top 5 product categories with the highest average sale price and
their corresponding average cost.

    SELECT
      ii.product_category,
      AVG(oi.sale_price) AS avg_sale_price,
      AVG(ii.cost) AS avg_cost
    FROM
      `ecommerce_data.order_items` AS oi
    JOIN
      `ecommerce_data.inventory_items` AS ii
    ON oi.inventory_item_id = ii.id
    GROUP BY
      ii.product_category
    ORDER BY
      avg_sale_price DESC
    LIMIT 5;

## Data insights workflows

This section outlines key workflows that different user roles can perform using
the data insights feature in BigQuery.

### Workflows for data consumers

These workflows focus on tasks for data analysts, business analysts,
and other users who need to find, understand, and analyze data.

- **Understand a BigQuery table:** quickly grasp the schema, content, and
  potential uses of a specific table. You can perform the following tasks after selecting a table in BigQuery Studio:

  - Review auto-generated table and column descriptions.

  - Examine suggested natural language questions and equivalent SQL
    queries to understand data nuances.

  - Adapt and run suggested queries to start analysis.

  For more information about generating and viewing table insights,
  see [Generate table insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights).
- **Explore an entire dataset:** discover the relationships between
  tables within a dataset and understand its overall structure. You can
  perform the following tasks after selecting a dataset in BigQuery Studio:

  - Generate and view dataset insights.

  - Use the interactive relationship graph to visualize table connections.

  - Analyze the relationship table for join keys and connection types
    (schema-defined, usage-based, LLM inferred).

  - Use suggested cross-table SQL queries to query multiple tables
    effectively.

  For more information about generating and viewing dataset insights,
  see [Generate dataset insights](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights).

### Workflows for data producers

These workflows are for data engineers, analytics engineers, and others who
build and manage data assets.

- **Generate baseline data documentation:** automatically create and maintain
  essential metadata descriptions. You can perform the following tasks:

  - After table creation or modification, trigger data insights to generate
    table and column descriptions. You can also generate these descriptions
    at scale by using the [Knowledge Catalog
    automated metadata generation
    API](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#add-aspects).

  - Review and refine the AI-generated text to ensure technical accuracy and
    business relevance.

  For more information about generating table and column descriptions,
  see [Generate table insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights).
- **Enhance dataset comprehension for users**: Make it easier for
  consumers to understand and use the datasets provided. You can perform the
  following tasks:

  - Generate dataset insights for key datasets, especially those with
    complex relationships.

  - Ensure data profile scans run on tables to provide rich context
    for more accurate and useful insights.

  For more information, see [Generate dataset insights](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights)
  and [Ground insights to data profiling results](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview).

### Workflows for data stewards

These workflows support data stewards and governance teams in maintaining data
integrity and trust.

- **Validate and audit AI-generated metadata:** ensure the accuracy and
  reliability of the metadata produced by data insights. You can perform the
  following tasks:

  - Routinely review descriptions and relationships generated by the
    insights feature.

  - Cross-reference inferred relationships in the relationship graph with
    established data models and business logic.

  - Review and fix inaccuracies in the AI-generated metadata.

  For more information, see [Generate table insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights)
  and [Generate dataset insights](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights).

## Pricing

For details about pricing for this feature,
see [Gemini in BigQuery pricing overview](https://docs.cloud.google.com/gemini/pricing#gemini-in-bigquery-pricing).

## Quotas and limits

For information about quotas and limits for this feature, see
[Quotas for Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/quotas#bigquery).

## Limitations

Data insights have the following limitations:

- Data insights are available for BigQuery tables,
  BigLake tables, external tables, and views.

- For multi-cloud customers, data from other clouds is not available.

- Data insights doesn't support `GEO` or `JSON` column types.

- Insights runs don't guarantee the presentation of queries every time. To
  increase the likelihood of generating more engaging queries, re-initiate the
  insights pipeline.

- For tables with column-level access control and restricted user permissions,
  you can generate insights if you have read access to all columns of the
  table. To run the generated queries, you must have sufficient
  [permissions](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#roles).

- Gemini generates column descriptions for a maximum of 350 columns in a
  table.

- For dataset insights, you can't edit relationships in the relationship
  graph.

- Generating new dataset insights overwrites the previous insights for that
  dataset.

- Dataset insights don't support linked datasets.

## Locations

You can use data insights in all
[BigQuery locations](https://docs.cloud.google.com/bigquery/docs/locations). To learn about
where Gemini in BigQuery processes your data, see
[Where Gemini in BigQuery processes your data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).

## What's next

- Learn how to [generate table insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights).

- Learn how to [generate dataset insights](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights).

- Learn more about
  [Knowledge Catalog data profiling](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview).

- Learn how to [write queries with Gemini assistance in
  BigQuery](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini).

- Learn more about [Gemini in
  BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/overview).

- Learn how to iterate on query results with natural
  language questions by using [Data Canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas).