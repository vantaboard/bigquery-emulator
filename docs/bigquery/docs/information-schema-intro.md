# Introduction to INFORMATION_SCHEMA

The BigQuery `INFORMATION_SCHEMA` views are read-only, system-defined
views that provide metadata information about your BigQuery
objects. The following table lists all `INFORMATION_SCHEMA` views that you can
query to retrieve metadata information:

| Resource type | INFORMATION_SCHEMA View |
|---|---|
| Access control | `https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges` |
| BI Engine | `https://docs.cloud.google.com/bigquery/docs/information-schema-bi-capacities` `https://docs.cloud.google.com/bigquery/docs/information-schema-bi-capacity-changes` |
| Configurations | `https://docs.cloud.google.com/bigquery/docs/information-schema-effective-project-options` `https://docs.cloud.google.com/bigquery/docs/information-schema-organization-options` `https://docs.cloud.google.com/bigquery/docs/information-schema-organization-options-changes` `https://docs.cloud.google.com/bigquery/docs/information-schema-project-options` `https://docs.cloud.google.com/bigquery/docs/information-schema-project-options-changes` |
| Datasets | `https://docs.cloud.google.com/bigquery/docs/information-schema-datasets-schemata https://docs.cloud.google.com/bigquery/docs/information-schema-datasets-schemata-links https://docs.cloud.google.com/bigquery/docs/information-schema-datasets-schemata-options https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage https://docs.cloud.google.com/bigquery/docs/information-schema-schemata-replicas https://docs.cloud.google.com/bigquery/docs/information-schema-schemata-replicas-by-failover-reservation ` |
| Graphs | `https://docs.cloud.google.com/bigquery/docs/information-schema-property-graphs` |
| Jobs | `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs` `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-by-user` `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-by-folder` `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-by-organization` |
| Jobs by timeslice | `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-timeline` `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-timeline-by-user` `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-timeline-by-folder` `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-timeline-by-organization` |
| Recommendations and insights | `https://docs.cloud.google.com/bigquery/docs/information-schema-insights` `https://docs.cloud.google.com/bigquery/docs/information-schema-recommendations` `https://docs.cloud.google.com/bigquery/docs/information-schema-recommendations-by-org` |
| Reservations | `https://docs.cloud.google.com/bigquery/docs/information-schema-assignments` `https://docs.cloud.google.com/bigquery/docs/information-schema-assignments-changes` `https://docs.cloud.google.com/bigquery/docs/information-schema-capacity-commitments` `https://docs.cloud.google.com/bigquery/docs/information-schema-capacity-commitment-changes` `https://docs.cloud.google.com/bigquery/docs/information-schema-reservations` `https://docs.cloud.google.com/bigquery/docs/information-schema-reservation-changes` `https://docs.cloud.google.com/bigquery/docs/information-schema-reservation-timeline` |
| Routines | `https://docs.cloud.google.com/bigquery/docs/information-schema-parameters` `https://docs.cloud.google.com/bigquery/docs/information-schema-routines` `https://docs.cloud.google.com/bigquery/docs/information-schema-routine-options` |
| Search indexes | `https://docs.cloud.google.com/bigquery/docs/information-schema-indexes` `https://docs.cloud.google.com/bigquery/docs/information-schema-index-columns` `https://docs.cloud.google.com/bigquery/docs/information-schema-index-column-options` `https://docs.cloud.google.com/bigquery/docs/information-schema-index-options` `https://docs.cloud.google.com/bigquery/docs/information-schema-indexes-by-organization` |
| Sessions | `https://docs.cloud.google.com/bigquery/docs/information-schema-sessions-by-project` `https://docs.cloud.google.com/bigquery/docs/information-schema-sessions-by-user` |
| Streaming | `https://docs.cloud.google.com/bigquery/docs/information-schema-streaming` `https://docs.cloud.google.com/bigquery/docs/information-schema-streaming-by-folder` `https://docs.cloud.google.com/bigquery/docs/information-schema-streaming-by-organization` |
| Tables | `https://docs.cloud.google.com/bigquery/docs/information-schema-columns` `https://docs.cloud.google.com/bigquery/docs/information-schema-column-field-paths` `https://docs.cloud.google.com/bigquery/docs/information-schema-constraint-column-usage` `https://docs.cloud.google.com/bigquery/docs/information-schema-key-column-usage` `https://docs.cloud.google.com/bigquery/docs/information-schema-partitions` `https://docs.cloud.google.com/bigquery/docs/information-schema-tables` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-options` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-constraints` `https://docs.cloud.google.com/bigquery/docs/information-schema-snapshots` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-by-folder` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-by-organization` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-usage` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-usage-by-folder` `https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-usage-by-organization` |
| Vector indexes | `https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes` `https://docs.cloud.google.com/bigquery/docs/information-schema-vector-index-columns` `https://docs.cloud.google.com/bigquery/docs/information-schema-vector-index-options` |
| Views | `https://docs.cloud.google.com/bigquery/docs/information-schema-views` `https://docs.cloud.google.com/bigquery/docs/information-schema-materialized-views` |
| Write API | `https://docs.cloud.google.com/bigquery/docs/information-schema-write-api` `https://docs.cloud.google.com/bigquery/docs/information-schema-write-api-by-folder` `https://docs.cloud.google.com/bigquery/docs/information-schema-write-api-by-organization` |

^†^ For `*BY_PROJECT` views, the `BY_PROJECT` suffix is optional. For
example, querying `INFORMATION_SCHEMA.JOBS_BY_PROJECT` and `INFORMATION_SCHEMA.JOBS`
return the same results.

