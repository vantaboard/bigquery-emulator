# Error messages

This document describes error messages that you might encounter when working with
BigQuery, including HTTP error codes and suggested troubleshooting
steps.

For more information about query errors, see
[Troubleshoot query errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-queries).

For more information about streaming insert errors, see
[Troubleshoot streaming inserts](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#troubleshooting).

## Error table

Responses from the BigQuery API include an HTTP error code and an error object
in the response body. An error object is typically one of the following:

- An [`errors` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobstatus), which contains an array of [`ErrorProto`
  objects](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#errorproto).
- An [`errorResults` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobstatus), which contains a single [`ErrorProto`
  object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#errorproto).

The **Error message** column in the following table maps to the `reason` property
in an `ErrorProto` object.

The table does not include all possible HTTP errors or other networking errors.
Therefore, don't assume that an error object is present in every error response
from BigQuery. In addition, you might receive different errors or
error objects if you use the Cloud Client Libraries for the BigQuery API. For
more information, see [BigQuery API Client
Libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries).

If you receive an HTTP response code that doesn't appear in the following table,
the response code indicates an issue or an expected result with the HTTP
request. Response codes in the `5xx` range indicate a server-side error. If you
receive a `5xx` response code, then retry the request later. In some cases, a
`5xx` response code might be returned by an intermediate server such as a
proxy. Examine the response body and response headers for details about the
error. For a full list of HTTP response codes, see [HTTP response
codes](https://en.wikipedia.org/wiki/HTTP_response_codes).

If you use the [bq command-line tool](https://docs.cloud.google.com/bigquery/bq-command-line-tool) to check job status,
the error object is not returned by default. To view the error object and the
corresponding `reason` property that maps to the following table, use the
`--format=prettyjson` flag. For example, `bq --format=prettyjson show -j
*<job id>*`. To view verbose logging for the bq tool, use
`--apilog=stdout`. To learn more about troubleshooting the bq tool,
see [Debugging](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#debugging).

| Error message | HTTP code | Description | Troubleshooting |
|---|---|---|---|
| accessDenied | 403 | This error returns when you try to access a resource such as a [dataset](https://docs.cloud.google.com/bigquery/docs/datasets), [table](https://docs.cloud.google.com/bigquery/docs/tables), [view](https://docs.cloud.google.com/bigquery/docs/views-intro), or [job](https://docs.cloud.google.com/bigquery/docs/managing-jobs) that you don't have access to. This error also returns when you try to modify a read-only object. | Contact the resource owner and [request access to the resource](https://docs.cloud.google.com/bigquery/access-control) for the user identified by the `principalEmail` value in the error's audit log. |
| attributeError | 400 | This error returns when there is an issue with the user code where a certain object attribute is called but does not exist. | Ensure that the object you are working with has the attribute you are trying to access. For more information on this error, see [AttributeError](https://docs.python.org/3/library/exceptions.html#AttributeError). |
| backendError | 500, 502, 503 or 504 | This error indicates that the service is currently unavailable. This can happen due to a number of transient issues, including: - **Service demand surge**: Sudden spikes in demand, such as peak usage times, can result in load shedding to protect the quality of service for all BigQuery users. To prevent the system from getting overwhelmed BigQuery can return 500 or 503 errors for a small portion of requests. - **Network issues**: The distributed nature of BigQuery means data is often transferred between different components or machines in the system. Various intermittent network connectivity issues can cause BigQuery to return a 5xx error, including SSL handshake failures, or other network infrastructure issues between the user and Google Cloud. - **Resource exhaustion**: BigQuery has various internal resource limits in place to protect the overall service performance from a single user or a single job from consuming too many resources. BigQuery implements load shedding to tackle resource exhaustion. - **Backend errors**: In rare cases, an internal problem within one of the BigQuery components can result in a 500 or 503 error returned to the client. | 5xx errors are service-side issues and the client has no way to fix or control them. From the client side, in order to mitigate the impact of 5xx errors, you need to retry your requests using [truncated exponential backoffs](https://cloud.google.com/monitoring/api/troubleshooting#exponential-retry). For more information about exponential backoffs, see [Exponential backoff](https://en.wikipedia.org/wiki/Exponential_backoff). However, there are two special cases for troubleshooting this error: `jobs.get` calls and `jobs.insert` calls. **`jobs.get` calls** - If you received a 503 error when polling `jobs.get`, wait a few seconds and poll again. - If the job completes but includes an error object that contains `backendError`, the job failed. You can safely retry the job without concerns about data consistency. **`jobs.insert` calls** If you receive this error when making a `jobs.insert` call, it's unclear if the job succeeded. In this situation, you'll need to retry the job. If the retries are not effective and the issues persist, you can [calculate the rate of failing requests](https://docs.cloud.google.com/bigquery/docs/error-messages#calculate-rate-of-failing-requests) and [contact support](https://cloud.google.com/support). Also, if you observe a specific request to BigQuery persistently fail with a 5xx error, even when retried using exponential backoff on multiple workflow restart attempts, you should escalate this to [support](https://cloud.google.com/support) to troubleshoot the issue from the BigQuery side, regardless of the overall calculated error rate. Make sure to clearly communicate the [business impact](https://cloud.google.com/support/docs/customer-care-procedures#support_case_priority) so that the issue can be triaged correctly. |
| badRequest | 400 | The error `'UPDATE or DELETE statement over table project.dataset.table would affect rows in the streaming buffer, which is not supported'` can occur when some recently streamed rows in a table might not be available for DML operations (`DELETE`, `UPDATE`,`MERGE`), typically for a few minutes, but in rare cases, up to 90 minutes. For more information, see [Streaming data availability](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#dataavailability) and [DML Limitations.](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations) | Wait a few minutes and try again, or filter your statement to only operate on older data that is outside of the streaming buffer. To see if data is available for table DML operations, check the [`tables.get` response](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get) for the [streamingBuffer section](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#streamingbuffer). If the streamingBuffer section is absent, then table data is available for DML operations. You can also use the `streamingBuffer.oldestEntryTime` field to identify the age of records in the streaming buffer. Alternatively, consider streaming data with the [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api#use_data_manipulation_language_dml_with_recently_streamed_data), which doesn't have this limitation. |
| billingNotEnabled | 403 | This error returns when billing isn't enabled for the project. | Enable billing for the project in the [Google Cloud console](https://console.cloud.google.com/). |
| billingTierLimitExceeded | 400 | This error returns when the value of `statistics.query.billingTier` for an on-demand Job exceeds 100. This occurs when on-demand queries use too much CPU relative to the amount of data scanned. For instructions on how to inspect job details, see [Managing jobs](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job). | This error most often results from executing inefficient cross-joins, either explicitly or implicitly, for example due to an inexact join condition. These types of queries are not suitable for on-demand pricing due to high resource consumption, and in general they may not scale well. You can either optimize the query or switch to use the [capacity-based (slots)](https://docs.cloud.google.com/bigquery/docs/reservations-intro) pricing model to resolve this error. For information about optimizing queries, see [Avoiding SQL anti-patterns](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-patterns). |
| blocked | 403 | This error returns when BigQuery has temporarily denylisted the operation you attempted to perform, usually to prevent a service outage. | [Contact support](https://cloud.google.com/support) for more information. |
| duplicate | 409 | This error returns when trying to create a job, dataset, or table that already exists. The error also returns when a job's `writeDisposition` property is set to `WRITE_EMPTY` and the destination table accessed by the job already exists. | Rename the resource you're trying to create, or change the `writeDisposition` value in the job. For more information, see how to troubleshoot the [Job already exists](https://docs.cloud.google.com/bigquery/docs/troubleshoot-queries#job_already_exists) error. |
| internalError | 500 | This error returns when an internal error occurs within BigQuery. | Wait according to the back-off requirements described in the [BigQuery Service Level Agreement](https://cloud.google.com/bigquery/sla), then try the operation again. If the error continues to occur, [contact support](https://cloud.google.com/support) or [file a bug](https://issuetracker.google.com/issues/new?component=187149&template=0) using the BigQuery issue tracker. You can also reduce the frequency of this error by using [Reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro). |
| invalid | 400 | This error returns when there is any type of invalid input other than an invalid query, such as missing required fields or an invalid table schema. Invalid queries return an `invalidQuery` error. |   |
| invalidQuery | 400 | This error returns when you attempt to run an invalid query. | Check your query for syntax errors. The [query reference](https://docs.cloud.google.com/bigquery/query-reference) contains descriptions and examples of how to construct valid queries. |
| invalidUser | 400 | This error returns when you attempt to schedule a query with invalid user credentials. | Refresh the user credentials, as explained in [Scheduling queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries#update_scheduled_query_credentials). |
| jobBackendError | 400 | This error returns when the job was created successfully, but failed with an internal error. You might see this error in `jobs.query` or `jobs.getQueryResults`. | Retry the job with a new `jobId`. If the error continues to occur, contact support. |
| jobInternalError | 400 | This error returns when the job was created successfully, but failed with an internal error. You might see this error in `jobs.query` or `jobs.getQueryResults`. | Retry the job with a new `jobId`. If the error continues to occur, contact support. |
| jobRateLimitExceeded | 400 | This error returns when the job was created successfully, but failed with a [rateLimitExceeded](https://docs.cloud.google.com/bigquery/docs/error-messages#rateLimitExceeded) error. You might see this error in `jobs.query` or `jobs.getQueryResults`. | Use [exponential backoff](https://cloud.google.com/monitoring/api/troubleshooting#exponential-retry) to reduce the request rate, and then retry the job with a new `jobId`. |
| notFound | 404 | This error returns when you refer to a resource (a dataset, a table, or a job) that doesn't exist, or when the location in the request does not match the location of the resource (for example, the location in which a job is running). This can also occur when using [table decorators](https://docs.cloud.google.com/bigquery/docs/table-decorators) to refer to deleted tables that have recently been [streamed to](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery). | Fix the resource names, correctly specify the location, or wait at least 6 hours after streaming before querying a deleted table. |
| notImplemented | 501 | This job error returns when you try to access a feature that isn't implemented. | [Contact support](https://cloud.google.com/support) for more information. |
| proxyAuthenticationRequired | 407 | This error returns between the client environment and the proxy server when the request lacks valid authentication credentials for the proxy server. For more information, see [407 Proxy Authentication Required](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/407). | Troubleshooting is specific to your environment. If you receive this error while working in Java, ensure you have set both the `jdk.http.auth.tunneling.disabledSchemes=` and `jdk.http.auth.proxying.disabledSchemes=` properties with no value following the equal sign. |
| quotaExceeded | 403 | This error returns when your project exceeds a [BigQuery quota](https://docs.cloud.google.com/bigquery/quota-policy), a [custom quota](https://docs.cloud.google.com/bigquery/docs/custom-quotas), or when you haven't set up billing and you have exceeded the [free tier for queries](https://cloud.google.com/bigquery/pricing#free-tier). | View the `message` property of the error object for more information about which quota was exceeded. To reset or raise a BigQuery quota, [contact support](https://cloud.google.com/support). To modify a custom quota, submit a request from the [Quotas](https://console.cloud.google.com/iam-admin/quotas) page. If you receive this error using the BigQuery sandbox, you can [upgrade from the sandbox](https://docs.cloud.google.com/bigquery/docs/sandbox#upgrade). For more information, see [Troubleshooting BigQuery quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas). |
| rateLimitExceeded | 403 | This error returns if your project exceeds a short-term rate limit by sending too many requests too quickly. For example, see the [rate limits for query jobs](https://docs.cloud.google.com/bigquery/quota-policy#query_jobs) and [rate limits for API requests](https://docs.cloud.google.com/bigquery/quota-policy#api_requests). | Slow down the request rate. If you believe that your project did not exceed one of these limits, [contact support](https://cloud.google.com/support). For more information, see [Troubleshooting BigQuery quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas). |
| resourceInUse | 400 | This error returns when you try to delete a dataset that contains tables or when you try to delete a job that is currently running. | Empty the dataset before attempting to delete it, or wait for a job to complete before deleting it. |
| resourcesExceeded | 400 | This error returns when your job uses too many resources. | This error returns when your job uses too many resources. For troubleshooting information, see [Troubleshoot resources exceeded errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-queries#ts-resources-exceeded). |
| responseTooLarge | 403 | This error returns when your query's results are larger than the [maximum response size](https://docs.cloud.google.com/bigquery/quota-policy#query_jobs). Some queries execute in multiple stages, and this error returns when any stage returns a response size that is too large, even if the final result is smaller than the maximum. This error commonly returns when queries use an `ORDER BY` clause. | Adding a `LIMIT` clause can sometimes help, or removing the `ORDER BY` clause. If you want to ensure that large results can return, you can set the `allowLargeResults` property to `true` and specify a destination table. For more information, see [Writing large query results](https://docs.cloud.google.com/bigquery/docs/writing-results#large-results). |
| stopped | 200 | This status code returns when a job is canceled. |   |
| tableUnavailable | 400 | Certain BigQuery tables are backed by data managed by other Google product teams. This error indicates that one of these tables is unavailable. | When you encounter this error message, you can retry your request (see [internalError](https://docs.cloud.google.com/bigquery/docs/error-messages#internalError) troubleshooting suggestions) or contact the Google product team that granted you access to their data. |
| timeout | 400 | The job timed out. | Consider reducing the amount of work performed by your operation so that it can complete within the set limit. For more information, see [Troubleshoot quota and limit errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas). |

### Sample error response

    GET https://bigquery.googleapis.com/bigquery/v2/projects/12345/datasets/foo
    Response:
    [404]
    {
      "error": {
      "errors": [
      {
        "domain": "global",
        "reason": "notFound",
        "message": "Not Found: Dataset myproject:foo"
      }],
      "code": 404,
      "message": "Not Found: Dataset myproject:foo"
      }
    }

### Calculate rate of failing requests and uptime

The majority of 500 and 503 errors can be resolved by performing a retry with
exponential backoff. In the case where 500 and 503 errors still persist, you
can calculate the overall rate of failing requests and corresponding uptime to
compare it to the [BigQuery Service Level Agreement
(SLA)](https://cloud.google.com/bigquery/sla) to determine if the service is working as expected.

To calculate the overall rate of failing requests during the last 30 days, take
the number of failed requests for a specific API call or method from the last
30 days and divide by the total number of requests for that API call or method
from the last 30 days. Multiply this value by 100 to get the average percentage
of failed requests over 30 days.

For example, you can query [Cloud Logging
data](https://docs.cloud.google.com/logging/docs/view/logging-query-language) to get the number of total
`jobs.insert` requests and the number of failed `jobs.insert` requests and
perform the calculation. You can also obtain the error rate values from the
[API dashboard](https://docs.cloud.google.com/apis/docs/monitoring#using_the_api_dashboard) or using the
Metrics Explorer in [Cloud Monitoring](https://docs.cloud.google.com/apis/docs/monitoring#using). These
options won't include data about networking or routing problems encountered
between the client and BigQuery, so we also recommend using a
client-side logging and reporting system for more precise failure rate
calculations.

First, take 100% minus the overall rate of failing requests. If this value is
more than or equal to the value described in the BigQuery SLA,
then the uptime also meets the BigQuery SLA. However, if this
value is less than the value described in the SLA, calculate the uptime
manually.

In order to calculate the uptime, you need to know the number of minutes that
are considered service downtime. Service downtime means a period of one minute
with more than 10% error rate calculated according to SLA definitions. To
calculate the uptime, take the total minutes from the last 30 days and subtract
the total minutes that the service was down. Divide the remaining time by the
total minutes from the last 30 days and multiply this value by 100 to get the
percentage of uptime over 30 days. For more information about the definitions
and calculations related to SLA , see [BigQuery Service Level
Agreement (SLA)](https://cloud.google.com/bigquery/sla)

If your monthly uptime percentage is greater than or equal to the value
described in the BigQuery SLA, then the error was most likely
caused by a transient issue, so you can continue to retry using [exponential
backoff](https://docs.cloud.google.com/monitoring/api/troubleshooting#exponential-retry).

If the uptime is below the value presented in the SLA, [contact
support](https://docs.cloud.google.com/support) for help and share the observed overall error rate and
uptime calculations.

## Authentication errors

Errors thrown by the OAuth token generation system return the following JSON
object, as defined by the [OAuth2
specification](https://tools.ietf.org/html/rfc6749#section-5.2).

`{"error" : "_description_string_"}`

The error is accompanied by either an HTTP `400` Bad Request error or an HTTP
`401` Unauthorized error. `_description_string_` is one of the error codes
defined by the OAuth2 specification. For example:

`{"error":"invalid_client"}`

### Review errors

You can use the [logs explorer](https://docs.cloud.google.com/logging/docs/view/logs-explorer-interface) to
view authentication errors for specific jobs, users, or other scopes. The
following are examples of logs explorer filters that you can use to review
authentication errors:

- Search for failed jobs with permission issues in the Policy Denied audit
  logs:

      resource.type="bigquery_resource"
      protoPayload.status.message=~"Access Denied"
      logName="projects/PROJECT_ID/logs/cloudaudit.googleapis.com%2Fdata_access"

  Replace `PROJECT_ID` with the ID of the project
  containing the resource.
- Search for a specific user or service account used for authentication:

      resource.type="bigquery_resource"
      protoPayload.authenticationInfo.principalEmail="EMAIL"

  Replace `EMAIL` with the email address of the user or
  service account.
- Search for Identity and Access Management policy changes in the Admin Activity audit
  logs:

      protoPayload.methodName=~"SetIamPolicy"
      logName="projects/PROJECT_ID/logs/cloudaudit.googleapis.com%2Factivity"

- Search for changes to a specific BigQuery dataset in the
  Data Access audit logs:

      resource.type="bigquery_resource"
      protoPayload.resourceName="projects/PROJECT_ID/datasets/DATASET_ID"
      logName=projects/PROJECT_ID/logs/cloudaudit.googleapis.com%2Fdata_access

  Replace `DATASET_ID` with the ID of the dataset containing the resource.

## Connectivity error messages

The following table lists error messages that you might see because of
intermittent connectivity issues when using the client libraries or calling the
BigQuery API from your code:

| Error message | Client library or API | Troubleshooting |
|---|---|---|
| com.google.cloud.bigquery.BigQueryException: Read timed out | Java | Set a larger timeout value. |
| Connection has been shutdown: javax.net.ssl.SSLException: java.net.SocketException: Connection reset at com.google.cloud.bigquery.spi.v2.HttpBigQueryRpc.translate(HttpBigQueryRpc.java:115) | Java | Implement a retry mechanism and set a larger timeout value. |
| javax.net.ssl.SSLHandshakeException: Remote host terminated the handshake | Java | Implement a retry mechanism and set a larger timeout value. |
| BrokenPipeError: \[Errno 32\] Broken pipe | Python | Implement a retry mechanism. For more information on this error, see [BrokenPipeError](https://docs.python.org/3/library/exceptions.html#BrokenPipeError). |
| Connection aborted. RemoteDisconnected('Remote end closed connection without response' | Python | Set a larger timeout value. |
| SSLEOFError (EOF occurred in violation of protocol) | Python | This error returns instead of a 413 (`ENTITY_TOO_LARGE`) HTTP error. Reduce the size of the request. |
| TaskCanceledException: A task was canceled | .NET library | Increase the timeout value on the client side. |
| google.api_core.exceptions.PreconditionFailed: 412 PATCH | Python | This error returns while trying to update a table resource using a [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/update). Make sure the ETag in the HTTP header isn't outdated. For table or dataset level operations, make sure that the resource hasn't changed since the last time it was instantiated and recreate the object if necessary. |
| Failed to establish a new connection: \[Errno 110\] Connection timed out | Client Libraries | This error returns when this request has reached the end-of-file (EOF) when streaming or reading data from BigQuery. Implement a [retry mechanism](https://cloud.google.com/monitoring/api/troubleshooting#exponential-retry) and set a larger timeout value. |
| socks.ProxyConnectionError: Error connecting to HTTP proxy :8080: \[Errno 110\] Connection timed out | Client Libraries | Troubleshoot proxy status and settings. Implement a [retry mechanism](https://cloud.google.com/monitoring/api/troubleshooting#exponential-retry) and set a larger timeout value. |
| Received an unexpected EOF or 0 bytes from the transport stream | Client Libraries | Implement a [retry mechanism](https://cloud.google.com/monitoring/api/troubleshooting#exponential-retry) and set a larger timeout value. |

## Google Cloud console error messages

The following table lists error messages that you might see while you work in the
Google Cloud console.

| Error message | Description | Troubleshooting |
|---|---|---|
| Unknown error response from the server. | This error displays when the Google Cloud console receives an unknown error from the server; for example, when you click a dataset or other type of link, and the page cannot be displayed. | Switch to your browser's incognito, or private, mode and repeating the action that resulted in the error. If no error results in incognito mode, then the error might be due to a browser extension, such as an ad blocker. Disable your browser extensions while not in incognito mode, and see if that resolves the issue. |