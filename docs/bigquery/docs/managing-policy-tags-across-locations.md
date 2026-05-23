# Managing policy tags across locations

This document describes how to manage policy tags across regional locations for
column-level security and dynamic data masking in BigQuery.

BigQuery provides [fine-grained access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro)
and [dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro) for
sensitive table columns through policy tags, supporting type-based
classification of data.

After creating a data-classification taxonomy and applying policy tags to your
data, you can further manage the policy tags across locations.

## Location considerations

Taxonomies are regional resources, like BigQuery datasets and
tables. When you create a taxonomy, you specify the region, or *location*, for
the taxonomy.

You can create a taxonomy and apply policy tags to tables in
[all regions where BigQuery is available](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations).
However, to apply policy tags from a taxonomy to a table column, the taxonomy and the table must exist in the same regional location.

Although you cannot apply a policy tag to a table column that exists in a
different location, you can copy the taxonomy to another location by explicitly
replicating it there.

## Using taxonomies across locations

You can explicitly copy (or replicate) a
taxonomy and its policy tag definitions to additional locations without having
to manually create a new taxonomy in each location. When you replicate
taxonomies, you can use the same policy tags for column-level security in
multiple locations, simplifying their management.

When you replicate them, the taxonomy and policy tags retain the same IDs
in each location.

> [!CAUTION]
> **Caution:** Syncing a taxonomy across locations syncs only the taxonomy and policy tags in the taxonomy. The permissions granted to a policy tag and the columns governed by that policy tag are not synced, and can vary by location.

