# BigQuery APIs and libraries overview

This page provides an overview of the various APIs associated with
BigQuery. While you can use the APIs directly
by making raw requests to the server, client libraries let you code in your
preferred language and provide simplifications
that significantly reduce the amount of code you need to write.
BigQuery supports client libraries in C#, Go, Java, Node.js, PHP,
Python, and Ruby.
For a more general overview of client libraries within Google Cloud, see
[Client libraries explained](https://docs.cloud.google.com/apis/docs/client-libraries-explained).

For examples of using the various BigQuery libraries and APIs,
see the [BigQuery Code Samples](https://docs.cloud.google.com/bigquery/docs/samples).

To use the APIs, you must authenticate to verify your client's identity. You can
do this by using
[Application Default Credentials](https://docs.cloud.google.com/bigquery/docs/authentication/getting-started),
a [service account key file](https://docs.cloud.google.com/bigquery/docs/authentication/service-account-file),
or [user credentials](https://docs.cloud.google.com/bigquery/docs/authentication/end-user-installed).
To learn more about authentication, see the
[Introduction to authentication](https://docs.cloud.google.com/bigquery/docs/authentication).

See [Pricing](https://cloud.google.com/bigquery/pricing) for more information about BigQuery
pricing, including [data ingestion](https://cloud.google.com/bigquery/pricing#data_ingestion_pricing)
and [data extraction](https://cloud.google.com/bigquery/pricing#data_extraction_pricing) pricing.

## BigQuery API

This is the main API that provides resources for creating, modifying, and
deleting core resources such as datasets, tables, jobs, and routines.

- For information about installation and usage, see [BigQuery API client libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries).
- For related quota information, see [BigQuery API quotas](https://docs.cloud.google.com/bigquery/quotas#api_request_quotas).

For links to the reference documentation and source code, select a language:

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.V2)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview)
- [Source Code](https://github.com/googleapis/java-bigquery)

### Node.js

- [API Reference Documentation](https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/nodejs-bigquery)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQuery)

### Python

- [API Reference Documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/python-bigquery)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery)

## BigQuery Data Policy API

This API helps users manage BigQuery data policies for
column-level security and data masking.

For information about this API and its usage, see
[BigQuery Data Policy API](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest).
For links to the reference documentation and source code, select a
language:

### C++

- [API Reference Documentation](https://docs.cloud.google.com/cpp/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery/datapolicies/v1)

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataPolicies.V1/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.DataPolicies.V1)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/datapolicies/apiv1)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery/datapolicies)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatapolicy/latest/overview)
- [Source Code](https://github.com/googleapis/java-bigquerydatapolicy)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery-datapolicies/latest)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQueryDataPolicies)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-data_policies/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery-data_policies)

## BigQuery Connection API

This API provides the control plane for establishing remote connections to allow
BigQuery to interact with remote data sources such as Cloud SQL. Some
federated query functionality is exposed within the BigQuery
API and libraries.

For more information about installation and usage, see
[BigQuery Connection client libraries](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection).
For links to the reference documentation and source code, select a
language:

### C++

- [API Reference Documentation](https://docs.cloud.google.com/cpp/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery/connection/v1)

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Connection.V1/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.Connection.V1)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/connection/apiv1)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery/connection)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/overview)
- [Source Code](https://github.com/googleapis/google-cloud-java/tree/main/java-bigqueryconnection)

### Node.js

- [API Reference Documentation](https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest)
- [Source Code](https://github.com/googleapis/google-cloud-node/tree/main/packages/google-cloud-bigquery-connection)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery-connection/latest)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQueryConnection)

### Python

- [API Reference Documentation](https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest)
- [Source Code](https://github.com/googleapis/python-bigquery-connection)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery-connection)

## BigQuery Migration API

This API supports mechanisms to help users migrate existing data warehouses to
BigQuery. It largely models work as a series of workflows and
tasks to be processed, such as translating SQL.

For more information about installation and usage, see
[BigQuery Migration client libraries](https://docs.cloud.google.com/bigquery/docs/reference/migration).
For links to the reference documentation and source code, select a
language:

### C++

- [API Reference Documentation](https://docs.cloud.google.com/cpp/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery/migration/v2)

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Migration.V2/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.Migration.V2)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/migration/apiv2)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery/migration)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerymigration/latest/overview)
- [Source Code](https://github.com/googleapis/google-cloud-java/tree/main/java-bigquerymigration)

### Node.js

- [API Reference Documentation](https://docs.cloud.google.com/nodejs/docs/reference/bigquery-migration/latest)
- [Source Code](https://github.com/googleapis/nodejs-bigquery-migration)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery-migration/latest)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQueryMigration)

### Python

- [API Reference Documentation](https://docs.cloud.google.com/python/docs/reference/bigquerymigration/latest)
- [Source Code](https://github.com/googleapis/python-bigquery-migration)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-migration/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery-migration)

## BigQuery Storage API

This API exposes high throughput data reading for consumers who need to scan
large volumes of managed data from their own applications and tools. The API
supports a parallel mechanism of scanning storage and exposes support for leveraging
features such as column projects and filtering.

For more information about installation and usage, see
[BigQuery Storage client libraries](https://docs.cloud.google.com/bigquery/docs/reference/storage/libraries).
For links to the reference documentation and source code, select a
language:

### C++

- [API Reference Documentation](https://docs.cloud.google.com/cpp/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery/storage/v1)

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.Storage.V1)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/apiv1)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery/storage)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/overview)
- [Source Code](https://github.com/googleapis/java-bigquerystorage)

### Node.js

- [API Reference Documentation](https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest)
- [Source Code](https://github.com/googleapis/nodejs-bigquery-storage)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery-storage/latest)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQueryStorage)

### Python

- [API Reference Documentation](https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest)
- [Source Code](https://github.com/googleapis/google-cloud-python)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-storage/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery-storage)

## BigQuery Reservation API

This API provides the mechanisms by which enterprise users can provision and manage
dedicated resources such as slots and BigQuery BI Engine memory allocations.

For more information about installation and usage, see
[BigQuery Reservation client libraries](https://docs.cloud.google.com/bigquery/docs/reference/reservations).
For links to the reference documentation and source code, select a
language:

### C++

- [API Reference Documentation](https://docs.cloud.google.com/cpp/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery/reservation/v1)

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Reservation.V1/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.Reservation.V1)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery/reservation/apiv1)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryreservation/latest/overview)
- [Source Code](https://github.com/googleapis/google-cloud-java/tree/main/java-bigqueryreservation)

### Node.js

- [API Reference Documentation](https://docs.cloud.google.com/nodejs/docs/reference/bigquery-reservation/latest)
- [Source Code](https://github.com/googleapis/google-cloud-node/tree/main/packages/google-cloud-bigquery-reservation)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery-reservation/latest)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQueryReservation)

### Python

- [API Reference Documentation](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest)
- [Source Code](https://github.com/googleapis/python-bigquery-reservation)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-reservation/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery-reservation)

## BigQuery sharing (formerly Analytics Hub)

This API facilitates data sharing within and across organizations. It allows
data providers to publish listings that reference shared
resources, including BigQuery datasets and Pub/Sub
topics. With BigQuery sharing, users can discover and search for listings
that they have access to. Subscribers can view and subscribe to listings. When
you subscribe to a listing, sharing creates a linked
dataset in your project.

For more information about this API and its usage, see
[Analytics Hub API](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest).
For links to the reference documentation and source code, select a
language:

### C++

- [API Reference Documentation](https://docs.cloud.google.com/cpp/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery/analyticshub/v1)

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.AnalyticsHub.V1/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.AnalyticsHub.V1)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/analyticshub/apiv1)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery/analyticshub/apiv1)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery-data-exchange/latest/overview)
- [Source Code](https://github.com/googleapis/google-cloud-java/tree/main/java-bigquery-data-exchange)

### Node.js

- [API Reference Documentation](https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-exchange/latest)
- [Source Code](https://github.com/googleapis/google-cloud-node/tree/main/packages/google-cloud-bigquery-dataexchange)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery-analyticshub/latest)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQueryAnalyticsHub)

### Python

- [API Reference Documentation](https://docs.cloud.google.com/python/docs/reference/analyticshub/latest)
- [Source Code](https://github.com/googleapis/python-bigquery-analyticshub)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-data_exchange/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery-data_exchange)

## BigQuery Data Transfer Service API

This API is used for managed ingestion pipelines. Examples of pipelines include
scheduling periodic ingestions from Cloud Storage, automated ingestion
of analytics data from other Google properties such as YouTube, or data transfers
from third-party partners who integrate with the service.

This API is also where scheduled queries are defined and managed within BigQuery.

For more information about installation and usage, see
[BigQuery Data Transfer Service client libraries](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/libraries). For links to the reference documentation and source code, select a
language:

### C++

- [API Reference Documentation](https://docs.cloud.google.com/cpp/docs/reference/bigquery/latest)
- [Source Code](https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery/datatransfer/v1)

### C#

- [API Reference Documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest)
- [Source Code](https://github.com/googleapis/google-cloud-dotnet/tree/main/apis/Google.Cloud.BigQuery.DataTransfer.V1)

### Go

- [API Reference Documentation](https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/datatransfer/apiv1)
- [Source Code](https://github.com/googleapis/google-cloud-go/tree/main/bigquery/datatransfer/apiv1)

### Java

- [API Reference Documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/overview)
- [Source Code](https://github.com/googleapis/google-cloud-java/tree/main/java-bigquerydatatransfer)

### Node.js

- [API Reference Documentation](https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest)
- [Source Code](https://github.com/googleapis/google-cloud-node/tree/main/packages/google-cloud-bigquery-datatransfer)

### PHP

- [API Reference Documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquerydatatransfer/latest)
- [Source Code](https://github.com/googleapis/google-cloud-php/tree/main/BigQueryDataTransfer)

### Python

- [API Reference Documentation](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest)
- [Source Code](https://github.com/googleapis/python-bigquery-datatransfer)

### Ruby

- [API Reference Documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-data_transfer/latest)
- [Source Code](https://github.com/googleapis/google-cloud-ruby/tree/main/google-cloud-bigquery-data_transfer)