# Where Gemini in BigQuery processes your data

This document helps you understand where Gemini in BigQuery
processes your data. This behavior applies to the following Gemini in BigQuery
features:

- [SQL code assist](https://docs.cloud.google.com/bigquery/docs/gemini-locations#sql-editor-canvas)
- [BigQuery Data Engineering Agent](https://docs.cloud.google.com/bigquery/docs/gemini-locations#bigquery-data-engineering-agent)
- [BigQuery data canvas](https://docs.cloud.google.com/bigquery/docs/gemini-locations#sql-editor-canvas)
- [BigQuery data insights](https://docs.cloud.google.com/bigquery/docs/gemini-locations#bigquery-data-insights)
- [BigQuery data preparation](https://docs.cloud.google.com/bigquery/docs/gemini-locations#bigquery-data-preparation)

For these features, Gemini processing occurs in the
jurisdictional boundaries of the query location, or where the
BigQuery dataset is stored. For example, if your
BigQuery query location or dataset is in the `europe-west1`
region, Gemini processing occurs in a location within the `EU`
jurisdictional boundary. This design minimizes data movement and follows data
governance best practices. For more information about restrictions on available
jurisdictions, see [Limitations](https://docs.cloud.google.com/bigquery/docs/gemini-locations#limitations).

For most Gemini in BigQuery features, the
Gemini processing location can be controlled by an administrator
by using the **Global Default Location** setting at the project or organization
level. BigQuery users can override this global default location
by using the **Query Location** setting in BigQuery Studio. In cases
where a query location setting isn't specified in configuration settings by an
administrator or explicitly by the user in the query, Gemini in
BigQuery uses the location derived from the query being edited.
To learn more about how BigQuery determines query location see
[Run a query](https://docs.cloud.google.com/bigquery/docs/running-queries).

Gemini in BigQuery determines the jurisdiction of
`US` or `EU` based upon these controls. If a jurisdiction cannot be determined,
then the global processing location is used based upon the
[Gemini serving locations](https://docs.cloud.google.com/gemini/docs/locations).

The following sections explain how you can manage where each
Gemini in BigQuery feature processes your data.

## SQL editor and data canvas

When you [generate code using the SQL editor](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini),
or use [data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas) to create data analysis
workflows, Gemini in BigQuery uses the following
logic to determine the processing location:

- A BigQuery administrator can specify a default
  organization-level or project-level location. To learn how to specify a
  default location, see [Specify the default organization-level or
  project-level location](https://docs.cloud.google.com/bigquery/docs/gemini-locations#specify-default-location).

- A BigQuery user can specify a query location in
  BigQuery Studio that overrides the administrator setting. To learn
  how to specify a default query location setting in BigQuery,
  see [Specify locations](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations).

- If a dataset's location cannot be determined, or if the user's default query
  location is unspecified,BigQuery attempts to determine the
  location of the dataset or query based on the [dry
  run](https://docs.cloud.google.com/bigquery/docs/running-queries#dry-run). For example:

  - SQL editor example: If your Gemini request for **Transform SQL with Gemini** references a dataset in `europe-west1`, then Gemini processes the data in the `EU` jurisdictional boundary.
  - Data canvas example: If your data canvas visualizes data from a dataset located in `us-east4`, any Gemini in BigQuery analyses or suggestions are processed in the `US` jurisdictional boundaries.

### Specify the default organization-level or project-level location

A BigQuery administrator can specify an organization-level or
project-level default location where Gemini requests are
processed. The default location is cached for the duration of the
user's session while they are editing within the current SQL editor tab. The
default location is used when Gemini in BigQuery
operations don't explicitly specify a location and a location cannot be inferred
from the request.

For more information on configuring the default location, see [Specify global
settings](https://docs.cloud.google.com/bigquery/docs/default-configuration#global-settings).

For more information on verifying the default location configuration, see
[Retrieve configuration settings](https://docs.cloud.google.com/bigquery/docs/default-configuration#retrieve-configuration).

## BigQuery Data Engineering Agent

The [Data Engineering Agent](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines)
supports jurisdiction-level regionalization that provides dedicated service
endpoints for the `us`, `eu`, and global regions. The regional preference is
automatically assigned based on the associated
Dataform workspace location.

When interacting with the agent in the Google Cloud console, all internal
processing---including the reasoning engine and temporary storage of conversation
context---is maintained strictly within the jurisdictional boundary defined by the
Dataform workspace region.

When interacting with the agent using the public API, select `us` or `eu` to
ensure that all processing, reasoning, and downstream service calls remain within that
jurisdiction. If the specified API region does not align with the workspace
region, the system returns an error.

To change your processing region, you must [create a new Dataform
repository](https://docs.cloud.google.com/dataform/docs/create-repository) and configure it to the updated
region.

## BigQuery data insights

To generate insights using [BigQuery data
insights](https://docs.cloud.google.com/bigquery/docs/data-insights), you can run data scan operations on
selected tables and dataset resources. These scans are created in the same
location as the BigQuery dataset resource. Within the `US` or
`EU` jurisdictions, Gemini in BigQuery processing
is restricted to the jurisdiction where the scan runs. Outside of the `US` and `EU`
jurisdictions, processing runs globally. To learn about where global
Gemini global data processing takes place, see
[Gemini serving locations](https://docs.cloud.google.com/gemini/docs/locations).

## BigQuery data preparation

The location where [BigQuery data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction)
processes data depends upon which data preparation feature you are using.

- For standalone data preparation, the Gemini in BigQuery processing location is the location where the BigQuery dataset is located.
- If you run data preparation as part of Dataform or BigQuery pipelines, then the Gemini in BigQuery data processing location is determined by the Dataform [`defaultLocation` setting](https://docs.cloud.google.com/dataform/docs/manage-repository#about-workflow-settings-yaml), if it's set. The `defaultLocation` setting also determines the BigQuery job location. This ensures that Gemini in BigQuery processing is done in the same jurisdictional boundaries.
- If `defaultLocation` for Dataform or the BigQuery pipeline that contains your data preparation is not set, then the Gemini in BigQuery processing region is determined by using the repository's [region setting](https://docs.cloud.google.com/dataform/docs/create-repository#repository-settings). A pipeline without a `defaultLocation` setting specified can run different BigQuery jobs in different locations based on the location of the tables used in pipeline nodes. As a best practice, you should set `defaultLocation` to ensure a consistent processing location.

## Limitations

The following limitations apply when you identify where Gemini in
BigQuery processes data:

- Gemini in BigQuery doesn't provide data residency for individual locations. Data processing can be specified for `US` and `EU` supported jurisdictions. Data outside these jurisdictions is processed globally.
- Gemini in BigQuery jurisdiction processing is only available for Gemini in BigQuery features that are generally available (GA). For a list of Gemini in BigQuery features, see [Overview of Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview).
- BigQuery Python notebook code assist and the Data Science
  Agent for Colab Enterprise in BigQuery only support
  global Gemini processing.

  > [!NOTE]
  > **Note:** To opt out of the Colab Enterprise preview feature without turning off other Gemini features, contact [vertex-notebooks-previews-external@google.com](mailto:vertex-notebooks-previews-external@google.com) or fill out the [Data Science Agent Public Preview Opt-out
  > form](https://forms.gle/KuTAunuLT2YmFAcs8). To learn more about how to turn off Data Science Agent, see [Turn off Gemini in
  > Colab Enterprise](https://docs.cloud.google.com/colab/docs/use-data-science-agent#turn-off).

- Gemini in Cloud Assist chat (GCA) only supports global
  Gemini processing. You can deny access to the GCA chat panel
  by removing the `geminicloudassist.agents.invoke` Identity and Access Management (IAM)
  permission for your users. To learn more about how to create custom roles,
  see [Create and manage custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles).

## What's next

- Read the [Gemini in BigQuery overview](https://docs.cloud.google.com/bigquery/docs/gemini-overview).
- Learn how to [set up Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).
- Learn how to [write queries with Gemini assistance](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini).
- Learn more about [Google Cloud compliance](https://cloud.google.com/security/compliance).
- Learn about [security, privacy, and compliance for Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-security-privacy-compliance).
- Learn more about [how Gemini for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).