# Authorize accounts for data transfer

This document provides an overview of how BigQuery Data Transfer Service interacts with
different account types, the types of account authorization that you need to
perform general transfer tasks, and troubleshooting steps for common permission
errors.

To start using the BigQuery Data Transfer Service, ensure that the accounts
associated with your project---both user accounts and service accounts---are
authenticated and authorized with the correct permissions to perform your
transfer needs. For information about data source-specific permissions, see the
transfer guide for each data source.

## Key concepts

The BigQuery Data Transfer Service automates data transfers from various data
sources into BigQuery. The authentication and authorization model
operates at two different stages, the control plane and the data plane, and for
two types of users, a transfer creator or transfer owner.

### Control plane

The control plane represents the stage in the authorization process where an
authenticated user is able to control and manage transfer configurations and
runs. A user in the control plane must have the appropriate
Identity and Access Management (IAM) permissions to control and manage their transfer
configurations and runs:

- The `bigquery.transfers.update` permission, which lets users do the following:
  - Set up data transfer configurations.
  - Administer the existing transfers, such as updating, disabling or deleting a transfer.
- The `bigquery.transfers.get` permission, which lets users monitor transfer runs, such as checking transfer run status or viewing transfer run history and logs.

If you are using the Google Cloud console or the bq command-line tool to create a
transfer, you must also have the `bigquery.transfers.get` permission.

