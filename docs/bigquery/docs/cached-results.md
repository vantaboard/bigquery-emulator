# Using cached query results

BigQuery writes all query results to a table. The table is either
explicitly identified by the user (a destination table), or it is a temporary,
cached results table. If you run the exact same query again,
BigQuery returns the results from
the cached table, if it exists.
Temporary, cached results tables are maintained per-user,
per-project. Depending on your edition, you might have access to
[cached results from other users](https://docs.cloud.google.com/bigquery/docs/cached-results#cross-user-caching) running queries in the
same project. There are no storage costs for cached query result tables, but if
you write query results to a permanent table, you are charged for
[storing](https://cloud.google.com/bigquery/pricing#storage) the data.

All query results, including both
[interactive and batch queries](https://docs.cloud.google.com/bigquery/docs/running-queries),
are cached in temporary tables for approximately 24 hours with some
[exceptions](https://docs.cloud.google.com/bigquery/docs/cached-results#cache-exceptions).

## Limitations

Using the query cache is subject to the following limitations:

- When you run a duplicate query, BigQuery attempts to reuse cached results. To retrieve data from the cache, the duplicate query text must be the same as the original query.
- For query results to persist in a cached results table, the result set must be smaller than the maximum response size. For more information about managing large result sets, see [Returning large query results](https://docs.cloud.google.com/bigquery/docs/writing-results#large-results).
- You cannot target cached result tables with [DML](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language) statements.
- Although current semantics allow it, the use of cached results as input for dependent jobs is discouraged. For example, you shouldn't submit query jobs that retrieve results from the cache table. Instead, write your results to a named destination table. To simplify cleanup, features such as the dataset level `defaultTableExpirationMs` property can expire the data automatically after a given duration.

## Pricing and quotas

Cached query results are stored as temporary tables. You aren't charged for the
storage of cached query results in temporary tables. When query results are
retrieved from a cached results table, the job statistics property
`statistics.query.cacheHit` returns as `true`, and you are not charged for the
query. Though you are not charged for queries that use cached results, the
queries are subject to the BigQuery
[quota policies](https://docs.cloud.google.com/bigquery/quota-policy).

In addition to reducing costs, queries that use cached results are
significantly faster because BigQuery does not need to compute
the result set.

## Exceptions to query caching

Query results are not cached:

- When a destination table is specified in the job configuration, the Google Cloud console, the bq command-line tool, or the API.
- If any of the referenced tables or logical views have changed since the results were previously cached.
- When any of the tables referenced by the query have recently received streaming inserts (table has data in the write-optimized storage) even if no new rows have arrived.
- If the query uses non-deterministic functions; for example, date and time functions such as `CURRENT_TIMESTAMP()` and `CURRENT_DATE`, and other functions such as `SESSION_USER()`, it returns different values depending on when a query is executed.
- If you are querying multiple tables using a [wildcard](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables).
- If the cached results have expired; typical cache lifetime is 24 hours, but the cached results are best-effort and may be invalidated sooner.
- If the query runs against an [external data source](https://docs.cloud.google.com/bigquery/external-data-sources). other than Cloud Storage. (GoogleSQL queries on Cloud Storage are supported by cached query results.)
- If the query runs against a table protected by [row-level security](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features), then the results are not cached.
- If the query runs against a table protected by [column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro), including data masking, results might not be cached.
- If the query text has changed in any way, including modified whitespace or comments.

## How cached results are stored

When you run a query, a temporary, cached results table is created in a special
type of [hidden dataset](https://docs.cloud.google.com/bigquery/docs/datasets#hidden_datasets) referred to as
an *anonymous dataset* . Unlike regular datasets which inherit permissions from
the IAM resource hierarchy model (project and organization
permissions), access to anonymous datasets is restricted to the owner. The owner
of an anonymous dataset is the user who ran the query that produced the cached
result. In addition, the `bigquery.jobs.create` permission is checked on the
project to verify that the user has access to the project.

BigQuery doesn't support sharing anonymous datasets. If you
intend to share
query results, don't use the cached results stored in an anonymous dataset.
Instead, write the results to a named destination table.

Although the user that runs the query has full access to the dataset and the
cached results table, using them as inputs for dependent jobs is discouraged.

The names of anonymous datasets begin with an underscore.
This hides them from the datasets list in the Google Cloud console.
You can list anonymous datasets and audit anonymous dataset access controls by
using the bq command-line tool or the API.

For more information about listing and getting information about datasets,
including anonymous datasets,
see [Listing datasets](https://docs.cloud.google.com/bigquery/docs/listing-datasets).

## Cross-user caching

If you have the required permissions to execute a query, the results of which
are cached in your project for another user, then BigQuery
returns results from the cache. The cached result is copied into your personal
anonymous dataset and remains there for 24 hours from when you ran the query.

Cross-user caching is available if you are using the Enterprise or
Enterprise Plus [edition](https://docs.cloud.google.com/bigquery/docs/editions-intro). The same
[limits and exceptions](https://docs.cloud.google.com/bigquery/docs/cached-results#cache-exceptions) for single-user caching apply to
cross-user caching.

## Disabling retrieval of cached results

The **Use cached results** option reuses results from a previous run of the
same query unless the tables being queried have changed. Using cached results is
only beneficial for repeated queries. For new queries, the **Use cached
results** option has no effect, though it is enabled by default.

When you repeat a query with the **Use cached results** option disabled,
the existing cached result is overwritten. This requires BigQuery
to compute the query result, and you are charged for the query. This is
particularly useful in benchmarking scenarios.

If you want to disable retrieving cached results and force live evaluation of a
query job, you can set the `configuration.query.useQueryCache`
property of your query job to `false`.

To disable the **Use cached results** option:

### Console

1. Open the Google Cloud console.  

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)

2. Click **Compose new query**.

3. Enter a valid SQL query in the **Query editor** text area.

4. Click **More** and select **Query settings**.


   ![Query settings](https://docs.cloud.google.com/static/bigquery/images/query-settings.png)
5. For **Cache preference** , clear **Use cached results**.

### bq

Use the `nouse_cache` flag to overwrite the query cache. The following
example forces BigQuery to process the query without using
the existing cached results:

     bq query \
     --nouse_cache \
     --batch \
     'SELECT
        name,
        count
      FROM
        `my-project`.mydataset.names_2013
      WHERE
        gender = "M"
      ORDER BY
        count DESC
      LIMIT
        6'

### API

To process a query without using the existing cached results, set the
`useQueryCache` property to `false` in the `query` job configuration.

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

    // queryDisableCache demonstrates issuing a query and requesting that the query cache is bypassed.
    func queryDisableCache(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	q := client.Query(
    		"SELECT corpus FROM `bigquery-public-data.samples.shakespeare` GROUP BY corpus;")
    	q.DisableQueryCache = true
    	// Location must match that of the dataset(s) referenced in the query.
    	q.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_Location = "US"

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

To process a query without using the existing cached results,
[set use query cache](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder#com_google_cloud_bigquery_QueryJobConfiguration_Builder_setUseQueryCache_java_lang_Boolean_)
to `false` when creating a
[QueryJobConfiguration](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration).


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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to running a query with the cache disabled.
    public class QueryDisableCache {

      public static void runQueryDisableCache() {
        String query = "SELECT corpus FROM `bigquery-public-data.samples.shakespeare` GROUP BY corpus;";
        queryDisableCache(query);
      }

      public static void queryDisableCache(String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  // Disable the query cache to force live query evaluation.
                  .setUseQueryCache(false)
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query disable cache performed successfully.");
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

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');

    async function queryDisableCache() {
      // Queries the Shakespeare dataset with the cache disabled.

      // Create a client
      const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

      const query = `SELECT corpus
        FROM \`bigquery-public-data.samples.shakespeare\`
        GROUP BY corpus`;
      const options = {
        query: query,
        // Location must match that of the dataset(s) referenced in the query.
        location: 'US',
        useQueryCache: false,
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
    // $projectId = 'The Google project ID';
    // $query = 'SELECT id, view_count FROM `bigquery-public-data.stackoverflow.posts_questions`';

    // Construct a BigQuery client object.
    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);

    // Set job configs
    $jobConfig = $bigQuery->query($query);
    $jobConfig->useQueryCache(false);

    // Extract query results
    $queryResults = $bigQuery->runQuery($jobConfig);

    $i = 0;
    foreach ($queryResults as $row) {
        printf('--- Row %s ---' . PHP_EOL, ++$i);
        foreach ($row as $column => $value) {
            printf('%s: %s' . PHP_EOL, $column, json_encode($value));
        }
    }
    printf('Found %s row(s)' . PHP_EOL, $i);

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

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(use_query_cache=False)
    sql = """
        SELECT corpus
        FROM `bigquery-public-data.samples.shakespeare`
        GROUP BY corpus;
    """
    query_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql, job_config=job_config)  # Make an API request.

    for row in query_job:
        print(row)

## Ensuring use of the cache

If you use the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert) method
to run a query, you can force a query job to fail unless cached results can be
used by setting the `createDisposition` property of the `query` job
configuration to `CREATE_NEVER`.

If the query result does not exist in the cache, a `NOT_FOUND` error is
returned.

### bq

Use the `--require_cache` flag to require results from the query cache. The
following example forces BigQuery to process the query if its
results exist in the cache:

     bq query \
     --require_cache \
     --batch \
     'SELECT
        name,
        count
      FROM
        `my-project`.mydataset.names_2013
      WHERE
        gender = "M"
      ORDER BY
        count DESC
      LIMIT
        6'

### API

To process a query with existing cached results, set the
[`createDisposition`
property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationQuery.FIELDS.create_disposition)
to `CREATE_NEVER` in the `query` job configuration.

## Verifying use of the cache

Use one of the following methods to determine if BigQuery returned a result using the cache:

- **Use the Google Cloud console** . Go to **Query results** and click **Job Information** . **Bytes processed** shows **0 B (results cached)**.
- **Use the [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/v2).** The `cacheHit` property in the query result is set to `true`.

## Impact of column-level security

By default, BigQuery caches query results for 24 hours, with the
[exceptions](https://docs.cloud.google.com/bigquery/docs/cached-results#cache-exceptions) noted previously. Queries against a table
protected by [column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) might not be cached. If
BigQuery does cache the result, the 24-hour cache lifetime
applies.

A change such as removing a group or a user from the Data Catalog
Fine-Grained Reader role used for a policy tag does not invalidate the 24-hour
cache. A change to the Data Catalog Fine-Grained Reader access control
group itself is propagated immediately, but the change does not invalidate the
cache.

The impact is if a user ran a query, the query results remain visible to the
user. The user can also retrieve those results from the cache even if they lost
access to the data within the last 24 hours.

During the 24 hours after a user is removed from the Data Catalog
Fine-Grained Reader role for a policy tag, the user can access the cached
data only for data that the user was previously allowed to see. If rows are
added to the table, the user can't see the added rows, even if the results
are cached.