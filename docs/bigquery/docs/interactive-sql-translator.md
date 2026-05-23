# Translate queries with the interactive SQL translator

This document describes how to translate a query from a different SQL dialect
into a GoogleSQL query by using the BigQuery
interactive SQL translator. The interactive SQL translator can help
reduce time and effort when you migrate workloads to BigQuery.
This document is intended for users who are familiar with the
[Google Cloud console](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui).

You can use the [translation rule feature](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#customize) to customize the way the
interactive SQL translator translates SQL.

## Before you begin

If your Google Cloud CLI project was created before February 15, 2022, enable
the BigQuery Migration API as follows:

1. In the Google Cloud console, go to the **BigQuery Migration API** page.

   [Go to BigQuery Migration API](https://console.cloud.google.com/apis/api/bigquerymigration.googleapis.com/overview)
2. Click **Enable**.

> [!NOTE]
> **Note:** Projects created after February 15, 2022 have this API enabled automatically.

### Permissions and roles

This section describes the
[Identity and Access Management (IAM) permissions](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
that you need in order to use the interactive SQL translator, including
the [predefined IAM roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
that grant those permissions. The section also describes the permissions needed
to configure additional translation configurations.

#### Permissions to use the interactive SQL translator


To get the permissions that
you need to use the interactive translator,

ask your administrator to grant you the
[MigrationWorkflow Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquerymigration#bigquerymigration.editor) (`roles/bigquerymigration.editor`) IAM role on the `parent` resource.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to use the interactive translator. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to use the interactive translator:

- `bigquerymigration.workflows.create`
- `bigquerymigration.workflows.get`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

#### Permissions to configure additional translation configurations

You can configure additional translation configurations using the **Translation
Config ID** and **Translation Configuration Source Location** fields in the
translation settings. To configure these translation configurations, you need
the following permissions:

- `bigquerymigration.workflows.get`
- `bigquerymigration.workflows.list`

The following predefined IAM role provide the permissions that you need
to configure additional translation configurations:

- `roles/bigquerymigration.viewer`

For more information about
BigQuery IAM, see
[Access control with IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

## Supported SQL dialects

The BigQuery interactive SQL translator can translate the
following SQL dialects into GoogleSQL:

- Amazon Redshift SQL
- Apache HiveQL and Beeline CLI
- IBM Netezza SQL and NZPLSQL
- Teradata and Teradata Vantage:
  - SQL
  - Basic Teradata Query (BTEQ)
  - Teradata Parallel Transport (TPT)

Additionally, translation of the following SQL dialects is supported in
[preview](https://cloud.google.com/products/#product-launch-stages):

- Apache Impala SQL
- Apache Spark SQL
- Azure Synapse T-SQL
- GoogleSQL (BigQuery)
- Greenplum SQL
- IBM DB2 SQL
- MySQL SQL
- Oracle SQL, PL/SQL, Exadata
- PostgreSQL SQL
- Trino or PrestoSQL
- Snowflake SQL
- SQL Server T-SQL
- SQLite
- Vertica SQL

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

The interactive SQL translator is available in the following
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

[Gemini-based translation configurations](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#ai_yaml_guidelines) are only available in specific processing locations. For more information, see [Google model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#google_model_endpoint_locations)

## Translate a query into GoogleSQL

Follow these steps to translate a query into GoogleSQL:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Editor** pane, click **More** , and then select **Translation settings**.

3. For **Source dialect**, select the SQL dialect that you want to translate.

4. Optional. For **Processing location** , select the location where you want the
   translation job to run. For example, if you are in Europe and you don't want
   your data to cross any location boundaries, select the `eu` region.

5. Click **Save**.

6. In the **Editor** pane, click **More** , and then select **Enable SQL
   translation**.

   The **Editor** pane splits into two panes.
7. In the left pane, enter the query you want to translate.

8. Click **Translate**.

   BigQuery translates your query into GoogleSQL and displays it in the right
   pane. For example, the following screenshot shows translated Teradata SQL:

   ![Displays a Teradata SQL query translated into GoogleSQL](https://docs.cloud.google.com/static/bigquery/images/translated-teradata-query.png)
9. Optional: To run the translated GoogleSQL query, click **Run**.

10. Optional: To return to the SQL editor, click **More** , and then select
    **Disable SQL translation**.

    The **Editor** pane returns to a single pane.

## Use Gemini with the interactive SQL translator

You can configure the interactive SQL translator to adjust how the
interactive SQL translator translates your source SQL. You can do so by
providing your own rules for use with Gemini in a YAML configuration
file, or by providing a configuration YAML file containing
SQL object metadata or object mapping information.

### Create and apply Gemini-enhanced translation rules

> [!NOTE]
> **Note:** To get support and provide feedback for preview features, contact [ai-sql-translation-help@google.com](mailto:ai-sql-translation-help@google.com).

You can customize the way the interactive SQL translator
translates SQL by creating translation rules. The interactive SQL translator
adjusts its translations based on any Gemini-enhanced SQL
translation rules that you assign to it, letting you customize the translation
results based on your migration needs.

To create a Gemini-enhanced SQL translation rule, you can either
create it in the console, or create a configuration YAML file and upload it
to Cloud Storage.

### Console

To create a Gemini-enhanced SQL translation rule for the input
SQL, write an input SQL query in the query editor,
then click **ASSIST** \> **Customize** . ([Preview](https://cloud.google.com/products#product-launch-stages))

![Customize translation input](https://docs.cloud.google.com/static/bigquery/images/customize-translation-input.png)

Similarly, to create a Gemini-enhanced SQL translation rule
for the output SQL, run an interactive translation, then click
**ASSIST** \> **Customize this translation**.

![Customize translation output](https://docs.cloud.google.com/static/bigquery/images/customize-translation-output.png)

When the **Customize** menu appears, continue with the following steps.

1. Use one or both of the following prompts to create a translation rule:

   - In the **Find and replace a pattern** prompt, specify a SQL
     pattern that you want to replace in the **Replace** field, and a SQL pattern
     to replace it in the **With** field.

     A SQL pattern can contain any number of statements, clauses, or functions
     in a SQL script. When you create a rule using this prompt, the Gemini
     enhanced SQL translation identifies any instances of that SQL pattern in
     the SQL query and dynamically replaces it with another SQL
     pattern. For example, you can use this prompt to create a rule that
     replaces all occurrences of `months_between (X,Y)` with `date_diff(X,Y,MONTH)`.
   - In the **Describe a change to the output** field, type
     a change to the SQL translation output in natural language.

     When you create a rule using this prompt, the Gemini-enhanced
     SQL translation identifies the request and makes the specified
     change to the SQL query.
2. Click **Preview**.

3. In the **Suggestions generated by Gemini** dialog, review the changes made by the
   Gemini-enhanced SQL translation to the SQL query based
   on your rule.

   ![Apply changes from Gemini-based configuration YAML file](https://docs.cloud.google.com/static/bigquery/images/gemini-suggested-changes-1.png)
4. Optional: To add this rule for use with future translations, select the
   **Save this prompt...** checkbox.

   Rules are saved in the default configuration YAML file, or `__default.ai_config.yaml`.
   This configuration YAML file is saved to the Cloud Storage folder as
   specified in the **Translation Configuration Source Location** field in
   the [translation settings](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#translate-with-additional-configs). If the
   **Translation Configuration Source Location** isn't already set, a
   folder browser appears and lets you select one. A configuration YAML
   file is subject to [configuration file size limitations](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#config-limitations).
5. To apply the suggested changes to the SQL query, click **Apply**.

### YAML

To create a Gemini-enhanced SQL translation rule, you can
create a Gemini-based configuration YAML file and upload
it to Cloud Storage. For more information, see
[Create a Gemini-based configuration YAML file](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#ai_yaml_guidelines).

Once you have uploaded a Gemini-enhanced SQL translation rule
and uploaded it to Cloud Storage, you can apply the rule by doing
the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, click **More \> Translation settings**.

3. In the **Translation Configuration Source Location** field, specify the
   path to the Gemini-based YAML file stored in a Cloud Storage
   folder.

4. Click **Save**.

   Once saved, run an interactive translation. The interactive translator suggests
   changes to your translations based on the rules in your configuration YAML
   file if one is available.

If a Gemini
suggestion is available for the input based on your rule, then the
**Preview suggested changes** dialog appears and shows possible changes to
the translation input. ([Preview](https://cloud.google.com/products#product-launch-stages))

If a Gemini suggestion is available for the output based on
your rule, a notification banner appears in the code
editor. To review and apply these suggestions, do the following:

1. Click **Assist** \> **View suggestions** on either side of the
   code editor to revisit the suggested changes to the corresponding query.

   ![Apply changes from Gemini-based configuration YAML file](https://docs.cloud.google.com/static/bigquery/images/gemini-suggested-changes-2.png)
2. In the **Suggestions generated by Gemini** dialog, review the changes made by
   Gemini to the SQL query based on your translation rule.

3. To apply the suggested changes to the translation output, click **Apply**.

#### Update Gemini-based configuration YAML file

To update an existing configuration YAML file, do the following:

1. On the **Suggestions generated in Gemini** dialog, click **View Gemini rule config file**.

2. When the configuration editor appears, select the configuration YAML file that you
   want to edit.

3. Make the change and click **Save**.

4. Close the YAML editor by clicking **Done**.

5. Run an interactive translation to apply the updated rule.

### Explain a translation

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
> **Note:** With Gemini-enhanced SQL translations, you can generate text explanation for your scripts by the Gemini model. Gemini-enhanced SQL translations are allowed a limited amount of Gemini usage at no charge. This usage is sufficient for most migration projects. To request an increase to this limit, or to get support and provide feedback for this Preview feature, contact [ai-sql-translation-help@google.com](mailto:ai-sql-translation-help@google.com).

After running an interactive translation, you can request a
Gemini-generated text explanation. The generated text includes a
summary of the translated SQL query. Gemini also identifies
translation differences and inconsistencies between the source SQL query and the
translated GoogleSQL query.

To get Gemini-generate SQL translation explanation, do the
following:

1. To create a Gemini-generated SQL translation explanation, click **Assist** , and then click **Explain this translation**.

   ![Explain translation button.](https://docs.cloud.google.com/static/bigquery/images/int-translate-explain.png)

### Translate with a batch translation configuration ID

You can run an interactive query with the same translation configurations as
a batch translation job by providing a batch translation configuration ID.

1. In the query editor, click **More \> Translation settings**.
2. In the **Translation Configuration ID** field, provide a batch translation
   configuration ID to apply the same translation configuration from a completed
   BigQuery batch migration job.

   To find a job's batch translation configuration ID, select a batch
   translation job from the **SQL translation** page, and then click the
   **Translation Configuration** tab. The batch translation configuration
   ID is listed as **Resource name**.
3. Click **Save**.

### Translate with additional configurations

You can run an interactive query with additional translation configurations
by specifying configuration YAML files
stored on a Cloud Storage folder. Translation configurations might include
SQL object metadata or object mapping information from the source database that
can improve translation quality. For example, include DDL information or schemas
from the source database to improve interactive SQL translation quality.

To specify translation configurations by providing a location to the translation
configuration source files, do the following:

1. In the query editor, click **More \> Translation settings**.
2. In the **Translation Configuration Source Location** field, specify the path
   to the translation configuration files stored in a Cloud Storage folder.

   The BigQuery interactive SQL translator supports
   metadata ZIP files containing
   [translation metadata](https://docs.cloud.google.com/bigquery/docs/generate-metadata)
   and [object name mapping](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#json_file_format).
   For information on how to upload files to Cloud Storage, see
   [Upload objects from a filesystem](https://docs.cloud.google.com/storage/docs/uploading-objects).
3. Click **Save**.

### Configuration file size limitations

When you use a translation configuration file with the BigQuery
interactive SQL translator, the compressed metadata file or YAML config file
must be smaller than 50 MB. If the file size exceeds 50 MB, the interactive
translator skips that configuration file during translation and produces an
error message similar to the following:

`CONFIG ERROR: Skip reading file "gs://metadata-file.zip". File size (150,000,000 bytes)
exceeds limit (50 MB).`

One method to reduce metadata file size is to use the `--database` or `--schema`
flags to only extract metadata for databases or schemas that are relevant for
the translation input queries. For more information about using these flags when
you [generate metadata files](https://docs.cloud.google.com/bigquery/docs/generate-metadata), see [Global flags](https://docs.cloud.google.com/bigquery/docs/generate-metadata#global_flags).

## Troubleshoot translation errors

The following are commonly encountered errors when using the interactive SQL translator.

### `RelationNotFound` or `AttributeNotFound` translation issues

After translating a query using the [interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#translate_a_query_into_standard_sql), you might encounter a failed translation with the `RelationNotFound` or `AttributeNotFound` error.

You can find failed translations by navigating to the **Translation details** page and opening the **Log Messages** tab.

To ensure the most accurate translation,
you can input the data definition language (DDL) statements for any tables used
in a query prior to the query itself. For example, if you want
to translate the Amazon Redshift query `select table1.field1, table2.field1
from table1, table2 where table1.id = table2.id;`, you would input the
following SQL statements into the interactive SQL translator:

    create table schema1.table1 (id int, field1 int, field2 varchar(16));
    create table schema1.table2 (id int, field1 varchar(30), field2 date);

    select table1.field1, table2.field1
    from table1, table2
    where table1.id = table2.id;

#### Fix translation issues with Gemini

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
> **Note:** To request feedback or support for this feature, contact [bq-edw-migration-support@google.com](mailto:bq-edw-migration-support@google.com).

To fixed failed translation jobs with the `RelationNotFound` or `AttributeNotFound` errors, you can also use Gemini to try to resolve these issues with the following steps.

1. Navigate to the **Translation details** page and open the **Log Messages** tab.

2. Click the query that has the message `RelationNotFound` or `AttributeNotFound` in the **Category** column.

3. Click **Suggested fix**.

4. Click **Apply**.

5. Click **Translate** to retranslate the query.

## Pricing

There is no charge to use the interactive SQL translator. However,
storage used to store input and output files incurs the normal fees. For more
information, see [Storage pricing](https://cloud.google.com/bigquery/pricing#storage).

## What's next

Learn more about the following steps in data warehouse migration:

- [Migration overview](https://docs.cloud.google.com/bigquery/docs/migration/migration-overview)
- [Migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment)
- [Schema and data transfer overview](https://docs.cloud.google.com/bigquery/docs/migration/schema-data-overview)
- [Batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator)
- [Data pipelines](https://docs.cloud.google.com/bigquery/docs/migration/pipelines)
- [Data security and governance](https://docs.cloud.google.com/bigquery/docs/data-governance)
- [Data validation tool](https://github.com/GoogleCloudPlatform/professional-services-data-validator#data-validation-tool)