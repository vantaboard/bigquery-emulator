# Event-Driven Transfers

You can use the BigQuery Data Transfer Service to create event-driven transfers that automatically loads data based on event notifications. We recommend using event-driven transfers if you require incremental data ingestion that optimizes cost efficiency.

When you set up event-driven transfers, there can be a delay of a few minutes between each data transfer. If you require immediate data availability, we recommend using the [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) that streams data directly into BigQuery with the lowest possible latency. The Storage Write API provides real-time updates for the most demanding use cases.

When choosing between the two, consider whether you need to prioritize cost-effective incremental batch ingestion with event-driven transfers, or whether you prefer the flexibility of the Storage Write API.

## Data sources with event-driven transfers support

BigQuery Data Transfer Service can use event-driven transfers with the
following data sources:

- [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer)

## Limitations

Event-driven transfers to BigQuery are subject to the following limitations:

- After an event-driven transfer is triggered, the BigQuery Data Transfer Service waits up to 10 minutes before it triggers the next transfer run, regardless if an event arrives within that time.
- Event-driven transfers don't support [runtime parameters](https://docs.cloud.google.com/bigquery/docs/gcs-transfer-parameters) for source URI or data path.
- The same Pub/Sub subscription cannot be reused by multiple event-driven transfer configurations.

## Set up a Cloud Storage event-driven transfer

