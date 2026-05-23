# Method: projects.locations.dataExchanges.listings.subscribe

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#body.SubscribeListingResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#body.aspect)
- [DestinationPubSubSubscription](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DestinationPubSubSubscription)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DestinationPubSubSubscription.SCHEMA_REPRESENTATION)
- [PubSubSubscription](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PubSubSubscription)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PubSubSubscription.SCHEMA_REPRESENTATION)
- [PushConfig](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PushConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PushConfig.SCHEMA_REPRESENTATION)
- [OidcToken](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#OidcToken)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#OidcToken.SCHEMA_REPRESENTATION)
- [PubsubWrapper](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PubsubWrapper)
- [NoWrapper](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#NoWrapper)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#NoWrapper.SCHEMA_REPRESENTATION)
- [BigQueryConfig](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#BigQueryConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#BigQueryConfig.SCHEMA_REPRESENTATION)
- [CloudStorageConfig](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#CloudStorageConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#CloudStorageConfig.SCHEMA_REPRESENTATION)
- [TextConfig](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#TextConfig)
- [AvroConfig](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#AvroConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#AvroConfig.SCHEMA_REPRESENTATION)
- [ExpirationPolicy](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#ExpirationPolicy)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#ExpirationPolicy.SCHEMA_REPRESENTATION)
- [DeadLetterPolicy](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DeadLetterPolicy)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DeadLetterPolicy.SCHEMA_REPRESENTATION)
- [RetryPolicy](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#RetryPolicy)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#RetryPolicy.SCHEMA_REPRESENTATION)
- [MessageTransform](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#MessageTransform)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#MessageTransform.SCHEMA_REPRESENTATION)
- [JavaScriptUDF](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#JavaScriptUDF)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#JavaScriptUDF.SCHEMA_REPRESENTATION)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#try-it)

Subscribes to a listing.

Currently, with Analytics Hub, you can create listings that reference only BigQuery datasets. Upon subscription to a listing for a BigQuery dataset, Analytics Hub creates a linked dataset in the subscriber's project.

### HTTP request

`POST https://analyticshub.googleapis.com/v1/{name=projects/*/locations/*/dataExchanges/*/listings/*}:subscribe`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. Resource name of the listing that you want to subscribe to. e.g. `projects/myproject/locations/us/dataExchanges/123/listings/456`. |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { // Union field `destination` can be only one of the following: "destinationDataset": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset`) }, "destinationPubsubSubscription": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DestinationPubSubSubscription`) } // End of list of possible types for union field `destination`. } ``` |

| Fields ||
|---|---|
| Union field `destination`. Resulting destination of the listing that you subscribed to. `destination` can be only one of the following: ||
| `destinationDataset` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/DestinationDataset`)`` Input only. BigQuery destination dataset to create for the subscriber. |
| `destinationPubsubSubscription` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DestinationPubSubSubscription`)`` Input only. Destination Pub/Sub subscription to create for the subscriber. |

### Response body

Message for response when you subscribe to a listing.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "subscription": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription`) } } ``` |

| Fields ||
|---|---|
| `subscription` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription`)`` Subscription object created from this subscribe action. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

## DestinationPubSubSubscription

Defines the destination Pub/Sub subscription.

| JSON representation |
|---|
| ``` { "pubsubSubscription": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PubSubSubscription`) } } ``` |

| Fields ||
|---|---|
| `pubsubSubscription` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PubSubSubscription`)`` Required. Destination Pub/Sub subscription resource. |

## PubSubSubscription

Defines the destination Pub/Sub subscription. If none of `pushConfig`, `bigqueryConfig`, `cloudStorageConfig`, `pubsubExportConfig`, or `pubsubliteExportConfig` is set, then the subscriber will pull and ack messages using API methods. At most one of these fields may be set.

| JSON representation |
|---|
| ``` { "name": string, "pushConfig": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PushConfig`) }, "bigqueryConfig": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#BigQueryConfig`) }, "cloudStorageConfig": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#CloudStorageConfig`) }, "ackDeadlineSeconds": integer, "retainAckedMessages": boolean, "messageRetentionDuration": string, "labels": { string: string, ... }, "enableMessageOrdering": boolean, "expirationPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#ExpirationPolicy`) }, "filter": string, "deadLetterPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DeadLetterPolicy`) }, "retryPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#RetryPolicy`) }, "detached": boolean, "enableExactlyOnceDelivery": boolean, "messageTransforms": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#MessageTransform`) } ], "tags": { string: string, ... } } ``` |

| Fields ||
|---|---|
| `name` | `string` Required. Name of the subscription. Format is `projects/{project}/subscriptions/{sub}`. |
| `pushConfig` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PushConfig`)`` Optional. If push delivery is used with this subscription, this field is used to configure it. |
| `bigqueryConfig` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#BigQueryConfig`)`` Optional. If delivery to BigQuery is used with this subscription, this field is used to configure it. |
| `cloudStorageConfig` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#CloudStorageConfig`)`` Optional. If delivery to Google Cloud Storage is used with this subscription, this field is used to configure it. |
| `ackDeadlineSeconds` | `integer` Optional. The approximate amount of time (on a best-effort basis) Pub/Sub waits for the subscriber to acknowledge receipt before resending the message. In the interval after the message is delivered and before it is acknowledged, it is considered to be *outstanding*. During that time period, the message will not be redelivered (on a best-effort basis). For pull subscriptions, this value is used as the initial value for the ack deadline. To override this value for a given message, call `ModifyAckDeadline` with the corresponding `ack_id` if using non-streaming pull or send the `ack_id` in a `StreamingModifyAckDeadlineRequest` if using streaming pull. The minimum custom deadline you can specify is 10 seconds. The maximum custom deadline you can specify is 600 seconds (10 minutes). If this parameter is 0, a default value of 10 seconds is used. For push delivery, this value is also used to set the request timeout for the call to the push endpoint. If the subscriber never acknowledges the message, the Pub/Sub system will eventually redeliver the message. |
| `retainAckedMessages` | `boolean` Optional. Indicates whether to retain acknowledged messages. If true, then messages are not expunged from the subscription's backlog, even if they are acknowledged, until they fall out of the `messageRetentionDuration` window. This must be true if you would like to [`Seek` to a timestamp](https://cloud.google.com/pubsub/docs/replay-overview#seek_to_a_time) in the past to replay previously-acknowledged messages. |
| `messageRetentionDuration` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#duration` format)`` Optional. How long to retain unacknowledged messages in the subscription's backlog, from the moment a message is published. If `retainAckedMessages` is true, then this also configures the retention of acknowledged messages, and thus configures how far back in time a `Seek` can be done. Defaults to 7 days. Cannot be more than 31 days or less than 10 minutes. A duration in seconds with up to nine fractional digits, ending with '`s`'. Example: `"3.5s"`. |
| `labels` | `map (key: string, value: string)` Optional. See [Creating and managing labels](https://cloud.google.com/pubsub/docs/labels). An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| `enableMessageOrdering` | `boolean` Optional. If true, messages published with the same `ordering_key` in `PubsubMessage` will be delivered to the subscribers in the order in which they are received by the Pub/Sub system. Otherwise, they may be delivered in any order. |
| `expirationPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#ExpirationPolicy`)`` Optional. A policy that specifies the conditions for this subscription's expiration. A subscription is considered active as long as any connected subscriber is successfully consuming messages from the subscription or is issuing operations on the subscription. If `expirationPolicy` is not set, a *default policy* with `ttl` of 31 days will be used. The minimum allowed value for `expirationPolicy.ttl` is 1 day. If `expirationPolicy` is set, but `expirationPolicy.ttl` is not set, the subscription never expires. |
| `filter` | `string` Optional. An expression written in the Pub/Sub [filter language](https://cloud.google.com/pubsub/docs/filtering). If non-empty, then only `PubsubMessage`s whose `attributes` field matches the filter are delivered on this subscription. If empty, then no messages are filtered out. |
| `deadLetterPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#DeadLetterPolicy`)`` Optional. A policy that specifies the conditions for dead lettering messages in this subscription. If deadLetterPolicy is not set, dead lettering is disabled. The Pub/Sub service account associated with this subscriptions's parent project (i.e., service-{projectNumber}@gcp-sa-pubsub.iam.gserviceaccount.com) must have permission to Acknowledge() messages on this subscription. |
| `retryPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#RetryPolicy`)`` Optional. A policy that specifies how Pub/Sub retries message delivery for this subscription. If not set, the default retry policy is applied. This generally implies that messages will be retried as soon as possible for healthy subscribers. RetryPolicy will be triggered on NACKs or acknowledgement deadline exceeded events for a given message. |
| `detached` | `boolean` Optional. Indicates whether the subscription is detached from its topic. Detached subscriptions don't receive messages from their topic and don't retain any backlog. `Pull` and `StreamingPull` requests will return FAILED_PRECONDITION. If the subscription is a push subscription, pushes to the endpoint will not be made. |
| `enableExactlyOnceDelivery` | `boolean` Optional. If true, Pub/Sub provides the following guarantees for the delivery of a message with a given value of `message_id` on this subscription: - The message sent to a subscriber is guaranteed not to be resent before the message's acknowledgement deadline expires. - An acknowledged message will not be resent to a subscriber. Note that subscribers may still receive multiple copies of a message when `enableExactlyOnceDelivery` is true if the message was published multiple times by a publisher client. These copies are considered distinct by Pub/Sub and have distinct `message_id` values. |
| `messageTransforms[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#MessageTransform`)`` Optional. Transforms to be applied to messages before they are delivered to subscribers. Transforms are applied in the order specified. |
| `tags` | `map (key: string, value: string)` Optional. Input only. Immutable. Tag keys/values directly bound to this resource. For example: "123/environment": "production", "123/costCenter": "marketing" An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |

## PushConfig

Configuration for a push delivery endpoint.

| JSON representation |
|---|
| ``` { "pushEndpoint": string, "attributes": { string: string, ... }, // Union field `authentication_method` can be only one of the following: "oidcToken": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#OidcToken`) } // End of list of possible types for union field `authentication_method`. // Union field `wrapper` can be only one of the following: "pubsubWrapper": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PubsubWrapper`) }, "noWrapper": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#NoWrapper`) } // End of list of possible types for union field `wrapper`. } ``` |

| Fields ||
|---|---|
| `pushEndpoint` | `string` Optional. A URL locating the endpoint to which messages should be pushed. For example, a Webhook endpoint might use `https://example.com/push`. |
| `attributes` | `map (key: string, value: string)` Optional. Endpoint configuration attributes that can be used to control different aspects of the message delivery. The only currently supported attribute is `x-goog-version`, which you can use to change the format of the pushed message. This attribute indicates the version of the data expected by the endpoint. This controls the shape of the pushed message (i.e., its fields and metadata). If not present during the `CreateSubscription` call, it will default to the version of the Pub/Sub API used to make such call. If not present in a `ModifyPushConfig` call, its value will not be changed. `GetSubscription` calls will always return a valid version, even if the subscription was created without this attribute. The only supported values for the `x-goog-version` attribute are: - `v1beta1`: uses the push format defined in the v1beta1 Pub/Sub API. - `v1` or `v1beta2`: uses the push format defined in the v1 Pub/Sub API. For example: `attributes { "x-goog-version": "v1" }` An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| Union field `authentication_method`. An authentication method used by push endpoints to verify the source of push requests. This can be used with push endpoints that are private by default to allow requests only from the Pub/Sub system, for example. This field is optional and should be set only by users interested in authenticated push. `authentication_method` can be only one of the following: ||
| `oidcToken` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#OidcToken`)`` Optional. If specified, Pub/Sub will generate and attach an OIDC JWT token as an `Authorization` header in the HTTP request for every pushed message. |
| Union field `wrapper`. The format of the delivered message to the push endpoint is defined by the chosen wrapper. When unset, `PubsubWrapper` is used. `wrapper` can be only one of the following: ||
| `pubsubWrapper` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#PubsubWrapper`)`` Optional. When set, the payload to the push endpoint is in the form of the JSON representation of a PubsubMessage (<https://cloud.google.com/pubsub/docs/reference/rpc/google.pubsub.v1#pubsubmessage)>. |
| `noWrapper` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#NoWrapper`)`` Optional. When set, the payload to the push endpoint is not wrapped. |

## OidcToken

Contains information needed for generating an [OpenID Connect token](https://developers.google.com/identity/protocols/OpenIDConnect).

| JSON representation |
|---|
| ``` { "serviceAccountEmail": string, "audience": string } ``` |

| Fields ||
|---|---|
| `serviceAccountEmail` | `string` Optional. [Service account email](https://cloud.google.com/iam/docs/service-accounts) used for generating the OIDC token. For more information on setting up authentication, see [Push subscriptions](https://cloud.google.com/pubsub/docs/push). |
| `audience` | `string` Optional. Audience to be used when generating OIDC token. The audience claim identifies the recipients that the JWT is intended for. The audience value is a single case-sensitive string. Having multiple values (array) for the audience field is not supported. More info about the OIDC JWT token audience here: <https://tools.ietf.org/html/rfc7519#section-4.1.3> Note: if not specified, the Push endpoint URL will be used. |

## PubsubWrapper

This type has no fields.
The payload to the push endpoint is in the form of the JSON representation of a PubsubMessage (<https://cloud.google.com/pubsub/docs/reference/rpc/google.pubsub.v1#pubsubmessage)>.

## NoWrapper

Sets the `data` field as the HTTP body for delivery.

| JSON representation |
|---|
| ``` { "writeMetadata": boolean } ``` |

| Fields ||
|---|---|
| `writeMetadata` | `boolean` Optional. When true, writes the Pub/Sub message metadata to `x-goog-pubsub-<KEY>:<VAL>` headers of the HTTP request. Writes the Pub/Sub message attributes to `<KEY>:<VAL>` headers of the HTTP request. |

## BigQueryConfig

Configuration for a BigQuery subscription.

| JSON representation |
|---|
| ``` { "table": string, "useTopicSchema": boolean, "writeMetadata": boolean, "dropUnknownFields": boolean, "useTableSchema": boolean, "serviceAccountEmail": string } ``` |

| Fields ||
|---|---|
| `table` | `string` Optional. The name of the table to which to write data, of the form {projectId}.{datasetId}.{tableId} |
| `useTopicSchema` | `boolean` Optional. When true, use the topic's schema as the columns to write to in BigQuery, if it exists. `useTopicSchema` and `useTableSchema` cannot be enabled at the same time. |
| `writeMetadata` | `boolean` Optional. When true, write the subscription name, message_id, publish_time, attributes, and ordering_key to additional columns in the table. The subscription name, message_id, and publish_time fields are put in their own columns while all other message properties (other than data) are written to a JSON object in the attributes column. |
| `dropUnknownFields` | `boolean` Optional. When true and useTopicSchema is true, any fields that are a part of the topic schema that are not part of the BigQuery table schema are dropped when writing to BigQuery. Otherwise, the schemas must be kept in sync and any messages with extra fields are not written and remain in the subscription's backlog. |
| `useTableSchema` | `boolean` Optional. When true, use the BigQuery table's schema as the columns to write to in BigQuery. `useTableSchema` and `useTopicSchema` cannot be enabled at the same time. |
| `serviceAccountEmail` | `string` Optional. The service account to use to write to BigQuery. The subscription creator or updater that specifies this field must have `iam.serviceAccounts.actAs` permission on the service account. If not specified, the Pub/Sub [service agent](https://cloud.google.com/iam/docs/service-agents), service-{projectNumber}@gcp-sa-pubsub.iam.gserviceaccount.com, is used. |

## CloudStorageConfig

Configuration for a Cloud Storage subscription.

| JSON representation |
|---|
| ``` { "bucket": string, "filenamePrefix": string, "filenameSuffix": string, "filenameDatetimeFormat": string, "maxDuration": string, "maxBytes": string, "maxMessages": string, "serviceAccountEmail": string, // Union field `output_format` can be only one of the following: "textConfig": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#TextConfig`) }, "avroConfig": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#AvroConfig`) } // End of list of possible types for union field `output_format`. } ``` |

| Fields ||
|---|---|
| `bucket` | `string` Required. User-provided name for the Cloud Storage bucket. The bucket must be created by the user. The bucket name must be without any prefix like "gs://". See the [bucket naming requirements](https://cloud.google.com/storage/docs/buckets#naming). |
| `filenamePrefix` | `string` Optional. User-provided prefix for Cloud Storage filename. See the [object naming requirements](https://cloud.google.com/storage/docs/objects#naming). |
| `filenameSuffix` | `string` Optional. User-provided suffix for Cloud Storage filename. See the [object naming requirements](https://cloud.google.com/storage/docs/objects#naming). Must not end in "/". |
| `filenameDatetimeFormat` | `string` Optional. User-provided format string specifying how to represent datetimes in Cloud Storage filenames. See the [datetime format guidance](https://cloud.google.com/pubsub/docs/create-cloudstorage-subscription#file_names). |
| `maxDuration` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#duration` format)`` Optional. File batching settings. If no maxDuration setting is specified, a maxDuration of 5 minutes will be set by default. maxDuration is required regardless of whether other file batching settings are specified. The maximum duration that can elapse before a new Cloud Storage file is created. Min 1 minute, max 10 minutes, default 5 minutes. May not exceed the subscription's acknowledgement deadline. A duration in seconds with up to nine fractional digits, ending with '`s`'. Example: `"3.5s"`. |
| `maxBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The maximum bytes that can be written to a Cloud Storage file before a new file is created. Min 1 KB, max 10 GiB. The maxBytes limit may be exceeded in cases where messages are larger than the limit. |
| `maxMessages` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The maximum number of messages that can be written to a Cloud Storage file before a new file is created. Min 1000 messages. |
| `serviceAccountEmail` | `string` Optional. The service account to use to write to Cloud Storage. The subscription creator or updater that specifies this field must have `iam.serviceAccounts.actAs` permission on the service account. If not specified, the Pub/Sub [service agent](https://cloud.google.com/iam/docs/service-agents), service-{projectNumber}@gcp-sa-pubsub.iam.gserviceaccount.com, is used. |
| Union field `output_format`. Defaults to text format. `output_format` can be only one of the following: ||
| `textConfig` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#TextConfig`)`` Optional. If set, message data will be written to Cloud Storage in text format. |
| `avroConfig` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#AvroConfig`)`` Optional. If set, message data will be written to Cloud Storage in Avro format. |

## TextConfig

This type has no fields.
Configuration for writing message data in text format. Message payloads will be written to files as raw text, separated by a newline.

## AvroConfig

Configuration for writing message data in Avro format. Message payloads and metadata will be written to files as an Avro binary.

| JSON representation |
|---|
| ``` { "writeMetadata": boolean, "useTopicSchema": boolean } ``` |

| Fields ||
|---|---|
| `writeMetadata` | `boolean` Optional. When true, write the subscription name, message_id, publish_time, attributes, and ordering_key as additional fields in the output. The subscription name, message_id, and publish_time fields are put in their own fields while all other message properties other than data (for example, an ordering_key, if present) are added as entries in the attributes map. |
| `useTopicSchema` | `boolean` Optional. When true, the output Cloud Storage file will be serialized using the topic schema, if it exists. |

## ExpirationPolicy

A policy that specifies the conditions for resource expiration (i.e., automatic resource deletion).

| JSON representation |
|---|
| ``` { "ttl": string } ``` |

| Fields ||
|---|---|
| `ttl` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#duration` format)`` Optional. Specifies the "time-to-live" duration for an associated resource. The resource expires if it is not active for a period of `ttl`. The definition of "activity" depends on the type of the associated resource. The minimum and maximum allowed values for `ttl` depend on the type of the associated resource, as well. If `ttl` is not set, the associated resource never expires. A duration in seconds with up to nine fractional digits, ending with '`s`'. Example: `"3.5s"`. |

## DeadLetterPolicy

Dead lettering is done on a best effort basis. The same message might be dead lettered multiple times.

If validation on any of the fields fails at subscription creation/updation, the create/update subscription request will fail.

| JSON representation |
|---|
| ``` { "deadLetterTopic": string, "maxDeliveryAttempts": integer } ``` |

| Fields ||
|---|---|
| `deadLetterTopic` | `string` Optional. The name of the topic to which dead letter messages should be published. Format is `projects/{project}/topics/{topic}`.The Pub/Sub service account associated with the enclosing subscription's parent project (i.e., service-{projectNumber}@gcp-sa-pubsub.iam.gserviceaccount.com) must have permission to Publish() to this topic. The operation will fail if the topic does not exist. Users should ensure that there is a subscription attached to this topic since messages published to a topic with no subscriptions are lost. |
| `maxDeliveryAttempts` | `integer` Optional. The maximum number of delivery attempts for any message. The value must be between 5 and 100. The number of delivery attempts is defined as 1 + (the sum of number of NACKs and number of times the acknowledgement deadline has been exceeded for the message). A NACK is any call to ModifyAckDeadline with a 0 deadline. Note that client libraries may automatically extend ack_deadlines. This field will be honored on a best effort basis. If this parameter is 0, a default value of 5 is used. |

## RetryPolicy

A policy that specifies how Pub/Sub retries message delivery.

Retry delay will be exponential based on provided minimum and maximum backoffs. <https://en.wikipedia.org/wiki/Exponential_backoff>.

RetryPolicy will be triggered on NACKs or acknowledgement deadline exceeded events for a given message.

Retry Policy is implemented on a best effort basis. At times, the delay between consecutive deliveries may not match the configuration. That is, delay can be more or less than configured backoff.

| JSON representation |
|---|
| ``` { "minimumBackoff": string, "maximumBackoff": string } ``` |

| Fields ||
|---|---|
| `minimumBackoff` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#duration` format)`` Optional. The minimum delay between consecutive deliveries of a given message. Value should be between 0 and 600 seconds. Defaults to 10 seconds. A duration in seconds with up to nine fractional digits, ending with '`s`'. Example: `"3.5s"`. |
| `maximumBackoff` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#duration` format)`` Optional. The maximum delay between consecutive deliveries of a given message. Value should be between 0 and 600 seconds. Defaults to 600 seconds. A duration in seconds with up to nine fractional digits, ending with '`s`'. Example: `"3.5s"`. |

## MessageTransform

All supported message transforms types.

| JSON representation |
|---|
| ``` { "enabled": boolean, "disabled": boolean, // Union field `transform` can be only one of the following: "javascriptUdf": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#JavaScriptUDF`) } // End of list of possible types for union field `transform`. } ``` |

| Fields ||
|---|---|
| `enabled (deprecated)` | `boolean` > [!WARNING] > This item is deprecated! Optional. This field is deprecated, use the `disabled` field to disable transforms. |
| `disabled` | `boolean` Optional. If true, the transform is disabled and will not be applied to messages. Defaults to `false`. |
| Union field `transform`. The type of transform to apply to messages. `transform` can be only one of the following: ||
| `javascriptUdf` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#JavaScriptUDF`)`` Optional. JavaScript User Defined Function. If multiple JavaScriptUDF's are specified on a resource, each must have a unique `functionName`. |

## JavaScriptUDF

User-defined JavaScript function that can transform or filter a Pub/Sub message.

| JSON representation |
|---|
| ``` { "functionName": string, "code": string } ``` |

| Fields ||
|---|---|
| `functionName` | `string` Required. Name of the JavasScript function that should applied to Pub/Sub messages. |
| `code` | `string` Required. JavaScript code that contains a function `functionName` with the below signature: /** * Transforms a Pub/Sub message. * @return {(Object<string, (string | Object<string, string>)>|null)} - To * filter a message, return `null`. To transform a message return a map * with the following keys: *   - (required) 'data' : {string} *   - (optional) 'attributes' : {Object<string, string>} * Returning empty `attributes` will remove all attributes from the * message. * * @param  {(Object<string, (string | Object<string, string>)>} Pub/Sub * message. Keys: *   - (required) 'data' : {string} *   - (required) 'attributes' : {Object<string, string>} * * @param  {Object<string, any>} metadata - Pub/Sub message metadata. * Keys: *   - (required) 'message_id'  : {string} *   - (optional) 'publish_time': {string} YYYY-MM-DDTHH:MM:SSZ format *   - (optional) 'ordering_key': {string} */ function <functionName>(message, metadata) { } |