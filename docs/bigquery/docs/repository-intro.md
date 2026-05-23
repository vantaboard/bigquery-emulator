# Introduction to repositories

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or ask questions that are related to this Preview feature, contact [bigquery-repositories-feedback@google.com](mailto:%20bigquery-repositories-feedback@google.com).

This document helps you understand the concept of repositories in
BigQuery. You can use repositories to perform version control on
files you use in BigQuery. BigQuery uses Git to record
changes and manage file versions.

Each BigQuery repository represents a Git repository. You can
use BigQuery's built-in Git capabilities, or you can connect to
a third-party Git repository. Within each repository, you can create one or more
[workspaces](https://docs.cloud.google.com/bigquery/docs/workspaces-intro) to edit the code stored in the
repository.

To view repositories, on the BigQuery page, in the left pane,
click **Explorer** ,
and then click **Repositories**. Your repositories are displayed in
alphabetical order in a new tab in the details pane.

> [!IMPORTANT]
> **Important:** If you create an asset in a BigQuery repository---for example, a query, notebook (including a notebook with an Apache Spark job), BigQuery pipeline, or Dataform workflow---you cannot schedule it for execution in BigQuery repository. For scheduling and executing Dataform workflows, you need to use Dataform repositories. For scheduling queries and notebooks, use BigQuery Studio. For more information, see [Scheduling queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries), [Schedule notebooks](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks), and [Schedule pipelines](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines).

## Third-party repositories

You can connect a BigQuery repository to
a third-party Git repository if you choose. In this case, the third-party repository stores
the repository code instead of BigQuery. BigQuery interacts with the third-party
repository to allow you to edit and execute its contents in a
BigQuery workspace. Depending on the type of repository you
choose, you can connect to a third-party repository by using SSH or HTTPS.

The following table lists supported Git providers and the connection
methods that are available for their repositories:

| Git provider | Connection method |
|---|---|
| Microsoft Azure DevOps Services | SSH |
| Bitbucket | SSH |
| GitHub | SSH or HTTPS |
| GitLab | SSH or HTTPS |

For more information, see
[Connect to a third-party repository](https://docs.cloud.google.com/bigquery/docs/repositories#connect-third-party).

## Service account

All BigQuery repositories are connected to the default
Dataform service agent. This service account is derived from your
project number in the following format:

    service-YOUR_PROJECT_NUMBER@gcp-sa-dataform.iam.gserviceaccount.com

[Strict act-as mode](https://docs.cloud.google.com/dataform/docs/strict-act-as-mode)
is enforced and requires all repositories to use a custom service account or
user credentials for a Google Account to schedule pipelines and notebooks.

## Locations

You can create repositories in all
[BigQuery Studio locations](https://docs.cloud.google.com/bigquery/docs/locations#bqstudio-loc).

## Quotas

[Dataform quotas](https://docs.cloud.google.com/dataform/docs/quotas#quotas) apply to use of
BigQuery repositories.

## Pricing

You are not charged for creating, updating, or deleting a repository.

For more information on BigQuery pricing, see [Pricing](https://cloud.google.com/bigquery/pricing).

## What's next

- Learn how to [create repositories](https://docs.cloud.google.com/bigquery/docs/repositories).
- Learn how to [create workspaces](https://docs.cloud.google.com/bigquery/docs/workspaces).