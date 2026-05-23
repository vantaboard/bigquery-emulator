# Use the Simba ODBC and JDBC drivers for BigQuery

The Simba Open Database Connectivity (ODBC) and Java Database Connectivity
(JDBC) drivers for BigQuery connect your applications to
BigQuery, letting you use BigQuery features with
your preferred tooling and infrastructure. Generally, the JDBC driver is used
with Java applications, and the ODBC driver is used otherwise.

The Simba ODBC and JDBC drivers are developed by
[insightsoftware](https://insightsoftware.com/simba/), a
[Google Cloud Ready - BigQuery partner](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-overview).
As an alternative to the Simba JDBC driver, a
[Google-developed JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery)
is available in
[Preview](https://cloud.google.com/products#product-launch-stages).

## Limitations

The Simba ODBC and JDBC drivers for BigQuery are subject to the
following limitations:

- [BigQuery load features](https://docs.cloud.google.com/bigquery/docs/loading-data) aren't supported.
- [BigQuery export features](https://docs.cloud.google.com/bigquery/docs/export-intro) aren't supported.
- [Query prefixes](https://docs.cloud.google.com/bigquery/docs/introduction-sql#sql) aren't supported.
- All [data manipulation language (DML) limitations](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations) apply.
- [Parameterized queries](https://docs.cloud.google.com/bigquery/docs/parameterized-queries) only provide query validation. Query performance isn't affected.
- The drivers are specific to BigQuery and can't be used with other products or services.

## Before you begin

When you use the Simba ODBC and JDBC drivers for BigQuery, you
have the option to read data with the BigQuery Storage Read API, instead of with
the standard BigQuery API. In the insightsoftware documentation, this
feature is called the *High-Throughput API* . If you plan to use this optional
feature, ensure that you have the [required roles](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#high-throughput-roles).

### Required roles for the High-Throughput API


To get the permissions that
you need to use the High-Throughput API,

ask your administrator to grant you the
[BigQuery Read Session User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.readSessionUser) (`roles/bigquery.readSessionUser`) IAM role on your BigQuery project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to use the High-Throughput API. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to use the High-Throughput API:

- `resourcemanager.projects.get`
- `resourcemanager.projects.list`
- `bigquery.readsessions.create`
- `bigquery.readsessions.getData`
- `bigquery.readsessions.update`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Install and configure the Simba ODBC driver for BigQuery

1. Download the 3.1.6.3037 version of the driver for your operating system:

   - [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.1.6.3037.msi) (`.msi` file)
   - [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.1.6.3037.msi) (`.msi` file)
   - [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.1.6.3037-Linux.tar.gz) (`.tar.gz` file)
   - [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.1.6.3037.dmg) (`.dmg` file)
2. Follow the instructions in the
   [insightsoftware installation and configuration guide](https://storage.googleapis.com/simba-bq-release/odbc/Simba%20Google%20BigQuery%20ODBC%20Connector%20Install%20and%20Configuration%20Guide-3.1.6.3037.pdf).

For information on feature changes and workflow updates, see
[Simba Google BigQuery ODBC Data Connector Release Notes](https://storage.googleapis.com/simba-bq-release/odbc/release-notes-3.1.6.3037.txt).

To see a list of previous driver versions, expand the following section:

### Previous Simba ODBC driver versions

#### 3.1.6.1026

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.1.6.1026.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.1.6.1026.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.1.6.1026-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.1.6.1026.dmg)

#### 3.1.5.1022

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.1.5.1022.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.1.5.1022.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.1.5.1022-Linux.tar)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.1.5.1022.dmg)

#### 3.1.4.1020

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.1.4.1020.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.1.4.1020.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.1.4.1020-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.1.4.1020.dmg)

#### 3.1.2.1009

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.1.2.1009.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.1.2.1009.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.1.2.1009-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.1.2.1009.dmg)

#### 3.1.2.1004

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.1.2.1004.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.1.2.1004.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.1.2.1004-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.1.2.1004.dmg)

#### 3.0.7.1016

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.0.7.1016.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.0.7.1016.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.0.7.1016-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.0.7.1016.dmg)

#### 3.0.5.1011

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.0.5.1011.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.0.5.1011.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.0.5.1011-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.0.5.1011.dmg)

#### 3.0.4.1008

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.0.4.1008.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.0.4.1008.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.0.4.1008-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.0.4.1008.dmg)

#### 3.0.3.1006

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.0.3.1006.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.0.3.1006.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.0.3.1006-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.0.3.1006.dmg)

#### 3.0.2.1005

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.0.2.1005.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.0.2.1005.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.0.2.1005-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.0.2.1005.dmg)

#### 3.0.0.1001

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_3.0.0.1001.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_3.0.0.1001.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_3.0.0.1001-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-3.0.0.1001.dmg)

#### 2.5.2.1004

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.5.2.1004.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.5.2.1004.msi))
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.5.2.1004-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.5.2.1004.dmg)

