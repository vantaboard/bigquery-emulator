# REST Resource: datasets

- [Resource: Dataset](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#Dataset)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#Dataset.SCHEMA_REPRESENTATION)
- [DatasetReference](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference.SCHEMA_REPRESENTATION)
- [LinkedDatasetSource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetSource)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetSource.SCHEMA_REPRESENTATION)
- [LinkedDatasetMetadata](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetMetadata)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetMetadata.SCHEMA_REPRESENTATION)
- [LinkState](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkState)
- [ExternalDatasetReference](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalDatasetReference)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalDatasetReference.SCHEMA_REPRESENTATION)
- [ExternalCatalogDatasetOptions](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalCatalogDatasetOptions)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalCatalogDatasetOptions.SCHEMA_REPRESENTATION)
- [GcpTag](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#GcpTag)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#GcpTag.SCHEMA_REPRESENTATION)
- [StorageBillingModel](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#StorageBillingModel)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#METHODS_SUMMARY)

## Resource: Dataset

Represents a BigQuery dataset.

| JSON representation |
|---|
| ``` { "kind": string, "etag": string, "id": string, "selfLink": string, "datasetReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`) }, "friendlyName": string, "description": string, "defaultTableExpirationMs": string, "defaultPartitionExpirationMs": string, "labels": { string: string, ... }, "access": [ { "role": string, "userByEmail": string, "groupByEmail": string, "domain": string, "specialGroup": string, "iamMember": string, "view": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/TableReference`) }, "routine": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineReference`) }, "dataset": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DatasetAccessEntry`) }, "condition": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Policy#Expr`) } } ], "creationTime": string, "lastModifiedTime": string, "location": string, "defaultEncryptionConfiguration": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/EncryptionConfiguration`) }, "satisfiesPzs": boolean, "satisfiesPzi": boolean, "type": string, "linkedDatasetSource": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetSource`) }, "linkedDatasetMetadata": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetMetadata`) }, "externalDatasetReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalDatasetReference`) }, "externalCatalogDatasetOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalCatalogDatasetOptions`) }, "isCaseInsensitive": boolean, "defaultCollation": string, "defaultRoundingMode": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/RoundingMode`), "maxTimeTravelHours": string, "tags": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#GcpTag`) } ], "storageBillingModel": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#StorageBillingModel`), "resourceTags": { string: string, ... }, "catalogSource": string } ``` |

