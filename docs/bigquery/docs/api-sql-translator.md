# Translate SQL queries with the translation API

This document describes how to use the translation API in
BigQuery to translate scripts written in other SQL dialects into
GoogleSQL queries. The translation API can simplify the process of [migrating workloads to BigQuery](https://docs.cloud.google.com/bigquery/docs/migration-intro).

## Before you begin

Before you submit a translation job, complete the following steps:

1. Ensure that you have all the required permissions.
2. Enable the BigQuery Migration API.
3. Collect the source files containing the SQL scripts and queries to be translated.
4. Upload the source files to Cloud Storage.

### Required permissions


To get the permissions that
you need to create translation jobs using the translation API,

ask your administrator to grant you the
[MigrationWorkflow Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquerymigration#bigquerymigration.editor) (`roles/bigquerymigration.editor`) IAM role on the `parent` resource.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create translation jobs using the translation API. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create translation jobs using the translation API:

- `bigquerymigration.workflows.create`
- `bigquerymigration.workflows.get`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Enable the BigQuery Migration API

If your Google Cloud CLI project was created before February 15, 2022, enable
the BigQuery Migration API as follows:

1. In the Google Cloud console, go to the **BigQuery Migration API** page.

   [Go to BigQuery Migration API](https://console.cloud.google.com/apis/api/bigquerymigration.googleapis.com/overview)
2. Click **Enable**.

> [!NOTE]
> **Note:** Projects created after February 15, 2022 have this API enabled automatically.

### Upload input files to Cloud Storage

If you want to use the Google Cloud console or the BigQuery Migration API
to perform a translation job, you must upload the source files containing
the queries and scripts you want to translate to Cloud Storage. You
can also upload [any metadata files](https://docs.cloud.google.com/bigquery/docs/generate-metadata) or [configuration YAML files](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation)
to the same Cloud Storage bucket containing the source files.
For more information about creating buckets and uploading files to
Cloud Storage, see [Create buckets](https://docs.cloud.google.com/storage/docs/creating-buckets)
and [Upload objects from a filesystem](https://docs.cloud.google.com/storage/docs/uploading-objects).

## Supported task types

The translation API can translate the
following SQL dialects into GoogleSQL:

- Amazon Redshift SQL - `Redshift2BigQuery_Translation`
- Apache HiveQL and Beeline CLI - `HiveQL2BigQuery_Translation`
- Apache Impala - `Impala2BigQuery_Translation`
- Apache Spark SQL - `SparkSQL2BigQuery_Translation`
- Azure Synapse T-SQL - `AzureSynapse2BigQuery_Translation`
- GoogleSQL (BigQuery) - `Bigquery2Bigquery_Translation`
- Greenplum SQL - `Greenplum2BigQuery_Translation`
- IBM Db2 SQL - `Db22BigQuery_Translation`
- IBM Netezza SQL and NZPLSQL - `Netezza2BigQuery_Translation`
- MySQL SQL - `MySQL2BigQuery_Translation`
- Oracle SQL, PL/SQL, Exadata - `Oracle2BigQuery_Translation`
- PostgreSQL SQL - `Postgresql2BigQuery_Translation`
- Presto or Trino SQL - `Presto2BigQuery_Translation`
- Snowflake SQL - `Snowflake2BigQuery_Translation`
- SQLite - `SQLite2BigQuery_Translation`
- SQL Server T-SQL - `SQLServer2BigQuery_Translation`
- Teradata and Teradata Vantage - `Teradata2BigQuery_Translation`
- Vertica SQL - `Vertica2BigQuery_Translation`

### Handling unsupported SQL functions with helper UDFs

When translating SQL from a source dialect to BigQuery, some functions might not have a direct equivalent. To address this, the BigQuery Migration Service (and the broader BigQuery community) provide helper user-defined functions (UDFs) that replicate the behavior of these unsupported source dialect functions.

These UDFs are often found in the `bqutil` public dataset, allowing translated queries to initially reference them using the format `bqutil.<dataset>.<function>()`. For example, `bqutil.fn.cw_count()`.

#### Important considerations for production environments:

While `bqutil` offers convenient access to these helper UDFs for initial translation and testing, direct reliance on `bqutil` for production workloads is not recommended for several reasons:

1. Version control: The `bqutil` project hosts the latest version of these UDFs, which means their definitions can change over time. Relying directly on `bqutil` could lead to unexpected behavior or breaking changes in your production queries if a UDF's logic is updated.
2. Dependency isolation: Deploying UDFs to your own project isolates your production environment from external changes.
3. Customization: You might need to modify or optimize these UDFs to better suit your specific business logic or performance requirements. This is only possible if they are within your own project.
4. Security and governance: Your organization's security policies might restrict direct access to public datasets like `bqutil` for production data processing. Copying UDFs to your controlled environment aligns with such policies.

#### Deploying helper UDFs to your project:

For reliable and stable production use, you should deploy these helper UDFs into your own project and dataset. This gives you full control over their version, customization, and access.
For detailed instructions on how to deploy these UDFs, refer to the [UDFs deployment guide on GitHub](https://github.com/GoogleCloudPlatform/bigquery-utils/tree/master/udfs#deploying-the-udfs). This guide provides the necessary scripts and steps to copy the UDFs into your environment.

## Locations

The translation API is available in the following
processing locations:

|   | **Region description** | **Region name** | **Details** |
|---|---|---|---|
| **Asia Pacific** ||||
|   | Bangkok | `asia-southeast3` |   |
|   | Delhi | `asia-south2` |   |
|   | Hong Kong | `asia-east2` |   |
|   | Jakarta | `asia-southeast2` |   |
|   | Melbourne | `australia-southeast2` |   |
|   | Mumbai | `asia-south1` |   |
|   | Osaka | `asia-northeast2` |   |
|   | Seoul | `asia-northeast3` |   |
|   | Singapore | `asia-southeast1` |   |
|   | Sydney | `australia-southeast1` |   |
|   | Taiwan | `asia-east1` |   |
|   | Tokyo | `asia-northeast1` |   |
| **Europe** ||||
|   | Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Berlin | `europe-west10` |   |
|   | EU multi-region | `eu` |
|   | Finland | `europe-north1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Frankfurt | `europe-west3` |   |
|   | London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Madrid | `europe-southwest1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Milan | `europe-west8` |   |
|   | Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Paris | `europe-west9` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Stockholm | `europe-north2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Turin | `europe-west12` |   |
|   | Warsaw | `europe-central2` |   |
|   | Zürich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| **Americas** ||||
|   | Columbus, Ohio | `us-east5` |   |
|   | Dallas | `us-south1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Las Vegas | `us-west4` |   |
|   | Los Angeles | `us-west2` |   |
|   | Mexico | `northamerica-south1` |   |
|   | Northern Virginia | `us-east4` |   |
|   | Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Québec | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Salt Lake City | `us-west3` |   |
|   | Santiago | `southamerica-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | South Carolina | `us-east1` |   |
|   | Toronto | `northamerica-northeast2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | US multi-region | `us` |
| **Africa** ||||
|   | Johannesburg | `africa-south1` |   |
| **MiddleEast** ||||
|   | Dammam | `me-central2` |   |
|   | Doha | `me-central1` |   |
|   | Israel | `me-west1` |   |

## Submit a translation job

To submit a translation job using the translation API, use the [`projects.locations.workflows.create`](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create)
method and supply an instance of the [`MigrationWorkflow`](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows#resource:-migrationworkflow)
resource with a [supported task type](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#supported_task_types).

Once the job is submitted, you can [issue a query to get results](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#explore_the_translation_output).

### Create a batch translation

The following `curl` command creates a batch translation job where the input
and output files are stored in Cloud Storage. The `source_target_mapping` field
contains a list that maps the source `literal` entries to an optional relative
path for the target output.

```
curl -d "{
  \"tasks\": {
      string: {
        \"type\": \"TYPE\",
        \"translation_details\": {
            \"target_base_uri\": \"TARGET_BASE\",
            \"source_target_mapping\": {
              \"source_spec\": {
                  \"base_uri\": \"BASE\"
              }
            },
            \"target_types\": \"TARGET_TYPES\",
        }
      }
  }
  }" \
  -H "Content-Type:application/json" \
  -H "Authorization: Bearer TOKEN" -X POST https://bigquerymigration.googleapis.com/v2alpha/projects/PROJECT_ID/locations/LOCATION/workflows
```

Replace the following:

- `TYPE`: the [task type](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#supported_task_types) of the translation, which determines the source and target dialect.
- `TARGET_BASE`: the base URI for all translation outputs.
- `BASE`: the base URI for all files read as sources for translation.
- `TARGET_TYPES` (optional): the generated output types. If not specified, SQL is generated.

  - `sql` (default): The translated SQL query files.
  - `suggestion`: AI generated suggestions.

  The output is stored in a subfolder in the output directory. The subfolder is named based on the value in `TARGET_TYPES`.
- `TOKEN`: the token for authentication. To generate
  a token, use the `gcloud auth print-access-token` command or the
  [OAuth 2.0 playground](https://developers.google.com/oauthplayground/) (use the scope `https://www.googleapis.com/auth/cloud-platform`).

- `PROJECT_ID`: the project to process the
  translation.

- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#locations) where the job is processed.

The preceding command returns a response that includes a workflow ID written in the format `projects/PROJECT_ID/locations/LOCATION/workflows/WORKFLOW_ID`.

#### Example batch translation

To translate the Teradata SQL scripts in the Cloud Storage directory
`gs://my_data_bucket/teradata/input/` and store the results in the
Cloud Storage directory `gs://my_data_bucket/teradata/output/`, you might use
the following query:

    {
      "tasks": {
         "task_name": {
           "type": "Teradata2BigQuery_Translation",
           "translation_details": {
             "target_base_uri": "gs://my_data_bucket/teradata/output/",
               "source_target_mapping": {
                 "source_spec": {
                   "base_uri": "gs://my_data_bucket/teradata/input/"
                 }
              },
           }
        }
      }
    }

> [!NOTE]
> **Note:** The string `"task_name"` in this example is an identifier for the translation task and can be set to any value you prefer.

This call will return a message containing the created workflow ID in the
`"name"` field:

    {
      "name": "projects/123456789/locations/us/workflows/12345678-9abc-def1-2345-6789abcdef00",
      "tasks": {
        "task_name": { /*...*/ }
      },
      "state": "RUNNING"
    }

To get the updated status for the workflow, [run a `GET` query](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#explore_the_translation_output).
The job sends outputs to Cloud Storage as it progresses. The job `state`
changes to `COMPLETED` after all the requested `target_types` are generated.
If the task succeeds, you can find the translated SQL query in
`gs://my_data_bucket/teradata/output`.

#### Example batch translation with AI suggestions

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** The translation API can call Gemini using BigQuery Vertex AI integration to generate suggestions to your translated SQL query based on your AI configuration YAML file.

The following example translates the Teradata SQL scripts located in the
`gs://my_data_bucket/teradata/input/` Cloud Storage directory and stores
results in the Cloud Storage directory `gs://my_data_bucket/teradata/output/`
with additional AI suggestion:

    {
      "tasks": {
         "task_name": {
           "type": "Teradata2BigQuery_Translation",
           "translation_details": {
             "target_base_uri": "gs://my_data_bucket/teradata/output/",
               "source_target_mapping": {
                 "source_spec": {
                   "base_uri": "gs://my_data_bucket/teradata/input/"
                 }
              },
              "target_types": "suggestion",
           }
        }
      }
    }

> [!NOTE]
> **Note:** To generate AI suggestions, the Cloud Storage source directory must contain at least one configuration YAML file with a suffix of `.ai_config.yaml`. To learn how to write the configuration YAML file for AI suggestions, see [Create a Gemini-based configuration YAML file](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#ai_yaml_guidelines).

After the task runs successfully, AI suggestions can be found in
`gs://my_data_bucket/teradata/output/suggestion` Cloud Storage directory.

### Create an interactive translation job with string literal inputs and outputs

The following `curl` command creates a translation job with string literal
inputs and outputs. The `source_target_mapping` field contains a list that maps the
source directories to an optional relative path for the target output.

```
curl -d "{
  \"tasks\": {
      string: {
        \"type\": \"TYPE\",
        \"translation_details\": {
        \"source_target_mapping\": {
            \"source_spec\": {
              \"literal\": {
              \"relative_path\": \"PATH\",
              \"literal_string\": \"STRING\"
              }
            }
        },
        \"target_return_literals\": \"TARGETS\",
        }
      }
  }
  }" \
  -H "Content-Type:application/json" \
  -H "Authorization: Bearer TOKEN" -X POST https://bigquerymigration.googleapis.com/v2alpha/projects/PROJECT_ID/locations/LOCATION/workflows
```

Replace the following:

- `TYPE`: the [task type](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#supported_task_types) of the translation, which determines the source and target dialect.
- `PATH`: the identifier of the literal entry, similar to a filename or path.
- `STRING`: string of literal input data (for example, SQL) to be translated.
- `TARGETS`: the expected targets that the user wants to be directly returned in the response in the `literal` format. These should be in the target URI format (for example, <var translate="no">GENERATED_DIR</var> + `target_spec.relative_path` + `source_spec.literal.relative_path`). Anything not in this list is not returned in the response. The generated directory, <var translate="no">GENERATED_DIR</var> for general SQL translations is `sql/`.
- `TOKEN`: the token for authentication. To generate a token, use the `gcloud auth print-access-token` command or the [OAuth 2.0 playground](https://developers.google.com/oauthplayground/) (use the scope `https://www.googleapis.com/auth/cloud-platform`).
- `PROJECT_ID`: the project to process the translation.
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#locations) where the job is processed.

The preceding command returns a response that includes a workflow ID written in the format `projects/PROJECT_ID/locations/LOCATION/workflows/WORKFLOW_ID`.

When your job completes, you can view the results by by [querying the job](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#explore_the_translation_output)
and examining the inline `translation_literals` field in the response after the
workflow completes.

#### Example Interactive Translation

To translate the Hive SQL string `select 1` interactively, you might use the
following query:

    "tasks": {
      string: {
        "type": "HiveQL2BigQuery_Translation",
        "translation_details": {
          "source_target_mapping": {
            "source_spec": {
              "literal": {
                "relative_path": "input_file",
                "literal_string": "select 1"
              }
            }
          },
          "target_return_literals": "sql/input_file",
        }
      }
    }

> [!NOTE]
> **Note:** The string `"task_name"` in this example is an identifier for the translation task and can be set to any value you prefer.

You can use any `relative_path` you would like for your literal, but the
translated literal will only appear in the results if you include
`sql/$relative_path` in your `target_return_literals`. You can also include
multiple literals in a single query, in which case each of their relative paths
must be included in `target_return_literals`.

This call will return a message containing the created workflow ID in the
`"name"` field:

    {
      "name": "projects/123456789/locations/us/workflows/12345678-9abc-def1-2345-6789abcdef00",
      "tasks": {
        "task_name": { /*...*/ }
      },
      "state": "RUNNING"
    }

To get the updated status for the workflow, [run a `GET` query](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#explore_the_translation_output).
The job is complete when `"state"` changes to `COMPLETED`. If the task succeeds,
you will find the translated SQL in the response message:

    {
      "name": "projects/123456789/locations/us/workflows/12345678-9abc-def1-2345-6789abcdef00",
      "tasks": {
        "string": {
          "id": "0fedba98-7654-3210-1234-56789abcdef",
          "type": "HiveQL2BigQuery_Translation",
          /* ... */
          "taskResult": {
            "translationTaskResult": {
              "translatedLiterals": [
                {
                  "relativePath": "sql/input_file",
                  "literalString": "-- Translation time: 2023-10-05T21:50:49.885839Z\n-- Translation job ID: projects/123456789/locations/us/workflows/12345678-9abc-def1-2345-6789abcdef00\n-- Source: input_file\n-- Translated from: Hive\n-- Translated to: BigQuery\n\nSELECT\n    1\n;\n"
                }
              ],
              "reportLogMessages": [
                ...
              ]
            }
          },
          /* ... */
        }
      },
      "state": "COMPLETED",
      "createTime": "2023-10-05T21:50:49.543221Z",
      "lastUpdateTime": "2023-10-05T21:50:50.462758Z"
    }

## Explore the translation output

After running the translation job, retrieve
the results by specifying the translation job workflow ID using the following
command:

```
curl \
-H "Content-Type:application/json" \
-H "Authorization:Bearer TOKEN" -X GET https://bigquerymigration.googleapis.com/v2alpha/projects/PROJECT_ID/locations/LOCATION/workflows/WORKFLOW_ID
```

Replace the following:

- `TOKEN`: the token for authentication. To generate a token, use the `gcloud auth print-access-token` command or the [OAuth 2.0 playground](https://developers.google.com/oauthplayground/) (use the scope `https://www.googleapis.com/auth/cloud-platform`).
- `PROJECT_ID`: the project to process the translation.
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/api-sql-translator#locations) where the job is processed.
- `WORKFLOW_ID`: the ID generated when you create a translation workflow.

The response contains the status of your migration workflow, and any completed
files in `target_return_literals`.

The response will contain the status of your migration workflow, and any
completed files in `target_return_literals`. You can poll this endpoint to check
your workflow's status.