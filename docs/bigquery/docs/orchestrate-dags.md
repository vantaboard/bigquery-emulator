# Schedule Airflow DAGs

This document describes how to schedule
[Airflow directed acyclic graphs (DAGs)](https://docs.cloud.google.com/composer/docs/composer-3/composer-overview#about-airflow)
from
[Managed Airflow 3](https://docs.cloud.google.com/composer/docs/composer-3/composer-overview) on the
**Scheduling** page in BigQuery, including how to trigger DAGs
manually, and how to view the history and logs of past DAG runs.

## About managing Airflow DAGs in BigQuery

The **Scheduling** page in BigQuery provides tools to
schedule Airflow DAGs that run in your Managed Airflow 3 environments.

Airflow DAGs that you schedule in BigQuery are executed in
one or more Managed Airflow environments in your project. The
**Scheduling** page in BigQuery combines information for
all Airflow DAGs in your project.

During a DAG run, Airflow schedules and executes individual tasks that make up
a DAG in a sequence defined by the DAG. On the **Scheduling** page in
BigQuery, you can view statuses of past DAG runs, explore
detailed logs of all DAG runs and all tasks from these DAG runs, and view
details about DAGs.

> [!NOTE]
> **Note:** You can't manage Managed Airflow environments in BigQuery. To manage environments, for example, to create an environment, install dependencies for your DAG files, upload, delete, or change individual DAGs, you use Managed Airflow.

To learn more about Airflow's core concepts such as Airflow DAGs, DAG runs,
tasks, or operators, see the
[Core Concepts](https://airflow.apache.org/docs/apache-airflow/stable/core-concepts/index.html)
page in the Airflow documentation.

To learn more about Managed Airflow environments, see the
[Managed Airflow 3 overview](https://docs.cloud.google.com/composer/docs/composer-3/composer-overview) page
in the Managed Airflow documentation.

## Before you begin

1.


   Enable the Cloud Composer API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=composer.googleapis.com)
2. Make sure that your Google Cloud project has at least one Managed Airflow 3 environment, with at least one already uploaded DAG file:
   - To get started with Airflow DAGs, follow the instructions in the [Run an
     Apache Airflow DAG in Managed Airflow 3](https://docs.cloud.google.com/composer/docs/composer-3/run-apache-airflow-dag) guide. As a part of this guide, you create a Managed Airflow 3 environment with the default configuration, upload a DAG to it, and check that Airflow runs it.
   - For detailed instructions to upload an Airflow DAG to a Managed Airflow 3 environment, see [Add and update
     DAGs](https://docs.cloud.google.com/composer/docs/composer-3/manage-dags).
   - For detailed instructions to create a Managed Airflow 3 environment, see [Create
     Managed Airflow environments](https://docs.cloud.google.com/composer/docs/composer-3/create-environments).

### Required permissions


To get the permissions that
you need to schedule Airflow DAGs,

ask your administrator to grant you the
following IAM roles on the project:

- To view Airflow DAGs and their details: [Environment and Storage Object Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/composer#composer.environmentAndStorageObjectViewer) (`roles/composer.environmentAndStorageObjectViewer`)
- To trigger and pause Airflow DAGs: [Environment and Storage Object User](https://docs.cloud.google.com/iam/docs/roles-permissions/composer#composer.environmentAndStorageObjectUser) (`roles/composer.environmentAndStorageObjectUser`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to schedule Airflow DAGs. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to schedule Airflow DAGs:

- To view Airflow DAGs and their details: `composers.dags.list, composer.environments.list`
- To trigger and pause Airflow DAGs: `composers.dags.list, composer.environments.list, composer.dags.execute`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

<br />

For more information about Managed Airflow 3 IAM, see
[Access control with IAM](https://docs.cloud.google.com/composer/docs/composer-3/access-control)
in Managed Airflow documentation.

## Manually trigger an Airflow DAG

When you manually trigger an Airflow DAG, Airflow runs
the DAG once, independently from the schedule specified for the DAG.

To manually trigger a selected Airflow DAG, follow these steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to the **Scheduling** page](https://console.cloud.google.com/bigquery/orchestration)
2. Do either of the following:

   - Click the name of the selected DAG, and then
     on the **DAG details** page, click **Trigger DAG**.

   - In the row that contains the selected DAG,
     click
     **View actions** in the **Actions** column, and then click
     **Trigger DAG**.

## View Airflow DAG run logs and details

To view details of a selected Airflow DAG, follow these steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to the **Scheduling** page](https://console.cloud.google.com/bigquery/orchestration)
2. Click the name of the selected DAG.

3. On the **DAG details** page, select the **Details** tab.

4. To view past DAG runs, select the **Runs** tab.

   1. Optional: The **Runs** tab displays DAG runs from the last 10 days by
      default. To filter DAG runs by a different time range, in
      the **10 days** drop-down menu, select a time range, and then click
      **OK**.

   2. Optional: To display additional columns with DAG run details in the list
      of all DAG runs, click
      **Column display options** , and then select columns and click **OK**.

   3. To view details and logs for a selected DAG run, select a DAG run.

5. To view a visualization of the DAG with task dependencies,
   select the **Diagram** tab.

   1. To view task details, select a task on the diagram.
6. To view the source code of the DAG, select the **Code** tab.

7. Optional: To refresh the displayed data, click **Refresh**.

## View all Airflow DAGs

To view Airflow DAGs from all Managed Airflow 3 environments in your
Google Cloud project, follow these steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to the **Scheduling** page](https://console.cloud.google.com/bigquery/orchestration)
2. Optional: To display additional columns with DAG details,
   click
   **Column display options** ,
   and then select columns and click **OK**.

## Pause an Airflow DAG

To pause a selected Airflow DAG, follow these steps:

1. In the Google Cloud console, go to the **Scheduling** page.

   [Go to the **Scheduling** page](https://console.cloud.google.com/bigquery/orchestration)
2. Do either of the following:

   - Click the name of the selected DAG, and then
     on the **DAG details** page, click **Pause DAG**.

   - In the row that contains the selected DAG,
     click
     **View actions** in the **Actions** column, and then click **Pause DAG**.

## Troubleshooting

For instructions to troubleshoot Airflow DAGs, see
[Troubleshooting Airflow DAGs](https://docs.cloud.google.com/composer/docs/composer-3/troubleshooting-dags)
in Managed Airflow documentation.

## What's next

- Learn more about [writing Airflow DAGs](https://docs.cloud.google.com/composer/docs/composer-3/write-dags).
- Learn more about [Airflow in Managed Airflow 3](https://docs.cloud.google.com/composer/docs/composer-3/composer-overview#about-airflow).