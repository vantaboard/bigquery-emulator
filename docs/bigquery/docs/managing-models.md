# Manage models

This document shows you how to manage BigQuery ML models, including
copying models and renaming models.

## Required roles


To get the permissions that
you need to read and create BigQuery models,

ask your administrator to grant you the
[BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to read and create BigQuery models. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to read and create BigQuery models:

- To read information from models: `bigquery.models.getData`
- To create models: `bigquery.models.create`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Rename models

You cannot change the name of an existing model. If you need to
change the model's name, follow the steps to [copy the model](https://docs.cloud.google.com/bigquery/docs/managing-models#copy-model). When
you specify the destination in the copy operation, use the new model name.

## Copy models

You can copy one or more models from a source dataset to a destination dataset
by:

- Using the Google Cloud console.
- Using the bq command-line tool's `bq cp` command.
- Calling the [jobs.insert](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API method directly and configuring a [copy job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationTableCopy) or by using the client libraries.

### Limitations on copying models

Model copy jobs are subject to the following limitations:

- When you copy a model, the name of the destination model must adhere to the same naming conventions as when you [create a model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#model_name).
- Model copies are subject to BigQuery [limits](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) on copy jobs.
- Copying a model is not supported by the Google Cloud console.
- Copying multiple source models in a single command is not supported.
- When you copy a model by using the CLI, the `--destination_kms_key` flag is not supported.

### Copy a model

You can copy a model by:

- Using the command-line tool's `bq cp` command
- Calling the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API method and configuring a [copy job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationTableCopy) or by using the client libraries

To copy a model:

### Console

The Google Cloud console does not support copying models.

### bq

Issue the `bq cp` command. Optional flags:

- `-f` or `--force` overwrites an existing model in the destination dataset and doesn't prompt you for confirmation.
- `-n` or `--no_clobber` returns the following error message if the model
  exists in the destination dataset: `'[PROJECT_ID]:[DATASET].[MODEL]'
  already exists, skipping`.

  If `-n` is not specified, the default behavior is to prompt you to choose
  whether to replace the destination model.

> [!NOTE]
> **Note:** The `--destination_kms_key` flag is not supported when you copy a model.

If the source or destination dataset is in a project other than your default
project, add the project ID to the dataset names in the following format:
`PROJECT_ID:DATASET`.

Supply the
`--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

```bash
bq --location=LOCATION cp -f -n PROJECT_ID:DATASET.SOURCE_MODEL PROJECT_ID:DATASET.DESTINATION_MODEL
```

Replace the following:

- <var translate="no">LOCATION</var>: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags). For a full list of locations, see [BigQuery locations](https://docs.cloud.google.com/bigquery/docs/locations).
- <var translate="no">PROJECT_ID</var>: your project ID.
- <var translate="no">DATASET</var>: the name of the source or destination dataset.
- <var translate="no">SOURCE_MODEL</var>: the model you're copying.
- <var translate="no">DESTINATION_MODEL</var>: the name of the model in the destination dataset.

Examples:

Enter the following command to copy `mydataset.mymodel` to
`mydataset2`.

Both datasets are in your default project and were
created in the `US` multi-region location.

```
bq --location=US cp mydataset.mymodel mydataset2.mymodel
```

Enter the following command to copy `mydataset.mymodel` and to overwrite a
destination model with the same name. The source dataset is in your default
project. The destination dataset is in `myotherproject`. The `-f` shortcut is
used to overwrite the destination model without a prompt.
`mydataset` and
`myotherdataset` were created in the `US` multi-region
location.

```
bq --location=US cp -f mydataset.mymodel myotherproject:myotherdataset.mymodel
```

Enter the following command to copy `mydataset.mymodel` and to return an error
if the destination dataset contains a model with the same name. The source
dataset is in your default project. The destination dataset is in
`myotherproject`. The `-n` shortcut is used to prevent overwriting a model with
the same name.
Both
datasets were created in the `US` multi-region location.

```
bq --location=US cp -n mydataset.mymodel myotherproject:myotherdataset.mymodel
```

Enter the following command to copy `mydataset.mymodel` to `mydataset2` and
to rename the model `mymodel2`. Both datasets are in your default project.
Both datasets were created in the `asia-northeast1` region.

```
bq --location=asia-northeast1 cp mydataset.mymodel mydataset2.mymodel2
```

### API

To copy a model by using the API, call the
[`bigquery.jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method and configure a `copy` job. Specify your location in the
`location` property in the `jobReference` section of the
[job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

You must specify the following values in your job configuration:

```
"copy": {
      "sourceTable": {       // Required
        "projectId": string, // Required
        "datasetId": string, // Required
        "tableId": string    // Required
      },
      "destinationTable": {  // Required
        "projectId": string, // Required
        "datasetId": string, // Required
        "tableId": string    // Required
      },
      "createDisposition": string,  // Optional
      "writeDisposition": string,   // Optional
    },
```

Where:

- `sourceTable`: provides information about the model to be copied.
- `destinationTable`: provides information about the new model.
- `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationTableCopy.FIELDS.create_disposition`: specifies whether to create the model if it doesn't exist.
- `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationTableCopy.FIELDS.write_disposition`: specifies whether to overwrite an existing model.

## Encrypt models

For more information about using a customer-managed encryption key (CMEK) to
encrypt a model, see
[Use CMEK to protect BigQuery ML models](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#cmek-bqml).

## What's next

- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To get started using BigQuery ML, see [Create machine learning models in BigQuery ML](https://docs.cloud.google.com/bigquery/docs/create-machine-learning-model).
- To learn more about working with models, see:
  - [Get model metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata)
  - [List models](https://docs.cloud.google.com/bigquery/docs/listing-models)
  - [Update model metadata](https://docs.cloud.google.com/bigquery/docs/updating-model-metadata)
  - [Delete models](https://docs.cloud.google.com/bigquery/docs/deleting-models)
  - [Manage models with Vertex AI](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex)