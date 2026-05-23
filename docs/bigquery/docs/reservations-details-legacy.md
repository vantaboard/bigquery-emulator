# Legacy capacity commitment plans

> [!NOTE]
> **Note:** Legacy reservations, including access to flat-rate billing or certain commitment lengths, are only available to allow-listed customers. To determine if you have access to these legacy features, contact your administrator. The flat-rate billing model defines how you are billed for compute, but flat-rate reservations and commitments function as Enterprise edition slots.

BigQuery offers the following capacity commitment plans:

- [Flex slots commitment](https://docs.cloud.google.com/bigquery/docs/reservations-details-legacy#flex_slots)
- [Monthly commitment](https://docs.cloud.google.com/bigquery/docs/reservations-details-legacy#monthly-commitments)
- [Annual commitment](https://docs.cloud.google.com/bigquery/docs/reservations-details-legacy#annual-commitments)

The minimum commitment size is 100 slots, and commitments are available in
100-slot increments, up to your [slot quota](https://docs.cloud.google.com/bigquery/docs/slots#slot_quotas_and_limits).
There is no limit on the
number of commitments that you can create. You are charged from the moment your
commitment purchase is successful. For information
about BigQuery costs, see [BigQuery pricing](https://cloud.google.com/bigquery/pricing).

> [!NOTE]
> **Note:** Customers who use on-demand billing typically have 2000 slots or more available for query processing. Allocating 100 slots might reduce the query performance.

To learn more about capacity commitments and reservations, see [Introduction to
reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro).

Purchasing a capacity commitment is optional when purchasing slots associated
with [BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro), but can
save on costs.

### Flex slots commitment plans

Flex slots are a way to purchase BigQuery capacity for
**60 seconds** . When you purchase a flex slots commitment, you can delete the
commitment 60 seconds after the commitment becomes active. This service bills by
the second. You can convert a flex slots commitment plan to a [monthly](https://docs.cloud.google.com/bigquery/docs/reservations-details-legacy#monthly-commitments)
or an [annual](https://docs.cloud.google.com/bigquery/docs/reservations-details-legacy#annual-commitments) commitment plan.

For example, if you purchase a commitment at 6:00 PM, you start being charged at
that second. You can delete the commitment after 6:01 PM, 60 seconds after the
commitment begins; this results in 60 seconds of billed usage. You are
billed for each second the commitment is active.

## Monthly commitment plans

> [!NOTE]
> **Note:** Support for monthly commitments ended in August, 2023, when monthly commitments were converted to either annual commitments or baseline slots. For more information about commitments and renewal, see [Slot
> commitments](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments).

## Annual commitment plans

With an annual commitment plan, you pay for the specified number
of slots for one year, and for each second thereafter. After the
expiration of the plan, you can delete the commitment at any time, or continue
to pay based on the conditions of the renewal plan.

A renewal plan is the capacity commitment plan that takes effect after an annual
commitment expires.

### Renew annual commitments

You select a renewal plan when you purchase an annual commitment
plan, or when you convert another commitment plan to an annual commitment. You
can change the renewal plan for an annual commitment at any time until it expires.

If you are on a flat-rate capacity model, you can change the renewal plan
before the plan expires to one of the following options:

- **None.** After 365 days, your plan expires and doesn't renew. It is removed.
- **Monthly.** After 365 days, your commitment converts to a monthly commitment. You are charged at the monthly rate, and you can delete the commitment after 30 days.
- **Flex.** After 365 days, your commitment converts to a flex slots commitment. You are charged at the flex slots rate, and you can delete the commitment at any time.
- **Annual.** After 365 days, your commitment renews for another year.

> [!NOTE]
> **Note:** Starting on July 5, 2023, BigQuery customers will no longer be able to purchase flat-rate annual, flat-rate monthly, and flex slots commitments. For more information, see [BigQuery pricing](https://cloud.google.com/bigquery/pricing#flat-rate_compute_analysis_pricing).

For information about purchasing and renewing commitments if you are not on a flat-rate capacity model, see [Create a capacity commitment](https://docs.cloud.google.com/bigquery/docs/reservations-commitments#create_a_capacity_commitment).

For example, suppose you purchased an annual commitment at 6:00 PM on October 5,
2019. You start being charged at that second. Expiration or renewal of the
commitment happens after 6:00 PM on October 4, 2020, noting that 2020 is a leap year.

- If you choose to renew to a monthly commitment, then at 6:00 PM on October
  4, 2020, your commitment converts into a monthly commitment. You are
  charged at the monthly commitment rate, and you cannot delete the commitment for
  30 days after the monthly renewal plan.

- If you choose to renew to a flex slots commitment, then at 6:00 PM on October
  4, 2020, your commitment converts into a flex slots commitment. You are
  charged at the flex slots rate and you can delete the commitment after 60 seconds.

- If you choose to renew annually, then at 6:00 PM on October 4,
  2020, your commitment renews for another year.

### Expired commitments

If you have flat-rate commitments, they are removed when they expire unless a
renewal plan is specified. To make sure you don't lose any capacity, the extra
slots are moved to the baseline of a system-created reservation called
`system-created-Enterprise`. After the commitments expire, your bill
consists of three parts:

1. Remaining commitments.
2. Baseline slots that are not covered by the remaining commitments.
3. Scaled slots managed by autoscaling.

The following scenarios describe what happens when the commitments expire:

#### Scenario 1: Commitments are equal to total baseline

You have one commitment that expires with 100 slots, and one
reservation with 100 baseline slots.

The 100 slots are removed and you are charged based on the 100 baseline.

#### Scenario 2: Commitments larger than total baseline

You have one commitment that expires with 200 slots, and one
reservation with 100 baseline slots.

The 200 slots are removed and `system-created-Enterprise` is created
with 100 baseline. You are charged based on the total 200 baseline.

#### Scenario 3: Commitments with Annual flat-rate renewal plan

You have one annual flat-rate commitment that expires with
100 slots and annual flat-rate renewal plan.

The 100 slots are moved into an Enterprise annual commitment with
annual renewal plan.

## Delete commitments

After you create a commitment, you can delete it only after the commitment
expires.

To delete an annual commitment, set its renewal plan to flex slots. After the
annual commitment expires and is renewed as a flex slots commitment, you can
delete it.

For instructions on how to delete a commitment, see
[Commitment expiration](https://docs.cloud.google.com/bigquery/docs/reservations-commitments#commitment_expiration).

If you accidentally purchased a commitment or made a mistake when you configured
your commitment, contact
[Cloud Billing Support](https://docs.cloud.google.com/support/billing) for help.

## What's next

- Learn about [reservations, its limitations, quotas, and pricing](https://docs.cloud.google.com/bigquery/docs/reservations-intro).
- Learn about [slots](https://docs.cloud.google.com/bigquery/docs/slots).
- Learn how to [purchase and manage slot capacity](https://docs.cloud.google.com/bigquery/docs/reservations-commitments).
- Learn about [BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).