# Enable the BigQuery Data Transfer Service

To use the BigQuery Data Transfer Service, you must complete the
following steps as a project
[Owner](https://docs.cloud.google.com/iam/docs/roles-overview#legacy-basic):

- Create a project and enable the BigQuery API.
- Enable the BigQuery Data Transfer Service.

For more information on Identity and Access Management (IAM) roles, see
[Roles and permissions](https://docs.cloud.google.com/iam/docs/roles-overview)
in the IAM documentation.

> [!NOTE]
> **Note:** If you call the BigQuery Data Transfer Service API immediately after you enable BigQuery Data Transfer Service programmatically, you should implement a retry mechanism with backoff delays between consecutive calls. This is necessary because API enablement is asynchronous and subject to propagation delays caused by eventual consistency.

## Create a project and enable the BigQuery API

Before using the BigQuery Data Transfer Service, you must
create a project and, in most cases, enable billing on that project. You can use
an existing project with the BigQuery Data Transfer Service, or
you can create a new one. If you are using an existing project, you may also
need to enable the BigQuery API.

To create a project and enable the BigQuery API:

1. In the Google Cloud console, go to the project selector page.

   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
2. Select or create a Google Cloud project.

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

3. Enable billing on your project for all transfers. You are billed $0 for
   free transfers.

   Enabling billing is only required once per project, even if you are
   transferring data from multiple sources. Billing must also be enabled to
   query the data in BigQuery, after the data is transferred.

   [Learn how to confirm that billing is enabled on your project](https://docs.cloud.google.com/billing/docs/how-to/modify-project).
4. BigQuery is automatically enabled in new projects. To activate BigQuery in an existing project, enable the BigQuery API.   

   [Enable the BigQuery API](https://console.cloud.google.com/apis/library/bigquery.googleapis.com)

## Enable the BigQuery Data Transfer Service

Before you can create a transfer, you must enable the BigQuery Data Transfer Service. To
enable the BigQuery Data Transfer Service, you must be granted the
[Owner](https://docs.cloud.google.com/iam/docs/roles-overview#legacy-basic)
role for your project.

To enable the BigQuery Data Transfer Service:

1. Open the [BigQuery Data Transfer API](https://console.cloud.google.com/apis/library/bigquerydatatransfer.googleapis.com)
   page in the API library.

2. From the drop-down menu, select the appropriate project.

3. Click the ENABLE button.

   [Enable the Data Transfer API](https://console.cloud.google.com/apis/library/bigquerydatatransfer.googleapis.com)

## Service Agent

The BigQuery Data Transfer Service uses a
[service agent](https://docs.cloud.google.com/iam/docs/service-account-types#service-agents) to access and
manage your resources. This includes, but is not limited to, the following
resources:

- Retrieving an access token for the service account to use when authorizing the data transfer.
- Publishing notifications to the provided Pub/Sub topic if enabled.
- Starting BigQuery jobs.
- Retrieving events from the provided Pub/Sub subscription for Cloud Storage event-driven transfer

The service agent is created automatically on your behalf after you enable the
BigQuery Data Transfer Service and use the API for the first time. Upon service agent
creation, Google grants the predefined
[service agent role](https://docs.cloud.google.com/bigquery/docs/access-control#bigquerydatatransfer.serviceAgent)
automatically.

### Cross-project Service Account Authorization

If you authorize the data transfer using a service account from a project that
is different from the project with the BigQuery Data Transfer Service enabled, you must
grant the `roles/iam.serviceAccountTokenCreator` role to the service agent using
the following Google Cloud CLI command:

```bash
gcloud iam service-accounts add-iam-policy-binding service_account \
--member serviceAccount:service-project_number@gcp-sa-bigquerydatatransfer.iam.gserviceaccount.com \
--role roles/iam.serviceAccountTokenCreator
```

Where:

- <var translate="no">service_account</var> is the cross-project service account used for authorizing the data transfer.
- <var translate="no">project_number</var> is the project number of the project where the BigQuery Data Transfer Service is enabled.

For more information about cross-project resource configuration, see
[Configuring for a resource in a different project](https://docs.cloud.google.com/iam/docs/attach-service-accounts#attaching-different-project)
in the Identity and Access Management service account impersonation documentation.

When you enable the BigQuery Data Transfer Service API through the Google Cloud console, Google
automatically attempts to grant the required permissions. However, if you enable
the API or create transfers through Terraform, the Google Cloud CLI, or other
programmatic methods, you must manually establish the required permissions. To
authorize a transfer using a service account from a different project, consider
the following:

- **Establish permissions for cross-project transfers:** To securely access
  cross-project data sources, grant the DTS service agent (resident in the
  destination project) the `roles/iam.serviceAccountTokenCreator` role on the
  source service account identity.

- **Enforce the principle of least privilege:** Grant this role at the
  resource level (on the specific service account being used) rather than the
  project level.

### Manual Service Agent Creation

If you want to trigger service agent creation before you interact with the API,
for example, if you need to grant extra roles to the service agent, you can use
one of the following approaches:

- API: [services.GenerateServiceIdentity](https://docs.cloud.google.com/service-usage/docs/reference/rest/v1beta1/services/generateServiceIdentity)
- gcloud CLI: [gcloud beta services identity create](https://docs.cloud.google.com/sdk/gcloud/reference/beta/services/identity/create)
- Terraform Provider: [google_project_service_identity](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/project_service_identity)

When you manually trigger service agent creation, Google doesn't grant the
predefined
[service agent role](https://docs.cloud.google.com/bigquery/docs/access-control#bigquerydatatransfer.serviceAgent)
automatically. You must manually grant the service agent the predefined role
using the following Google Cloud CLI command:

```bash
gcloud projects add-iam-policy-binding project_number \
--member serviceAccount:service-project_number@gcp-sa-bigquerydatatransfer.iam.gserviceaccount.com \
--role roles/bigquerydatatransfer.serviceAgent
```

Where:

- <var translate="no">project_number</var> is the project number of the project where the BigQuery Data Transfer Service is enabled.

> [!WARNING]
> **Warning:** Don't revoke the service agent role from the service agent. If you revoke the role, the BigQuery Data Transfer Service will no longer work.

## Grant `bigquery.admin` access

We recommend granting the `bigquery.admin` predefined IAM role to
users who create BigQuery Data Transfer Service transfers.
The `bigquery.admin` role includes the IAM permissions needed to
perform the most common tasks. The `bigquery.admin` role includes the
following BigQuery Data Transfer Service permissions:

- BigQuery Data Transfer Service permissions:
  - `bigquery.transfers.update`
  - `bigquery.transfers.get`
- BigQuery permissions:
  - `bigquery.datasets.get`
  - `bigquery.datasets.getIamPolicy`
  - `bigquery.datasets.update`
  - `bigquery.datasets.setIamPolicy`
  - `bigquery.jobs.create`

> [!NOTE]
> **Note:** Starting March 17, 2026, the BigQuery Data Transfer Service will require the `bigquery.datasets.getIamPolicy` and `bigquery.datasets.setIamPolicy` permissions. For more information, see [Changes to dataset-level access
> controls](https://docs.cloud.google.com/bigquery/docs/dataset-access-control).

> [!NOTE]
> **Note:** If the `bigquery.admin` role is too broad for a specific use case, you can [create a custom IAM role](https://docs.cloud.google.com/iam/docs/creating-custom-roles) with only the necessary permissions.

In some cases, the required permissions might differ between different data
sources. Refer to the "Required permissions" section in each data source
transfer guide for specific IAM information. For example, see [Amazon S3 transfer permissions](https://docs.cloud.google.com/bigquery/docs/s3-transfer#required_permissions)
or [Cloud Storage transfer permissions](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer#required_permissions).

To grant the `bigquery.admin` role:

### Console

1. Open the IAM page in the Google Cloud console

   [Open
   the IAM page](https://console.cloud.google.com/iam-admin/iam)

   <br />

2. Click **Select a project**.

3. Select a project and click **Open**.

4. Click **Add** to add new members to the project and set their
   permissions.

5. In the **Add members** dialog:

   - For **Members**, enter the email address of the user or group.
   - In the **Select a role** drop-down, click **BigQuery \> BigQuery
     Admin**.
   - Click **Add**.

     ![Grant admin](https://docs.cloud.google.com/static/bigquery/images/grant-bigquery-admin.png)

### gcloud

You can use the Google Cloud CLI to grant a user or group the
`bigquery.admin` role.

> [!NOTE]
> **Note:** When managing access for users in [external identity providers](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), replace instances of Google Account principal identifiers---like `user:kiran@example.com`, `group:support@example.com`, and `domain:example.com`---with appropriate [Workforce Identity Federation principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers).

To add a single binding to your project's IAM policy, type
the following command. To add a user, supply the `--member` flag in the
format `user:user@example.com`. To add a group, supply the `--member` flag
in the format `group:group@example.com`.

```bash
gcloud projects add-iam-policy-binding project_id \
--member principal:address \
--role roles/bigquery.admin
```

Where:

- <var translate="no">project_id</var> is your project ID.
- <var translate="no">principal</var> is either `group` or `user`.
- <var translate="no">address</var> is the user or group's email address.

For example:

    gcloud projects add-iam-policy-binding myproject \
    --member group:group@example.com \
    --role roles/bigquery.admin

The command outputs the updated policy:

```
    bindings:
    - members:
      - group:group@example.com
        role: roles/bigquery.admin
    
```

<br />

For more information on IAM roles in BigQuery, see
[Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

## What's next

After enabling the BigQuery Data Transfer Service, create a transfer for your data source.

- SaaS platforms:
  - [Salesforce](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer)
  - [Salesforce Marketing Cloud](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer)

  - [ServiceNow](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer)
- Marketing platforms:
  - [Facebook Ads](https://docs.cloud.google.com/bigquery/docs/facebook-ads-transfer)
  - [HubSpot](https://docs.cloud.google.com/bigquery/docs/hubspot-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Klaviyo](https://docs.cloud.google.com/bigquery/docs/klaviyo-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Mailchimp](https://docs.cloud.google.com/bigquery/docs/mailchimp-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
- Payment platforms:
  - [PayPal](https://docs.cloud.google.com/bigquery/docs/paypal-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Stripe](https://docs.cloud.google.com/bigquery/docs/stripe-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Shopify](https://docs.cloud.google.com/bigquery/docs/shopify-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
- Databases and data warehouses:
  - [Amazon Redshift](https://docs.cloud.google.com/bigquery/docs/migration/redshift)
  - [Apache Hive Metastore](https://docs.cloud.google.com/bigquery/docs/hdfs-data-lake-transfer)
  - [Microsoft SQL Server](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [MySQL](https://docs.cloud.google.com/bigquery/docs/mysql-transfer)
  - [Oracle](https://docs.cloud.google.com/bigquery/docs/oracle-transfer)
  - [PostgreSQL](https://docs.cloud.google.com/bigquery/docs/postgresql-transfer)
  - [Snowflake](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Teradata](https://docs.cloud.google.com/bigquery/docs/migration/teradata)
- Cloud storage:
  - [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer)
  - [Amazon Simple Storage Service (Amazon S3)](https://docs.cloud.google.com/bigquery/docs/s3-transfer)
  - [Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer)
- Google Services:
  - [Campaign Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-campaign-transfer)
  - [Comparison Shopping Service (CSS) Center](https://docs.cloud.google.com/bigquery/docs/css-center-transfer-schedule-transfers) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Display \& Video 360](https://docs.cloud.google.com/bigquery/docs/display-video-transfer)
  - [Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
  - [Google Ad Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer)
  - [Google Analytics 4](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer)
  - [Google Merchant Center](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Search Ads 360](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer)
  - [Google Play](https://docs.cloud.google.com/bigquery/docs/play-transfer)
  - [YouTube Channel](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer)
  - [YouTube Content Owner](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer)