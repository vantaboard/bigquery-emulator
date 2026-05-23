# Use the BigQuery Graph visual modeler

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

To request support or provide feedback for this feature, send an email to
[bq-graph-preview-support@google.com](mailto:bq-graph-preview-support@google.com).

This document describes how to use the visual graph modeler in
BigQuery Studio to define nodes and edges from your
BigQuery tables, and how to edit and query graph data. You can also
create a graph by using the
[`CREATE PROPERTY GRAPH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_create_graph).

## Required roles


To get the permissions that
you need to create and query graphs,

ask your administrator to grant you the
[BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) IAM role on the dataset in which you create the node tables, edge tables, and graph.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create a graph

To create a graph using the visual modeler, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. To open the gallery, in the editor tab bar, click the

   **arrow** next to

   **SQL query** , and then click **Graph**.

3. In the **Graph name** field, enter a name for the graph.

4. In the **Graph destination in dataset** field, do one of the following:

   - Click **Create new dataset** to [create a new
     dataset](https://docs.cloud.google.com/bigquery/docs/datasets#create-dataset).
   - Select an existing dataset.
5. Select the tables that you want to use as your data sources.

6. Click **Create**. BigQuery creates the graph resource and
   navigates you to the visual modeler.

## View a graph

To view an existing graph, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to view.

## Edit a graph

After you create a graph, you can use the visual modeler to add, edit, or remove
nodes and edges. The following sections describe how to edit an existing graph.

### Manage nodes

This section describes how to manage graph nodes.

#### Add a node

To add a node, follow these steps. Changes that you make aren't applied until you
[publish your changes](https://docs.cloud.google.com/bigquery/docs/graph-modeler#publish-graph-changes).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to edit.

7. In the graph action bar, click **New node**.

8. In the **Add node** panel, click **Add table**.

9. In the **Select table** pane, select the dataset table that you want to add, and then click **Add**.

10. Click **Edit primary key**.

11. Select the fields that you want to use as the primary keys, and then click **Add**.

12. Optional: Click **Add label**.

    1. In the **Name** field, enter a name for your label.
    2. In the **Description** field, enter a description of your label.
    3. In the **Synonyms** field, enter up to eight synonyms.
    4. Click **Add property**.
    5. In the **Select fields** pane, select all the properties that you want to add to the label.

       Optional: You can add a custom field with a measure. For more information, see [Graph measures](https://docs.cloud.google.com/bigquery/docs/graph-measures).
    6. Click **Add**.

    7. Click **Done**.

13. Click **Done**.

#### Edit a node

To edit a node, follow these steps. Changes that you make aren't applied until you
[publish your changes](https://docs.cloud.google.com/bigquery/docs/graph-modeler#publish-graph-changes).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to edit.

7. In the graph, select the node that you want to edit.

8. In the **Node window** , click **Edit**.

9. In the **Edit node** panel, click **Edit primary key**.

10. Select the fields that you want to use as the primary keys, and then click **Add**.

11. Optional: Click **Add label**.

    1. In the **Name** field, enter a name for your label.
    2. In the **Description** field, enter a description of your label.
    3. In the **Synonyms** field, enter up to eight synonyms.
    4. Click **Add property**.
    5. In the **Select fields** pane, select all the properties that you want to add to the label.
    6. Click **Add**.
    7. Optional: Click **Add measure** to add a custom field with a measure, as described in [Graph measures](https://docs.cloud.google.com/bigquery/docs/graph-measures).
    8. Click **Done**.
12. Click **Done**.

#### Remove a node

To remove a node, follow these steps. Changes that you make aren't applied until you
[publish your changes](https://docs.cloud.google.com/bigquery/docs/graph-modeler#publish-graph-changes).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to edit.

7. In the graph, select the node to remove.

8. In the **Node window** , click **Remove**.

9. Click **Delete**.

### Manage edges

This section describes how to manage graph edges.

#### Add an edge

To add an edge, follow these steps. Changes that you make aren't applied until you
[publish your changes](https://docs.cloud.google.com/bigquery/docs/graph-modeler#publish-graph-changes).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to edit.

7. In the graph action bar, click **New edge**.

8. In the **Add edge** panel, click **Add table**.

9. In the **Select table** pane, select the table that you want to use as a data source, and then click **Add**.

10. In the **Source node** list, select the node that you want to use as the source.

11. Click **Add referenced columns in the node**.

    1. Select the fields that you want to reference.
    2. Click **Add**.
12. In the **Target node** list, select the node that you want to use as the target.

13. Click **Add referenced columns in the node**.

    1. Select the fields that you want to reference.
    2. Click **Add**.
14. Click **Add referenced columns in the edge**.

    1. Select the fields that you want to reference. The number of columns and data types of these edge columns must match the referenced columns selected for the source and target nodes.
    2. Click **Add**.
15. Optional: Click **Add label**.

    1. In the **Name** field, enter a name for your label.
    2. In the **Description** field, enter a description of your label.
    3. In the **Synonyms** field, enter up to eight synonyms.
    4. Click **Add property**.
    5. In the **Select fields** pane, select all the properties that you want to add to the label.
    6. Click **Add**.
    7. Optional: Click **Add measure** to add a custom field with a measure, as described in [Graph measures](https://docs.cloud.google.com/bigquery/docs/graph-measures).
    8. Click **Done**.
16. Click **Done**.

#### Edit an edge

To edit an edge, follow these steps. Changes that you make aren't applied until you
[publish your changes](https://docs.cloud.google.com/bigquery/docs/graph-modeler#publish-graph-changes).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to edit.

7. In the graph, select the edge that you want to edit.

8. In the **Node window** , click **Edit**.

9. In the **Edit edge** panel, in the **Source node** list, select the node that you want to use as the source.

10. Click **Add referenced columns in the node**.

    1. Select the fields that you want to reference.
    2. Click **Add**.
11. In the **Target node** list, select the node that you want to use as the target.

12. Click **Add referenced columns in the node**.

    1. Select the fields that you want to reference.
    2. Click **Add**.
13. Click **Add referenced columns in the edge**.

    1. Select the fields that you want to reference. The number of columns and data types of these edge columns must match the referenced columns selected for the source and target nodes.
    2. Click **Add**.
14. Optional: Click **Add label**.

    1. In the **Name** field, enter a name for your label.
    2. In the **Description** field, enter a description of your label.
    3. In the **Synonyms** field, enter up to eight synonyms.
    4. Click **Add property**.
    5. In the **Select fields** pane, select all the properties that you want to add to the label.
    6. Click **Add**.
    7. Optional: Click **Add measure** to add a custom field with a measure, as described in [Graph measures](https://docs.cloud.google.com/bigquery/docs/graph-measures).
    8. Click **Done**.
15. Click **Done**.

#### Remove an edge

To remove an edge, follow these steps. Changes that you make aren't applied until you
[publish your changes](https://docs.cloud.google.com/bigquery/docs/graph-modeler#publish-graph-changes).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to edit.

7. In the graph, select the edge to remove.

8. In the **Node window** , click **Remove**.

9. Confirm the deletion.

## Publish graph changes

To save your modifications, publish them to the graph resource. Follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation pane, click **Explorer**:

3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your graph.

5. Click the **Graphs** tab.

6. Select the graph that you want to edit.

7. After you finish editing, click **Publish**.

8. Review the `CREATE PROPERTY GRAPH`statement in the **Publish graph** pane,
   and then click **Publish**.

## What's next

- Learn more about [BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview).
- Learn how to [create and query a graph](https://docs.cloud.google.com/bigquery/docs/graph-create).
- Learn how to [visualize graphs](https://docs.cloud.google.com/bigquery/docs/graph-visualization).