The `bigquery.transfers.update` permission is not required to set up a
[scheduled query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries). For more information, see
the [required permissions](https://docs.cloud.google.com/bigquery/docs/scheduling-queries#required_permissions)
for scheduled queries.

### Data plane

The data plane represents the stage outside of a user's direct control. In the
data plane, the BigQuery Data Transfer Service is able to operate data transfers in
an offline mode and can trigger transfer runs automatically based on a
user-specified schedule. In the data plane, the transfer owner's credential is
used to access the source data, and (depending on the data source) either the
transfer owner's credentials or the BigQuery Data Transfer Service [service agent](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#service_agent)
is used for starting BigQuery jobs and writing data into the
destination dataset.

For more details on required permissions, refer to the following sections in
this guide:

- [Read-access authorization for external data sources](https://docs.cloud.google.com/bigquery/docs/dts-authentication-authorization#read-access-external-data)
- [Authorization to start BigQuery jobs](https://docs.cloud.google.com/bigquery/docs/dts-authentication-authorization#start-bq-jobs)
- [Authorization to execute BigQuery jobs and write data to the destination dataset](https://docs.cloud.google.com/bigquery/docs/dts-authentication-authorization#execute-bq-jobs)

### Transfer creator versus transfer owner

A transfer creator refers to the user identity who created and set up the
transfer configuration. A BigQuery Data Transfer Service user and transfer creator
can be a user account or a [service account](https://docs.cloud.google.com/iam/docs/service-account-overview).

A transfer owner refers to the user identity that the BigQuery Data Transfer Service
uses to authorize the data transfer, specifically, for extracting the source
data. For [the data sources that support service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts#data_sources_with_service_account_support),
the transfer owner can be a user account or a service account. For other data
sources, the transfer owner must be a user account.

The transfer owner and transfer creator can have the same user identity, but it
is not a requirement. There are multiple ways of setting the transfer owner to
be a different user than the transfer creator:

- When creating a transfer, you can set the owner to a service account if the data source supports service accounts.
- Once a transfer has been created, you can transfer ownership to a new user account (or to a service account if the data source supports service accounts) that has the `bigquery.transfers.update` and `bigquery.transfers.get` permissions. You must be logged in to the new account when you [update the credentials](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#update_credentials).

## Read-access authorization for external data sources

The permissions required to read source data might vary from one data source to
another. For example, accessing
[Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#required_permissions)
requires read-access permissions to the Google Ads Customer ID.
Similarly, [Google Play](https://docs.cloud.google.com/bigquery/docs/play-transfer#required_permissions)
requires report access in the
Google Play console. For more information about permissions that are specific
to a data source, see the transfer guides for each data source.

Depending on the transfer owner's identity type, a different authorization
method is required to retrieve the access token to access the source data.

### Transfer owner as a service account

When a service account is used as the transfer owner, the necessary permissions
are automatically granted when the BigQuery Data Transfer Service API is enabled for
your project. The BigQuery Data Transfer Service uses a [*service
agent*](https://docs.cloud.google.com/iam/docs/service-account-types#service-agents) to get the access token
for the user-provided service account (transfer owner).

When you enable the BigQuery Data Transfer Service API, a [service agent](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#service_agent)
is created for your project. The system also grants the service agent the
[BigQuery Data Transfer Service Agent role](https://docs.cloud.google.com/bigquery/docs/access-control#bigquerydatatransfer.serviceAgent) (`roles/bigquerydatatransfer.serviceAgent`),
which includes the permission `iam.serviceAccounts.getAccessToken`. That
permission allows the BigQuery Data Transfer Service service agent to impersonate
the transfer owner service account to retrieve the access token.

For more information about the BigQuery Data Transfer Service service agent,
see [Service agent](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#service_agent).
For more information about using service accounts and the up-to-date list of
data sources that have service account support, see [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

> [!WARNING]
> **Warning:** Don't remove the predefined role from the service agent. The [BigQuery Data Transfer Service Agent role](https://docs.cloud.google.com/bigquery/docs/access-control#bigquerydatatransfer.serviceAgent) is required for the BigQuery Data Transfer Service to work.

### Transfer owner as a user account

If the transfer owner creating the transfer configuration is a user account (not
a service account), you must manually grant permission for the
BigQuery Data Transfer Service to get the access token for the user account and
access the source data on the transfer owner's behalf. You can grant manual
approval with the OAuth dialog interface.

You only need to give permission to the BigQuery Data Transfer Service for the
first time when creating a transfer for a given data source. You must give the
permission again when you create the first transfer for a newly used region,
even if you are using the same data source. Data transfers from Youtube Channels
are the exception - you must manually grant permissions approval every time you
create a Youtube Channel data transfer.

Changing the transfer owner by updating credentials also requires manual
approval if the new owner has never created a transfer for the data source in
that region before.

The following screenshot shows the OAuth dialog interface when you are creating a
Google Ads transfer. The dialog displays data source-specific
permissions to be given:

![Allow BigQuery Data Transfer Service to access Google Ads.](https://docs.cloud.google.com/static/bigquery/images/dts-auth-allow.png)

> [!NOTE]
> **Note:** The BigQuery Data Transfer Service no longer supports the `authorization_code` parameter for Youtube Channel data transfers. You can use the `version_info` parameter to provide your authorization result to the transfer to allow it to get credentials. The `version_info` parameter is only required in the `bq` CLI or API calls.

To revoke the permissions that were given, follow these steps:

1. Go to the [Google Account page](https://myaccount.google.com/).
2. Click **BigQuery Data Transfer Service**.
3. To revoke the permissions, click **Remove access** . ![Remove access that you've given to BigQuery Data Transfer Service.](https://docs.cloud.google.com/static/bigquery/images/dts-auth-remove.png)

> [!WARNING]
> **Warning:** Revoking access permissions prevents any future transfer runs for the transfer configurations that this account owns across all regions.

## Authorization to start BigQuery jobs

When you migrate from most data sources, except when migrating using [scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries)
or [dataset copies](https://docs.cloud.google.com/bigquery/docs/managing-datasets#copy-datasets), the
BigQuery Data Transfer Service relies on [service agents](https://docs.cloud.google.com/iam/docs/service-account-types#service-agents)
to start BigQuery jobs for
your project. The required permission `bigquery.job.create` is automatically
given to the [service agent](https://docs.cloud.google.com/bigquery/docs/access-control#bigquerydatatransfer.serviceAgent)
when you enable the BigQuery Data Transfer Service API for your project. For more
information, see [Enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).

When you migrate using [scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) or
[dataset copies](https://docs.cloud.google.com/bigquery/docs/managing-datasets#copy-datasets),
BigQuery Data Transfer Service uses the transfer owner's credentials to start the
BigQuery jobs.

> [!WARNING]
> **Warning:** Don't remove the predefined role from the service agent. The service agent role is required for BigQuery Data Transfer Service to work.

## Authorization to execute BigQuery jobs and write data to the destination dataset

When you migrate from most data sources, except when migrating using
[scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) or [dataset copies](https://docs.cloud.google.com/bigquery/docs/managing-datasets#copy-datasets),
the BigQuery Data Transfer Service relies on the service agent to write data into
the BigQuery destination dataset. The required permission,
`roles/bigquery.dataEditor`, is granted to the service agent by the
BigQuery Data Transfer Service when you create the transfer. You must have
`bigquery.datasets.update` permission on the destination dataset to successfully
grant the permission.

When you migrate using [scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) or [dataset copies](https://docs.cloud.google.com/bigquery/docs/managing-datasets#copy-datasets),
BigQuery Data Transfer Service uses the transfer owner's credentials to execute the
BigQuery jobs and write the data into BigQuery
destination dataset.

> [!NOTE]
> **Note:** The `roles/bigquery.dataEditor` role granted to the BigQuery Data Transfer Service agent is only limited to the destination dataset that is used in a transfer configuration. Other BigQuery datasets under the same project are not affected.

> [!WARNING]
> **Warning:** Don't remove the service agent's `roles/bigquery.dataEditor` role from the destination dataset. The `roles/bigquery.dataEditor` role is required for BigQuery Data Transfer Service to work.

## Troubleshoot permission errors

If you are encountering authorization or permissions related issues for your
transfer, see [Authorization and permission issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#authorization_and_permission_issues).