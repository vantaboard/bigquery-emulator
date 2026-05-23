## Index

- `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Locations` (interface)
- `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.GetLocationRequest` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.ListLocationsRequest` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.ListLocationsResponse` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Location` (message)

## Locations

An abstract interface that provides location-related information for a service. Service-specific metadata is provided through the `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Location.FIELDS.google.protobuf.Any.google.cloud.location.Location.metadata` field.

| GetLocation |
|---|
| `` rpc GetLocation(`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.GetLocationRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Location`) `` Gets information about a location. Authorization scopes :   Requires the following OAuth scope: - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| ListLocations |
|---|
| `` rpc ListLocations(`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.ListLocationsRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.ListLocationsResponse`) `` Lists information about the supported locations for this service. This method lists locations based on the resource scope provided in the `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.ListLocationsRequest.FIELDS.string.google.cloud.location.ListLocationsRequest.name` field: - **Global locations** : If `name` is empty, the method lists the public locations available to all projects. - **Project-specific locations** : If `name` follows the format `projects/{project}`, the method lists locations visible to that specific project. This includes public, private, or other project-specific locations enabled for the project. For gRPC and client library implementations, the resource name is passed as the `name` field. For direct service calls, the resource name is incorporated into the request path based on the specific service implementation and version. Authorization scopes :   Requires the following OAuth scope: - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

## GetLocationRequest

The request message for `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Locations.GetLocation`.

| Fields ||
|---|---|
| `name` | `string` Resource name for the location. |

## ListLocationsRequest

The request message for `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Locations.ListLocations`.

| Fields ||
|---|---|
| `name` | `string` The resource that owns the locations collection, if applicable. |
| `filter` | `string` A filter to narrow down results to a preferred subset. The filtering language accepts strings like `"displayName=tokyo"`, and is documented in more detail in [AIP-160](https://google.aip.dev/160). |
| `page_size` | `int32` The maximum number of results to return. If not set, the service selects a default. |
| `page_token` | `string` A page token received from the `next_page_token` field in the response. Send that page token to receive the subsequent page. |
| `extra_location_types[]` | `string` Optional. Do not use this field. It is unsupported and is ignored unless explicitly documented otherwise. This is primarily for internal usage. |

## ListLocationsResponse

The response message for `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Locations.ListLocations`.

| Fields ||
|---|---|
| `locations[]` | `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rpc/google.cloud.location#google.cloud.location.Location` A list of locations that matches the specified filter in the request. |
| `next_page_token` | `string` The standard List next-page token. |

## Location

A resource that represents a Google Cloud location.

| Fields ||
|---|---|
| `name` | `string` Resource name for the location, which may vary between implementations. For example: `"projects/example-project/locations/us-east1"` |
| `location_id` | `string` The canonical id for this location. For example: `"us-east1"`. |
| `display_name` | `string` The friendly name for this location, typically a nearby city name. For example, "Tokyo". |
| `labels` | `map<string, string>` Cross-service attributes for the location. For example {"cloud.googleapis.com/region": "us-east1"} |
| `metadata` | `https://protobuf.dev/reference/protobuf/google.protobuf#any` Service-specific metadata. For example the available capacity at the given location. |