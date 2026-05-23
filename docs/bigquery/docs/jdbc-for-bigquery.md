# Use the JDBC driver for BigQuery

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

> [!NOTE]
> **Note:** To request feedback or support for this feature, send an email to [bigquery-drivers-feedback@google.com](mailto:bigquery-drivers-feedback@google.com).

The Java Database Connectivity (JDBC) driver for BigQuery connects your
Java applications to BigQuery, letting you use
BigQuery features with your preferred tooling and infrastructure.
To connect non-Java applications to BigQuery, use the
[Simba Open Database Connectivity (ODBC) driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers).

## Limitations

The JDBC driver for BigQuery is subject to the following
limitations:

- The driver is specific to BigQuery and can't be used with other products or services.
- The `INTERVAL` data type isn't supported with the BigQuery Storage Read API.
- All [data manipulation language (DML) limitations](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations) apply.

## Before you begin

1. Make sure that you're familiar with JDBC drivers, Apache Maven, and the [`java.sql` package](https://docs.oracle.com/javase/8/docs/api/java/sql/package-summary.html).
2. Verify that your system is configured with Java Runtime Environment (JRE) 8.0 or later. For information on checking your JRE version, see [Verifying the JRE Environment](https://docs.oracle.com/goldengate/dir1212/gg-director/GDRAD/verifying-jre-environment.htm).
3. [Authenticate to BigQuery](https://docs.cloud.google.com/bigquery/docs/authentication),
   and take note of the following information, which is used later when you
   establish a connection with the JDBC driver for BigQuery. You
   only need to note the information that corresponds to the authentication
   method that you use.

   | **Authentication method** | **Authentication information** | **Example** | **Connection property (to set later)** |
   |---|---|---|---|
   | Standard service account | Service account email | `bq-jdbc-sa@mytestproject.iam.gserviceaccount.com` | `OAuthServiceAcctEmail` |
   | Standard service account | Service account key (JSON object) | `my-sa-key` | `OAuthPvtKey` |
   | Service account key file | Service account key file (full path) | `path/to/file/secret.json` | `OAuthPvtKeyPath` |
   | Google user account | Client ID | `123-abc.apps.googleusercontent.com` | `OAuthClientId` |
   | Google user account | Client secret | `_aB-C1D_E2fGh3Ij4kL5m6No7p8QR9sT0uV` | `OAuthClientSecret` |
   | Pre-generated access token | Access token | `ya29.a0AfH6SMCiH1L-x_yZ` | `OAuthAccessToken` |
   | Pre-generated refresh token | Refresh token | `1/fFAGRNJru1FTz70BzhT3Zg` | `OAuthRefreshToken` |
   | Pre-generated refresh token | Client ID | `123-abc.apps.googleusercontent.com` | `OAuthClientId` |
   | Pre-generated refresh token | Client secret | `_aB-C1D_E2fGh3Ij4kL5m6No7p8QR9sT0uV` | `OAuthClientSecret` |
   | Application Default Credentials | None | N/A | N/A |
   | Configuration file | Configuration file (JSON object or full path) | `path/to/file/secret.json` | `OAuthPvtKey` |
   | External account configuration object | Account configuration object | `external_account_configuration_object` | `OAuthPvtKey` |
   | Other | Audience property of the external account configuration file | `//iam.googleapis.com/projects/my-project/locations/US-EAST1/workloadIdentityPools/my-pool-/providers/my-provider` | `BYOID_AudienceUri` |
   | Other | Token retrieval and environmental information file | `{\"file\":\"/path/to/file\"}` | `BYOID_CredentialSource` |
   | Other | User project (only if using a workforce pool) | `my_project` | `BYOID_PoolUserProject` |
   | Other | URI for service account impersonation (only if using a workforce pool) | `my-sa` | `BYOID_SA_Impersonation_Uri` |
   | Other | Security Token Service token based on the token exchange specification | `urn:ietf:params:oauth:tokentype:id_token` | `BYOID_SubjectTokenType` |
   | Other | Security Token Service token exchange endpoint | `https://sts.googleapis.com/v1/token` | `BYOID_TokenUri` |

## Configure your development environment

The JDBC driver for BigQuery is available on
[Maven Central](https://mvnrepository.com/artifact/com.google.cloud/google-cloud-bigquery-jdbc).

To configure your development environment with the JDBC driver, add the driver as a dependency to your project:

### Maven

Add the following dependency to your
`pom.xml` file:

```java
<dependency>
    <groupId>com.google.cloud</groupId>
    <artifactId>google-cloud-bigquery-jdbc</artifactId>
    <version>0.3.0</version>
</dependency>
```

### Gradle

Add the following to your `build.gradle` file:

```java
dependencies {
// ... other dependencies
implementation("com.google.cloud:google-cloud-bigquery-jdbc:0.3.0")
}
```

### Other (Shaded Uber JAR)

If you need a standalone JAR file, for example to include in a
non-Maven or non-Gradle project, download the
[shaded Uber JAR](https://storage.googleapis.com/bq-driver-releases/jdbc/google-cloud-bigquery-jdbc-latest-all.jar).
A shaded JAR file with all dependencies included is also available from
Maven with the classifier `all`.

## Establish a connection

To establish a connection between your Java application and
BigQuery with the JDBC driver for BigQuery, do
the following:

1. Identify your connection string for the JDBC driver for
   BigQuery. This string captures all the required information
   to establish a connection between your Java application and
   BigQuery. The connection string has the following format:

   ```java
   jdbc:bigquery://HOST:PORT;ProjectId=PROJECT_ID;OAuthType=AUTH_TYPE;AUTH_PROPS;OTHER_PROPS
   ```

   Replace the following:
   - <var translate="no">`HOST`</var>: the DNS or IP address of the server.
   - <var translate="no">`PORT`</var>: the TCP port number.
   - <var translate="no">`PROJECT_ID`</var>: the ID of your BigQuery project.
   - <var translate="no">`AUTH_TYPE`</var>: a number specifying the type of authentication that you used. One of the following:
     - `0`: for service account authentication (standard and key file)
     - `1`: for Google user account authentication
     - `2`: for pre-generated refresh or access token authentication
     - `3`: for Application Default Credential authentication
     - `4`: for other authentication methods
   - <var translate="no">`AUTH_PROPS`</var>: the authentication information that you noted when you [authenticated to BigQuery](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery#before_you_begin), listed in the `property_1=value_1; property_2=value_2;...` format---for example, `OAuthPvtKeyPath=path/to/file/secret.json`, if you authenticated with a service account key file.
   - <var translate="no">`OTHER_PROPS`</var> (optional): additional connection properties for the JDBC driver, listed in the `property_1=value_1; property_2=value_2;...` format. For a full list of connection properties, see [Connection properties](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery#connection_properties).
2. Connect your Java application to the JDBC driver for
   BigQuery with either the
   [`DriverManager`](https://docs.oracle.com/javase/8/docs/api/java/sql/DriverManager.html)
   or
   [`DataSource`](https://docs.oracle.com/javase/8/docs/api/javax/sql/DataSource.html)
   class.

   - Connect with the `DriverManager` class:

     ```java
     import java.sql.Connection;
     import java.sql.DriverManager;

     private static Connection getJdbcConnectionDM(){
       Connection connection = DriverManager.getConnection(CONNECTION_STRING);
       return connection;
     }
     ```

     Replace <var translate="no">`CONNECTION_STRING`</var> with the connection
     string from the previous step.
   - Connect with the `DataSource` class:

     ```java
     import com.google.cloud.bigquery.jdbc.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.jdbc.DataSource.html;
     import java.sql.Connection;
     import java.sql.SQLException;

     private static public Connection getJdbcConnectionDS() throws SQLException {
       Connection connection = null;
       DataSource dataSource = new com.google.cloud.bigquery.jdbc.DataSource();
       dataSource.setURL(CONNECTION_STRING);
       connection = dataSource.getConnection();
       return connection;
     }
     ```

     Replace <var translate="no">`CONNECTION_STRING`</var> with the connection
     string from the previous step.

     The `DataSource` class also has setter methods that you can use to set
     [connection properties](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery#connection_properties), rather than including
     them in the connection string. The following is an example:

     ```java
     private static Connection getConnection() throws SQLException {
       DataSource ds = new DataSource();
       ds.setURL(jdbc:bigquery://https://www.googleapis.com/bigquery/v2:443;);
       ds.setAuthType(3);  // Application Default Credentials
       ds.setProjectId("MyTestProject");
       ds.setEnableHighThroughputAPI(true);
       ds.setLogLevel("6");
       ds.setUseQueryCache(false);
       return ds.getConnection();
     }
     ```

### Connection properties

JDBC driver connection properties are configuration parameters that you include
in the connection string or pass through setter methods when you
[establish a connection](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery#establish_a_connection) to a database. The following
connection properties are supported by the JDBC driver for
BigQuery.

> [!NOTE]
> **Note:** All connection property names are case-insensitive. Boolean connection properties accept both `TRUE`/`FALSE` and `1`/`0`.

| **Connection property** | **Description** | **Default value** | **Data type** | **Required** |
|---|---|---|---|---|
| `AdditionalProjects` | Projects that the driver can access for queries and metadata operations, in addition to the primary project set by the `ProjectId` property. | N/A | Comma-separated string | No |
| `AllowLargeResults` | Determines if the driver processes query results that are larger than 128 MB when the `QueryDialect` property is set to `BIG_QUERY`. If the `QueryDialect` property is set to `SQL`, the driver always processes large query results. | `TRUE` | Boolean | No |
| `BYOID_AudienceUri` | The audience property in an external account configuration file. The audience property can contain the resource name for the workload identity pool or workforce pool, as well as the provider identifier in that pool. | N/A | String | Only when `OAuthType=4` |
| `BYOID_CredentialSource` | The token retrieval and environmental information. | N/A | String | Only when `OAuthType=4` |
| `BYOID_PoolUserProject` | The user project when a workforce pool is being used for authentication. | N/A | String | Only when `OAuthType=4` and using the workforce pool |
| `BYOID_SA_Impersonation_Uri` | The URI for the service account impersonation when a workforce pool is being used for authentication. | N/A | String | Only when `OAuthType=4` and using the workforce pool |
| `BYOID_SubjectTokenType` | The Security Token Service token based on the token exchange specification. One of the following: - `urn:ietf:params:oauth:token-type:jwt` - `urn:ietf:params:oauth:token-type:id_token` - `urn:ietf:params:oauth:token-type:saml2` - `urn:ietf:params:aws:token-type:aws4_request` | `urn:ietf:params:oauth:tokentype:id_token` | String | Only when `OAuthType=4` |
| `BYOID_TokenUri` | The Security Token Service token exchange endpoint. | `https://sts.googleapis.com/v1/token` | String | No |
| `ConnectionPoolSize` | The connection pool size, if connection pooling is enabled. | `10` | Long | No |
| `DefaultDataset` | The dataset that's used when one isn't specified in a query. | N/A | String | No |
| `EnableHighThroughputAPI` | Determines if the Storage Read API can be used. The `HighThroughputActivationRatio` and `HighThroughputMinTableSize` properties must also be set to `TRUE` to use the Storage Read API. | `FALSE` | Boolean | No |
| `EnableSession` | Determines if the connection starts a session. If set to `TRUE`, the session ID is passed to all subsequent queries. | `FALSE` | Boolean | No |
| `EnableWriteAPI` | Determines if the Storage Write API can be used. It must be set to `TRUE` to enable bulk inserts. | `FALSE` | Boolean | No |
| `EndpointOverrides` | Custom endpoints to overwrite the following: - `BIGQUERY=https://bigquery.googleapis.com` - `READ_API=https://bigquerystorage.googleapis.com` - `OAUTH2=https://oauth2.googleapis.com` - `STS=https://sts.googleapis.com` | N/A | Comma-separated string | No |
| `FilterTablesOnDefaultDataset` | Determines the scope of metadata returned by the `DatabaseMetaData.getTables()` and `DatabaseMetaData.getColumns()` methods. When set to `FALSE`, no filtering occurs. The `DefaultDataset` property must also be set to enable filtering. | `FALSE` | Boolean | No |
| `HighThroughputActivationRatio` | The threshold for the number of pages in a query response. When this number is exceeded, and the `EnableHighThroughputAPI` and `HighThroughputMinTableSize` conditions are met, the driver starts using the Storage Read API. | `2` | Integer | No |
| `HighThroughputMinTableSize` | The threshold for the number of rows in a query response. When this number is exceeded, and the `EnableHighThroughputAPI` and `HighThroughputActivationRatio` conditions are met, the driver starts using the Storage Read API. | `10000` | Integer | No |
| `JobCreationMode` | Determines if queries are run with or without jobs. A `1` value means that jobs are created for every query, and a `2` value means that queries can be executed without jobs. | `2` | Integer | No |
| `JobTimeout` | The job timeout (in seconds) after which the job is cancelled on the server. | `0` | Long | No |
| `KMSKeyName` | The KMS key name for encrypting data. | N/A | String | No |
| `Labels` | Labels that are associated with the query to organize and group query jobs. | N/A | Map\<String, String\> | No |
| `LargeResultDataset` | The destination dataset for large query results, only when the `LargeResultTable` property is set. When you set this property, data writes bypass the result cache and trigger billing for each query, even if the results are small. | `_google_jdbc` | String | No |
| `LargeResultsDatasetExpirationTime` | The lifetime of all tables in a large result dataset, in milliseconds. This property is ignored if the dataset already has a default expiration time set. | `3600000` | Long | No |
| `LargeResultTable` | The destination table for large query results, only when the `LargeResultDataset` property is set. When you set this property, data writes bypass the result cache and trigger billing for each query, even if the results are small. | `temp_table...` | String | No |
| `ListenerPoolSize` | The listener pool size, if connection pooling is enabled. | `10` | Long | No |
| `Location` | The [location](https://docs.cloud.google.com/bigquery/docs/locations) where datasets are created or queried. BigQuery automatically determines the location if this property isn't set. | N/A | String | No |
| `LogLevel` | The level of detail logged by the driver. For more information, see [Logging](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery#logging). | `0` | Integer | No |
| `LogPath` | The directory where log files are written. | N/A | String | No |
| `MaximumBytesBilled` | The limit of bytes billed. Queries with bytes billed greater than this number fail without incurring a charge. | `0` | Long | No |
| `MaxResults` | The maximum number of results per page. | `10000` | Long | No |
| `MetaDataFetchThreadCount` | The number of threads used for database metadata methods. | `32` | Integer | No |
| `OAuthAccessToken` | The access token that's used for pre-generated access token authentication. | N/A | String | Only when `OAUTH_TYPE=2` |
| `OAuthClientId` | The client ID for pre-generated refresh token authentication and user account authentication. | N/A | String | Only when `OAUTH_TYPE=1` or `OAUTH_TYPE=2` |
| `OAuthClientSecret` | The client secret for pre-generated refresh token authentication and user account authentication. | N/A | String | Only when `OAUTH_TYPE=1` or `OAUTH_TYPE=2` |
| `OAuthP12Password` | The password for the PKCS12 key file. | `notasecret` | String | No |
| `OAuthPvtKey` | The service account key when using service account authentication. This value can be a raw JSON keyfile object or a path to the JSON keyfile. | N/A | String | Only when `OAUTH_TYPE=0` and the `OAuthPvtKeyPath` value isn't set |
| `OAuthPvtKeyPath` | The path to the service account key when using service account authentication. | N/A | String | Only when `OAUTH_TYPE=0` and the `OAuthPvtKey` and `OAuthServiceAcctEmail` values aren't set |
| `OAuthRefreshToken` | The refresh token for pre-generated refresh token authentication. | N/A | String | Only when `OAUTH_TYPE=2` |
| `OAuthServiceAcctEmail` | The service account email when using service account authentication. | N/A | String | Only when `OAUTH_TYPE=0` and the `OAuthPvtKeyPath` value isn't set |
| `OAuthType` | The authentication type. One of the following: - `0`: service account authentication - `1`: user account authentication - `2`: pre-generated refresh or access token authentication - `3`: Application Default Credential authentication - `4`: other authentication methods | `-1` | Integer | Yes |
| `PartnerToken` | A token that's used by Google Cloud partners to track usage of the driver. | N/A | String | No |
| `ProjectId` | The default project ID for the driver. This project is used to execute queries and is billed for resource usage. If not set, the driver infers a project ID. | N/A | String | No, but highly recommended |
| `ProxyHost` | The hostname or IP address of a proxy server through which the JDBC connection is routed. | N/A | String | No |
| `ProxyPort` | The port number on which the proxy server is listening for connections. | N/A | String | No |
| `ProxyPwd` | The password for authentication when connecting through a proxy server that requires it. | N/A | String | No |
| `ProxyUid` | The username for authentication when connecting through a proxy server that requires it. | N/A | String | No |
| `QueryDialect` | The SQL dialect for query execution. Use `SQL` for GoogleSQL (highly recommended) and `BIG_QUERY` for legacy SQL. | `SQL` | String | No |
| `QueryProperties` | [REST connection properties](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/ConnectionProperty) that customize query behavior. | N/A | Map\<String, String\> | No |
| `RequestGoogleDriveScope` | Adds read-only Drive scope to the connection when set to `1`. | `0` | Integer | No |
| `RetryInitialDelay` | Sets the delay (in seconds) before the first retry. | `0` | Long | No |
| `RetryMaxDelay` | Sets the maximum limit (in seconds) for the retry delay. | `0` | Long | No |
| `ServiceAccountImpersonationChain` | A comma-separated list of service account emails in the impersonation chain. | N/A | String | No |
| `ServiceAccountImpersonationEmail` | The service account email to be impersonated. | N/A | String | No |
| `ServiceAccountImpersonationScopes` | A comma-separated list of OAuth2 scopes to use with the impersonated account. | `https://www.googleapis.com/auth/bigquery` | String | No |
| `ServiceAccountImpersonationTokenLifetime` | The impersonated account token lifetime (in seconds). | `3600` | Integer | No |
| `SSLTrustStore` | The full path to the Java TrustStore that contains trusted Certificate Authority (CA) certificates. The driver utilizes this truststore to validate the identity of the server during the SSL/TLS handshake. | N/A | String | No |
| `SSLTrustStorePwd` | The password to the Java TrustStore specified in the `SSLTrustStore` property. | N/A | String | Only if the Java TrustStore is password-protected |
| `SWA_ActivationRowCount` | The threshold of `executeBatch insert` rows which, when exceeded, causes the connector to switch to the Storage Write API. | `3` | Integer | No |
| `SWA_AppendRowCount` | The size of the write stream. | `1000` | Integer | No |
| `Timeout` | The length of time, in seconds, that the connector retries a failed API call before timing out. | `0` | Long | No |
| `UniverseDomain` | The top-level domain that's associated with your organization's Google Cloud resources. | `googleapis.com` | String | No |
| `UnsupportedHTAPIFallback` | Determines if the connector falls back to the REST API (when set to `TRUE`) or returns an error (when set to `FALSE`). | `TRUE` | Boolean | No |
| `UseQueryCache` | Enables query caching. | `TRUE` | Boolean | No |

## Run queries with the driver

With your Java application connected to BigQuery through the
JDBC driver, you can now run queries in your development environment through the
[standard JDBC process](https://docs.oracle.com/javase/tutorial/jdbc/basics/processingsqlstatements.html).
All [BigQuery quotas and limits](https://docs.cloud.google.com/bigquery/quotas) apply.

### Data type mapping

When you run queries through the JDBC driver for BigQuery, the
following data type mapping occurs:

| **GoogleSQL type** | **Java type** |
|---|---|
| `ARRAY` | `Array` |
| `BIGNUMERIC` | `BigDecimal` |
| `BOOL` | `Boolean` |
| `BYTES` | `byte[]` |
| `DATE` | `Date` |
| `DATETIME` | `String` |
| `FLOAT64` | `Double` |
| `GEOGRAPHY` | `String` |
| `INT64` | `Long` |
| `INTERVAL` | `String` |
| `JSON` | `String` |
| `NUMERIC` | `BigDecimal` |
| `STRING` | `String` |
| `STRUCT` | `Struct` |
| `TIME` | `Time` |
| `TIMESTAMP` | `Timestamp` |

### Examples

The following sections provide examples that use BigQuery
features through the JDBC driver for BigQuery.

#### Positional parameters

The following example runs a query with a
[positional parameter](https://docs.cloud.google.com/bigquery/docs/parameterized-queries):

```java
PreparedStatement preparedStatement = connection.prepareStatement(
    "SELECT * FROM MyTestTable where testColumn = ?");
preparedStatement.setString(1, "string2");
ResultSet resultSet = statement.executeQuery(selectQuery);
```

#### Nested and repeated records

The following example queries the base record of `Struct` data:

```java
ResultSet resultSet = statement.executeQuery("SELECT STRUCT(\"Adam\" as name, 5 as age)");
    resultSet.next();
    Struct obj = (Struct) resultSet.getObject(1);
    System.out.println(obj.toString());
```

The driver returns the base record as a struct object or a string
representation of a JSON object. The result is similar to the following:

```
{
  "v": {
    "f": [
      {
        "v": "Adam"
      },
      {
        "v": "5"
      }
    ]
  }
}
```

The following example queries the subcomponents of a `Struct` object:

```java
ResultSet resultSet = statement.executeQuery("SELECT STRUCT(\"Adam\" as name, 5 as age)");
    resultSet.next();
    Struct structObject = (Struct) resultSet.getObject(1);
    Object[] structComponents = structObject.getAttributes();
    for (Object component : structComponents){
      System.out.println(component.toString());
    }
```

The following example queries a standard array of repeated data, then verifies
the result:

```java
// Execute Query
ResultSet resultSet = statement.executeQuery("SELECT [1,2,3]");
resultSet.next();
Object[] arrayObject = (Object[]) resultSet.getArray(1).getArray();

// Verify Result
int count =0;
for (; count < arrayObject.length; count++) {
  System.out.println(arrayObject[count]);
}
```

The following example queries a `Struct` array of repeated data, then verifies
the result:

```java
// Execute Query
ResultSet resultSet = statement.executeQuery("SELECT "
    + "[STRUCT(\"Adam\" as name, 12 as age), "
    + "STRUCT(\"Lily\" as name, 17 as age)]");

Struct[] arrayObject = (Struct[]) resultSet.getArray(1).getArray();

// Verify Result
for (int count =0; count < arrayObject.length; count++) {
  System.out.println(arrayObject[count]);
}
```

#### Bulk-insert

The following example performs a bulk-insert operation with the
[`executeBatch` method](https://docs.oracle.com/javase/8/docs/api/java/sql/Statement.html#executeBatch--).

```java
Connection conn = DriverManager.getConnection(connectionUrl);
PreparedStatement statement = null;
Statement st = conn.createStatement();
final String insertQuery = String.format(
        "INSERT INTO `%s.%s.%s` "
      + " (StringField, IntegerField, BooleanField) VALUES(?, ?, ?);",
        DEFAULT_CATALOG, DATASET, TABLE_NAME);

statement = conn.prepareStatement(insertQuery1);

for (int i=0; i<2000; ++i) {
      statement.setString(1, i+"StringField");
      statement.setInt(2, i);
      statement.setBoolean(3, true);
      statement.addBatch();
}

statement.executeBatch();
```

## Logging

To troubleshoot issues with the JDBC driver for BigQuery, you can
enable logging by setting connection properties or environment variables.
Logging can affect performance and take disk space, so only enable it
temporarily to capture an issue.

### Log levels

The `LogLevel` property determines the level of detail that is logged by the
`java.util.logging` package:

- `0`: `OFF` (Default)
- `1`: `SEVERE`
- `2`: `WARNING`
- `3`: `INFO`
- `4`: `CONFIG`
- `5`: `FINE`
- `6`: `FINER`
- `7`: `FINEST`
- `8`: `ALL`

We recommend level 6 for general troubleshooting. Levels 7 and 8 are limited to
`ResultSet` operations and generate a large volume of logs.

### Enable logging in the connection string

To enable logging in the connection string, add the `LogLevel` and `LogPath`
connection properties, for example:

    jdbc:bigquery://https://www.googleapis.com/bigquery/v2:443;ProjectId=MyTestProject;OAuthType=3;LogLevel=6;LogPath=/tmp/jdbc-logs;

### Enable logging with environment variables

If your development tool doesn't allow connection string edits, you can also set
the log level and log path with the following environment variables before
running your application:

- `BIGQUERY_JDBC_LOG_LEVEL`: the log level (0-8).
- `BIGQUERY_JDBC_LOG_PATH`: the directory for log files.

For example, in a Linux or macOS environment, run the following:

    export BIGQUERY_JDBC_LOG_LEVEL=6
    export BIGQUERY_JDBC_LOG_PATH=/tmp/jdbc-logs

## Pricing

You can download the JDBC driver for BigQuery at no cost, and you
don't need any additional licenses to use the drivers. However, when you use the
driver,
[standard BigQuery pricing](https://cloud.google.com/bigquery/pricing)
applies.

## What's next

- Learn more about the [Simba ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers).
- Explore other [BigQuery developer tools](https://docs.cloud.google.com/bigquery/docs/developer-overview).