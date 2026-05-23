# REST Resource: projects.transferConfigs.transferResources

- [Resource: TransferResource](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources#TransferResource)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources#TransferResource.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources#METHODS_SUMMARY)

## Resource: TransferResource

Resource (table/partition) that is being transferred.

| JSON representation |
|---|
| ``` { "name": string, "type": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceType`), "destination": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceDestination`), "latestRun": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferRunBrief`) }, "latestStatusDetail": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferResourceStatusDetail`) }, "lastSuccessfulRun": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferRunBrief`) }, "hierarchyDetail": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.HierarchyDetail`) }, "updateTime": string } ``` |

| Fields ||
|---|---|
| `name` | `string` Identifier. Resource name. |
| `type` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceType`)`` Optional. Resource type. |
| `destination` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceDestination`)`` Optional. Resource destination. |
| `latestRun` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferRunBrief`)`` Optional. Run details for the latest run. |
| `latestStatusDetail` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferResourceStatusDetail`)`` Optional. Status details for the latest run. |
| `lastSuccessfulRun` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferRunBrief`)`` Output only. Run details for the last successful run. |
| `hierarchyDetail` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.HierarchyDetail`)`` Optional. Details about the hierarchy. |
| `updateTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Time when the resource was last updated. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources/get` | Returns a transfer resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources/list` | Returns information about transfer resources. |