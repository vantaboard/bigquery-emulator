# Purchase and manage legacy slot commitments

> [!NOTE]
> **Note:** Legacy reservations, including access to flat-rate billing or certain commitment lengths, are only available to allow-listed customers. To determine if you have access to these legacy features, contact your administrator. The flat-rate billing model defines how you are billed for compute, but flat-rate reservations and commitments function as Enterprise edition slots.

The BigQuery Reservation API lets you purchase dedicated slots (called
[*commitments*](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments)), create pools
of slots (called [*reservations*](https://docs.cloud.google.com/bigquery/docs/reservations-intro#reservations)),
and assign projects, folders, and organizations to those reservations.

A *capacity commitment* is a purchase of BigQuery compute capacity for
some minimum duration of time. Purchasing a capacity commitment is optional when
creating a reservation with an [edition](https://docs.cloud.google.com/bigquery/docs/editions-intro), but can
provide cost savings.

Commitments are a regional resource. Commitments purchased in one region or
multi-region cannot be used in any other regions or multi-regions.
Commitments cannot be moved between regions or between regions and
multi-regions.

## Enable the Reservations API

The BigQuery Reservation API is distinct from the existing
BigQuery API and must be enabled independently. For more
information, see
[Enabling and disabling APIs](https://docs.cloud.google.com/apis/docs/getting-started#enabling_apis).

- The name of the API is "BigQuery Reservations API"
- The endpoint for the BigQuery Reservation API is `bigqueryreservation.googleapis.com`.

<br />

![Enable API.](https://docs.cloud.google.com/static/bigquery/images/reservations-enable-api.png)

<br />

> [!NOTE]
> **Note:** If you want to prevent anyone in your organization from enabling the BigQuery Reservation API, [contact support](https://docs.cloud.google.com/support).

## Purchase slots

To reserve capacity for some minimum amount of time, you can purchase a
[capacity commitment](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments).
This provides a discount and saves on costs. For more information about the specific costs, see [BigQuery pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

### Required permissions

To create a capacity commitment, you need the following
Identity and Access Management (IAM) permission:

- `bigquery.capacityCommitments.create` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that maintains ownership of the commitments.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Create a capacity commitment

> [!CAUTION]
> **Caution:** Before creating a capacity commitment, understand the details of the [commitment plans](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments) and [pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

Commitments are a regional resource. Commitments purchased in one region or
multi-region cannot be used in any other regions or multi-regions. Commitments
cannot be moved between regions or between regions and multi-regions.

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click **Create Commitment**.

4. Under **Configure**:

   1. Select the location.
   2. In the **Capacity model** section, select the capacity model.
   3. Select the **Commitment duration**, which specifies your commitment plan.
   4. If you are purchasing an **Annual** commitment, select the **Renewal
      plan that you want to take effect when the commitment expires**:

      1. **Do not renew and convert to a monthly commitment** (Default). When the annual commitment expires, it converts to a monthly commitment.
      2. **Renew annually**. When the annual commitment expires, it renews for another year as an annual commitment.
      3. **Do not renew and convert to flex slots**. When the annual commitment expires, it converts to a flex slots commitment.

      For more information, see [Slot commitments](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments).
   5. Enter the **Number of slots** you want to purchase.

   6. Click **Next**.

5. Review the **Cost** estimate for your purchase.

6. Under **Confirm and submit**:

   1. Type <kbd>CONFIRM</kbd> to confirm the purchase.
   2. Click **Purchase** to purchase the slots.
7. To view the commitment, click **View slot commitments**. After the
   capacity is provisioned, the requested capacity commitment has a green
   status.

   > [!NOTE]
   > **Note:** Slots are usually provisioned quickly, but in rare cases it can take several hours. If you have a critical workload where you expect to have increased demand, reserve your slots at least one day in advance.

The first time you purchase capacity, a `default` reservation is created.

### SQL

To create a capacity commitment, use the
[`CREATE CAPACITY` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_capacity_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE CAPACITY
     `ADMIN_PROJECT_ID.region-LOCATION.COMMITMENT_ID`
   OPTIONS (
     slot_count = NUMBER_OF_SLOTS,
     plan = 'PLAN_TYPE');
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that will maintain ownership of this commitment
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitment
   - `COMMITMENT_ID`: the ID of the commitment

     It must be unique to the project and location. It must start and end
     with a lowercase letter or a number and contain only lowercase
     letters, numbers, and dashes.
   - `NUMBER_OF_SLOTS`: the number of slots to purchase
   - `PLAN_TYPE`: the [plan type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments), such as `FLEX`, `MONTHLY`, or `ANNUAL`.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk)
with the
[`--capacity_commitment` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-capacity-commitment)
to purchase slots.

```
bq mk \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --capacity_commitment=true \
    --plan=PLAN_TYPE \
    --slots=NUMBER_OF_SLOTS
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that will maintain ownership this commitment
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitment
- `PLAN_TYPE`: the [plan type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments), such as `FLEX`, `MONTHLY`, or `ANNUAL`.
- `NUMBER_OF_SLOTS`: the number of slots to purchase.

## View capacity commitments

### Required permissions

To view commitments, you need the following Identity and Access Management (IAM)
permission:

- `bigquery.capacityCommitments.list` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that maintains ownership of the commitments.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`
- `BigQuery Resource Editor`
- `BigQuery Resource Viewer`
- `BigQuery User`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

> [!NOTE]
> **Note:** When you view legacy capacity commitments, the value of the **Capacity
> Model** column is `Flat-Rate`. The value of the **Edition** column is `Enterprise`. The value of the **Edition** column doesn't indicate that you have purchased a commitment associated with an edition.

### View capacity commitments by project

To view your capacity commitments by project:

### Console

It's not possible to view legacy capacity commitments using the Google Cloud console.

### SQL

To view the commitments for an administration project, query the
[`INFORMATION_SCHEMA.CAPACITY_COMMITMENTS_BY_PROJECT` view](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations#schema).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     capacity_commitment_id
   FROM
     `region-LOCATION`.INFORMATION_SCHEMA.CAPACITY_COMMITMENTS_BY_PROJECT
   WHERE
     project_id = 'ADMIN_PROJECT_ID'
     AND slot_count = 100;
   ```


   Replace the following:
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitments
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the commitments

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq ls` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_ls)
with the
[`--capacity_commitment` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#ls-capacity_commitment-flag)
to list the commitments for an administration project.

```
bq ls \
    --capacity_commitment=true \
    --location=LOCATION \
    --project_id=ADMIN_PROJECT_ID
```

Replace the following:

- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitments
- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the commitments

## Update capacity commitments

You can make the following updates to a capacity commitment:

- Renew an existing commitment.
- Convert a commitment to a commitment plan with a longer duration.
- Split a commitment into two commitments.
- Merge two commitments into a single commitment.

### Required permissions

To update capacity commitments, you need the following
Identity and Access Management (IAM) permission:

- `bigquery.capacityCommitments.update` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that maintains ownership of the commitments.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Renew a commitment

Annual commitments have a renewal plan, which you specify when you
create or convert to an annual commitment. You can change your
[annual commitment's renewal plan](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#renew-commitments)
at any time before the commitment end date.

### Console

You can change your renewal plan for an annual commitment as follows:

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot commitments** tab.

4. Find the commitment you want to edit.

5. Click

   **Actions** , and then select the **Edit renewal plan** option.

6. Select the new renewal plan.

### bq

To change the renewal plan choice for an annual commitment, use the
[`bq update` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update)
with the
[`--capacity_commitment` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#update-capacity-commitment-flag)
and the [`--renewal_plan` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#renewal_plan_flag).

```
bq update \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --renewal_plan=PLAN_TYPE \
    --capacity_commitment=true \
    COMMITMENT_ID
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that will maintain ownership this commitment
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitment
- `PLAN_TYPE`: the [plan type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments), such as `FLEX`, `MONTHLY`, or `ANNUAL`.
- `COMMITMENT_ID`: the ID of the commitment

  To get the ID, see [View purchased commitments](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy#view-commitments).

### Convert a commitment to longer duration

You can choose to convert your commitment to a longer-duration commitment type
at any time:

- You can convert your flex slots commitment to a monthly or annual commitment.
- You can convert your monthly commitment to an annual commitment.

As soon as you update your commitment, you are charged the rate
associated with the new plan, and the end date resets.

To convert a commitment, use the
[`bq update` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update)
with the
[`--plan` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#update-plan-flag).

```
bq update \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --plan=PLAN_TYPE \
    --renewal_plan=RENEWAL_PLAN \
    --capacity_commitment=true \
    COMMITMENT_ID
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitment
- `PLAN_TYPE`: the [plan type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments), such as `FLEX`, `MONTHLY`, or `ANNUAL`.
- `RENEWAL_PLAN`: the
  [renewal](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#renew-commitments)
  plan

  This applies only if the `PLAN_TYPE` is
  `ANNUAL`. If the `PLAN_TYPE` is `MONTHLY`, omit this
  flag.
- `COMMITMENT_ID`: the ID of the commitment

  To get the ID, see [View purchased commitments](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy#view-commitments).

### Split a commitment

You can split your commitment into two commitments. This can be useful if you
want to
[renew](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy#renewing-commitments) part of a commitment. For example, if you have an
annual commitment of 1,000 slots, you could split off 300 slots into a new
commitment, leaving 700 slots in the original commitment. You could then renew
700 slots at the
annual rate, and convert 300 slots to flex slots after the end date.

When you split a commitment, the new commitment has the same plan and the same
commitment end date as the original commitment.

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot commitments** tab.

4. Select the commitment that you want to split.

5. Click **Split**.

6. In the **Split commitment** page, use the **Configure split** slider to
   select how many slots go into each split, in increments of 100 slots.

7. Click **Split** to split the commitment. The new commitment is listed in
   the **Slot commitments** tab.

### bq

To split commitments, use the `bq update` command.

```
bq update \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --split \
    --slots=SLOTS_TO_SPLIT \
    --capacity_commitment=true \
    COMMITMENT_ID
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitment
- `SLOTS_TO_SPLIT`: the number of slots to split from the original commitment into a new commitment
- `COMMITMENT_ID`: the ID of the commitment

  To get the ID, see [View purchased commitments](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy#view-commitments).

### Merge two commitments

You can merge multiple commitments into one commitment. The merging commitments
must all be of the same type (`FLEX`, `MONTHLY`, `ANNUAL`, or `THREE_YEAR`). The end
date of the combined commitment is the maximum end date of the original
commitments. If any of the commitments have an earlier end date,
they are extended to the later date and you are charged a prorated amount for
those slots.

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot commitments** tab.

4. Select the commitments that you want to merge.

5. Click **Merge**.

6. In the **Merge commitments** page, review the details of the merge and
   click **Merge** . The new merged commitment is listed in the
   **Slot commitments** tab.

### bq

To merge two commitments into one commitment, use the `bq update` command:

```
bq update \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --merge=true \
    --capacity_commitment=true \
    COMMITMENT1,COMMITMENT2
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitments
- `COMMITMENT1`: the first commitment to merge
- `COMMITMENT2`: the second commitment to merge

## Delete a commitment

You can delete a capacity commitment if its end date is in the past.
The commitment end date is shown in the Google Cloud console.
Before you delete a commitment, make sure there are enough
unallocated slots. If not, you must decrease the number of slots in a
reservation or remove reservations entirely.

### Required permissions

To delete capacity commitments, you need the following
Identity and Access Management (IAM) permission:

- `bigquery.capacityCommitments.delete` on the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that maintains ownership of the commitments.

Each of the following predefined IAM roles includes this
permission:

- `BigQuery Admin`
- `BigQuery Resource Admin`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Deleting a capacity commitment

> [!CAUTION]
> **Caution:** This operation cannot be undone.

### Console

To delete a capacity commitment, complete the following steps:

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot commitments** tab.

4. In the **Location** drop-down list, select the appropriate location.

5. Find the commitment you want to delete.

6. Expand the

   **Actions** option.

7. Click **Delete**.

8. Type "REMOVE" and click **Proceed**.

### SQL

To delete a capacity commitment, use the
[`DROP CAPACITY` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_capacity_statement).

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   DROP CAPACITY
     `ADMIN_PROJECT_ID.region-LOCATION.COMMITMENT_ID`;
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project that owns the commitment
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitment
   - `COMMITMENT_ID`: the ID of the commitment

     To get the ID, see
     [View purchased commitments](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy#view-commitments).

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To delete a capacity commitment, use the `bq rm` command with the
`--capacity_commitment` flag:

```
bq rm \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --capacity_commitment=true \
    COMMITMENT_ID
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the commitment
- `COMMITMENT_ID`: the ID of the commitment you
  want to delete

  To get the ID, see [View purchased commitments](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy#view-commitments).

## Troubleshooting capacity commitments

This section describes troubleshooting steps that you might find helpful if you
run into problems using BigQuery Reservations.

### Purchased slots are pending

Slots are subject to available capacity. When you purchase slot commitments and
BigQuery allocates them, then the **Status** column shows a check
mark. If BigQuery can't allocate the requested slots immediately,
then the **Status** column remains pending. You might have to wait several hours
for the slots to become available. If you need access to slots sooner, try the
following:

1. Delete the pending commitment.
2. Purchase a new commitment for a smaller number of slots. Depending on capacity, the smaller commitment might become active immediately.
3. Purchase the remaining slots as a separate commitment. These slots might show as pending in the **Status** column, but they generally become active within a few hours.
4. Optional: When both commitments are available, you can [merge](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy#merging-commitments) them into a single commitment, as long as you purchased the same plan for both.

If a slot commitment fails or takes a long time to complete, consider using
[on-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing) temporarily. With this
solution, you might need to run critical queries on a different project that's
not assigned to any reservations, or you might need to remove the project
assignment altogether.