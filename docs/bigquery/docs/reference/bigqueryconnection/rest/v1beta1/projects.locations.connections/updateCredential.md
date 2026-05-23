# Method: projects.locations.connections.updateCredential

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#body.aspect)
- [ConnectionCredential](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#ConnectionCredential)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#ConnectionCredential.SCHEMA_REPRESENTATION)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#try-it)

Sets the credential for the specified connection.

### HTTP request

`PATCH https://bigqueryconnection.googleapis.com/v1beta1/{name=projects/*/locations/*/connections/*/credential}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. Name of the connection, for example: `projects/{projectId}/locations/{locationId}/connections/{connectionId}/credential` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.connections.update` |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential#ConnectionCredential`.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

## ConnectionCredential

Credential to use with a connection.

| JSON representation |
|---|
| ``` { // Union field `credential` can be only one of the following: "cloudSql": { object (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlCredential`) } // End of list of possible types for union field `credential`. } ``` |

| Fields ||
|---|---|
| Union field `credential`. Credential specific to the underlying data source. `credential` can be only one of the following: ||
| `cloudSql` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlCredential`)`` Credential for Cloud SQL database. |