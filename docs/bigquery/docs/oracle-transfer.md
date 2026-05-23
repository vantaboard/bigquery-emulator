# Load Oracle data into BigQuery

You can load data from Oracle to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Oracle connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Oracle to
BigQuery.

## Limitations

Oracle transfers are subject to the following limitations:

- The maximum number of simultaneous connections to an Oracle database is limited, and as a result, the number of simultaneous transfer runs to a single Oracle database is limited to that maximum amount.
- You must [set up a network
  attachment](https://docs.cloud.google.com/vpc/docs/create-manage-network-attachments) in cases where a public IP is not available for an Oracle database connection, with the following requirements:
  - The data source must be accessible from the subnet where the network attachment resides.
  - The network attachment must not be in the subnet within the range `240.0.0.0/24`.
  - Network attachments cannot be deleted if there are active connections to the attachment. To delete a network attachment, [contact Cloud Customer Care](https://docs.cloud.google.com/bigquery/docs/getting-support).
  - For the `us` multi-region, the network attachment must be in the `us-central1` region. For the `eu` multi-region, the network attachment must be in the `europe-west4` region.
- The minimum interval time between recurring Oracle transfers is 15 minutes. The default interval for a recurring transfer is 24 hours.
- A single transfer configuration can only support one data transfer run at a given time. In the case where a second data transfer is scheduled to run before the first transfer is completed, then only the first data transfer completes while any other data transfers that overlap with the first transfer is skipped.
  - To avoid skipped transfers within a single transfer configuration, we recommend that you increase the duration of time between large data transfers by configuring the **Repeat frequency**.
- During a data transfer, the Oracle connector identifies indexed and partitioned key columns to transfer your data in parallel batches. For this reason, we recommend that you specify primary key columns or use indexed columns in your table to improve the performance and reduce the error rate in your data transfers.
  - If you have indexed or primary key constraints, only the following column types are supported for creating parallel batches:
    - `INTEGER`
    - `TINYINT`
    - `SMALLINT`
    - `FLOAT`
    - `REAL`
    - `DOUBLE`
    - `NUMERIC`
    - `BIGINT`
    - `DECIMAL`
    - `DATE`
  - Oracle data transfers that don't use primary key or indexed columns can't support more than 2,000,000 records per table.
- If your configured network attachment and virtual machine (VM) instance are located in different regions, there might be cross-region data movement when you transfer data from Oracle.

### Incremental transfer limitations

Incremental Oracle transfers are subject to the following limitations:

<br />

- You can only choose `TIMESTAMP` columns as watermark columns.
- Incremental ingestion is only supported for assets with valid watermark columns.
- Values in a watermark column must be monotonically increasing.
- Incremental transfers cannot sync delete operations in the source table.
- A single transfer configuration can only support either incremental or full ingestion.
- You cannot update objects in the `asset` list after the first incremental ingestion run.
- You cannot change the write mode in a transfer configuration after the first incremental ingestion run.
- You cannot change the watermark column or the primary key after the first incremental ingestion run.
- The destination BigQuery table is clustered using the provided primary key and is subject to [clustered table limitations](https://docs.cloud.google.com/bigquery/docs/clustered-tables#limitations).
- When you update an existing transfer configuration to the incremental ingestion mode for the first time, the first data transfer after that update transfers all available data from your data source. Any subsequent incremental data transfers will transfer only the new and updated rows from your data source.

## Data ingestion options

The following sections provide more information about the data ingestion options
when you set up an Oracle data transfer.

### TLS configuration

The Oracle connector supports the configuration for transport level
security (TLS) to encrypt your data transfers into BigQuery. The
Oracle connector supports the following TLS configurations:

- The *Encrypt data, and verify CA and hostname* mode. This mode performs a full
  validation of the server using TLS over the TCPS protocol. It encrypts all
  data in transit and verifies that the database server's certificate is signed
  by a trusted certificate authority (CA). This mode also checks that the
  hostname you're connecting to exactly matches the Common Name (CN) or a
  Subject Alternative Name (SAN) on the server's certificate. This mode prevents
  attackers from using a valid certificate for a different domain to impersonate
  your database server.

  If your hostname does not match the certificate CN or SAN, the connection
  fails. You must configure a DNS resolution to match the certificate or use a
  different security mode. Use this mode for the most secure option to prevent
  person-in-the-middle (PITM) attacks.
- The *Encrypt data, and verify CA only* mode. This mode encrypts all data using
  TLS over the TCPS protocol and verifies that the server's certificate is
  signed by a CA that the client trusts. However, this mode does not verify the
  server's hostname. This mode successfully connects as long as the certificate
  is valid and issued by a trusted CA, regardless of whether the hostname in the
  certificate matches the hostname you are connecting to.

  Use this mode if you want to ensure that you are connecting to a server
  whose certificate is signed by a trusted CA, but the hostname is not
  verifiable or you don't have control over the hostname configuration.
- The *Encryption only* mode. This mode encrypts
  all data transferred between the client and the server using Oracle's Native
  Network Encryption over the standard TCP port. It does
  not perform any certificate or hostname validation.

  This mode provides some level of security by protecting data in transit, but
  it can be vulnerable to PITM attacks.

  Use this mode if you need to ensure all data is encrypted but can't or
  don't want to verify the server's identity. We recommend using this mode
  when working with private VPCs.
- The *No encryption or verification* mode. This mode does not encrypt any data
  and does not perform any certificate or hostname verification. All data is
  sent as plain text.

  We don't recommend using this mode in an environment where sensitive data is
  handled. We only recommend using this mode for testing purposes on an isolated
  network where security is not a concern.

#### Trusted Server Certificate (PEM)

If you are using either the *Encrypt data, and verify CA and hostname* mode or
the *Encrypt data, and verify CA* mode, then you can also provide one or more
PEM-encoded certificates. These certificates are required in some scenarios
where the BigQuery Data Transfer Service needs to verify the identity of your
database server during the TLS connection:

- If you are using a certificate signed by a private CA within your organization or a self-signed certificate, you must provide the full certificate chain or the single self-signed certificate. This is required for certificates issued by internal CAs of managed cloud provider services, such as the Amazon Relational Database Service (RDS).
- If your database server certificate is signed by a public CA (for example, Let's Encrypt, DigiCert, or GlobalSign), you don't need to provide a certificate. The root certificates for these public CAs are pre-installed and trusted by the BigQuery Data Transfer Service.

You can specify PEM-encoded certificates in the **Trusted PEM Certificate**
field in the transfer configuration, with the
following requirements:

- The certificate must be a valid PEM-encoded certificate chain.
- The certificate must be entirely correct. Any missing certificates in the chain or incorrect content causes the TLS connection to fail.
- For a single certificate, you can provide a single, self-signed certificate from the database server.
- For a full certificate chain issued by a private CA, you must provide the full chain of trust. This includes the certificate from the database server and any intermediate and root CA certificates.

### Full or incremental transfers

You can specify how data is loaded into BigQuery by selecting
either the **Full** or **Incremental** write
preference in the transfer configuration when you [set up an
Oracle transfer](https://docs.cloud.google.com/bigquery/docs/oracle-transfer#oracle-transfer-setup). Incremental transfers
are supported in [preview](https://cloud.google.com/products#product-launch-stages).

> [!NOTE]
> **Note:** To request feedback or support for incremental transfers, send email to [dts-preview-support@google.com](mailto:dts-preview-support@google.com).

You can configure a *full* data transfer to transfer all data from your Oracle datasets with each data transfer.

<br />

Alternatively, you can configure an *incremental* data transfer
([Preview](https://cloud.google.com/products#product-launch-stages)) to only
transfer data that was changed since the last data transfer, instead of loading
the entire dataset with each data transfer. If you select **Incremental** for
your data transfer, you must specify either the **Append** or **Upsert** write
modes to define how data is written to BigQuery during an
incremental data transfer. The following sections describe the available write
modes.

#### Append write mode

The append write mode only inserts new rows to your destination table. This option
strictly appends transferred data without checking for existing records, so
this mode can potentially cause data duplication in the destination table.

When you select the append mode, you must select a watermark column. A
watermark column is required for the Oracle connector to track changes
in the source table.
For Oracle transfers, we recommend selecting a column that is only updated when the record was created, and won't change with subsequent updates. For example, the `CREATED_AT` column.

<br />

#### Upsert write mode

The upsert write mode either updates a row or inserts a new row in your
destination table by checking for a primary key. You can specify a primary key
to let the Oracle connector determine what changes are needed to keep
your destination table up to date with your source table. If the specified
primary key is present in the destination BigQuery table during a data transfer, then the Oracle
connector updates that row with new data from the source table. If a primary key
is not present during a data transfer, then the Oracle connector
inserts a new row.

When you select the upsert mode, you must select a watermark column and a
primary key:

- A watermark column is required for the Oracle connector to track changes in the source table.
  - Select a watermark column that updates every time a row is modified. We recommend columns similar to the `UPDATED_AT` or `LAST_MODIFIED` column.

<!-- -->

- The primary key can be one or more columns on your table that are required
  for the Oracle connector to determine if it
  needs to insert or update a row.

  Select columns that contain non-null values that are unique across all
  rows of the table. We recommend columns that include system-generated
  identifiers, unique reference codes (for example, auto-incrementing IDs), or
  immutable time-based sequence IDs.

  To prevent potential data loss or data corruption, the primary key columns
  that you select must have unique values. If you have doubts about the
  uniqueness of your chosen primary key column, then we recommend that you
  use the append write mode instead.

### Incremental ingestion behavior

When you make changes to the table schema in your data source, incremental data transfers
from those tables are reflected in BigQuery in the following
ways:

| Changes to data source | Incremental ingestion behavior |
|---|---|
| Adding a new column | A new column is added to the destination BigQuery table. Any previous records for this column will have null values. |
| Deleting a column | The deleted column remains in the destination BigQuery table. New entries to this deleted column are populated with null values. |
| Changing the data type in a column | The connector only supports [data type conversions that are supported by the `ALTER COLUMN` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#details_21). Any other data type conversions causes the data transfer to fail. If you encounter any issues, we recommend creating a new transfer configuration. |
| Renaming a column | The original column remains in the destination BigQuery table as is, while a new column is added to the destination table with the updated name. |

## Before you begin

The following sections describe the steps that you need to take before you
create an Oracle transfer.

### Oracle prerequisites

- [Create a User credential](https://docs.oracle.com/cd/B13789_01/server.101/b10759/statements_8003.htm) in the Oracle database.
- [Grant `Create Session` system privileges to the user](https://docs.oracle.com/cd/B13789_01/server.101/b10759/statements_9013.htm) to allow session creation.
- [Assign a tablespace](https://docs.oracle.com/cd/B19306_01/network.102/b14266/admusers.htm#i1006219) to the user account.

You must also have the following Oracle database information when
creating an
Oracle transfer.

| Parameter Name | Description |
|---|---|
| `database` | Name of the database. |
| `host` | Hostname or IP address of the database. |
| `port` | Port number of the database. |
| `username` | Username to access the database. |
| `password` | Password to access the database. |

### BigQuery prerequisites

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store your data.
- If you intend to set up transfer run notifications for Pub/Sub, verify that you have the `pubsub.topics.setIamPolicy` Identity and Access Management (IAM) permission. Pub/Sub permissions are not required if you only set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

### Required BigQuery roles


To get the permissions that
you need to create a BigQuery Data Transfer Service data transfer,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role on your project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create a BigQuery Data Transfer Service data transfer. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create a BigQuery Data Transfer Service data transfer:

- BigQuery Data Transfer Service permissions:
  - `bigquery.transfers.update`
  - `bigquery.transfers.get`
- BigQuery permissions:
  - `bigquery.datasets.get`
  - `bigquery.datasets.getIamPolicy`
  - `bigquery.datasets.update`
  - `bigquery.datasets.setIamPolicy`
  - `bigquery.jobs.create`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information, see [Grant `bigquery.admin` access](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#grant_bigqueryadmin_access).

## Load Oracle data into BigQuery

Add Oracle data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , select **Oracle**.

4. In the **Data source details** section, do the following:

   - For **Network attachment** , select an existing network attachment or click **Create Network Attachment**.
   - For **Host**, enter the hostname or IP of the database.
   - For **Port** , enter the port number that the Oracle database is using for incoming connections, such as `1521`.
   - For **Database name**, enter the name of the Oracle database.
   - For **Connection type** , enter the connection URL type, either `SERVICE`, `SID`, or `TNS`.
   - For **Username**, enter the username of the user initiating the Oracle database connection.
   - For **Password**, enter the password of the user initiating the Oracle database connection.
   - For **TLS Mode** , select an option from the drop-down menu. For more information about TLS modes, see [TLS configuration](https://docs.cloud.google.com/bigquery/docs/oracle-transfer#tls_configuration).
   - For **Trusted PEM Certificate** , enter the public certificate of the certificate authority (CA) that issued the TLS certificate of the database server. For more information, see [Trusted Server Certificate (PEM)](https://docs.cloud.google.com/bigquery/docs/oracle-transfer#trusted_server_certificate_pem).
   - For **Ingestion type** , select **Full** or **Incremental** .
     - If you select **Incremental** ([Preview](https://cloud.google.com/products#product-launch-stages)), for **Write mode** , select either **Append** or **Upsert** . For more information about the different write modes, see [Full or
       incremental
       transfers](https://docs.cloud.google.com/bigquery/docs/oracle-transfer#full_or_incremental_transfers).
   - For **Oracle objects to transfer** , click **Browse** :
     - Select any objects to be transferred to the BigQuery destination dataset. You can also manually enter any objects to include in the data transfer in this field.
     - If you have selected **Append** as your incremental write mode, you must select a column as the watermark column.
     - If you have selected **Upsert** as your incremental write mode, you must select a column as the watermark column, and then select one or more columns as the primary key.
5. In the **Destination settings** section, for **Dataset**, select the
   dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a
   name for the data transfer.

7. In the **Schedule options** section:

   - In the **Repeat frequency** list, select an option to specify how often this data transfer runs. To specify a custom repeat frequency, select **Custom** . If you select **On-demand** , then this transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, click the **Email notification** toggle. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To enable [Pub/Sub transfer run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for this transfer, click the **Pub/Sub notifications** toggle. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name, or you can click **Create a topic** to create one.
9. Click **Save**.

### bq

Enter the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk)
and supply the transfer creation flag
`--transfer_config`:

```bash
bq mk
    --transfer_config
    --project_id=PROJECT_ID
    --data_source=DATA_SOURCE
    --display_name=DISPLAY_NAME
    --target_dataset=DATASET
    --params='PARAMETERS'
```

Where:

- <var translate="no">PROJECT_ID</var> (optional): your Google Cloud project ID. If `--project_id` isn't supplied to specify a particular project, the default project is used.
- <var translate="no">DATA_SOURCE</var>: the data source --- `oracle`.
- <var translate="no">DISPLAY_NAME</var>: the display name for the transfer configuration. The data transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">DATASET</var>: the target dataset for the transfer configuration.
- <var translate="no">PARAMETERS</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  an Oracle data transfer:

  - `connector.networkAttachment` (optional): name of the network attachment to connect to the Oracle database.
  - `connector.authentication.Username`: username of the Oracle account.
  - `connector.authentication.Password`: password of the Oracle account.
  - `connector.database`: name of the Oracle database.
  - `connector.endpoint.host`: the hostname or IP of the database.
  - `connector.endpoint.port`: the port number that the Oracle database is using for incoming connections, such as `1520`.
  - `connector.connectionType`: the connection URL type, either `SERVICE`, `SID`, or `TNS`.
  - `connector.tls.mode`: specify a [TLS configuration](https://docs.cloud.google.com/bigquery/docs/oracle-transfer#tls_configuration) to use with this transfer:
    - `ENCRYPT_VERIFY_CA_AND_HOST` to encrypt data, and verify CA and hostname
    - `ENCRYPT_VERIFY_CA` to encrypt data, and verify CA only
    - `ENCRYPT_VERIFY_NONE` for data encryption only
    - `DISABLE` for no encryption or verification
  - `connector.tls.trustedServerCertificate`: (optional) provide one or more [PEM-encoded certificates](https://docs.cloud.google.com/bigquery/docs/mysql-transfer#trusted_server_certificate_pem). Required only if `connector.tls.mode` is `ENCRYPT_VERIFY_CA_AND_HOST` or `ENCRYPT_VERIFY_CA`.
  - `ingestionType`: specify either `full` or `incremental`. Incremental transfers are supported in [preview](https://cloud.google.com/products#product-launch-stages). For more information, see [Full or incremental transfers](https://docs.cloud.google.com/bigquery/docs/oracle-transfer#full_or_incremental_transfers).
  - `writeMode`: specify either `WRITE_MODE_APPEND` or `WRITE_MODE_UPSERT`.
  - `watermarkColumns`: specify columns in your table as watermark columns. This field is required for incremental transfers.
  - `primaryKeys`: specify columns in your table as primary keys. This field is required for incremental transfers.
  - `assets`: the path to the Oracle objects to be transferred to BigQuery, using the format: `DATABASE_NAME/SCHEMA_NAME/TABLE_NAME`

When specifying multiple assets during an incremental transfer, the values
of the `watermarkColumns` and `primaryKeys` fields correspond to the
position of values in the `assets` field. In the following example,
`dep_id` corresponds to the table `DB1/USER1/DEPARTMENT`, while `report_by`
and `report_title` corresponds to the table `DB1/USER1/EMPLOYEES`.

<br />

```bash
      "primaryKeys":[['dep_id'], ['report_by','report_title']],
      "assets":["DB1/USER1/DEPARTMENT","DB1/USER1/EMPLOYEES"],
  
```

<br />

For example, the following command creates an Oracle data transfer
in the default project with all the required parameters:

```bash
bq mk
    --transfer_config
    --target_dataset=mydataset
    --data_source=oracle
    --display_name='My Transfer'
    --params='{"assets":["DB1/USER1/DEPARTMENT","DB1/USER1/EMPLOYEES"],
        "connector.authentication.username": "User1",
        "connector.authentication.password":"ABC12345",
        "connector.database":"DB1",
        "connector.endpoint.host":"192.168.0.1",
        "connector.endpoint.port":1520,
        "connector.connectionType":"SERVICE",
        "connector.tls.mode": "ENCRYPT_VERIFY_CA_AND_HOST",
        "connector.tls.trustedServerCertificate": "PEM-encoded certificate",
        "connector.networkAttachment":
        "projects/dev-project1/regions/us-central1/networkattachments/na1"
        "ingestionType":"incremental",
        "writeMode":"WRITE_MODE_APPEND",
        "watermarkColumns":["createdAt","createdAt"],
        "primaryKeys":[['dep_id'], ['report_by','report_title']]}'
```

### API

Use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.
When you save the transfer configuration, the Oracle connector automatically triggers a transfer run according to your schedule option. With every transfer run, the Oracle connector transfers all available data from Oracle into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

The following table maps Oracle data types to the
corresponding BigQuery data types.

| Oracle data type | BigQuery data type |
|---|---|
| `BFILE` | `BYTES` |
| `BINARY_DOUBLE` | `FLOAT` |
| `BINARY_FLOAT` | `FLOAT` |
| `BLOB` | `BYTES` |
| `CHAR` | `STRING` |
| `CLOB` | `STRING` |
| `DATE` | `DATETIME` |
| `FLOAT` | `FLOAT` |
| `INTERVAL DAY TO SECOND` | `STRING` |
| `INTERVAL YEAR TO MONTH` | `STRING` |
| `LONG` | `STRING` |
| `LONG RAW` | `BYTES` |
| `NCHAR` | `STRING` |
| `NCLOB` | `STRING` |
| `NUMBER (without precision and scale)` | `STRING` |
| `NUMBER (with precision and scale lower than the BigQuery Numeric range)` | `NUMERIC` |
| `NUMBER (with precision and scale lower than the BigQuery BigNumeric range)` | `BIGNUMERIC` |
| `NUMBER (with precision and scale greater than the BigQuery BigNumeric range)` | `STRING` |
| `NVARCHAR2` | `STRING` |
| `RAW` | `BYTES` |
| `ROWID` | `STRING` |
| `TIMESTAMP` | `DATETIME` |
| `TIMESTAMP WITH LOCAL TIME ZONE` | `DATETIME` |
| `TIMESTAMP WITH TIME ZONE` | `TIMESTAMP` |
| `UROWID` | `STRING` |
| `VARCHAR` | `STRING` |
| `VARCHAR2` | `STRING` |

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [Oracle transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#oracle-issues).

## Pricing

For pricing information about Oracle transfers, see
[Data Transfer Service pricing](https://docs.cloud.google.com/bigquery/pricing#bqdts).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Working with transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).