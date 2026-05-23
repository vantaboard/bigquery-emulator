# REST Resource: projects.locations.subscriptions

- [Resource: Subscription](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions#Subscription)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions#Subscription.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions#METHODS_SUMMARY)

## Resource: Subscription

A subscription represents a subscribers' access to a particular set of published data. It contains references to associated listings, data exchanges, and linked datasets.

| JSON representation |
|---|
| ``` { "name": string, "creationTime": string, "lastModifyTime": string, "organizationId": string, "organizationDisplayName": string, "state": enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/State`), "linkedDatasetMap": { string: { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription#LinkedResource`) }, ... }, "subscriberContact": string, "linkedResources": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription#LinkedResource`) } ], "resourceType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/SharedResourceType`), "commercialInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription#CommercialInfo`) }, "destinationDataset": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset`) }, // Union field `resource_name` can be only one of the following: "listing": string, "dataExchange": string // End of list of possible types for union field `resource_name`. "logLinkedDatasetQueryUserEmail": boolean } ``` |

| Fields ||
|---|---|
| `name` | `string` Output only. The resource name of the subscription. e.g. `projects/myproject/locations/us/subscriptions/123`. |
| `creationTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Timestamp when the subscription was created. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `lastModifyTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Timestamp when the subscription was last modified. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `organizationId` | `string` Output only. Organization of the project this subscription belongs to. |
| `organizationDisplayName` | `string` Output only. Display name of the project of this subscription. |
| `state` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/State`)`` Output only. Current state of the subscription. |
| `linkedDatasetMap` | ``map (key: string, value: object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription#LinkedResource`))`` Output only. Map of listing resource names to associated linked resource, e.g. projects/123/locations/us/dataExchanges/456/listings/789 -\> projects/123/datasets/my_dataset For listing-level subscriptions, this is a map of size 1. Only contains values if state == STATE_ACTIVE. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| `subscriberContact` | `string` Output only. Email of the subscriber. |
| `linkedResources[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription#LinkedResource`)`` Output only. Linked resources created in the subscription. Only contains values if state = STATE_ACTIVE. |
| `resourceType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/SharedResourceType`)`` Output only. Listing shared asset type. |
| `commercialInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription#CommercialInfo`)`` Output only. This is set if this is a commercial subscription i.e. if this subscription was created from subscribing to a commercial listing. |
| `destinationDataset` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset`)`` Optional. BigQuery destination dataset to create for the subscriber. |
| Union field `resource_name`. `resource_name` can be only one of the following: ||
| `listing` | `string` Output only. Resource name of the source Listing. e.g. projects/123/locations/us/dataExchanges/456/listings/789 |
| `dataExchange` | `string` Output only. Resource name of the source Data Exchange. e.g. projects/123/locations/us/dataExchanges/456 |
| `logLinkedDatasetQueryUserEmail` | `boolean` Output only. By default, false. If true, the Subscriber agreed to the email sharing mandate that is enabled for DataExchange/Listing. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/delete` | Deletes a subscription. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/get` | Gets the details of a Subscription. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/getIamPolicy` | Gets the IAM policy. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/list` | Lists all subscriptions in a given project and location. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/refresh` | Refreshes a Subscription to a Data Exchange. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/revoke` | Revokes a given subscription. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/setIamPolicy` | Sets the IAM policy. |