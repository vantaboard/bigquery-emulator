# Method: datasets.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#body.DatasetList.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list#try-it)

Lists all datasets in the specified project to which the user has been granted the READER dataset role.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the datasets to be listed |

### Query parameters

| Parameters ||
|---|---|
| `maxResults` | `integer` The maximum number of results to return in a single response page. Leverage the page tokens to iterate through the entire collection. |
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results |
| `all` | `boolean` Whether to list all datasets, including hidden ones |
| `filter` | `string` An expression for filtering the results of the request by label. The syntax is `labels.<name>[:<value>]`. Multiple filters can be AND-ed together by connecting with a space. Example: `labels.department:receiving labels.active`. See [Filtering datasets using labels](https://cloud.google.com/bigquery/docs/filtering-labels#filtering_datasets_using_labels) for details. |

### Request body

The request body must be empty.

### Response body

Response format for a page of results when listing datasets.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "etag": string, "nextPageToken": string, "datasets": [ { "kind": string, "id": string, "datasetReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`) }, "labels": { string: string, ... }, "friendlyName": string, "location": string, "type": string, "catalogSource": string, "externalDatasetReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalDatasetReference`) } } ], "unreachable": [ string ] } ``` |

| Fields ||
|---|---|
| `kind` | `string` Output only. The resource type. This property always returns the value "bigquery#datasetList" |
| `etag` | `string` Output only. A hash value of the results page. You can use this property to determine if the page has changed since the last request. |
| `nextPageToken` | `string` A token that can be used to request the next results page. This property is omitted on the final results page. |
| `datasets[]` | `object` An array of the dataset resources in the project. Each resource contains basic information. For full information about a particular dataset resource, use the Datasets: get method. This property is omitted when there are no datasets in the project. |
| `datasets[].kind` | `string` The resource type. This property always returns the value "bigquery#dataset" |
| `datasets[].id` | `string` The fully-qualified, unique, opaque ID of the dataset. |
| `datasets[].datasetReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`)`` The dataset reference. Use this property to access specific parts of the dataset's ID, such as project ID or dataset ID. |
| `datasets[].labels` | `map (key: string, value: string)` The labels associated with this dataset. You can use these to organize and group your datasets. |
| `datasets[].friendlyName` | `string` An alternate name for the dataset. The friendly name is purely decorative in nature. |
| `datasets[].location` | `string` The geographic location where the dataset resides. |
| `datasets[].type` | `string` Output only. Same as `type` in `Dataset`. The type of the dataset, one of: - DEFAULT - only accessible by owner and authorized accounts, - PUBLIC - accessible by everyone, - LINKED - linked dataset, - EXTERNAL - dataset with definition in external metadata catalog, - BIGLAKE_ICEBERG - a Biglake dataset accessible through the Iceberg API, - BIGLAKE_HIVE - a Biglake dataset accessible through the Hive API. |
| `datasets[].catalogSource` | `string` Output only. The origin of the dataset, one of: - (Unset) - Native BigQuery Dataset. - BIGLAKE - Dataset is backed by a namespace stored natively in Biglake. |
| `datasets[].externalDatasetReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalDatasetReference`)`` Output only. Reference to a read-only external dataset defined in data catalogs outside of BigQuery. Filled out when the dataset type is EXTERNAL. |
| `unreachable[]` | `string` A list of skipped locations that were unreachable. For more information about BigQuery locations, see: <https://cloud.google.com/bigquery/docs/locations>. Example: "europe-west5" |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).