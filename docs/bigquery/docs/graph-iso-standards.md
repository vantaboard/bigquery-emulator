# BigQuery Graph and ISO standards

The document describes how BigQuery Graph supports the ISO international
standard query language for graph databases.

BigQuery Graph is based on two ISO standards:

- [ISO/IEC 9075-16:2023 - Information technology --- Database languages SQL Property Graph Queries (SQL/PGQ)](https://www.iso.org/standard/79473.html), Edition 1, 2023
- [ISO/IEC 39075:2024 - Information technology --- Database languages --- GQL](https://www.iso.org/standard/76120.html), Edition 1, 2024

The following tables describe the high-level relationship between SQL/PGQ, GQL,
and how BigQuery Graph supports these standards.

| Standard |   | SQL/PGQ | GQL | BigQuery Graph |
|---|---|---|---|---|
| Query | Graph pattern matching capabilities | Shares the core Graph Pattern Matching Language (GPML) functionalities with GQL. | Shares the core GPML functionalities with SQL/PGQ. | Both standards are supported. For more information, see BigQuery Graph [GQL patterns](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns). |
| Query | Other query language features (for example, `LIMIT`, `ORDER`, aggregation) | SQL-based. | Similar to SQL, but the GQL query features are linearly composable graph query statements. | Both standards are supported. For more information, see BigQuery Graph [GQL query statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements) and [Query syntax in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax). |
| Query | Graph and table interoperability | Supported. | Not supported. | Both standards are supported. For more information, see [`GRAPH_TABLE` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-queries#graph_table_operator). |
| Types |   | Data types, functions and expressions in SQL/PGQ and GQL are similar. | Data types, functions and expressions in SQL/PGQ and GQL are similar. | Supports most data types and expressions in SQL/PGQ and GQL. For more information, see [Data types in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types). |
| DML |   | SQL/PGQ inherits DML from SQL. | Graph-based DML is supported. | Supports SQL table-based DML. For more information, see the [GoogleSQL data manipulation language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax). |
| Schema |   | Supports using `CREATE PROPERTY GRAPH` from tables. | Supports using `CREATE PROPERTY GRAPH` with open types and closed types. | Supports the SQL/PGQ method. For more information, see the [`CREATE PROPERTY GRAPH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_create_graph) definition. |

## SQL/PGQ support

> [!NOTE]
> **Note:** G000 is the feature ID from the ISO SQL/PGQ standard.

| Standard | SQL/PGQ feature ID | BigQuery Graph |
|---|---|---|
| Query (Graph and table interoperability) | Feature G900: `GRAPH_TABLE` | Supported. For more information, see [`GRAPH_TABLE`](https://docs.cloud.google.com/bigquery/docs/graph-query-overview#query-graphs-and-tables-together) operator. |
| Schema | Feature G924: Explicit key clause for element tables. This implies a claim of conformance to Feature G920: DDL-based SQL-property graphs. | Supported. For more information, see [`CREATE_PROPERTY_GRAPH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_create_graph). |
| Schema | Feature G925: Explicit label and properties clause for element tables. This implies a claim of conformance to Feature G920: DDL-based SQL-property graphs. | Supported. For more information, see [`CREATE_PROPERTY_GRAPH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_create_graph). |
| Query (GPML) | Feature G001: Repeatable-elements match mode. | Supported. Repeatable elements match mode is the default semantic. Explicit repeatable elements match mode clause syntax is not supported. |
| Query (GPML) | Feature G008: Graph pattern `WHERE` clause. This implies a claim of conformance to Feature G000: Graph pattern. | Supported. For more information, see [Graph pattern](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#graph_pattern_definition). |
| Query (GPML) | Feature G034: Path concatenation. | Supported. For more information, see see [Graph pattern](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#graph_pattern_definition). |
| Query (GPML) | Feature G040: Vertex pattern. | Supported. For more information, see [Element pattern](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#element_pattern_definition). |
| Query (GPML) | Feature G042: Basic full edge patterns. | Supported. For more information, see [Element pattern](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#element_pattern_definition). |
| Query (GPML) | Feature G070: Label expression: label disjunction. | Supported. For more information, see [Label expression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#label_expression_definition). |
| Query (GPML) | Feature G073: Label expression: individual label name. | Supported. For more information, see [Label expression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#label_expression_definition). |
| Query (GPML) | Feature G090: Property reference. | Supported. |

## GQL support

> [!NOTE]
> **Note:** GG00 is the feature ID from the ISO GQL standard.

| Standard | GQL feature ID | BigQuery Graph |
|---|---|---|
| Schema | Feature GG02: Graph with a closed graph type. Conformance with at least one of GG20, GG21, GG22, or GG23: - Feature GG20: Explicit element type names. <!-- --> - Feature GG21: Explicit element type key label sets. <!-- --> - Feature GG22: Element type key label set inference. <!-- --> - Feature GG22: Element type key label set inference. <!-- --> - Feature GG23 Optional element type key label sets. | Supported. GQL support can be chosen from GG01: Graph with an open type or GG02. BigQuery Graph doesn't support the exact same `CREATE_GRAPH_TYPE` statement as GQL. However, the [`CREATE_PROPERTY_GRAPH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#gql_create_graph) supported by BigQuery Graph is closely related to GG02 (with similar support for GG20, GG21, GG22, and GG23). |
| Lexical structure | "A claim of conformance to a specific version of TheUnicode® Standard and the synchronous versions of Unicode Technical Standard #10, Unicode Standard Annex #15, and Unicode Standard Annex #31. The claimed version of The Unicode® Standard shall not be less than 13.0.0." | BigQuery Graph GQL shares the exact [lexical structure](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical) with GoogleSQL. For reference to unicode escape values, see [Escape sequences for string and bytes literals](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#escape_sequences). |
| Data types | "A claim of conformance to the set of all value types that are supported as the types of property values. At minimum, this set shall include: - The character string type specified by `STRING` or `VARCHAR`. <!-- --> - The boolean type specified by `BOOLEAN` or `BOOL`. <!-- --> - The signed regular integer type specified by `SIGNED INTEGER`, `INTEGER`, or `INT`. <!-- --> - The approximate numeric type specified by `FLOAT`." | Supported. For more information, see [a full list of data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types) supported by BigQuery Graph GQL. |

## Additional features

The features listed in the previous sections are the minimal conformance
features of the standards. BigQuery Graph supports additional features
in the ISO standards. To learn more, see
[BigQuery Graph schema overview](https://docs.cloud.google.com/bigquery/docs/graph-schema-overview)
and [GQL overview](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-intro).