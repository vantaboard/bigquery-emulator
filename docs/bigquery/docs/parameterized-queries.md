# Run parameterized queries

When querying BigQuery data using GoogleSQL syntax, you can
use parameters to protect queries made from user input against
[SQL injection](https://en.wikipedia.org/wiki/SQL_injection). The parameters
substitute arbitrary expressions in your GoogleSQL queries.

You pass query parameters for various data types, including the following:

- Arrays
- Timestamps
- Structs
- Ranges

## Pass a parameter in a query

Query parameters are only supported in
[GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql). Parameters
cannot be used as substitutes for identifiers, column names, table names, or
other parts of the query.

To specify a named parameter, use the `@` character followed by an
[identifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#identifiers), such as
`@param_name`. Alternatively, use the placeholder value `?` to specify a
positional parameter. A query can use positional or named parameters,
but not both.

> [!NOTE]
> **Note:** To protect potentially sensitive information, the parameter value isn't logged in the BigQuery [logs](https://docs.cloud.google.com/bigquery/docs/monitoring#logs) when you run a query with a parameter.

You can run a parameterized query in BigQuery in the following
ways:

- The BigQuery Studio query editor in the Google Cloud console
- The bq command-line tool's `bq query` command
- The API
- The client libraries

The following examples show how to pass parameter values to a parameterized
query:

### Console

To run a parameterized query in the Google Cloud console, configure
parameters in **Query settings** , and then reference them in your SQL
query by prefixing each parameter name with the `@` character.

**Supported data types** : the Google Cloud console only supports
parameterized queries of primitive data types, such as `BIGNUMERIC`, `BOOL`,
`BYTES`, `DATE`, `DATETIME`, `FLOAT64`, `GEOGRAPHY`, `INT64`, `INTERVAL`,
`NUMERIC`, `STRING`, `TIME`, or `TIMESTAMP`. Complex data types, such as
`ARRAY` and `STRUCT`, aren't supported in the Google Cloud console.

## Add the parameters in the Google Cloud console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor toolbar, click
   **More** and select **Query settings**.

3. In the **Query settings** pane, locate the **Query parameters** section
   and click **Add parameter**.

4. For each parameter in your query, provide the following:

   - **Name** : Enter the parameter name (don't include the `@` character).
   - **Type**: Select the data type for the parameter.
   - **Value**: Enter the value you want to use for this execution.
5. Click **Save**.

## Pass parameter values to a query in the Google Cloud console

1. In the query editor, enter a SQL query using the parameters you
   configured in the previous step. Reference them by prefixing their
   names with the `@` character, as shown in the example.

   **Example**:

       SELECT
           word,
           word_count
         FROM
           `bigquery-public-data.samples.shakespeare`
         WHERE
           corpus = @corpus
         AND
           word_count >= @min_word_count
         ORDER BY
           word_count DESC;

   For this example, you would add the `corpus` parameter as a `STRING`
   with value `romeoandjuliet`, and the `min_word_count` parameter as an
   `INT64` with value `250`.

   If the query contains a missing or invalid parameter, an error message
   is displayed. Click **Set parameter** in the error message to adjust the
   parameter settings.
2. To run the parameterized query in the query editor, click **Run**.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. Use `--parameter` to provide values for parameters in the form
   `name:type:value`. An empty name produces a positional parameter.
   The type may be omitted to assume `STRING`.

   The `--parameter` flag must be used in conjunction with the flag
   `--use_legacy_sql=false` to specify GoogleSQL syntax.

   (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/locations) using the
   `--location` flag.

   ```bash
   bq query \
      --use_legacy_sql=false \
      --parameter=corpus::romeoandjuliet \
      --parameter=min_word_count:INT64:250 \
      'SELECT
        word,
        word_count
      FROM
        `bigquery-public-data.samples.shakespeare`
      WHERE
        corpus = @corpus
      AND
        word_count >= @min_word_count
      ORDER BY
        word_count DESC;'
   ```

### API

To use named parameters, set the `parameterMode` to `NAMED` in the `query` job
configuration.

Populate `queryParameters` with the list of parameters in the `query` job
configuration. Set the `name` of each parameter with the `@param_name` used in
the query.

[Enable GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/introduction-sql)
by setting `useLegacySql` to `false`.

<br />

    {
      "query": "SELECT word, word_count FROM `bigquery-public-data.samples.shakespeare` WHERE corpus = @corpus AND word_count >= @min_word_count ORDER BY word_count DESC;",
      "queryParameters": [
        {
          "parameterType": {
            "type": "STRING"
          },
          "parameterValue": {
            "value": "romeoandjuliet"
          },
          "name": "corpus"
        },
        {
          "parameterType": {
            "type": "INT64"
          },
          "parameterValue": {
            "value": "250"
          },
          "name": "min_word_count"
        }
      ],
      "useLegacySql": false,
      "parameterMode": "NAMED"
    }

[Try it in the Google APIs Explorer](https://developers.google.com/apis-explorer/#p/bigquery/v2/bigquery.jobs.query?projectId=my-project-id&_h=1&resource=%257B%250A++%2522query%2522%253A+%2522SELECT+word%252C+word_count%255CnFROM+%2560bigquery-public-data.samples.shakespeare%2560%255CnWHERE+corpus+%253D+%2540corpus%255CnAND+word_count+%253E%253D+%2540min_word_count%255CnORDER+BY+word_count+DESC%253B%2522%252C%250A++%2522queryParameters%2522%253A+%250A++%255B%250A++++%257B%250A++++++%2522parameterType%2522%253A+%250A++++++%257B%250A++++++++%2522type%2522%253A+%2522STRING%2522%250A++++++%257D%252C%250A++++++%2522parameterValue%2522%253A+%250A++++++%257B%250A++++++++%2522value%2522%253A+%2522romeoandjuliet%2522%250A++++++%257D%252C%250A++++++%2522name%2522%253A+%2522corpus%2522%250A++++%257D%252C%250A++++%257B%250A++++++%2522parameterType%2522%253A+%250A++++++%257B%250A++++++++%2522type%2522%253A+%2522INT64%2522%250A++++++%257D%252C%250A++++++%2522parameterValue%2522%253A+%250A++++++%257B%250A++++++++%2522value%2522%253A+%2522250%2522%250A++++++%257D%252C%250A++++++%2522name%2522%253A+%2522min_word_count%2522%250A++++%257D%250A++%255D%252C%250A++%2522useLegacySql%2522%253A+false%252C%250A++%2522parameterMode%2522%253A+%2522NAMED%2522%250A%257D&).

To use positional parameters, set the `parameterMode` to `POSITIONAL` in the
`query` job configuration.

### C#

<br />


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To use named parameters:


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryQueryWithNamedParameters
    {
        public void QueryWithNamedParameters(string projectId = "your-project-id")
        {
            var corpus = "romeoandjuliet";
            var minWordCount = 250;

            // Note: Standard SQL is required to use query parameters.
            var query = @"
                SELECT word, word_count
                FROM `bigquery-public-data.samples.shakespeare`
                WHERE corpus = @corpus
                AND word_count >= @min_word_count
                ORDER BY word_count DESC";

            // Initialize client that will be used to send requests.
            var client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);

            var parameters = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html[]
            {
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html("corpus", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String, corpus),
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html("min_word_count", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_Int64, minWordCount)
            };

            var job = client.CreateQueryJob(
                sql: query,
                parameters: parameters,
                options: new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.QueryOptions.html { UseQueryCache = false });
            // Wait for the job to complete.
            job = job.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();
            // Display the results
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryRow.html row in client.GetQueryResults(job.Reference))
            {
                Console.WriteLine($"{row["word"]}: {row["word_count"]}");
            }
        }
    }

<br />


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To use positional parameters:


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryQueryWithPositionalParameters
    {
        public void QueryWithPositionalParameters(string projectId = "project-id")
        {
            var corpus = "romeoandjuliet";
            var minWordCount = 250;

            // Note: Standard SQL is required to use query parameters.
            var query = @"
                    SELECT word, word_count
                    FROM `bigquery-public-data.samples.shakespeare`
                    WHERE corpus = ?
                    AND word_count >= ?
                    ORDER BY word_count DESC;";

            // Initialize client that will be used to send requests.
            var client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);

            // Set the name to None to use positional parameters.
            // Note that you cannot mix named and positional parameters.
            var parameters = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html[]
            {
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html(null, https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String, corpus),
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html(null, https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_Int64, minWordCount)
            };

            var job = client.CreateQueryJob(
                sql: query,
                parameters: parameters,
                options: new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.QueryOptions.html
                {
                    UseQueryCache = false,
                    ParameterMode = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameterMode.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameterMode.html#Google_Cloud_BigQuery_V2_BigQueryParameterMode_Positional
                });
            // Wait for the job to complete.
            job = job.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();
            // Display the results
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryRow.html row in client.GetQueryResults(job.Reference))
            {
                Console.WriteLine($"{row["word"]}: {row["word_count"]}");
            }
        }
    }

