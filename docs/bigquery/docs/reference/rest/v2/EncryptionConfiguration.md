# EncryptionConfiguration

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/EncryptionConfiguration#SCHEMA_REPRESENTATION)

Configuration for Cloud KMS encryption settings.

| JSON representation |
|---|
| ``` { "kmsKeyName": string } ``` |

| Fields ||
|---|---|
| `kmsKeyName` | `string` Optional. Describes the Cloud KMS encryption key that will be used to protect destination BigQuery table. The BigQuery Service Account associated with your project requires access to this encryption key. |