Event-driven transfers from Cloud Storage use Pub/Sub notifications
to know when objects in the source bucket have been modified or added.
When using [incremental transfer](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer-overview#incremental_transfers) mode,
deleting an object at the source bucket does not delete the associated data in the destination BigQuery table.

### Before you begin

Before configuring a Cloud Storage event-driven transfer, you must perform the following steps:

1. Enable the Pub/Sub API for the project that receives
   notifications.

   [Enable
   the API](https://console.cloud.google.com/flows/enableapi?apiid=pubsub)
2. If you are the Cloud Storage Admin (`roles/storage.admin`) and the Pub/Sub Admin (`roles/pubsub.admin`), you can proceed to [Create an event-driven transfer configuration](https://docs.cloud.google.com/bigquery/docs/event-driven-transfer#create-transfer-configuration).

3. If you aren't the Cloud Storage Admin (`roles/storage.admin`) and the Pub/Sub Admin (`roles/pubsub.admin`), ask your administrator to grant you the `roles/storage.admin` and `roles/pubsub.admin` roles or ask your administrator to complete [configure Pub/Sub](https://docs.cloud.google.com/bigquery/docs/event-driven-transfer#configure-pubsub) and [configure Service Agent permissions](https://docs.cloud.google.com/bigquery/docs/event-driven-transfer#configure-service-agent-permissions) in the following sections and use the pre-configured Pub/Sub subscription to [create an event-driven transfer configuration](https://docs.cloud.google.com/bigquery/docs/event-driven-transfer#create-transfer-configuration).

4. Detailed permissions required to set up event-driven transfer config notifications:

   - If you plan to create topics and subscriptions for publishing notifications, you must have the [`pubsub.topics.create`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles) and [`pubsub.subscriptions.create`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles) permissions.

   - Whether you plan to use new or existing topics and subscriptions, you must have the following permissions. If you have already created topics and subscriptions in Pub/Sub, then you likely already have these permissions.

     - [`pubsub.topics.getIamPolicy`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles)
     - [`pubsub.topics.setIamPolicy`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles)
     - [`pubsub.subscriptions.getIamPolicy`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles)
     - [`pubsub.subscriptions.setIamPolicy`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles)
   - You must have the following permissions on the Cloud Storage bucket which you want to configure Pub/Sub notifications.

     - `storage.buckets.get`
     - `storage.buckets.update`
   - The `pubsub.admin` and `storage.admin` predefined IAM role has all the required permissions to configure a Cloud Storage event-driven transfer. For more information, see [Pub/Sub access control](https://docs.cloud.google.com/pubsub/docs/access_control#console).

#### Configure Pub/Sub notifications in Cloud Storage

1. Ensure that you've satisfied the
   [Prerequisites](https://docs.cloud.google.com/storage/docs/reporting-changes) for using
   Pub/Sub with Cloud Storage.

2. Apply a notification configuration to your Cloud Storage bucket:

   ```bash
   gcloud storage buckets notifications create gs://BUCKET_NAME --topic=TOPIC_NAME --event-types=OBJECT_FINALIZE
   ```

   Replace the following:
   - `BUCKET_NAME`: The name of the Cloud Storage bucket you want to trigger file notification events
   - `TOPIC_NAME`: The name of the Pub/Sub topic you want to receive the file notification events

   You can also add a notification configuration using other methods besides the gcloud CLI. For more information, see [Apply a notification configuration](https://docs.cloud.google.com/storage/docs/reporting-changes#command-line).
3. Verify that the Pub/Sub notification is correctly configured for Cloud Storage.
   Use the `gcloud storage buckets notifications list` command:

   ```
   gcloud storage buckets notifications list gs://BUCKET_NAME
   ```

   If successful, the response looks similar to the following:

   ```json
   etag: '132'
   id: '132'
   kind: storage#notification
   payload_format: JSON_API_V1
   selfLink: https://www.googleapis.com/storage/v1/b/my-bucket/notificationConfigs/132
   topic: //pubsub.googleapis.com/projects/my-project/topics/my-bucket
   ```
4. Create a pull subscription for the topic:

   ```bash
   gcloud pubsub subscriptions create SUBSCRIPTION_ID --topic=TOPIC_NAME
   ```

   Replace `SUBSCRIPTION_ID` with the name or ID of your new Pub/Sub pull subscription.

   You can create a pull subscription using [other methods](https://docs.cloud.google.com/pubsub/docs/create-subscription#pubsub_create_pull_subscription).

   > [!NOTE]
   > **Note:** The same Pub/Sub subscription cannot be reused by multiple event-driven transfer configurations.

#### Configure Service Agent permissions

1. Find the name of the [BigQuery Data Transfer Service agent](https://docs.cloud.google.com/bigquery/docs/access-control#bigquerydatatransfer.serviceAgent) for your project:

   1. Go to the **IAM \& Admin** page.

      [Go to IAM \& Admin](https://console.cloud.google.com/iam-admin/iam?project)
   2. Select the **Include Google-provided role grants** checkbox.

   3. The BigQuery Data Transfer Service agent is listed with the name `service-<project_number>@gcp-sa-bigquerydatatransfer.iam.gserviceaccount.com`
      is shown and is granted the [BigQuery Data Transfer Service Agent role](https://docs.cloud.google.com/bigquery/docs/access-control#bigquerydatatransfer.serviceAgent)
      (`roles/bigquerydatatransfer.serviceAgent`).

      ![Verify if the service account has the service agent role.](https://docs.cloud.google.com/static/bigquery/images/dts-verify-agent.png)

   For more information about service agents, see [Service agents](https://docs.cloud.google.com/iam/docs/service-agents).
2. Grant the [Pub/Sub Subscriber role](https://docs.cloud.google.com/iam/docs/roles-permissions/pubsub#pubsub.subscriber)
   (`pubsub.subscriber`) to the BigQuery Data Transfer Service agent.

   ### Cloud console

   Follow the instructions in
   [Controlling access through the Google Cloud console](https://docs.cloud.google.com/pubsub/docs/access-control#console)
   to grant the `Pub/Sub Subscriber` role to the BigQuery Data Transfer Service agent. The
   role can be granted at the topic, subscription, or project level.

   ### `gcloud` CLI

   Follow the instructions in
   [Setting a policy](https://docs.cloud.google.com/pubsub/docs/access-control#setting_a_policy) to
   add the following binding:

   ```json
   {
     "role": "roles/pubsub.subscriber",
     "members": [
       "serviceAccount:project-PROJECT_NUMBER@gcp-sa-bigquerydatatransfer.iam.gserviceaccount.com"
   }
   ```

   Replace `PROJECT_NUMBER` with the [project ID](https://docs.cloud.google.com/resource-manager/docs/view-update-projects#identifying_projects) that is hosting the transfer resources are created and billed.

   **Quota usage attribution:** when the BigQuery Data Transfer Service agent access the Pub/Sub subscription, the quota usage is charged against the user project.
3. Verify that the BigQuery Data Transfer Service agent is granted the
   [Pub/Sub Subscriber role](https://docs.cloud.google.com/iam/docs/roles-permissions/pubsub#pubsub.subscriber)
   (`pubsub.subscriber`).

   1. In the Google Cloud console, go to the **Pub/Sub** page.

      [Go to Pub/Sub](https://console.cloud.google.com/cloudpubsub/subscription/list?project)
   2. Select the Pub/Sub subscription that you used in the event-driven
      transfer.

   3. If the info panel is hidden, click **Show info panel** in the upper right
      corner.

   4. In the **Permissions** tab, verify that the BigQuery Data Transfer Service [service agent](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#service_agent)
      has the
      [Pub/Sub Subscriber role](https://docs.cloud.google.com/iam/docs/roles-permissions/pubsub#pubsub.subscriber)
      (`pubsub.subscriber`).

   ![Verify if the service agent has pubsub.subscriber on the subscription.](https://docs.cloud.google.com/static/bigquery/images/dts-event-driven-verify-pubsub-subscriber-permission.png)

#### Summarized commands to configure notification and permissions

The following Google Cloud CLI commands includes all the necessary commands to set up notifications and permissions as detailed in the previous sections.

### gcloud

```bash
PROJECT_ID=project_id
CONFIG_NAME=config_name
RESOURCE_NAME="bqdts-event-driven-${CONFIG_NAME}"
# Create a Pub/Sub topic.
gcloud pubsub topics create "${RESOURCE_NAME}" --project="${PROJECT_ID}"
# Create a Pub/Sub subscription.
gcloud pubsub subscriptions create "${RESOURCE_NAME}" --project="${PROJECT_ID}" --topic="projects/${PROJECT_ID}/topics/${RESOURCE_NAME}"
# Create a Pub/Sub notification.
gcloud storage buckets notifications create gs://"${RESOURCE_NAME}" --topic="projects/${PROJECT_ID}/topics/${RESOURCE_NAME}" --event-types=OBJECT_FINALIZE
# Grant roles/pubsub.subscriber permission to the DTS service agent.
PROJECT_NUMBER=$(gcloud projects describe "${PROJECT_ID}" --format='value(projectNumber)')
gcloud pubsub subscriptions add-iam-policy-binding "${RESOURCE_NAME}"  --project="${PROJECT_ID}"  --member=serviceAccount:service-"${PROJECT_NUMBER}"@gcp-sa-bigquerydatatransfer.iam.gserviceaccount.com  --role=roles/pubsub.subscriber
```

Replace the following:

- <var translate="no">`PROJECT_ID`</var>: The ID of your project.
- <var translate="no">`CONFIG_NAME`</var>: A name to identify this transfer configuration.

### Create a transfer configuration

You can create an event-driven Cloud Storage transfer by [creating a Cloud Storage transfer](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer#set_up_a_cloud_storage_transfer) and selecting **Event-driven** as the **Schedule Type** . As the Cloud Storage Admin (`roles/storage.admin`) and the Pub/Sub Admin (`roles/pubsub.admin`), you have sufficient permissions for the BigQuery Data Transfer Service to automatically configure Cloud Storage to send notifications.

If you aren't the Cloud Storage Admin (`roles/storage.admin`) and the Pub/Sub Admin (`roles/pubsub.admin`), you must instead ask your administrator to grant you the roles or ask your administrator to complete the required [Pub/Sub notifications in Cloud Storage configurations](https://docs.cloud.google.com/bigquery/docs/event-driven-transfer#configure-pubsub) and [Service Agent permission configurations](https://docs.cloud.google.com/bigquery/docs/event-driven-transfer#configure-service-agent-permissions) before you can create the event-driven transfer.

> [!CAUTION]
> **Caution:** Don't remove the [BigQuery Data Transfer Service Agent](https://docs.cloud.google.com/iam/docs/service-agents#bigquerydatatransfer.serviceAgent) from the `pubsub.subscriber` and `serviceusage.serviceUsageConsumer` predefined IAM role. The removal prevent BigQuery from receiving notifications from the Pub/Sub topic and subscription.