| Fields ||
|---|---|
| `kind` | `string` Output only. The resource type. |
| `etag` | `string` Output only. A hash of the resource. |
| `id` | `string` Output only. The fully-qualified unique name of the dataset in the format projectId:datasetId. The dataset name without the project name is given in the datasetId field. When creating a new dataset, leave this field blank, and instead specify the datasetId field. |
| `selfLink` | `string` Output only. A URL that can be used to access the resource again. You can use this URL in Get or Update requests to the resource. |
| `datasetReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`)`` Required. A reference that identifies the dataset. |
| `friendlyName` | `string` Optional. A descriptive name for the dataset. |
| `description` | `string` Optional. A user-friendly description of the dataset. |
| `defaultTableExpirationMs` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The default lifetime of all tables in the dataset, in milliseconds. The minimum lifetime value is 3600000 milliseconds (one hour). To clear an existing default expiration with a PATCH request, set to 0. Once this property is set, all newly-created tables in the dataset will have an expirationTime property set to the creation time plus the value in this property, and changing the value will only affect new tables, not existing ones. When the expirationTime for a given table is reached, that table will be deleted automatically. If a table's expirationTime is modified or removed before the table expires, or if you provide an explicit expirationTime when creating a table, that value takes precedence over the default expiration time indicated by this property. |
| `defaultPartitionExpirationMs` | `string (https://developers.google.com/discovery/v1/type-format format)` This default partition expiration, expressed in milliseconds. When new time-partitioned tables are created in a dataset where this property is set, the table will inherit this value, propagated as the `TimePartitioning.expirationMs` property on the new table. If you set `TimePartitioning.expirationMs` explicitly when creating a table, the `defaultPartitionExpirationMs` of the containing dataset is ignored. When creating a partitioned table, if `defaultPartitionExpirationMs` is set, the `defaultTableExpirationMs` value is ignored and the table will not be inherit a table expiration deadline. |
| `labels` | `map (key: string, value: string)` The labels associated with this dataset. You can use these to organize and group your datasets. You can set this property when inserting or updating a dataset. See [Creating and Updating Dataset Labels](https://cloud.google.com/bigquery/docs/creating-managing-labels#creating_and_updating_dataset_labels) for more information. |
| `access[]` | `object` Optional. An array of objects that define dataset access for one or more entities. You can set this property when inserting or updating a dataset in order to control who is allowed to access the data. If unspecified at dataset creation time, BigQuery adds default dataset access for the following entities: access.specialGroup: projectReaders; access.role: READER; access.specialGroup: projectWriters; access.role: WRITER; access.specialGroup: projectOwners; access.role: OWNER; access.userByEmail: \[dataset creator email\]; access.role: OWNER; If you patch a dataset, then this field is overwritten by the patched dataset's access field. To add entities, you must supply the entire existing access array in addition to any new entities that you want to add. |
| `access[].role` | `string` An IAM role ID that should be granted to the user, group, or domain specified in this access entry. The following legacy mappings will be applied: - `OWNER`: `roles/bigquery.dataOwner` - `WRITER`: `roles/bigquery.dataEditor` - `READER`: `roles/bigquery.dataViewer` This field will accept any of the above formats, but will return only the legacy format. For example, if you set this field to "roles/bigquery.dataOwner", it will be returned back as "OWNER". |
| `access[].userByEmail` | `string` \[Pick one\] An email address of a user to grant access to. For example: [fred@example.com](mailto:fred@example.com). Maps to IAM policy member "user:EMAIL" or "serviceAccount:EMAIL". |
| `access[].groupByEmail` | `string` \[Pick one\] An email address of a Google Group to grant access to. Maps to IAM policy member "group:GROUP". |
| `access[].domain` | `string` \[Pick one\] A domain to grant access to. Any users signed in with the domain specified will be granted the specified access. Example: "example.com". Maps to IAM policy member "domain:DOMAIN". |
| `access[].specialGroup` | `string` \[Pick one\] A special group to grant access to. Possible values include: - projectOwners: Owners of the enclosing project. - projectReaders: Readers of the enclosing project. - projectWriters: Writers of the enclosing project. - allAuthenticatedUsers: All authenticated BigQuery users. Maps to similarly-named IAM members. |
| `access[].iamMember` | `string` \[Pick one\] Some other type of member that appears in the IAM Policy but isn't a user, group, domain, or special group. |
| `access[].view` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/TableReference`)`` \[Pick one\] A view from a different dataset to grant access to. Queries executed against that view will have read access to views/tables/routines in this dataset. The role field is not required when this field is set. If that view is updated by any user, access to the view needs to be granted again via an update operation. |
| `access[].routine` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineReference`)`` \[Pick one\] A routine from a different dataset to grant access to. Queries executed against that routine will have read access to views/tables/routines in this dataset. Only UDF is supported for now. The role field is not required when this field is set. If that routine is updated by any user, access to the routine needs to be granted again via an update operation. |
| `access[].dataset` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DatasetAccessEntry`)`` \[Pick one\] A grant authorizing all resources of a particular type in a particular dataset access to this dataset. Only views are supported for now. The role field is not required when this field is set. If that dataset is deleted and re-created, its access needs to be granted again via an update operation. |
| `access[].condition` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Policy#Expr`)`` Optional. condition for the binding. If CEL expression in this field is true, this access binding will be considered |
| `creationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The time when this dataset was created, in milliseconds since the epoch. |
| `lastModifiedTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The date when this dataset was last modified, in milliseconds since the epoch. |
| `location` | `string` The geographic location where the dataset should reside. See <https://cloud.google.com/bigquery/docs/locations> for supported locations. |
| `defaultEncryptionConfiguration` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/EncryptionConfiguration`)`` The default encryption key for all tables in the dataset. After this property is set, the encryption key of all newly-created tables in the dataset is set to this value unless the table creation request or query explicitly overrides the key. |
| `satisfiesPzs` | `boolean` Output only. Reserved for future use. |
| `satisfiesPzi` | `boolean` Output only. Reserved for future use. |
| `type` | `string` Output only. Same as `type` in `ListFormatDataset`. The type of the dataset, one of: - DEFAULT - only accessible by owner and authorized accounts, - PUBLIC - accessible by everyone, - LINKED - linked dataset, - EXTERNAL - dataset with definition in external metadata catalog, - BIGLAKE_ICEBERG - a Biglake dataset accessible through the Iceberg API, - BIGLAKE_HIVE - a Biglake dataset accessible through the Hive API. |
| `linkedDatasetSource` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetSource`)`` Optional. The source dataset reference when the dataset is of type LINKED. For all other dataset types it is not set. This field cannot be updated once it is set. Any attempt to update this field using Update and Patch API Operations will be ignored. |
| `linkedDatasetMetadata` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkedDatasetMetadata`)`` Output only. Metadata about the LinkedDataset. Filled out when the dataset type is LINKED. |
| `externalDatasetReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalDatasetReference`)`` Optional. Reference to a read-only external dataset defined in data catalogs outside of BigQuery. Filled out when the dataset type is EXTERNAL. |
| `externalCatalogDatasetOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#ExternalCatalogDatasetOptions`)`` Optional. Options defining open source compatible datasets living in the BigQuery catalog. Contains metadata of open source database, schema or namespace represented by the current dataset. |
| `isCaseInsensitive` | `boolean` Optional. TRUE if the dataset and its table names are case-insensitive, otherwise FALSE. By default, this is FALSE, which means the dataset and its table names are case-sensitive. This field does not affect routine references. |
| `defaultCollation` | `string` Optional. Defines the default collation specification of future tables created in the dataset. If a table is created in this dataset without table-level default collation, then the table inherits the dataset default collation, which is applied to the string fields that do not have explicit collation specified. A change to this field affects only tables created afterwards, and does not alter the existing tables. The following values are supported: - 'und:ci': undetermined locale, case insensitive. - '': empty string. Default to case-sensitive behavior. |
| `defaultRoundingMode` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/RoundingMode`)`` Optional. Defines the default rounding mode specification of new tables created within this dataset. During table creation, if this field is specified, the table within this dataset will inherit the default rounding mode of the dataset. Setting the default rounding mode on a table overrides this option. Existing tables in the dataset are unaffected. If columns are defined during that table creation, they will immediately inherit the table's default rounding mode, unless otherwise specified. |
| `maxTimeTravelHours` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Defines the time travel window in hours. The value can be from 48 to 168 hours (2 to 7 days). The default value is 168 hours if this is not set. |
| `tags[] (deprecated)` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#GcpTag`)`` > [!WARNING] > This item is deprecated! Output only. Tags for the dataset. To provide tags as inputs, use the `resourceTags` field. |
| `storageBillingModel` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#StorageBillingModel`)`` Optional. Updates storageBillingModel for the dataset. |
| `resourceTags` | `map (key: string, value: string)` Optional. The [tags](https://cloud.google.com/bigquery/docs/tags) attached to this dataset. Tag keys are globally unique. Tag key is expected to be in the namespaced format, for example "123456789012/environment" where 123456789012 is the ID of the parent organization or project resource for this tag key. Tag value is expected to be the short name, for example "Production". See [Tag definitions](https://cloud.google.com/iam/docs/tags-access-control#definitions) for more details. |
| `catalogSource` | `string` Output only. The origin of the dataset, one of: - (Unset) - Native BigQuery Dataset - BIGLAKE - Dataset is backed by a namespace stored natively in Biglake |

## DatasetReference

Identifier for a dataset.

| JSON representation |
|---|
| ``` { "datasetId": string, "projectId": string } ``` |

| Fields ||
|---|---|
| `datasetId` | `string` Required. A unique ID for this dataset, without the project name. The ID must contain only letters (a-z, A-Z), numbers (0-9), or underscores (_). The maximum length is 1,024 characters. |
| `projectId` | `string` Optional. The ID of the project containing this dataset. |

## LinkedDatasetSource

A dataset source type which refers to another BigQuery dataset.

| JSON representation |
|---|
| ``` { "sourceDataset": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`) } } ``` |

| Fields ||
|---|---|
| `sourceDataset` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`)`` The source dataset reference contains project numbers and not project ids. |

