# Introduction to SQL in BigQuery

This document provides an overview of supported statements and SQL dialects in
BigQuery.

GoogleSQL is an ANSI-compliant
[Structured Query Language (SQL)](https://en.wikipedia.org/wiki/SQL)
that includes the following types of supported statements:

- [Query statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax), also known as Data Query Language (DQL) statements, are the primary method to analyze data in BigQuery. They scan one or more tables or expressions and return the computed result rows. Query statements can include [pipe syntax](https://docs.cloud.google.com/bigquery/docs/pipe-syntax).
- [Procedural language statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language) are procedural extensions to GoogleSQL that allow you to execute multiple SQL statements in one request. Procedural statements can use variables and control-flow statements, and can have side effects.
- [Data Definition Language (DDL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)
  let you create and modify objects such as the following:

  - Datasets
  - Tables, including their schema and column types
  - Table clones and snapshots
  - Views
  - Functions
  - Indexes
  - Capacity commitments, reservations, and assignments
  - Row-level access policies
- [Data Manipulation Language (DML) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax)
  enable you to update, insert, and delete data from your
  BigQuery tables.

- [Data Control Language (DCL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language)
  let you control BigQuery system resources such as access and
  capacity.

- [Transaction Control Language (TCL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#transactions)
  allow you to manage transactions for data modifications.

- [Load statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements)
  and [export statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements)
  to manage data coming in and out of BigQuery.

## BigQuery SQL dialects

BigQuery supports the GoogleSQL dialect, which is the
recommended dialect for all new projects. A legacy SQL dialect is also available
with [some restrictions](https://docs.cloud.google.com/bigquery/docs/legacy-sql-feature-availability). We
recommend that you [migrate from legacy SQL to GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql).

### Changing from the default dialect

The interface you use to query your data determines which query dialect is the
default. To switch to a different dialect:

### Console

The default dialect for the Google Cloud console is GoogleSQL. To
change the dialect to legacy SQL:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, click the **More \> Query settings** button.

3. In the **Advanced options** section, for **SQL dialect** , click
   **Legacy** , then click **Save** . This sets the legacy SQL option for this
   query. When you click add_box
   **SQL Query** to create a new query, you must select the legacy SQL option
   again.

### SQL

The default SQL dialect is GoogleSQL.
You can set the SQL dialect by including the prefix
`#standardSQL` or `#legacySQL` as part of your query.
These query prefixes are not case-sensitive, must precede the query, and
must be separated from the query by a newline character. The following
example sets the dialect to legacy SQL and queries the natality dataset:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   #legacySQL
   SELECT
     weight_pounds, state, year, gestation_weeks
   FROM
     [bigquery-public-data:samples.natality]
   ORDER BY
     weight_pounds DESC
   LIMIT
     10;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

The default query dialect in the `bq` command-line tool is legacy SQL. To
switch to the GoogleSQL dialect, add the `--use_legacy_sql=false` or
`--nouse_legacy_sql` flag to your command-line statement.

**Switch to the GoogleSQL dialect**

To use GoogleSQL syntax in a query job, set the `use_legacy_sql`
parameter to `false`.

      bq query \
      --use_legacy_sql=false \
      'SELECT
        word
      FROM
        `bigquery-public-data.samples.shakespeare`'

**Set GoogleSQL as the default dialect**

You can set GoogleSQL as the default dialect for the command-line tool and
the interactive shell by editing the command-line tool's configuration file:
`.bigqueryrc`.

For more information on `.bigqueryrc`, see
[Setting default values for command-specific flags](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).

To set `--use_legacy_sql=false` in `.bigqueryrc`:

1. Open `.bigqueryrc` in a text editor. By default, `.bigqueryrc` should be in your user directory, for example, `$HOME/.bigqueryrc`.
2. Add the following text to the file. This example sets GoogleSQL as the
   default syntax for queries and for the `mk` command (used when you create a
   view). If you have already configured default values for `query` or `mk` command
   flags, you do not need to add `[query]` or `[mk]` again.

       [query]
       --use_legacy_sql=false
       [mk]
       --use_legacy_sql=false

3. Save and close the file.

4. If you are using the interactive shell, you must exit and restart for the changes to be applied.

For information on available command-line flags, see
[bq command-line tool reference](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference).

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

By default, the C# library uses GoogleSQL.

<br />

**Switch to the legacy SQL dialect**

To use legacy SQL syntax in a query job, set the `UseLegacySql`
parameter to `true`.



    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryQueryLegacy
    {
        public void QueryLegacy(
            string projectId = "your-project-id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            string query = @"
                SELECT name FROM [bigquery-public-data:usa_names.usa_1910_2013]
                WHERE state = 'TX'
                LIMIT 100";
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html job = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_CreateQueryJob_System_String_System_Collections_Generic_IEnumerable_Google_Cloud_BigQuery_V2_BigQueryParameter__Google_Cloud_BigQuery_V2_QueryOptions_(
                sql: query,
                parameters: null,
                options: new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.QueryOptions.html { UseLegacySql = true });
            // Wait for the job to complete.
            job = job.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();
            // Display the results
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryRow.html row in client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetQueryResults_Google_Apis_Bigquery_v2_Data_JobReference_Google_Cloud_BigQuery_V2_GetQueryResultsOptions_(job.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_Reference))
            {
                Console.WriteLine($"{row["name"]}");
            }
        }
    }

<br />

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

By default, the Go client library uses GoogleSQL.

<br />

**Switch to the legacy SQL dialect**

To use legacy SQL syntax in a query job, set the `UseLegacySQL` property
within the query configuration to `true`.


    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // queryLegacy demonstrates running a query using Legacy SQL.
    func queryLegacy(w io.Writer, projectID, sqlString string) error {
    	// projectID := "my-project-id"
    	// sqlString = "SELECT 3 as somenum"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	q := client.Query(sqlString)
    	q.UseLegacySQL = true

    	// Run the query and process the returned row iterator.
    	it, err := q.Read(ctx)
    	if err != nil {
    		return fmt.Errorf("query.Read(): %w", err)
    	}
    	for {
    		var row []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Value
    		err := it.Next(&row)
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		fmt.Fprintln(w, row)
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

By default, the Java client library uses GoogleSQL.

<br />

**Switch to the legacy SQL dialect**

To use legacy SQL syntax in a query job, set the `useLegacySql` parameter
to `true`.


    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    public class RunLegacyQuery {

      public static void main(String[] args) {
        runLegacyQuery();
      }

      public static void runLegacyQuery() {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // To use legacy SQL syntax, set useLegacySql to true.
          String query =
              "SELECT corpus FROM [bigquery-public-data:samples.shakespeare] GROUP BY corpus;";
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query).setUseLegacySql(true).build();

          // Execute the query.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html result = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          // Print the results.
          result.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__().forEach(rows -> rows.forEach(row -> System.out.println(row.getValue())));

          System.out.println("Legacy query ran successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Legacy query did not run \n" + e.toString());
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

By default, the Node.js client library uses GoogleSQL.

<br />

**Switch to the legacy SQL dialect**

To use legacy SQL syntax in a query job, set the `useLegacySql` parameter
to `true`.


    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryLegacy() {
      // Queries the U.S. given names dataset for the state of Texas using legacy SQL.

      const query =
        'SELECT word FROM [bigquery-public-data:samples.shakespeare] LIMIT 10;';

      // For all options, see https://cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query
      const options = {
        query: query,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
        useLegacySql: true,
      };

      // Run the query as a job
      const [job] = await bigquery.createQueryJob(options);
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} started.`);

      // Wait for the query to finish
      const [rows] = await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/job.html();

      // Print the results
      console.log('Rows:');
      rows.forEach(row => console.log(row));
    }

<br />

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

By default, the PHP client library uses GoogleSQL.

<br />

**Switch to the legacy SQL dialect**

To use legacy SQL syntax in a query job, set the `useLegacySql` parameter
to `true`.


    use Google\Cloud\BigQuery\BigQueryClient;

    /**
     * Query using legacy sql
     *
     * @param string $projectId The project Id of your Google Cloud Project.
     */
    function query_legacy(string $projectId): void
    {
        $query = 'SELECT corpus FROM [bigquery-public-data:samples.shakespeare] GROUP BY corpus';

        $bigQuery = new BigQueryClient([
          'projectId' => $projectId,
        ]);
        $jobConfig = $bigQuery->query($query)->useLegacySql(true);

        $queryResults = $bigQuery->runQuery($jobConfig);

        $i = 0;
        foreach ($queryResults as $row) {
            printf('--- Row %s ---' . PHP_EOL, ++$i);
            foreach ($row as $column => $value) {
                printf('%s: %s' . PHP_EOL, $column, json_encode($value));
            }
        }
        printf('Found %s row(s)' . PHP_EOL, $i);
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

By default, the Python client library uses GoogleSQL.

<br />

**Switch to the legacy SQL dialect**

To use legacy SQL syntax in a query job, set the `use_legacy_sql` parameter
to `True`.


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    query = (
        "SELECT name FROM [bigquery-public-data:usa_names.usa_1910_2013] "
        'WHERE state = "TX" '
        "LIMIT 100"
    )

    # Set use_legacy_sql to True to use legacy SQL syntax.
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(use_legacy_sql=True)

    # Start the query and waits for query job to complete, passing in the extra configuration.
    results = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        query, job_config=job_config
    )  # Make an API request.

    print("The query data:")
    for row in results:
        print(row)

<br />

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

By default, the Ruby client library uses GoogleSQL.

<br />

**Switch to the legacy SQL dialect**

To use legacy SQL syntax in a query job, pass the option `legacy_sql: true`
with your query.


    require "google/cloud/bigquery"

    def query_legacy
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      sql = "SELECT name FROM [bigquery-public-data:usa_names.usa_1910_2013] " \
            "WHERE state = 'TX' " \
            "LIMIT 100"

      results = bigquery.query sql, legacy_sql: true do |config|
        # Location must match that of the dataset(s) referenced in the query.
        config.location = "US"
      end

      results.each do |row|
        puts row.inspect
      end
    end

<br />

## What's next

- For information about how to run a SQL query in BigQuery, see [Running interactive and batch query jobs](https://docs.cloud.google.com/bigquery/docs/running-queries).
- For more information about query optimization in general, see [Introduction to optimizing query performance](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-overview).
- To learn about the GoogleSQL syntax used for querying data in BigQuery, see [Query syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax).
- To learn more about how to use pipe syntax in your queries, see [pipe syntax](https://docs.cloud.google.com/bigquery/docs/pipe-syntax).