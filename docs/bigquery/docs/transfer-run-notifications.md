# BigQuery Data Transfer Service run notifications

This page provides an overview of run notifications for the
BigQuery Data Transfer Service.

There are two types of run notifications you can configure for the
BigQuery Data Transfer Service:

- **Pub/Sub notifications**: machine-readable notifications sent when a transfer run succeeds or fails
- **Email notifications**: human-readable notifications sent when a transfer run fails

You can configure each type individually, or you can use both Pub/Sub
and email run notifications.

## Pub/Sub notifications

Pub/Sub notifications send information about transfer runs to
a [Pub/Sub](https://docs.cloud.google.com/pubsub) topic. Pub/Sub notifications are triggered by
completed transfer runs in the following [states](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/TransferState):

- `SUCCEEDED`
- `FAILED`
- `CANCELLED`

You can send notifications to any Pub/Sub topic in any project for
which you have sufficient permissions. Once received by the
Pub/Sub topic, the resulting message can be sent to any number of
subscribers to the topic.

### Before you begin

Before configuring Pub/Sub transfer run notifications, you should:

1. Enable the Pub/Sub API for the project that will receive
   notifications.

   [Enable
   the API](https://console.cloud.google.com/flows/enableapi?apiid=pubsub)
2. Have sufficient permissions on the project that will receive notifications:

   - If you own the project that will receive notifications, you most likely
     have the necessary permission.

   - If you plan to create topics for receiving notifications, you should
     have [`pubsub.topics.create`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles) permissions.

   - Whether you plan to use new or existing topics, you should have
     [`pubsub.topics.getIamPolicy`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles) and
     [`pubsub.topics.setIamPolicy`](https://docs.cloud.google.com/pubsub/docs/access_control#tbl_roles) permissions. If you create a topic,
     you typically have permission for it already. The following predefined
     IAM role has both `pubsub.topics.getIamPolicy` and
     `pubsub.topics.setIamPolicy` permissions: `pubsub.admin`. See
     [Pub/Sub access control](https://docs.cloud.google.com/pubsub/docs/access_control#console) for more information.

3. [Have an existing Pub/Sub topic](https://docs.cloud.google.com/pubsub/docs/create-topic) that you want to send
   notifications to.

> [!CAUTION]
> **Caution:** Don't remove the [BigQuery Data Transfer Service Agent](https://docs.cloud.google.com/iam/docs/service-agents#bigquerydatatransfer.serviceAgent) from the `pubsub.publisher` predefined IAM role. The removal can cause publishing notification failures to the Pub/Sub topic.

> [!CAUTION]
> **Caution:** Don't specify any custom schema when creating the Pub/Sub topic. Specifying a custom schema can cause the notification publication to fail.

### Notification format

Notifications sent to the Pub/Sub topic consist of two parts:

- **Attributes**: A set of key:value pairs describing the event.
- **Payload**: A string that contains the metadata of the changed object.

#### Attributes

Attributes are key:value pairs contained in all notifications sent by
BigQuery Data Transfer Service to your Pub/Sub topic. Notifications always
contain the following set of key:value pairs, regardless of the notification's
payload:

| **Attribute name** | **Example** | **Description** |
|---|---|---|
| **eventType** | `TRANSFER_RUN_FINISHED` | The type of event that has just occurred. `TRANSFER_RUN_FINISHED` is the only possible value. |
| **payloadFormat** | `JSON_API_V1` | The format of the object payload. `JSON_API_V1` is the only possible value. |

#### Payload

The payload is a string that contains the metadata of the transfer run. The type
of payload is not configurable at this time and is provided to accommodate
future API version changes.

| **Payload type** | **Description** |
|---|---|
| **JSON_API_V1** | The payload will be a UTF-8 JSON-serialized string containing the [resource representation of a `TransferRun`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs#TransferRun). |

## Email notifications

Email notifications send human-readable email messages when a transfer run
fails. These messages are sent to the email of the *transfer administrator* -
the account that set up the transfer. You cannot configure the content of the
message, and you cannot configure the recipient of the message.

If you used a service account to authenticate a transfer configuration, then you
might not have access to the email to receive transfer run notification emails.
In such cases, we recommend that you set up [Pub/Sub notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications#notifications)
to receive transfer run notifications.

To send transfer run email notifications to more users, set up email
forwarding rules to distribute the messages. If you are using Gmail, you can
[Automatically forward Gmail messages to another account](https://support.google.com/mail/answer/10957).

The email notification is sent by the BigQuery Data Transfer Service and contains details
on the transfer configuration, the transfer run, and a link to the run history
for the failed run. For example:

```
From: bigquery-data-transfer-service-noreply@google.com
To: TRANSFER_ADMIN
Title: BigQuery Data Transfer Service — Transfer Run Failure —
DISPLAY_NAME

Transfer Configuration
Display Name: DISPLAY_NAME
Source: DATA_SOURCE
Destination: PROJECT_ID

Run Summary
Run: RUN_NAME
Schedule Time: SCHEDULE_TIME
Run Time: RUN_TIME
View Run History


Google LLC 1600 Amphitheatre Parkway, Mountain View, CA 94043

This email was sent because you indicated you are willing to receive Run
Notifications from the BigQuery Data Transfer Service. If you do not wish to
receive such emails in the future, click View Transfer Configuration and
un-check the "Send E-mail Notifications" option.
```

## Turn on or edit notifications

To turn on notifications, or edit an existing one, choose one of the following:

### Console

1. Go to the BigQuery page in the Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Data transfers**.

3. To turn on notifications for a new transfer, click
   **Create transfer** . To adjust
   notifications for an existing transfer, click the name of the transfer
   and then click **Edit**.

4. In the **Notification options** section, click the toggles next to the
   notification types to enable.

   - **Email notifications**: When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - **Pub/Sub notifications** : When you enable this option, choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.api.gax.rpc.https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html;
    import java.io.IOException;
    import java.util.HashMap;
    import java.util.Map;

    // Sample to get run notification
    public class RunNotification {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String projectId = "MY_PROJECT_ID";
        final String datasetId = "MY_DATASET_ID";
        final String pubsubTopicName = "MY_TOPIC_NAME";
        final String query =
            "SELECT CURRENT_TIMESTAMP() as current_time, @run_time as intended_run_time, "
                + "@run_date as intended_run_date, 17 as some_integer";
        Map<String, Value> params = new HashMap<>();
        params.put("query", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(query).build());
        params.put(
            "destination_table_name_template",
            https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue("my_destination_table_{run_date}").build());
        params.put("write_disposition", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue("WRITE_TRUNCATE").build());
        params.put("partitioning_field", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().build());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(datasetId)
                .setDisplayName("Your Scheduled Query Name")
                .setDataSourceId("scheduled_query")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .setSchedule("every 24 hours")
                .setNotificationPubsubTopic(pubsubTopicName)
                .build();
        runNotification(projectId, transferConfig);
      }

      public static void runNotification(String projectId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html dataTransferServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = dataTransferServiceClient.createTransferConfig(request);
          System.out.println(
              "\nScheduled query with run notification created successfully :" + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("\nScheduled query with run notification was not created." + ex.toString());
        }
      }
    }

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    transfer_config_name = "projects/1234/locations/us/transferConfigs/abcd"
    pubsub_topic = "projects/PROJECT-ID/topics/TOPIC-ID"
    from google.cloud import bigquery_datatransfer
    from google.protobuf import field_mask_pb2

    transfer_client = bigquery_datatransfer.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html()

    transfer_config = bigquery_datatransfer.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.types.TransferConfig.html(name=transfer_config_name)
    transfer_config.notification_pubsub_topic = pubsub_topic
    update_mask = field_mask_pb2.FieldMask(paths=["notification_pubsub_topic"])

    transfer_config = transfer_client.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html#google_cloud_bigquery_datatransfer_v1_services_data_transfer_service_DataTransferServiceClient_update_transfer_config(
        {"transfer_config": transfer_config, "update_mask": update_mask}
    )

    print(f"Updated config: '{transfer_config.name}'")
    print(f"Notification Pub/Sub topic: '{transfer_config.notification_pubsub_topic}'")

## Run notification pricing

If you configure Pub/Sub run notifications, you will incur
Pub/Sub charges. For more information, see the Pub/Sub
[Pricing](https://cloud.google.com/pubsub/pricing) page.

## What's next

- [Learn more about Pub/Sub](https://docs.cloud.google.com/pubsub/docs/overview).
- Learn more about creating Pub/Sub [topics](https://docs.cloud.google.com/pubsub/docs/create-topic).
- Learn more about the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).