# Introduction to the BigQuery entity resolution framework

This document describes the architecture of the BigQuery entity
resolution framework. Entity resolution matches records across
shared data where no common identifier exists or augments shared data using an
identity service from a Google Cloud partner.

This document is for entity resolution end users and identity providers. For
implementation details, see
[Configure and use entity resolution in BigQuery](https://docs.cloud.google.com/bigquery/docs/entity-resolution-setup).

You can use BigQuery entity resolution for data prepared before
you contribute it to a
[data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms).
Entity resolution is available in on-demand and capacity pricing models and in
all BigQuery editions.

## Benefits

End users gain the following benefits from entity resolution:

- Resolve entities in place without data transfer fees. A subscriber or Google Cloud partner matches your data to their identity table and writes the match results to a dataset in your Google Cloud project.
- Avoid managing extract, transform, and load (ETL) jobs.

Identity providers gain the following benefits from entity resolution:

- Offer entity resolution as a managed software as a service (SaaS) offering on [Google Cloud Marketplace](https://docs.cloud.google.com/marketplace/docs/partners/integrated-saas).
- Use proprietary identity graphs and match logic without revealing them to users.

## Architecture

BigQuery implements entity resolution using remote function
calls that activate entity resolution processes in an identity provider's
environment. Your data isn't copied or moved during this process.
The following diagram and explanation describe the entity resolution workflow:

![A diagram showing two main sections: an end-user project and an identity provider project.](https://docs.cloud.google.com/static/bigquery/images/entity-resolution-arch-diagram.svg)

1. The end user grants the identity provider's service account read access to their input dataset and write access to their output dataset.
2. The user calls the remote function that matches their input data with the provider's identity graph data. The remote function passes matching parameters to the provider.
3. The provider's service account reads and processes the input dataset.
4. The provider's service account writes the entity resolution results to the user's output dataset.

The following sections describe the end-user components and provider projects.

### End-user components

End-user components include the following:

- **Remote function call**: a call that runs a procedure defined and implemented by the identity provider. This call starts the entity resolution process.
- **Input dataset**: the source dataset that contains the data to be matched. Optionally, the dataset can contain a metadata table with additional parameters. Providers specify schema requirements for input datasets.
- **Output dataset**: the destination dataset where the provider stores the matched results as an output table. Optionally, the provider can write a job status table that contains entity resolution job details to this dataset. The output dataset can be the same as the input dataset.

### Identity provider components

Identity provider components include the following:

- **Control plane** : contains a [BigQuery remote function](https://docs.cloud.google.com/bigquery/docs/remote-functions) that orchestrates the matching process. This function can be implemented as a [Cloud Run](https://docs.cloud.google.com/run/docs/overview/what-is-cloud-run) job, or a [Cloud Run function](https://docs.cloud.google.com/functions/docs/concepts/overview). The control plane can also contain other services, such as authentication and authorization.
- **Data plane** : contains the identity graph dataset and the stored procedure that implements the provider matching logic. The stored procedure can be implemented as a [SQL stored procedure](https://docs.cloud.google.com/bigquery/docs/procedures) or an [Apache Spark stored procedure](https://docs.cloud.google.com/bigquery/docs/spark-procedures). The identity graph dataset contains the tables that the end-user data is matched against.

> [!NOTE]
> **Note:** Identity graphs can also be stored in some external databases.

## What's next

- Learn how to [configure and use entity resolution](https://docs.cloud.google.com/bigquery/docs/entity-resolution-setup).
- Learn about [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions).
- Learn about [stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures).
- Learn about [data clean rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms).