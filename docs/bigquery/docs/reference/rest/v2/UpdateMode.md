# UpdateMode specifies which dataset fields is updated.

| Enums ||
|---|---|
| `UPDATE_MODE_UNSPECIFIED` | The default value. Default to the UPDATE_FULL. |
| `UPDATE_METADATA` | Includes metadata information for the dataset, such as friendlyName, description, labels, etc. |
| `UPDATE_ACL` | Includes ACL information for the dataset, which defines dataset access for one or more entities. |
| `UPDATE_FULL` | Includes both dataset metadata and ACL information. |