## LinkedDatasetMetadata

Metadata about the Linked Dataset.

| JSON representation |
|---|
| ``` { "linkState": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkState`) } ``` |

| Fields ||
|---|---|
| `linkState` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#LinkState`)`` Output only. Specifies whether Linked Dataset is currently in a linked state or not. |

## LinkState

Specifies whether Linked Dataset is currently in a linked state or not.

| Enums ||
|---|---|
| `LINK_STATE_UNSPECIFIED` | The default value. Default to the LINKED state. |
| `LINKED` | Normal Linked Dataset state. Data is queryable via the Linked Dataset. |
| `UNLINKED` | Data publisher or owner has unlinked this Linked Dataset. It means you can no longer query or see the data in the Linked Dataset. |

## ExternalDatasetReference

Configures the access a dataset defined in an external metadata storage.

| JSON representation |
|---|
| ``` { "externalSource": string, "connection": string } ``` |

| Fields ||
|---|---|
| `externalSource` | `string` Required. External source that backs this dataset. |
| `connection` | `string` Required. The connection id that is used to access the externalSource. Format: projects/{projectId}/locations/{locationId}/connections/{connectionId} |

## ExternalCatalogDatasetOptions

Options defining open source compatible datasets living in the BigQuery catalog. Contains metadata of open source database, schema, or namespace represented by the current dataset.

