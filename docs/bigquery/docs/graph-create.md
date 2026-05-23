# Create and query a graph

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

This document shows you how to use
[BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview) to create a
graph with financial information and run
graph queries using the
[Graph Query Language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-intro) (GQL).

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

## Create node and edge tables

Graphs are built from existing BigQuery tables and stored in
datasets.
To store the tables and graph that you create in the following examples,
[create a dataset](https://docs.cloud.google.com/bigquery/docs/datasets).
The following query creates a dataset called `graph_db`:

    CREATE SCHEMA IF NOT EXISTS graph_db;

The following tables contain information about people and accounts,
and the relationships between each of these entities:

- `Person`: information about people.
- `Account`: information about bank accounts.
- `PersonOwnAccount`: information about who owns which accounts.
- `AccountTransferAccount`: information about transfers between accounts.

To create these tables, run the following
[`CREATE TABLE` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement):

    CREATE OR REPLACE TABLE graph_db.Person (
      id               INT64,
      name             STRING,
      birthday         TIMESTAMP,
      country          STRING,
      city             STRING,
      PRIMARY KEY (id) NOT ENFORCED
    );

    CREATE OR REPLACE TABLE graph_db.Account (
      id               INT64,
      create_time      TIMESTAMP,
      is_blocked       BOOL,
      nick_name        STRING,
      PRIMARY KEY (id) NOT ENFORCED
    );

    CREATE OR REPLACE TABLE graph_db.PersonOwnAccount (
      id               INT64 NOT NULL,
      account_id       INT64 NOT NULL,
      create_time      TIMESTAMP,
      PRIMARY KEY (id, account_id) NOT ENFORCED,
      FOREIGN KEY (id) REFERENCES graph_db.Person(id) NOT ENFORCED,
      FOREIGN KEY (account_id) REFERENCES graph_db.Account(id) NOT ENFORCED
    );

    CREATE OR REPLACE TABLE graph_db.AccountTransferAccount (
      id               INT64 NOT NULL,
      to_id            INT64 NOT NULL,
      amount           FLOAT64,
      create_time      TIMESTAMP NOT NULL,
      order_number     STRING,
      PRIMARY KEY (id, to_id, create_time) NOT ENFORCED,
      FOREIGN KEY (id) REFERENCES graph_db.Account(id) NOT ENFORCED,
      FOREIGN KEY (to_id) REFERENCES graph_db.Account(id) NOT ENFORCED
    );

## Create a graph

To create a graph, you can use the
[`CREATE PROPERTY GRAPH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_create_graph)
or the [visual graph modeler](https://docs.cloud.google.com/bigquery/docs/graph-modeler).
The following example uses the `CREATE PROPERTY GRAPH` statement to create a
graph called `FinGraph` in the `graph_db` dataset. The `Account` and `Person`
tables are the node tables. The `AccountTransferAccount` and `PersonOwnAccount`
tables are the edge tables, which represent relationships between the node tables.

    CREATE OR REPLACE PROPERTY GRAPH graph_db.FinGraph
      NODE TABLES (
        graph_db.Account,
        graph_db.Person
      )
      EDGE TABLES (
        graph_db.PersonOwnAccount
          SOURCE KEY (id) REFERENCES Person (id)
          DESTINATION KEY (account_id) REFERENCES Account (id)
          LABEL Owns,
        graph_db.AccountTransferAccount
          SOURCE KEY (id) REFERENCES Account (id)
          DESTINATION KEY (to_id) REFERENCES Account (id)
          LABEL Transfers
      );

## Insert data

To update the data in a graph, you update the data in your node and edge tables.
When you create a graph, your data isn't moved or copied. Instead, a
graph acts as a logical view of the data that exists in your node and edge
tables. Your graph queries return results based on the data that exists in
your node and edge tables at the time you run the query.

The following query inserts data into the tables that you created:

    INSERT INTO graph_db.Account
      (id, create_time, is_blocked, nick_name)
    VALUES
      (7,"2020-01-10 06:22:20.222",false,"Vacation Fund"),
      (16,"2020-01-27 17:55:09.206",true,"Vacation Fund"),
      (20,"2020-02-18 05:44:20.655",false,"Rainy Day Fund");

    INSERT INTO graph_db.Person
      (id, name, birthday, country, city)
    VALUES
      (1,"Alex","1991-12-21 00:00:00","Australia","Adelaide"),
      (2,"Dana","1980-10-31 00:00:00","Czech_Republic","Moravia"),
      (3,"Lee","1986-12-07 00:00:00","India","Kollam");

    INSERT INTO graph_db.AccountTransferAccount
      (id, to_id, amount, create_time, order_number)
    VALUES
      (7,16,300,"2020-08-29 15:28:58.647","304330008004315"),
      (7,16,100,"2020-10-04 16:55:05.342","304120005529714"),
      (16,20,300,"2020-09-25 02:36:14.926","103650009791820"),
      (20,7,500,"2020-10-04 16:55:05.342","304120005529714"),
      (20,16,200,"2020-10-17 03:59:40.247","302290001255747");

    INSERT INTO graph_db.PersonOwnAccount
      (id, account_id, create_time)
    VALUES
      (1,7,"2020-01-10 06:22:20.222"),
      (2,20,"2020-01-27 17:55:09.206"),
      (3,16,"2020-02-18 05:44:20.655");

![Visualization of financial graph example](https://docs.cloud.google.com/static/bigquery/images/fingraph-example.png)

## Query a graph

To query a graph, you run queries that use the
[Graph Query Language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-intro).

The following query uses a
[`MATCH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_match)
to find information about who Dana transferred money to:

    GRAPH graph_db.FinGraph
    MATCH
      (person:Person {name: "Dana"})-[own:Owns]->
      (account:Account)-[transfer:Transfers]->(account2:Account)<-[own2:Owns]-(person2:Person)
    RETURN
      person.name AS owner,
      transfer.amount AS amount,
      person2.name AS transferred_to
    ORDER BY person2.name;

The results look similar to the following:

```
+---+---+---+
| owner | amount | transferred_to |
+---+---+---+
| Dana  | 500.0  | Alex           |
| Dana  | 200.0  | Lee            |
+---+---+---+
```

You can also use graphs as a data source in
[conversational analytics](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#graphs),
which lets you ask natural language questions about your graphs. For example,
you might ask "Who has Dana transferred money to and how much was transferred?"

## Visualize graph query results

You can
[visualize your graph query results](https://docs.cloud.google.com/bigquery/docs/graph-visualization#visualization-results)
in a notebook by using the `%%bigquery --graph` magic command followed by your
GQL query. The query must return graph elements in the JSON format. To visualize
the results of the query you ran in the previous section, run the following
query in a notebook code cell:

    %%bigquery --graph
    GRAPH graph_db.FinGraph
    MATCH
      p = ((person:Person {name: "Dana"})-[own:Owns]->
      (account:Account)-[transfer:Transfers]->(acount2:Account)<-[own2:Owns]-(person2:Person))
    RETURN
      TO_JSON(p) AS path;

![Visualization of Dana's transfers in a notebook](https://docs.cloud.google.com/static/bigquery/images/graph-transfer-visualization.png)

## Delete a graph

To delete a graph, use the [`DROP PROPERTY GRAPH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_drop_graph). Deleting a graph has no effect
on the tables that were used to define the graph nodes and edges. The following
query deletes the `FinGraph` graph:

    DROP PROPERTY GRAPH graph_db.FinGraph;

> [!WARNING]
> **Warning:** BigQuery doesn't prevent you from deleting or modifying the schema of tables that a graph depends on. Deleting or modifying node or edge tables without updating your graph schema can cause your graph queries to fail.

## What's next

- See the [introduction to BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview).
- Learn more about [graph schemas](https://docs.cloud.google.com/bigquery/docs/graph-schema-overview).
- Learn more about how to [write graph queries](https://docs.cloud.google.com/bigquery/docs/graph-query-overview).
- Learn more about the [Graph Query Language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-intro) (GQL).
- For a tutorial about fraud detection, see [Fraud Detection with BigQuery Graph](https://codelabs.developers.google.com/codelabs/fraud-bigquery-graph).
- For a tutorial about customer profiles, see [Build customer 360 recommendations with BigQuery Graph](https://codelabs.developers.google.com/codelabs/c360-bigquery-graph).
- For a tutorial about supply chains, see [Supply chain traceability with BigQuery Graph](https://codelabs.developers.google.com/codelabs/supplychaingraph).