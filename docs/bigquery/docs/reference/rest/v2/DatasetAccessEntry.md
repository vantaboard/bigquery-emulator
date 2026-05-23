# DatasetAccessEntry

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DatasetAccessEntry#SCHEMA_REPRESENTATION)

Grants all resources of particular types in a particular dataset read access to the current dataset.

Similar to how individually authorized views work, updates to any resource granted through its dataset (including creation of new resources) requires read permission to referenced resources, plus write permission to the authorizing dataset.

| JSON representation |
|---|
| ``` { "dataset": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`) }, "targetTypes": [ enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/TargetType`) ] } ``` |

| Fields ||
|---|---|
| `dataset` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`)`` The dataset this entry applies to |
| `targetTypes[]` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/TargetType`)`` Which resources in the dataset this entry applies to. Currently, only views are supported, but additional target types may be added in the future. |