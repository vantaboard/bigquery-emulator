# Listing datasets

This document describes how to list and get information about datasets in BigQuery.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document.

### Required role


To get the permission that
you need to list datasets or get information on datasets,

ask your administrator to grant you the
[BigQuery Metadata Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.metadataViewer) (`roles/bigquery.metadataViewer`) IAM role on your project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`bigquery.datasets.get`
permission,
which is required to
list datasets or get information on datasets.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

When you apply the `roles/bigquery.metadataViewer` role at the project or
organization level, you can list all the datasets in the project. When you
apply the `roles/bigquery.metadataViewer` role at the dataset level, you can
list all the datasets for which you have been granted that role.

## List datasets

Select one of the following options:

### Console

1. In the navigation menu, click **Studio**.

2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand a project, click **Datasets**
   to see the datasets in that project, and then click the name of your dataset. You can also use the search field or filters to find your
   dataset.

### SQL

Query the [`INFORMATION_SCHEMA.SCHEMATA` view](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets-schemata):

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     schema_name
   FROM
     PROJECT_ID.`region-REGION`.INFORMATION_SCHEMA.SCHEMATA;
   ```


   Replace the following:
   - `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.
   - `REGION`: any [dataset region name](https://docs.cloud.google.com/bigquery/docs/locations). For example, `us`.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Issue the `bq ls` command to list datasets by dataset ID. The `--format`
flag can be used to control the output. If you are listing datasets in a
project other than your default project, add the `--project_id` flag to the
command.

To list all datasets in a project, including
[hidden datasets](https://docs.cloud.google.com/bigquery/docs/datasets#hidden_datasets),
use the `--all` flag or the `-a` shortcut.

To list all datasets in a project, excluding hidden datasets, use the
`--datasets` flag or the `-d` shortcut. This flag is optional. By default,
hidden datasets are not listed.

Additional flags include:

- `--filter`: List datasets that match the filter expression. Use a space-separated list of label keys and values in the form `labels.key:value`. For more information on filtering datasets using labels, see [Adding and using labels](https://docs.cloud.google.com/bigquery/docs/filtering-labels#filtering_datasets_using_labels). Use the `status:live` keyword to filter datasets based on status. Valid values of `status` are `live`(default), `deleted`, and `any`.
- `--max_results` or `-n`: An integer indicating the maximum number of results. The default value is `50`.

```bash
bq ls --filter labels.key:value \
--max_results integer \
--format=prettyjson \
--project_id project_id
```

Replace the following:

- <var translate="no">key:value</var>: a label key and value
- <var translate="no">integer</var>: an integer representing the number of datasets to list
- <var translate="no">project_id</var>: the name of your project

Examples:

Enter the following command to list datasets in your default project. `--
format` is set to pretty to return a basic formatted table.

    bq ls --format=pretty

Enter the following command to list datasets in `myotherproject`. `--format`
is set to `prettyjson` to return detailed results in JSON format.

    bq ls --format=prettyjson --project_id myotherproject

Enter the following command to list all datasets including hidden
datasets in your default project. In the output, hidden datasets begin
with an underscore.

    bq ls -a

Enter the following command to return more than the default output of 50
datasets from your default project.

    bq ls --max_results 60

Enter the following command to list datasets in your default project with
the label `org:dev`.

    bq ls --filter labels.org:dev

### API

To list datasets using the API, call the [`datasets.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list)
API method.

### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;
    using System.Collections.Generic;
    using System.Linq;

    public class BigQueryListDatasets
    {
        public void ListDatasets(
            string projectId = "your-project-id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            // Retrieve list of datasets in project
            List<BigQueryDataset> datasets = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_ListDatasets_Google_Apis_Bigquery_v2_Data_ProjectReference_Google_Cloud_BigQuery_V2_ListDatasetsOptions_().ToList();
            // Display the results
            if (datasets.Count > 0)
            {
                Console.WriteLine($"Datasets in project {projectId}:");
                foreach (var dataset in datasets)
                {
                    Console.WriteLine($"\t{dataset.Reference.DatasetId}");
                }
            }
            else
            {
                Console.WriteLine($"{projectId} does not contain any datasets.");
            }
        }
    }

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

    // listDatasets demonstrates iterating through the collection of datasets in a project.
    func listDatasets(projectID string, w io.Writer) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	it := client.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Client_Datasets(ctx)
    	for {
    		dataset, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		fmt.Fprintln(w, dataset.DatasetID)
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

    import com.google.api.gax.paging.https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.paging.Page.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.DatasetListOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html;

    public class ListDatasets {

      public static void runListDatasets() {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        listDatasets(projectId);
      }

      public static void listDatasets(String projectId) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          Page<Dataset> datasets = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_listDatasets_com_google_cloud_bigquery_BigQuery_DatasetListOption____(projectId, DatasetListOption.pageSize(100));
          if (datasets == null) {
            System.out.println("Dataset does not contain any models");
            return;
          }
          datasets
              .iterateAll()
              .forEach(
                  dataset -> System.out.printf("Success! Dataset ID: %s ", dataset.getDatasetId()));
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Project does not contain any datasets \n" + e.toString());
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

    async function listDatasets() {
      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const projectId = "my_project_id";

      // Lists all datasets in the specified project.
      // If projectId is not specified, this method will take
      // the projectId from the authenticated BigQuery Client.
      const [datasets] = await bigquery.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html({projectId});
      console.log('Datasets:');
      datasets.forEach(dataset => console.log(dataset.id));
    }

### PHP


Before trying this sample, follow the PHP setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery PHP API
reference documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    use Google\Cloud\BigQuery\BigQueryClient;

    /** Uncomment and populate these variables in your code */
    // $projectId  = 'The Google project ID';

    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $datasets = $bigQuery->datasets();
    foreach ($datasets as $dataset) {
        print($dataset->id() . PHP_EOL);
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

    datasets = list(client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_datasets())  # Make an API request.
    project = client.project

    if datasets:
        print("Datasets in project {}:".format(project))
        for dataset in datasets:
            print("\t{}".format(dataset.dataset_id))
    else:
        print("{} project does not contain any datasets.".format(project))

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://googleapis.dev/ruby/google-cloud-bigquery/latest/Google/Cloud/Bigquery.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    require "google/cloud/bigquery"

    def list_datasets project_id = "your-project-id"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html project: project_id

      puts "Datasets in project #{project_id}:"
      bigquery.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Project.html.each do |dataset|
        puts "\t#{dataset.dataset_id}"
      end
    end

## Get information about datasets

Select one of the following options:

### Console

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand a project, click **Datasets**
   to see the datasets in that project, and then click the name of your dataset. You can also use the search field or filters to find your
   dataset.

   The description and details appear in the **Details** tab.
3. Optional: You can go to other tabs to see the list of tables, routines,
   and models in the dataset.

By default, [hidden datasets](https://docs.cloud.google.com/bigquery/docs/datasets#hidden_datasets)
are hidden from the Google Cloud console. To show information about
hidden datasets, use the bq command-line tool or the API.

### SQL

Query the [`INFORMATION_SCHEMA.SCHEMATA` view](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets-schemata):

<br />


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     * EXCEPT (schema_owner)
   FROM
     PROJECT_ID.`region-REGION`.INFORMATION_SCHEMA.SCHEMATA;
   ```


   Replace the following:
   - `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.
   - `REGION`: any [dataset region name](https://docs.cloud.google.com/bigquery/docs/locations). For example, `us`.

   <br />

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

You can also query the [`INFORMATION_SCHEMA.SCHEMATA_OPTIONS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets-schemata-options).

```googlesql
SELECT
  *
FROM
  PROJECT_ID.`region-REGION`.INFORMATION_SCHEMA.SCHEMATA_OPTIONS;
```

### bq

Issue the `bq show` command. The `--format` flag can be used to control the
output. If you are getting information about a dataset in a project other
than your default project, add the project ID to the dataset name in the
following format: `project_id:dataset`.
The output displays the dataset's information such as access
control, labels, and location. This command doesn't display a dataset's
inherited permissions, but you can see them in the Google Cloud console.

To show information about a
[hidden dataset](https://docs.cloud.google.com/bigquery/docs/datasets#hidden_datasets),
use the [`bq ls --all`](https://docs.cloud.google.com/bigquery/docs/listing-datasets)
command to list all datasets and then use the name of the hidden dataset
in the `bq show` command.

```bash
bq show --format=prettyjson project_id:dataset
```

Replace the following:

- <var translate="no">project_id</var> is the name of your project.
- <var translate="no">dataset</var> is the name of the dataset.

Examples:

Enter the following command to display information about `mydataset` in your
default project.

    bq show --format=prettyjson mydataset

Enter the following command to display information about `mydataset` in
`myotherproject`.

    bq show --format=prettyjson myotherproject:mydataset

Enter the following command to display information about the hidden dataset
`_1234abcd56efgh78ijkl1234` in your default project.

    bq show --format=prettyjson _1234abcd56efgh78ijkl1234

### API

Call the [`datasets.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get)
API method and provide any relevant parameters.

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

    // printDatasetInfo demonstrates fetching dataset metadata and printing some of it to an io.Writer.
    func printDatasetInfo(w io.Writer, projectID, datasetID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	meta, err := client.Dataset(datasetID).Metadata(ctx)
    	if err != nil {
    		return err
    	}

    	fmt.Fprintf(w, "Dataset ID: %s\n", datasetID)
    	fmt.Fprintf(w, "Description: %s\n", meta.Description)
    	fmt.Fprintln(w, "Labels:")
    	for k, v := range meta.Labels {
    		fmt.Fprintf(w, "\t%s: %s", k, v)
    	}
    	fmt.Fprintln(w, "Tables:")
    	it := client.Dataset(datasetID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Dataset_Tables(ctx)

    	cnt := 0
    	for {
    		t, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		cnt++
    		fmt.Fprintf(w, "\t%s\n", t.TableID)
    	}
    	if cnt == 0 {
    		fmt.Fprintln(w, "\tThis dataset does not contain any tables.")
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

    import com.google.api.gax.paging.https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.paging.Page.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.TableListOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;

    public class GetDatasetInfo {

      public static void runGetDatasetInfo() {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        getDatasetInfo(projectId, datasetName);
      }

      public static void getDatasetInfo(String projectId, String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html datasetId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html.of(projectId, datasetName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html dataset = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getDataset_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetOption____(datasetId);

          // View dataset properties
          String description = dataset.getDescription();
          System.out.println(description);

          // View tables in the dataset
          // For more information on listing tables see:
          // https://javadoc.io/static/com.google.cloud/google-cloud-bigquery/0.22.0-beta/com/google/cloud/bigquery/BigQuery.html
          Page<Table> tables = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_listTables_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_TableListOption____(datasetName, TableListOption.pageSize(100));

          tables.iterateAll().forEach(table -> System.out.print(table.getTableId().getTable() + "\n"));

          System.out.println("Dataset info retrieved successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Dataset info not retrieved. \n" + e.toString());
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

    async function getDataset() {
      // Retrieves dataset named "my_dataset".

      /**
       * TODO(developer): Uncomment the following lines before running the sample
       */
      // const datasetId = "my_dataset";

      // Retrieve dataset reference
      const [dataset] = await bigquery.dataset(datasetId).get();

      console.log('Dataset:');
      console.log(dataset.metadata.datasetReference);
    }
    getDataset();

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

    # TODO(developer): Set dataset_id to the ID of the dataset to fetch.
    # dataset_id = 'your-project.your_dataset'

    dataset = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_dataset(dataset_id)  # Make an API request.

    full_dataset_id = "{}.{}".format(dataset.project, dataset.dataset_id)
    friendly_name = dataset.friendly_name
    print(
        "Got dataset '{}' with friendly_name '{}'.".format(
            full_dataset_id, friendly_name
        )
    )

    # View dataset properties.
    print("Description: {}".format(dataset.description))
    print("Labels:")
    labels = dataset.labels
    if labels:
        for label, value in labels.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Row.html#google_cloud_bigquery_table_Row_items():
            print("\t{}: {}".format(label, value))
    else:
        print("\tDataset has no labels defined.")

    # View tables in dataset.
    print("Tables:")
    tables = list(client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_tables(dataset))  # Make an API request(s).
    if tables:
        for table in tables:
            print("\t{}".format(table.table_id))
    else:
        print("\tThis dataset does not contain any tables.")

## Verify the dataset name

The following samples show how to check if a dataset exists:

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html;

    // Sample to check dataset exist
    public class DatasetExists {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        datasetExists(datasetName);
      }

      public static void datasetExists(String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html dataset = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getDataset_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetOption____(DatasetId.of(datasetName));
          if (dataset != null) {
            System.out.println("Dataset already exists.");
          } else {
            System.out.println("Dataset not found.");
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Something went wrong. \n" + e.toString());
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

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest
    from google.cloud.exceptions import NotFound

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set dataset_id to the ID of the dataset to determine existence.
    # dataset_id = "your-project.your_dataset"

    try:
        client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_dataset(dataset_id)  # Make an API request.
        print("Dataset {} already exists".format(dataset_id))
    except NotFound:
        print("Dataset {} is not found".format(dataset_id))

<br />

## What's next

- For more information on creating datasets, see [Creating datasets](https://docs.cloud.google.com/bigquery/docs/datasets).
- For more information on assigning access controls to datasets, see [Controlling access to datasets](https://docs.cloud.google.com/bigquery/docs/dataset-access-controls).
- For more information on changing dataset properties, see [Updating dataset properties](https://docs.cloud.google.com/bigquery/docs/updating-datasets).
- For more information on creating and managing labels, see [Creating and managing labels](https://docs.cloud.google.com/bigquery/docs/labels).
- To see an overview of `INFORMATION_SCHEMA`, go to [Introduction to BigQuery `INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro).