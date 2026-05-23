# REST Resource: projects.locations.transferConfigs

- [Resource: TransferConfig](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.SCHEMA_REPRESENTATION)
  - [ScheduleOptions](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptions)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptions.SCHEMA_REPRESENTATION)
  - [ScheduleOptionsV2](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptionsV2)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptionsV2.SCHEMA_REPRESENTATION)
  - [TimeBasedSchedule](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.TimeBasedSchedule)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.TimeBasedSchedule.SCHEMA_REPRESENTATION)
  - [ManualSchedule](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ManualSchedule)
  - [EventDrivenSchedule](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EventDrivenSchedule)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EventDrivenSchedule.SCHEMA_REPRESENTATION)
  - [UserInfo](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.UserInfo)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.UserInfo.SCHEMA_REPRESENTATION)
  - [EncryptionConfiguration](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EncryptionConfiguration)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EncryptionConfiguration.SCHEMA_REPRESENTATION)
  - [ManagedTableType](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ManagedTableType)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#METHODS_SUMMARY)

## Resource: TransferConfig

Represents a data transfer configuration. A transfer configuration contains all metadata needed to perform a data transfer. For example, `destinationDatasetId` specifies where data should be stored. When a new transfer configuration is created, the specified `destinationDatasetId` is created when needed and shared with the appropriate data source service account.

| JSON representation |
|---|
| ``` { "name": string, "displayName": string, "dataSourceId": string, "params": { object }, "schedule": string, "scheduleOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptions`) }, "scheduleOptionsV2": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptionsV2`) }, "dataRefreshWindowDays": integer, "disabled": boolean, "updateTime": string, "nextRunTime": string, "state": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/TransferState`), "userId": string, "datasetRegion": string, "notificationPubsubTopic": string, "emailPreferences": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/EmailPreferences`) }, "encryptionConfiguration": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EncryptionConfiguration`) }, "error": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/Status`) }, "managedTableType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ManagedTableType`), // Union field `destination` can be only one of the following: "destinationDatasetId": string // End of list of possible types for union field `destination`. "ownerInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.UserInfo`) } } ``` |

