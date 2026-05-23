# Introduction to data governance in
BigQuery

BigQuery has built-in governance capabilities that simplify how
you discover, manage, monitor, govern, and use your data and AI assets.

Administrators, data stewards, data governance managers, and data custodians can
use the governance capabilities in BigQuery to do the following:

- Discover data.
- Curate data.
- Gather and enrich metadata.
- Manage data quality.
- Ensure that data is used consistently and in compliance with organizational policies.
- Share data at scale and in a secure fashion.

BigQuery governance capabilities are powered by
[Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/catalog-overview), a centralized inventory of all data assets in your organization.
Knowledge Catalog holds business, technical, and operational metadata for
all of your data. It helps you discover relationships and semantics in the
metadata by applying artificial intelligence and machine learning.

Lakehouse runtime catalog lets you use
multiple data processing engines to query a single copy of data with a single
schema, without data duplication. The data processing engines that you can use
include BigQuery, Apache Spark, Apache
Flink, and Apache Hive. Your data can be stored in locations like
BigQuery storage tables, Apache Iceberg managed tables, or
BigLake external tables.

BigQuery supports an end-to-end data lifecycle, from
discovery to use of data. Governance features are also available in
Knowledge Catalog.

## Data discovery

BigQuery discovers data across the organization in Google Cloud,
whether the data is in BigQuery, Spanner, Cloud SQL,
Pub/Sub, or Cloud Storage. The metadata is automatically
extracted and stored in Knowledge Catalog. For
example, you can extract metadata for structured
and unstructured data from Cloud Storage, and you can automatically
create query-ready BigLake tables at scale. This lets you perform
analytics with an open source engine without data duplication.

You can also extract and catalog metadata from third-party data sources using
custom connectors.

BigQuery offers the following data discovery
capabilities:

- **Search.** Search for data and AI resources across projects and the organization. Within BigQuery in the Google Cloud console, use [semantic search](https://docs.cloud.google.com/bigquery/docs/search-resources) ([Preview](https://cloud.google.com/products#product-launch-stages)) to search for resources by using everyday language. Or, find resources by using [keyword search](https://docs.cloud.google.com/dataplex/docs/search-assets) in Knowledge Catalog.
- **[Automatic discovery of Cloud Storage
  data](https://docs.cloud.google.com/bigquery/docs/automatic-discovery).** Scan for data in Cloud Storage buckets to extract and then catalog metadata. Automatic discovery creates tables for both structured and unstructured data.
- **[Metadata import](https://docs.cloud.google.com/dataplex/docs/managed-connectivity-overview).** Import metadata at scale from third-party systems into Knowledge Catalog. You can build custom connectors to extract data from your data sources, and then run managed connectivity pipelines that orchestrate the metadata import workflow.
- **[Metadata export](https://docs.cloud.google.com/dataplex/docs/export-metadata).** Export metadata at scale out of Knowledge Catalog. You can analyze the exported metadata with BigQuery, or integrate the metadata into custom applications or programmatic processing workflows.

## Curation and data stewardship

To improve the discoverability and usability of data, data stewards and
administrators can use BigQuery to review, update, and analyze
metadata. BigQuery data curation and stewardship capabilities
help you ensure that your data is accurate, consistent, and aligned with your
organization's policies.

BigQuery offers the following data curation and
stewardship capabilities:

- **[Business glossary](https://docs.cloud.google.com/dataplex/docs/create-glossary).** Improve context, collaboration, and search by defining your organization's terminology in a glossary. Identify data stewards for the terms, and attach terms to data asset fields.
- **[Data insights](https://docs.cloud.google.com/bigquery/docs/data-insights).** Gemini uses metadata to generate natural language questions about your table and the SQL queries to answer them. These data insights help you uncover patterns, assess data quality, and perform statistical analysis.
- **[Data profiling](https://docs.cloud.google.com/bigquery/docs/data-profile-scan).** Identify common statistical characteristics of the columns in BigQuery tables to understand and analyze your data more effectively.
- **[Data quality](https://docs.cloud.google.com/bigquery/docs/data-quality-scan).** Define and run data quality checks across tables in BigQuery and Cloud Storage, and apply regular and ongoing data controls in BigQuery environments.
- **[Data lineage](https://docs.cloud.google.com/dataplex/docs/about-data-lineage).** Track how data moves through your systems: where it comes from, where it's passed to, and what transformations are applied to it. BigQuery supports data lineage at the table- and column-levels.

### Next steps for curation and data stewardship

The following table outlines next steps that you can take to learn more about
curation and data stewardship features:

| Experience level | Learning path |
|---|---|
| New cloud users | - Run a [data profile scan](https://docs.cloud.google.com/bigquery/docs/data-profile-scan) to gain insights about your data, including the limits or averages of your data. |
| Experienced cloud users | - Enable [data lineage](https://docs.cloud.google.com/dataplex/docs/about-data-lineage#auto-lineage-bq-support) in your BigQuery project to automatically record lineage information for BigQuery operations like load, copy, and data modifications. - Set up a recurring [data quality scan](https://docs.cloud.google.com/bigquery/docs/data-quality-scan) to alert you to possible data issues by using [predefined scan rules](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview#predefined-rules). - Set up [custom data quality rules](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview#supported-custom-sql-rule-types) for your data quality scans so that your scans are tailored to your specific needs. |

## Security and access control

Data access management is the process of defining, enforcing, and monitoring
the rules and policies governing who has access to data. Access management
ensures that data is only accessible to those who are authorized to access it.

BigQuery offers the following security and access control
capabilities:

- **[Identity and Access Management (IAM)](https://docs.cloud.google.com/bigquery/docs/access-control).** IAM lets you control who has access to your BigQuery resources, such as projects, datasets, tables, and views. You can grant IAM roles to users, groups, and service accounts. These roles define what they can do with your resources.
- **[Column-level access
  controls](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro)** and **[row-level access controls](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro).** Column-level and row-level access controls let you restrict access to specific columns and rows in a table, based on user attributes or data values. This control lets you implement fine-grained access to help protect sensitive data from unauthorized access.
- **[Data transfer management](https://docs.cloud.google.com/bigquery/docs/vpc-sc).** VPC Service Controls lets you create perimeters around Google Cloud resources and control access to those resources based on your organization's policies.
- **[Audit logs](https://docs.cloud.google.com/bigquery/docs/introduction-audit-workloads).** Audit logs provide you with a detailed record of user activity and system events in your organization. These logs help you enforce data governance policies and identify potential security risks.
- **[Data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro).** Data masking lets you obscure sensitive data in a table while still permitting authorized users to access the surrounding data. Data masking can also obscure data that matches sensitive data patterns, safeguarding against accidental data disclosure.
- **[Encryption](https://docs.cloud.google.com/bigquery/docs/encryption-at-rest).** BigQuery automatically encrypts all data at rest and in transit, while letting you customize your encryption settings to meet your specific requirements.

### Next steps for security and access control

The following table outlines next steps that you can take to learn more about
access control features:

| Experience level | Learning path |
|---|---|
| New cloud users | - Take a look at [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined) in BigQuery and consider how to assign them based on the [principle of least privilege](https://wikipedia.org/wiki/Principle_of_least_privilege). - Learn how Google encrypts your data [at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption) and [in transit](https://docs.cloud.google.com/docs/security/encryption-in-transit) by default. |
| Experienced cloud users | - For greater flexibility and granularity in managing your permissions, consider [creating custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) that match your needs. - Add [row](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) and [column controls](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) to help control access to specific rows and columns in your tables. - Establish an access perimeter around your Google Cloud resources by [setting up VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/set-up-service-perimeter-verify-access). - Add [column-level data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking) to your table to share information through your organization without revealing sensitive data. - Use [Sensitive Data Protection](https://docs.cloud.google.com/sensitive-data-protection/docs/data-profiles) to scan your data for sensitive and high-risk information, such as personally identifiable information (PII), financial data, and health information. |

## Shared data and insights

BigQuery lets you share data and insights at scale within and
across organizational boundaries. It has a robust security and privacy framework
through a built-in data exchange platform. Using
[BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction),
you can discover, access, and consume a data library that's curated by a wide
selection of data providers.

BigQuery offers the following sharing capabilities:

- **[Share more than data](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).** You can share a wide range of data and AI assets such as BigQuery datasets, tables, views, real-time streams with Pub/Sub topics, SQL stored procedures, and BigQuery ML models.
- **[Access Google datasets](https://cloud.google.com/datasets).** Augment your analytics and ML initiatives with Google datasets from Search Trends, DeepMind WeatherNext models, Google Maps Platform, Google Earth Engine, and more.
- **[Integrate with data governance
  principles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles).** Data owners retain control over their data and have the ability to define and configure rules or policies to restrict access and usage.
- **[Live, zero-copy data sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_resources).** Data is shared in place with no integration, data movement, or replication needed, ensuring analysis is based on the latest information. Linked datasets created are a live pointer to the shared asset.
- **[Enhance security posture](https://docs.cloud.google.com/bigquery/docs/analytics-hub-vpc-sc-rules).** You can use access controls to reduce overprovisioning access, including built-in VPC Service Controls support.
- **[Increase visibility with provider usage
  metrics](https://docs.cloud.google.com/bigquery/docs/analytics-hub-monitor-listings).** Data publishers can view and monitor usage for shared assets such as the number of jobs executed, total bytes scanned, and subscribers for each organization.
- **[Collaborate on sensitive data with data clean
  rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms).** Data clean rooms provide a security-enhanced environment in which multiple parties can share, join, and analyze their data assets without moving or revealing the underlying data.
- **[Built on BigQuery](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).** You can build on the scalability and massive processing capabilities in BigQuery, allowing for large scale collaborations.

### Next steps for sharing

The following table outlines next steps that you can take to learn more about
sharing features:

| Experience level | Learning path |
|---|---|
| New cloud users | - Learn how to create and manage [exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges) and [listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings) to start sharing within or outside of your organization. |
| Experienced cloud users | - Share real-time streaming data with [Pub/Sub topics](https://docs.cloud.google.com/bigquery/docs/analytics-hub-stream-sharing). - Share and collaborate on sensitive data with [data clean rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms). - Further protect data exfiltration by configuring [VPC Service Controls](https://docs.cloud.google.com/bigquery/docs/analytics-hub-vpc-sc-rules) around your shared assets. - [Commercialize](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace) and sell your assets on Google Cloud Marketplace |

## What's next

- Learn about [authentication at Google](https://docs.cloud.google.com/docs/authentication).
- Learn about [data deletion on Google Cloud](https://docs.cloud.google.com/docs/security/deletion).
- Learn more about [IAM best practices](https://docs.cloud.google.com/iam/docs/using-iam-securely).
- Learn the [resource hierarchy on Google Cloud](https://docs.cloud.google.com/resource-manager/docs/cloud-platform-resource-hierarchy).
- Learn about [IAM on Google Cloud](https://docs.cloud.google.com/iam/docs/overview).