# Load Microsoft SQL Server data into BigQuery

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

Note: To get support or provide feedback for this feature, contact [dts-preview-support@google.com](mailto:dts-preview-support@google.com).

<br />

You can load data from Microsoft SQL Server to BigQuery using
the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Microsoft SQL Server connector. The
Microsoft SQL Server connector supports data loads from Microsoft SQL Server
instances hosted in on-premises environments and other cloud providers, such as
Cloud SQL, Amazon Web Services (AWS), or Microsoft Azure. With the BigQuery Data Transfer Service, you
can create on-demand and recurring data transfer jobs to transfer data from your
Microsoft SQL Server instance into BigQuery.

## Limitations

Microsoft SQL Server data transfer jobs are subject to the following limitations:

- There is a limited number of simultaneous connections to a Microsoft SQL Server database. Therefore, the number of simultaneous transfer runs to a single Microsoft SQL Server database is also limited. Ensure that the number of concurrent transfer jobs is less than the maximum number of concurrent connections supported by the Microsoft SQL Server database.
- Some Microsoft SQL Server data types might be mapped to the `STRING` type in BigQuery to avoid data loss. For example, certain numeric types in Microsoft SQL Server that don't have precision and scale defined might be mapped to `STRING` in BigQuery. For more information, see [Data type mapping](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#data_type_mapping).

### Incremental transfer limitations

Incremental Microsoft SQL Server transfers are subject to the following limitations:

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
- We recommend that you create indexes on the watermark column. This connector uses watermark columns for filters in incremental transfers, so indexing these columns can improve performance.
- When making an incremental transfer, you must use the updated data type mapping.

## Data ingestion options

The following section provides information about the data ingestion options
when you set up a Microsoft SQL Server data transfer.

### TLS configuration

The Microsoft SQL Server connector supports the configuration for transport level
security (TLS) to encrypt your data transfers into BigQuery. The
Microsoft SQL Server connector supports the following TLS configurations:

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
  all data transferred between the client and the server. It does
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
either the *Full* or *Incremental* write
preference in the transfer configuration when you [set up a
Microsoft SQL Server transfer](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#set-up). Incremental transfers
are supported in [preview](https://cloud.google.com/products#product-launch-stages).

> [!NOTE]
> **Note:** To request feedback or support for incremental transfers, send email to [dts-preview-support@google.com](mailto:dts-preview-support@google.com).

You can configure a *full* data transfer to transfer all data from your Microsoft SQL Server datasets with each data transfer.

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
watermark column is required for the Microsoft SQL Server connector to track changes
in the source table.
For Microsoft SQL Server transfers, we recommend selecting a column that is only updated when the record was created, and won't change with subsequent updates. For example, the `CREATED_AT` column.

<br />

#### Upsert write mode

The upsert write mode either updates a row or inserts a new row in your
destination table by checking for a primary key. You can specify a primary key
to let the Microsoft SQL Server connector determine what changes are needed to keep
your destination table up to date with your source table. If the specified
primary key is present in the destination BigQuery table during a data transfer, then the Microsoft SQL Server
connector updates that row with new data from the source table. If a primary key
is not present during a data transfer, then the Microsoft SQL Server connector
inserts a new row.

When you select the upsert mode, you must select a watermark column and a
primary key:

- A watermark column is required for the Microsoft SQL Server connector to track changes in the source table.
  - Select a watermark column that updates every time a row is modified. We recommend columns similar to the `UPDATED_AT` or `LAST_MODIFIED` column.

<!-- -->

- The primary key can be one or more columns on your table that are required
  for the Microsoft SQL Server connector to determine if it
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

Before you can schedule a Microsoft SQL Server data transfer,
you must meet the following prerequisites.

### Microsoft SQL Server prerequisites

You must have created a user account in the Microsoft SQL Server database. For
more information, see [Create a user with a
login](https://learn.microsoft.com/en-us/sql/relational-databases/security/authentication-access/create-a-database-user?view=sql-server-ver17#create-a-user-with-a-login).

### BigQuery prerequisites

- Verify that you have completed all the actions that are required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store your data.

#### Required roles


To get the permissions that
you need to create a Microsoft SQL Server data transfer,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role on your project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create a Microsoft SQL Server data transfer. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create a Microsoft SQL Server data transfer:

- `bigquery.transfers.update`
- `bigquery.datasets.get`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Network configuration

You must set up specific network configurations when a public IP address isn't
available for the Microsoft SQL Server database connection. For more
information, see the following sections:

- [Configure a connection to Google Cloud instance](https://docs.cloud.google.com/bigquery/docs/cloud-sql-instance-access)
- [Configure a connection to AWS](https://docs.cloud.google.com/bigquery/docs/aws-vpn-network-attachment)
- [Configure a connection to Azure](https://docs.cloud.google.com/bigquery/docs/azure-vpn-network-attachment)

## Set up a Microsoft SQL Server data transfer

Select one of the following options:

### Console

1. Go to the **Data transfers** page.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , select **Microsoft SQL Server**.

4. In the **Data source details** section, do the following:

   - For **Network attachment** , select an existing network attachment or click **Create Network Attachment**.
   - For **Host**, enter the hostname or IP address of the Microsoft SQL Server database.
   - For **Port number**, enter the port number for the Microsoft SQL Server database.
   - For **Database name**, enter the name of the Microsoft SQL Server database.
   - For **Username**, enter the username of the Microsoft SQL Server user initiating the Microsoft SQL Server database connection.
   - For **Password**, enter the password of the Microsoft SQL Server user initiating the Microsoft SQL Server database connection.
   - For **TLS Mode** , select an option from the menu. For more information about TLS modes, see [TLS configuration](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#tls_configuration).
   - For **Trusted PEM Certificate** , enter the public certificate of the certificate authority (CA) that issued the TLS certificate of the database server. For more information, see [Trusted Server Certificate (PEM)](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#trusted_server_certificate_pem).
   - For **Enable legacy mapping** , select **true** (default) to use the [legacy
     data type mapping](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#data_type_mapping). Select **false** to use the updated data type mapping. If you are making an incremental transfer, this value must be **false** . For more information about the data type mapping updates, see [March 16, 2027](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-sqlserver). database server.
   - For **Ingestion type** , select **Full** or **Incremental** .
     - If you select **Incremental** ([Preview](https://cloud.google.com/products#product-launch-stages)), for **Write mode** , select either **Append** or **Upsert** . For more information about the different write modes, see [Full or
       incremental
       transfers](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#full_or_incremental_transfers).
   - For **Microsoft SQL Server objects to transfer**, browse the Microsoft SQL Server table or manually enter the names of the tables that are required for the transfer.
5. In the **Destination settings** section, for **Dataset** , select the
   dataset that you created to store your data, or click **Create new dataset**
   and create one to use as the destination dataset.

6. In the **Transfer config name** section, for **Display name**,
   enter a name for the transfer. The transfer name can be any value that
   lets you identify the transfer if you need to modify it later.

7. In the **Schedule options** section, do the following:

   - Select a repeat frequency. If you select the **Hours** , **Days** (default), **Weeks** , or **Months** option, you must also specify a frequency. You can also select the **Custom** option to create a more specific repeat frequency. If you select the **On-demand** option, this data transfer only runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either the **Start now** or **Start at a set time** option and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, click the **Email notifications** toggle to the on position. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To configure Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer, click the **Pub/Sub notifications** toggle to the on position. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/admin) name or click **Create a topic** to create one.
9. Optional: In the **Advanced options** section, select an encryption type
   for this transfer. You can select either a Google-owned and Google-managed encryption key
   or a customer-owned Cloud Key Management Service key. For more information about encryption
   keys, see [Customer-managed encryption keys (CMEK)](https://docs.cloud.google.com/kms/docs/cmek).

10. Click **Save**.

### bq

Enter the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk)
and supply the transfer creation flag `--transfer_config`:

```bash
bq mk \
    --transfer_config \
    --project_id=PROJECT_ID \
    --data_source=DATA_SOURCE \
    --display_name=DISPLAY_NAME \
    --target_dataset=DATASET \
    --params='PARAMETERS'
```

Replace the following:

- `PROJECT_ID` (optional): your Google Cloud project ID. If the `--project_id` flag isn't supplied to specify a particular project, the default project is used.
- `DATA_SOURCE`: the data source, which is `sqlserver`.
- `DISPLAY_NAME`: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- `DATASET`: the target dataset for the data transfer configuration.
- `PARAMETERS`: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a Microsoft SQL Server transfer:

  - `connector.networkAttachment` (optional): the name of the network attachment to connect to the Microsoft SQL Server database.
  - `connector.database`: the name of the Microsoft SQL Server database.
  - `connector.endpoint.host`: the hostname or IP address of the database.
  - `connector.endpoint.port`: the port number of the database.
  - `connector.authentication.username`: the username of the database user.
  - `connector.authentication.password`: the password of the database user.
  - `connector.legacyMapping`: set to `true` (default) to use the [legacy
    data type mapping](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#data_type_mapping). Set to `false` to use the updated data type mapping. If you are making an incremental transfer, this value must be `false`. For more information about the data type mapping updates, see [March 16, 2027](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-sqlserver).
  - `connector.tls.mode`: specify a [TLS configuration](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#tls_configuration) to use with this transfer:
    - `ENCRYPT_VERIFY_CA_AND_HOST` to encrypt data, and verify CA and hostname
    - `ENCRYPT_VERIFY_CA` to encrypt data, and verify CA only
    - `ENCRYPT_VERIFY_NONE` for data encryption only
    - `DISABLE` for no encryption or verification
  - `connector.tls.trustedServerCertificate`: (optional) provide one or more [PEM-encoded certificates](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#trusted_server_certificate_pem). Required only if the value of `connector.tls.mode` is `ENCRYPT_VERIFY_CA_AND_HOST` or `ENCRYPT_VERIFY_CA`.
  - `ingestionType`: specify either `full` or `incremental`. Incremental transfers are supported in [preview](https://cloud.google.com/products#product-launch-stages). For more information, see [Full or incremental transfers](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#full_or_incremental_transfers).
  - `writeMode`: specify either `WRITE_MODE_APPEND` or `WRITE_MODE_UPSERT`.
  - `watermarkColumns`: specify columns in your table as watermark columns. This field is required for incremental transfers.
  - `primaryKeys`: specify columns in your table as primary keys. This field is required for incremental transfers.
  - `assets`: a list of the names of the Microsoft SQL Server tables to be transferred from the Microsoft SQL Server database as part of the transfer.

For example, the following command creates a Microsoft SQL Server
transfer called `My Transfer`:

```bash
bq mk \
    --transfer_config
    --target_dataset=mydataset
    --data_source=sqlserver
    --display_name='My Transfer'
    --params='{"assets":["DB1/DEPARTMENT","DB1/EMPLOYEES"],
        "connector.authentication.username": "User1",
        "connector.authentication.password":"ABC12345",
        "connector.database":"DB1",
        "connector.endpoint.host":"192.168.0.1",
        "connector.endpoint.port":"1520",
        "connector.networkAttachment":"projects/dev-project1/regions/us-central1/networkattachments/na1",
        "ingestionType":"incremental",
        "writeMode":"WRITE_MODE_APPEND",
        "watermarkColumns":["createdAt","createdAt"],
        "primaryKeys":[['dep_id'], ['report_by','report_title']],
        "connector.tls.mode": "ENCRYPT_VERIFY_CA_AND_HOST",
        "connector.tls.trustedServerCertificate": "PEM-encoded certificate"}'
```

When specifying multiple assets during an incremental transfer, the values
of the `watermarkColumns` and `primaryKeys` fields correspond to the
position of values in the `assets` field. In the following example,
`dep_id` corresponds to the table `DB1/DEPARTMENT`, while `report_by`
and `report_title` corresponds to the table `DB1/EMPLOYEES`.

<br />

```bash
      "primaryKeys":[['dep_id'], ['report_by','report_title']],
      "assets":["DB1/DEPARTMENT","DB1/EMPLOYEES"],
  
```

<br />

When you save the transfer configuration, the Microsoft SQL Server connector automatically triggers a transfer run according to your schedule option. With every transfer run, the Microsoft SQL Server connector transfers all available data from Microsoft SQL Server into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

> [!NOTE]
> **Note:** On March 16, 2027, the Microsoft SQL Server connector will update some of its data type mapping, as indicated in the following table. For more information, see [March 16, 2027](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-sqlserver).

The following table maps Microsoft SQL Server data types to the
corresponding BigQuery data types:

| Microsoft SQL Server data type | BigQuery data type | [Updated BigQuery data type](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-sqlserver) |
|---|---|---|
| `tinyint` | `INTEGER` |   |
| `smallint` | `INTEGER` |   |
| `int` | `INTEGER` |   |
| `bigint` | `BIGNUMERIC` |   |
| `bit` | `BOOLEAN` |   |
| `decimal` | `BIGNUMERIC` |   |
| `numeric` | `NUMERIC` |   |
| `money` | `BIGNUMERIC` |   |
| `smallmoney` | `BIGNUMERIC` |   |
| `float` | `FLOAT` |   |
| `real` | `FLOAT` |   |
| `date` | `DATE` |   |
| `time` | `TIME` |   |
| `datetime2` | `TIMESTAMP` | `DATETIME` |
| `datetimeoffset` | `TIMESTAMP` |   |
| `datetime` | `TIMESTAMP` | `DATETIME` |
| `smalldatetime` | `TIMESTAMP` | `DATETIME` |
| `char` | `STRING` |   |
| `varchar` | `STRING` |   |
| `text` | `STRING` |   |
| `nchar` | `STRING` |   |
| `nvarchar` | `STRING` |   |
| `ntext` | `STRING` |   |
| `binary` | `BYTES` |   |
| `varbinary` | `BYTES` |   |
| `image` | `BYTES` |   |
| `geography` | `STRING` |   |
| `geometry` | `STRING` |   |
| `hierarchyid` | `BYTES` |   |
| `rowversion` | `BYTES` |   |
| `sql_variant` | `BYTES` |   |
| `uniqueidentifier` | `STRING` |   |
| `xml` | `STRING` |   |
| `json` | `STRING` |   |
| `vector` | `STRING` |   |

The `json` and `vector` data types are only supported in Azure.

The [JSON data
type](https://learn.microsoft.com/en-us/sql/t-sql/data-types/json-data-type) is
supported in Azure SQL databases and Azure SQL
managed instances configured with the always-up-to-date update policy. The JSON
data type is not supported in Azure SQL managed instances
configured with the Microsoft SQL Server 2022 update policy.

Microsoft SQL Server stores JSON as `NVARCHAR(MAX)`, and not as a JSON
type. We recommend that you use `CHECK (ISJSON(json_col) = 1)` for validation,
and `JSON_VALUE()` for querying.

The Microsoft SQL Server doesn't have vector support for the
`vector` data type. We recommend that you store vectors as JSON arrays in
`NVARCHAR(MAX)` and use `JSON_VALUE()` for extraction, with manual `FLOAT`
calculations for similarity.

## Troubleshoot

To troubleshoot issues for your data transfer, see
[Microsoft SQL Server transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#sqlserver-issues).

## Pricing

There is no cost to transfer Microsoft SQL Server data into
BigQuery while this feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).

## What's next

- Read [an overview about the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- Learn about [managing transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers), including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history.
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).