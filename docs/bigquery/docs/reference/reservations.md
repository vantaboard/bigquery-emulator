This page shows how to get started with the Cloud Client Libraries for the
BigQuery Reservation API. Client libraries make it easier to access
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
Install-Package Google.Cloud.BigQuery.Reservation.V1 -Pre
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
        <artifactId>google-cloud-bigqueryreservation</artifactId>
      </dependency>
    </dependencies>

If you are using [Gradle](https://gradle.org/),
add the following to your dependencies:

    implementation 'com.google.cloud:google-cloud-bigqueryreservation:2.92.0'

If you are using [sbt](https://www.scala-sbt.org/), add
the following to your dependencies:

    libraryDependencies += "com.google.cloud" % "google-cloud-bigqueryreservation" % "2.92.0"

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
npm install @google-cloud/bigquery-reservation
```

For more information, see [Setting Up a Node.js Development Environment](https://docs.cloud.google.com/nodejs/docs/setup).

### PHP

```
composer require google/cloud-bigquery-reservation
```

For more information, see [Using PHP on Google Cloud](https://docs.cloud.google.com/php/docs).

### Python

```
pip install --upgrade google-cloud-bigquery-reservation
```

For more information, see [Setting Up a Python Development Environment](https://docs.cloud.google.com/python/docs/setup).

### Ruby

```
gem install google-cloud-bigquery-reservation
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


The following example demonstrates some basic interactions with the BigQuery Reservation API by enumerating resources, namely reservations and capacity
commitments.

### Go


    // The bigquery_reservation_quickstart application demonstrates usage of the
    // BigQuery reservation API by enumerating some of the resources that can be
    // associated with a cloud project.
    package main

    import (
    	"bytes"
    	"context"
    	"flag"
    	"fmt"
    	"log"

    	reservation "cloud.google.com/go/bigquery/reservation/apiv1"
    	"google.golang.org/api/iterator"
    	reservationpb "google.golang.org/genproto/googleapis/cloud/bigquery/reservation/v1"
    )

    func main() {

    	// Define two command line flags for controlling the behavior of this quickstart.
    	var (
    		projectID = flag.String("project_id", "", "Cloud Project ID, used for session creation.")
    		location  = flag.String("location", "US", "BigQuery location used for interactions.")
    	)
    	// Parse flags and do some minimal validation.
    	flag.Parse()
    	if *projectID == "" {
    		log.Fatal("empty --project_id specified, please provide a valid project ID")
    	}
    	if *location == "" {
    		log.Fatal("empty --location specified, please provide a valid location")
    	}

    	ctx := context.Background()
    	bqResClient, err := reservation.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1.html#cloud_google_com_go_bigquery_reservation_apiv1_Client_NewClient(ctx)
    	if err != nil {
    		log.Fatalf("NewClient: %v", err)
    	}
    	defer bqResClient.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1.html#cloud_google_com_go_bigquery_reservation_apiv1_Client_Close()

    	s, err := reportCapacityCommitments(ctx, bqResClient, *projectID, *location)
    	if err != nil {
    		log.Fatalf("printCapacityCommitments: %v", err)
    	}
    	fmt.Println(s)

    	s, err = reportReservations(ctx, bqResClient, *projectID, *location)
    	if err != nil {
    		log.Fatalf("printReservations: %v", err)
    	}
    	fmt.Println(s)
    }

    // printCapacityCommitments iterates through the capacity commitments and returns a byte buffer with details.
    func reportCapacityCommitments(ctx context.Context, client *reservation.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1.html#cloud_google_com_go_bigquery_reservation_apiv1_Client, projectID, location string) (string, error) {
    	var buf bytes.Buffer
    	fmt.Fprintf(&buf, "Capacity commitments in project %s in location %s:\n", projectID, location)

    	req := &reservationpb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1/reservationpb.html#cloud_google_com_go_bigquery_reservation_apiv1_reservationpb_ListCapacityCommitmentsRequest{
    		Parent: fmt.Sprintf("projects/%s/locations/%s", projectID, location),
    	}
    	totalCommitments := 0
    	it := client.ListCapacityCommitments(ctx, req)
    	for {
    		commitment, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return "", err
    		}
    		fmt.Fprintf(&buf, "\tCommitment %s in state %s\n", commitment.GetName(), commitment.GetState().String())
    		totalCommitments++
    	}
    	fmt.Fprintf(&buf, "\n%d commitments processed.\n", totalCommitments)
    	return buf.String(), nil
    }

    // printReservations iterates through reservations defined in an admin project.
    func reportReservations(ctx context.Context, client *reservation.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1.html#cloud_google_com_go_bigquery_reservation_apiv1_Client, projectID, location string) (string, error) {
    	var buf bytes.Buffer
    	fmt.Fprintf(&buf, "Reservations in project %s in location %s:\n", projectID, location)

    	req := &reservationpb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1/reservationpb.html#cloud_google_com_go_bigquery_reservation_apiv1_reservationpb_ListReservationsRequest{
    		Parent: fmt.Sprintf("projects/%s/locations/%s", projectID, location),
    	}
    	totalReservations := 0
    	it := client.ListReservations(ctx, req)
    	for {
    		reservation, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return "", err
    		}
    		fmt.Fprintf(&buf, "\tReservation %s has %d slot capacity.\n", reservation.GetName(), reservation.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/reservation/apiv1/reservationpb.html#cloud_google_com_go_bigquery_reservation_apiv1_reservationpb_Reservation_GetSlotCapacity())
    		totalReservations++
    	}
    	fmt.Fprintf(&buf, "\n%d reservations processed.\n", totalReservations)
    	return buf.String(), nil
    }

### Java

    import com.google.cloud.bigquery.reservation.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryreservation/latest/com.google.cloud.bigquery.reservation.v1.ReservationServiceClient.html;
    import java.io.IOException;

    public class QuickstartSample {

      public static void main(String... args) throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "YOUR_PROJECT_ID";
        String location = "LOCATION";
        quickStartSample(projectId, location);
      }

      public static void quickStartSample(String projectId, String location) throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryreservation/latest/com.google.cloud.bigquery.reservation.v1.ReservationServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryreservation/latest/com.google.cloud.bigquery.reservation.v1.ReservationServiceClient.html.create()) {
          // list reservations in the project
          String parent = String.format("projects/%s/locations/%s", projectId, location);
          client
              .listReservations(parent)
              .iterateAll()
              .forEach(res -> System.out.println("Reservation resource name: " + res.getName()));

          // list capacity commitments in the project
          client
              .listCapacityCommitments(parent)
              .iterateAll()
              .forEach(
                  commitment ->
                      System.out.println("Capacity commitment resource name: " + commitment.getName()));
        }
      }
    }

### Node.js

    // Imports the Google Cloud client library
    const {
      ReservationServiceClient,
    } = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-reservation/latest/overview.html');

    // Creates a client
    const client = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-reservation/latest/overview.html();

    // project = 'my-project' // Project to list reservations for.
    // location = 'US' // BigQuery location.

    async function listReservations() {
      const [reservations] = await client.listReservations({
        parent: `projects/${project}/locations/${location}`,
      });

      console.info(`found ${reservations.length} reservations`);
      console.info(reservations);
    }

    async function listCapacityCommitments() {
      const [commitments] = await client.listCapacityCommitments({
        parent: `projects/${project}/locations/${location}`,
      });

      console.info(`found ${commitments.length} commitments`);
      console.info(commitments);
    }

    listReservations();
    listCapacityCommitments();

### Python

    import argparse

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest


    def main(
        project_id: str = "your-project-id", location: str = "US", transport: str = "grpc"
    ) -> None:
        # Constructs the client for interacting with the service.
        client = https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient.html(transport=transport)

        report_reservations(client, project_id, location)


    def report_reservations(
        client: https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient.html,
        project_id: str,
        location: str,
    ) -> None:
        """Prints details and summary information about reservations defined within
        a given admin project and location.
        """
        print("Reservations in project {} in location {}".format(project_id, location))
        req = https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.types.ListReservationsRequest.html(
            parent=client.https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient.html#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient_common_location_path(project_id, location)
        )
        total_reservations = 0
        for reservation in client.https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest/google.cloud.bigquery_reservation_v1.services.reservation_service.ReservationServiceClient.html#google_cloud_bigquery_reservation_v1_services_reservation_service_ReservationServiceClient_list_reservations(request=req):
            print(
                f"\tReservation {reservation.name} "
                f"has {reservation.slot_capacity} slot capacity."
            )
            total_reservations = total_reservations + 1
        print(f"\n{total_reservations} reservations processed.")


    if __name__ == "__main__":
        parser = argparse.ArgumentParser()
        parser.add_argument("--project_id", type=str)
        parser.add_argument("--location", default="US", type=str)
        args = parser.parse_args()
        main(project_id=args.project_id, location=args.location)

<br />

## Additional resources

### C#

The following list contains links to more resources related to the
client library for C#:

- [API reference](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Reservation.V1/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-dotnet/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bc%23%5D)
- [Source code](https://github.com/googleapis/google-cloud-dotnet)

### Go

The following list contains links to more resources related to the
client library for Go:

- [API reference](https://pkg.go.dev/cloud.google.com/go/bigquery/reservation/apiv1?tab=doc)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-go/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bgo%5D)
- [Source code](https://github.com/googleapis/google-cloud-go)

### Java

The following list contains links to more resources related to the
client library for Java:

- [API reference](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryreservation/latest/overview)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/java-bigqueryreservation/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bjava%5D)
- [Source code](https://github.com/googleapis/java-bigqueryreservation)

### Node.js

The following list contains links to more resources related to the
client library for Node.js:

- [API reference](https://googleapis.dev/nodejs/bigqueryreservation/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/nodejs-bigquery-reservation/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bnode.js%5D)
- [Source code](https://github.com/googleapis/nodejs-bigquery-reservation)

### PHP

The following list contains links to more resources related to the
client library for PHP:

- [API reference](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-php/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bphp%5D)
- [Source code](https://github.com/googleapis/google-cloud-php)

### Python

The following list contains links to more resources related to the
client library for Python:

- [API reference](https://docs.cloud.google.com/python/docs/reference/bigqueryreservation/latest)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/python-bigquery-reservation/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bpython%5D)
- [Source code](https://github.com/googleapis/python-bigquery-reservation)

### Ruby

The following list contains links to more resources related to the
client library for Ruby:

- [API reference](https://googleapis.dev/ruby/google-cloud-bigquery-reservation/latest/index.html)
- [Client libraries best practices](https://docs.cloud.google.com/apis/docs/client-libraries-best-practices)
- [Issue tracker](https://github.com/googleapis/google-cloud-ruby/issues)
- [`google-bigquery` on Stack Overflow](https://stackoverflow.com/search?q=%5Bgoogle-bigquery%5D+%5Bruby%5D)
- [Source code](https://github.com/googleapis/google-cloud-ruby)

<br />


### What's next?

For more background and conceptual information about reservations, see [Introduction to Reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro).

<br />