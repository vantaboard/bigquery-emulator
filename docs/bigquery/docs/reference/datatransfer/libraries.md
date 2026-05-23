# BigQuery Data Transfer Service API Client Libraries

This page shows how to get started with the Cloud Client Libraries for the
BigQuery Data Transfer API. Client libraries make it easier to access
Google Cloud APIs from a supported language. Although you can use
Google Cloud APIs directly by making raw requests to the server, client
libraries provide simplifications that significantly reduce the amount of code
you need to write.

Read more about the Cloud Client Libraries
and the older Google API Client Libraries in
[Client libraries explained](https://docs.cloud.google.com/apis/docs/client-libraries-explained).

## Install the client library

### C#

```
Install-Package Google.Cloud.BigQuery.DataTransfer.V1 -Pre
```

For more information, see [Setting Up a C# Development Environment](https://docs.cloud.google.com/dotnet/docs/setup).

### Go

```
go get cloud.google.com/go/bigquery/datatransfer/apiv1
```

For more information, see [Setting Up a Go Development Environment](https://docs.cloud.google.com/go/docs/setup).

### Java

If you are using [Maven](https://maven.apache.org/), add
the following to your `pom.xml` file. For more information about
BOMs, see [The Google Cloud Platform Libraries BOM](https://cloud.google.com/java/docs/bom).

    <dependencyManagement>
      <dependencies>
        <dependency>
          <groupId>com.google.cloud</groupId>
          <artifactId>libraries-bom</artifactId>
          <version>26.72.0</version>
          <type>pom</type>
          <scope>import</scope>
        </dependency>
      </dependencies>
    </dependencyManagement>

    <dependencies>
      <dependency>
        <groupId>com.google.cloud</groupId>
        <artifactId>google-cloud-bigquerydatatransfer</artifactId>
      </dependency>
    </dependencies>

If you are using [Gradle](https://gradle.org/),
add the following to your dependencies:

    implementation 'com.google.cloud:google-cloud-bigquerydatatransfer:2.80.0'

If you are using [sbt](https://www.scala-sbt.org/), add
the following to your dependencies:

    libraryDependencies += "com.google.cloud" % "google-cloud-bigquerydatatransfer" % "2.80.0"

If you're using Visual Studio Code or IntelliJ, you can add client libraries to your
project using the following IDE plugins:

- [Cloud Code for VS Code](https://docs.cloud.google.com/code/docs/vscode/client-libraries)
- [Cloud Code for IntelliJ](https://docs.cloud.google.com/code/docs/intellij/client-libraries)

The plugins provide additional functionality, such as key management for service accounts. Refer
to each plugin's documentation for details.

> [!NOTE]
> **Note:** Cloud Java client libraries do not currently support Android.

For more information, see [Setting Up a Java Development Environment](https://docs.cloud.google.com/java/docs/setup).

### Node.js

```
npm install @google-cloud/bigquery-data-transfer
```

For more information, see [Setting Up a Node.js Development Environment](https://docs.cloud.google.com/nodejs/docs/setup).

### PHP

```
composer require google/cloud-bigquerydatatransfer
```

For more information, see [Using PHP on Google Cloud](https://docs.cloud.google.com/php/docs).

### Python

```
pip install --upgrade google-cloud-bigquery-datatransfer
```

For more information, see [Setting Up a Python Development Environment](https://docs.cloud.google.com/python/docs/setup).

### Ruby

```
gem install google-cloud-bigquery-data_transfer
```

For more information, see [Setting Up a Ruby Development Environment](https://docs.cloud.google.com/ruby/docs/setup).

<br />

## Set up authentication

To authenticate calls to Google Cloud APIs, client libraries support [Application Default Credentials (ADC)](https://docs.cloud.google.com/docs/authentication/application-default-credentials); the libraries look for credentials in a set of defined locations and use those credentials to authenticate requests to the API. With ADC, you can make credentials available to your application in a variety of environments, such as local development or production, without needing to modify your application code.

For production environments, the way you set up ADC depends on the service
and context. For more information, see [Set up Application Default Credentials](https://docs.cloud.google.com/docs/authentication/provide-credentials-adc).

For a local development environment, you can set up ADC with the credentials
that are associated with your Google Account:

1.
   [Install](https://docs.cloud.google.com/sdk/docs/install) the Google Cloud CLI.

   After installation,
   [initialize](https://docs.cloud.google.com/sdk/docs/initializing) the Google Cloud CLI by running the following command:

   ```bash
   gcloud init
   ```


   If you're using an external identity provider (IdP), you must first
   [sign in to the gcloud CLI with your federated identity](https://docs.cloud.google.com/iam/docs/workforce-log-in-gcloud).
2.

   If you're using a local shell, then create local authentication credentials for your user
   account:

   ```bash
   gcloud auth application-default login
   ```

   You don't need to do this if you're using Cloud Shell.


   If an authentication error is returned, and you are using an external identity provider
   (IdP), confirm that you have
   [signed in to the gcloud CLI with your federated identity](https://docs.cloud.google.com/iam/docs/workforce-log-in-gcloud).


   A sign-in screen appears. After you sign in, your credentials are stored in the
   [local credential file used by ADC](https://docs.cloud.google.com/docs/authentication/application-default-credentials#personal).

## Use the client library


The following example shows how to use the client library.

### C#


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.ResourceNames.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest/Google.Cloud.BigQuery.DataTransfer.V1.html;
    using System;

    namespace GoogleCloudSamples
    {
        public class QuickStart
        {
            public static void Main(string[] args)
            {
                // Instantiates a client
                https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest/Google.Cloud.BigQuery.DataTransfer.V1.DataTransferServiceClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest/Google.Cloud.BigQuery.DataTransfer.V1.DataTransferServiceClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest/Google.Cloud.BigQuery.DataTransfer.V1.DataTransferServiceClient.html#Google_Cloud_BigQuery_DataTransfer_V1_DataTransferServiceClient_Create();

                // Your Google Cloud Platform project ID
                string projectId = "YOUR-PROJECT-ID";

                https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.ResourceNames.ProjectName.html project = https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.ResourceNames.ProjectName.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.ResourceNames.ProjectName.html#Google_Api_Gax_ResourceNames_ProjectName_FromProject_System_String_(projectId);
                var sources = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest/Google.Cloud.BigQuery.DataTransfer.V1.DataTransferServiceClient.html#Google_Cloud_BigQuery_DataTransfer_V1_DataTransferServiceClient_ListDataSources_Google_Api_Gax_ResourceNames_LocationName_System_String_System_Nullable_System_Int32__Google_Api_Gax_Grpc_CallSettings_(project);
                Console.WriteLine("Supported Data Sources:");
                foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest/Google.Cloud.BigQuery.DataTransfer.V1.DataSource.html source in sources)
                {
                    Console.WriteLine(
                        $"{source.DataSourceId}: " +
                        $"{source.DisplayName} ({source.Description})");
                }
            }
        }
    }

### Go


    // Sample bigquery-quickstart creates a Google BigQuery dataset.
    package main

    import (
    	"fmt"
    	"log"

    	"golang.org/x/net/context"
    	"google.golang.org/api/iterator"

    	// Imports the BigQuery Data Transfer client package.
    	datatransfer "cloud.google.com/go/bigquery/datatransfer/apiv1"
    	datatransferpb "google.golang.org/genproto/googleapis/cloud/bigquery/datatransfer/v1"
    )

    func main() {
    	ctx := context.Background()

    	// Sets your Google Cloud Platform project ID.
    	projectID := "YOUR_PROJECT_ID"

    	// Creates a client.
    	client, err := datatransfer.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/datatransfer/apiv1.html#cloud_google_com_go_bigquery_datatransfer_apiv1_Client_NewClient(ctx)
    	if err != nil {
    		log.Fatalf("Failed to create client: %v", err)
    	}

    	req := &datatransferpb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/datatransfer/apiv1/datatransferpb.html#cloud_google_com_go_bigquery_datatransfer_apiv1_datatransferpb_ListDataSourcesRequest{
    		Parent: fmt.Sprintf("projects/%s", projectID),
    	}
    	it := client.ListDataSources(ctx, req)
    	fmt.Println("Supported Data Sources:")
    	for {
    		ds, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			log.Fatalf("Failed to list sources: %v", err)
    		}
    		fmt.Println(ds.DisplayName)
    		fmt.Println("\tID: ", ds.DataSourceId)
    		fmt.Println("\tFull path: ", ds.Name)
    		fmt.Println("\tDescription: ", ds.Description)
    	}
    }

### Java

    // Imports the Google Cloud client library

    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataSource.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.ListDataSourcesPagedResponse.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ListDataSourcesRequest.html;

    public class QuickstartSample {
      /** List available data sources for the BigQuery Data Transfer service. */
      public static void main(String... args) throws Exception {
        // Sets your Google Cloud Platform project ID.
        // String projectId = "YOUR_PROJECT_ID";
        String projectId = args[0];

        // Instantiate a client. If you don't specify credentials when constructing a client, the
        // client library will look for credentials in the environment, such as the
        // GOOGLE_APPLICATION_CREDENTIALS environment variable.
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          // Request the list of available data sources.
          String parent = String.format("projects/%s", projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ListDataSourcesRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ListDataSourcesRequest.html.newBuilder().setParent(parent).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.ListDataSourcesPagedResponse.html response = client.listDataSources(request);

          // Print the results.
          System.out.println("Supported Data Sources:");
          for (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataSource.html dataSource : response.iterateAll()) {
            System.out.println(dataSource.getDisplayName());
            System.out.printf("\tID: %s%n", dataSource.getDataSourceId());
            System.out.printf("\tFull path: %s%n", dataSource.getName());
            System.out.printf("\tDescription: %s%n", dataSource.getDescription());
          }
        }
      }
    }

### Node.js

    const bigqueryDataTransfer = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest/overview.html');
    const client = new bigqueryDataTransfer.v1.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest/overview.html();

    async function quickstart() {
      const projectId = await client.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest/bigquery-data-transfer/v1.datatransferserviceclient.html();

      // Iterate over all elements.
      const formattedParent = client.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest/bigquery-data-transfer/v1.datatransferserviceclient.html(projectId, 'us-central1');
      let nextRequest = {parent: formattedParent};
      const options = {autoPaginate: false};
      console.log('Data sources:');
      do {
        // Fetch the next page.
        const responses = await client.listDataSources(nextRequest, options);
        // The actual resources in a response.
        const resources = responses[0];
        // The next request if the response shows that there are more responses.
        nextRequest = responses[1];
        // The actual response object, if necessary.
        // const rawResponse = responses[2];
        resources.forEach(resource => {
          console.log(`  ${resource.name}`);
        });
      } while (nextRequest);

      console.log('\n\n');
      console.log('Sources via stream:');

      client
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest/bigquery-data-transfer/v1.datatransferserviceclient.html({parent: formattedParent})
        .on('data', element => {
          console.log(`  ${element.name}`);
        });
    }
    quickstart();

### PHP

    # Includes the autoloader for libraries installed with composer
    require __DIR__ . '/vendor/autoload.php';

    # Imports the Google Cloud client library
    use Google\Cloud\BigQuery\DataTransfer\V1\DataTransferServiceClient;

    # Instantiates a client
    $bqdtsClient = new DataTransferServiceClient();

    # Your Google Cloud Platform project ID
    $projectId = 'YOUR_PROJECT_ID';
    $parent = sprintf('projects/%s/locations/us', $projectId);

    try {
        echo 'Supported Data Sources:', PHP_EOL;
        $pagedResponse = $bqdtsClient->listDataSources($parent);
        foreach ($pagedResponse->iterateAllElements() as $dataSource) {
            echo 'Data source: ', $dataSource->getDisplayName(), PHP_EOL;
            echo 'ID: ', $dataSource->getDataSourceId(), PHP_EOL;
            echo 'Full path: ', $dataSource->getName(), PHP_EOL;
            echo 'Description: ', $dataSource->getDescription(), PHP_EOL;
        }
    } finally {
        $bqdtsClient->close();
    }

### Python

    from google.cloud import bigquery_datatransfer

    client = bigquery_datatransfer.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html()

    # TODO: Update to your project ID.
    project_id = "my-project"

    # Get the full path to your project.
    parent = client.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html#google_cloud_bigquery_datatransfer_v1_services_data_transfer_service_DataTransferServiceClient_common_project_path(project_id)

    print("Supported Data Sources:")

    # Iterate over all possible data sources.
    for data_source in client.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html#google_cloud_bigquery_datatransfer_v1_services_data_transfer_service_DataTransferServiceClient_list_data_sources(parent=parent):
        print("{}:".format(data_source.display_name))
        print("\tID: {}".format(data_source.data_source_id))
        print("\tFull path: {}".format(data_source.name))
        print("\tDescription: {}".format(data_source.description))

### Ruby

    # Imports the Google Cloud client library
    require "google/cloud/bigquery/data_transfer"

    # Your Google Cloud Platform project ID
    # project_id = "YOUR_PROJECT_ID"

    # Instantiate a client
    data_transfer = Google::Cloud::Bigquery::DataTransfer.data_transfer_service

    # Get the full path to your project.
    project_path = data_transfer.project_path project: project_id

    puts "Supported Data Sources:"

    # Iterate over all possible data sources.
    data_transfer.list_data_sources(parent: project_path).each do |data_source|
      puts "Data source: #{data_source.display_name}"
      puts "ID: #{data_source.data_source_id}"
      puts "Full path: #{data_source.name}"
      puts "Description: #{data_source.description}"
    end

<br />

## Additional resources

### C#

The following list contains links to more resources related to the
client library for C#:

- [API reference](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.DataTransfer.V1/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-dotnet/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bc%23%5D)
- [Source code](https://github.com/googleapis/google-cloud-dotnet)

### Go

The following list contains links to more resources related to the
client library for Go:

- [API reference](https://godoc.org/cloud.google.com/go/bigquery/datatransfer/apiv1)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-go/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bgo%5D)
- [Source code](https://github.com/googleapis/google-cloud-go)

### Java

The following list contains links to more resources related to the
client library for Java:

- [API reference](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/overview)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-java/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bjava%5D)
- [Source code](https://github.com/googleapis/google-cloud-java)

### Node.js

The following list contains links to more resources related to the
client library for Node.js:

- [API reference](https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-transfer/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-node/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bnode.js%5D)
- [Source code](https://github.com/googleapis/google-cloud-node)

### PHP

The following list contains links to more resources related to the
client library for PHP:

- [API reference](https://docs.cloud.google.com/php/docs/reference/cloud-bigquerydatatransfer/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-php/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bphp%5D)
- [Source code](https://github.com/googleapis/google-cloud-php)

### Python

The following list contains links to more resources related to the
client library for Python:

- [API reference](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-python/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bpython%5D)
- [Source code](https://github.com/googleapis/google-cloud-python)

### Ruby

The following list contains links to more resources related to the
client library for Ruby:

- [API reference](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-data_transfer/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-ruby/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bruby%5D)
- [Source code](https://github.com/googleapis/google-cloud-ruby)

<br />