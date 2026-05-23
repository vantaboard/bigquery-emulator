# Migrate to BigQuery DataFrames version 2.0

Version 2.0 of BigQuery DataFrames makes security and performance improvements
to the BigQuery DataFrames API, adds new features, and introduces
breaking changes. This document describes the changes and provides migration
guidance. You can apply these recommendations before installing the 2.0 version
by using the latest version 1.x of BigQuery DataFrames.

BigQuery DataFrames version 2.0 has the following benefits:

- Faster queries and fewer tables are created when you run queries that return results to the client, because `allow_large_results` defaults to `False`. This design can reduce storage costs, especially if you use physical bytes billing.
- Improved security by default in the remote functions deployed by BigQuery DataFrames.

## Install BigQuery DataFrames version 2.0

To avoid breaking changes, pin to a specific version of
BigQuery DataFrames in your `requirements.txt` file (for example,
`bigframes==1.42.0`) or your `pyproject.toml` file (for example,
`dependencies = ["bigframes = 1.42.0"]`). When you're ready to try the latest
version, you can run `pip install --upgrade bigframes` to install the latest
version of BigQuery DataFrames.

## Use the `allow_large_results` option

BigQuery has a
[maximum response size limit](https://docs.cloud.google.com/bigquery/quotas#query_jobs) for query jobs.
Starting in BigQuery DataFrames version 2.0, BigQuery DataFrames
enforces this limit by default in methods that return results to the client,
such as `peek()`, `to_pandas()`, and `to_pandas_batches()`. If your job returns
large results, you can set `allow_large_results` to `True` in your
`BigQueryOptions` object to avoid breaking changes. This option is set to
`False` by default in BigQuery DataFrames version 2.0.

```python
import bigframes.pandas as bpd

bpd.options.bigquery.allow_large_results = True
```

You can override the `allow_large_results` option by using the
`allow_large_results` parameter in `to_pandas()` and other methods. For example:

```python
bf_df = bpd.read_gbq(query)
# ... other operations on bf_df ...
pandas_df = bf_df.to_pandas(allow_large_results=True)
```

## Use the `@remote_function` decorator

BigQuery DataFrames version 2.0 makes some changes to the default
behavior of the `@remote_function` decorator.

### Keyword arguments are enforced for ambiguous parameters

To prevent passing values to an unintended parameter,
BigQuery DataFrames version 2.0 and beyond enforces the use of keyword
arguments for the following parameters:

- `bigquery_connection`
- `reuse`
- `name`
- `packages`
- `cloud_function_service_account`
- `cloud_function_kms_key_name`
- `cloud_function_docker_repository`
- `max_batching_rows`
- `cloud_function_timeout`
- `cloud_function_max_instances`
- `cloud_function_vpc_connector`
- `cloud_function_memory_mib`
- `cloud_function_ingress_settings`

When using these parameters, supply the parameter name. For example:

```python
@remote_function(
  name="my_remote_function",
  ...
)
def my_remote_function(parameter: int) -> str:
  return str(parameter)
```

### Set a service account

As of version 2.0, BigQuery DataFrames no longer uses the
Compute Engine service account by default for the Cloud Run functions
it deploys. To limit the permissions of the function that you deploy, do the
following:

1. [Create a service account](https://docs.cloud.google.com/iam/docs/service-accounts-create) with minimal permissions.
2. Supply the service account email to the `cloud_function_service_account` parameter of the `@remote_function` decorator.

For example:

```python
@remote_function(
  cloud_function_service_account="my-service-account@my-project.iam.gserviceaccount.com",
  ...
)
def my_remote_function(parameter: int) -> str:
  return str(parameter)
```

If you would like to use the Compute Engine service account, you can set the
`cloud_function_service_account` parameter of the `@remote_function` decorator
to `"default"`. For example:

```python
# This usage is discouraged. Use only if you have a specific reason to use the
# default Compute Engine service account.
@remote_function(cloud_function_service_account="default", ...)
def my_remote_function(parameter: int) -> str:
  return str(parameter)
```

### Set ingress settings

As of version 2.0, BigQuery DataFrames sets the
[ingress settings of the Cloud Run functions](https://docs.cloud.google.com/functions/docs/networking/network-settings#ingress_settings) it
deploys to `"internal-only"`. Previously, the ingress settings were set to
`"all"` by default. You can change the ingress settings by setting the
`cloud_function_ingress_settings` parameter of the `@remote_function` decorator.
For example:

```python
@remote_function(cloud_function_ingress_settings="internal-and-gclb", ...)
def my_remote_function(parameter: int) -> str:
  return str(parameter)
```

## Use custom endpoints

In BigQuery DataFrames versions earlier than 2.0, if a region didn't
support
[regional service endpoints](https://docs.cloud.google.com/vpc/docs/regional-service-endpoints#bigquery) and
`bigframes.pandas.options.bigquery.use_regional_endpoints = True`, then
BigQuery DataFrames would fall back to
[locational endpoints](https://docs.cloud.google.com/storage/docs/locational-endpoints). Version 2.0 of
BigQuery DataFrames removes this fallback behavior. To connect to
locational endpoints in version 2.0, set the
`bigframes.pandas.options.bigquery.client_endpoints_override` option. For
example:

```python
import bigframes.pandas as bpd

bpd.options.bigquery.client_endpoints_override = {
  "bqclient": "https://LOCATION-bigquery.googleapis.com",
  "bqconnectionclient": "LOCATION-bigqueryconnection.googleapis.com",
  "bqstoragereadclient": "LOCATION-bigquerystorage.googleapis.com",
}
```

Replace <var translate="no">LOCATION</var> with the name of the BigQuery
location that you want to connect to.

## Use the `bigframes.ml.llm` module

In BigQuery DataFrames version 2.0, the default `model_name` for
`GeminiTextGenerator` has been updated to `"gemini-2.0-flash-001"`. It is
recommended that you supply a `model_name` directly to avoid breakages if the
default model changes in the future.

```python
import bigframes.ml.llm

model = bigframes.ml.llm.GeminiTextGenerator(model_name="gemini-2.0-flash-001")
```

## What's next

- Learn how to [visualize graphs using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-visualizations).
- Learn how to [generate BigQuery DataFrames code with Gemini](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#dataframe).
- Learn how to [analyze package downloads from PyPI with BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/dataframes/pypi.ipynb).
- View BigQuery DataFrames [source code](https://github.com/googleapis/python-bigquery-dataframes), [sample notebooks](https://github.com/googleapis/python-bigquery-dataframes/tree/main/notebooks), and [samples](https://github.com/googleapis/python-bigquery-dataframes/tree/main/samples/snippets) on GitHub.
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).