# BigQuery Data Transfer API

Schedule queries or transfer external data from SaaS applications to Google BigQuery on a regular basis.

## Service: bigquerydatatransfer.googleapis.com

The Service name `bigquerydatatransfer.googleapis.com` is needed to create RPC client stubs.

## `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService`

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.CheckValidCreds` `` | Returns true if valid credentials exist for the given data source and requesting user. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.CreateTransferConfig` `` | Creates a new data transfer configuration. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferConfig` `` | Deletes a data transfer configuration, including any associated transfer runs and logs. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferRun` `` | Deletes the specified transfer run. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.EnrollDataSources` `` | Enroll data sources in a user project. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.GetDataSource` `` | Retrieves a supported data source and returns its settings. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferConfig` `` | Returns information about a data transfer config. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferResource` `` | Returns a transfer resource. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferRun` `` | Returns information about the particular transfer run. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.ListDataSources` `` | Lists supported data sources and returns their settings. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferConfigs` `` | Returns information about all transfer configs owned by a project in the specified location. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferLogs` `` | Returns log messages for the transfer run. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferResources` `` | Returns information about transfer resources. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferRuns` `` | Returns information about running and completed transfer runs. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.ScheduleTransferRuns` (deprecated) `` | Creates transfer runs for a time range \[start_time, end_time\]. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.StartManualTransferRuns` `` | Manually initiates transfer runs. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.UnenrollDataSources` `` | Unenroll data sources in a user project. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.bigquery.datatransfer.v1#google.cloud.bigquery.datatransfer.v1.DataTransferService.UpdateTransferConfig` `` | Updates a data transfer configuration. |

## `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Locations`

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Locations.GetLocation` `` | Gets information about a location. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Locations.ListLocations` `` | Lists information about the supported locations for this service. |