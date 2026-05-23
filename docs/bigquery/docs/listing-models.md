# List models

This page shows you how to list BigQuery ML models in a dataset. You can
list BigQuery ML models by:

- Using the Google Cloud console.
- Using the `bq ls` command in the bq command-line tool.
- Calling the [`models.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/list) API method directly or by using the client libraries.

## Required permissions

To list models in a dataset, you must be assigned the
[`READER`](https://docs.cloud.google.com/bigquery/docs/access-control-basic-roles#dataset-basic-roles)
role on the dataset, or you must be assigned a project-level Identity and Access Management (IAM) role that
includes `bigquery.models.list` permissions. If you are granted
`bigquery.models.list` permissions at the project level, you can list models in
any dataset in the project. The following predefined, project-level IAM roles
include `bigquery.models.list` permissions:

- `bigquery.dataViewer`
- `bigquery.dataEditor`
- `bigquery.dataOwner`
- `bigquery.metadataViewer`
- `bigquery.user`
- `bigquery.admin`

For more information on IAM roles and permissions in BigQuery ML,
see [Access control](https://docs.cloud.google.com/bigquery/docs/access-control). For more
information on dataset-level roles, see
[Basic roles for datasets](https://docs.cloud.google.com/bigquery/docs/access-control-basic-roles#dataset-basic-roles).

## List models

To list models in a dataset:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your model.

5. Click the **Models** tab.

### bq

Issue the `bq ls` command with the `--models` or `-m` flag. The
[`--format`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#global_flags)
flag can be used to control the output. If you are listing models
in a project other than your default project,
add the project ID to the dataset in the following format:
`[PROJECT_ID]:[DATASET]`.

```
bq ls -m --format=pretty PROJECT_ID:DATASET
```

Replace the following:

- `PROJECT_ID` is your project ID.
- `DATASET` is the name of the dataset.

The command output looks like the following when the `--format=pretty` flag
is used. `--format=pretty` produces formatted table output. The `Model Type`
column displays the model type, for example, `KMEANS`.

```
+---+---+---+---+
|           Id            | Model Type | Labels |  Creation Time  |
+---+---+---+---+
| mymodel                 | KMEANS     |        | 03 May 03:02:27 |
+---+---+---+---+
```

Examples:

Enter the following command to list models in dataset `mydataset` in your
default project.

    bq ls --models --format=pretty mydataset

Enter the following command to list models in dataset `mydataset` in
`myotherproject`. This command uses the `-m` shortcut to list models.

    bq ls -m --format=pretty myotherproject:mydataset

### API

To list models by using the API, call the [`models.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/list)
method and provide the `projectId` and `datasetId`.

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
    	"io"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // listModels demonstrates iterating through the collection of ML models in a dataset
    // and printing a basic identifier of the model.
    func listModels(w io.Writer, projectID, datasetID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	fmt.Fprintf(w, "Models contained in dataset %q\n", datasetID)
    	it := client.Dataset(datasetID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Dataset_Models(ctx)
    	for {
    		m, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		fmt.Fprintf(w, "Model: %s\n", m.FullyQualifiedName())
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

    import com.google.api.gax.paging.https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.paging.Page.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.ModelListOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Model.html;

    public class ListModels {

      public static void runListModels() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        listModels(datasetName);
      }

      public static void listModels(String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          Page<Model> models = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_listModels_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_ModelListOption____(datasetName, ModelListOption.pageSize(100));
          if (models == null) {
            System.out.println("Dataset does not contain any models.");
            return;
          }
          models
              .iterateAll()
              .forEach(model -> System.out.printf("Success! Model ID: %s", model.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ModelInfo.html#com_google_cloud_bigquery_ModelInfo_getModelId__()));
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Models not listed in dataset due to error: \n" + e.toString());
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

    async function listModels() {
      // Lists all existing models in the dataset.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";

      const dataset = bigquery.dataset(datasetId);

      dataset.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/dataset.html().then(data => {
        const models = data[0];
        console.log('Models:');
        models.forEach(model => console.log(model.metadata));
      });
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

    # TODO(developer): Set dataset_id to the ID of the dataset that contains
    #                  the models you are listing.
    # dataset_id = 'your-project.your_dataset'

    models = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_models(dataset_id)  # Make an API request.

    print("Models contained in '{}':".format(dataset_id))
    for model in models:
        full_model_id = "{}.{}.{}".format(
            model.project, model.dataset_id, model.model_id
        )
        friendly_name = model.friendly_name
        print("{}: friendly_name='{}'".format(full_model_id, friendly_name))

<br />

## What's next

- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To get started using BigQuery ML, see [Create machine learning models in BigQuery ML](https://docs.cloud.google.com/bigquery/docs/create-machine-learning-model).
- To learn more about working with models, see:
  - [Get model metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata)
  - [Update model metadata](https://docs.cloud.google.com/bigquery/docs/updating-model-metadata)
  - [Manage models](https://docs.cloud.google.com/bigquery/docs/managing-models)
  - [Delete models](https://docs.cloud.google.com/bigquery/docs/deleting-models)