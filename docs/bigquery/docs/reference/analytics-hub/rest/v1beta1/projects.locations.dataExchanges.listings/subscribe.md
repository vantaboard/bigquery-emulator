# Method: projects.locations.dataExchanges.listings.subscribe

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#body.aspect_1)
- [DestinationDataset](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDataset)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDataset.SCHEMA_REPRESENTATION)
- [DestinationDatasetReference](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDatasetReference)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDatasetReference.SCHEMA_REPRESENTATION)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#try-it)

Subscribes to a listing.

Currently, with Analytics Hub, you can create listings that reference only BigQuery datasets. Upon subscription to a listing for a BigQuery dataset, Analytics Hub creates a linked dataset in the subscriber's project.

### HTTP request

`POST https://analyticshub.googleapis.com/v1beta1/{name=projects/*/locations/*/dataExchanges/*/listings/*}:subscribe`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. Resource name of the listing that you want to subscribe to. e.g. `projects/myproject/locations/us/dataExchanges/123/listings/456`. |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { // Union field `destination` can be only one of the following: "destinationDataset": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDataset`) } // End of list of possible types for union field `destination`. } ``` |

| Fields ||
|---|---|
| Union field `destination`. Resulting destination of the listing that you subscribed to. `destination` can be only one of the following: ||
| `destinationDataset` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDataset`)`` BigQuery destination dataset to create for the subscriber. |

### Response body

If successful, the response body is empty.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `name` resource:

- `analyticshub.listings.subscribe`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).

## DestinationDataset

Defines the destination bigquery dataset.

| JSON representation |
|---|
| ``` { "datasetReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDatasetReference`) }, "friendlyName": string, "description": string, "labels": { string: string, ... }, "location": string } ``` |

| Fields ||
|---|---|
| `datasetReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe#DestinationDatasetReference`)`` Required. A reference that identifies the destination dataset. |
| `friendlyName` | `string` Optional. A descriptive name for the dataset. |
| `description` | `string` Optional. A user-friendly description of the dataset. |
| `labels` | `map (key: string, value: string)` Optional. The labels associated with this dataset. You can use these to organize and group your datasets. You can set this property when inserting or updating a dataset. See <https://cloud.google.com/resource-manager/docs/creating-managing-labels> for more information. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| `location` | `string` Required. The geographic location where the dataset should reside. See <https://cloud.google.com/bigquery/docs/locations> for supported locations. |

## DestinationDatasetReference

| JSON representation |
|---|
| ``` { "datasetId": string, "projectId": string } ``` |

| Fields ||
|---|---|
| `datasetId` | `string` Required. A unique ID for this dataset, without the project name. The ID must contain only letters (a-z, A-Z), numbers (0-9), or underscores (_). The maximum length is 1,024 characters. |
| `projectId` | `string` Required. The ID of the project containing this dataset. |