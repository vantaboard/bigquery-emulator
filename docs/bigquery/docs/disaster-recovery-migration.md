# Migrate to managed disaster recovery

This page describes how to migrate from BigQuery cross-region
replication to BigQuery managed disaster recovery.

## Overview

BigQuery [cross-region
replication](https://docs.cloud.google.com/bigquery/docs/data-replication) (CRR) and [managed disaster
recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery) (DR) are both features designed to
enhance data availability and disaster recovery capabilities. However, they
handle regional outages in different ways. CRR doesn't allow promoting the
secondary replica if the primary region is unavailable. In contrast, DR offers
more comprehensive protection by allowing a failover to the secondary replica
even if the primary region is unavailable. With CRR, only storage is replicated,
while with DR both storage and compute capacity are replicated.

The following table describes the feature capabilities of CRR and DR:

| Feature | CRR | DR |
|---|---|---|
| Initial replication process | Uses CRR to replicate the dataset initially. | The initial load is previously replicated with CRR before migrating a CRR dataset to a DR dataset. |
| Promotion replication | Uses standard replication. | Uses [Turbo replication](https://docs.cloud.google.com/storage/docs/availability-durability#turbo-replication). |
| Promotion process | Promotes at the dataset level. | Promotes at the reservation level (reservation failover and dataset promotion). Many datasets can be attached to one failover reservation. Dataset level promotion is unavailable with DR. |
| Promotion execution | Through UI or SQL based DDL command for each dataset. There is no support for CLI, Client Libraries, API, or Terraform. | Through UI or SQL based DDL command for each EPE reservation. There is no support for CLI, client libraries, API, or Terraform. |
| Failover mode | Soft failover. | Hard failover. |
| Edition requirement | Any capacity model. | Enterprise Plus edition. |
| Limitations | [CRR limitations](https://docs.cloud.google.com/bigquery/docs/data-replication#limitations). | Includes both [CRR limitations](https://docs.cloud.google.com/bigquery/docs/data-replication#limitations) and [DR limitations](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery#limitations). |
| Write access | Jobs running under any capacity model can write to replicated datasets in the primary region. Secondary is always read-only. | Only the jobs running under Enterprise Plus reservations can write to replicated datasets in the primary region. Secondary dataset and reservation replicas are always read-only. |
| Read access | Jobs running under any capacity model can read from replicated datasets. | Jobs running under any capacity model can read from replicated datasets. |

## Migration implications

The following sections provide an overview of the cost and capability changes
that occur when you migrate to DR.

### Cost implications

Consider the following cost implications when you migrate from CRR to DR:

- DR only supports write access from the Enterprise Plus edition, which
  incurs higher compute costs. You can read from any capacity model, so read costs
  for existing jobs don't change.

- DR uses [Turbo
  replication](https://docs.cloud.google.com/storage/docs/availability-durability#turbo-replication), which
  incurs additional costs depending on the region pair.

- Storage prices are the same for both CRR and DR.

For more information about pricing, see [Pricing](https://cloud.google.com/bigquery/pricing).

### Capability implications

Consider the following capability implications when you migrate from CRR to DR:

- DR only supports failover at the reservation level. Any existing jobs that
  rely on dataset-level failover fail.

- Only Enterprise Plus edition queries can write to the dataset once it
  is attached to the DR reservation. Any existing write jobs that don't use an
  Enterprise Plus edition for their compute capacity fail.

## Before you begin

Before you begin the migration, familiarize yourself with the concepts in
[cross-region replication](https://docs.cloud.google.com/bigquery/docs/data-replication) and [managed
disaster recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery).

To migrate to DR, you must have the following prerequisites:

- You have an active Google Cloud project with BigQuery enabled.

- You have created and replicated datasets with CRR.

- The datasets have the same primary and secondary locations you want to use for
  DR.

- You have the necessary permissions to work with DR. For more information about
  permissions, see [Before you
  begin](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery#before_you_begin).

## Migrate from CRR to DR

The following sections describe how to migrate your datasets from CRR to DR. It
assumes you have already configured your datasets for CRR.

### Create a failover reservation

To enable disaster recovery, you must create a failover reservation in the
primary region. Configure your reservation with the appropriate primary and
secondary region. The primary and secondary regions should match the regions of
all the CRR datasets you intend to migrate to DR. To create a failover
reservation, choose one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management** , and
   then click **Create reservation**.

3. In the **Reservation name** field, enter a name for the reservation.

4. In the **Location** list, select the location.

5. In the **Edition** list, select the Enterprise Plus edition.

6. In the **Max reservation size selector** list, select the maximum reservation size.

7. Optional: In the **Baseline slots** field, enter the number of baseline
   slots for the reservation.

   The number of available autoscaling slots is determined by
   subtracting the **Baseline slots** value from the **Max reservation
   size** value. For example, if you create a reservation with 100 baseline
   slots and a max reservation size of 400, your reservation has 300
   autoscaling slots. For more information about baseline slots, see
   [Using reservations with baseline and autoscaling
   slots](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro#using_reservations_with_baseline_and_autoscaling_slots).
8. In the **Secondary location** list, select the secondary location.

9. To disable [idle slot sharing](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots)
   and use only the specified slot capacity, click the **Ignore idle slots**
   toggle.

10. To expand the **Advanced settings** section, click the
    expander arrow.

11. Optional: To set the target job concurrency, click the **Override
    automatic target job concurrency** toggle to on, and then enter a value for **Target
    Job Concurrency** .
    The breakdown of slots is displayed in the **Cost estimate** table. A
    summary of the reservation is displayed in the **Capacity summary**
    table.

12. Click **Save**.

The new reservation is visible in the **Slot reservations** tab.

### SQL

To create a reservation, use the
[`CREATE RESERVATION` data definition language (DDL) statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_reservation_statement).

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
     edition = ENTERPRISE_PLUS,
     secondary_location = SECONDARY_LOCATION);
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource.
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation. If you select a [BigQuery Omni
     location](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations), your edition option is limited to the Enterprise edition.
   - `RESERVATION_NAME`: the name of the
     reservation.

     The name must start and end with a lowercase letter or a number
     and contain only lowercase letters, numbers, and dashes.
   - `NUMBER_OF_BASELINE_SLOTS`: the number of baseline slots to allocate to the reservation. You cannot set the `slot_capacity` option and the `edition` option in the same reservation.
   - `SECONDARY_LOCATION`: the secondary [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation. In the case of an outage, any datasets attached to this reservation will fail over to this location.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### Attach the dataset to the reservation

Once you've created the failover reservation, attach your cross-region dataset, or datasets, to the reservation. This enables failover for all of the attached datasets. To attach the dataset to the reservation, choose one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management** , and then
   click the **Slot Reservations** tab.

3. Click the reservation that you want to attach a dataset to.

4. Click the **Disaster recovery** tab.

5. Click **Add failover dataset**.

6. Enter the name of the dataset you want to associate with the reservation.

7. Click **Add**.

### SQL

To attach a dataset to a reservation, use the
[`ALTER SCHEMA SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_set_options_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER SCHEMA
     `DATASET_NAME`
   SET OPTIONS (
     failover_reservation = ADMIN_PROJECT_ID.RESERVATION_NAME);
   ```


   Replace the following:
   - `DATASET_NAME`: the name of the
     dataset.

   - `ADMIN_PROJECT_ID.RESERVATION_NAME`: the name of the reservation you want to associate the dataset to.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### Verify the configuration

To verify the status of your configuration, query the
[`INFORMATION_SCHEMA.SCHEMATA_REPLICAS`
view](https://docs.cloud.google.com/bigquery/docs/information-schema-schemata-replicas).

```
PROJECT_ID.`region-REGION`.INFORMATION_SCHEMA.SCHEMATA_REPLICAS[_BY_PROJECT]
```

Verify that the datasets are attached to the correct reservation in the correct
regions.
Replace the following:

- Optional: `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.
- `REGION`: any [dataset region name](https://docs.cloud.google.com/bigquery/docs/locations). For example, `` `region-us` ``.

  <br />

  <br />

  > [!NOTE]
  > **Note:** You must use [a region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier) to query `INFORMATION_SCHEMA` views. The location of the query execution must match the region of the `INFORMATION_SCHEMA` view.

<br />

## Examples

The following example walks you through the steps to migrate from CRR to DR with
practical examples using GoogleSQL. For this example, assume the
following:

- You are working in a project named `myproject`.

- You have already created a dataset named `mydataset` and configured it with
  CRR.

- The primary region of `mydataset` is `us-central1` and the secondary region is
  `us-west1`.

To begin migrating your dataset to DR, first create a reservation with the
Enterprise Plus edition. In this example, the name of the reservation
is `myreservation`.

    CREATE RESERVATION `myproject.region-us-central1.myreservation`
    OPTIONS (
      slot_capacity = 0,
      edition = ENTERPRISE_PLUS,
      autoscale_max_slots = 50,
      secondary_location = 'us-west-1');

Once the reservation is created, you can then attach the dataset to the
reservation. The following example attaches the dataset to the reservation:

    ALTER SCHEMA
      `myproject.mydataset`
    SET OPTIONS (
      failover_reservation = 'myproject.myreservation');

Then, verify that the dataset has been successfully attached.

    SELECT
      failover_reservation_project_id,failover_reservation_name,
    FROM
     `myproject`.`region-us-west1`.INFORMATION_SCHEMA.SCHEMATA_REPLICAS
    WHERE
     schema_name='mydataset';

The results of this query should look similar to the following:

```
+---+---+
| failover_reservation_project_id | failover_reservation_name |
+---+---+
| myproject                       | myreservation             |
| myproject                       | myreservation             |
+---+---+
```

## What's next?

- For more information about cross-region replication, see [Cross-region dataset
  replication](https://docs.cloud.google.com/bigquery/docs/data-replication).

- For more information about managed disaster recovery, see [Managed disaster
  recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery).