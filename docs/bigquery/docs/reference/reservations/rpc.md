# BigQuery Reservation API

A service to modify your BigQuery reservations.

## Service: bigqueryreservation.googleapis.com

The Service name `bigqueryreservation.googleapis.com` is needed to create RPC client stubs.

## `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService`

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.CreateAssignment` `` | Creates an assignment object which allows the given project to submit jobs of a certain type using slots from the specified reservation. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.CreateCapacityCommitment` `` | Creates a new capacity commitment resource. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.CreateReservation` `` | Creates a new reservation resource. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.CreateReservationGroup` `` | Creates a new reservation group. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.DeleteAssignment` `` | Deletes a assignment. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.DeleteCapacityCommitment` `` | Deletes a capacity commitment. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.DeleteReservation` `` | Deletes a reservation. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.DeleteReservationGroup` `` | Deletes a reservation. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.FailoverReservation` `` | Fail over a reservation to the secondary location. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.GetBiReservation` `` | Retrieves a BI reservation. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.GetCapacityCommitment` `` | Returns information about the capacity commitment. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.GetIamPolicy` `` | Gets the access control policy for a resource. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.GetReservation` `` | Returns information about the reservation. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.GetReservationGroup` `` | Returns information about the reservation group. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.ListAssignments` `` | Lists assignments. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.ListCapacityCommitments` `` | Lists all the capacity commitments for the admin project. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.ListReservationGroups` `` | Lists all the reservation groups for the project in the specified location. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.ListReservations` `` | Lists all the reservations for the project in the specified location. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.MergeCapacityCommitments` `` | Merges capacity commitments of the same plan into a single commitment. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.MoveAssignment` `` | Moves an assignment under a new reservation. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.SearchAllAssignments` `` | Looks up assignments for a specified resource for a particular region. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.SearchAssignments` (deprecated) `` | Deprecated: Looks up assignments for a specified resource for a particular region. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.SetIamPolicy` `` | Sets an access control policy for a resource. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.SplitCapacityCommitment` `` | Splits capacity commitment to two commitments of the same plan and `commitment_end_time`. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.TestIamPermissions` `` | Gets your permissions on a resource. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.UpdateAssignment` `` | Updates an existing assignment. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.UpdateBiReservation` `` | Updates a BI reservation. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.UpdateCapacityCommitment` `` | Updates an existing capacity commitment. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1#google.cloud.bigquery.reservation.v1.ReservationService.UpdateReservation` `` | Updates an existing reservation resource. |