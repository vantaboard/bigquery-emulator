# FederatedAttributeStatus

Explicit indication of why a particular federated attribute is not included in a request. This is necessary because the server needs to behave differently if an attribute is federated and known to be empty than if the caller is expecting IAM to read it from central storage. It also allows the server to identify requests where the caller failed to populate a particular attribute due to a bug. If the resource doesn't exist, then use FEDERATED_AND_EMPTY.

| Enums ||
|---|---|
| `FEDERATION_STATUS_UNSET` |   |
| `NOT_FEDERATED` | This attribute is not provided in the request and instead should be read from IAM central storage. |
| `FEDERATED_AND_EMPTY` | This attribute is stored by the calling service, but is empty or unset for this particular resource. |