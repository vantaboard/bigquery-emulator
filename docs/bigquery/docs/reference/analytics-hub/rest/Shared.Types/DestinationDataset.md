# DestinationDataset

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset#SCHEMA_REPRESENTATION)
- [DestinationDatasetReference](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset#DestinationDatasetReference)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset#DestinationDatasetReference.SCHEMA_REPRESENTATION)

Defines the destination bigquery dataset.

| JSON representation |
|---|
| ``` { "datasetReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset#DestinationDatasetReference`) }, "friendlyName": string, "description": string, "labels": { string: string, ... }, "location": string, "replicaLocations": [ string ] } ``` |

| Fields ||
|---|---|
| `datasetReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset#DestinationDatasetReference`)`` Required. A reference that identifies the destination dataset. |
| `friendlyName` | `string` Optional. A descriptive name for the dataset. |
| `description` | `string` Optional. A user-friendly description of the dataset. |
| `labels` | `map (key: string, value: string)` Optional. The labels associated with this dataset. You can use these to organize and group your datasets. You can set this property when inserting or updating a dataset. See <https://cloud.google.com/resource-manager/docs/creating-managing-labels> for more information. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| `location` | `string` Required. The geographic location where the dataset should reside. See <https://cloud.google.com/bigquery/docs/locations> for supported locations. |
| `replicaLocations[]` | `string` Optional. The geographic locations where the dataset should be replicated. See [BigQuery locations](https://cloud.google.com/bigquery/docs/locations) for supported locations. |

## DestinationDatasetReference

| JSON representation |
|---|
| ``` { "datasetId": string, "projectId": string } ``` |

| Fields ||
|---|---|
| `datasetId` | `string` Required. A unique ID for this dataset, without the project name. The ID must contain only letters (a-z, A-Z), numbers (0-9), or underscores (_). The maximum length is 1,024 characters. |
| `projectId` | `string` Required. The ID of the project containing this dataset. |