The taxonomy and policy tags can be synced again, to keep them
unified across multiple locations. Explicit replication of a taxonomy is
accomplished by a call to the
[Data Catalog](https://docs.cloud.google.com/data-catalog/docs) API. Future syncs of the replicated
taxonomy use the same API command, which overwrites the previous taxonomy.

To facilitate syncing of the taxonomy, you can use
[Cloud Scheduler](https://docs.cloud.google.com/scheduler/docs) to periodically perform a sync of the
taxonomy across regions, either on a set schedule or with a manual button push.
To use Cloud Scheduler, you need to set up a service account.

## Replicating a taxonomy in a new location

### Required permissions

The user credentials or service account replicating the taxonomy must have the
Data Catalog Policy Tag Admin role.

Read more about granting the Policy Tag Admin role in
[Restricting access with BigQuery column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security).

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

To replicate a taxonomy across locations:

### API

Call the
[`projects.locations.taxonomies.import`](https://docs.cloud.google.com/data-catalog/docs/reference/rest/v1/projects.locations.taxonomies/import)
method of the Data Catalog API, supplying a `POST` request and
the name of the destination project and location in the HTTP string.

`POST https://datacatalog.googleapis.com/{parent}/taxonomies:import`

The `parent` path parameter is the destination project and location
that you want to copy the taxonomy to. Example:
`projects/MyProject/locations/eu`

> [!IMPORTANT]
> **Key Point:** To sync the replicated taxonomy across locations in the future, repeat the same steps.

## Syncing a replicated taxonomy

To sync a taxonomy that's already been replicated across locations, repeat the
Data Catalog API call as described in
[Replicating a taxonomy in a new location](https://docs.cloud.google.com/bigquery/docs/managing-policy-tags-across-locations#replicating_a_taxonomy_in_a_new_location).

Alternatively, you can use a service account and Cloud Scheduler to sync the
taxonomy on a specified schedule. Setting up a service account in
Cloud Scheduler also lets you trigger an on-demand (unscheduled) sync
through the Cloud Scheduler page in the Google Cloud console or with
Google Cloud CLI.

> [!CAUTION]
> **Note:** Once a taxonomy is replicated in other locations, changes made in one location are not automatically reflected in other locations. The taxonomy must be explicitly synced again, by a user or a service account with the appropriate permissions.

## Syncing a replicated taxonomy with Cloud Scheduler

To sync a replicated taxonomy across locations with Cloud Scheduler, you
need a service account.

### Service accounts

You can grant permissions for the replication sync to an existing service
account, or you can create a new service account.

To create a new service account, see
[Create service accounts](https://docs.cloud.google.com/iam/docs/service-accounts-create).

### Required permissions

1. The service account that is syncing the taxonomy must have the
   Data Catalog Policy Tag Admin role. For more information, see
   [The Data Catalog Policy Tag Admin role](https://docs.cloud.google.com/bigquery/docs/column-level-security#policy_tags_admin).

2. [Enable the Cloud Scheduler API](https://console.cloud.google.com/apis/library/cloudscheduler.googleapis.com)

### Setting up a taxonomy sync with Cloud Scheduler

To sync a replicated taxonomy across locations with Cloud Scheduler:

### Console

First, create the sync job and its schedule.

1. Follow the instructions to
   [Create a job in Cloud Scheduler](https://docs.cloud.google.com/scheduler/docs/schedule-run-cron-job#create_a_job).

2. For **Target** , see the instructions at
   [Creating a scheduler job with authentication](https://docs.cloud.google.com/scheduler/docs/http-target-auth#creating_a_scheduler_job_with_authentication).

Next, add the authentication needed for the scheduled sync.

1. Click **SHOW MORE** to display the authentication fields.

2. For **Auth header**, select "Add OAuth token".

3. Add your service account's information.

4. For the **Scope**, enter "https://www.googleapis.com/auth/cloud-platform".

5. Click **Create** to save the scheduled sync.

   ![Create a scheduler job part 2](https://docs.cloud.google.com/static/bigquery/images/create-scheduler-job2.png)

Now, test that the job is configured correctly.

1. After the job is created, click **Run now** to test the job
   is configured correctly. Subsequently, the Cloud Scheduler
   triggers the HTTP request based on the schedule that you specified.

   ![Test a scheduler job](https://docs.cloud.google.com/static/bigquery/images/test-scheduler-job.png)

> [!IMPORTANT]
> **Key Point:** For future on-demand (unscheduled) syncing of the replicated taxonomy, click **Run now** in the [Cloud Scheduler](https://console.cloud.google.com/cloudscheduler) page in the Google Cloud console.

### gcloud

Syntax:

<br />

```bash
  gcloud scheduler jobs create http "JOB_ID" --schedule="FREQUENCY" --uri="URI" --oath-service-account-email="CLIENT_SERVICE_ACCOUNT_EMAIL" --time-zone="TIME_ZONE" --message-body-from-file="MESSAGE_BODY"
  
```

<br />

Replace the following:

1. `${JOB_ID}` is a name for the job. It must be unique in the project. Note that you cannot re-use a job name in a project even if you delete its associated job.
2. `${FREQUENCY}` is the schedule, also called *job interval* , of how often the job should run. For example, "every 3 hours". The string that you supply here can be any crontab-compatible string. Alternatively, developers familiar with legacy App Engine cron can use [App Engine Cron](https://docs.cloud.google.com/appengine/docs/standard/python/config/cronref) syntax.
3. `${URI}` is the fully qualified URL of the endpoint.
4. `--oauth-service-account-email` defines the token type. Note that Google APIs hosted on `*.googleapis.com` expect an OAuth token.
5. `${CLIENT_SERVICE_ACCOUNT_EMAIL}` is the email of the client service account.
6. `${MESSAGE_BODY}` is the path to the file that contains the POST request body.

Other option parameters are available, which are described in the
[Google Cloud CLI reference](https://docs.cloud.google.com/sdk/gcloud/reference/scheduler/jobs/create/http).

Example:

<br />

```bash
  gcloud scheduler jobs create http cross_regional_copy_to_eu_scheduler --schedule="0 0 1 * *" --uri="https://datacatalog.googleapis.com/v1/projects/my-project/locations/eu/taxonomies:import" --oauth-service-account-email="policytag-manager-service-acou@my-project.iam.gserviceaccount.com" --time-zone="America/Los_Angeles" --message-body-from-file=request_body.json
  
```

<br />

## What's next

- For an overview of column-level security with policy tags, see [Introduction to BigQuery column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro).
- For more information about creating and applying policy tags, see [Restricting access with BigQuery column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security).
- To learn about the impact to writes when you use BigQuery column-level security, see [Impact on writes with BigQuery column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-writes).
- For information about best practices for using policy tags, see [Using policy tags in BigQuery](https://docs.cloud.google.com/bigquery/docs/best-practices-policy-tags).