| JSON representation |
|---|
| ``` { "parameters": { string: string, ... }, "defaultStorageLocationUri": string } ``` |

| Fields ||
|---|---|
| `parameters` | `map (key: string, value: string)` Optional. A map of key value pairs defining the parameters and properties of the open source schema. Maximum size of 2MiB. |
| `defaultStorageLocationUri` | `string` Optional. The storage location URI for all tables in the dataset. Equivalent to hive metastore's database locationUri. Maximum length of 1024 characters. |

## GcpTag

A global tag managed by Resource Manager. <https://cloud.google.com/iam/docs/tags-access-control#definitions>

| JSON representation |
|---|
| ``` { "tagKey": string, "tagValue": string } ``` |

| Fields ||
|---|---|
| `tagKey` | `string` Required. The namespaced friendly name of the tag key, e.g. "12345/environment" where 12345 is org id. |
| `tagValue` | `string` Required. The friendly short name of the tag value, e.g. "production". |

## StorageBillingModel

Indicates the billing model that will be applied to the dataset.

| Enums ||
|---|---|
| `STORAGE_BILLING_MODEL_UNSPECIFIED` | Value not set. |
| `LOGICAL` | Billing for logical bytes. |
| `PHYSICAL` | Billing for physical bytes. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete` | Deletes the dataset specified by the datasetId value. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get` | Returns the dataset specified by datasetID. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert` | Creates a new empty dataset. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list` | Lists all datasets in the specified project to which the user has been granted the READER dataset role. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch` | Updates information in an existing dataset. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/undelete` | Undeletes a dataset which is within time travel window based on datasetId. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/update` | Updates information in an existing dataset. |