# Delete models

This page shows you how to delete BigQuery ML models. You can delete a
model by:

- Using the Google Cloud console
- Using the bq command-line tool's `bq rm` command or `bq query` command
- Calling the [`models.delete`](https://docs.cloud.google.com/bigquery/docs/reference/v2/models/delete) API method or calling the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) method
- Using the client libraries

You can only delete one model at a time. When you delete a model, any
data in the model is also deleted.

To automatically delete models after a specified period of time, set the model's
expiration time when you create it using the bq command-line tool, the API, or the client
libraries. If you did not set the expiration when the model was created, you can
[update the model's expiration time](https://docs.cloud.google.com/bigquery/docs/updating-model-metadata#expiration).

## Limitations on deleting models

Deleting a model is subject to the following limitations:

- You can't delete multiple models at the same time. You must delete them individually.
- You can't restore a deleted model.

## Required permissions

To delete models in a dataset, you must be assigned the
[`WRITER`](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_dataset)
role on the dataset, or you must be assigned a project-level Identity and Access Management (IAM) role that
includes `bigquery.models.delete` permissions. If you are granted
`bigquery.models.delete` permissions at the project level, you can delete models
in any dataset in the project. The following project-level IAM roles
include `bigquery.models.delete` permissions:

- `bigquery.dataEditor`
- `bigquery.dataOwner`
- `bigquery.admin`

For more information about IAM roles and permissions in BigQuery ML,
see [Access control](https://docs.cloud.google.com/bigquery/docs/access-control).

## Delete a model

To delete a model, do the following:

### Console

You can delete a model in the Google Cloud console by using the **Delete Model** option
or by running a query that contains a
[`DROP MODEL | DROP MODEL IF EXISTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-drop-model)
DDL statement.

**Option one:** Use the **Delete Model** option.

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
2. In the **Explorer** pane, expand your project, click **Datasets**,
   and then click your dataset.

3. Click the **Models** tab, and then click a model name
   to select the model.

4. Click the options icon
   for
   the model and then click **Delete**.

5. In the **Delete model** dialog, type `delete` and then click
   **Delete**.

**Option two:** Use a DDL statement.

1. In the Google Cloud console, go to the BigQuery page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. Click **Compose new query**.

3. Type your DDL statement in the **Query editor** text area.

   <br />

   ```googlesql
    DROP MODEL mydataset.mymodel
   ```

   <br />

4. Click **Run**. When the query completes, the model is removed from
   the navigation pane.

### bq

You can delete a model using the bq command-line tool by entering the:

- `bq rm` command with the `--model` or `-m` flag
- `bq query` command with the DDL statement as the query parameter

If you are deleting a model in a project other than your default project,
add the project ID to the dataset in the following format:
`[PROJECT_ID]:[DATASET].[MODEL]`.

**Option one:** Enter the `bq rm` command

When you use the `bq rm` command to remove a model, you must confirm the
action. You can use the `--force flag` (or `-f` shortcut) to skip confirmation.

```
bq rm -f --model PROJECT_ID:DATASET.MODEL
```

Replace the following:

- `PROJECT_ID` is your project ID.
- `DATASET` is the name of the dataset.
- `MODEL` is the name of the model.

The `rm` command produces no output.

Examples:

Enter the following command to delete `mymodel` from `mydataset`. `mydataset`
is in your default project.

    bq rm --model mydataset.mymodel

Enter the following command to delete `mymodel` from `mydataset`. `mydataset`
is in `myotherproject`, not your default project.

    bq rm --model myotherproject:mydataset.mymodel

Enter the following command to delete `mymodel` from `mydataset`. `mydataset`
is in your default project. The command uses the `-f` shortcut to bypass
confirmation.

    bq rm -f --model mydataset.mymodel

You can confirm that the model was deleted by issuing the `bq ls` command.
For more information, see [List models](https://docs.cloud.google.com/bigquery/docs/listing-models).

**Option two:** Enter the `bq query` command

To delete a model by using the `bq query` command, supply the `DROP MODEL`
statement in the query parameter and supply the `--use_legacy_sql=false`
flag to specify GoogleSQL query syntax.

Examples:

Enter the following command to delete `mymodel` from `mydataset`. `mydataset`
is in your default project.

    bq query --use_legacy_sql=false 'DROP MODEL mydataset.mymodel'

Enter the following command to delete `mymodel` from `mydataset`. `mydataset`
is in `myotherproject`, not your default project.

    bq query --use_legacy_sql=false \
    'DROP MODEL myotherproject:mydataset.mymodel'

### API

**Option one:** Call the `models.delete` method

To delete a model, call the [`models.delete`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete)
method and provide the `projectId`, `datasetId`, and `modelId`.

**Option two:** Call the `jobs.query` method

To delete a model, call the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query)
method and supply the `DROP MODEL` DDL statement in the request body's
[query](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#queryrequest) property.

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

    // deleteModel demonstrates deletion of BigQuery ML model.
    func deleteModel(projectID, datasetID, modelID string) error {
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
    	if err := model.Delete(ctx); err != nil {
    		return fmt.Errorf("couldn't delete model: %w", err)
    	}
    	return nil
    }

<br />

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ModelId.html;

    // Sample to delete a model
    public class DeleteModel {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String modelName = "MY_MODEL_NAME";
        deleteModel(datasetName, modelName);
      }

      public static void deleteModel(String datasetName, String modelName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();
          boolean success = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____(ModelId.of(datasetName, modelName));
          if (success) {
            System.out.println("Model deleted successfully");
          } else {
            System.out.println("Model was not found");
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Model was not deleted. \n" + e.toString());
        }
      }
    }

<br />

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

    async function deleteModel() {
      // Deletes a model named "my_model" from "my_dataset".

      /**
       * TODO(developer): Uncomment the following lines before running the sample
       */
      // const datasetId = "my_dataset";
      // const modelId = "my_model";

      const dataset = bigquery.dataset(datasetId);
      const model = dataset.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/dataset.html(modelId);
      await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/dataset.html.delete();

      console.log(`Model ${modelId} deleted.`);
    }

<br />

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

    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_delete_model(model_id)  # Make an API request.

    print("Deleted model '{}'.".format(model_id))

<br />

## Restore a deleted model

You can't restore a deleted model.

## What's next

- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To get started using BigQuery ML, see [Create machine learning models in BigQuery ML](https://docs.cloud.google.com/bigquery/docs/create-machine-learning-model).
- To learn more about working with models, see:
  - [Get model metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata)
  - [List models](https://docs.cloud.google.com/bigquery/docs/listing-models)
  - [Update model metadata](https://docs.cloud.google.com/bigquery/docs/updating-model-metadata)
  - [Manage models](https://docs.cloud.google.com/bigquery/docs/managing-models)