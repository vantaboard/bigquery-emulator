This page shows how to get started with the Cloud Client Libraries for the
Analytics Hub API. Client libraries make it easier to access
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
Install-Package Google.Cloud.BigQuery.AnalyticsHub.V1 -Pre
```

For more information, see [Setting Up a C# Development Environment](https://docs.cloud.google.com/dotnet/docs/setup).

### Go

```
go get cloud.google.com/go/bigquery
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
          <version>26.80.0</version>
          <type>pom</type>
          <scope>import</scope>
        </dependency>
      </dependencies>
    </dependencyManagement>

    <dependencies>
      <dependency>
        <groupId>com.google.cloud</groupId>
        <artifactId>google-cloud-analyticshub</artifactId>
      </dependency>
    </dependencies>

If you are using [Gradle](https://gradle.org/),
add the following to your dependencies:

    implementation 'com.google.cloud:google-cloud-analyticshub:0.88.0'

If you are using [sbt](https://www.scala-sbt.org/), add
the following to your dependencies:

    libraryDependencies += "com.google.cloud" % "google-cloud-analyticshub" % "0.88.0"

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
npm install @google-cloud/bigquery-data-exchange
```

For more information, see [Setting Up a Node.js Development Environment](https://docs.cloud.google.com/nodejs/docs/setup).

### PHP

```
composer require google/cloud-bigquery-analyticshub
```

For more information, see [Using PHP on Google Cloud](https://docs.cloud.google.com/php/docs).

### Python

```
pip install --upgrade google-cloud-bigquery-analyticshub
```

For more information, see [Setting Up a Python Development Environment](https://docs.cloud.google.com/python/docs/setup).

### Ruby

```
gem install google-cloud-bigquery-analytics_hub-v1
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


The following example demonstrates some basic interactions with Sharing.

<br />

### Go

    // Copyright 2022 Google LLC
    //
    // Licensed under the Apache License, Version 2.0 (the "License");
    // you may not use this file except in compliance with the License.
    // You may obtain a copy of the License at
    //
    //     https://www.apache.org/licenses/LICENSE-2.0
    //
    // Unless required by applicable law or agreed to in writing, software
    // distributed under the License is distributed on an "AS IS" BASIS,
    // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    // See the License for the specific language governing permissions and
    // limitations under the License.


    // The analyticshub quickstart application demonstrates usage of the
    // Analytics hub API by creating an example data exchange and listing.
    package main

    import (
    	"context"
    	"flag"
    	"fmt"
    	"log"

    	analyticshub "cloud.google.com/go/bigquery/analyticshub/apiv1"
    	"cloud.google.com/go/bigquery/analyticshub/apiv1/analyticshubpb"
    	"google.golang.org/grpc/codes"
    	"google.golang.org/grpc/status"
    )

    func main() {

    	// Define the command line flags for controlling the behavior of this quickstart.
    	var (
    		projectID            = flag.String("project_id", "", "Cloud Project ID, used for session creation.")
    		location             = flag.String("location", "US", "BigQuery location used for interactions.")
    		exchangeID           = flag.String("exchange_id", "ExampleDataExchange", "identifier of the example data exchange")
    		listingID            = flag.String("listing_id", "ExampleDataExchange", "identifier of the example data exchange")
    		exampleDatasetSource = flag.String("dataset_source", "", "dataset source in the form projects/myproject/datasets/mydataset")
    		delete               = flag.Bool("delete_exchange", true, "delete exchange at the end of quickstart")
    	)
    	flag.Parse()
    	// Perform simple validation of the specified flags.
    	if *projectID == "" {
    		log.Fatal("empty --project_id specified, please provide a valid project ID")
    	}
    	if *exampleDatasetSource == "" {
    		log.Fatalf("empty --dataset_source specified, please provide in the form \"projects/myproject/datasets/mydataset\"")
    	}

    	// Instantiate the client.
    	ctx := context.Background()
    	ahubClient, err := analyticshub.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/analyticshub/apiv1.html#cloud_google_com_go_bigquery_analyticshub_apiv1_Client_NewClient(ctx)
    	if err != nil {
    		log.Fatalf("NewClient: %v", err)
    	}
    	defer ahubClient.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/analyticshub/apiv1.html#cloud_google_com_go_bigquery_analyticshub_apiv1_Client_Close()

    	// Then, create the data exchange (or return information about one already bearing the example name), and
    	// print information about it.
    	exchange, err := createOrGetDataExchange(ctx, ahubClient, *projectID, *location, *exchangeID)
    	if err != nil {
    		log.Fatalf("failed to get information about the exchange: %v", err)
    	}
    	fmt.Printf("\nData Exchange Information\n")
    	fmt.Printf("Exchange Name: %s\n", exchange.GetName())
    	if desc := exchange.GetDescription(); desc != "" {
    		fmt.Printf("Exchange Description: %s", desc)
    	}

    	// Finally, create a listing within the data exchange and print information about it.
    	listing, err := createListing(ctx, ahubClient, *projectID, *location, *exchangeID, *listingID, *exampleDatasetSource)
    	if err != nil {
    		log.Fatalf("failed to create the listing within the exchange: %v", err)
    	}
    	fmt.Printf("\n\nListing Information\n")
    	fmt.Printf("Listing Name: %s\n", listing.GetName())
    	if desc := listing.GetDescription(); desc != "" {
    		fmt.Printf("Listing Description: %s\n", desc)
    	}
    	fmt.Printf("Listing State: %s\n", listing.GetState().String())
    	if source := listing.GetSource(); source != nil {
    		if dsSource, ok := source.(*analyticshubpb.Listing_BigqueryDataset); ok && dsSource.BigqueryDataset != nil {
    			if dataset := dsSource.BigqueryDataset.GetDataset(); dataset != "" {
    				fmt.Printf("Source is a bigquery dataset: %s", dataset)
    			}
    		}
    	}
    	// Optionally, delete the data exchange at the end of the quickstart to clean up the resources used.
    	if *delete {
    		fmt.Printf("\n\n")
    		if err := deleteDataExchange(ctx, ahubClient, *projectID, *location, *exchangeID); err != nil {
    			log.Fatalf("failed to delete exchange: %v", err)
    		}
    		fmt.Printf("Exchange projects/%s/locations/%s/dataExchanges/%s was deleted.\n", *projectID, *location, *exchangeID)
    	}
    	fmt.Printf("\nQuickstart completed.\n")
    }

    // createOrGetDataExchange creates an example data exchange, or returns information about the exchange already bearing
    // the example identifier.
    func createOrGetDataExchange(ctx context.Context, client *analyticshub.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/analyticshub/apiv1.html#cloud_google_com_go_bigquery_analyticshub_apiv1_Client, projectID, location, exchangeID string) (*analyticshubpb.DataExchange, error) {
    	req := &analyticshubpb.CreateDataExchangeRequest{
    		Parent:         fmt.Sprintf("projects/%s/locations/%s", projectID, location),
    		DataExchangeId: exchangeID,
    		DataExchange: &analyticshubpb.DataExchange{
    			DisplayName:    "Example Data Exchange",
    			Description:    "Exchange created as part of an API quickstart",
    			PrimaryContact: "",
    			Documentation:  "https://link.to.optional.documentation/",
    		},
    	}

    	resp, err := client.CreateDataExchange(ctx, req)
    	if err != nil {
    		// We'll handle one specific error case specially, the case of the exchange already existing.  In this instance,
    		// we'll issue a second request to fetch the exchange information for the already present exchange and return it.
    		if code := status.Code(err); code == codes.AlreadyExists {
    			getReq := &analyticshubpb.GetDataExchangeRequest{
    				Name: fmt.Sprintf("projects/%s/locations/%s/dataExchanges/%s", projectID, location, exchangeID),
    			}
    			resp, err = client.GetDataExchange(ctx, getReq)
    			if err != nil {
    				return nil, fmt.Errorf("error getting dataExchange: %w", err)
    			}
    			return resp, nil
    		}
    		// For all other cases, return the error from creation request.
    		return nil, err
    	}
    	return resp, nil
    }

    // createListing creates an example listing within the specified exchange using the provided source dataset.
    func createListing(ctx context.Context, client *analyticshub.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/analyticshub/apiv1.html#cloud_google_com_go_bigquery_analyticshub_apiv1_Client, projectID, location, exchangeID, listingID, sourceDataset string) (*analyticshubpb.Listing, error) {
    	req := &analyticshubpb.CreateListingRequest{
    		Parent:    fmt.Sprintf("projects/%s/locations/%s/dataExchanges/%s", projectID, location, exchangeID),
    		ListingId: listingID,
    		Listing: &analyticshubpb.Listing{
    			DisplayName: "Example Exchange Listing",
    			Description: "Example listing created as part of an API quickstart",
    			Categories: []analyticshubpb.Listing_Category{
    				analyticshubpb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/analyticshub/apiv1/analyticshubpb.html#cloud_google_com_go_bigquery_analyticshub_apiv1_analyticshubpb_Listing_CATEGORY_UNSPECIFIED_Listing_CATEGORY_OTHERS_Listing_CATEGORY_ADVERTISING_AND_MARKETING_Listing_CATEGORY_COMMERCE_Listing_CATEGORY_CLIMATE_AND_ENVIRONMENT_Listing_CATEGORY_DEMOGRAPHICS_Listing_CATEGORY_ECONOMICS_Listing_CATEGORY_EDUCATION_Listing_CATEGORY_ENERGY_Listing_CATEGORY_FINANCIAL_Listing_CATEGORY_GAMING_Listing_CATEGORY_GEOSPATIAL_Listing_CATEGORY_HEALTHCARE_AND_LIFE_SCIENCE_Listing_CATEGORY_MEDIA_Listing_CATEGORY_PUBLIC_SECTOR_Listing_CATEGORY_RETAIL_Listing_CATEGORY_SPORTS_Listing_CATEGORY_SCIENCE_AND_RESEARCH_Listing_CATEGORY_TRANSPORTATION_AND_LOGISTICS_Listing_CATEGORY_TRAVEL_AND_TOURISM_Listing_CATEGORY_GOOGLE_EARTH_ENGINE,
    			},
    			Source: &analyticshubpb.Listing_BigqueryDataset{
    				BigqueryDataset: &analyticshubpb.Listing_BigQueryDatasetSource{
    					Dataset: sourceDataset,
    				},
    			},
    		},
    	}
    	return client.CreateListing(ctx, req)
    }

    // deleteDataExchange deletes a data exchange.
    func deleteDataExchange(ctx context.Context, client *analyticshub.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/analyticshub/apiv1.html#cloud_google_com_go_bigquery_analyticshub_apiv1_Client, projectID, location, exchangeID string) error {
    	req := &analyticshubpb.DeleteDataExchangeRequest{
    		Name: fmt.Sprintf("projects/%s/locations/%s/dataExchanges/%s", projectID, location, exchangeID),
    	}
    	return client.DeleteDataExchange(ctx, req)
    }

### Node.js

    /**
     * TODO(developer): Uncomment these variables before running the sample.
     */
    /**
     *  Required. The parent resource path of the DataExchanges.
     *  e.g. `projects/myproject/locations/US`.
     */
    // const parent = 'abc123'
    /**
     *  The maximum number of results to return in a single response page. Leverage
     *  the page tokens to iterate through the entire collection.
     */
    // const pageSize = 1234
    /**
     *  Page token, returned by a previous call, to request the next page of
     *  results.
     */
    // const pageToken = 'abc123'

    // Imports the Dataexchange library
    const {AnalyticsHubServiceClient} =
      require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-exchange/latest/overview.html').v1beta1;

    // Instantiates a client
    const dataexchangeClient = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-exchange/latest/overview.html();

    async function callListDataExchanges() {
      // Construct request
      const request = {
        parent,
      };

      // Run request
      const iterable = await dataexchangeClient.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-data-exchange/latest/bigquery-data-exchange/v1beta1.analyticshubserviceclient.html(request);
      for await (const response of iterable) {
        console.log(response);
      }
    }

    callListDataExchanges();

<br />

## Additional resources

### C#

The following list contains links to more resources related to the
client library for C#:

- [API reference](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.AnalyticsHub.V1/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-dotnet/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bc%23%5D)
- [Source code](https://github.com/googleapis/google-cloud-dotnet)

### Go

The following list contains links to more resources related to the
client library for Go:

- [API reference](https://pkg.go.dev/cloud.google.com/go/bigquery/analyticshub/apiv1?tab=doc)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-go/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bgo%5D)
- [Source code](https://github.com/googleapis/google-cloud-go)

### Java

The following list contains links to more resources related to the
client library for Java:

- [API reference](https://docs.cloud.google.com/java/docs/reference/google-cloud-analyticshub/latest/overview)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-java/tree/main/java-analyticshub/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bjava%5D)
- [Source code](https://github.com/googleapis/google-cloud-java/tree/main/java-analyticshub)

### Node.js

The following list contains links to more resources related to the
client library for Node.js:

- [API reference](https://googleapis.dev/nodejs/analyticshub/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-node/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bnode.js%5D)
- [Source code](https://github.com/googleapis/google-cloud-node)

### PHP

The following list contains links to more resources related to the
client library for PHP:

- [API reference](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-php-bigquery-analyticshub/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bphp%5D)
- [Source code](https://github.com/googleapis/google-cloud-php-bigquery-analyticshub)

### Python

The following list contains links to more resources related to the
client library for Python:

- [API reference](https://docs.cloud.google.com/python/docs/reference/analyticshub/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-python/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bpython%5D)
- [Source code](https://github.com/googleapis/google-cloud-python)

### Ruby

The following list contains links to more resources related to the
client library for Ruby:

- [API reference](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-analytics_hub/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-ruby/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bruby%5D)
- [Source code](https://github.com/googleapis/google-cloud-ruby)

<br />


### What's next?

For more background, see [Introduction to Sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).

<br />