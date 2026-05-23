# Update model metadata

This page shows you how to update BigQuery ML model metadata. You can
update model metadata by:

- Using the Google Cloud console.
- Using the `bq update` command in the bq command-line tool.
- Calling the [`models.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch) API method directly or by using the client libraries.

The following model metadata can be updated:

- [**Description**](https://docs.cloud.google.com/bigquery/docs/updating-model-metadata#description): Can be updated by using the Google Cloud console, bq command-line tool, API, or client libraries.
- [**Labels**](https://docs.cloud.google.com/bigquery/docs/updating-model-metadata#labels): Can be updated by using the Google Cloud console, bq command-line tool, API, or client libraries.
- [**Expiration time**](https://docs.cloud.google.com/bigquery/docs/updating-model-metadata#expiration): Can be updated by using the bq tool, API, or client libraries.

## Required permissions

To update model metadata, you must be assigned the
[`WRITER`](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_dataset)
role on the dataset, or you must be assigned a project-level Identity and Access Management (IAM) role that
includes `bigquery.models.updateMetadata` permissions. If you are granted
`bigquery.models.updateMetadata` permissions at the project level, you can
update metadata for models in any dataset in the project. The following
predefined, project-level IAM roles include `bigquery.models.updateMetadata`
permissions:

- `bigquery.dataEditor`
- `bigquery.dataOwner`
- `bigquery.admin`

For more information on IAM roles and permissions in BigQuery ML,
see [Access control](https://docs.cloud.google.com/bigquery/docs/access-control).

## Update a model's description

A model's description is a text string that is used to identify the
model.

To update a model's description:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset.

4. Click the **Models** tab, and then click a model name
   to select the model.

5. Click the **Details** tab.

6. To update the model's description, click **Edit**
   .

7. In the **Edit detail** dialog, update the description and then
   click **Save**.

### bq

To update a model's description, issue the `bq update` command with the
`--model` or `-m` flag and the `--description` flag.

If you are updating a model in a project other than your default project,
add the project ID to the dataset in the following format:
`[PROJECT_ID]:[DATASET]`.

```
bq update --model --description "[STRING]" PROJECT_ID:DATASET.MODEL
```

Replace the following:

- `STRING` is the text string that describes your model in quotes.
- `PROJECT_ID` is your project ID.
- `DATASET` is the name of the dataset.
- `MODEL` is the name of the model.

The command output looks like the following:

```
Model 'myproject.mydataset.mymodel' successfully updated.
```

You can confirm your changes by issuing the `bq show` command. For more
information, see [Get model metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata).

Examples:

Enter the following command to update the description of `mymodel` in
`mydataset` in your default project.

    bq update --model --description "My updated description" \
    mydataset.mymodel

Enter the following command to update the description of `mymodel` in
`mydataset` in `myotherproject`.

    bq update --model --description "My updated description" \
    myotherproject:mydataset.mymodel

### API

To update a model's description by using the API, call the
[`models.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch)
method and provide the `projectId`, `datasetId`, and `modelId`. To modify
the description, add to or update the "description" property for the
[model resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#Model).

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // updateModelDescription demonstrates fetching BigQuery ML model metadata and updating the
    // Description metadata.
    func updateModelDescription(projectID, datasetID, modelID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// modelID := "mymodel"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	model := client.Dataset(datasetID).Model(modelID)
    	oldMeta, err := model.Metadata(ctx)
    	if err != nil {
    		return fmt.Errorf("couldn't retrieve model metadata: %w", err)
    	}
    	update := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_ModelMetadataToUpdate{
    		Description: "This model was modified from a Go program",
    	}
    	if _, err = model.Update(ctx, update, oldMeta.ETag); err != nil {
    		return fmt.Errorf("couldn't update model: %w", err)
    	}
    	return nil
    }

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

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Model.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ModelId.html;

    // Sample to update description on a model
    public class UpdateModelDescription {

      public static void runUpdateModelDescription() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String modelName = "MY_MODEL_NAME";
        String newDescription = "A really great model.";
        updateModelDescription(datasetName, modelName, newDescription);
      }

      public static void updateModelDescription(
          String datasetName, String modelName, String newDescription) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Model.html model = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getModel_com_google_cloud_bigquery_ModelId_com_google_cloud_bigquery_BigQuery_ModelOption____(ModelId.of(datasetName, modelName));
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_update_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(model.toBuilder().setDescription(newDescription).build());
          System.out.println("Model description updated successfully to " + newDescription);
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Model description was not updated \n" + e.toString());
        }
      }
    }

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function updateModel() {
      // Updates a model's metadata.

      /**
       * TODO(developer): Uncomment the following lines before running the sample
       */
      // const datasetId = "my_dataset";
      // const modelId = "my__model";

      const metadata = {
        description: 'A really great model.',
      };

      const dataset = bigquery.dataset(datasetId);
      const [apiResponse] = await dataset.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/dataset.html(modelId).setMetadata(metadata);
      const newDescription = apiResponse.description;

      console.log(`${modelId} description: ${newDescription}`);
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


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set model_id to the ID of the model to fetch.
    # model_id = 'your-project.your_dataset.your_model'

    model = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_model(model_id)  # Make an API request.
    model.description = "This model was modified from a Python program."
    model = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_update_model(model, ["description"])  # Make an API request.

    full_model_id = "{}.{}.{}".format(model.project, model.dataset_id, model.model_id)
    print(
        "Updated model '{}' with description '{}'.".format(
            full_model_id, model.description
        )
    )

## Update a model's labels

Labels are key-value pairs that you can attach to a resource. When you create
BigQuery ML resources, labels are optional. For more information, see
[Adding and using labels](https://docs.cloud.google.com/bigquery/docs/adding-using-labels).

To update a model's labels:

### Console

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset.

3. Click the **Models** tab, and then click a model name
   to select the model.

4. Click the **Details** tab.

5. To update the model's labels, click **Edit**
   .

6. In the **Edit detail** dialog, add, delete, or modify labels, and then
   click **Save**.

### bq

To update a model's labels, issue the `bq update` command with the
`--model` or `-m` flag and the `--set_label` flag. Repeat the `--set_label`
flag to add or update multiple labels.

If you are updating a model in a project other than your default project,
add the project ID to the dataset in the following format:
`[PROJECT_ID]:[DATASET]`.

```
bq update --model --set_label KEY:VALUE \
PROJECT_ID:DATASET.MODEL
```

Replace the following:

- `KEY:VALUE` corresponds to a key:value pair for a label that you want to add or update. If you specify the same key as an existing label, the value for the existing label is updated. The key must be unique.
- `PROJECT_ID` is your project ID.
- `DATASET` is the name of the dataset.
- `MODEL` is the name of the model.

The command output looks like the following.

```
Model 'myproject.mydataset.mymodel' successfully updated.
```

You can confirm your changes by issuing the `bq show` command. For more
information, see [Get model metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata).

Examples:

To update the `department` label on `mymodel`, enter the `bq update` command
and specify `department` as the label key. For example, to update the
`department:shipping` label to `department:logistics`, enter the following
command. `mydataset` is in `myotherproject`, not your default project.

    bq update --model --set_label department:logistics \
    myotherproject:mydataset.mymodel

### API

To update a model's labels by using the API, call the
[`models.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch)
method and provide the `projectId`, `datasetId`, and `modelId`. To modify
the labels, add to or update the "labels" property for the
[model resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#Model).

## Update a model's expiration time

A model's expiration time is a timestamp value that dictates when a model is
deleted. You can set a model's expiration time when the model is created by
using the CLI, the API, or the client libraries. You can also set or update the
expiration time on a model after it is created. A model's expiration time is
often referred to as "time to live" or TTL.

If you don't set an expiration time on a model, the model never expires and you
must [delete](https://docs.cloud.google.com/bigquery/docs/deleting-models) the model manually.

> [!NOTE]
> **Note:** Setting or updating the expiration time on a model is not supported by the Google Cloud console.

The value for the expiration time is expressed differently depending
on where the value is set. Use the method that gives you the appropriate
level of granularity:

- In the command-line tool, expiration is expressed in seconds from the current UTC time. When you specify the expiration on the command line, the integer value in seconds is added to the current UTC timestamp.
- In the API, expiration is expressed in milliseconds since the epoch. If you specify an expiration value that is less than the current timestamp, the model expires immediately.

To update the expiration time for a model:

### Console

Setting or updating the expiration time on a model is not
supported by the Google Cloud console.

### bq

To update a model's expiration time, issue the `bq update` command with the
`--model` or `-m` flag and the `--expiration` flag.

If you are updating a model in a project other than your default project,
add the project ID to the dataset in the following format:
`[PROJECT_ID]:[DATASET]`.

```
bq update --model --expiration INTEGER \
PROJECT_ID:DATASET.MODEL
```

Replace the following:

- `INTEGER` is the lifetime (in seconds) for the model. The minimum value is 3600 seconds (one hour). The expiration time evaluates to the current UTC time plus the integer value.
- `PROJECT_ID` is your project ID.
- `DATASET` is the name of the dataset.
- `MODEL` is the name of the model.

The command output looks like the following.

```
Model 'myproject.mydataset.mymodel' successfully updated.
```

You can confirm your changes by issuing the `bq show` command. For more
information, see [Get model metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata).

Examples:

Enter the following command to update the expiration time of `mymodel` in
`mydataset` to 5 days (432000 seconds). `mydataset` is in your default
project.

    bq update --model --expiration 432000 mydataset.mymodel

Enter the following command to update the expiration time of `mymodel` in
`mydataset` to 5 days (432000 seconds). `mydataset` is in `myotherproject`,
not your default project.

    bq update --model --expiration 432000 myotherproject:mydataset.mymodel

### API

To update a model's expiration by using the API, call the
[`models.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch)
method and provide the `projectId`, `datasetId`, and `modelId`. To modify
the expiration, add to or update the "expirationTime" property for the
[model resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#Model).
"expirationTime" is expressed in milliseconds since the epoch.

## What's next

- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To get started using BigQuery ML, see [Create machine learning models in BigQuery ML](https://docs.cloud.google.com/bigquery/docs/create-machine-learning-model).
- To learn more about working with models, see:
  - [Get model metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata)
  - [List models](https://docs.cloud.google.com/bigquery/docs/listing-models)
  - [Manage models](https://docs.cloud.google.com/bigquery/docs/managing-models)
  - [Delete models](https://docs.cloud.google.com/bigquery/docs/deleting-models)