# Use open source Python libraries

You can choose from among three Python libraries in BigQuery,
based on your use case.

|   | Use case | Maintained by | Description |
|---|---|---|---|
| BigQuery DataFrames | Python based data processing and ML operations with server-side processing (for example, using slots) | Google | Pandas and scikit-learn APIs implemented with server-side pushdown. For more information, see [Introduction to BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction). |
| pandas-gbq | Python based data processing using client side data copy | Open source library maintained by PyData and volunteer contributors | Lets you move data to and from Python DataFrames on the client side. For more information, see the [documentation](https://googleapis.dev/python/pandas-gbq/latest/index.html) and [source code](https://github.com/googleapis/python-bigquery-pandas). |
| google-cloud-bigquery | BigQuery deployment, administration, and SQL-based querying | Open source library maintained by Google | Python package that wraps all the BigQuery APIs. For more information, see the [documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest) and [source code](https://github.com/googleapis/python-bigquery). |

## Using pandas-gbq and google-cloud-bigquery

The `pandas-gbq` library provides a simple interface for running queries and
uploading pandas DataFrames to BigQuery. It is a thin wrapper
around the [BigQuery client library](https://docs.cloud.google.com/bigquery/docs/reference/libraries),
`google-cloud-bigquery`. Both of these libraries focus on helping you perform
data analysis using SQL.

### Install the libraries

To use the code samples in this guide, install the `pandas-gbq` package and the
BigQuery Python client libraries.

Install the
[`pandas-gbq`](https://pypi.org/project/pandas-gbq/) and
[`google-cloud-bigquery`](https://pypi.org/project/google-cloud-bigquery/)
packages.

    pip install --upgrade pandas-gbq 'google-cloud-bigquery[bqstorage,pandas]'

### Running Queries

Both libraries support querying data stored in BigQuery. Key
differences between the libraries include:

|   | pandas-gbq | google-cloud-bigquery |
|---|---|---|
| Default SQL syntax | GoogleSQL (configurable with `pandas_gbq.context.dialect`) | GoogleSQL |
| Query configurations | Sent as dictionary in the format of a [query request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#QueryRequest). | Use the [`QueryJobConfig`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig) class, which contains properties for the various API configuration options. |

#### Querying data with the GoogleSQL syntax

The following sample shows how to run a GoogleSQL query with and without
explicitly specifying a project. For both libraries, if a project is not
specified, the project will be determined from the
[default credentials](https://googleapis.dev/python/google-auth/latest/reference/google.auth.html#google.auth.default).

> [!NOTE]
> **Note:** The `pandas.read_gbq` method defaults to legacy SQL. To use standard SQL, you must explicitly set the `dialect` parameter to `'standard'`, as shown.

**`pandas-gbq`:**

    import pandas

    sql = """
        SELECT name
        FROM `bigquery-public-data.usa_names.usa_1910_current`
        WHERE state = 'TX'
        LIMIT 100
    """

    # Run a Standard SQL query using the environment's default project
    df = pandas.read_gbq(sql, dialect="standard")

    # Run a Standard SQL query with the project set explicitly
    project_id = "your-project-id"
    df = pandas.read_gbq(sql, project_id=project_id, dialect="standard")

<br />

**`google-cloud-bigquery`:**

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
    sql = """
        SELECT name
        FROM `bigquery-public-data.usa_names.usa_1910_current`
        WHERE state = 'TX'
        LIMIT 100
    """

    # Run a Standard SQL query using the environment's default project
    df = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql).to_dataframe()

    # Run a Standard SQL query with the project set explicitly
    project_id = "your-project-id"
    df = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql, project=project_id).to_dataframe()

<br />

#### Querying data with the legacy SQL syntax

The following sample shows how to run a query using legacy SQL syntax. See the
[GoogleSQL migration guide](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql)
for guidance on updating your queries to GoogleSQL.

**`pandas-gbq`:**

    import pandas

    sql = """
        SELECT name
        FROM [bigquery-public-data:usa_names.usa_1910_current]
        WHERE state = 'TX'
        LIMIT 100
    """

    df = pandas.read_gbq(sql, dialect="legacy")

<br />

**`google-cloud-bigquery`:**

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
    sql = """
        SELECT name
        FROM [bigquery-public-data:usa_names.usa_1910_current]
        WHERE state = 'TX'
        LIMIT 100
    """
    query_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(use_legacy_sql=True)

    df = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql, job_config=query_config).to_dataframe()

<br />

#### Using the BigQuery Storage API to download large results

Use the [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage) to [speed up
downloads of large results by 15 to 31
times](https://friendliness.dev/2019/07/29/bigquery-arrow/).

**`pandas-gbq`:**

    import pandas

    sql = "SELECT * FROM `bigquery-public-data.irs_990.irs_990_2012`"

    # Use the BigQuery Storage API to download results more quickly.
    df = pandas.read_gbq(sql, dialect="standard", use_bqstorage_api=True)

<br />

**`google-cloud-bigquery`:**

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
    sql = "SELECT * FROM `bigquery-public-data.irs_990.irs_990_2012`"

    # The client library uses the BigQuery Storage API to download results to a
    # pandas dataframe if the API is enabled on the project, the
    # `google-cloud-bigquery-storage` package is installed, and the `pyarrow`
    # package is installed.
    df = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql).to_dataframe()

<br />

#### Running a query with a configuration

Sending a configuration with a BigQuery API request is required
to perform certain complex operations, such as running a parameterized query or
specifying a destination table to store the query results. In `pandas-gbq`, the
configuration must be sent as a dictionary in the format of a [query request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#QueryRequest).
In `google-cloud-bigquery`, job configuration classes are provided, such as
[`QueryJobConfig`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig),
which contain the necessary properties to configure complex jobs.

The following sample shows how to run a query with named parameters.

**`pandas-gbq`:**

    import pandas

    sql = """
        SELECT name
        FROM `bigquery-public-data.usa_names.usa_1910_current`
        WHERE state = @state
        LIMIT @limit
    """
    query_config = {
        "query": {
            "parameterMode": "NAMED",
            "queryParameters": [
                {
                    "name": "state",
                    "parameterType": {"type": "STRING"},
                    "parameterValue": {"value": "TX"},
                },
                {
                    "name": "limit",
                    "parameterType": {"type": "INTEGER"},
                    "parameterValue": {"value": 100},
                },
            ],
        }
    }

    df = pandas.read_gbq(sql, configuration=query_config)

<br />

**`google-cloud-bigquery`:**

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
    sql = """
        SELECT name
        FROM `bigquery-public-data.usa_names.usa_1910_current`
        WHERE state = @state
        LIMIT @limit
    """
    query_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        query_parameters=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html("state", "STRING", "TX"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.query.ScalarQueryParameter.html("limit", "INTEGER", 100),
        ]
    )

    df = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql, job_config=query_config).to_dataframe()

<br />

### Loading a pandas DataFrame to a BigQuery table

Both libraries support uploading data from a pandas DataFrame to a new table in
BigQuery. Key differences include:

|   | pandas-gbq | google-cloud-bigquery |
|---|---|---|
| Type support | Converts the DataFrame to CSV format before sending to the API, which does not support nested or array values. | Converts the DataFrame to Parquet or CSV format before sending to the API, which supports nested and array values. Choose Parquet for struct and array values and CSV for date and time serialization flexibility. Parquet is the default choice. Note that `pyarrow`, which is the parquet engine used to send the DataFrame data to the BigQuery API, must be installed to load the DataFrame to a table. |
| Load configurations | You can optionally specify a [table schema](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableSchema). | Use the [`LoadJobConfig`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig) class, which contains properties for the various API configuration options. |

**`pandas-gbq`:**

    import pandas

    df = pandas.DataFrame(
        {
            "my_string": ["a", "b", "c"],
            "my_int64": [1, 2, 3],
            "my_float64": [4.0, 5.0, 6.0],
            "my_timestamp": [
                pandas.Timestamp("1998-09-04T16:03:14"),
                pandas.Timestamp("2010-09-13T12:03:45"),
                pandas.Timestamp("2015-10-02T16:00:00"),
            ],
        }
    )
    table_id = "my_dataset.new_table"

    df.to_gbq(table_id)

<br />

**`google-cloud-bigquery`:**
The `google-cloud-bigquery` package requires the `pyarrow` library to serialize a pandas DataFrame to a Parquet file.

<br />

Install the `pyarrow` package:

     pip install pyarrow


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest
    import pandas

    df = pandas.DataFrame(
        {
            "my_string": ["a", "b", "c"],
            "my_int64": [1, 2, 3],
            "my_float64": [4.0, 5.0, 6.0],
            "my_timestamp": [
                pandas.Timestamp("1998-09-04T16:03:14"),
                pandas.Timestamp("2010-09-13T12:03:45"),
                pandas.Timestamp("2015-10-02T16:00:00"),
            ],
        }
    )
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
    table_id = "my_dataset.new_table"
    # Since string columns use the "object" dtype, pass in a (partial) schema
    # to ensure the correct BigQuery data type.
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        schema=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("my_string", "STRING"),
        ]
    )

    job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_dataframe(df, table_id, job_config=job_config)

    # Wait for the load job to complete.
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()

<br />

### Features not supported by pandas-gbq

While the `pandas-gbq` library provides a useful interface for querying data
and writing data to tables, it does not cover many of the
BigQuery API features, including but not limited to:

- [Managing datasets](https://docs.cloud.google.com/bigquery/docs/datasets), including [creating new datasets](https://docs.cloud.google.com/bigquery/docs/datasets), [updating dataset properties](https://docs.cloud.google.com/bigquery/docs/updating-datasets), and [deleting datasets](https://docs.cloud.google.com/bigquery/docs/managing-datasets#delete-datasets)
- [Loading data into BigQuery](https://docs.cloud.google.com/bigquery/docs/loading-data) from formats other than pandas DataFrames or from pandas DataFrames with JSON columns
- [Managing tables](https://docs.cloud.google.com/bigquery/docs/managing-tables), including [listing tables in a dataset](https://docs.cloud.google.com/bigquery/docs/tables#list_tables_in_a_dataset), [copying table data](https://docs.cloud.google.com/bigquery/docs/managing-tables#copying_a_single_source_table), and [deleting tables](https://docs.cloud.google.com/bigquery/docs/managing-tables#deleting_a_table)
- [Exporting BigQuery data](https://docs.cloud.google.com/bigquery/docs/exporting-data) directly to Cloud Storage

## Troubleshooting connection pool errors

Error string: `Connection pool is full, discarding connection: bigquery.googleapis.com.
Connection pool size: 10`

If you use the default BigQuery client object in Python, you are
limited to a maximum of 10 threads because the default pool size for the [Python HTTPAdapter](https://docs.python-requests.org/en/latest/api/#requests.adapters.HTTPAdapter)
is 10. To use more than 10 connections, create a custom `requests.adapters.HTTPAdapter`
object. For example:

```bash
client = bigquery.Client()
adapter = requests.adapters.HTTPAdapter(pool_connections=128,
pool_maxsize=128,max_retries=3)
client._http.mount("https://",adapter)
client._http._auth_request.session.mount("https://",adapter)
query_job = client.query(QUERY)
```