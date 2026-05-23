# Method: projects.locations.reservations.failoverReservation

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#body.aspect)
- [FailoverMode](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#FailoverMode)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#try-it)

Fail over a reservation to the secondary location. The operation should be done in the current secondary location, which will be promoted to the new primary location for the reservation. Attempting to failover a reservation in the current primary location will fail with the error code `google.rpc.Code.FAILED_PRECONDITION`.

### HTTP request

`POST https://bigqueryreservation.googleapis.com/v1/{name=projects/*/locations/*/reservations/*}:failoverReservation`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. Resource name of the reservation to failover. E.g., `projects/myproject/locations/US/reservations/team1-prod` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.reservations.update` |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { "failoverMode": enum (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#FailoverMode`) } ``` |

| Fields ||
|---|---|
| `failoverMode` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation#FailoverMode`)`` Optional. A parameter that determines how writes that are pending replication are handled after a failover is initiated. If not specified, HARD failover mode is used by default. |

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations#Reservation`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

## FailoverMode

The failover mode when a user initiates a failover on a reservation determines how writes that are pending replication are handled after the failover is initiated.

| Enums ||
|---|---|
| `FAILOVER_MODE_UNSPECIFIED` | Invalid value. |
| `SOFT` | When customers initiate a soft failover, BigQuery will wait until all committed writes are replicated to the secondary. This mode requires both regions to be available for the failover to succeed and prevents data loss. |
| `HARD` | When customers initiate a hard failover, BigQuery will not wait until all committed writes are replicated to the secondary. There can be data loss for hard failover. |