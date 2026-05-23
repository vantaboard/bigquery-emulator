# Committed use discounts

> [!IMPORTANT]
> **Important:** This page describes committed use discounts for BigQuery PAYG capacity. For information about other Google Cloud CUDs, see [Committed use discounts](https://docs.cloud.google.com/billing/docs/resources/multiprice-cuds).

This page describes how spend-based committed use discounts (CUDs) work with
BigQuery.

BigQuery supports two different types of commitments:

- [Capacity
  commitments](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing), which are a
  commitment to purchase a specific number of slots with an edition in an
  administration project.

- Spend-based commitments, which are described on this
  page.

## BigQuery spend-based CUDs

Spend-based CUDs for BigQuery provide discounted prices in
exchange for your commitment to use BigQuery capacity in a particular
region.

- **10% discount**: You get this by committing to a 1-year term. For the duration of your term, you pay the BigQuery CUD 1-year price (consumption model ID DD83-D9A3-79AF) as your committed hourly spend amount.
- **20% discount**: You get this by committing to a 3-year term. For the duration of your term, you pay the BigQuery CUD 3-year price (consumption model ID 4D8D-49A7-C5B1) as your committed hourly spend amount.

BigQuery spend-based CUDs are ideal for workloads with predictable
spend across hours. You commit to a consistent amount of spend, measured in cost
per hour, for a one- or three-year term. In exchange, you receive a discounted
rate for BigQuery SKUs on the applicable usage your
commitment covers.

You can purchase spend-based CUDs from any Cloud Billing
account, and the applicable discount applies to any eligible usage in projects
that the Cloud Billing account pays for. When you purchase a
BigQuery spend-based CUD, you pay the same commitment fee for the
entirety of the commitment term, even if the price of applicable usage changes.
The commitment fee is billed monthly. Any overage in usage is charged at the
PAYG rate.

> [!NOTE]
> **Note** : This document discusses some spend-based committed use discounts (CUDs) that automatically migrated to a new consumption model, which leverages discounts rather than credits. The migration date is indicated by a notification in the Google Cloud console Billing Overview page. For more information about the improvements, the affected CUDs, and any required actions on your part, see [Spend-based CUDs](https://docs.cloud.google.com/docs/cuds-multiprice).

When deciding to purchase spend-based CUDs, keep the following in mind:

- Regions: You purchase spend-based CUDs for individual regions. If you're running in multiple regions, calculate and purchase spend-based CUDs in each separate region.
- Projects: Determine the consistent hourly PAYG slot usage per project.
- BigQuery capabilities: Spend-based CUDs work across all BigQuery PAYG SKUs.

## BigQuery spend-based CUDs usage types

Spend-based CUDs automatically apply to aggregate BigQuery instance usage in a
region, which gives you low, predictable costs without needing to make any
manual changes or updates. This flexibility helps you to achieve high
utilization rates across your commitments, saving you time and money.
BigQuery spend-based CUDs apply to all BigQuery PAYG usage.
BigQuery spend-based CUDs apply to usage from all supported compute capacity
types, which include the following:

- BigQuery editions
- Composer 3 (also referred to as BigQuery engine for Apache Airflow)
- Knowledge Catalog
- BigQuery services
- Managed Service for Apache Spark (previously known as *Dataproc Serverless*)

For a complete list of applicable SKUs, see [BigQuery CUD Eligible SKUs](https://docs.cloud.google.com/skus/sku-groups/bigquery-cud-eligible-skus).

## Purchase BigQuery spend-based CUDs

After your purchase a CUD, you can't cancel your commitment. Make sure the size and duration of your commitment aligns with both your historical and your expected minimum expenditure on BigQuery capacity. For more information, see [Canceling commitments](https://docs.cloud.google.com/docs/cuds-spend-based#canceling_commitments).

To purchase or manage BigQuery committed use discounts for your
Cloud Billing account, follow the instructions at
[Purchasing spend-based
commitments](https://docs.cloud.google.com/docs/cuds-spend-based#purchasing).

## Calculate the spend-based CUD discount

The following example describes how to calculate the cost of using
BigQuery with a spend-based CUD.

Key points to keep in mind:

- BigQuery spend-based CUDs apply only to the PAYG capacity of the features previously listed.
- BigQuery spend-based CUDs don't apply to storage or on-demand.
- BigQuery spend-based CUDs apply to all PAYG slot SKUs in a given region.
- BigQuery spend-based CUDs are measured in dollars per hourly discounted commitment.

When you calculate the hourly rate for BigQuery PAYG in the region
where you want to purchase a CUD, first consider whether it saves money for
your situation. Any usage greater than the commitment is charged at the regular
PAYG price.

For example, assume you are running a workload that uses Enterprise
edition slot-hours in the `us-central1` region for BigQuery.

Use the [BigQuery pricing](https://cloud.google.com/bigquery/pricing) page to determine the per hour cost for a three-year commitment, for example $0.036 / slot hour (20% discount). That amount is your commitment price, which you commit to spend per hour on BigQuery for three years in `us-central1` across all projects. Any overage in usage is charged at the PAYG rate.
In the legacy CUDs program before automatic account migration, your commitment amount was the on-demand price instead. For more information, see [Spend-based CUDs](https://docs.cloud.google.com/docs/cuds-multiprice).

You are charged a minimum of $0.036 / slot hour for the whole year,
independent from actual usage. When you make the commitment, you're charged that amount even if you decide to stop or reduce the hourly usage for the duration of the commitment.

## Pricing

BigQuery provides a 10% discount for one year and 20% discount for a three year spend-based commitment. Contact your sales representative for pricing information.

For more information about BigQuery spend-based CUDs pricing, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing). The discounts are the
same in all regions and multi-regions.

## What's next

- Learn how to purchase spend-based commitments in [Committed use
  discounts](https://docs.cloud.google.com/docs/cuds-spend-based).