#### 2.5.0.1001

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.5.0.1001.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.5.0.1001.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.5.0.1001-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.5.0.1001.dmg)

#### 2.4.6.1015

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.4.6.1015.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.4.6.1015.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.4.6.1015-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.4.6.1015.dmg)

#### 2.4.5.1014

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.4.5.1014.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.4.5.1014.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.4.5.1014-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.4.5.1014.dmg)

#### 2.4.3.1012

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.4.3.1012.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.4.3.1012.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.4.3.1012-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.4.3.1012.dmg)

#### 2.4.1.1009

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.4.1.1009.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.4.1.1009.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.4.1.1009-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.4.1.1009.dmg)

#### 2.4.0.1002

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.4.0.1002.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.4.0.1002.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.4.0.1002-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.4.0.1002.dmg)

#### 2.3.5.1009

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.3.5.1009.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.3.5.1009.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.3.5.1009-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.3.5.1009.dmg)

#### 2.3.3.1005

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.3.3.1005.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.3.3.1005.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.3.3.1005-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.3.3.1005.dmg)

#### 2.3.2.1003

- [Windows 32-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery32_2.3.2.1003.msi)
- [Windows 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery64_2.3.2.1003.msi)
- [Linux 32-bit and 64-bit](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery_2.3.2.1003-Linux.tar.gz)
- [macOS](https://storage.googleapis.com/simba-bq-release/odbc/SimbaODBCDriverforGoogleBigQuery-2.3.2.1003.dmg)

## Install and configure the Simba JDBC driver for BigQuery

> [!NOTE]
> **Note:** As an alternative to the Simba JDBC driver, a [Google-developed JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery) is available in [Preview](https://cloud.google.com/products#product-launch-stages).

1. Download the
   [1.7.0.1001 version of the driver](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.7.0.1001.zip).

2. Follow the instructions in the
   [insightsoftware installation and configuration guide](https://storage.googleapis.com/simba-bq-release/jdbc/Simba%20Google%20BigQuery%20JDBC%20Connector%20Install%20and%20Configuration%20Guide_1.7.0.1001.pdf).

For information on feature changes and workflow updates, see
[Simba Google BigQuery JDBC Data Connector Release Notes](https://storage.googleapis.com/simba-bq-release/jdbc/release-notes_1.7.0.1001.txt).

To see a list of previous driver versions, expand the following section:

### Previous Simba JDBC driver versions

- [1.6.5.1002](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.6.5.1002.zip)
- [1.6.5.1001](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.6.5.1001.zip)
- [1.6.3.1004](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.6.3.1004.zip)
- [1.6.2.1003](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.6.2.1003.zip)
- [1.6.1.1002](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.6.1.1002.zip)
- [1.5.4.1008](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.5.4.1008.zip)
- [1.5.0.1001](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.5.0.1001.zip)
- [1.3.3.1004](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.3.3.1004.zip)
- [1.3.2.1003](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaBigQueryJDBC42-1.3.2.1003.zip)
- [1.3.0.1001](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.3.0.1001.zip)
- [1.2.25.1029](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.25.1029.zip)
- [1.2.23.1027](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.23.1027.zip)
- [1.2.22.1026](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.22.1026.zip)
- [1.2.21.1025](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.21.1025.zip)
- [1.2.19.1023](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.19.1023.zip)
- [1.2.18.1022](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.18.1022.zip)
- [1.2.16.1020](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.16.1020.zip)
- [1.2.14.1017](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.14.1017.zip)
- [1.2.1.1001 (JDBC 4.2-compatible)](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery42_1.2.1.1001.zip)
- [1.2.1.1001 (JDBC 4.1-compatible)](https://storage.googleapis.com/simba-bq-release/jdbc/SimbaJDBCDriverforGoogleBigQuery41_1.2.1.1001.zip)

## Support

Support for the Simba ODBC and JDBC drivers for BigQuery is
available through standard [Cloud Customer Care](https://docs.cloud.google.com/bigquery/support) channels.

## Pricing

You can download the Simba ODBC and JDBC drivers for BigQuery
at no cost, and you don't need any additional licenses to use the drivers.
However, when you use the driver, the following BigQuery pricing
applies:

- [Compute pricing](https://cloud.google.com/bigquery/pricing#compute-pricing-models) for the queries that you run.
- [Storage pricing](https://cloud.google.com/bigquery/pricing#storage-pricing), if your driver is configured to write large result sets to a destination table.
- [BigQuery Storage Read API pricing](https://cloud.google.com/bigquery/pricing#data-extraction-pricing) for data reads of large result sets, if your driver uses the High-Throughput API feature.

## What's next

- Learn more about the [Google-developed JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery).
- Explore other [BigQuery developer tools](https://docs.cloud.google.com/bigquery/docs/developer-overview).