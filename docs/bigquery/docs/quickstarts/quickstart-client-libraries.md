# Query a public dataset with the BigQuery client libraries

Learn how to query a public dataset with the BigQuery client
libraries.

*** ** * ** ***

To follow step-by-step guidance for this task directly in the
Google Cloud console, select your preferred programming language:

### C#

[Take the C# tour](https://console.cloud.google.com/?walkthrough_id=bigquery--csharp-client-library)

### Go

[Take the Go tour](https://console.cloud.google.com/?walkthrough_id=bigquery--go-client-library)

### Java

[Take the Java tour](https://console.cloud.google.com/?walkthrough_id=bigquery--java-client-library)

### Node.js

[Take the Node.js tour](https://console.cloud.google.com/?walkthrough_id=bigquery--node-client-library)

### PHP

[Take the PHP tour](https://console.cloud.google.com/?walkthrough_id=bigquery--php-client-library)

### Python

[Take the Python tour](https://console.cloud.google.com/?walkthrough_id=bigquery--python-client-library)

### Ruby

[Take the Ruby tour](https://console.cloud.google.com/?walkthrough_id=bigquery--ruby-client-library)

*** ** * ** ***

## Before you begin

1. [Create or select a Google Cloud project](https://cloud.google.com/resource-manager/docs/creating-managing-projects).

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

   - Create a Google Cloud project:

     ```
     gcloud projects create PROJECT_ID
     ```

     Replace `PROJECT_ID` with a name for the Google Cloud project you are creating.
   - Select the Google Cloud project that you created:

     ```
     gcloud config set project PROJECT_ID
     ```

     Replace `PROJECT_ID` with your Google Cloud project name.
2. Choose whether to
   [use the BigQuery sandbox at no charge](https://docs.cloud.google.com/bigquery/docs/sandbox),
   or to
   [enable billing for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/modify-project).

   If you do not enable billing for a project, you automatically work in the
   BigQuery sandbox. The BigQuery sandbox lets you learn
   BigQuery with a limited set of BigQuery
   features at no charge. If you do not plan to use your project beyond this
   document, we recommend that you use the BigQuery sandbox.
3.
   Grant roles to your user account. Run the following command once for each of the following
   IAM roles:
   `roles/serviceusage.serviceUsageAdmin, roles/bigquery.jobUser`

   ```bash
   gcloud projects add-iam-policy-binding PROJECT_ID --member="user:USER_IDENTIFIER" --role=ROLE
   ```

   Replace the following:
   - `PROJECT_ID`: Your project ID.
   - `USER_IDENTIFIER`: The identifier for your user account. For example, `myemail@example.com`.
   - `ROLE`: The IAM role that you grant to your user account.
4.


   Enable the BigQuery API:


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which contains the
   `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   ```bash
   gcloud services enable bigquery
   ```

   For new projects, the BigQuery API is automatically enabled.
5. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)
6. Activate your Google Cloud project in Cloud Shell:

       gcloud config set project PROJECT_ID

   Replace <var translate="no">PROJECT_ID</var> with the project that you selected for
   this walkthrough.

   The output is similar to the following:

   ```
   Updated property [core/project].
   ```

## Query a public dataset

Select one of the following languages:

### C#

1. In Cloud Shell, create a new C# project and file:

   ```bash
   dotnet new console -n BigQueryCsharpDemo
   ```

   The output is similar to the following. Several lines are omitted to
   simplify the output.

   ```
   Welcome to .NET 6.0!
   ---
   SDK Version: 6.0.407
   ...
   The template "Console App" was created successfully.
   ...
   ```

   This command creates a C# project that's named
   `BigQueryCsharpDemo` and a file that's named `Program.cs`.
2. Open the Cloud Shell Editor:

   ```bash
   cloudshell workspace BigQueryCsharpDemo
   ```
3. To open a terminal in the Cloud Shell Editor, click
   **Open Terminal**.

4. Open your project directory:

   ```bash
   cd BigQueryCsharpDemo
   ```
5. Install the BigQuery client library for C#:

   ```bash
   dotnet add package Google.Cloud.BigQuery.V2
   ```

   The output is similar to the following. Several lines are omitted to
   simplify the output.

   ```
   Determining projects to restore...
   Writing /tmp/tmpF7EKSd.tmp
   ...
   info : Writing assets file to disk.
   ...
   ```
6. Set the variable `GOOGLE_PROJECT_ID` to the value `GOOGLE_CLOUD_PROJECT`
   and export the variable:

   ```bash
   export GOOGLE_PROJECT_ID=$GOOGLE_CLOUD_PROJECT
   ```
7. Click **Open Editor**.

8. In the **Explorer** pane, locate your `BIGQUERYCSHARPDEMO`
   project.

9. Click the `Program.cs` file to open it.

10. To create a query against the
    `bigquery-public-data.stackoverflow` dataset that returns the
    top 10 most viewed Stack Overflow pages and their view counts, replace
    the contents of the file with the following code:



        using System;
        using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;

        namespace GoogleCloudSamples
        {
            public class Program
            {
                public static void Main(string[] args)
                {
                    string projectId = Environment.GetEnvironmentVariable("GOOGLE_PROJECT_ID");
                    var client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
                    string query = @"SELECT
                        CONCAT(
                            'https://stackoverflow.com/questions/',
                            CAST(id as STRING)) as url, view_count
                        FROM `bigquery-public-data.stackoverflow.posts_questions`
                        WHERE tags like '%google-bigquery%'
                        ORDER BY view_count DESC
                        LIMIT 10";
                    var result = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_ExecuteQuery_System_String_System_Collections_Generic_IEnumerable_Google_Cloud_BigQuery_V2_BigQueryParameter__Google_Cloud_BigQuery_V2_QueryOptions_Google_Cloud_BigQuery_V2_GetQueryResultsOptions_(query, parameters: null);
                    Console.Write("\nQuery Results:\n---\n");
                    foreach (var row in result)
                    {
                        Console.WriteLine($"{row["url"]}: {row["view_count"]} views");
                    }
                }
            }
        }

    <br />

11. Click **Open Terminal**.

12. In the terminal, run the `Program.cs` script. If you are prompted to
    authorize Cloud Shell and agree to the terms, click
    **Authorize**.

    ```bash
    dotnet run
    ```

    The result is similar to the following:

    ```
    Query Results:
    ---
    https://stackoverflow.com/questions/35159967: 170023 views
    https://stackoverflow.com/questions/22879669: 142581 views
    https://stackoverflow.com/questions/10604135: 132406 views
    https://stackoverflow.com/questions/44564887: 128781 views
    https://stackoverflow.com/questions/27060396: 127008 views
    https://stackoverflow.com/questions/12482637: 120766 views
    https://stackoverflow.com/questions/20673986: 115720 views
    https://stackoverflow.com/questions/39109817: 108368 views
    https://stackoverflow.com/questions/11057219: 105175 views
    https://stackoverflow.com/questions/43195143: 101878 views
    ```

You have successfully queried a public dataset with the
BigQuery C# client library.

### Go

1. In Cloud Shell, create a new Go project and file:

   ```bash
   mkdir bigquery-go-quickstart \
       && touch \
       bigquery-go-quickstart/app.go
   ```

   This command creates a Go project that's named
   `bigquery-go-quickstart` and a file that's named `app.go`.
2. Open the Cloud Shell Editor:

   ```bash
   cloudshell workspace bigquery-go-quickstart
   ```
3. To open a terminal in the Cloud Shell Editor, click
   **Open Terminal**.

4. Open your project directory:

   ```bash
   cd bigquery-go-quickstart
   ```
5. Create a `go.mod` file:

   ```bash
   go mod init quickstart
   ```

   The output is similar to the following:

   ```
   go: creating new go.mod: module quickstart
   go: to add module requirements and sums:
           go mod tidy
   ```
6. Install the BigQuery client library for Go:

   ```bash
   go get cloud.google.com/go/bigquery
   ```

   The output is similar to the following. Several lines are omitted to
   simplify the output.

   ```
   go: downloading cloud.google.com/go/bigquery v1.49.0
   go: downloading cloud.google.com/go v0.110.0
   ...
   go: added cloud.google.com/go/bigquery v1.49.0
   go: added cloud.google.com/go v0.110.0
   ```
7. Click **Open Editor**.

8. In the **Explorer** pane, locate your `BIGQUERY-GO-QUICKSTART`
   project.

9. Click the `app.go` file to open it.

10. To create a query against the
    `bigquery-public-data.stackoverflow` dataset that returns the
    top 10 most viewed Stack Overflow pages and their view counts, copy the
    following code into the `app.go` file:



        // Command simpleapp queries the Stack Overflow public dataset in Google BigQuery.
        package main

        import (
        	"context"
        	"fmt"
        	"io"
        	"log"
        	"os"

        	"cloud.google.com/go/bigquery"
        	"google.golang.org/api/iterator"
        )


        func main() {
        	projectID := os.Getenv("GOOGLE_CLOUD_PROJECT")
        	if projectID == "" {
        		fmt.Println("GOOGLE_CLOUD_PROJECT environment variable must be set.")
        		os.Exit(1)
        	}

        	ctx := context.Background()

        	client, err := bigquery.NewClient(ctx, projectID)
        	if err != nil {
        		log.Fatalf("bigquery.NewClient: %v", err)
        	}
        	defer client.Close()

        	rows, err := query(ctx, client)
        	if err != nil {
        		log.Fatal(err)
        	}
        	if err := printResults(os.Stdout, rows); err != nil {
        		log.Fatal(err)
        	}
        }

        // query returns a row iterator suitable for reading query results.
        func query(ctx context.Context, client *bigquery.Client) (*bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_RowIterator, error) {

        	query := client.Query(
        		`SELECT
        			CONCAT(
        				'https://stackoverflow.com/questions/',
        				CAST(id as STRING)) as url,
        			view_count
        		FROM ` + "`bigquery-public-data.stackoverflow.posts_questions`" + `
        		WHERE tags like '%google-bigquery%'
        		ORDER BY view_count DESC
        		LIMIT 10;`)
        	return query.Read(ctx)
        }

        type StackOverflowRow struct {
        	URL       string `bigquery:"url"`
        	ViewCount int64  `bigquery:"view_count"`
        }

        // printResults prints results from a query to the Stack Overflow public dataset.
        func printResults(w io.Writer, iter *bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_RowIterator) error {
        	for {
        		var row StackOverflowRow
        		err := iter.Next(&row)
        		if err == iterator.Done {
        			return nil
        		}
        		if err != nil {
        			return fmt.Errorf("error iterating through results: %w", err)
        		}

        		fmt.Fprintf(w, "url: %s views: %d\n", row.URL, row.ViewCount)
        	}
        }

    <br />

11. Click **Open Terminal**.

12. In the terminal, run the `app.go` script. If you are prompted to
    authorize Cloud Shell and agree to the terms, click
    **Authorize**.

    ```bash
    go run app.go
    ```

    The result is similar to the following:

    ```
    https://stackoverflow.com/questions/35159967 : 170023 views
    https://stackoverflow.com/questions/22879669 : 142581 views
    https://stackoverflow.com/questions/10604135 : 132406 views
    https://stackoverflow.com/questions/44564887 : 128781 views
    https://stackoverflow.com/questions/27060396 : 127008 views
    https://stackoverflow.com/questions/12482637 : 120766 views
    https://stackoverflow.com/questions/20673986 : 115720 views
    https://stackoverflow.com/questions/39109817 : 108368 views
    https://stackoverflow.com/questions/11057219 : 105175 views
    https://stackoverflow.com/questions/43195143 : 101878 views
    ```

You have successfully queried a public dataset with the
BigQuery Go client library.

### Java

1. In Cloud Shell, create a new Java project using Apache Maven:

   ```bash
   mvn archetype:generate \
       -DgroupId=com.google.app \
       -DartifactId=bigquery-java-quickstart \
       -DinteractiveMode=false
   ```

   This command creates a Maven project that's named
   `bigquery-java-quickstart`.

   The output is similar to the following. Several lines are omitted to
   simplify the output.

   ```
   [INFO] Scanning for projects...
   ...
   [INFO] Building Maven Stub Project (No POM) 1
   ...
   [INFO] BUILD SUCCESS
   ...
   ```

   There are many dependency management systems that you can use other than
   Maven. For more information, learn how to
   [set up a Java development environment](https://docs.cloud.google.com/java/docs/setup) to use with
   client libraries.
2. Rename the `App.java` file that Maven creates by default:

   ```bash
   mv \
       bigquery-java-quickstart/src/main/java/com/google/app/App.java \
       bigquery-java-quickstart/src/main/java/com/google/app/SimpleApp.java
   ```
3. Open the Cloud Shell Editor:

   ```bash
   cloudshell workspace bigquery-java-quickstart
   ```
4. If you are prompted whether to synchronize the Java
   classpath or configuration, click **Always**.

   If you are not prompted and encounter an error that is
   related to the classpath during this walkthrough, do the following:
   1. Click **File \> Preferences \> Open Settings (UI)**.
   2. Click **Extensions \> Java**.
   3. Scroll to **Configuration: Update Build Configuration** and select **automatic**.
5. In the **Explorer** pane, locate your `BIGQUERY-JAVA-QUICKSTART`
   project.

6. Click the `pom.xml` file to open it.

7. Inside the `<dependencies>` tag, add the following dependency
   after any existing ones. Do not replace any existing dependencies.

       <dependency>
         <groupId>com.google.cloud</groupId>
         <artifactId>google-cloud-bigquery</artifactId>
       </dependency>

8. On the line following the closing tag (`</dependencies>`), add the
   following:

       <dependencyManagement>
         <dependencies>
           <dependency>
             <groupId>com.google.cloud</groupId>
             <artifactId>libraries-bom</artifactId>
             <version>26.1.5</version>
             <type>pom</type>
             <scope>import</scope>
           </dependency>
         </dependencies>
       </dependencyManagement>

9. In the **Explorer** pane, in your `BIGQUERY-JAVA-QUICKSTART` project,
   click **src \> main/java/com/google/app \> SimpleApp.java**.
   The file opens.

10. To create a query against the
    `bigquery-public-data.stackoverflow` dataset, leave the
    first line of the file (`package com.google.app;`), and replace the
    remaining contents of the file with the following code:



        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FieldValueList.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
        import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;


        public class SimpleApp {

          public static void main(String... args) throws Exception {
            // TODO(developer): Replace these variables before running the app.
            String projectId = "MY_PROJECT_ID";
            simpleApp(projectId);
          }

          public static void simpleApp(String projectId) {
            try {
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(
                          "SELECT CONCAT('https://stackoverflow.com/questions/', "
                              + "CAST(id as STRING)) as url, view_count "
                              + "FROM `bigquery-public-data.stackoverflow.posts_questions` "
                              + "WHERE tags like '%google-bigquery%' "
                              + "ORDER BY view_count DESC "
                              + "LIMIT 10")
                      // Use standard SQL syntax for queries.
                      // See: https://cloud.google.com/bigquery/sql-reference/
                      .setUseLegacySql(false)
                      .build();

              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html jobId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html.newBuilder().setProject(projectId).build();
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html queryJob = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.newBuilder(queryConfig).setJobId(jobId).build());

              // Wait for the query to complete.
              queryJob = queryJob.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();

              // Check for errors
              if (queryJob == null) {
                throw new RuntimeException("Job no longer exists");
              } else if (queryJob.getStatus().https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobStatus.html#com_google_cloud_bigquery_JobStatus_getExecutionErrors__() != null
                  && queryJob.getStatus().getExecutionErrors().size() > 0) {
                // TODO(developer): Handle errors here. An error here do not necessarily mean that the job
                // has completed or was unsuccessful.
                // For more details: https://cloud.google.com/bigquery/troubleshooting-errors
                throw new RuntimeException("An unhandled error has occurred");
              }

              // Get the results.
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html result = queryJob.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_getQueryResults_com_google_cloud_bigquery_BigQuery_QueryResultsOption____();

              // Print all pages of the results.
              for (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FieldValueList.html row : result.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()) {
                // String type
                String url = row.get("url").getStringValue();
                String viewCount = row.get("view_count").getStringValue();
                System.out.printf("%s : %s views\n", url, viewCount);
              }
            } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
              System.out.println("Simple App failed due to error: \n" + e.toString());
            }
          }
        }

    <br />

    The query returns the top 10 most viewed Stack Overflow pages and
    their view counts.
11. Right-click **SimpleApp.java** and click **Run Java** . If you are
    prompted to authorize Cloud Shell and agree to the terms, click
    **Authorize**.

    The result is similar to the following:

    ```
    https://stackoverflow.com/questions/35159967 : 170023 views
    https://stackoverflow.com/questions/22879669 : 142581 views
    https://stackoverflow.com/questions/10604135 : 132406 views
    https://stackoverflow.com/questions/44564887 : 128781 views
    https://stackoverflow.com/questions/27060396 : 127008 views
    https://stackoverflow.com/questions/12482637 : 120766 views
    https://stackoverflow.com/questions/20673986 : 115720 views
    https://stackoverflow.com/questions/39109817 : 108368 views
    https://stackoverflow.com/questions/11057219 : 105175 views
    https://stackoverflow.com/questions/43195143 : 101878 views
    ```

You have successfully queried a public dataset with the
BigQuery Java client library.

### Node.js

1. In Cloud Shell, create a new Node.js project and file:

   ```bash
   mkdir bigquery-node-quickstart \
       && touch \
       bigquery-node-quickstart/app.js
   ```

   This command creates a Node.js project that's named
   `bigquery-node-quickstart` and a file that's named `app.js`.
2. Open the Cloud Shell Editor:

   ```bash
   cloudshell workspace bigquery-node-quickstart
   ```
3. To open a terminal in the Cloud Shell Editor, click
   **Open Terminal**.

4. Open your project directory:

   ```bash
   cd bigquery-node-quickstart
   ```
5. Install the BigQuery client library for Node.js:

   ```bash
   npm install @google-cloud/bigquery
   ```

   The output is similar to the following:

   ```
   added 63 packages in 2s
   ```
6. Click **Open Editor**.

7. In the **Explorer** pane, locate your `BIGQUERY-NODE-QUICKSTART`
   project.

8. Click the `app.js` file to open it.

9. To create a query against the
   `bigquery-public-data.stackoverflow` dataset that returns the
   top 10 most viewed Stack Overflow pages and their view counts, copy the
   following code into the `app.js` file:


       // Import the Google Cloud client library
       const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');

       async function queryStackOverflow() {
         // Queries a public Stack Overflow dataset.

         // Create a client
         const bigqueryClient = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

         // The SQL query to run
         const sqlQuery = `SELECT
           CONCAT(
             'https://stackoverflow.com/questions/',
             CAST(id as STRING)) as url,
           view_count
           FROM \`bigquery-public-data.stackoverflow.posts_questions\`
           WHERE tags like '%google-bigquery%'
           ORDER BY view_count DESC
           LIMIT 10`;

         const options = {
           query: sqlQuery,
           // Location must match that of the dataset(s) referenced in the query.
           location: 'US',
         };

         // Run the query
         const [rows] = await bigqueryClient.query(options);

         console.log('Query Results:');
         rows.forEach(row => {
           const url = row['url'];
           const viewCount = row['view_count'];
           console.log(`url: ${url}, ${viewCount} views`);
         });
       }
       queryStackOverflow();

   <br />

10. Click **Open Terminal**.

11. In the terminal, run the `app.js` script. If you are prompted to
    authorize Cloud Shell and agree to the terms, click
    **Authorize**.

    ```bash
    node app.js
    ```

    The result is similar to the following:

    ```
    Query Results:
    url: https://stackoverflow.com/questions/35159967, 170023 views
    url: https://stackoverflow.com/questions/22879669, 142581 views
    url: https://stackoverflow.com/questions/10604135, 132406 views
    url: https://stackoverflow.com/questions/44564887, 128781 views
    url: https://stackoverflow.com/questions/27060396, 127008 views
    url: https://stackoverflow.com/questions/12482637, 120766 views
    url: https://stackoverflow.com/questions/20673986, 115720 views
    url: https://stackoverflow.com/questions/39109817, 108368 views
    url: https://stackoverflow.com/questions/11057219, 105175 views
    url: https://stackoverflow.com/questions/43195143, 101878 views
    ```

You have successfully queried a public dataset with the
BigQuery Node.js client library.

### PHP

1. In Cloud Shell, create a new PHP project and file:

   ```bash
   mkdir bigquery-php-quickstart \
       && touch \
       bigquery-php-quickstart/app.php
   ```

   This command creates a PHP project that's named
   `bigquery-php-quickstart` and a file that's named `app.php`.
2. Open the Cloud Shell Editor:

   ```bash
   cloudshell workspace bigquery-php-quickstart
   ```
3. To open a terminal in the Cloud Shell Editor, click
   **Open Terminal**.

4. Open your project directory:

   ```bash
   cd bigquery-php-quickstart
   ```
5. Install the BigQuery client library for PHP:

   ```bash
   composer require google/cloud-bigquery
   ```

   The output is similar to the following. Several lines are omitted to
   simplify the output.

   ```
   Running composer update google/cloud-bigquery
   Loading composer repositories with package information
   Updating dependencies
   ...
   No security vulnerability advisories found
   Using version ^1.24 for google/cloud-bigquery
   ```
6. Click **Open Editor**.

7. In the **Explorer** pane, locate your `BIGQUERY-PHP-QUICKSTART`
   project.

8. Click the `app.php` file to open it.

9. To create a query against the
   `bigquery-public-data.stackoverflow` dataset that returns the
   top 10 most viewed Stack Overflow pages and their view counts, copy the
   following code into the `app.php` file:


       <?php
       # ...

       require __DIR__ . '/vendor/autoload.php';

       use Google\Cloud\BigQuery\BigQueryClient;


       $bigQuery = new BigQueryClient();
       $query = <<<ENDSQL
       SELECT
         CONCAT(
           'https://stackoverflow.com/questions/',
           CAST(id as STRING)) as url,
         view_count
       FROM `bigquery-public-data.stackoverflow.posts_questions`
       WHERE tags like '%google-bigquery%'
       ORDER BY view_count DESC
       LIMIT 10;
       ENDSQL;
       $queryJobConfig = $bigQuery->query($query);
       $queryResults = $bigQuery->runQuery($queryJobConfig);

       if ($queryResults->isComplete()) {
           $i = 0;
           $rows = $queryResults->rows();
           foreach ($rows as $row) {
               printf('--- Row %s ---' . PHP_EOL, ++$i);
               printf('url: %s, %s views' . PHP_EOL, $row['url'], $row['view_count']);
           }
           printf('Found %s row(s)' . PHP_EOL, $i);
       } else {
           throw new Exception('The query failed to complete');
       }

   <br />

10. Click **Open Terminal**.

11. In the terminal, run the `app.php` script. If you are prompted to
    authorize Cloud Shell and agree to the terms, click
    **Authorize**.

    ```bash
    php app.php
    ```

    The result is similar to the following:

    ```
    --- Row 1 ---
    url: https://stackoverflow.com/questions/35159967, 170023 views
    --- Row 2 ---
    url: https://stackoverflow.com/questions/22879669, 142581 views
    --- Row 3 ---
    url: https://stackoverflow.com/questions/10604135, 132406 views
    --- Row 4 ---
    url: https://stackoverflow.com/questions/44564887, 128781 views
    --- Row 5 ---
    url: https://stackoverflow.com/questions/27060396, 127008 views
    --- Row 6 ---
    url: https://stackoverflow.com/questions/12482637, 120766 views
    --- Row 7 ---
    url: https://stackoverflow.com/questions/20673986, 115720 views
    --- Row 8 ---
    url: https://stackoverflow.com/questions/39109817, 108368 views
    --- Row 9 ---
    url: https://stackoverflow.com/questions/11057219, 105175 views
    --- Row 10 ---
    url: https://stackoverflow.com/questions/43195143, 101878 views
    Found 10 row(s)
    ```

You have successfully queried a public dataset with the
BigQuery PHP client library.

### Python

1. In Cloud Shell, create a new Python project and file:

   ```bash
   mkdir bigquery-python-quickstart \
       && touch \
       bigquery-python-quickstart/app.py
   ```

   This command creates a Python project that's named
   `bigquery-python-quickstart` and a file that's named `app.py`.
2. Open the Cloud Shell Editor:

   ```bash
   cloudshell workspace bigquery-python-quickstart
   ```
3. To open a terminal in the Cloud Shell Editor, click
   **Open Terminal**.

4. Open your project directory:

   ```bash
   cd bigquery-python-quickstart
   ```
5. Install the BigQuery client library for Python:

   ```bash
   pip install --upgrade google-cloud-bigquery
   ```

   The output is similar to the following. Several lines are omitted to
   simplify the output.

   ```
   Installing collected packages: google-cloud-bigquery
   ...
   Successfully installed google-cloud-bigquery-3.9.0
   ...
   ```
6. Click **Open Editor**.

7. In the **Explorer** pane, locate your `BIGQUERY-PYTHON-QUICKSTART`
   project.

8. Click the `app.py` file to open it.

9. To create a query against the
   `bigquery-public-data.stackoverflow` dataset that returns the
   top 10 most viewed Stack Overflow pages and their view counts, copy the
   following code into the `app.py` file:


       from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest



       def query_stackoverflow() -> None:
           client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
           results = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
               """
               SELECT
                 CONCAT(
                   'https://stackoverflow.com/questions/',
                   CAST(id as STRING)) as url,
                 view_count
               FROM `bigquery-public-data.stackoverflow.posts_questions`
               WHERE tags like '%google-bigquery%'
               ORDER BY view_count DESC
               LIMIT 10"""
           )  # Waits for job to complete.

           for row in results:
               print("{} : {} views".format(row.url, row.view_count))


       if __name__ == "__main__":
           query_stackoverflow()

   <br />

10. Click **Open Terminal**.

11. In the terminal, run the `app.py` script. If you are prompted to
    authorize Cloud Shell and agree to the terms, click
    **Authorize**.

    ```bash
    python app.py
    ```

    The result is similar to the following:

    ```
    https://stackoverflow.com/questions/35159967 : 170023 views
    https://stackoverflow.com/questions/22879669 : 142581 views
    https://stackoverflow.com/questions/10604135 : 132406 views
    https://stackoverflow.com/questions/44564887 : 128781 views
    https://stackoverflow.com/questions/27060396 : 127008 views
    https://stackoverflow.com/questions/12482637 : 120766 views
    https://stackoverflow.com/questions/20673986 : 115720 views
    https://stackoverflow.com/questions/39109817 : 108368 views
    https://stackoverflow.com/questions/11057219 : 105175 views
    https://stackoverflow.com/questions/43195143 : 101878 views
    ```

You have successfully queried a public dataset with the
BigQuery Python client library.

### Ruby

1. In Cloud Shell, create a new Ruby project and file:

   ```bash
   mkdir bigquery-ruby-quickstart \
       && touch \
       bigquery-ruby-quickstart/app.rb
   ```

   This command creates a Ruby project that's named
   `bigquery-ruby-quickstart` and a file that's named `app.rb`.
2. Open the Cloud Shell Editor:

   ```bash
   cloudshell workspace bigquery-ruby-quickstart
   ```
3. To open a terminal in the Cloud Shell Editor, click
   **Open Terminal**.

4. Open your project directory:

   ```bash
   cd bigquery-ruby-quickstart
   ```
5. Install the BigQuery client library for Ruby:

   ```bash
   gem install google-cloud-bigquery
   ```

   The output is similar to the following. Several lines are omitted to
   simplify the output.

   ```
   23 gems installed
   ```
6. Click **Open Editor**.

7. In the **Explorer** pane, locate your `BIGQUERY-RUBY-QUICKSTART`
   project.

8. Click the `app.rb` file to open it.

9. To create a query against the
   `bigquery-public-data.stackoverflow` dataset that returns the
   top 10 most viewed Stack Overflow pages and their view counts, copy the
   following code into the `app.rb` file:


       require "google/cloud/bigquery"

       # This uses Application Default Credentials to authenticate.
       # @see https://cloud.google.com/bigquery/docs/authentication/getting-started
       bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html

       sql     = "SELECT " \
                 "CONCAT('https://stackoverflow.com/questions/', CAST(id as STRING)) as url, view_count " \
                 "FROM `bigquery-public-data.stackoverflow.posts_questions` " \
                 "WHERE tags like '%google-bigquery%' " \
                 "ORDER BY view_count DESC LIMIT 10"
       results = bigquery.query sql

       results.each do |row|
         puts "#{https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-InsertResponse-InsertError.html[:url]}: #{https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-InsertResponse-InsertError.html[:view_count]} views"
       end

   <br />

10. Click **Open Terminal**.

11. In the terminal, run the `app.rb` script. If you are prompted to
    authorize Cloud Shell and agree to the terms, click
    **Authorize**.

    ```bash
    ruby app.rb
    ```

    The result is similar to the following:

    ```
    https://stackoverflow.com/questions/35159967: 170023 views
    https://stackoverflow.com/questions/22879669: 142581 views
    https://stackoverflow.com/questions/10604135: 132406 views
    https://stackoverflow.com/questions/44564887: 128781 views
    https://stackoverflow.com/questions/27060396: 127008 views
    https://stackoverflow.com/questions/12482637: 120766 views
    https://stackoverflow.com/questions/20673986: 115720 views
    https://stackoverflow.com/questions/39109817: 108368 views
    https://stackoverflow.com/questions/11057219: 105175 views
    https://stackoverflow.com/questions/43195143: 101878 views
    ```

You have successfully queried a public dataset with the
BigQuery Ruby client library.

## Clean up

To avoid incurring charges to your Google Cloud account, either delete
your Google Cloud project, or delete the resources that you created in
this walkthrough.

### Delete the project


The easiest way to eliminate billing is to delete the project that you
created for the tutorial.

To delete the project:

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

<br />

<br />

### Delete the resources

If you used an existing project, delete the resources that you created:

### C#

1. In Cloud Shell, move up a directory:

   ```bash
   cd ..
   ```
2. Delete the `BigQueryCsharpDemo` folder that you created:

   ```bash
   rm -R BigQueryCsharpDemo
   ```

   The `-R` flag deletes all assets in a folder.

### Go

1. In Cloud Shell, move up a directory:

   ```bash
   cd ..
   ```
2. Delete the `bigquery-go-quickstart` folder that you created:

   ```bash
   rm -R bigquery-go-quickstart
   ```

   The `-R` flag deletes all assets in a folder.

### Java

1. In Cloud Shell, move up a directory:

   ```bash
   cd ..
   ```
2. Delete the `bigquery-java-quickstart` folder that you created:

   ```bash
   rm -R bigquery-java-quickstart
   ```

   The `-R` flag deletes all assets in a folder.

### Node.js

1. In Cloud Shell, move up a directory:

   ```bash
   cd ..
   ```
2. Delete the `bigquery-node-quickstart` folder that you created:

   ```bash
   rm -R bigquery-node-quickstart
   ```

   The `-R` flag deletes all assets in a folder.

### PHP

1. In Cloud Shell, move up a directory:

   ```bash
   cd ..
   ```
2. Delete the `bigquery-php-quickstart` folder that you created:

   ```bash
   rm -R bigquery-php-quickstart
   ```

   The `-R` flag deletes all assets in a folder.

### Python

1. In Cloud Shell, move up a directory:

   ```bash
   cd ..
   ```
2. Delete the `bigquery-python-quickstart` folder that you created:

   ```bash
   rm -R bigquery-python-quickstart
   ```

   The `-R` flag deletes all assets in a folder.

### Ruby

1. In Cloud Shell, move up a directory:

   ```bash
   cd ..
   ```
2. Delete the `bigquery-ruby-quickstart` folder that you created:

   ```bash
   rm -R bigquery-ruby-quickstart
   ```

   The `-R` flag deletes all assets in a folder.

## What's next

- Learn more about using the [BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries).
- Learn more about [BigQuery public datasets](https://docs.cloud.google.com/bigquery/public-data).
- Learn how to [load data into BigQuery](https://docs.cloud.google.com/bigquery/docs/loading-data).
- Learn more about [querying data in BigQuery](https://docs.cloud.google.com/bigquery/docs/query-overview).
- Get [updates about BigQuery](https://docs.cloud.google.com/bigquery/docs/release-notes).
- Learn about [BigQuery pricing](https://cloud.google.com/bigquery/pricing).
- Learn about [BigQuery quotas and limits](https://docs.cloud.google.com/bigquery/quotas).