| Fields ||
|---|---|
| `name` | `string` Identifier. The resource name of the transfer config. Transfer config names have the form either `projects/{projectId}/locations/{region}/transferConfigs/{configId}` or `projects/{projectId}/transferConfigs/{configId}`, where `configId` is usually a UUID, even though it is not guaranteed or required. The name is ignored when creating a transfer config. |
| `displayName` | `string` User specified display name for the data transfer. |
| `dataSourceId` | `string` Data source ID. This cannot be changed once data transfer is created. The full list of available data source IDs can be returned through an API call: <https://cloud.google.com/bigquery-transfer/docs/reference/datatransfer/rest/v1/projects.locations.dataSources/list> |
| `params` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` Parameters specific to each data source. For more information see the bq tab in the 'Setting up a data transfer' section for each data source. For example the parameters for Cloud Storage transfers are listed here: <https://cloud.google.com/bigquery-transfer/docs/cloud-storage-transfer#bq> |
| `schedule` | `string` Data transfer schedule. If the data source does not support a custom schedule, this should be empty. If it is empty, the default value for the data source will be used. The specified times are in UTC. Examples of valid format: `1st,3rd monday of month 15:30`, `every wed,fri of jan,jun 13:15`, and `first sunday of quarter 00:00`. See more explanation about the format here: <https://cloud.google.com/appengine/docs/flexible/python/scheduling-jobs-with-cron-yaml#the_schedule_format> NOTE: The minimum interval time between recurring transfers depends on the data source; refer to the documentation for your data source. |
| `scheduleOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptions`)`` Options customizing the data transfer schedule. |
| `scheduleOptionsV2` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ScheduleOptionsV2`)`` Options customizing different types of data transfer schedule. This field replaces "schedule" and "scheduleOptions" fields. ScheduleOptionsV2 cannot be used together with ScheduleOptions/Schedule. |
| `dataRefreshWindowDays` | `integer` The number of days to look back to automatically refresh the data. For example, if `dataRefreshWindowDays = 10`, then every day BigQuery reingests data for \[today-10, today-1\], rather than ingesting data for just \[today-1\]. Only valid if the data source supports the feature. Set the value to 0 to use the default value. |
| `disabled` | `boolean` Is this config disabled. When set to true, no runs will be scheduled for this transfer config. |
| `updateTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Data transfer modification time. Ignored by server on input. |
| `nextRunTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Next time when data transfer will run. |
| `state` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/TransferState`)`` Output only. State of the most recently updated transfer run. |
| `userId` | `string (https://developers.google.com/discovery/v1/type-format format)` Deprecated. Unique ID of the user on whose behalf transfer is done. |
| `datasetRegion` | `string` Output only. Region in which BigQuery dataset is located. |
| `notificationPubsubTopic` | `string` Pub/Sub topic where notifications will be sent after transfer runs associated with this transfer config finish. The format for specifying a pubsub topic is: `projects/{projectId}/topics/{topic_id}` |
| `emailPreferences` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/EmailPreferences`)`` Email notifications will be sent according to these preferences to the email address of the user who owns this transfer config. |
| `encryptionConfiguration` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EncryptionConfiguration`)`` The encryption configuration part. Currently, it is only used for the optional KMS key name. The BigQuery service account of your project must be granted permissions to use the key. Read methods will return the key name applied in effect. Write methods will apply the key if it is present, or otherwise try to apply project default keys if it is absent. |
| `error` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/Status`)`` Output only. Error code with detailed information about reason of the latest config failure. |
| `managedTableType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ManagedTableType`)`` The classification of the destination table. |
| Union field `destination`. The destination of the transfer config. `destination` can be only one of the following: ||
| `destinationDatasetId` | `string` The BigQuery target dataset id. |
| `ownerInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.UserInfo`)`` Output only. Information about the user whose credentials are used to transfer data. Populated only for `transferConfigs.get` requests. In case the user information is not available, this field will not be populated. |

### ScheduleOptions

Options customizing the data transfer schedule.

| JSON representation |
|---|
| ``` { "disableAutoScheduling": boolean, "startTime": string, "endTime": string } ``` |

| Fields ||
|---|---|
| `disableAutoScheduling` | `boolean` If true, automatic scheduling of data transfer runs for this configuration will be disabled. The runs can be started on ad-hoc basis using transferConfigs.startManualRuns API. When automatic scheduling is disabled, the TransferConfig.schedule field will be ignored. |
| `startTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Specifies time to start scheduling transfer runs. The first run will be scheduled at or after the start time according to a recurrence pattern defined in the schedule string. The start time can be changed at any moment. The time when a data transfer can be triggered manually is not limited by this option. |
| `endTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Defines time to stop scheduling transfer runs. A transfer run cannot be scheduled at or after the end time. The end time can be changed at any moment. The time when a data transfer can be triggered manually is not limited by this option. |

### ScheduleOptionsV2

V2 options customizing different types of data transfer schedule. This field supports existing time-based and manual transfer schedule. Also supports Event-Driven transfer schedule. ScheduleOptionsV2 cannot be used together with ScheduleOptions/Schedule.

| JSON representation |
|---|
| ``` { // Union field `schedule` can be only one of the following: "timeBasedSchedule": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.TimeBasedSchedule`) }, "manualSchedule": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ManualSchedule`) }, "eventDrivenSchedule": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EventDrivenSchedule`) } // End of list of possible types for union field `schedule`. } ``` |

| Fields ||
|---|---|
| Union field `schedule`. Data transfer schedules. `schedule` can be only one of the following: ||
| `timeBasedSchedule` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.TimeBasedSchedule`)`` Time based transfer schedule options. This is the default schedule option. |
| `manualSchedule` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.ManualSchedule`)`` Manual transfer schedule. If set, the transfer run will not be auto-scheduled by the system, unless the client invokes transferConfigs.startManualRuns. This is equivalent to disableAutoScheduling = true. |
| `eventDrivenSchedule` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig.EventDrivenSchedule`)`` Event driven transfer schedule options. If set, the transfer will be scheduled upon events arrial. |

### TimeBasedSchedule

Options customizing the time based transfer schedule. Options are migrated from the original ScheduleOptions message.

| JSON representation |
|---|
| ``` { "schedule": string, "startTime": string, "endTime": string } ``` |

| Fields ||
|---|---|
| `schedule` | `string` Data transfer schedule. If the data source does not support a custom schedule, this should be empty. If it is empty, the default value for the data source will be used. The specified times are in UTC. Examples of valid format: `1st,3rd monday of month 15:30`, `every wed,fri of jan,jun 13:15`, and `first sunday of quarter 00:00`. See more explanation about the format here: <https://cloud.google.com/appengine/docs/flexible/python/scheduling-jobs-with-cron-yaml#the_schedule_format> NOTE: The minimum interval time between recurring transfers depends on the data source; refer to the documentation for your data source. |
| `startTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Specifies time to start scheduling transfer runs. The first run will be scheduled at or after the start time according to a recurrence pattern defined in the schedule string. The start time can be changed at any moment. |
| `endTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Defines time to stop scheduling transfer runs. A transfer run cannot be scheduled at or after the end time. The end time can be changed at any moment. |

### ManualSchedule

This type has no fields.
Options customizing manual transfers schedule.

### EventDrivenSchedule

Options customizing EventDriven transfers schedule.

| JSON representation |
|---|
| ``` { // Union field `eventStream` can be only one of the following: "pubsubSubscription": string // End of list of possible types for union field `eventStream`. } ``` |

| Fields ||
|---|---|
| Union field `eventStream`. The event stream which specifies the Event-driven transfer options. Event-driven transfers listen to an event stream to transfer data. `eventStream` can be only one of the following: ||
| `pubsubSubscription` | `string` Pub/Sub subscription name used to receive events. Only Google Cloud Storage data source support this option. Format: projects/{project}/subscriptions/{subscription} |

### UserInfo

Information about a user.

| JSON representation |
|---|
| ``` { "email": string } ``` |

| Fields ||
|---|---|
| `email` | `string` E-mail address of the user. |

### EncryptionConfiguration

Represents the encryption configuration for a transfer.

| JSON representation |
|---|
| ``` { "kmsKeyName": string } ``` |

| Fields ||
|---|---|
| `kmsKeyName` | `string` The name of the KMS key used for encrypting BigQuery data. |

### ManagedTableType

The classifications of managed tables that can be created, native or BigLake.

| Enums ||
|---|---|
| `MANAGED_TABLE_TYPE_UNSPECIFIED` | Type unspecified. This defaults to `NATIVE` table. |
| `NATIVE` | The managed table is a native BigQuery table. This is the default value. |
| `BIGLAKE` | The managed table is a BigQuery table for Apache Iceberg (formerly BigLake managed tables), with a BigLake configuration. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create` | Creates a new data transfer configuration. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/delete` | Deletes a data transfer configuration, including any associated transfer runs and logs. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/get` | Returns information about a data transfer config. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/list` | Returns information about all transfer configs owned by a project in the specified location. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/patch` | Updates a data transfer configuration. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/scheduleRuns (deprecated)` | Creates transfer runs for a time range \[start_time, end_time\]. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/startManualRuns` | Manually initiates transfer runs. |