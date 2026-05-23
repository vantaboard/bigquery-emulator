# REST Resource: projects.locations.transferConfigs.transferResources

- [Resource: TransferResource](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.SCHEMA_REPRESENTATION)
  - [ResourceType](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceType)
  - [ResourceDestination](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceDestination)
  - [TransferRunBrief](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferRunBrief)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferRunBrief.SCHEMA_REPRESENTATION)
  - [TransferResourceStatusDetail](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferResourceStatusDetail)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferResourceStatusDetail.SCHEMA_REPRESENTATION)
  - [ResourceTransferState](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceTransferState)
  - [TransferStatusSummary](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusSummary)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusSummary.SCHEMA_REPRESENTATION)
  - [TransferStatusMetric](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusMetric)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusMetric.SCHEMA_REPRESENTATION)
  - [TransferStatusUnit](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusUnit)
  - [HierarchyDetail](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.HierarchyDetail)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.HierarchyDetail.SCHEMA_REPRESENTATION)
  - [TableDetail](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TableDetail)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TableDetail.SCHEMA_REPRESENTATION)
  - [PartitionDetail](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.PartitionDetail)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.PartitionDetail.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#METHODS_SUMMARY)

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

### ResourceType

Type of resource being transferred.

| Enums ||
|---|---|
| `RESOURCE_TYPE_UNSPECIFIED` | Default value. |
| `RESOURCE_TYPE_TABLE` | Table resource type. |
| `RESOURCE_TYPE_PARTITION` | Partition resource type. |

### ResourceDestination

The destination for a transferred resource.

| Enums ||
|---|---|
| `RESOURCE_DESTINATION_UNSPECIFIED` | Default value. |
| `RESOURCE_DESTINATION_BIGQUERY` | BigQuery. |
| `RESOURCE_DESTINATION_DATAPROC_METASTORE` | Dataproc Metastore. |
| `RESOURCE_DESTINATION_BIGLAKE_METASTORE` | BigLake Metastore. |
| `RESOURCE_DESTINATION_BIGLAKE_REST_CATALOG` | BigLake REST Catalog. |
| `RESOURCE_DESTINATION_BIGLAKE_HIVE_CATALOG` | BigLake Hive Catalog. |

### TransferRunBrief

Basic information about a transfer run.

| JSON representation |
|---|
| ``` { "run": string, "startTime": string } ``` |

| Fields ||
|---|---|
| `run` | `string` Optional. Run URI. The format must be: `projects/{project}/locations/{location}/transferConfigs/{transferConfig}/run/{run}` |
| `startTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Optional. Start time of the transfer run. |

### TransferResourceStatusDetail

Status details of the resource being transferred.

| JSON representation |
|---|
| ``` { "state": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceTransferState`), "summary": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusSummary`) }, "error": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/Status`) }, "completedPercentage": number } ``` |

| Fields ||
|---|---|
| `state` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.ResourceTransferState`)`` Optional. Transfer state of the resource. |
| `summary` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusSummary`)`` Optional. Transfer status summary of the resource. |
| `error` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/Status`)`` Optional. Transfer error details for the resource. |
| `completedPercentage` | `number` Output only. Percentage of the transfer completed. Valid values: 0-100. |

### ResourceTransferState

The transfer state of an individual resource (e.g., a table or partition). This may differ from the overall transfer run's state. For instance, a resource can be transferred successfully even if the run as a whole fails.

| Enums ||
|---|---|
| `RESOURCE_TRANSFER_STATE_UNSPECIFIED` | Default value. |
| `RESOURCE_TRANSFER_PENDING` | Resource is waiting to be transferred. |
| `RESOURCE_TRANSFER_RUNNING` | Resource transfer is running. |
| `RESOURCE_TRANSFER_SUCCEEDED` | Resource transfer is a success. |
| `RESOURCE_TRANSFER_FAILED` | Resource transfer failed. |
| `RESOURCE_TRANSFER_CANCELLED` | Resource transfer was cancelled. |

### TransferStatusSummary

Status summary of the resource being transferred.

| JSON representation |
|---|
| ``` { "metrics": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusMetric`) } ], "progressUnit": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusUnit`) } ``` |

| Fields ||
|---|---|
| `metrics[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusMetric`)`` Optional. List of transfer status metrics. |
| `progressUnit` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusUnit`)`` Input only. Unit based on which transfer status progress should be calculated. |

### TransferStatusMetric

Metrics for tracking the transfer status.

| JSON representation |
|---|
| ``` { "completed": string, "pending": string, "failed": string, "total": string, "unit": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusUnit`) } ``` |

| Fields ||
|---|---|
| `completed` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Number of units transferred successfully. |
| `pending` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Number of units pending transfer. |
| `failed` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Number of units that failed to transfer. |
| `total` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Total number of units for the transfer. |
| `unit` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TransferStatusUnit`)`` Optional. Unit for measuring progress (e.g., BYTES). |

### TransferStatusUnit

Unit of the transfer status.

| Enums ||
|---|---|
| `TRANSFER_STATUS_UNIT_UNSPECIFIED` | Default value. |
| `TRANSFER_STATUS_UNIT_BYTES` | Bytes. |
| `TRANSFER_STATUS_UNIT_OBJECTS` | Objects. |

### HierarchyDetail

Details about the hierarchy.

| JSON representation |
|---|
| ``` { // Union field `detail` can be only one of the following: "tableDetail": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TableDetail`) }, "partitionDetail": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.PartitionDetail`) } // End of list of possible types for union field `detail`. } ``` |

| Fields ||
|---|---|
| Union field `detail`. Details about the hierarchy can be one of table/partition. `detail` can be only one of the following: ||
| `tableDetail` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.TableDetail`)`` Optional. Table details related to hierarchy. |
| `partitionDetail` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource.PartitionDetail`)`` Optional. Partition details related to hierarchy. |

### TableDetail

Table details related to hierarchy.

| JSON representation |
|---|
| ``` { "partitionCount": string } ``` |

| Fields ||
|---|---|
| `partitionCount` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Total number of partitions being tracked within the table. |

### PartitionDetail

Partition details related to hierarchy.

| JSON representation |
|---|
| ``` { "table": string } ``` |

| Fields ||
|---|---|
| `table` | `string` Optional. Name of the table which has the partitions. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/get` | Returns a transfer resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list` | Returns information about transfer resources. |