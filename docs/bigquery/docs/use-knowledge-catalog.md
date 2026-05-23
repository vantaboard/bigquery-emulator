# Use Knowledge Catalog with BigQuery

Knowledge Catalog, (formerly known as Dataplex Universal Catalog),
interacts with BigQuery as a
central data governance and agentic access layer for BigQuery's
metadata. For more information, see the
[Knowledge Catalog overview](https://docs.cloud.google.com/dataplex/docs/introduction).

## How do I use Knowledge Catalog with BigQuery?

Knowledge Catalog interacts with BigQuery in the following
ways:

### Automated metadata ingestion

Knowledge Catalog automatically discovers and indexes technical metadata
from BigQuery assets. This includes the following:

- **Asset types** : [datasets, tables, views, models, routines, connections, and
  linked datasets](https://docs.cloud.google.com/bigquery/docs/resource-hierarchy).
- **BigQuery sharing** : [exchanges and listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction) from BigQuery sharing (formerly Analytics Hub).
- **Real-time updates** : the system supports near real-time ingestion and provides [*metadata change feeds*](https://docs.cloud.google.com/dataplex/docs/metadata-change-feeds-overview) using Pub/Sub to notify downstream systems of schema changes or deletions in BigQuery.
- **Dark data discovery** : Knowledge Catalog can [scan unstructured files](https://docs.cloud.google.com/dataplex/docs/data-insights-unstructured-data) (like PDFs in Cloud Storage), extract entities, and convert them into queryable assets in BigQuery. This capability makes previously inaccessible "dark data" available for BigQuery-based analytics and AI grounding.

### Metadata representation and enrichment

- **Entries** : each BigQuery table or asset is represented as [an *entry*](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources) in the catalog, rather than the entire table; for example, `project.dataset.table`.
- **Column-level metadata** : individual columns or fields are represented as *[paths](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata)*, allowing you to attach specific metadata, like PII markers or data quality scores, to individual fields within a BigQuery table rather than just the table itself.
- **Aspects** : technical metadata is enriched with [*aspects*](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspects), which add business context to data, such as ownership, data quality, and documentation.
- **Data products** : You can package related BigQuery assets into [data products](https://docs.cloud.google.com/dataplex/docs/data-products-overview)---for example, ecommerce business data---which have shared access and governance constraints.

### Data discovery and search

- **Semantic search** : users can use natural language to [search](https://docs.cloud.google.com/dataplex/docs/semantic-search) for BigQuery data, which is especially useful for data scientists and AI agents to find trusted data products using long or complex queries.
- **Name translation** : for easier programmatic lookup, the system allows for [translating](https://docs.cloud.google.com/dataplex/docs/samples/dataplex-lookup-entry#code-sample) BigQuery SQL names, or fully qualified names, into Knowledge Catalog entry names.

### Agentic access and grounding

- **Agentic access** : AI agents can discover and adaptively use Knowledge Catalog tools through [a local or remote MCP server](https://docs.cloud.google.com/dataplex/docs/mcp-overview).
- **Context for AI Agents** : Knowledge Catalog curates [a context graph](https://docs.cloud.google.com/dataplex/docs/introduction#sample-workflows) that links BigQuery datasets with business semantics, helping reduce AI hallucinations by ensuring models use enterprise-approved data.

### Governance and compliance

- **Data Lineage** : Knowledge Catalog automatically [tracks](https://docs.cloud.google.com/dataplex/docs/about-data-lineage) how data flows and transforms into and out of BigQuery tables. This capability is critical for auditing sensitive information, like PII, across the data estate.
- **Access Control:** metadata management is integrated with [Identity and Access Management (IAM)](https://docs.cloud.google.com/bigquery/docs/access-control) and [VPC Service Controls](https://docs.cloud.google.com/bigquery/docs/vpc-sc) to ensure that discovery and access to BigQuery metadata follow organizational security policies.

## Migration considerations

Migrating to Knowledge Catalog from the deprecated Data Catalog
involves several steps. Standard metadata from BigQuery
(like datasets, tables, views) is automatically available in Knowledge Catalog,
so the migration process primarily focuses on custom metadata, API usage, and
user interface defaults.

The following are the main points to consider when migrating:

### Understand the change

Knowledge Catalog offers enhanced features for metadata management,
governance, and discovery when compared to Data Catalog.
Knowledge Catalog uses a different API (the Knowledge Catalog API) and
has a slightly different data model; for example, Knowledge Catalog uses
[aspects and aspect types](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata) instead of
tags and tag templates.

### Assess current data catalog usage

- **No Custom Metadata**: if you only relied on Knowledge Catalog for the automatic ingestion and discovery of standard BigQuery metadata without creating any custom tags, tag templates, custom entries, or entry groups, then the transition is straightforward. You can start using the Knowledge Catalog interface immediately.
- **Custom Metadata or Programmatic Use**: if you created custom tags or templates, custom entries, or use the Data Catalog API, client libraries, Google Cloud CLI commands, or Terraform, then you need a more structured transition.

### Specific BigQuery considerations

- **Automatic Ingestion**: technical metadata from BigQuery assets (datasets, tables, views, models, and routines) continue to be automatically ingested into Knowledge Catalog, as they were with Dataplex Universal Catalog.
- **Policy Tags** : policy tags used for BigQuery column-level access control are *not* deprecated and their management remains within BigQuery.
- **Lineage** : data lineage for BigQuery operations is surfaced within Knowledge Catalog. For more information about data lineage, see [Track data lineage for a BigQuery table](https://docs.cloud.google.com/dataplex/docs/track-lineage-quickstart).

### Follow the transition guide

To migrate to Knowledge Catalog, follow the steps in
[Transition from Data Catalog to Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/transition-to-dataplex-catalog).

To update programmatic workflows to the Knowledge Catalog API, see
[Map Data Catalog API methods to Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/catalog-overview#map-api-methods).

## What's next

Learn more about Knowledge Catalog:

- [Knowledge Catalog use cases](https://docs.cloud.google.com/dataplex/docs/use-cases)
- [Knowledge Catalog FAQ](https://docs.cloud.google.com/dataplex/docs/faq)
- [About metadata management in Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/catalog-overview)