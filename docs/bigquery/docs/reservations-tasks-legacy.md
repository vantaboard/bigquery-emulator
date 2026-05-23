# Work with legacy slot reservations

> [!NOTE]
> **Note:** Legacy reservations, including access to flat-rate billing or certain commitment lengths, are only available to allow-listed customers. To determine if you have access to these legacy features, contact your administrator. The flat-rate billing model defines how you are billed for compute, but flat-rate reservations and commitments function as Enterprise edition slots.

The BigQuery Reservation API lets you purchase dedicated slots (called
[*commitments*](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments)), create pools
of slots (called [*reservations*](https://docs.cloud.google.com/bigquery/docs/reservations-intro#reservations)),
and assign projects, folders, and organizations to those reservations.

Reservations allow you to assign a dedicated number of slots
to a workload. For example, you might not want
a production workload to compete with test workloads for slots. You could
create a reservation named `prod` and assign your production workloads to this
reservation. For more information, see
[Reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro#reservations).

## Create reservations

### Required permissions

To create a reservation, you need the following Identity and Access Management (IAM)
permission:

- `bigquery.reservations.create` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that maintains ownership of the commitments.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Create a reservation with dedicated slots

Select one of the following options:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click **Create reservation**.

4. In the **Reservation name** field, enter a name for the reservation.

5. In the **Location** drop-down list, select the location.

6. In the **Capacity Model** section, select the capacity model.

7. If you select the **Flat-Rate** option, under **Baseline slots**, enter
   the number of slots for the reservation.

   1. In the **Max reservation size selector** list, select the maximum reservation size.
   2. Optional: In the **Baseline slots** field, enter the number of baseline
      slots for the reservation. To use only the specified slot capacity,
      click the **Ignore idle slots** toggle.

      The amount of autoscaling slots available to you is determined by
      subtracting the baseline slots value from the maximum reservation
      size value. For example, if you create a reservation with 100 baseline
      slots and a max reservation size of 400, your reservation has 300
      autoscaling slots. For more information about baseline slots, see
      [Using reservations with baseline and autoscaling
      slots](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro#using_reservations_with_baseline_and_autoscaling_slots).
8. To disable [idle slot sharing](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots), click the **Ignore idle slots** toggle.

9. The breakdown of slots is displayed in the **Cost estimate** table.
   A summary of the reservation is displayed in the **Capacity summary** table.

10. Click **Save**.

The new reservation is visible in the **Reservations** tab.

### SQL

To create a reservation, use the
[`CREATE RESERVATION` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_reservation_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE RESERVATION
     `ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME`
   OPTIONS (
     slot_capacity = NUMBER_OF_BASELINE_SLOTS,
   );
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation. If you select a [BigQuery Omni
     location](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations), your edition option is limited to the Enterprise edition.
   - `RESERVATION_NAME`: the name of the
     reservation

     It must start and end with a lowercase letter or a number
     and contain only lowercase letters, numbers, and dashes.
   - `NUMBER_OF_BASELINE_SLOTS`: the number of baseline slots to allocate to the reservation. You cannot set the `slot_capacity` option and the `edition` option in the same reservation.
   - `EDITION`: the edition of the reservation. Assigning a reservation to an edition comes with feature and pricing changes. For more information, see [Introduction to BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).
   - `NUMBER_OF_AUTOSCALING_SLOTS`: the number of autoscaling slots assigned to the reservation. This is equal to the value of the max reservation size minus the number of baseline slots.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To create a reservation, use the `bq mk` command with the `--reservation`
flag:

```
bq mk \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation \
    --slots=NUMBER_OF_BASELINE_SLOTS \
    --ignore_idle_slots=false \
    RESERVATION_NAME
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation. If you select a [BigQuery Omni
  location](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations), your edition option is limited to the Enterprise edition.
- `NUMBER_OF_BASELINE_SLOTS`: the number of baseline slots to
  allocate to the reservation

- `RESERVATION_NAME`: the name of the reservation

- `EDITION`: the edition of the reservation. Assigning a reservation to an edition comes with feature and pricing changes. For more information, see [Introduction to BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

- `NUMBER_OF_AUTOSCALING_SLOTS`: the number of autoscaling slots assigned to the reservation. This is equal to the value of the max reservation size minus the number of baseline slots.

For information about the `--ignore_idle_slots` flag, see
[Idle slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots). The default
value is `false`.

### Python


Install the [google-cloud-bigquery-reservation package](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest) before using this code sample. Construct a [ReservationServiceClient](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient). Describe the reservation you'd like to create with a [Reservation](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.types.Reservation). Create the reservation with the [create_reservation](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient_create_reservation) method.

    # TODO(developer): Set project_id to the project ID containing the
    # reservation.
    project_id = "your-project-id"

    # TODO(developer): Set location to the location of the reservation.
    # See: https://cloud.google.com/bigquery/docs/locations for a list of
    # available locations.
    location = "US"

    # TODO(developer): Set reservation_id to a unique ID of the reservation.
    reservation_id = "sample-reservation"

    # TODO(developer): Set slot_capicity to the number of slots in the
    # reservation.
    slot_capacity = 100

    # TODO(developer): Choose a transport to use. Either 'grpc' or 'rest'
    transport = "grpc"

    # ...

    from google.cloud.bigquery_reservation_v1.services import reservation_service
    from google.cloud.bigquery_reservation_v1.types import (
        reservation as reservation_types,
    )

    reservation_client = reservation_service.ReservationServiceClient(
        transport=transport
    )

    parent = reservation_client.common_location_path(project_id, location)

    reservation = reservation_types.Reservation(slot_capacity=slot_capacity)
    reservation = reservation_client.create_reservation(
        parent=parent,
        reservation=reservation,
        reservation_id=reservation_id,
    )

    print(f"Created reservation: {reservation.name}")

<br />

## Update reservations

You can make the following updates to a reservation:

- Change the reservation size by adding or removing slots.
- Configure whether queries in this reservation use idle slots.
- Change the amount of baseline or autoscaling slots allocated to a reservation.

### Required permissions

To update a reservation, you need the following Identity and Access Management (IAM)
permission:

- `bigquery.reservations.update` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that maintains ownership of the commitments.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Change the size of a reservation

You can add or remove slots from an existing reservation.

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Reservations** tab.

4. Find the reservation you want to update.

5. Expand the

   **Actions** option.

6. Click **Edit**.

7. In the **Max reservation size selector** dialog, enter the max reservation size.

8. In the **Baseline slots** field, enter the number of baseline slots.

9. Click **Save**.

### SQL

To change the size of a reservation, use the
[`ALTER RESERVATION SET OPTIONS` data definition language (DDL) statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_reservation_set_options_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER RESERVATION
     `ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME`
   SET OPTIONS (
     slot_capacity = NUMBER_OF_BASELINE_SLOTS,
   );
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation, for example `europe-west9`.
   - `RESERVATION_NAME`: the name of the
     reservation. It must start and end with a lowercase letter or a number
     and contain only lowercase letters, numbers, and dashes.

   - `NUMBER_OF_BASELINE_SLOTS`: the number of baseline slots to allocate to the reservation.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To update the size of a reservation, use the `bq update` command with the
`--reservation` flag:

```
bq update \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --slots=NUMBER_OF_BASELINE_SLOTS \
    --reservation RESERVATION_NAME
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
- `NUMBER_OF_BASELINE_SLOTS`: the number of baseline slots to allocate to the reservation
- `RESERVATION_NAME`: the name of the reservation

### Python


Install the [google-cloud-bigquery-reservation package](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest) before using this code sample. Construct a [ReservationServiceClient](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient). Describe the updated properties with a [Reservation](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.types.Reservation) and the [FieldMask.paths](https://googleapis.dev/python/protobuf/latest/google/protobuf/field_mask_pb2.html#google.protobuf.field_mask_pb2.FieldMask.paths) property. Update the reservation with the [update_reservation](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient_update_reservation) method.

    # TODO(developer): Set project_id to the project ID containing the
    # reservation.
    project_id = "your-project-id"

    # TODO(developer): Set location to the location of the reservation.
    # See: https://cloud.google.com/bigquery/docs/locations for a list of
    # available locations.
    location = "US"

    # TODO(developer): Set reservation_id to a unique ID of the reservation.
    reservation_id = "sample-reservation"

    # TODO(developer): Set slot_capicity to the new number of slots in the
    # reservation.
    slot_capacity = 50

    # TODO(developer): Choose a transport to use. Either 'grpc' or 'rest'
    transport = "grpc"

    # ...

    from google.cloud.bigquery_reservation_v1.services import reservation_service
    from google.cloud.bigquery_reservation_v1.types import (
        reservation as reservation_types,
    )
    from google.protobuf import field_mask_pb2

    reservation_client = reservation_service.ReservationServiceClient(
        transport=transport
    )

    reservation_name = reservation_client.reservation_path(
        project_id, location, reservation_id
    )
    reservation = reservation_types.Reservation(
        name=reservation_name,
        slot_capacity=slot_capacity,
    )
    field_mask = field_mask_pb2.FieldMask(paths=["slot_capacity"])
    reservation = reservation_client.update_reservation(
        reservation=reservation, update_mask=field_mask
    )

    print(f"Updated reservation: {reservation.name}")
    print(f"\tslot_capacity: {reservation.slot_capacity}")

<br />

### Configure whether queries use idle slots

The `--ignore_idle_slots` flag controls whether queries running in a reservation
can use idle slots from other reservations. For more information, see
[Idle slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots). You can update this
configuration on an existing reservation.

To update a reservation, use the `bq update` command with the `--reservation`
flag . The following example sets `--ignore_idle_slots` to `true`,
meaning the reservation will only use slots allocated to the reservation.

```
bq update \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --ignore_idle_slots=true \
    --reservation RESERVATION_NAME
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
- `RESERVATION_NAME`: the name of the reservation

### List the idle slot configuration

To list the [idle slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots) setting
for a reservation, do the following:

### SQL

Query the `ignore_idle_slots` column of the
[`INFORMATION_SCHEMA.RESERVATIONS_BY_PROJECT` view](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations#schema).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     reservation_name,
     ignore_idle_slots
   FROM
     `ADMIN_PROJECT_ID.region-LOCATION`.INFORMATION_SCHEMA.RESERVATIONS_BY_PROJECT;
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resources
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservations

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq ls` command with the `--reservation` flag:

```
bq ls --reservation \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resources
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservations

The `ignoreIdleSlots` field contains the configuration setting.

## Delete reservations

When you delete a reservation, any jobs that are currently executing with slots
from that reservation will fail. To prevent errors, allow running jobs to
complete before deleting the reservation.

### Required permissions

To delete a reservation, you need the following Identity and Access Management (IAM)
permission:

- `bigquery.reservations.delete` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that maintains ownership of the commitments.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Delete a reservation

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Reservations** tab.

4. Find the reservation you want to delete.

5. Expand the

   **Actions** option.

6. Click **Delete**.

7. In the **Delete reservation** dialog, click **Delete**.

### SQL

To delete a reservation, use the
[`DROP RESERVATION` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_reservation_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   DROP RESERVATION
     `ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME`;
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
   - `RESERVATION_NAME`: the ID of the reservation

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To delete a reservation, use the `bq rm` command with the `--reservation`
flag:

```
bq rm \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation RESERVATION_NAME
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
- `RESERVATION_NAME`: the name of the reservation

### Python


Install the [google-cloud-bigquery-reservation package](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest) before using this code sample. Construct a [ReservationServiceClient](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient). Delete the reservation with the [delete_reservation](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient_delete_reservation) method.

    # TODO(developer): Set project_id to the project ID containing the
    # reservation.
    project_id = "your-project-id"

    # TODO(developer): Set location to the location of the reservation.
    # See: https://cloud.google.com/bigquery/docs/locations for a list of
    # available locations.
    location = "US"

    # TODO(developer): Set reservation_id to a unique ID of the reservation.
    reservation_id = "sample-reservation"

    # TODO(developer): Choose a transport to use. Either 'grpc' or 'rest'
    transport = "grpc"

    # ...

    from google.cloud.bigquery_reservation_v1.services import reservation_service

    reservation_client = reservation_service.ReservationServiceClient(
        transport=transport
    )
    reservation_name = reservation_client.reservation_path(
        project_id, location, reservation_id
    )
    reservation_client.delete_reservation(name=reservation_name)

    print(f"Deleted reservation: {reservation_name}")

<br />

## Add BigQuery Reservation API to VPC Service Controls

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

The BigQuery Reservation API supports [VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls).
To integrate the BigQuery Reservation API with VPC Service Controls, follow the
instructions in [Create a service perimeter](https://docs.cloud.google.com/vpc-service-controls/docs/create-service-perimeters),
and add the BigQuery Reservation API to the protected services.

A service perimeter protects access to reservations, commitments, and
assignments within administration projects that are specified in the perimeter.
When creating an assignment, the VPC Service Controls protects the
administration project and assignee project, folder, and organization.