> [!NOTE]
> **Note:** Not all `INFORMATION_SCHEMA` views are supported for [BigQuery Omni system tables](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations). You can view resource metadata with `INFORMATION_SCHEMA` for [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#view_resource_metadata) and [Azure Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table#view_resource_metadata_with_information_schema).

## Pricing

For projects that use on-demand pricing, queries against `INFORMATION_SCHEMA`
views incur a minimum of 10 MB of data processing charges, even if the bytes
processed by the query are less than 10 MB. 10 MB is the minimum
billing amount for on-demand queries. For more information, see
[On-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing).

For projects that use capacity-based pricing, queries against `INFORMATION_SCHEMA`
views and tables consume your purchased BigQuery slots. For more
information, see [capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

Because `INFORMATION_SCHEMA` queries are not cached, you are charged each time
that you run an `INFORMATION_SCHEMA` query, even if the query text is the same
each time you run it.

You are not charged storage fees for the `INFORMATION_SCHEMA` views.

## Syntax

An `INFORMATION_SCHEMA` view needs to be qualified with a dataset or region.

> [!NOTE]
> **Note:** You must [specify a location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations) to query an `INFORMATION_SCHEMA` view. Querying an `INFORMATION_SCHEMA` view fails with the following error if the location of the query execution doesn't match the location of the dataset or regional qualifier used:  
>
> ```
> Table myproject: region-us.INFORMATION_SCHEMA.[VIEW] not found in location US
> ```

### Dataset qualifier

When present, a dataset qualifier restricts results to the specified dataset.
For example:

    -- Returns metadata for tables in a single dataset.
    SELECT * FROM myDataset.INFORMATION_SCHEMA.TABLES;

The following `INFORMATION_SCHEMA` views support dataset qualifiers:

- `COLUMNS`
- `COLUMN_FIELD_PATHS`
- `MATERIALIZED_VIEWS`
- `PARAMETERS`
- `PARTITIONS`
- `ROUTINES`
- `ROUTINE_OPTIONS`
- `TABLES`
- `TABLE_OPTIONS`
- `VIEWS`

### Region qualifier

Region qualifiers are represented using a
`region-REGION` syntax.
Any [dataset location name](https://docs.cloud.google.com/bigquery/docs/locations) can be used for
`REGION`. For example, the following region qualifiers
are valid:

- `region-us`
- `region-asia-east2`
- `region-europe-north1`

When present, a region qualifier restricts results to the specified location.
[Region qualifiers](https://docs.cloud.google.com/bigquery/docs/locations#locations_and_regions) aren't
hierarchical, which means the EU multi-region does not include `europe-*`
regions nor does the US multi-region include the `us-*` regions. For example,
the following query returns metadata for all datasets in the `US` multi-region
for the project in which the query is executing, but doesn't include datasets in
the `us-west1` region:

```googlesql
-- Returns metadata for all datasets in the US multi-region.
SELECT * FROM region-us.INFORMATION_SCHEMA.SCHEMATA;
```

The following `INFORMATION_SCHEMA` views don't support region qualifiers:

- [`INFORMATION_SCHEMA.PARTITIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-partitions#scope_and_syntax)
- [`INFORMATION_SCHEMA.SEARCH_INDEXES`](https://docs.cloud.google.com/bigquery/docs/information-schema-indexes#scope_and_syntax)
- [`INFORMATION_SCHEMA.SEARCH_INDEX_COLUMNS`](https://docs.cloud.google.com/bigquery/docs/information-schema-index-columns)
- [`INFORMATION_SCHEMA.SEARCH_INDEX_OPTIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-index-options)

If neither a region qualifier nor a dataset qualifier is specified, you will
receive an error.

Queries against a region-qualified `INFORMATION_SCHEMA` view run in the region that you specify, which means that you can't write a single query to join data from views in different regions. To combine `INFORMATION_SCHEMA` views from multiple regions, read and combine the query results locally, or [copy](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy_tables_across_regions) the resulting tables to a common region.

### Project qualifier

When present, a project qualifier restricts results to the specified project.
For example:

    -- Returns metadata for the specified project and region.
    SELECT * FROM myProject.`region-us`.INFORMATION_SCHEMA.TABLES;

    -- Returns metadata for the specified project and dataset.
    SELECT * FROM myProject.myDataset.INFORMATION_SCHEMA.TABLES;

All `INFORMATION_SCHEMA` views support project qualifiers. If a project
qualifier is not specified, the view will default to the
project in which the query is executing.

Specifying a project qualifier for organization-level views
(for example, `STREAMING_TIMELINE_BY_ORGANIZATION`)
has no impact on the results.

## Limitations

- BigQuery `INFORMATION_SCHEMA` queries must be in GoogleSQL syntax. `INFORMATION_SCHEMA` does not support legacy SQL.
- `INFORMATION_SCHEMA` query results are not cached.
- `INFORMATION_SCHEMA` views cannot be used in DDL statements.
- `INFORMATION_SCHEMA` views don't contain information about [hidden datasets](https://docs.cloud.google.com/bigquery/docs/datasets#hidden_datasets).
- `INFORMATION_SCHEMA` queries with region qualifiers might include metadata from resources in that region from [deleted datasets that are within your time travel window](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets).
- When you list resources from an `INFORMATION_SCHEMA` view, the permissions are checked only at the parent level, not at an individual row level. Therefore, any [deny policy](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#deny_access_to_a_resource) ([preview](https://cloud.google.com/products#product-launch-stages)) that conditionally targets an individual row using tags is ignored.