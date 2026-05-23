# REST Resource: projects.locations.connections

- [Resource: Connection](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#Connection)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#Connection.SCHEMA_REPRESENTATION)
- [CloudSqlProperties](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlProperties)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlProperties.SCHEMA_REPRESENTATION)
- [DatabaseType](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#DatabaseType)
- [CloudSqlCredential](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlCredential)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlCredential.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#METHODS_SUMMARY)

## Resource: Connection

Configuration parameters to establish connection with an external data source, except the credential attributes.

| JSON representation |
|---|
| ``` { "name": string, "friendlyName": string, "description": string, "creationTime": string, "lastModifiedTime": string, "hasCredential": boolean, // Union field `properties` can be only one of the following: "cloudSql": { object (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlProperties`) } // End of list of possible types for union field `properties`. } ``` |

| Fields ||
|---|---|
| `name` | `string` The resource name of the connection in the form of: `projects/{projectId}/locations/{locationId}/connections/{connectionId}` |
| `friendlyName` | `string` User provided display name for the connection. |
| `description` | `string` User provided description. |
| `creationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The creation timestamp of the connection. |
| `lastModifiedTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The last update timestamp of the connection. |
| `hasCredential` | `boolean` Output only. True, if credential is configured for this connection. |
| Union field `properties`. Properties specific to the underlying data source. `properties` can be only one of the following: ||
| `cloudSql` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlProperties`)`` Cloud SQL properties. |

## CloudSqlProperties

Connection properties specific to the Cloud SQL.

| JSON representation |
|---|
| ``` { "instanceId": string, "database": string, "type": enum (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#DatabaseType`), "credential": { object (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlCredential`) }, "serviceAccountId": string } ``` |

| Fields ||
|---|---|
| `instanceId` | `string` Cloud SQL instance ID in the form `project:location:instance`. |
| `database` | `string` Database name. |
| `type` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#DatabaseType`)`` Type of the Cloud SQL database. |
| `credential` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections#CloudSqlCredential`)`` Input only. Cloud SQL credential. |
| `serviceAccountId` | `string` Output only. The account ID of the service used for the purpose of this connection. When the connection is used in the context of an operation in BigQuery, this service account will serve as the identity being used for connecting to the CloudSQL instance specified in this connection. |

## DatabaseType

Supported Cloud SQL database types.

| Enums ||
|---|---|
| `DATABASE_TYPE_UNSPECIFIED` | Unspecified database type. |
| `POSTGRES` | Cloud SQL for PostgreSQL. |
| `MYSQL` | Cloud SQL for MySQL. |

## CloudSqlCredential

Credential info for the Cloud SQL.

| JSON representation |
|---|
| ``` { "username": string, "password": string } ``` |

| Fields ||
|---|---|
| `username` | `string` The username for the credential. |
| `password` | `string` The password for the credential. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/create` | Creates a new connection. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/delete` | Deletes connection and associated credential. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/get` | Returns specified connection. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/getIamPolicy` | Gets the access control policy for a resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/list` | Returns a list of connections in the given project. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/patch` | Updates the specified connection. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/setIamPolicy` | Sets the access control policy on the specified resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/testIamPermissions` | Returns permissions that a caller has on the specified resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential` | Sets the credential for the specified connection. |