# PROPERTY_GRAPHS view

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
> **Note:** To request support or provide feedback for this feature, send email to [bq-graph-preview-support@google.com](mailto:bq-graph-preview-support@google.com).

The `INFORMATION_SCHEMA.PROPERTY_GRAPHS` view contains one row for each
[property graph](https://docs.cloud.google.com/bigquery/docs/graph-overview) in the dataset or region that
you specify.

## Required permissions

To query the `INFORMATION_SCHEMA.PROPERTY_GRAPHS` view, ask your administrator
to grant you one of the following predefined IAM roles:

- `roles/bigquery.metadataViewer`
- `roles/bigquery.dataViewer`
- `roles/bigquery.admin`

## Schema

When you query the `INFORMATION_SCHEMA.PROPERTY_GRAPHS` view, the query results
contain one row for each property graph in the resource scope that you
specify.

The `INFORMATION_SCHEMA.PROPERTY_GRAPHS` view has the following schema:

| Column name | Data type | Value |
|---|---|---|
| `property_graph_catalog` | `STRING` | The name of the project that contains the dataset where the property graph is defined. |
| `property_graph_schema` | `STRING` | The name of the dataset that contains the property graph. |
| `property_graph_name` | `STRING` | The name of the property graph. |
| `property_graph_metadata_json` | `STRING` | The JSON representation of the property graph definition. This representation contains information about the graph, such as its nodes, edges, labels, properties, creation timestamp, and modification timestamps. |
| `ddl` | `STRING` | A DDL statement that can be used to create the property graph. |

## Scope and syntax

Queries against this view must include a dataset or a region qualifier. For
queries with a dataset qualifier, you must have permissions for the dataset.
For queries with a region qualifier, you must have permissions for the project.
For more
information see [Syntax](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#syntax).
The following table explains the region and resource scopes for this view:

| View name | Resource scope | Region scope |
|---|---|---|
| ``[PROJECT_ID.]`region-REGION`.INFORMATION_SCHEMA.PROPERTY_GRAPHS`` | Project level | `REGION` |
| `[PROJECT_ID.]DATASET_ID.INFORMATION_SCHEMA.PROPERTY_GRAPHS` | Dataset level | Dataset location |

Replace the following:

- Optional: `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.
- `REGION`: any [dataset region name](https://docs.cloud.google.com/bigquery/docs/locations). For example, `` `region-us` ``.
- `DATASET_ID`: the ID of your dataset. For more information, see [Dataset qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#dataset_qualifier).

  <br />

  <br />

  > [!NOTE]
  > **Note:** You must use [a region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier) to query `INFORMATION_SCHEMA` views. The location of the query execution must match the region of the `INFORMATION_SCHEMA` view.

<br />

## Example

The following example retrieves the `property_graph_name`,
`property_graph_metadata_json`, and `ddl` columns from the
`INFORMATION_SCHEMA.PROPERTY_GRAPHS` view:

```googlesql
SELECT
  property_graph_name, property_graph_metadata_json, ddl
FROM
  `region-REGION`.INFORMATION_SCHEMA.PROPERTY_GRAPHS;
```

> [!NOTE]
> **Note:** `INFORMATION_SCHEMA` view names are case-sensitive.

The result is similar to the following:

```
+---+---+---+
| property_graph_name | property_graph_metadata_json                   | ddl                                                  |
+---+---+---+
| FinGraph            | {"creationTime":"2026-01-05T22:22:22.365394Z", | CREATE PROPERTY GRAPH `my_project.graph_db.FinGraph` |
|                     | "edgeTables":[{"dataSourceTable":{"datasetId": | NODE TABLES (`my_project.graph_db.Account` AS        |
|                     | "graph_db","projectId":"my_project","tableId   | Account KEY (id)                                     |
|                     | ...                                            | ...                                                  |
+---+---+---+
```