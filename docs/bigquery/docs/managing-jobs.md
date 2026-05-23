# Manage jobs

This document describes how to manage jobs in BigQuery,
including how to [view job details](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job),
[list jobs](https://docs.cloud.google.com/bigquery/docs/managing-jobs#list_jobs_in_a_project), [cancel a job](https://docs.cloud.google.com/bigquery/docs/managing-jobs#cancel_jobs),
[repeat a job](https://docs.cloud.google.com/bigquery/docs/managing-jobs#repeat_jobs),
and [delete job metadata](https://docs.cloud.google.com/bigquery/docs/managing-jobs#delete_job_metadata).

## About BigQuery jobs

Every time you [load](https://docs.cloud.google.com/bigquery/docs/loading-data),
[export](https://docs.cloud.google.com/bigquery/exporting-data-from-bigquery),
[query](https://docs.cloud.google.com/bigquery/docs/running-queries), or
[copy data](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table),
BigQuery automatically creates, schedules, and runs a job
that tracks the progress of the task.

Because jobs can potentially take a long time to complete, they run asynchronously and can be
polled for their status. Shorter actions, such as listing resources or getting metadata, are not
managed as jobs.

When a job is submitted, it can be in one of the following states:

- `PENDING`: The job is scheduled and waiting to be run.
- `RUNNING`: The job is in progress.
- `DONE`: The job is completed. If the job failed, the [JobStatus.errorResult](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobStatus.FIELDS.error_result) will be present.

### Quotas

For information about job quotas, see the documentation for the job type on the
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas) page:

- [Load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs)
- [Copy jobs](https://docs.cloud.google.com/bigquery/quotas#copy_jobs)
- [Extract jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs)
- [Query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs)

### Pricing

Every job is associated with a specific project that you specify. The billing
account attached to the associated project is billed for any usage incurred by
the job. If you share access to a project, any jobs run in the project are
also billed to the billing account.

For example, when running a query job,
the cost is billed to the project that runs the job. Thus when you view the job
ID of a query job with the format of `<project_id>:<region>.<job_id>`, the
`project_id` is the ID of the project billed for the query.

For more information, see [Pricing](https://cloud.google.com/bigquery/pricing).

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary
permissions to perform each task in this document.

### Required roles


To get the permissions that
you need to run and manage jobs,

ask your administrator to grant you the
following IAM roles on your project:

- BigQuery Job User (`roles/bigquery.jobUser`) - to run or repeat a job, list your jobs, view details of your jobs, and cancel your jobs.
- BigQuery User (`roles/bigquery.user`) - to run or repeat a job, list your jobs, view details of your jobs, and cancel your jobs (this role is more permissive than BigQuery Job User).
- BigQuery Resource Admin (`roles/bigquery.resourceAdmin`) - to list all jobs and retrieve metadata on any job.
- BigQuery Admin (`roles/bigquery.admin`) - to list all jobs, retrieve metadata on any job, and cancel any job.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to run and manage jobs. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to run and manage jobs:

- `bigquery.jobs.create` on the project to run or repeat a job and list your jobs.
- `bigquery.jobs.get` on the project to view the metadata for any job.
- `bigquery.jobs.update` on the project to cancel any job.
- `bigquery.jobs.listAll` on the organization, folder, or project to list all jobs and retrieve metadata on any job submitted by any user. To see details for all jobs, the `bigquery.jobs.list` permission is also required.
- `bigquery.jobs.list` on the project to list all jobs and retrieve metadata on any job submitted by any user. For jobs submitted by other users, details and metadata are redacted.
- `bigquery.jobs.listExecutionMetadata` on the organization to list all job execution metadata (without sensitive information) for any job submitted by any user.
- `bigquery.jobs.update` on the project to cancel any job.
- `bigquery.jobs.delete` on the project to delete any job.


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information about IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## View job details

You can view job details by using the Google Cloud console, the
bq command-line tool, the API, or the client libraries. The details include data and
metadata, such as the job type, the job state, and the user who created the job.

To view job details, follow these steps:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Job history**.

4. Select the type of job history you want to view:

   - To display information of your recent jobs, click **Personal history**.
   - To display information of recent jobs in your project, click **Project history**.
5. To view job details, click a job.

   > [!NOTE]
   > **Note:** The duration of a job is calculated by subtracting start time (instead of creation time) from end time.

### bq

Issue the [`bq show`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show)
command with the `--job=true` flag and a job ID.

When you supply the job ID, you can use the fully qualified ID or the short
form. For example, job IDs listed in the Google Cloud console are fully
qualified, that is, they include the project and location:

`my-project-1234:US.bquijob_123x456_123y123z123c`

Job IDs in the command-line tool are listed using the short form.
Project ID and location are not included:

`bquijob_123x456_123y123z123c`

To specify the job location, supply the `--location` flag and set the value
to your [location](https://docs.cloud.google.com/bigquery/docs/locations). This flag is optional
if you use the fully qualified job ID. If you include the `--location`
flag and you're using the fully qualified job ID, the `--location` flag is
ignored.

The following command requests information about a job:

```bash
bq --location=LOCATION show --job=true JOB_ID
```

Replace the following:

- `LOCATION`: the name of the location where the job runs. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location using the [`.bigqueryrc` file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags). If the location isn't specified as part of the job ID or by using the `--location` flag, the default location is used.
- `JOB_ID`: the ID of the job

**Examples**

The following command gets summary information about job
`US.bquijob_123x456_123y123z123c` running in `myproject`:

    bq show --job=true myproject:US.bquijob_123x456_123y123z123c

The output is similar to the following:

```
 Job Type    State      Start Time      Duration      User Email       Bytes Processed   Bytes Billed   Billing Tier   Labels
 --- --- --- --- --- --- --- --- ---
 extract    SUCCESS   06 Jul 11:32:10   0:01:41    user@example.com
```

To see full job details, enter the following:

    bq show --format=prettyjson --job=true myproject:US.bquijob_123x456_789y123z456c

The output is similar to the following:

```
{
  "configuration": {
    "extract": {
      "compression": "NONE",
      "destinationUri": "[URI removed]",
      "destinationUris": [
        "[URI removed]"
      ],
      "sourceTable": {
        "datasetId": "github_repos",
        "projectId": "bigquery-public-data",
        "tableId": "commits"
      }
    }
  },
  "etag": "\"[etag removed]\"",
  "id": "myproject:bquijob_123x456_789y123z456c",
  "jobReference": {
    "jobId": "bquijob_123x456_789y123z456c",
    "projectId": "[Project ID removed]"
  },
  "kind": "bigquery#job",
  "selfLink": "https://bigquery.googleapis.com/bigquery/v2/projects/federated-testing/jobs/bquijob_123x456_789y123z456c",
  "statistics": {
    "creationTime": "1499365894527",
    "endTime": "1499365894702",
    "startTime": "1499365894702"
  },
  "status": {
    "errorResult": {
      "debugInfo": "[Information removed for readability]",
      "message": "Operation cannot be performed on a nested schema. Field: author",
      "reason": "invalid"
    },
    "errors": [
      {
        "message": "Operation cannot be performed on a nested schema. Field: author",
        "reason": "invalid"
      }
    ],
    "state": "DONE"
  },
  "user_email": "user@example.com"
}
```

### API

Call [jobs.get](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get) and provide
the `jobId` and `projectId` parameters. (Optional) Supply the `location`
parameter and set the value to the [location](https://docs.cloud.google.com/bigquery/docs/locations)
where the job runs. This parameter is optional if you use the
fully qualified job ID that includes the location, for example,
`my-project-1234:US.bquijob_123x456_123y123z123c`.

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
    )

    // getJobInfo demonstrates retrieval of a job, which can be used to monitor
    // completion or print metadata about the job.
    func getJobInfo(w io.Writer, projectID, jobID string) error {
    	// projectID := "my-project-id"
    	// jobID := "my-job-id"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	job, err := client.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Client_JobFromID(ctx, jobID)
    	if err != nil {
    		return err
    	}

    	status := job.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_LastStatus()
    	state := "Unknown"
    	switch status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_State {
    	case bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StateUnspecified_Pending_Running_Done:
    		state = "Pending"
    	case bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StateUnspecified_Pending_Running_Done:
    		state = "Running"
    	case bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StateUnspecified_Pending_Running_Done:
    		state = "Done"
    	}
    	fmt.Fprintf(w, "Job %s was created %v and is in state %s\n",
    		jobID, status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Statistics.CreationTime, state)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html;

    // Sample to get a job
    public class GetJob {

      public static void runGetJob() {
        // TODO(developer): Replace these variables before running the sample.
        String jobName = "MY_JOB_NAME";
        getJob(jobName);
      }

      public static void getJob(String jobName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html jobId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html.of(jobName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getJob_com_google_cloud_bigquery_JobId_com_google_cloud_bigquery_BigQuery_JobOption____(jobId);
          System.out.println("Job retrieved successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Job not retrieved. \n" + e.toString());
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

    async function getJob() {
      // Get job properties.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const jobId = "existing-job-id";

      // Create a job reference
      const job = bigquery.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html(jobId);

      // Retrieve job
      const [jobResult] = await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.get();

      console.log(jobResult.metadata.jobReference);
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


    def get_job(
        client: https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html,
        location: str = "us",
        job_id: str = "abcd-efgh-ijkl-mnop",
    ) -> None:
        job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_job(job_id, location=location)

        # All job classes have "location" and "job_id" string properties.
        # Use these properties for job operations such as "cancel_job" and
        # "delete_job".
        print(f"{https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.location}:{https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.job_id}")
        print(f"Type: {https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.job_type}")
        print(f"State: {https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.state}")
        print(f"Created: {https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.created.isoformat()}")

If you need more information to troubleshoot a job, see the [`INFORMATION_SCHEMA.JOBS*` views](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) and [Logs](https://docs.cloud.google.com/bigquery/docs/monitoring#logs).

## List jobs

BigQuery saves a six-month job history
for all the jobs of a project, for all locations. The job history includes jobs
that are in the `RUNNING` state and jobs that are
`DONE` (indicated by reporting the state as `SUCCESS` or `FAILURE`).

To list jobs in a project, follow these steps:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Job history**.

4. To list all jobs in a project, click **Project history**. If you aren't
   the project owner, you might not have permission to view all the jobs for
   a project. The most recent jobs are listed first.

5. To list your jobs, click **Personal history**.

### bq

Issue the [`bq ls`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_ls)
command with one of the following flags:

- `--jobs=true` or `-j`: identifies jobs as the type of resource to list.
- `--all=true` or `-a`: lists jobs from all users. To see full (unredacted) details for all jobs, you must have `bigquery.jobs.listAll` permissions.
- `--min_creation_time`: lists jobs after a supplied timestamp value. This value is represented as a [Unix epoch](https://wikipedia.org/wiki/Unix_time) timestamp in milliseconds.
- `--max_creation_time`: lists jobs before a supplied timestamp value. This value is represented as a [Unix epoch](https://wikipedia.org/wiki/Unix_time) timestamp in milliseconds.
- `--max_results` or `-n` limits the results. The default is 50 results.

```bash
bq ls --jobs=true --all=true \
    --min_creation_time=MIN_TIME \
    --max_creation_time=MAX_TIME \
    --max_results=MAX_RESULTS \
    PROJECT_ID
```

Replace the following:

- `MIN_TIME`: an integer that represents a [Unix epoch](https://wikipedia.org/wiki/Unix_time) timestamp in milliseconds.
- `MAX_TIME`: an integer that represents a [Unix epoch](https://wikipedia.org/wiki/Unix_time) timestamp in milliseconds.
- `MAX_RESULTS`: an integer that indicates the number of jobs returned.
- `PROJECT_ID`: the ID of the project that contains the jobs that you're listing. If you [set a default project](https://docs.cloud.google.com/sdk/gcloud/reference/config/set), you don't need to provide the `PROJECT_ID` parameter.

**Examples**

The following command lists all jobs for the current user. Running this
command requires `bigquery.jobs.list` permissions.

    bq ls --jobs=true myproject

The following command lists all jobs for all users. Running this command
requires `bigquery.jobs.listAll` permissions.

    bq ls --jobs=true --all=true myproject

The following command lists the 10 most recent jobs in `myproject`:

    bq ls --jobs=true --all=true --max_results=10 myproject

The following command lists all jobs submitted before March 3, 2032, at
4:04:00 AM. This timestamp (in milliseconds) is equivalent to the following
integer value: `1961899440000`.

    bq ls --jobs=true --max_creation_time=1961899440000

### API

Call the
[`jobs.list` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/list) and provide
the `projectId` parameter. To list jobs for all users, set the `allUsers`
parameter to `true`. Setting `allUsers` to `true` requires
`bigquery.jobs.listAll` permissions. The `jobs.list` method doesn't return
child jobs. To list child jobs, use the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#child_jobs).

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

    // listJobs demonstrates iterating through the BigQuery jobs collection.
    func listJobs(w io.Writer, projectID string) error {
    	// projectID := "my-project-id"
    	// jobID := "my-job-id"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	it := client.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Client_Jobs(ctx)
    	// List up to 10 jobs to demonstrate iteration.
    	for i := 0; i < 10; i++ {
    		j, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		state := "Unknown"
    		switch j.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_LastStatus().https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_State {
    		case bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StateUnspecified_Pending_Running_Done:
    			state = "Pending"
    		case bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StateUnspecified_Pending_Running_Done:
    			state = "Running"
    		case bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StateUnspecified_Pending_Running_Done:
    			state = "Done"
    		}
    		fmt.Fprintf(w, "Job %s in state %s\n", j.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_ID(), state)
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;

    // Sample to get list of jobs
    public class ListJobs {

      public static void runListJobs() {
        listJobs();
      }

      public static void listJobs() {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          Page<Job> jobs = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_listJobs_com_google_cloud_bigquery_BigQuery_JobListOption____(BigQuery.JobListOption.pageSize(10));
          if (jobs == null) {
            System.out.println("Dataset does not contain any jobs.");
            return;
          }
          jobs.getValues().forEach(job -> System.out.printf("Success! Job ID: %s", job.getJobId()));
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Jobs not listed in dataset due to error: \n" + e.toString());
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

    async function listJobs() {
      // Lists all jobs in current GCP project.

      // List the 10 most recent jobs in reverse chronological order.
      //  Omit the max_results parameter to list jobs from the past 6 months.
      const options = {maxResults: 10};
      const [jobs] = await bigquery.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html(options);

      console.log('Jobs:');
      jobs.forEach(job => console.log(https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id));
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

    import datetime

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # List the 10 most recent jobs in reverse chronological order.
    # Omit the max_results parameter to list jobs from the past 6 months.
    print("Last 10 jobs:")
    for job in client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_jobs(max_results=10):  # API request(s)
        print("{}".format(https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.job_id))

    # The following are examples of additional optional parameters:

    # Use min_creation_time and/or max_creation_time to specify a time window.
    print("Jobs from the last ten minutes:")
    ten_mins_ago = datetime.datetime.utcnow() - datetime.timedelta(minutes=10)
    for job in client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_jobs(min_creation_time=ten_mins_ago):
        print("{}".format(https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.job_id))

    # Use all_users to include jobs run by all users in the project.
    print("Last 10 jobs run by all users:")
    for job in client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_jobs(max_results=10, all_users=True):
        print("{} run by user: {}".format(https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.job_id, https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.user_email))

    # Use state_filter to filter by job state.
    print("Last 10 jobs done:")
    for job in client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_list_jobs(max_results=10, state_filter="DONE"):
        print("{}".format(https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.job_id))

## Cancel a job

You can cancel a `RUNNING` or `PENDING` job.
It usually takes less than a minute to complete a job cancellation.

Even if the job can be canceled, success is not guaranteed. The job might have
completed by the time the cancel request is submitted, or the job might be in a
stage where it cannot be canceled.

> [!NOTE]
> **Note:** You can still incur costs after canceling a job depending on the stage at which it was canceled. For more information, see the [Pricing](https://cloud.google.com/bigquery/pricing) page.

To cancel a job, follow these steps:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **Compose new query** and enter a query.

3. To run the query, click **Run**.

4. To cancel a job, click **Cancel**.

### SQL

Use the `BQ.JOBS.CANCEL` system procedure:


      CALL BQ.JOBS.CANCEL('JOB_ID');

<br />

Replace <var translate="no">JOB_ID</var> with the ID of the job you're canceling.

If you are in a different project but in the same region as the job you
want to cancel, you must also include the project ID:


      CALL BQ.JOBS.CANCEL('PROJECT_ID.JOB_ID');

<br />

Replace the following:

- `PROJECT_ID`: the ID of the project that contains the job that you're canceling
- `JOB_ID`: the ID of the job that you're canceling

The procedure returns immediately, and BigQuery cancels the
job shortly afterward. If the job has already succeeded or failed, the
procedure has no effect.

### bq

Issue the [`bq cancel`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cancel)
command with the `JOB_ID` argument. You can request
cancellation and return immediately by using the `--nosync=true` flag. By
default, cancellation requests wait for completion.

When you supply the `JOB_ID` argument, you can use the
fully qualified ID or the short
form. For example, job IDs listed in the Google Cloud console are fully
qualified; that is, they include the project and location:

`my-project-1234:US.bquijob_123x456_123y123z123c`

Job IDs in the bq command-line tool are listed using the short form. Project ID and
location are not included:

`bquijob_123x456_123y123z123c`

To specify the job location, supply the `--location` flag and set the value
to your [location](https://docs.cloud.google.com/bigquery/docs/locations). This flag is optional
if you use the fully qualified job ID. If you include the `--location`
flag and you're using the fully qualified job ID, the `--location` flag is
ignored.

The following command requests job cancellation and waits for completion. If
the fully qualified job ID is supplied, the `--location` flag is ignored:

```bash
bq --location=LOCATION cancel JOB_ID
```

The following command requests job cancellation and returns immediately. If
the fully qualified job ID is supplied, the `--location` flag is ignored:

```bash
bq --location=LOCATION --nosync cancel JOB_ID
```

Replace the following:

- `LOCATION` (optional): the name of the location where the job runs. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location using the [`.bigqueryrc` file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `JOB_ID`: the ID of the job that you're canceling. If you copy the job ID from the Google Cloud console, the project ID and location are included in the job ID. For example, `my-project-1234:US.bquijob_123x456_123y123z123c`.

**Examples**

The following command cancels the job
`my-project-1234:US.bquijob_123x456_123y123z123c` running in
the `US` multi-region location in the `my-project-1234` project, and waits
for
completion. Because the fully qualified job ID is used, the location flag is
not supplied.

    bq cancel my-project-1234:US.bquijob_123x456_123y123z123c

The following command cancels the job `bquijob_123x456_123y123z123c` running in
the `US` multi-region location in the `my-project-1234` project and waits
for
completion. Because the short form of the job ID is used, the `--location`
flag is supplied.

    bq --location=US cancel bquijob_123x456_123y123z123c

The following command cancels the job `bquijob_123x456_123y123z123c` running
in the `US` multi-region location in the `my-project-1234` project,
and returns immediately.
Because the fully qualified job ID is used, the `--location` flag is
not supplied.

    bq --nosync cancel my-project-1234:US.bquijob_123x456_123y123z123c

### API

Call [jobs.cancel](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel) and provide
the `jobId` and `projectId` parameters. Supply the `location`
parameter and set the value to the [location](https://docs.cloud.google.com/bigquery/docs/locations)
where the job runs.

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

    	"cloud.google.com/go/bigquery"
    )

    // cancelJob demonstrates how a job cancellation request can be issued for a specific
    // BigQuery job.
    func cancelJob(projectID, jobID string) error {
    	// projectID := "my-project-id"
    	// jobID := "my-job-id"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	job, err := client.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Client_JobFromID(ctx, jobID)
    	if err != nil {
    		return nil
    	}
    	return job.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_Cancel(ctx)
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
    import java.util.UUID;

    // Sample to cancel a job
    public class CancelJob {

      public static void runCancelJob() {
        // TODO(developer): Replace these variables before running the sample.
        String query = "SELECT country_name from `bigquery-public-data.utility_us.country_code_iso`";
        cancelJob(query);
      }

      public static void cancelJob(String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Specify a job configuration to set optional job resource properties.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query).build();

          // The location and job name are optional,
          // if both are not specified then client will auto-create.
          String jobName = "jobId_" + UUID.randomUUID().toString();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html jobId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html.newBuilder().setLocation("us").https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.Builder.html#com_google_cloud_bigquery_JobId_Builder_setJob_java_lang_String_(jobName).build();

          // Create a job with job ID
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(jobId, queryConfig));

          // Get a job that was just created
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getJob_com_google_cloud_bigquery_JobId_com_google_cloud_bigquery_BigQuery_JobOption____(jobId);
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_cancel__()) {
            System.out.println("Job canceled successfully");
          } else {
            System.out.println("Job was not canceled");
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Job was not canceled.\n" + e.toString());
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

    async function cancelJob() {
      // Attempts to cancel a job.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const jobId = "existing-job-id";

      // Create a job reference
      const job = bigquery.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html(jobId);

      // Attempt to cancel job
      const [apiResult] = await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/job.html();

      console.log(apiResult.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status);
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


    def cancel_job(
        client: https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html,
        location: str = "us",
        job_id: str = "abcd-efgh-ijkl-mnop",
    ) -> None:
        job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_cancel_job(job_id, location=location)
        print(f"{https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.location}:{https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.job_id} cancelled")

## Delete job metadata

You can delete the metadata for a specific job using the bq command-line tool and the Python client library.
BigQuery preserves a history of jobs executed
in the past 6 months. You can use this method to remove sensitive information
that might be present in query statements. Job metadata can only be deleted
after the job is complete. If a job has created child jobs, the child jobs are
also deleted. Deletion of child jobs is not
allowed. Only parent or top-level jobs can be deleted.

To delete job metadata, follow these steps:

### bq

Issue the `bq rm` command with the `-j` flag and a job ID.

When you supply the job ID, you can use the fully qualified ID or the short
form. For example, job IDs listed in the Google Cloud console are fully
qualified, that is, they include the project and location:

`my-project-1234:US.bquijob_123x456_123y123z123c`

Job IDs in the bq command-line tool are listed using the short form.
Project ID and location are not included:

`bquijob_123x456_123y123z123c`

To specify the job location, supply the `--location` flag and set the value
to your [location](https://docs.cloud.google.com/bigquery/docs/locations). This flag is optional
if you use the fully qualified job ID. If you include the `--location`
flag and you're using the fully qualified job ID, the `--location` flag is
ignored.

The following command deletes a job:

```bash
bq --location=location \
    --project_id=project_id \
    rm -j job_id
```

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

    from google.api_core import exceptions
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # TODO(developer): Set the job ID to the ID of the job whose metadata you
    #                  wish to delete.
    job_id = "abcd-efgh-ijkl-mnop"

    # TODO(developer): Set the location to the region or multi-region
    #                  containing the job.
    location = "us-east1"

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_delete_job_metadata(job_id, location=location)

    try:
        client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_job(job_id, location=location)
    except exceptions.NotFound:
        print(f"Job metadata for job {location}:{job_id} was deleted.")

## Repeat jobs

It is not possible to repeat a job by using the same job ID. Instead, you create a
new job with the same configuration. When you submit the new job in the
Google Cloud console or the bq command-line tool, a new job ID is assigned. When you
submit the job using the API or client libraries, you must generate a new job
ID.

To repeat a job, follow these steps:

### Console

To repeat a query job, follow these steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Job history**.

4. To list all your jobs, click **Personal history** . To list all
   jobs in a project, click **Project history**.

5. Click a query job to open the job details.

6. To repeat a query, click **Open as new query**.

7. Click **Run**.

To repeat a load job, do the following:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Job history**.

4. To list all your jobs, click **Personal history** . To list all
   jobs in a project, click **Project history**.

5. Click a load job to open the job details.

6. To repeat a job, click **Repeat load job**.

> [!NOTE]
> **Note:** You cannot repeat an extract job or a copy job using the Google Cloud console.

### bq

Issue your command again and BigQuery automatically
generates a job with a new job ID.

### API

There is no single-call method to repeat a job; if you want to repeat a
specific job:

1. Call [`jobs.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get) to
   retrieve the resource for the job to repeat.

2. Remove the *id* , *status* , and *statistics* field.
   Change the *jobId* field to a new value generated by your client
   code. Change any other fields as necessary.

3. Call [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) with
   the modified resource and the new job ID to start the new job.

## What's next

- Learn how to [run jobs programmatically](https://docs.cloud.google.com/bigquery/docs/running-jobs).