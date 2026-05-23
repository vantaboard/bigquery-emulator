# Package google.cloud.bigquery.connection.v1beta1

## Index

- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionService` (interface)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CloudSqlCredential` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CloudSqlProperties` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CloudSqlProperties.DatabaseType` (enum)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.Connection` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionCredential` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CreateConnectionRequest` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.DeleteConnectionRequest` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.GetConnectionRequest` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ListConnectionsRequest` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ListConnectionsResponse` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.UpdateConnectionCredentialRequest` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.UpdateConnectionRequest` (message)

## ConnectionService

Manages external data source connections and credentials.

| CreateConnection |
|---|
| `` rpc CreateConnection(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CreateConnectionRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.Connection`) `` Creates a new connection. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| DeleteConnection |
|---|
| `` rpc DeleteConnection(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.DeleteConnectionRequest`) returns (`https://protobuf.dev/reference/protobuf/google.protobuf#empty`) `` Deletes connection and associated credential. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| GetConnection |
|---|
| `` rpc GetConnection(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.GetConnectionRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.Connection`) `` Returns specified connection. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| GetIamPolicy |
|---|
| `` rpc GetIamPolicy(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.iam.v1#google.iam.v1.GetIamPolicyRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.iam.v1#google.iam.v1.Policy`) `` Gets the access control policy for a resource. Returns an empty policy if the resource exists and does not have a policy set. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| ListConnections |
|---|
| `` rpc ListConnections(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ListConnectionsRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ListConnectionsResponse`) `` Returns a list of connections in the given project. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| SetIamPolicy |
|---|
| `` rpc SetIamPolicy(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.iam.v1#google.iam.v1.SetIamPolicyRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.iam.v1#google.iam.v1.Policy`) `` Sets the access control policy on the specified resource. Replaces any existing policy. Can return `NOT_FOUND`, `INVALID_ARGUMENT`, and `PERMISSION_DENIED` errors. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| TestIamPermissions |
|---|
| `` rpc TestIamPermissions(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.iam.v1#google.iam.v1.TestIamPermissionsRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.iam.v1#google.iam.v1.TestIamPermissionsResponse`) `` Returns permissions that a caller has on the specified resource. If the resource does not exist, this will return an empty set of permissions, not a `NOT_FOUND` error. Note: This operation is designed to be used for building permission-aware UIs and command-line tools, not for authorization checking. This operation may "fail open" without warning. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| UpdateConnection |
|---|
| `` rpc UpdateConnection(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.UpdateConnectionRequest`) returns (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.Connection`) `` Updates the specified connection. For security reasons, also resets credential if connection properties are in the update field mask. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

| UpdateConnectionCredential |
|---|
| `` rpc UpdateConnectionCredential(`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.UpdateConnectionCredentialRequest`) returns (`https://protobuf.dev/reference/protobuf/google.protobuf#empty`) `` Sets the credential for the specified connection. Authorization scopes :   Requires one of the following OAuth scopes: - `https://www.googleapis.com/auth/bigquery` - `https://www.googleapis.com/auth/cloud-platform` For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp). |

## CloudSqlCredential

Credential info for the Cloud SQL.

| Fields ||
|---|---|
| `username` | `string` The username for the credential. |
| `password` | `string` The password for the credential. |

## CloudSqlProperties

Connection properties specific to the Cloud SQL.

| Fields ||
|---|---|
| `instance_id` | `string` Cloud SQL instance ID in the form `project:location:instance`. |
| `database` | `string` Database name. |
| `type` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CloudSqlProperties.DatabaseType` Type of the Cloud SQL database. |
| `credential` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CloudSqlCredential` Input only. Cloud SQL credential. |
| `service_account_id` | `string` Output only. The account ID of the service used for the purpose of this connection. When the connection is used in the context of an operation in BigQuery, this service account will serve as the identity being used for connecting to the CloudSQL instance specified in this connection. |

## DatabaseType

Supported Cloud SQL database types.

| Enums ||
|---|---|
| `DATABASE_TYPE_UNSPECIFIED` | Unspecified database type. |
| `POSTGRES` | Cloud SQL for PostgreSQL. |
| `MYSQL` | Cloud SQL for MySQL. |

## Connection

Configuration parameters to establish connection with an external data source, except the credential attributes.

| Fields ||
|---|---|
| `name` | `string` The resource name of the connection in the form of: `projects/{project_id}/locations/{location_id}/connections/{connection_id}` |
| `friendly_name` | `string` User provided display name for the connection. |
| `description` | `string` User provided description. |
| `creation_time` | `int64` Output only. The creation timestamp of the connection. |
| `last_modified_time` | `int64` Output only. The last update timestamp of the connection. |
| `has_credential` | `bool` Output only. True, if credential is configured for this connection. |
| Union field `properties`. Properties specific to the underlying data source. `properties` can be only one of the following: ||
| `cloud_sql` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CloudSqlProperties` Cloud SQL properties. |

## ConnectionCredential

Credential to use with a connection.

| Fields ||
|---|---|
| Union field `credential`. Credential specific to the underlying data source. `credential` can be only one of the following: ||
| `cloud_sql` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.CloudSqlCredential` Credential for Cloud SQL database. |

## CreateConnectionRequest

The request for `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionService.CreateConnection`.

| Fields ||
|---|---|
| `parent` | `string` Required. Parent resource name. Must be in the format `projects/{project_id}/locations/{location_id}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `parent`: - `bigquery.connections.create` |
| `connection_id` | `string` Optional. Connection id that should be assigned to the created connection. |
| `connection` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.Connection` Required. Connection to create. |

## DeleteConnectionRequest

The request for \[ConnectionService.DeleteConnectionRequest\]\[\].

| Fields ||
|---|---|
| `name` | `string` Required. Name of the deleted connection, for example: `projects/{project_id}/locations/{location_id}/connections/{connection_id}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.connections.delete` |

## GetConnectionRequest

The request for `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionService.GetConnection`.

| Fields ||
|---|---|
| `name` | `string` Required. Name of the requested connection, for example: `projects/{project_id}/locations/{location_id}/connections/{connection_id}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.connections.get` |

## ListConnectionsRequest

The request for `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionService.ListConnections`.

| Fields ||
|---|---|
| `parent` | `string` Required. Parent resource name. Must be in the form: `projects/{project_id}/locations/{location_id}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `parent`: - `bigquery.connections.list` |
| `max_results` | `https://protobuf.dev/reference/protobuf/google.protobuf#uint32-value` Required. Maximum number of results per page. |
| `page_token` | `string` Page token. |

## ListConnectionsResponse

The response for `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionService.ListConnections`.

| Fields ||
|---|---|
| `next_page_token` | `string` Next page token. |
| `connections[]` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.Connection` List of connections. |

## UpdateConnectionCredentialRequest

The request for `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionService.UpdateConnectionCredential`.

| Fields ||
|---|---|
| `name` | `string` Required. Name of the connection, for example: `projects/{project_id}/locations/{location_id}/connections/{connection_id}/credential` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.connections.update` |
| `credential` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionCredential` Required. Credential to use with the connection. |

## UpdateConnectionRequest

The request for `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.ConnectionService.UpdateConnection`.

| Fields ||
|---|---|
| `name` | `string` Required. Name of the connection to update, for example: `projects/{project_id}/locations/{location_id}/connections/{connection_id}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.connections.update` |
| `connection` | `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1beta1#google.cloud.bigquery.connection.v1beta1.Connection` Required. Connection containing the updated fields. |
| `update_mask` | `https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` Required. Update mask for the connection fields to be updated. |