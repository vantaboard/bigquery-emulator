# Configure and use entity resolution in BigQuery

This document describes how to implement
[entity resolution](https://docs.cloud.google.com/bigquery/docs/entity-resolution-intro)
for end users and identity providers.

You can use this document to connect with an identity provider and use their
service to match records. Identity providers can use this document to
set up services to share with you on the Google Cloud Marketplace.

## Workflow for end users

The following sections show you how to configure entity resolution in
BigQuery. For a visual representation of the complete setup, see
[entity resolution architecture](https://docs.cloud.google.com/bigquery/docs/entity-resolution-intro#architecture).

### Before you begin

1. Contact an identity provider. BigQuery supports entity resolution with [LiveRamp](mailto:LiveRampIdentitySupport@liveramp.com) and [TransUnion](mailto:PDLtucloudappsupport@transunion.com).
2. Get the following items from the identity provider:
   - Service account credentials
   - Remote function signature
3. Create two datasets in your Google Cloud project:
   - Input dataset
   - Output dataset

### Required roles


To get the permissions that
you need to run entity resolution jobs,

ask your administrator to grant you the
following IAM roles:

- For the identity provider's service account to read the input dataset and write to the output dataset:
  - [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the input dataset
  - [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the output dataset


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Translate or resolve entities

For identity provider-specific instructions, see the following sections.

### LiveRamp

#### Prerequisites

- Configure LiveRamp Embedded Identity in BigQuery. For more information, see [Enabling LiveRamp Embedded Identity in BigQuery](https://docs.liveramp.com/identity/en/liveramp-embedded-identity-in-bigquery.html#enabling-liveramp-embedded-identity-in-bigquery).
- Coordinate with LiveRamp to enable API credentials for use with Embedded Identity. For more information, see [Authentication](https://docs.liveramp.com/identity/en/liveramp-embedded-identity-in-bigquery.html#id74765).

#### Setup

The following steps are required when you use LiveRamp Embedded Identity for the
first time. After setup, you only need to modify the input table and metadata
table between runs.

##### Create an input table

Create a table in the input dataset. Populate the table with RampIDs, target
domains, and target types. For details and examples, see
[Input Table Columns and Descriptions](https://docs.liveramp.com/identity/en/perform-rampid-transcoding-in-bigquery.html#input-table-columns-and-descriptions).

##### Create a metadata table

The metadata table controls the execution of LiveRamp Embedded
Identity on BigQuery. Create a metadata table in the input
dataset. Populate the metadata table with client IDs, execution modes, target
domains, and target types. For details and examples, see
[Metadata Table Columns and Descriptions](https://docs.liveramp.com/identity/en/perform-rampid-transcoding-in-bigquery.html#metadata-table-columns-and-descriptions).

##### Share tables with LiveRamp

Grant the LiveRamp Google Cloud service account access to view and
process data in your input dataset. For details and examples, see
[Share Tables and Datasets with LiveRamp](https://docs.liveramp.com/identity/en/perform-rampid-transcoding-in-bigquery.html#share-tables-and-datasets-with-liveramp-71).

#### Run an embedded identity job

To run an embedded identity job with LiveRamp in BigQuery,
complete the following steps:

1. Confirm that all RampIDs that were encoded in your domain are in your input table.
2. Confirm that your metadata table is still accurate before you run the job.
3. Contact [LiveRampIdentitySupport@liveramp.com](mailto:LiveRampIdentitySupport@liveramp.com) with a job process request. Include the project ID, dataset ID, and table ID (if applicable) for your input table, metadata table, and output dataset.

Results are generally delivered to your output dataset within three business days.

#### LiveRamp support

For support issues, contact
[LiveRamp Identity Support](mailto:LiveRampIdentitySupport@liveramp.com).

#### LiveRamp billing

[LiveRamp](https://cloud.google.com/find-a-partner/partner/liveramp)
handles billing for entity resolution.

### TransUnion

#### Prerequisites

- Contact [TransUnion Cloud Support](mailto:PDLtucloudappsupport@transunion.com) to sign an agreement to access the service. Provide your Google Cloud project ID, input data types, use case, and data volume.
- TransUnion Cloud Support enables the service for your Google Cloud project and shares a detailed implementation guide that includes available output data.

#### Setup

The following steps are required when you use TransUnion's TruAudience
Identity Resolution and Enrichment service in your BigQuery
environment.

##### Create an external connection

[Create a connection to an external data source](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection)
of the **Vertex AI remote models, remote functions and BigLake (Cloud Resource)**
type. You use this connection to trigger the identity resolution service
hosted in the TransUnion Google Cloud account from your
Google Cloud account.

Copy the connection ID and service account ID and share these
identifiers with the TransUnion customer delivery team.

##### Create a remote function

[Create a remote function](https://docs.cloud.google.com/bigquery/docs/remote-functions#create_a_remote_function)
to interact with the service orchestrator endpoint hosted on the
TransUnion Google Cloud project to pass the necessary metadata (including
schema mappings) to the TransUnion service. Use the connection ID from the
external connection that you created and the TransUnion-hosted cloud function
endpoint shared by the TransUnion customer delivery team.

##### Create an input table

Create a table in the input dataset. TransUnion supports name, postal address,
email, phone, date of birth, IPv4 address, and device IDs as inputs. Follow
the formatting guidelines in the implementation guide that TransUnion shared
with you.

##### Create a metadata table

Create a metadata table to store the configuration required by the
identity resolution service to process data, including schema mappings. For
details and examples, see the implementation guide that TransUnion shared
with you.

##### Create a job status table

Create a table to receive updates about the processing of an input
batch. You can query this table to trigger other downstream processes in your
pipeline. Possible job statuses include `RUNNING`, `COMPLETED`, or `ERROR`.

##### Create the service invocation

Use the following procedure to call the TransUnion identity resolution service
after collecting all the metadata, packaging it, and passing it to the
invocation cloud function endpoint hosted by TransUnion.

    -- create service invocation procedure
    CREATE OR REPLACE
      PROCEDURE
        `<project_id>.<dataset_id>.TransUnion_get_identities`(metadata_table STRING, config_id STRING)
          begin
            declare sql_query STRING;

    declare json_result STRING;
    declare base64_result STRING;

    SET sql_query =
      '''select to_json_string(array_agg(struct(config_id,key,value))) from `''' || metadata_table
      || '''` where  config_id="''' || config_id || '''" ''';

    EXECUTE immediate sql_query INTO json_result;

    SET base64_result = (SELECT to_base64(CAST(json_result AS bytes)));

    SELECT `<project_id>.<dataset_id>.remote_call_TransUnion_er`(base64_result);

    END;

##### Create the matching output table

Run the following SQL script to create the matching output table. This is the
standard output of the application, which includes match flags, scores,
persistent individual IDs, and household IDs.

    -- create output table
    CREATE TABLE `<project_id>.<dataset_id>.TransUnion_identity_output`(
      batchid STRING,
      uniqueid STRING,
      ekey STRING,
      hhid STRING,
      collaborationid STRING,
      firstnamematch STRING,
      lastnamematch STRING,
      addressmatches STRING,
      addresslinkagescores STRING,
      phonematches STRING,
      phonelinkagescores STRING,
      emailmatches STRING,
      emaillinkagescores STRING,
      dobmatches STRING,
      doblinkagescore STRING,
      ipmatches STRING,
      iplinkagescore STRING,
      devicematches STRING,
      devicelinkagescore STRING,
      lastprocessed STRING);

##### Configure metadata

Follow the implementation guide that TransUnion shared with you to map your
input schema to the application schema. This metadata also configures the
generation of collaboration IDs, which are shareable non-persistent
identifiers that can be used in data clean rooms.

#### Grant read and write access

Obtain the service account ID of the Apache Spark connection
from the TransUnion customer delivery team and grant it read and write
access to the dataset containing the input and output tables. We recommend
providing the service account ID with a
[BigQuery Data Editor role](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor)
on the dataset.

#### Invoke the application

You can invoke the application from within your environment by running the
following script.

> [!NOTE]
> **Note:** You can use multiple input tables, as long as they are mapped to different metadata configurations.

    call `<project_id>.<dataset_id>.TransUnion_get_identities`("<project_id>.<dataset_id>.TransUnion_er_metadata","1");
    -- using metadata table, and 1 = config_id for the batch run

#### Support

For technical issues, contact
[TransUnion Cloud Support](mailto:PDLtucloudappsupport@transunion.com).

#### Billing and usage

TransUnion tracks usage of the application and uses it for billing purposes.
Active customers can contact their TransUnion delivery representative
for more information.

## Workflow for identity providers

The following sections show you how to configure entity
resolution in BigQuery. For a visual representation of the
complete setup, see [entity resolution architecture](https://docs.cloud.google.com/bigquery/docs/entity-resolution-intro#architecture).

### Before you begin

1. Create a [Cloud Run](https://docs.cloud.google.com/run/docs/overview/what-is-cloud-run) job or a [Cloud Run function](https://docs.cloud.google.com/functions/docs/concepts/overview#functions) to integrate with the remote function. Both options are suitable for this purpose.
2. Get the name of the service account that's associated with the
   Cloud Run or Cloud Run function:

   1. In the Google Cloud console, go to the **Cloud Functions** page.

      [Go to Cloud Functions](https://console.cloud.google.com/functions)
   2. Click the function's name, and then click the **Details** tab.

   3. In the **General Information** pane, find and record the service
      account name for the remote function.

3. Create a
   [remote function](https://docs.cloud.google.com/bigquery/docs/remote-functions#create_a_remote_function).

4. Get end-user principals from the end user.

### Required roles


To get the permissions that
you need to run entity resolution jobs,

ask your administrator to grant you the
following IAM roles:

- For the service account that's associated with your function to read and write to associated datasets and launch jobs:
  - [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the project
  - [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`) on the project
- For the end-user principal to see and connect to the remote function:
  - [BigQuery Connection User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`) on the connection
  - [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the control plane dataset with the remote function


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Share entity resolution remote function

Modify and share the following remote interface code with the end user. The end
user needs this code to start the entity resolution job.

    `PARTNER_PROJECT_ID.DATASET_ID`.match`(LIST_OF_PARAMETERS)

Replace <var translate="no">LIST_OF_PARAMETERS</var> with the list of parameters that are
passed to the remote function.

### Optional: Provide job metadata

You can optionally provide job metadata by using a separate remote function
or by writing a new status table in the user's output dataset. Examples of
metadata include job statuses and metrics.

## Billing for identity providers

To streamline customer billing and onboarding, integrate your entity resolution service with the
[Google Cloud Marketplace](https://docs.cloud.google.com/marketplace).
This lets you set up a
[pricing model](https://docs.cloud.google.com/marketplace/docs/partners/integrated-saas/select-pricing)
based on the entity resolution job usage, with Google handling the billing for
you. For more information, see
[Offering software as a service (SaaS) products](https://docs.cloud.google.com/marketplace/docs/partners/integrated-saas).

## What's next

- Learn about [entity resolution in BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/entity-resolution-intro).
- Learn how to [create a remote function](https://docs.cloud.google.com/bigquery/docs/remote-functions#create_a_remote_function).
- Learn how to [create a connection to an external data source](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection).
- For identity providers, learn how to [make your entity resolution service available on Google Cloud Marketplace](https://docs.cloud.google.com/marketplace/docs/partners/integrated-saas).