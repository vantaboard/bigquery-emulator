# Schema detection and mapping for Snowflake

This guide shows you how to define your schema when transferring data from
Snowflake to BigQuery. You can use the
BigQuery Data Transfer Service to automatically detect schema and data-type mapping,
or you can use the translation engine to define your schema and data types manually.

## Enable automatic default schema detection

The Snowflake connector can automatically detect your
Snowflake table schema. To use automatic schema detection, you
can leave the **Translation output GCS path** field blank when you [set up a
Snowflake
transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#set-up-transfer).

The following list shows how the Snowflake connector maps your
Snowflake data types into BigQuery:

- The following data types are mapped as `STRING` in BigQuery:
  - `TIMESTAMP_TZ`
  - `TIMESTAMP_LTZ`
  - `OBJECT`
  - `VARIANT`
  - `ARRAY`
- The following data types are mapped as `TIMESTAMP` in BigQuery:
  - `TIMESTAMP_NTZ`

All other Snowflake data types are mapped directly to their
equivalent types in BigQuery.

## Manually define schema using translation engine output

The BigQuery Data Transfer Service for Snowflake connector uses the BigQuery
migration service translation engine for schema mapping when migrating Snowflake
tables into BigQuery.

To define your schema manually (for example, to override certain schema
attributes), you can generate your metadata, then run the translation engine.

### Limitations

- Data is extracted from Snowflake in the Parquet data format
  before it is loaded into BigQuery:

  - The following Parquet data types are unsupported:
    - `TIMESTAMP_TZ`, `TIMESTAMP_LTZ`
    - For more information, see [Assess Snowflake data](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#limitations).
  - The following Parquet data types are unsupported, but can be converted:

    - `TIMESTAMP_NTZ`
    - `OBJECT`, `VARIANT`, `ARRAY`

    Use the [global type conversion configuration YAML](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#global_type_conversion)
    to override the default behavior of these data types when you run translation engine.

    The configuration YAML might look similar to the following example:

        type: experimental_object_rewriter
        global:
          typeConvert:
            datetime: TIMESTAMP
            json: VARCHAR

### Required service account permissions

In a Snowflake transfer, a service account is used to read
data from the translation engine output in the specified Cloud Storage path.
You must grant the service account the `storage.objects.get` and the
`storage.objects.list` permissions.

We recommend that the service account belongs to the same Google Cloud project
where the transfer configuration and destination dataset is created. If the
service account is in a Google Cloud project that is different from the
project that created the BigQuery data transfer, then you must
[enable cross-project service account authorization](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#cross-project_service_account_authorization).

For more information, see [BigQuery IAM roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Manually define schema mapping

You can manually define your schema mapping with the following steps:

1. Run the `dwh-migration-tool` for Snowflake. For more information, see [Generate metadata for translation and assessment](https://docs.cloud.google.com/bigquery/docs/generate-metadata#snowflake).
2. Upload the generated `metadata.zip` file to a Cloud Storage bucket. The `metadata.zip` file is used as input for the translation engine.
3. Run the batch translation service, specifying the `target_types` field as
   `metadata`. For more information, see [Translate SQL queries with the
   translation API](https://docs.cloud.google.com/bigquery/docs/api-sql-translator).

   - The following is an example of a command to run a batch translation for Snowflake:

         curl -d "{
         \"name\": \"sf_2_bq_translation\",
         \"displayName\": \"Snowflake to BigQuery Translation\",
         \"tasks\": {
             string: {
               \"type\": \"Snowflake2BigQuery_Translation\",
               \"translation_details\": {
                   \"target_base_uri\": \"gs://sf_test_translation/output\",
                   \"source_target_mapping\": {
                     \"source_spec\": {
                         \"base_uri\": \"gs://sf_test_translation/input\"
                     }
                   },
                   \"target_types\": \"metadata\",
               }
             }
         },
         }" \
         -H "Content-Type:application/json" \
         -H "Authorization: Bearer TOKEN" -X POST https://bigquerymigration.googleapis.com/v2alpha/projects/project_id/locations/location/workflows

   - You can check the status of this command in the [SQL Translation page](https://console.cloud.google.com/bigquery/migrations/batch-translation) in BigQuery. The output of the batch translation job is stored in `gs://translation_target_base_uri/metadata/config/`.