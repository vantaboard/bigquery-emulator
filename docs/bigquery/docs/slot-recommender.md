# View edition slot recommendations

The BigQuery slot recommender creates recommendations for
[edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) or on-demand workloads. The recommender
analyzes historical slot usage for query jobs and calculates the cost-optimal
settings for edition commitment slots and
[autoscaling](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) slots while maintaining
similar performance. The slot recommender can also recommend a maximum
reservation size that can improve performance.

You can use the slot recommender for both reservations billing and on-demand
billing:

- For reservations billing, you can get a cost-optimized recommendation for Enterprise or Enterprise Plus edition workloads and performance-based recommendations for your reservations.
- For on-demand billing, you can get cost-optimized recommendations for on-demand workloads across the organization, for a specific project, or a group of projects if you were to convert one or more projects to the Enterprise edition.

For more information about the recommender service, see the
[Recommender overview](https://docs.cloud.google.com/recommender/docs/overview).

## Cost-optimized recommendations

The slot recommender estimates your autoscaling usage based on slot usage over
the past 30 days. For more information about slots autoscaling, see
[Introduction to slots autoscaling](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro).
The recommender can generate multiple commitment options and calculate the
total cost for each option. The recommender can also recommend options with the
lowest total cost by using custom pricing. The recommended commitment and
autoscale slots are meant to cover P99 slot usage of the entire 30-day
observation window.

The slot recommender offers recommendations for different pricing types,
including pay as you go (no commitments), 1-year and 3-year commitments. It
displays the monthly cost for each option based on custom pricing.

![Slot usage chart in the
Google Cloud console.](https://docs.cloud.google.com/static/bigquery/images/slot-recommender-usage-chart.png)

![Slot recommendations in the
Google Cloud console.](https://docs.cloud.google.com/static/bigquery/images/slot-recommender.png)

The recommendation includes the following details:

- Baseline commitment slots: The number of commitment slots to achieve optimal cost without affecting performance. You can also view the optimal commitments in the usage chart above by selecting **View optimal
  commitments**.
- Baseline commitment monthly cost: The monthly cost of the optimal commitment slots, calculated using the custom edition commitment price. A month is defined as 730 hours.
- Autoscale slots: The maximum number of autoscale slots used at a time. This represents the additional slots beyond the optimal commitment slots that are covered by autoscaling. This value does not include the commitment or baseline slots.
- Expected autoscale utilization: The expected monthly utilization of autoscale slots, calculated as the expected autoscale slots used divided by the maximum autoscale slots.
- Autoscale monthly cost: The monthly cost of using the expected amount of autoscale slots, calculated using the custom autoscale price.
- Total monthly cost: The total monthly cost, which includes the commitment monthly cost and the autoscale monthly cost.

### Best practices when applying recommendations

> [!NOTE]
> **Note:** The recommendations provided are based on historical data, and the actual results may vary depending on the specific characteristics of your workloads.

1. Ensure that the sum of baseline slots for all reservations under the edition is equal or less than the commitment slots. This ensures that any slot usage exceeding the commitment slots can be covered by autoscale slots. If the baseline slots exceed the commitment slots, you are billed for additional baseline slots.
2. Autoscale slots in the settings is chosen so that available capacity matches the peak historical usage. This is to ensure the performance is not impacted. You can also adjust the autoscale slots to a value lower than the maximum, which can increase autoscale utilization. However, note that when your slot usage cannot be fully covered, it may affect query performance.
3. In circumstances where your workload experiences spikes and temporarily exceeds its maximum capacity, the slot recommender mechanism can overestimate its recommendations. In such situations, you may consider maintaining the current settings, assuming that you are satisfied with the current level of performance.

You might see the message `Slot Estimator doesn't have any recommendations
that would be more effective than your current settings` even if your slot usage
sometimes exceeds your configured maximum. This can occur because BigQuery
occasionally overprovisions slots temporarily to boost query speed, at no extra
cost. The slot recommender aims to maintain your recent performance, including
these bursts. If this P99 usage is higher than your current maximum, any
recommendation at or below your current setting wouldn't meet those performance
levels, so no change is suggested.

### Required permissions

To view cost-optimal commitment slots recommendations, you need the following
Identity and Access Management (IAM) permissions:

- `recommender.bigqueryCapacityCommitmentsRecommendations.get`
- `recommender.bigqueryCapacityCommitmentsRecommendations.list`

Each of the following predefined IAM roles includes these
permissions:

- `BigQuery Resource Admin`
- `BigQuery Slot Recommender Viewer`
- `BigQuery Slot Recommender Admin`

To view recommendations for your edition workloads, you must have the listed
permission for the administration project.

To view project level recommendations for your on-demand workloads, you must
have the previously listed permissions at the project level.

To view cost-optimized recommendations for on-demand workloads for a group of
projects, you must have the previously listed permissions at the organization
level and either `bigquery.jobs.listExecutionMetadata` or
`bigquery.jobs.listAll` permission.

To view organization level recommendations for your on-demand workloads, you
must have the previously listed permissions at the organization level. You also
need the `resourcemanager.organizations.get` permission. The `Organization
Viewer` IAM role includes this permission.

In the recommendations settings, rows such as
**Baseline commitment slots** and **Total monthly cost** are visible, but
the values of the monthly cost details are hidden. To view the hidden values,
you also need the following permission:

- `billing.accounts.getPricing`

Each of the following predefined IAM roles includes these
permissions:

- `Billing Account Viewer`
- `Billing Account Administrator`

For edition workloads, you need the previously listed permissions at the billing
account associated with the administrator project. For project-level on-demand
workloads, you need the permissions at the billing account associated with the
project or at the organization level for organization-level recommendations.

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Performance-improving recommendation

When you select a specific reservation workload, the slot recommender suggests
the maximum reservation size that can enhance job performance. The slot
estimator then analyzes the
[slot modeling](https://docs.cloud.google.com/bigquery/docs/slot-estimator#model_slot_performance) data and
finds the minimum incremental value for the maximum reservation size that is
capable of elevating job performance by at least 5%. If your current maximum
reservation size meets your historical needs, then no recommendation is made.

![Slot recommender reservation recommendation in the
Google Cloud console.](https://docs.cloud.google.com/static/bigquery/images/slot-recommender-reservation-recommendation.png)

> [!NOTE]
> **Note:** Recommendations are based on historical data. Job performance can vary based on actual use.

To implement a recommendation, click **Apply** to be redirected to the page
where you can update the reservation.

![Slot recommender reservation recommendation edit reservation in the
Google Cloud console.](https://docs.cloud.google.com/static/bigquery/images/slot-recommender-edit-reservation.png)

## Before you begin

Before you can view the recommendations, you must [enable the Recommender
API](https://docs.cloud.google.com/recommender/docs/enabling). To view the recommendations within the
Google Cloud console, you must also [enable the Reservations
API](https://docs.cloud.google.com/bigquery/docs/reservations-commitments#enabling-reservations-api).

### Required permissions

The slot recommender for reservations performance improvement recommendations
requires that you have the following IAM permissions on the
administration project:

- `bigquery.reservations.list`
- `bigquery.reservationAssignments.list`
- `bigquery.capacityCommitments.list`

To apply the recommended updates to the reservation, you must also have the
following IAM permissions on the administration project:

- `bigquery.reservations.update`

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Pricing

This recommender is displayed within the context of [slot estimator](https://docs.cloud.google.com/bigquery/docs/slot-estimator). You can use the recommendations at no charge.

## View slot recommendations

To view slot recommendations using the Google Cloud console, perform the following
steps.

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. For edition workloads recommendations, select the administrator project. For
   on-demand workloads recommendations, select any project within your
   organization that meets the predefined requirements.

3. For on-demand workloads, if you have organization level permissions, you can
   select any individual project or the entire organization in the side panel
   options to view recommendations for the specific scope.

4. In the navigation menu, click **Capacity management**.

5. Click the **Slot estimator** tab.

6. In the **Source** pane, select an on-demand workload or an
   edition (Enterprise or Enterprise Plus) workload.

   - If you select an edition workload, detailed recommendations appear under the graph of historical usage.
   - If you select an on-demand workload, the organization administrator is able to switch between organization level and project level (for one or more projects).