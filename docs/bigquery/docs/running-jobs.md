# Running jobs programmatically

To run a BigQuery job programmatically using the REST API or
client libraries, you:

1. Call the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) method.
2. Periodically request the job resource and examine the status property to learn when the job is complete.
3. Check to see whether the job finished successfully.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document.

### Required permissions

To run a BigQuery job, you need the `bigquery.jobs.create` IAM permission.

Each of the following predefined IAM roles includes the permissions that you need in order to run a job:

- `roles/bigquery.user`
- `roles/bigquery.jobUser`
- `roles/bigquery.admin`

Additionally, when you create a job, you are automatically granted the following permissions for that job:

- `bigquery.jobs.get`
- `bigquery.jobs.update`

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Running jobs

To run a job programmatically:

1. Start the job by calling the `jobs.insert` method. When you call the
   `jobs.insert` method, include a [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs)
   representation.

2. In the [`configuration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfiguration)
   section of the job resource, include a child property that specifies the job
   type --- `load`, `query`, `extract`, or `copy`.

3. After calling the `jobs.insert` method, check the job status by calling
   `jobs.get` with the job ID and location, and check the `status.state`
   value to learn the job status. When `status.state` is `DONE`, the job has
   stopped running; however, a `DONE` status does not mean that the job
   completed successfully, only that it is no longer running.

   > [!NOTE]
   > **Note:** There are some wrapper functions that manage job status requests for you. For example, running `jobs.query` creates a job and periodically polls for `DONE` status for a specified period of time.

4. Check for job success. If the job has an `errorResult` property, the job has
   failed. The `status.errorResult` property holds information describing what went
   wrong in a failed job. If `status.errorResult` is absent, the job finished
   successfully, although there might have been some nonfatal errors, such as
   problems importing a few rows in a load job. Nonfatal errors are returned in
   the job's `status.errors` list.

## Running jobs using client libraries

To create and run a job using the Cloud Client Libraries for
BigQuery:

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

    public class BigQueryCreateJob
    {
        public BigQueryJob CreateJob(string projectId = "your-project-id")
        {
            string query = @"
                SELECT country_name from `bigquery-public-data.utility_us.country_code_iso";

            // Initialize client that will be used to send requests.
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);

            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.QueryOptions.html queryOptions = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.QueryOptions.html
            {
                JobLocation = "us",
                JobIdPrefix = "code_sample_",
                Labels = new Dictionary<string, string>
                {
                    ["example-label"] = "example-value"
                },
                MaximumBytesBilled = 1000000
            };

            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html queryJob = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_CreateQueryJob_System_String_System_Collections_Generic_IEnumerable_Google_Cloud_BigQuery_V2_BigQueryParameter__Google_Cloud_BigQuery_V2_QueryOptions_(
                sql: query,
                parameters: null,
                options: queryOptions);

            Console.WriteLine($"Started job: {queryJob.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_Reference.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.JobCreationOptions.html#Google_Cloud_BigQuery_V2_JobCreationOptions_JobId}");
            return queryJob;
        }
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.common.collect.ImmutableMap;
    import java.util.UUID;

    // Sample to create a job
    public class CreateJob {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String query = "SELECT country_name from `bigquery-public-data.utility_us.country_code_iso`";
        createJob(query);
      }

      public static void createJob(String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Specify a job configuration to set optional job resource properties.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .setLabels(ImmutableMap.of("example-label", "example-value"))
                  .build();

          // The location and job name are optional,
          // if both are not specified then client will auto-create.
          String jobName = "jobId_" + UUID.randomUUID().toString();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html jobId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html.newBuilder().setLocation("us").https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.Builder.html#com_google_cloud_bigquery_JobId_Builder_setJob_java_lang_String_(jobName).build();

          // Create a job with job ID
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(jobId, queryConfig));

          // Get a job that was just created
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getJob_com_google_cloud_bigquery_JobId_com_google_cloud_bigquery_BigQuery_JobOption____(jobId);
          if (job.getJobId().getJob().equals(jobId.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html#com_google_cloud_bigquery_JobId_getJob__())) {
            System.out.print("Job created successfully." + job.getJobId().getJob());
          } else {
            System.out.print("Job was not created");
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.print("Job was not created. \n" + e.toString());
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

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    query_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(
        "SELECT country_name from `bigquery-public-data.utility_us.country_code_iso`",
        # Explicitly force job execution to be routed to a specific processing
        # location.
        location="US",
        # Specify a job configuration to set optional job resource properties.
        job_config=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
            labels={"example-label": "example-value"}, maximum_bytes_billed=1000000
        ),
        # The client libraries automatically generate a job ID. Override the
        # generated ID with either the job_id_prefix or job_id parameters.
        job_id_prefix="code_sample_",
    )  # Make an API request.

    print("Started job: {}".format(https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dbapi.Cursor.html#google_cloud_bigquery_dbapi_Cursor_query_job.job_id))

<br />

## Adding job labels

Labels can be added to query jobs through the command line by using the
bq command-line tool's `--label` flag. The bq tool supports adding
labels only to query jobs.

You can also add a label to a job when it's submitted through the API by specifying
the `labels` property in the job configuration when you call the
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) method. The API
can be used to add labels to any job type.

You cannot add labels to or update labels on pending, running, or completed
jobs.

When you add a label to a job, the label is included in your billing data.

For more information, see
[Adding job labels](https://docs.cloud.google.com/bigquery/docs/adding-labels#job-label).

## What's next

- See [Running queries](https://docs.cloud.google.com/bigquery/docs/running-queries#batch) for a code example that starts and polls a query job.
- For more information on creating a job resource representation, see the [Jobs overview page](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs) in the API reference.