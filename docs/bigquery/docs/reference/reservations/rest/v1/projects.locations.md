# REST Resource: projects.locations.reservationGroups

- [Resource: ReservationGroup](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups#ReservationGroup)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups#ReservationGroup.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups#METHODS_SUMMARY)

## Resource: ReservationGroup

A reservation group is a container for reservations.

| JSON representation |
|---|
| ``` { "name": string } ``` |

| Fields ||
|---|---|
| `name` | `string` Identifier. The resource name of the reservation group, e.g., `projects/*/locations/*/reservationGroups/team1-prod`. The reservationGroupId must only contain lower case alphanumeric characters or dashes. It must start with a letter and must not end with a dash. Its maximum length is 64 characters. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/create` | Creates a new reservation group. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/delete` | Deletes a reservation. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/get` | Returns information about the reservation group. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservationGroups/list` | Lists all the reservation groups for the project in the specified location. |