# ListSharedResourceSubscriptionsResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/ListSharedResourceSubscriptionsResponse#SCHEMA_REPRESENTATION)

Message for response to the listing of shared resource subscriptions.

| JSON representation |
|---|
| ``` { "sharedResourceSubscriptions": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `sharedResourceSubscriptions[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Subscription`)`` The list of subscriptions. |
| `nextPageToken` | `string` Next page token. |