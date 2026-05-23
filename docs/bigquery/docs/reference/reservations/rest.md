# BigQuery Reservation API

A service to modify your BigQuery reservations.

- [REST Resource: v1.projects.locations](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest#v1.projects.locations)
- [REST Resource: v1.projects.locations.capacityCommitments](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest#v1.projects.locations.capacityCommitments)
- [REST Resource: v1.projects.locations.reservationGroups](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest#v1.projects.locations.reservationGroups)
- [REST Resource: v1.projects.locations.reservations](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest#v1.projects.locations.reservations)
- [REST Resource: v1.projects.locations.reservations.assignments](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest#v1.projects.locations.reservations.assignments)

## Service: bigqueryreservation.googleapis.com

To call this service, we recommend that you use the Google-provided [client libraries](https://cloud.google.com/apis/docs/client-libraries-explained). If your application needs to use your own libraries to call this service, use the following information when you make the API requests.

### Discovery document

A [Discovery Document](https://developers.google.com/discovery/v1/reference/apis) is a machine-readable specification for describing and consuming REST APIs. It is used to build client libraries, IDE plugins, and other tools that interact with Google APIs. One service may provide multiple discovery documents. This service provides the following discovery document:

- <https://bigqueryreservation.googleapis.com/$discovery/rest?version=v1>

### Service endpoint

A [service endpoint](https://cloud.google.com/apis/design/glossary#api_service_endpoint) is a base URL that specifies the network address of an API service. One service might have multiple service endpoints. This service has the following service endpoint and all URIs below are relative to this service endpoint:

- `https://bigqueryreservation.googleapis.com`

## REST Resource: [v1.projects.locations](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/getBiReservation` | `GET /v1/{name=projects/*/locations/*/biReservation}` Retrieves a BI reservation. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments` | `GET /v1/{parent=projects/*/locations/*}:searchAllAssignments` Looks up assignments for a specified resource for a particular region. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAssignments (deprecated)` | `GET /v1/{parent=projects/*/locations/*}:searchAssignments` Deprecated: Looks up assignments for a specified resource for a particular region. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/updateBiReservation` | `PATCH /v1/{biReservation.name=projects/*/locations/*/biReservation}` Updates a BI reservation. |

## REST Resource: [v1.projects.locations.capacityCommitments](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments/create` | `POST /v1/{parent=projects/*/locations/*}/capacityCommitments` Creates a new capacity commitment resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments/delete` | `DELETE /v1/{name=projects/*/locations/*/capacityCommitments/*}` Deletes a capacity commitment. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments/get` | `GET /v1/{name=projects/*/locations/*/capacityCommitments/*}` Returns information about the capacity commitment. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments/list` | `GET /v1/{parent=projects/*/locations/*}/capacityCommitments` Lists all the capacity commitments for the admin project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments/merge` | `POST /v1/{parent=projects/*/locations/*}/capacityCommitments:merge` Merges capacity commitments of the same plan into a single commitment. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments/patch` | `PATCH /v1/{capacityCommitment.name=projects/*/locations/*/capacityCommitments/*}` Updates an existing capacity commitment. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.capacityCommitments/split` | `POST /v1/{name=projects/*/locations/*/capacityCommitments/*}:split` Splits capacity commitment to two commitments of the same plan and `commitment_end_time`. |

## REST Resource: [v1.projects.locations.reservationGroups](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/create` | `POST /v1/{parent=projects/*/locations/*}/reservationGroups` Creates a new reservation group. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/delete` | `DELETE /v1/{name=projects/*/locations/*/reservationGroups/*}` Deletes a reservation. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/get` | `GET /v1/{name=projects/*/locations/*/reservationGroups/*}` Returns information about the reservation group. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/list` | `GET /v1/{parent=projects/*/locations/*}/reservationGroups` Lists all the reservation groups for the project in the specified location. |

## REST Resource: [v1.projects.locations.reservations](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/create` | `POST /v1/{parent=projects/*/locations/*}/reservations` Creates a new reservation resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/delete` | `DELETE /v1/{name=projects/*/locations/*/reservations/*}` Deletes a reservation. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/failoverReservation` | `POST /v1/{name=projects/*/locations/*/reservations/*}:failoverReservation` Fail over a reservation to the secondary location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/get` | `GET /v1/{name=projects/*/locations/*/reservations/*}` Returns information about the reservation. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/getIamPolicy` | `GET /v1/{resource=projects/*/locations/*/reservations/*}:getIamPolicy` Gets the access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list` | `GET /v1/{parent=projects/*/locations/*}/reservations` Lists all the reservations for the project in the specified location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/patch` | `PATCH /v1/{reservation.name=projects/*/locations/*/reservations/*}` Updates an existing reservation resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/setIamPolicy` | `POST /v1/{resource=projects/*/locations/*/reservations/*}:setIamPolicy` Sets an access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/testIamPermissions` | `POST /v1/{resource=projects/*/locations/*/reservations/*}:testIamPermissions` Gets your permissions on a resource. |

## REST Resource: [v1.projects.locations.reservations.assignments](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/create` | `POST /v1/{parent=projects/*/locations/*/reservations/*}/assignments` Creates an assignment object which allows the given project to submit jobs of a certain type using slots from the specified reservation. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete` | `DELETE /v1/{name=projects/*/locations/*/reservations/*/assignments/*}` Deletes a assignment. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/getIamPolicy` | `GET /v1/{resource=projects/*/locations/*/reservations/*/assignments/*}:getIamPolicy` Gets the access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/list` | `GET /v1/{parent=projects/*/locations/*/reservations/*}/assignments` Lists assignments. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/move` | `POST /v1/{name=projects/*/locations/*/reservations/*/assignments/*}:move` Moves an assignment under a new reservation. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/patch` | `PATCH /v1/{assignment.name=projects/*/locations/*/reservations/*/assignments/*}` Updates an existing assignment. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/setIamPolicy` | `POST /v1/{resource=projects/*/locations/*/reservations/*/assignments/*}:setIamPolicy` Sets an access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/testIamPermissions` | `POST /v1/{resource=projects/*/locations/*/reservations/*/assignments/*}:testIamPermissions` Gets your permissions on a resource. |