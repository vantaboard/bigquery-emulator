# Querying nested and repeated fields in legacy SQL


This document details how to query nested and repeated data in legacy SQL query syntax.
The preferred query syntax for BigQuery is GoogleSQL. For information on
handling nested and repeated data in GoogleSQL, see the
[GoogleSQL migration guide](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql#differences_in_repeated_field_handling).

BigQuery supports [loading](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#loading_nested_and_repeated_json_data)
and [exporting](https://docs.cloud.google.com/bigquery/exporting-data-from-bigquery) nested and repeated data in the
form of JSON and Avro files. For many legacy SQL queries, BigQuery can automatically
flatten the data. For example, many `SELECT` statements can retrieve nested or repeated
fields while maintaining the structure of the data, and `WHERE` clauses can filter data
while maintaining its structure. Conversely, `ORDER BY` and `GROUP BY`
clauses implicitly flatten queried data. For circumstances where data is not implicitly flattened,
such as querying multiple repeated fields in legacy SQL, you can query your data using the
`FLATTEN` and `WITHIN` SQL functions.

<br />

### FLATTEN

When you query nested data, BigQuery automatically flattens the table data for you.
For example, let's take a look at a sample schema for person data:

```
   Last modified                 Schema                 Total Rows   Total Bytes   Expiration
 --- --- --- --- ---
  27 Sep 10:01:06   |- kind: string                     4            794
                    |- fullName: string (required)
                    |- age: integer
                    |- gender: string
                    +- phoneNumber: record
                    |  |- areaCode: integer
                    |  |- number: integer
                    +- children: record (repeated)
                    |  |- name: string
                    |  |- gender: string
                    |  |- age: integer
                    +- citiesLived: record (repeated)
                    |  |- place: string
                    |  +- yearsLived: integer (repeated)
```

Notice that there are several repeated and nested fields. If you run a legacy SQL query like the
following against the person table:

```
SELECT
  fullName AS name,
  age,
  gender,
  citiesLived.place,
  citiesLived.yearsLived
FROM [dataset.tableId]
```

BigQuery returns your data with a flattened output:

```
+---+---+---+---+---+
|     name      | age | gender | citiesLived_place | citiesLived_yearsLived |
+---+---+---+---+---+
| John Doe      |  22 | Male   | Seattle           |                   1995 |
| John Doe      |  22 | Male   | Stockholm         |                   2005 |
| Mike Jones    |  35 | Male   | Los Angeles       |                   1989 |
| Mike Jones    |  35 | Male   | Los Angeles       |                   1993 |
| Mike Jones    |  35 | Male   | Los Angeles       |                   1998 |
| Mike Jones    |  35 | Male   | Los Angeles       |                   2002 |
| Mike Jones    |  35 | Male   | Washington DC     |                   1990 |
| Mike Jones    |  35 | Male   | Washington DC     |                   1993 |
| Mike Jones    |  35 | Male   | Washington DC     |                   1998 |
| Mike Jones    |  35 | Male   | Washington DC     |                   2008 |
| Mike Jones    |  35 | Male   | Portland          |                   1993 |
| Mike Jones    |  35 | Male   | Portland          |                   1998 |
| Mike Jones    |  35 | Male   | Portland          |                   2003 |
| Mike Jones    |  35 | Male   | Portland          |                   2005 |
| Mike Jones    |  35 | Male   | Austin            |                   1973 |
| Mike Jones    |  35 | Male   | Austin            |                   1998 |
| Mike Jones    |  35 | Male   | Austin            |                   2001 |
| Mike Jones    |  35 | Male   | Austin            |                   2005 |
| Anna Karenina |  45 | Female | Stockholm         |                   1992 |
| Anna Karenina |  45 | Female | Stockholm         |                   1998 |
| Anna Karenina |  45 | Female | Stockholm         |                   2000 |
| Anna Karenina |  45 | Female | Stockholm         |                   2010 |
| Anna Karenina |  45 | Female | Moscow            |                   1998 |
| Anna Karenina |  45 | Female | Moscow            |                   2001 |
| Anna Karenina |  45 | Female | Moscow            |                   2005 |
| Anna Karenina |  45 | Female | Austin            |                   1995 |
| Anna Karenina |  45 | Female | Austin            |                   1999 |
+---+---+---+---+---+
```

In this example, `citiesLived.place` is now `citiesLived_place` and
`citiesLived.yearsLived` is now `citiesLived_yearsLived`.

Although BigQuery can automatically flatten nested fields, you may need to
explicitly call `FLATTEN` when dealing with more than one repeated field. For example,
if you try to run a legacy SQL query like the following:

```
SELECT fullName, age
FROM [dataset.tableId]
WHERE
  (citiesLived.yearsLived > 1995 ) AND
  (children.age > 3)
```

BigQuery returns an error similar to:

```
Cannot query the cross product of repeated fields children.age and citiesLived.yearsLived
```

To query across more than one repeated field, you need to flatten one of the fields:

```
SELECT
  fullName,
  age,
  gender,
  citiesLived.place
FROM (FLATTEN([dataset.tableId], children))
WHERE
  (citiesLived.yearsLived > 1995) AND
  (children.age > 3)
GROUP BY fullName, age, gender, citiesLived.place
```

Which returns:

```
+---+---+---+---+
|  fullName  | age | gender | citiesLived_place |
+---+---+---+---+
| John Doe   |  22 | Male   | Stockholm         |
| Mike Jones |  35 | Male   | Los Angeles       |
| Mike Jones |  35 | Male   | Washington DC     |
| Mike Jones |  35 | Male   | Portland          |
| Mike Jones |  35 | Male   | Austin            |
+---+---+---+---+
```

### WITHIN Clause

The `WITHIN` keyword specifically works with aggregate functions to aggregate across
children and repeated fields within records and nested fields. When you specify the `WITHIN`
keyword, you need to specify the scope over which you want to aggregate:

- `WITHIN RECORD`: Aggregates data in the repeated values within the record.
- `WITHIN *node_name*`: Aggregates data in the repeated values within the specified node, where a node is a parent node of the field in the aggregation function.

Suppose that you want to find the number of children each person in our previous example has. To
do so, you can count the number of children.name each record has:

```
SELECT
  fullName,
  COUNT(children.name) WITHIN RECORD AS numberOfChildren
FROM [dataset.tableId];
```

You get the following result:

```
+---+---+
|   fullName    | numberOfChildren |
+---+---+
| John Doe      |                2 |
| Jane Austen   |                2 |
| Mike Jones    |                3 |
| Anna Karenina |                0 |
+---+---+
```

To compare, try listing all of the children's names:

```
SELECT fullName, children.name
FROM [dataset.tableId]
```

```
+---+---+
|   fullName    | children_name |
+---+---+
| John Doe      | Jane          |
| John Doe      | John          |
| Jane Austen   | Josh          |
| Jane Austen   | Jim           |
| Mike Jones    | Earl          |
| Mike Jones    | Sam           |
| Mike Jones    | Kit           |
| Anna Karenina | None          |
+---+---+
```

This matches with our `WITHIN RECORD` query results; John Doe does have two children
named Jane and John, Jane Austen has two children named Josh and Jim, Mike Jones has three
children named Earl, Sam, and Kit, and Anna Karenina doesn't have any children.

Now, suppose that you want to find the number of times a person has lived in different places.
You can use the `WITHIN` clause to aggregate across one particular node:

```
SELECT
  fullName,
  COUNT(citiesLived.place) WITHIN RECORD AS numberOfPlacesLived,
  citiesLived.place,
  COUNT(citiesLived.yearsLived) WITHIN citiesLived AS numberOfTimesInEachCity,
FROM [dataset.tableId];
```

```
+---+---+---+---+
|   fullName    | numberOfPlacesLived | citiesLived_place | numberOfTimesInEachCity |
+---+---+---+---+
| John Doe      |                   2 | Seattle           |                       1 |
| John Doe      |                   2 | Stockholm         |                       1 |
| Mike Jones    |                   4 | Los Angeles       |                       4 |
| Mike Jones    |                   4 | Washington DC     |                       4 |
| Mike Jones    |                   4 | Portland          |                       4 |
| Mike Jones    |                   4 | Austin            |                       4 |
| Anna Karenina |                   3 | Stockholm         |                       4 |
| Anna Karenina |                   3 | Moscow            |                       3 |
| Anna Karenina |                   3 | Austin            |                       2 |
+---+---+---+---+
```

This query does the following:

- Performs a `WITHIN RECORD` on `citiesLived.place` and counts the number of places each person has lived in
- Performs a `WITHIN` on `citiesLived.yearsLived` and counts the number of times each person has lived in each city (counting just across `citiesLived`).

Using scoped aggregation over nested and repeated fields is one of BigQuery's most
powerful features, which can often eliminate expensive joins in queries.