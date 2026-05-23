# Get started with reservations

Learn how to create and assign a reservation in BigQuery.

BigQuery reservations let you purchase dedicated processing
capacity, measured in slots, instead of paying *on-demand* pricing per each byte
of data processed. With reservations, costs are more predictable and workload
performance is often more consistent. Reservations are associated with editions
that provide scaled pricing and meet the requirements of different
organizations.

When working with reservations, you can create assignments, which link specific
Google Cloud projects, folders, or your entire organization to a particular
reservation. This lets you isolate workloads, ensure resources for critical
tasks, and manage your BigQuery spending more effectively.

In this tutorial, you create a Standard edition reservation with 100
autoscaling slots and assign a project to the reservation. You can then choose
to delete the reservation to avoid incurring costs.

> [!CAUTION]
> **Caution:** This tutorial incurs charges. Before purchasing slots, understand [reservation
> pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing). To avoid incurring charges after you complete this tutorial, make sure to delete the reservation as described in [Clean up](https://docs.cloud.google.com/bigquery/docs/reservations-get-started#clean-up).

*** ** * ** ***

To follow step-by-step guidance for this task directly in the
Google Cloud console, click **Guide me**:

[Guide me](https://console.cloud.google.com/freetrial?redirectPath=/?walkthrough_id%3Dbigquery__reservations-get-started)

*** ** * ** ***

## Before you begin

1. In the Google Cloud console, on the project selector page,
   select or create a Google Cloud project.

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
   You can create a separate Google Cloud project to administer the reservation and give it a descriptive name like `bq-COMPANY_NAME-admin`.
2.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

3.


   Enable the BigQuery Reservation API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigqueryreservation.googleapis.com)

   For more information, see [Enable the BigQuery Reservation API](https://docs.cloud.google.com/bigquery/docs/reservations-commitments#enabling-reservations-api).
4. In the Google Cloud console, view your slot quotas:

   [View your slot quotas](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots)

   To purchase slots, you must have enough slot quota for the region in which
   you want to purchase slots.

   If your slot quota for the region is less than the number of slots
   you want to purchase, see [Request a quota increase](https://docs.cloud.google.com/bigquery/quotas#requesting_a_quota_increase).

### Required roles


To get the permissions that
you need to create a reservation, assign a project to a reservation, and delete the reservation,

ask your administrator to grant you the
[BigQuery Resource Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.resourceEditor) (`roles/bigquery.resourceEditor`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create an autoscaling reservation

Create a reservation named `test` in the `US` multi-region, and allocate a
maximum of 100 autoscaling slots to it. Autoscaling slots scale up or down based
on your workload demands.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click **Create reservation**.

4. In the **Reservation name** field, enter `test`.

5. In the **Location** drop-down list, select **us (multiple regions in United States)**.

6. In the **Edition** list, select **Standard** . For more information, see
   [Understand BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

7. For **Max reservation size selector** , select **Small (100 Slots)**.

8. Leave the other default settings as they are, and click **Save**.

To learn how to create a reservation using SQL or the bq tool,
see [Create a reservation with dedicated slots](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_a_reservation_with_dedicated_slots).

## Assign a project to a reservation

Assign a project to the `test` reservation. Any query jobs that run from this
project will use the pool of slots from the `test` reservation. (In this
tutorial, you don't run a job.)

You can assign any project that's in the same organization and region as the
administration project where you created the reservation.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot reservations** tab.

4. In the **Actions** column for the reservation named **`test`** , click

   **Actions**.

   ![Assignments project picker.](https://docs.cloud.google.com/static/bigquery/images/reservations-assignments.png)
5. Click **Create assignment**.

6. In the **Select an organization, folder or project** section, click **Browse**.

7. Browse or search for the project, and then select it.

8. Click **Create**.

When you create a reservation assignment, wait at least 5 minutes before running
a query. Otherwise the query might be billed using on-demand pricing.

To learn how to assign a project to a reservation using SQL or the bq tool,
see [Assign a project or folder to a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign_my_prod_project_to_prod_reservation).

## Clean up


To avoid incurring charges to your Google Cloud account for
the resources used on this page, follow these steps.

### Delete the project


The easiest way to eliminate billing is to delete the project that you
created for the tutorial.

To delete the project:

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

<br />

<br />

### Delete the reservation

When you delete a reservation, any jobs that are currently executing with slots
from that reservation will fail. To prevent errors, allow in-flight jobs to
complete before deleting the reservation.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click the **Slot reservations** tab.

4. For the reservation named **`test`** , click **Toggle node**.

5. For each assignment in that reservation, click **Actions** , and then click
   **Delete**.

6. In the **Actions** column for the reservation named **`test`** , click
   **Actions**.

7. Click **Delete**.

To learn how to delete a reservation using SQL or the bq tool, see
[Delete a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#delete_a_reservation).

## What's next

- To learn how to use BigQuery reservations to manage your
  workloads, see [Understand
  reservations](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management).

- To learn more about slots, see
  [Understand slots](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management).

- To learn how to use BigQuery assignments to organize your
  workloads, see [Manage workload
  assignments](https://docs.cloud.google.com/bigquery/docs/reservations-assignments).

- To learn how to purchase a commitment, see
  [Slot
  commitments](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#slot_commitments).