### Go

<br />


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To use named parameters:

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // queryWithNamedParams demonstrate issuing a query using named query parameters.
    func queryWithNamedParams(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	q := client.Query(
    		`SELECT word, word_count
            FROM ` + "`bigquery-public-data.samples.shakespeare`" + `
            WHERE corpus = @corpus
            AND word_count >= @min_word_count
            ORDER BY word_count DESC;`)
    	q.Parameters = []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryParameter{
    		{
    			Name:  "corpus",
    			Value: "romeoandjuliet",
    		},
    		{
    			Name:  "min_word_count",
    			Value: 250,
    		},
    	}
    	// Run the query and print results when the query job is completed.
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	it, err := job.Read(ctx)
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

To use positional parameters:

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // queryWithPostionalParams demonstrate issuing a query using positional query parameters.
    func queryWithPositionalParams(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	q := client.Query(
    		`SELECT word, word_count
            FROM ` + "`bigquery-public-data.samples.shakespeare`" + `
            WHERE corpus = ?
            AND word_count >= ?
            ORDER BY word_count DESC;`)
    	q.Parameters = []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryParameter{
    		{
    			Value: "romeoandjuliet",
    		},
    		{
    			Value: 250,
    		},
    	}
    	// Run the query and print results when the query job is completed.
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	it, err := job.Read(ctx)
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

### Java

<br />


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To use named parameters:

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    public class QueryWithNamedParameters {

      public static void queryWithNamedParameters() {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          String corpus = "romeoandjuliet";
          long minWordCount = 250;
          String query =
              "SELECT word, word_count\n"
                  + "FROM `bigquery-public-data.samples.shakespeare`\n"
                  + "WHERE corpus = @corpus\n"
                  + "AND word_count >= @min_word_count\n"
                  + "ORDER BY word_count DESC";

          // Note: Standard SQL is required to use query parameters.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addNamedParameter_java_lang_String_com_google_cloud_bigquery_QueryParameterValue_("corpus", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_string_java_lang_String_(corpus))
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addNamedParameter_java_lang_String_com_google_cloud_bigquery_QueryParameterValue_("min_word_count", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_int64_java_lang_Integer_(minWordCount))
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query with named parameters performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

<br />

To use positional parameters:

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    public class QueryWithPositionalParameters {
      public static void queryWithPositionalParameters() {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          String corpus = "romeoandjuliet";
          long minWordCount = 250;
          String query =
              "SELECT word, word_count\n"
                  + "FROM `bigquery-public-data.samples.shakespeare`\n"
                  + "WHERE corpus = ?\n"
                  + "AND word_count >= ?\n"
                  + "ORDER BY word_count DESC";

          // Note: Standard SQL is required to use query parameters.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addPositionalParameter_com_google_cloud_bigquery_QueryParameterValue_(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_string_java_lang_String_(corpus))
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addPositionalParameter_com_google_cloud_bigquery_QueryParameterValue_(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_int64_java_lang_Integer_(minWordCount))
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query with positional parameters performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

### Node.js

<br />


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To use named parameters:

    // Run a query using named query parameters

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryParamsNamed() {
      // The SQL query to run
      const sqlQuery = `SELECT word, word_count
            FROM \`bigquery-public-data.samples.shakespeare\`
            WHERE corpus = @corpus
            AND word_count >= @min_word_count
            ORDER BY word_count DESC`;

      const options = {
        query: sqlQuery,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
        params: {corpus: 'romeoandjuliet', min_word_count: 250},
      };

      // Run the query
      const [rows] = await bigquery.query(options);

      console.log('Rows:');
      rows.forEach(row => console.log(row));
    }

<br />

To use positional parameters:

    // Run a query using positional query parameters

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryParamsPositional() {
      // The SQL query to run
      const sqlQuery = `SELECT word, word_count
            FROM \`bigquery-public-data.samples.shakespeare\`
            WHERE corpus = ?
            AND word_count >= ?
            ORDER BY word_count DESC`;

      const options = {
        query: sqlQuery,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
        params: ['romeoandjuliet', 250],
      };

      // Run the query
      const [rows] = await bigquery.query(options);

      console.log('Rows:');
      rows.forEach(row => console.log(row));
    }

### Python

<br />


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To use named parameters:

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    query = """
        SELECT word, word_count
        FROM `bigquery-public-data.samples.shakespeare`
        WHERE corpus = @corpus
        AND word_count >= @min_word_count
        ORDER BY word_count DESC;
    """
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        query_parameters=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html("corpus", "STRING", "romeoandjuliet"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html("min_word_count", "INT64", 250),
        ]
    )
    results = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        query, job_config=job_config
    )  # Make an API request.

    for row in results:
        print("{}: \t{}".format(row.word, row.word_count))

<br />

To use positional parameters:

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    query = """
        SELECT word, word_count
        FROM `bigquery-public-data.samples.shakespeare`
        WHERE corpus = ?
        AND word_count >= ?
        ORDER BY word_count DESC;
    """
    # Set the name to None to use positional parameters.
    # Note that you cannot mix named and positional parameters.
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        query_parameters=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html(None, "STRING", "romeoandjuliet"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html(None, "INT64", 250),
        ]
    )
    results = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        query, job_config=job_config
    )  # Make an API request.

    for row in results:
        print("{}: \t{}".format(row.word, row.word_count))

<br />

## Use arrays in parameterized queries

To use an array type in a query parameter, set the type to `ARRAY<T>` where `T`
is the type of the elements in the array. Construct the value as a
comma-separated list of elements enclosed in square brackets, such as `[1, 2,
3]`.

See the [data types reference for more information about the array type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type).

### Console

Arrays in parameterized queries aren't supported by the
Google Cloud console.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. This query selects the most popular names for baby boys born in US states
   starting with the letter W:

   > [!NOTE]
   > **Note:** This example queries a US-based public dataset. Because the public dataset is stored in the US multi-region location, the dataset that contains your destination table must also be in the US. You cannot query a dataset in one location and write the results to a destination table in another location.

   ```bash
   bq query \
      --use_legacy_sql=false \
      --parameter='gender::M' \
      --parameter='states:ARRAY<STRING>:["WA", "WI", "WV", "WY"]' \
      'SELECT
        name,
        SUM(number) AS count
      FROM
        `bigquery-public-data.usa_names.usa_1910_2013`
      WHERE
        gender = @gender
        AND state IN UNNEST(@states)
      GROUP BY
        name
      ORDER BY
        count DESC
      LIMIT
        10;'
   ```

   Be careful to enclose the array type declaration in single quotes so that the
   command output is not accidentally redirected to a file by the `>` character.

### API

To use an array-valued parameter, set the
[`parameterType`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter) to `ARRAY` in
the `query` job configuration.

If the array values are scalars set the [`parameterType`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter)
to the type of the values, such as `STRING`. If the array values are
structures set this to `STRUCT` and add the needed field definitions to
`structTypes`.

For example, this query selects the most popular names for baby boys born in US
states starting with the letter W.

    {
     "query": "SELECT name, sum(number) as count\nFROM `bigquery-public-data.usa_names.usa_1910_2013`\nWHERE gender = @gender\nAND state IN UNNEST(@states)\nGROUP BY name\nORDER BY count DESC\nLIMIT 10;",
     "queryParameters": [
      {
       "parameterType": {
        "type": "STRING"
       },
       "parameterValue": {
        "value": "M"
       },
       "name": "gender"
      },
      {
       "parameterType": {
        "type": "ARRAY",
        "arrayType": {
         "type": "STRING"
        }
       },
       "parameterValue": {
        "arrayValues": [
         {
          "value": "WA"
         },
         {
          "value": "WI"
         },
         {
          "value": "WV"
         },
         {
          "value": "WY"
         }
        ]
       },
       "name": "states"
      }
     ],
     "useLegacySql": false,
     "parameterMode": "NAMED"
    }

[Try it in the Google APIs Explorer](https://developers.google.com/apis-explorer/#p/bigquery/v2/bigquery.jobs.query?projectId=my-project-id&_h=1&resource=%257B%250A++%2522query%2522%253A+%2522SELECT+name%252C+sum(number)+as+count%255CnFROM+%2560bigquery-public-data.usa_names.usa_1910_2013%2560%255CnWHERE+gender+%253D+%2540gender%255CnAND+state+IN+UNNEST(%2540states)%255CnGROUP+BY+name%255CnORDER+BY+count+DESC%255CnLIMIT+10%253B%2522%252C%250A++%2522queryParameters%2522%253A+%250A++%255B%250A++++%257B%250A++++++%2522parameterType%2522%253A+%250A++++++%257B%250A++++++++%2522type%2522%253A+%2522STRING%2522%250A++++++%257D%252C%250A++++++%2522parameterValue%2522%253A+%250A++++++%257B%250A++++++++%2522value%2522%253A+%2522M%2522%250A++++++%257D%252C%250A++++++%2522name%2522%253A+%2522gender%2522%250A++++%257D%252C%250A++++%257B%250A++++++%2522parameterType%2522%253A+%250A++++++%257B%250A++++++++%2522type%2522%253A+%2522ARRAY%2522%252C%250A++++++++%2522arrayType%2522%253A+%250A++++++++%257B%250A++++++++++%2522type%2522%253A+%2522STRING%2522%250A++++++++%257D%250A++++++%257D%252C%250A++++++%2522parameterValue%2522%253A+%250A++++++%257B%250A++++++++%2522arrayValues%2522%253A+%250A++++++++%255B%250A++++++++++%257B%250A++++++++++++%2522value%2522%253A+%2522WA%2522%250A++++++++++%257D%252C%250A++++++++++%257B%250A++++++++++++%2522value%2522%253A+%2522WI%2522%250A++++++++++%257D%252C%250A++++++++++%257B%250A++++++++++++%2522value%2522%253A+%2522WV%2522%250A++++++++++%257D%252C%250A++++++++++%257B%250A++++++++++++%2522value%2522%253A+%2522WY%2522%250A++++++++++%257D%250A++++++++%255D%250A++++++%257D%252C%250A++++++%2522name%2522%253A+%2522states%2522%250A++++%257D%250A++%255D%252C%250A++%2522useLegacySql%2522%253A+false%252C%250A++%2522parameterMode%2522%253A+%2522NAMED%2522%250A%257D&).

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

    public class BigQueryQueryWithArrayParameters
    {
        public void QueryWithArrayParameters(string projectId = "your-project-id")
        {
            var gender = "M";
            string[] states = { "WA", "WI", "WV", "WY" };

            // Note: Standard SQL is required to use query parameters.
            var query = @"
                SELECT name, sum(number) as count
                FROM `bigquery-public-data.usa_names.usa_1910_2013`
                WHERE gender = @gender
                AND state IN UNNEST(@states)
                GROUP BY name
                ORDER BY count DESC
                LIMIT 10;";

            // Initialize client that will be used to send requests.
            var client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);

            var parameters = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html[]
            {
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html("gender", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String, gender),
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html("states", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_Array, states)
            };

            var job = client.CreateQueryJob(
                sql: query,
                parameters: parameters,
                options: new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.QueryOptions.html { UseQueryCache = false });
            // Wait for the job to complete.
            job = job.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();
            // Display the results
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryRow.html row in client.GetQueryResults(job.Reference))
            {
                Console.WriteLine($"{row["name"]}: {row["count"]}");
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

    // queryWithArrayParams demonstrates issuing a query and specifying query parameters that include an
    // array of strings.
    func queryWithArrayParams(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	q := client.Query(
    		`SELECT
    			name,
    			sum(number) as count 
            FROM ` + "`bigquery-public-data.usa_names.usa_1910_2013`" + `
    		WHERE
    			gender = @gender
            	AND state IN UNNEST(@states)
    		GROUP BY
    			name
    		ORDER BY
    			count DESC
    		LIMIT 10;`)
    	q.Parameters = []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryParameter{
    		{
    			Name:  "gender",
    			Value: "M",
    		},
    		{
    			Name:  "states",
    			Value: []string{"WA", "WI", "WV", "WY"},
    		},
    	}
    	// Run the query and print results when the query job is completed.
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	it, err := job.Read(ctx)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to running a query with array query parameters.
    public class QueryWithArrayParameters {

      public static void runQueryWithArrayParameters() {
        String gender = "M";
        String[] states = {"WA", "WI", "WV", "WY"};
        String query =
            "SELECT name, sum(number) as count\n"
                + "FROM `bigquery-public-data.usa_names.usa_1910_2013`\n"
                + "WHERE gender = @gender\n"
                + "AND state IN UNNEST(@states)\n"
                + "GROUP BY name\n"
                + "ORDER BY count DESC\n"
                + "LIMIT 10;";
        queryWithArrayParameters(query, gender, states);
      }

      public static void queryWithArrayParameters(String query, String gender, String[] states) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Note: Standard SQL is required to use query parameters.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addNamedParameter_java_lang_String_com_google_cloud_bigquery_QueryParameterValue_("gender", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_string_java_lang_String_(gender))
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addNamedParameter_java_lang_String_com_google_cloud_bigquery_QueryParameterValue_("states", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue__T_array_T___com_google_cloud_bigquery_StandardSQLTypeName_(states, String.class))
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          // Print the results.
          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));
          System.out.println("Query with arrays parameters performed successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
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

    // Run a query using array query parameters

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryParamsArrays() {
      // The SQL query to run
      const sqlQuery = `SELECT name, sum(number) as count
      FROM \`bigquery-public-data.usa_names.usa_1910_2013\`
      WHERE gender = @gender
      AND state IN UNNEST(@states)
      GROUP BY name
      ORDER BY count DESC
      LIMIT 10;`;

      const options = {
        query: sqlQuery,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
        params: {gender: 'M', states: ['WA', 'WI', 'WV', 'WY']},
      };

      // Run the query
      const [rows] = await bigquery.query(options);

      console.log('Rows:');
      rows.forEach(row => console.log(row));
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

    query = """
        SELECT name, sum(number) as count
        FROM `bigquery-public-data.usa_names.usa_1910_2013`
        WHERE gender = @gender
        AND state IN UNNEST(@states)
        GROUP BY name
        ORDER BY count DESC
        LIMIT 10;
    """
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        query_parameters=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html("gender", "STRING", "M"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ArrayQueryParameter.html("states", "STRING", ["WA", "WI", "WV", "WY"]),
        ]
    )
    rows = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(query, job_config=job_config)  # Make an API request.

    for row in rows:
        print("{}: \t{}".format(row.name, row.count))

## Use timestamps in parameterized queries

To use a timestamp in a query parameter, the underlying REST API takes a value
of type `TIMESTAMP` in the format `YYYY-MM-DD HH:MM:SS.DDDDDD time_zone`. If you
are using the client libraries, you create a built-in date object in that
language, and the library converts it to the right format. For more information,
see the following language-specific examples.

For more information about the `TIMESTAMP` type, see the
[data types reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type).

### Console

Follow the steps for [adding parameters in the Google Cloud console](https://docs.cloud.google.com/bigquery/docs/parameterized-queries#add-parameters-in-console)
described earlier in this document. Select `TIMESTAMP` for the parameter
type and enter the timestamp value in the format
`YYYY-MM-DD HH:MM:SS.DDDDDD time_zone`.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. This query adds an hour to the timestamp parameter value:

   ```bash
   bq query \
      --use_legacy_sql=false \
      --parameter='ts_value:TIMESTAMP:2016-12-07 08:00:00' \
      'SELECT
        TIMESTAMP_ADD(@ts_value, INTERVAL 1 HOUR);'
   ```

### API

To use a timestamp parameter, set the
[`parameterType`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter)
to `TIMESTAMP` in the query job configuration.

This query adds an hour to the timestamp parameter value.

    {
      "query": "SELECT TIMESTAMP_ADD(@ts_value, INTERVAL 1 HOUR);",
      "queryParameters": [
        {
          "name": "ts_value",
          "parameterType": {
            "type": "TIMESTAMP"
          },
          "parameterValue": {
            "value": "2016-12-07 08:00:00"
          }
        }
      ],
      "useLegacySql": false,
      "parameterMode": "NAMED"
    }

[Try it in the Google APIs Explorer](https://developers.google.com/apis-explorer/#p/bigquery/v2/bigquery.jobs.query?projectId=my-project-id&_h=1&resource=%257B%250A++%2522query%2522%253A+%2522SELECT+TIMESTAMP_ADD(%2540ts_value%252C+INTERVAL+1+HOUR)%253B%2522%252C%250A++%2522queryParameters%2522%253A+%250A++%255B%250A++++%257B%250A++++++%2522name%2522%253A+%2522ts_value%2522%252C%250A++++++%2522parameterType%2522%253A+%250A++++++%257B%250A++++++++%2522type%2522%253A+%2522TIMESTAMP%2522%250A++++++%257D%252C%250A++++++%2522parameterValue%2522%253A+%250A++++++%257B%250A++++++++%2522value%2522%253A+%25222016-12-07+08%253A00%253A00%2522%250A++++++%257D%250A++++%257D%250A++%255D%252C%250A++%2522useLegacySql%2522%253A+false%252C%250A++%2522parameterMode%2522%253A+%2522NAMED%2522%250A%257D&).

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

    public class BigQueryQueryWithTimestampParameters
    {
        public void QueryWithTimestampParameters(string projectId = "project-id")
        {
            var timestamp = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_DateTime(2016, 12, 7, 8, 0, 0, DateTimeKind.Utc);

            // Note: Standard SQL is required to use query parameters.
            var query = "SELECT TIMESTAMP_ADD(@ts_value, INTERVAL 1 HOUR);";

            // Initialize client that will be used to send requests.
            var client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);

            var parameters = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html[]
            {
                new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter.html("ts_value", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_Timestamp, timestamp),
            };

            var job = client.CreateQueryJob(
                sql: query,
                parameters: parameters,
                options: new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.QueryOptions.html { UseQueryCache = false });
            // Wait for the job to complete.
            job = job.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();
            // Display the results
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryRow.html row in client.GetQueryResults(job.Reference))
            {
                Console.WriteLine(row[0]);
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
    	"time"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // queryWithTimestampParam demonstrates issuing a query and supplying a timestamp query parameter.
    func queryWithTimestampParam(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	q := client.Query(
    		`SELECT TIMESTAMP_ADD(@ts_value, INTERVAL 1 HOUR);`)
    	q.Parameters = []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryParameter{
    		{
    			Name:  "ts_value",
    			Value: time.Date(2016, 12, 7, 8, 0, 0, 0, time.UTC),
    		},
    	}
    	// Run the query and print results when the query job is completed.
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	it, err := job.Read(ctx)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import org.threeten.bp.LocalDateTime;
    import org.threeten.bp.ZoneOffset;
    import org.threeten.bp.ZonedDateTime;

    // Sample to running a query with timestamp query parameters.
    public class QueryWithTimestampParameters {

      public static void runQueryWithTimestampParameters() {
        queryWithTimestampParameters();
      }

      public static void queryWithTimestampParameters() {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          ZonedDateTime timestamp = LocalDateTime.of(2016, 12, 7, 8, 0, 0).atZone(ZoneOffset.UTC);
          String query = "SELECT TIMESTAMP_ADD(@ts_value, INTERVAL 1 HOUR);";
          // Note: Standard SQL is required to use query parameters.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addNamedParameter_java_lang_String_com_google_cloud_bigquery_QueryParameterValue_(
                      "ts_value",
                      https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_timestamp_java_lang_Long_(
                          // Timestamp takes microseconds since 1970-01-01T00:00:00 UTC
                          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_timestamp_java_lang_Long_.toInstant().toEpochMilli() * 1000))
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s", val.toString())));

          System.out.println("Query with timestamp parameter performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
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

    // Run a query using timestamp parameters

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryParamsTimestamps() {
      // The SQL query to run
      const sqlQuery = `SELECT TIMESTAMP_ADD(@ts_value, INTERVAL 1 HOUR);`;

      const options = {
        query: sqlQuery,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
        params: {ts_value: new Date()},
      };

      // Run the query
      const [rows] = await bigquery.query(options);

      console.log('Rows:');
      rows.forEach(row => console.log(row.f0_));
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

    import datetime

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    query = "SELECT TIMESTAMP_ADD(@ts_value, INTERVAL 1 HOUR);"
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        query_parameters=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html(
                "ts_value",
                "TIMESTAMP",
                datetime.datetime(2016, 12, 7, 8, 0, tzinfo=datetime.timezone.utc),
            )
        ]
    )
    results = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        query, job_config=job_config
    )  # Make an API request.

    for row in results:
        print(row)

## Use structs in parameterized queries

To use a struct in a query parameter, set the type to `STRUCT<T>` where `T`
defines the fields and types within the struct. Field definitions are
separated by commas and are of the form `field_name TF` where `TF` is the type
of the field. For example, `STRUCT<x INT64, y STRING>` defines a struct with a
field named `x` of type `INT64` and a second field named `y` of type `STRING`.

For more information about the `STRUCT` type, see the
[data types reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type).

### Console

Structs in parameterized queries aren't supported by the
Google Cloud console.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. This trivial query demonstrates the use of structured types by returning the
   parameter value:

   ```bash
   bq query \
      --use_legacy_sql=false \
      --parameter='struct_value:STRUCT<x INT64, y STRING>:{"x": 1, "y": "foo"}' \
      'SELECT
        @struct_value AS s;'
   ```

### API

To use a struct parameter, set the
[`parameterType`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType)
to `STRUCT` in the query job configuration.

Add an object for each field of the struct to
[`structTypes`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType)
in the job's `queryParameters`.
If the struct values are scalars set the
[`type`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType)
to the type of the values, such as `STRING`. If the struct values are arrays
set this to `ARRAY`, and set the nested `arrayType` field to the appropriate
type. If the struct values are structures set `type` to `STRUCT` and add the
needed `structTypes`.

This trivial query demonstrates the use of structured types by returning the
parameter value.

    {
      "query": "SELECT @struct_value AS s;",
      "queryParameters": [
        {
          "name": "struct_value",
          "parameterType": {
            "type": "STRUCT",
            "structTypes": [
              {
                "name": "x",
                "type": {
                  "type": "INT64"
                }
              },
              {
                "name": "y",
                "type": {
                  "type": "STRING"
                }
              }
            ]
          },
          "parameterValue": {
            "structValues": {
              "x": {
                "value": "1"
              },
              "y": {
                "value": "foo"
              }
            }
          }
        }
      ],
      "useLegacySql": false,
      "parameterMode": "NAMED"
    }

[Try it in the Google APIs Explorer](https://developers.google.com/apis-explorer/#p/bigquery/v2/bigquery.jobs.query?projectId=my-project-id&_h=1&resource=%257B%250A++%2522query%2522%253A+%2522SELECT+%2540struct_value+AS+s%253B%2522%252C%250A++%2522queryParameters%2522%253A+%250A++%255B%250A++++%257B%250A++++++%2522name%2522%253A+%2522struct_value%2522%252C%250A++++++%2522parameterType%2522%253A+%250A++++++%257B%250A++++++++%2522type%2522%253A+%2522STRUCT%2522%252C%250A++++++++%2522structTypes%2522%253A+%250A++++++++%255B%250A++++++++++%257B%250A++++++++++++%2522name%2522%253A+%2522x%2522%252C%250A++++++++++++%2522type%2522%253A+%250A++++++++++++%257B%250A++++++++++++++%2522type%2522%253A+%2522INT64%2522%250A++++++++++++%257D%250A++++++++++%257D%252C%250A++++++++++%257B%250A++++++++++++%2522name%2522%253A+%2522y%2522%252C%250A++++++++++++%2522type%2522%253A+%250A++++++++++++%257B%250A++++++++++++++%2522type%2522%253A+%2522STRING%2522%250A++++++++++++%257D%250A++++++++++%257D%250A++++++++%255D%250A++++++%257D%252C%250A++++++%2522parameterValue%2522%253A+%250A++++++%257B%250A++++++++%2522structValues%2522%253A+%250A++++++++%257B%250A++++++++++%2522x%2522%253A+%250A++++++++++%257B%250A++++++++++++%2522value%2522%253A+%25221%2522%250A++++++++++%257D%252C%250A++++++++++%2522y%2522%253A+%250A++++++++++%257B%250A++++++++++++%2522value%2522%253A+%2522foo%2522%250A++++++++++%257D%250A++++++++%257D%250A++++++%257D%250A++++%257D%250A++%255D%252C%250A++%2522useLegacySql%2522%253A+false%252C%250A++%2522parameterMode%2522%253A+%2522NAMED%2522%250A%257D&).

### C#

The [BigQuery client library for .NET](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest)
does not support
[struct parameters](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryParameter).

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

    // queryWithStructParam demonstrates running a query and providing query parameters that include struct
    // types.
    func queryWithStructParam(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	type MyStruct struct {
    		X int64
    		Y string
    	}
    	q := client.Query(
    		`SELECT @struct_value as s;`)
    	q.Parameters = []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryParameter{
    		{
    			Name:  "struct_value",
    			Value: MyStruct{X: 1, Y: "foo"},
    		},
    	}
    	// Run the query and print results when the query job is completed.
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	it, err := job.Read(ctx)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import java.util.HashMap;
    import java.util.Map;

    public class QueryWithStructsParameters {

      public static void queryWithStructsParameters() {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Create struct
          Map<String, QueryParameterValue> struct = new HashMap<>();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_struct_java_util_Map_java_lang_String_com_google_cloud_bigquery_QueryParameterValue__.put("booleanField", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_bool_java_lang_Boolean_(true));
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_struct_java_util_Map_java_lang_String_com_google_cloud_bigquery_QueryParameterValue__.put("integerField", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_string_java_lang_String_("test-stringField"));
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_struct_java_util_Map_java_lang_String_com_google_cloud_bigquery_QueryParameterValue__.put("stringField", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_int64_java_lang_Integer_(10));
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html recordValue = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_struct_java_util_Map_java_lang_String_com_google_cloud_bigquery_QueryParameterValue__(struct);

          String query = "SELECT STRUCT(@recordField) AS record";
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .setUseLegacySql(false)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addNamedParameter_java_lang_String_com_google_cloud_bigquery_QueryParameterValue_("recordField", recordValue)
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s", val.toString())));

          System.out.println("Query with struct parameter performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
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

    // Run a query using struct query parameters

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryParamsStructs() {
      // The SQL query to run
      const sqlQuery = `SELECT @struct_value AS struct_obj;`;

      const options = {
        query: sqlQuery,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
        params: {struct_value: {x: 1, y: 'foo'}},
      };

      // Run the query
      const [rows] = await bigquery.query(options);

      console.log('Rows:');
      rows.forEach(row => console.log(row.struct_obj.y));
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

    query = "SELECT @struct_value AS s;"
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        query_parameters=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.StructQueryParameter.html(
                "struct_value",
                https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html("x", "INT64", 1),
                https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html("y", "STRING", "foo"),
            )
        ]
    )
    results = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_query_and_wait(
        query, job_config=job_config
    )  # Make an API request and waits for results.

    for row in results:
        print(row.s)

## Use ranges in parameterized queries

To use a range in a query parameter, set the `type` field to `RANGE`.

For more information about the `RANGE` type, see the
[data types reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#range_type).

### Console

Ranges in parameterized queries aren't supported by the
Google Cloud console.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. This query demonstrates the use of range types by returning the
   parameter value:

   ```bash
   bq query \
      --use_legacy_sql=false \
      --parameter='my_param:RANGE<DATE>:[2020-01-01, 2020-12-31)' \
      'SELECT @my_param AS foo;'
   ```

### API

To use a range parameter, in the
[`parameterType`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType)
set the `type` field to `RANGE` and set the
`rangeElementType` field to the type of range you want to use.

This query shows how to use the `RANGE` parameter type by returning the
parameter value.

    {
      "query": "SELECT @my_param AS value_of_range_parameter;",
      "queryParameters": [
        {
          "name": "range_param",
          "parameterType": {
            "type": "RANGE",
            "rangeElementTYpe": {
              "type": "DATE"
            }
          },
          "parameterValue": {
            "rangeValue": {
              "start": {
                "value": "2020-01-01"
              },
              "end": {
                "value": "2020-12-31"
              }
            }
          }
        }
      ],
      "useLegacySql": false,
      "parameterMode": "NAMED"
    }

## What's next

- Learn about [verified parameterized queries](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-param-verified-queries) in BigQuery conversational analytics.