# Filtering resources using labels

To filter resources using labels, you can do one of the following:

- Use the search bar in the Google Cloud console.
- Create a filter specification for use in the API, bq command-line tool, or client libraries.

## Limitations

- The API, bq command-line tool, and client libraries support filtering only for datasets.
- You cannot filter jobs by label in any of the BigQuery tools.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document.

### Required permissions

To filter resources using labels, you must be able to retrieve resource
metadata. To filter resources using labels, you need the following IAM permissions:

- `bigquery.datasets.get` (lets you filter datasets)
- `bigquery.tables.get` (lets you filter tables and views)

Each of the following predefined IAM roles includes the permissions that you need in order to filter datasets:

- `roles/bigquery.user`
- `roles/bigquery.metadataViewer`
- `roles/bigquery.dataViewer`
- `roles/bigquery.dataOwner`
- `roles/bigquery.dataEditor`
- `roles/bigquery.admin`

Each of the following predefined IAM roles includes the permissions that you need in order to filter tables and views:

- `roles/bigquery.metadataViewer`
- `roles/bigquery.dataViewer`
- `roles/bigquery.dataOwner`
- `roles/bigquery.dataEditor`
- `roles/bigquery.admin`

Additionally, if you have the `bigquery.datasets.create` permission, you can filter the resources that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Filter resources in the Google Cloud console

To generate a filtered list of resources, use the Google Cloud console:

1. In the Google Cloud console, go to the **Explorer** pane.

2. In the search bar, enter the `key` or `key:value` pair. Your
   results include any partial matches.

   For example, to show only datasets with the label `department:shipping`,
   you can enter `department` or `department:shipping`.

## Filter datasets in the API or bq command-line tool

The API, bq command-line tool, and client libraries support filtering only for
datasets.

To filter datasets by using the API, bq tool, or client libraries,
create a filter specification and use the specification:

- As the parameter for the `--filter` flag in the bq tool
- As the value for the `filter` property in the API's `datasets.list` method

### Limitations on filter specifications

Filter specifications have the following limitations:

- Only the `AND` logical operator is supported. Space-separated comparisons are treated as having implicit `AND` operators.
- The only field eligible for filtering is `labels.key` where `key` is the name of a label.
- Each `key` in a filtering expression must be unique.
- The filter can include up to ten expressions.
- Filtering is case-sensitive.
- The API, bq command-line tool, and client libraries support filtering only for datasets.

### Filter specification examples

A filter specification uses the following syntax:

`"field[:value][ field[:value]]..."`

Replace the following:

- `field` is expressed as `labels.key` where <var translate="no">key</var> is a label key.
- `value` is an optional label value.

The following examples show how to generate filter expressions.

To list resources that have a `department:shipping` label, use the following
filter specification:

`labels.department:shipping`

To list resources using multiple labels, separate the `key:value` pairs with a
space. The space is treated as a logical `AND` operator. For example, to list
datasets with the `department:shipping` label and the `location:usa` label,
use the following filter specification:

`labels.department:shipping labels.location:usa`

You can filter on the presence of a key alone, rather than matching against a
key:value pair. The following filter specification lists all datasets
labeled `department` regardless of the value.

`labels.department`

An equivalent filter specification uses an asterisk to represent all possible
values associated with the `department` key.

`labels.department:*`

You can also use tags in a filter specification. For example, to list resources
with the `department:shipping` label and `test_data` tag, use the following
filter specification:

`labels.department:shipping labels.test_data`

### Filtering datasets in the bq command-line tool and the API

To filter datasets by using the API, bq command-line tool, or client libraries:

### bq

Issue the `bq ls` command with the `--filter` flag. If you are listing
datasets in a project other than your default project, specify the
`--project_id` flag.

```bash
bq ls \
--filter "filter_specification" \
--project_id project_id
```

Replace the following:

- `filter_specification` is a valid filter specification.
- `project_id` is your project ID.

Examples:

Enter the following command to list datasets in your default project that
have a `department:shipping` label:

    bq ls --filter "labels.department:shipping"

Enter the following command to list datasets in your default project that
have a `department:shipping` label and a `test_data` tag.

    bq ls --filter "labels.department:shipping labels.test_data"

Enter the following command to list datasets in `myotherproject` that have
a `department:shipping` label:

    bq ls --filter "labels.department:shipping" --project_id myotherproject

The output for each of these commands returns a list of datasets like the
following.

```
+---+
| datasetId |
+---+
| mydataset |
| mydataset2|
+---+
```

<br />

### API

Call the [`datasets.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list)
API method and provide the filter specification using the `filter` property.

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

    // listDatasetsByLabel demonstrates walking the collection of datasets in a project, and
    // filtering that list to a subset that has specific label metadata.
    func listDatasetsByLabel(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	it := client.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Client_Datasets(ctx)
    	it.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/migration/apiv2alpha/migrationpb.html#cloud_google_com_go_bigquery_migration_apiv2alpha_migrationpb_Filter = "labels.color:green"
    	for {
    		dataset, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		fmt.Fprintf(w, "dataset: %s\n", dataset.DatasetID)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html;

    // Sample to get list of datasets by label
    public class ListDatasetsByLabel {

      public static void runListDatasetsByLabel() {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String filter = "MY_LABEL_FILTER";
        listDatasetsByLabel(projectId, filter);
      }

      public static void listDatasetsByLabel(String projectId, String filter) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          Page<Dataset> datasets =
              bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_listDatasets_com_google_cloud_bigquery_BigQuery_DatasetListOption____(
                  projectId,
                  BigQuery.DatasetListOption.pageSize(100),
                  BigQuery.DatasetListOption.labelFilter(filter)); // "labels.color:green"
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

    async function listDatasetsByLabel() {
      // Lists all datasets in current GCP project, filtering by label color:green.

      const options = {
        filter: 'labels.color:green',
      };
      // Lists all datasets in the specified project
      const [datasets] = await bigquery.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html(options);

      console.log('Datasets:');
      datasets.forEach(dataset => console.log(dataset.id));
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

    label_filter = "labels.color:green"
    datasets = list(client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_datasets(filter=label_filter))  # Make an API request.

    if datasets:
        print("Datasets filtered by {}:".format(label_filter))
        for dataset in datasets:
            print("\t{}.{}".format(dataset.project, dataset.dataset_id))
    else:
        print("No datasets found with this filter.")

## What's next

- Learn how to [add labels](https://docs.cloud.google.com/bigquery/docs/adding-labels) to BigQuery resources.
- Learn how to [identify and analyze agent-generated queries using labels](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#agent-queries).
- Learn how to [view labels](https://docs.cloud.google.com/bigquery/docs/viewing-labels) on BigQuery resources.
- Learn how to [update labels](https://docs.cloud.google.com/bigquery/docs/updating-labels) on BigQuery resources.
- Learn how to [delete labels](https://docs.cloud.google.com/bigquery/docs/deleting-labels) on BigQuery resources.
- Read about [using labels](https://docs.cloud.google.com/resource-manager/docs/using-labels) in the Resource Manager documentation.