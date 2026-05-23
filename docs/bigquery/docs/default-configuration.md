# Manage configuration settings

BigQuery administrators and project owners can manage
configuration settings at the organization and project levels. You can set
configurations to enforce security, control costs, and optimize query
performance across your entire data infrastructure. By setting default values,
you can ensure consistent compliance and operational efficiency, making it
easier to manage your BigQuery environment.

The following sections describe how to specify default configuration
settings. Default settings are configured at an organization or project level
but can be overridden at the session or job level.

## Required roles


To get the permission that
you need to specify a configuration setting,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`bigquery.config.update`
permission,
which is required to
specify a configuration setting.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information on IAM roles and permissions in
BigQuery, see
[Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

## Specify global settings

You can specify global settings at the organization or project level.

### Limitations

Global configuration settings are subject to the following limitations:

- Global organization and project settings are not available in BigQuery [Omni locations](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc).
- When you modify the `default_location` global setting, it can take up to 10 minutes to propagate. Until the setting is propagated, it is possible for eligible queries to be routed to the previous default location.

### Configure global organization settings

If you don't [explicitly specify a location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations),
the location is determined in one of the following ways:

- The location of the datasets referenced in the request. For example, if a query references a table or view in a dataset stored in the `asia-northeast1` region, the query job runs in `asia-northeast1`.
- The region specified for a connection referenced in a request.
- The location of a destination table.

If the location isn't explicitly specified, and it can't be determined from the
resources in the request, the default location is used. If default location
isn't set, the job runs in the `US` multi-region.

You can configure global settings at the organization level by using the
[`ALTER ORGANIZATION SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement). The default
location is the only global organization setting. The default location is used
to run jobs when the location can't be inferred from the request.

When you configure the default location, you don't specify a region where the
setting applies. You can't mix global and regional settings in the same DDL
statement.

> [!NOTE]
> **Note:** When you modify the `default_location` global setting, it can take up to 10 minutes to propagate. Until the setting is propagated, it is possible for eligible queries to be routed to the previous default location.

To configure the `default_location` at the organization level, follow these
steps:

### Console

1. Go to the **BigQuery** page in the Google Cloud console.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click the query editor. This tab is labeled search_insights
   **Untitled query**.

3. To configure the `default_location`, enter the following DDL statement
   into the **Query editor**:

   ```googlesql
     ALTER ORGANIZATION
     SET OPTIONS (
     `default_location` = 'LOCATION'
     );
   ```

   Replace `LOCATION` with a region or multi-region
   [location](https://docs.cloud.google.com/bigquery/docs/locations). This value is the location used to
   run jobs when it can't be inferred from the request. For example, the
   default location is used if the location of the datasets in a query
   can't be determined.
4. Alternatively, to clear the `default_location` organization-level global
   settings, enter the following DDL statement into the **Query editor**:

   ```googlesql
     ALTER ORGANIZATION
     SET OPTIONS (
     `default_location` = NULL
     );
   ```
5. Click **Run**.

### bq

1. To configure the `default_location` at the organization level, enter the
   [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
   and supply the following DDL statement as the query parameter. Set the
   `use_legacy_sql` flag to `false`.

   ```googlesql
   ALTER ORGANIZATION
   SET OPTIONS (
   `default_location` = 'LOCATION'
   );
   ```

   Replace `LOCATION` with a region or multi-region
   [location](https://docs.cloud.google.com/bigquery/docs/locations). This value is the location used to run
   jobs when it can't be inferred from the request. For example, the default
   location is used if the location of the datasets in a query can't be
   determined.
2. To clear the `default_location` at the organization level,
   enter the [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query)
   command and supply the following DDL statement as the query parameter. Set
   the `use_legacy_sql` flag to `false`.

   ```googlesql
   ALTER ORGANIZATION
   SET OPTIONS (
   `default_location` = NULL
   );
   ```

### API

Call the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) method
and supply the DDL statement in the request body's `query` property.

### Configure global project settings

If you don't [explicitly specify a location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations),
the location is determined in one of the following ways:

- The location of the datasets referenced in the request. For example, if a query references a table or view in a dataset stored in the `asia-northeast1` region, the query job runs in `asia-northeast1`.
- The region specified for a connection referenced in a request.
- The location of a destination table.

If the location isn't explicitly specified, and it can't be determined from the
resources in the request, the default location is used. If default location
isn't set, the job runs in the `US` multi-region.

You can configure global settings at the project level by using by using the
[`ALTER PROJECT SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement).
The `ALTER PROJECT SET OPTIONS` DDL statement optionally accepts the
`PROJECT_ID` variable. If the `PROJECT_ID` is not specified, it defaults to the
current project where you run the `ALTER PROJECT` DDL statement.

The default location is the only global project setting. When you configure the
default location, you don't specify a region where the setting applies. You
can't mix global and regional settings in the same DDL statement.

Project-level configurations override organization-level configurations.
Project-level configurations can in turn be overridden by
[session-level configurations](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries), which can
be overridden by [job-level configurations](https://docs.cloud.google.com/bigquery/docs/running-queries).

To configure the `default_location` at the project level, follow these steps:

### Console

1. Go to the BigQuery page in the Google Cloud console.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click the query editor. This tab is labeled search_insights
   **Untitled query**.

3. To configure the `default_location`, enter the following DDL statement
   into the **Query editor**:

   ```googlesql
     ALTER PROJECT PROJECT_ID
     SET OPTIONS (
     `default_location` = 'LOCATION'
     );
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID.

   - `LOCATION`: a region or multi-region
     [location](https://docs.cloud.google.com/bigquery/docs/locations). This value is the location
     used to run jobs when it can't be inferred from the request. For
     example, the default location is used if the location of the
     datasets in a query can't be determined.

4. Alternatively, to clear the `default_location` setting, enter the
   following DDL statement into the **Query editor** . If you
   clear the `default_location` at the project level, organization-level
   default settings are used, if they exist. Otherwise, the system default
   setting is used.

   ```googlesql
     ALTER PROJECT PROJECT_ID
     SET OPTIONS (
     `default_location` = NULL
     );
   ```
5. Click **Run**.

### bq

1. To configure the `default_location` at the project level, enter the
   [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
   and supply the following DDL statement as the query parameter. Set the
   `use_legacy_sql` flag to `false`.

   ```googlesql
   ALTER PROJECT PROJECT_ID
   SET OPTIONS (
   `default_location` = 'LOCATION'
   );
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID.
   - `LOCATION`: a region or multi-region [location](https://docs.cloud.google.com/bigquery/docs/locations). This value is the location used to run jobs when it can't be inferred from the request. For example, the default location is used if the location of the datasets in a query can't be determined.
2. Alternatively, to clear the `default_location` at the project level,
   enter the [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query)
   command and supply the following DDL statement as the query parameter. Set
   the `use_legacy_sql` flag to `false`. If you clear the `default_location` at
   the project level, organization-level default settings are used, if they
   exist. Otherwise, the system default setting is used.

   ```googlesql
   ALTER PROJECT PROJECT_ID
   SET OPTIONS (
   `default_location` = NULL
   );
   ```

### API

Call the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) method
and supply the DDL statement in the request body's `query` property.

## Specify regional settings

You can configure regional settings at the organization or project level.

### Manage reservation and billing controls

You can control how queries use reservations or on-demand billing. These
settings help ensure cost predictability by either allowing flexible reservation
usage or requiring that queries use reservation slots.

Common reservation and billing controls include settings such as
`reservation_override_mode` ([Preview](https://docs.cloud.google.com/products#product-launch-stages)) and
`disable_on_demand_billing` ([Preview](https://docs.cloud.google.com/products#product-launch-stages)). For a complete list of settings and options, see
the [`ALTER PROJECT SET OPTIONS` options list](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#project_set_options_list).

### Configure regional organization settings

You can configure regional settings at the organization level by using the
[`ALTER ORGANIZATION SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement). You must
specify the region where each organization setting applies. You can only use one
region in a statement.

To configure regional organization settings, follow these steps. The following
example specifies several default regional configurations, including the
following:

- Time zone: `America/Chicago`
- Cloud KMS key: a user-defined key
- Query timeout: 30 minutes (1,800,000 milliseconds)
- Interactive query queue timeout: 10 minutes (600,000 milliseconds)
- Batch query queue timeout: 20 minutes (1,200,000 milliseconds)
- `INFORMATION_SCHEMA`: enabled

To see all regional organization settings, go to
[`organization_set_options_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#organization_set_options_list).

### Console

1. Go to the BigQuery page in the Google Cloud console.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click the query editor. This tab is labeled search_insights
   **Untitled query**.

3. To configure the regional organization settings, enter the following DDL
   statement into the **Query editor**:

   ```googlesql
     ALTER ORGANIZATION
     SET OPTIONS (
     `region-REGION.default_time_zone`= 'America/Chicago',
     -- Ensure all service accounts under the organization have permission to KMS_KEY
     `region-REGION.default_kms_key_name` = KMS_KEY,
     `region-REGION.default_query_job_timeout_ms` = 1800000,
     `region-REGION.default_interactive_query_queue_timeout_ms` = 600000,
     `region-REGION.default_batch_query_queue_timeout_ms` = 1200000,
     `region-REGION.enable_info_schema_storage` = true);
   ```

   Replace the following:
   - `REGION`: the [region](https://docs.cloud.google.com/bigquery/docs/locations#regions) associated with your project or organization---for example, `us` or `europe-west6`. The value for `REGION` must be the same for each option in the command.
   - `KMS_KEY`: a user-defined Cloud KMS key. For more information, see [Customer-managed Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).
4. Alternatively, to clear the regional organization settings, enter the
   following DDL statement into the **Query editor**:

   ```googlesql
     ALTER ORGANIZATION
     SET OPTIONS (
     `region-REGION.default_time_zone` = NULL,
     `region-REGION.default_kms_key_name` = NULL,
     `region-REGION.default_query_job_timeout_ms` = NULL,
     `region-REGION.default_interactive_query_queue_timeout_ms` = NULL,
     `region-REGION.default_batch_query_queue_timeout_ms` = NULL,
     `region-REGION.enable_info_schema_storage` = NULL);
   ```
5. Click **Run**.

### bq

To configure the regional organization settings, enter the
[`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
and supply the following DDL statement as the query parameter. Set the
`use_legacy_sql` flag to `false`.

```googlesql
  ALTER ORGANIZATION
  SET OPTIONS (
  `region-REGION.default_time_zone`= 'America/Chicago',
  -- Ensure all service accounts under the organization have permission to KMS_KEY
  `region-REGION.default_kms_key_name` = KMS_KEY,
  `region-REGION.default_query_job_timeout_ms` = 1800000,
  `region-REGION.default_interactive_query_queue_timeout_ms` = 600000,
  `region-REGION.default_batch_query_queue_timeout_ms` = 1200000);
```

Replace the following:

- `REGION`: the [region](https://docs.cloud.google.com/bigquery/docs/locations#regions) associated with your project or organization---for example, `us` or `europe-west6`. The value for `REGION` must be the same for each option in the command.
- `KMS_KEY`: a user-defined Cloud KMS key. For more information, see [Customer-managed Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).

Alternatively, to clear the regional organization settings, enter the
[`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
and supply the following DDL statement as the query parameter. Set the
`use_legacy_sql` flag to `false`:

```googlesql
ALTER ORGANIZATION
SET OPTIONS (
  `region-REGION.default_time_zone` = NULL,
  `region-REGION.default_kms_key_name` = NULL,
  `region-REGION.default_query_job_timeout_ms` = NULL,
  `region-REGION.default_interactive_query_queue_timeout_ms` = NULL,
  `region-REGION.default_batch_query_queue_timeout_ms` = NULL,
  `region-REGION.default_storage_billing_model`= NULL,
  `region-REGION.default_max_time_travel_hours` = NULL,
  `region-REGION.default_cloud_resource_connection_id` = NULL,
  `region-REGION.default_sql_dialect_option` = NULL,
  `region-REGION.enable_reservation_based_fairness` = NULL,
  `region-REGION.enable_global_queries_execution` = NULL,
  `region-REGION.enable_global_queries_data_access` = NULL);
```

### API

Call the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) method
and supply the DDL statement in the request body's `query` property.

### Configure regional project settings

You can configure regional settings at the project level by using the
[`ALTER PROJECT SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement).
When you specify the configuration, you must specify the region where it
applies. You can only use one region in each statement.

Project-level configurations override organization-level configurations.
Project-level configurations can in turn be overridden by
[session-level configurations](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries), which can
be overridden by [job-level configurations](https://docs.cloud.google.com/bigquery/docs/running-queries).

The `ALTER PROJECT SET OPTIONS` DDL statement optionally accepts the
`PROJECT_ID` variable. If the `PROJECT_ID` variable is not specified, it
defaults to the current project where you run the `ALTER PROJECT` DDL statement.

The following example specifies several regional, project-level settings,
including the following:

- Time zone: `America/Los_Angeles`
- Cloud KMS key: an example key
- Query timeout: 1 hour (1,800,000 milliseconds)
- Interactive query queue timeout: 10 minutes (600,000 milliseconds)
- Batch query queue timeout: 20 minutes (1,200,000 milliseconds)
- Reservation-based fairness: enabled
- Global queries: enabled for running and for accessing data
- `INFORMATION_SCHEMA`: enabled

To see all regional project settings, go to
[`project_set_options_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#project_set_options_list).

### Console

1. Go to the BigQuery page in the Google Cloud console.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click the query editor. This tab is labeled search_insights
   **Untitled query**.

3. To configure the regional project settings, enter the following DDL
   statement into the **Query editor**:

   ```googlesql
    ALTER PROJECT PROJECT_ID
    SET OPTIONS (
    `region-REGION.default_time_zone` = 'America/Los_Angeles',
    -- Ensure all service accounts under the project have permission to KMS_KEY
    `region-REGION.default_kms_key_name` = KMS_KEY,
    `region-REGION.default_query_job_timeout_ms` = 3600000,
    `region-REGION.default_interactive_query_queue_timeout_ms` = 600000,
    `region-REGION.default_batch_query_queue_timeout_ms` = 1200000,
    `region-REGION.enable_reservation_based_fairness` = true,
    `region-REGION.enable_global_queries_execution` = true,
    `region-REGION.enable_global_queries_data_access` = true,
    `region-REGION.enable_info_schema_storage` = true);
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID.
   - `REGION`: the [region](https://docs.cloud.google.com/bigquery/docs/locations#regions) associated with your project or organization---for example, `us` or `europe-west6`. The value for `REGION` must be the same for each option in the command.
   - `KMS_KEY`: a user-defined Cloud KMS key. For more information, see [Customer-managed Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).
4. Alternatively, to clear the regional project settings, enter the
   following DDL statement into the **Query editor**:

   ```googlesql
     ALTER PROJECT PROJECT_ID
     SET OPTIONS (
     `region-REGION.default_time_zone` = NULL,
     `region-REGION.default_kms_key_name` = NULL,
     `region-REGION.default_query_job_timeout_ms` = NULL,
     `region-REGION.default_interactive_query_queue_timeout_ms` = NULL,
     `region-REGION.default_batch_query_queue_timeout_ms` = NULL,
     `region-REGION.enable_reservation_based_fairness` = false,
     `region-REGION.enable_info_schema_storage` = NULL);
   ```
5. Click **Run**.

### bq

1. To configure the regional project settings, enter the
   [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
   and supply the following DDL statement as the query parameter. Set the
   `use_legacy_sql` flag to `false`.

   ```googlesql
   ALTER PROJECT PROJECT_ID
   SET OPTIONS (
   `region-REGION.default_time_zone`= 'America/Chicago',
   -- Ensure all service accounts under the organization have permission to KMS_KEY
   `region-REGION.default_kms_key_name` = KMS_KEY,
   `region-REGION.default_query_job_timeout_ms` = 1800000,
   `region-REGION.default_interactive_query_queue_timeout_ms` = 600000,
   `region-REGION.default_batch_query_queue_timeout_ms` = 1200000,
   `region-REGION.enable_reservation_based_fairness` = true);
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID.
   - `REGION`: the [region](https://docs.cloud.google.com/bigquery/docs/locations#regions) associated with your project or organization---for example, `us` or `europe-west6`. The value for `REGION` must be the same for each option in the command.
   - `KMS_KEY`: a user-defined Cloud KMS key. For more information, see [Customer-managed Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).
2. Alternatively, to clear the regional project settings, enter the
   [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
   and supply the following DDL statement as the query parameter. Set the
   `use_legacy_sql` flag to `false`:

   ```googlesql
   ALTER ORGANIZATION
   SET OPTIONS (
   `region-REGION.default_time_zone` = NULL,
   `region-REGION.default_kms_key_name` = NULL,
   `region-REGION.default_query_job_timeout_ms` = NULL,
   `region-REGION.default_interactive_query_queue_timeout_ms` = NULL,
   `region-REGION.default_batch_query_queue_timeout_ms` = NULL,
   `region-REGION.enable_reservation_based_fairness` = false,
   `region-REGION.enable_global_queries_execution` = NULL,
   `region-REGION.enable_global_queries_data_access` = NULL);
   ```

### API

Call the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) method
and supply the DDL statement in the request body's `query` property.

## Retrieve configuration settings

You can view the configuration settings for an organization or project
by using the following [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro)
views:

- [`INFORMATION_SCHEMA.PROJECT_OPTIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-project-options): the configurations applied to a project.
- [`INFORMATION_SCHEMA.EFFECTIVE_PROJECT_OPTIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-effective-project-options): the effective configurations applied to a project. Effective configurations include all configurations set at the project level as well as all settings inherited by the project from an organization.
- [`INFORMATION_SCHEMA.ORGANIZATION_OPTIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-organization-options): the configurations applied to an organization.

It may take a few minutes for new configurations to become effective and
reflected within the `INFORMATION_SCHEMA` view.

### Required roles


To get the permission that
you need to retrieve configuration settings,

ask your administrator to grant you the
[BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`) IAM role on the specified project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`bigquery.config.get`
permission,
which is required to
retrieve configuration settings.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information on IAM roles and permissions in
BigQuery, see
[Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

### Examples

Use the following query examples to retrieve your project and organization
settings from the `INFORMATION_SCHEMA` views.

#### View global settings

To view all global organization settings, run the following query:

```googlesql
SELECT * FROM INFORMATION_SCHEMA.ORGANIZATION_OPTIONS;
```

To view just the default location organization setting, run the following query:

```googlesql
SELECT
    option_value
FROM INFORMATION_SCHEMA.ORGANIZATION_OPTIONS
WHERE option_name = 'default_location'
```

To view all effective global configurations for your default project, run the
following query:

```googlesql
SELECT * FROM INFORMATION_SCHEMA.EFFECTIVE_PROJECT_OPTIONS;
```

To view just the default location effective global configuration for your
default project, run the following query:

```googlesql
SELECT
    option_value
FROM INFORMATION_SCHEMA.EFFECTIVE_PROJECT_OPTIONS
WHERE option_name = 'default_location'
```

To view all global configurations for your default project, run the
following query:

```googlesql
SELECT * FROM INFORMATION_SCHEMA.PROJECT_OPTIONS;
```

To view just the default location setting for your default project, run the
following query:

```googlesql
SELECT
    option_value
FROM INFORMATION_SCHEMA.PROJECT_OPTIONS
WHERE option_name = 'default_location'
```

#### View regional settings

To view the configurations under an organization in the `us` region, run the
following query:

```googlesql
SELECT * FROM region-us.INFORMATION_SCHEMA.ORGANIZATION_OPTIONS;
```

To view the effective configurations under your default project in the `us`
region, run the following query:

```googlesql
SELECT * FROM region-us.INFORMATION_SCHEMA.EFFECTIVE_PROJECT_OPTIONS;
```

To view the configurations under your default project in the `us` region,
run the following query:

```googlesql
SELECT * FROM region-us.INFORMATION_SCHEMA.PROJECT_OPTIONS;
```

## Configuration settings

The following sections describe the configuration settings that you can specify.

### Query and job execution settings

Use the following settings to control how queries are executed, timed, and
queued.

- `default_batch_query_queue_timeout_ms`: The default amount of time, in
  milliseconds, that a batch query is
  [queued](https://docs.cloud.google.com/bigquery/docs/query-queues). If unset, the default is 24
  hours. The minimum value is 1 millisecond. The maximum value is 48 hours.
  To turn off batch query queueing, set the value to `-1`.

- `default_interactive_query_queue_timeout_ms`: The default amount of time,
  in milliseconds, that an interactive query is
  [queued](https://docs.cloud.google.com/bigquery/docs/query-queues). If unset, the default
  is six hours. The minimum value is 1 millisecond. The maximum value is 48
  hours. To turn off interactive query queueing, set the value to `-1`.

- `default_query_job_timeout_ms`: The default time after which a query job
  times out, including the time the job is queued and the time spent running.
  The timeout period must be between 5 minutes and 48 hours. This timeout
  only applies to individual query jobs and the child jobs of scripts. To set
  a timeout for script jobs, you should use the [jobs.insert](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
  API method and set the `jobTimeoutMs` field.

  > [!NOTE]
  > **Note:** The `default_query_job_timeout_ms` setting also applies to [continuous query](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) jobs. To override this project-level setting for an individual continuous query, assign a [job timeout](https://docs.cloud.google.com/bigquery/docs/continuous-queries#run_a_continuous_query_by_using_a_service_account) to the continuous query in question. Continuous queries still adhere to [maximum runtimes](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#authorization).

- `default_location`: The [`default_location` configuration setting](https://docs.cloud.google.com/bigquery/docs/default-configuration#global-settings)
  is used to run jobs when the [location isn't set or can't be determined](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations).
  If `default_location` isn't set, the job runs in the `US` multi-region.

- `enable_reservation_based_fairness`: The option that determines how idle
  slots are shared. The default value is false, which means idle slots are
  equally distributed across all query projects. If enabled, the idle slots
  are shared equally across all reservations first, and then across projects
  within the reservation. For more information, see
  [reservation-based fairness](https://docs.cloud.google.com/bigquery/docs/slots#fairness). This option is
  only supported at the project level. You can't specify it at the
  organization or job level.

- `default_time_zone`: The default time zone to use in time zone-dependent
  GoogleSQL functions, when a time zone is not specified as an
  argument. This configuration does not apply to
  [time-unit column partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#date_timestamp_partitioned_tables)
  (which use UTC as the time zone), the
  [Storage Transfer Service schedule transfers](https://docs.cloud.google.com/storage-transfer/docs/schedule-transfer-jobs),
  or [loading data with the bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#loading_data).
  For more information, see
  [time zones](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zones).

  > [!NOTE]
  > **Note:** If you want to set a default time zone, ensure any existing queries that use `DATETIME` literals aren't affected. This includes queries with the explicit `DATETIME` keyword, implicitly converted string literals passed as a parameter to time functions like `DATETIME_DIFF('2022-10-01', ...)`, the `PARSE_DATETIME()` function, and more. For this reason, it is safer to only set the `default_time_zone` parameter on new projects.

- `default_query_optimizer_options`: The history-based query optimizations.
  This option can be one of the following:

  - `'adaptive=on'`: Use history-based query optimizations.
  - `'adaptive=off'`: Don't use history-based query optimizations.
  - `NULL` (default): Use the default history-based query optimizations setting, which is equivalent to `'adaptive=on'`.
- `default_sql_dialect_option`: The default SQL query dialect for executing
  query jobs using the bq command-line tool or BigQuery API. Changing this setting
  doesn't affect the default dialect in the console. This option can be one of
  the following:

  - `'default_legacy_sql'` (default): Use legacy SQL if the query dialect isn't specified at the job level.
  - `'default_google_sql'`: Use GoogleSQL if the query dialect isn't specified at the job level.
  - `'only_google_sql'`: Use GoogleSQL if the query dialect isn't specified at the job level. Reject jobs with query dialect set to legacy SQL.
  - `NULL`: Use the default query dialect setting, which is equivalent to `'default_legacy_sql'`.
- `enable_global_queries_execution`: The option that determines if
  [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries) can be run. The default
  value is `FALSE`, which means that global queries are not enabled.

- `enable_global_queries_data_access`: The option that determines if
  [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries) can access data stored in
  the region. The default value is `FALSE`, which means that global queries
  can't copy data from this region regardless of the project in which they
  run.

### Data management settings

Use the following settings to define rules for data creation, security, and
lifecycle.

- `default_column_name_character_map`: The default scope and handling of
  characters in column names. If unset, load jobs that use unsupported
  characters in column names fail with an error message. Some older
  tables might be set to replace unsupported characters in column names.
  For more information, see
  [`load_option_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements#load_option_list).

- `default_kms_key_name`: The default Cloud Key Management Service key for encrypting table
  data, including temporary or anonymous tables. For more information, see
  [Customer-managed Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).

  > [!NOTE]
  > **Note:** to set a default Cloud KMS key, you must grant the Encrypter/Decrypter role to all BigQuery service accounts that are used within the project or organization. If a service account within the project or organization doesn't have appropriate permissions, all queries run by the service account fail. For information about assigning the Encrypter/Decrypter role, see [Assign the Encrypter/Decrypter role](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#assign_role). If you set a default Cloud KMS key without first assigning the appropriate roles, you can clear the default key by setting the value to `NULL`. For examples, see [Configure organization settings](https://docs.cloud.google.com/bigquery/docs/default-configuration#configure-organization-settings) and [Configure project settings](https://docs.cloud.google.com/bigquery/docs/default-configuration#configure-project-settings).

- `default_max_time_travel_hours`: The default time travel window in hours for
  new datasets. This duration must be within the range of 48 to 168,
  inclusive, and must be divisible by 24. Changing the default max time travel
  hours does not affect existing datasets. For more information, see
  [Time Travel and data retention](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel).

- `enable_info_schema_storage`: The option that provides access to
  [`INFORMATION_SCHEMA.TABLE_STORAGE`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage)
  and [`SEARCH_INDEXES`](https://docs.cloud.google.com/bigquery/docs/information-schema-indexes)
  views and their variants. By default, this option is not enabled. If you
  query these views for the first time without setting this option to `TRUE`,
  the query fails and provides instructions to enable it. After this option is
  enabled, queries succeed immediately and return data generated from that
  point forward. A complete backfill of historical data can take approximately
  one day to become available in the views. If you used these
  views before this setting was introduced, this option is already enabled.

### Cost and resource settings

Use the following settings to determine how resources are billed and connected.

- `default_storage_billing_model`: The default storage billing model for new datasets. Set the value to `PHYSICAL` to use physical bytes when calculating storage charges or to `LOGICAL` to use logical bytes. Note that changing the default storage billing model does not affect existing datasets. For more information, see [Storage billing models](https://docs.cloud.google.com/bigquery/docs/datasets-intro#dataset_storage_billing_models).
- `default_cloud_resource_connection_id`: The default connection to use when creating tables and models. Only specify the connection's ID or name, and exclude the attached project ID and region prefixes. Using default connections can cause the permissions granted to the connection's service account to be updated, depending on the type of table or model you're creating. For more information, see the [Default connection
  overview](https://docs.cloud.google.com/bigquery/docs/default-connections).
- `reservation_override_mode` ([Preview](https://docs.cloud.google.com/products#product-launch-stages)): Specifies how query reservations can be overridden in the region (for example, `'ALLOW_ANY_OVERRIDE'`). For details, see the [`ALTER PROJECT SET OPTIONS` options
  list](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#project_set_options_list).
- `disable_on_demand_billing` ([Preview](https://docs.cloud.google.com/products#product-launch-stages)): Determines whether on-demand billing is disabled for queries in the region. If `true`, all queries must use an assigned reservation or they fail. For details, see the [`ALTER PROJECT SET OPTIONS`
  options
  list](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#project_set_options_list).

## Pricing

There is no additional charge to use the BigQuery configuration
service. For more information, see [Pricing](https://cloud.google.com/bigquery/pricing).