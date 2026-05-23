# BigQuery regional endpoints

This page describes how you can use [Private Service Connect regional
endpoints](https://docs.cloud.google.com/vpc/docs/about-accessing-regional-google-apis-endpoints)
to access resources in BigQuery. Regional endpoints let
you run your workloads in a manner that complies with [data
residency](https://docs.cloud.google.com/assured-workloads/docs/data-residency) and data sovereignty
requirements, where your request traffic is routed directly to the region
specified in the endpoint.

## Overview

Regional endpoints are request endpoints that help restrict requests to proceed
only if the affected resource exists in the location specified by the endpoint.
For example, if you use the endpoint
`https://bigquery.us-central1.rep.googleapis.com` in a delete dataset
request, then the request only proceeds if the dataset is located in
`US-CENTRAL1`.

Unlike global endpoints, where requests can be routed through a different
location from where the resource resides, regional endpoints can help to
restrict your requests to the location specified by the endpoint where the
resource resides. Regional endpoints terminate TLS sessions in the location
specified by the endpoint for requests received from the Internet, other
Google Cloud resources (such as Compute Engine virtual machines), on-premise
services using VPN or Interconnect, and Virtual Private Clouds (VPCs).

Regional endpoints help to ensure data residency by keeping your at-rest and
in-transit table data within the location specified by the endpoint. This
excludes resource metadata, such as dataset names and
IAM policies. For more information, see [Note on service
data](https://docs.cloud.google.com/assured-workloads/docs/data-residency#service-data).

BigQuery includes multiple APIs. The following APIs are available
for use with regional endpoint:

| API | URL | Reference |
|---|---|---|
| BigQuery API | `bigquery.LOCATION.rep.googleapis.com` | [REST](https://docs.cloud.google.com/bigquery/docs/reference/rest) |
| BigQuery Storage API | `bigquerystorage.LOCATION.rep.googleapis.com` | [RPC](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc) |
| BigQuery Reservations API | `bigqueryreservation.LOCATION.rep.googleapis.com` | [RPC](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc) and [REST](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest) |
| BigQuery Migration API | `bigquerymigration.LOCATION.rep.googleapis.com` | [REST](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest) |
| BigQuery Data Transfer Service API | `bigquerydatatransfer.LOCATION.rep.googleapis.com` | [RPC](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc) and [REST](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest) |

## Supported locations

You can use regional endpoints to keep your data within the following locations:

- Asia-Pacific

  - Delhi `asia-south2`
  - Mumbai `asia-south1`
- Europe

  - Belgium `europe-west1`
  - Frankfurt `europe-west3`
  - London `europe-west2`
  - Milan `europe-west8`
  - Netherlands `europe-west4`
  - Paris `europe-west9`
  - Zürich `europe-west6`
- Middle East

  - Dammam `me-central2`
- Americas

  - Columbus, Ohio `us-east5`
  - Dallas `us-south1`
  - Iowa `us-central1`
  - Las Vegas `us-west4`
  - Los Angeles `us-west2`
  - Montréal `northamerica-northeast1`
  - Northern Virginia `us-east4`
  - Oregon `us-west1`
  - Salt Lake City `us-west3`
  - South Carolina `us-east1`
  - Toronto `northamerica-northeast2`

## Supported operations

Regional endpoints can only be used to perform operations that access or mutate
resources stored in the location specified by the endpoint.
Regional endpoints cannot be used to perform operations that access or mutate
resources outside of the location specified by the endpoint.

For example, when you use the regional endpoint
`https://bigquery.us-central1.rep.googleapis.com`, you can read tables in
datasets located in `US-CENTRAL1`, and copy a table from a source dataset to a
destination dataset only when both datasets are located in `US-CENTRAL1`. If you
attempt to read or copy a table from outside `US-CENTRAL1`, you get an
error.

## Limitations and restrictions

Regional endpoints cannot be used to perform the following operations:

- Operations that access or mutate resources outside of the location specified by the endpoint
- Copying, replicating, or rewriting resources from one location to another.

Keep in mind the following restrictions when using regional endpoints:

- Regional endpoints don't support [mutual Transport Layer Security (mTLS)](https://docs.cloud.google.com/chrome-enterprise-premium/docs/understand-mtls).
- Using a regional endpoint won't restrict the creation of resources outside of the endpoint region. To restrict resource creation, use [Organization Policy Service
  resource locations constraint](https://docs.cloud.google.com/resource-manager/docs/organization-policy/defining-locations).
- [Cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication) and [cross-region table copying](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy_tables_across_regions) aren't restricted by endpoint protection.
- Running [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries)

## Tools for using regional endpoints

### Console

To access BigQuery resources in a manner that's compliant with
data residency or sovereignty requirements, use the jurisdictional
Google Cloud console URLs:

| Resource | URL |
|---|---|
| Dataset list for a project | `https://console.JURISDICTION.cloud.google.com/bigquery?project=PROJECT_ID` |
| Table list for a dataset | `https://console.JURISDICTION.cloud.google.com/bigquery/projects/PROJECT_ID/datasets/DATASET_NAME/tables` |
| Details for a table | `https://console.JURISDICTION.cloud.google.com/bigquery/projects/PROJECT_ID/datasets/DATASET_NAME/tables/TABLE_NAME` |

Replace `JURISDICTION` with one of the following
values:

- `eu` if the resource is located in the European Union
- `sa` if the resource is located in the Kingdom of Saudi Arabia
- `us` if the resource is located in the United States

> [!NOTE]
> **Note:** You cannot use the jurisdictional Google Cloud console to upload files in `eu`, `sa`, or `us`.

### Command line

To configure the Google Cloud CLI for use with regional endpoints,
complete the following steps:

1. Make sure you're using the Google Cloud CLI 402.0.0 or newer.

2. Set the `api_endpoint_overrides/bigquery` property to the regional
   endpoint you want to use:

   ```
   gcloud config set api_endpoint_overrides/bigquery https://bigquery.LOCATION.rep.googleapis.com/bigquery/v2/
   ```

   Alternatively, you can set the
   `CLOUDSDK_API_ENDPOINT_OVERRIDES_BIGQUERY` environment variable to the
   endpoint:

   ```
   CLOUDSDK_API_ENDPOINT_OVERRIDES_BIGQUERY=https://bigquery.LOCATION.rep.googleapis.com/bigquery/v2/ gcloud  alpha bq  datasets list
   ```

### REST APIs

For REST API, instead of sending a REST request to a
[service endpoint](https://docs.cloud.google.com/bigquery/docs/reference/rest#service-endpoint), send the
request to the regional endpoint in the following format:
`https://bigquery.LOCATION.rep.googleapis.com`.

## Restrict global API endpoint usage

To help enforce the use of regional endpoints, use the
`constraints/gcp.restrictEndpointUsage` organization policy constraint to block
requests to the global API endpoint. For more information, see
[Restrict endpoint usage](https://docs.cloud.google.com/docs/security/compliance/restrict-endpoint-usage).