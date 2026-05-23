# Map SQL object names for batch translation

> [!WARNING]
>
> **Preview**
>
>
> This product is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** Object name mapping using JSON is only supported by the legacy batch API. If you are using the [BigQuery Migration API](https://docs.cloud.google.com/bigquery/docs/api-sql-translator) or starting batch jobs from the Google Cloud console, use [YAML-based object name mapping](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#output_name_mapping) instead.

This document describes how to configure *name mapping* to rename SQL
objects during [batch translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator).

## Overview

Name mapping lets you identify the names of SQL objects in your
source files, and specify target names for those objects in
BigQuery. You can use some or all of the following components
to configure name mapping for an object:

- A name mapping rule, composed of:
  - Source [name parts](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#name_parts) that provide the fully qualified name of the object in the source system.
  - A [type](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#object_types) that identifies the source object's type.
  - Target name parts that provide the name of the object in BigQuery.
- A [default database](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#default_database) name to use with any source objects that don't specify one.
- A [default schema](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#default_schema) name to use with any source objects that don't specify one.

### Name parts

You provide the values for the source and target object names in a name mapping
rule by using a combination of the following name parts:

- **Database** : The top level of the naming hierarchy. Your source platform might use an alternative term for this, for example *project*.
- **Schema** : The second level of the naming hierarchy. Your source platform might use an alternative term for this, for example *dataset*.
- **Relation** : The third level of the naming hierarchy. Your source platform might use an alternative term for this, for example *table*.
- **Attribute** : The lowest level of the naming hierarchy. Your source platform might use an alternative term for this, for example *column*.

### Object types

You must also specify the type of source object you are renaming in a
name mapping rule. The following object types are supported:

- `Database`: A top-level object in the object hierarchy, for example **`database`** `.schema.relation.attribute`. Your source platform might use an alternative term for this, for example *project* . Specifying `database` as the object type changes all references to the source string in both DDL and DML statements.
- `Schema`: A second-level object in the object hierarchy. Your source platform might use an alternative term for this, for example *dataset* . Specifying `schema` as the object type changes all references to the source string in both DDL and DML statements.
- `Relation`: A third-level object in the object hierarchy. Your source platform might use an alternative term for this, for example *table* . Specifying `relation` as the object type changes all references to the source string in DDL statements.
- `Relation alias`: An alias for a third-level object. For example, in the query `SELECT t.field1, t.field2 FROM myTable t;`, `t` is a relation alias. In the query `SELECT field1, field2 FROM schema1.table1`, `table1` is also a relation alias. Specifying `relation alias` as the object type creates aliases for all references to the source string in DML statements. For example, if `tableA` is specified as the target name, the preceding examples are translated as `SELECT tableA.field1, tableA.field2 FROM myTable AS tableA;` and `SELECT tableA.field1, tableA.field2 FROM schema1.table1 AS tableA`, respectively.
- `Function`: A procedure, for example `create procedure db.test.function1(a int)`. Specifying `function` as the object type changes all references to the source string in both DDL and DML statements.
- `Attribute`: A fourth-level object in the object hierarchy. Your source platform might use an alternative term for this, for example *column* . Specifying `attribute` as the object type changes all references to the source string in DDL statements.
- `Attribute alias`: An alias for a fourth-level object. For example, in the query `SELECT field1 FROM myTable;`, `field1` is an attribute alias. Specifying `attribute alias` as the object type changes all references to the source string in DML statements.

#### Required name parts for object types

To describe an object in a name mapping rule, use the name parts identified for
each object type in the following table:

| **Type** | **Source object name** |||| **Target object name** ||||
|---|---|---|---|---|---|---|---|---|
|   | **Database name part** | **Schema name part** | **Relation name part** | **Attribute name part** | **Database name part** | **Schema name part** | **Relation name part** | **Attribute name part** |
| `Database` | X |   |   |   | X |   |   |   |
| `Schema` | X | X |   |   | X | X |   |   |
| `Relation` | X | X | X |   | X | X | X |   |
| `Function` | X | X | X |   | X | X | X |   |
| `Attribute` | X | X | X | X |   |   |   | X |
| `Attribute alias` | X | X | X | X |   |   |   | X |
| `Relation alias` |   |   | X |   |   |   | X |   |

### Default database

If you want to append a BigQuery project name to all translated
objects, the easiest thing to do is to specify a default database name when
you
[create a translation job](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#submit_a_translation_job).
This works for source files where
three-part naming is used, or where four-part naming is used but the highest
level object name isn't specified.

For example, if you specify the default database name `myproject`, then a
source statement like `SELECT * FROM database.table` is translated to
`SELECT * FROM myproject.database.table`. If you have objects that already use a
database name part, like `SELECT * FROM database.schema.table`, then you have
to use a name mapping rule to rename `database.schema.table` to
`myproject.schema.table`.

### Default schema

If you want to fully qualify all object names in the source files that don't
use four-part naming, you can provide both a
[default database name](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#default_database) and a default schema name when
you
[create a translation job](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#submit_a_translation_job).
The default schema name is provided as the first schema name in the schema
search path option.

For example, if you specify the default database name `myproject` and the
default schema name `myschema`, then the following source statements:

- `SELECT * FROM database.table`
- `SELECT * FROM table1`

Are translated to:

- `SELECT * FROM myproject.database.table`.
- `SELECT * FROM myproject.myschema.table1`

## Name mapping rule behavior

The following sections describe how name mapping rules behave.

### Rule inheritance flows down the object hierarchy

A name change that affects a higher-level object affects the target object, and
also all of its child objects in the same hierarchy.

For example, if you specify the following name mapping rule with an object
type of `schema`:

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `sales_db` | `sales` |
| Schema | `cust_mgmt` | `cms` |
| Relation |   |   |
| Attribute |   |   |

When it is applied, the database and schema name parts of all `relation` and
`attribute` objects under the `sales_db.cust_mgmt` schema are also changed. For
instance, a `relation` object named `sales_db.cust_mgmt.history` becomes
`sales.cms.history`.

Conversely, name changes that target lower-level objects don't affect
higher- or same-level objects in the object hierarchy.

For example, if you specify the following name mapping rule with an object
type of `relation`:

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `sales_db` | `sales` |
| Schema | `cust_mgmt` | `cms` |
| Relation | `clients` | `accounts` |
| Attribute |   |   |

When it is applied, no other objects at the `sales_db` or `sales_db.cust_mgmt`
level of the object hierarchy have their name changed.

### The most specific rule is applied

Only one name mapping rule is applied to an object. If multiple rules could
affect a single object, the rule that affects the lowest level name part
is applied. For example, if a `database` type name mapping rule and a `schema`
type name mapping rule could both affect the name of a `relation` object, the
`schema` type name mapping rule is applied.

### Use a unique combination of type and source values

You can't specify more than one name mapping rule with the same type and source
values. For example, you can't specify both of the following name mapping rules:

|   | **Rule 1, type `attribute`** || **Rule 2, type `attribute`** ||
|---|---|---|---|---|
| **Name part** | **Source** | **Target** | **Source** | **Target** |
| Database | `project` |   | `project` |   |
| Schema | `dataset1` |   | `dataset1` |   |
| Relation | `table1` |   | `table1` |   |
| Attribute | `lname` | `last_name` | `lname` | `lastname` |

### Create matching `attribute` and `attribute alias` name mapping rules

When you use an `attribute` type name mapping rule to change an attribute name
in DDL statements, you must create an `attribute alias` name mapping rule to
change that attribute's name in DML statements as well.

### Name changes don't cascade

Name changes don't cascade across name rules.
For example, if you created a name mapping rule that renames `database1` to
`project1`, and another that renames `project1` to `project2`, the translator
doesn't map `database1` to `project2`.

## Handle source objects that don't have four-part names

Some source systems, like Teradata, use three name parts to fully qualify object
names. Many source systems also allow you to use partially qualified names in
their SQL dialects,
for example using `database1.schema1.table1`, `schema1.table1`, and `table1`
to refer to the same object in different contexts. If your source files contain
objects that don't use four-part object names, you can use name mapping in
combination with specifying a [default database name](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#default_database)
and a [default schema name](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#default_schema) to achieve
the name mapping that you want.

For examples of using name mapping rules with a default database name or a
default schema name, see
[Change the database name part for objects with varying levels of name completion](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#partial-database)
and
[Change a partially qualified relation object name](https://docs.cloud.google.com/bigquery/docs/output-name-mapping#partial-relation).

## Name mapping examples

Use the examples in this section to see how name mapping rules work for common
use cases.

### Change the database name part for fully qualified objects

The following example renames the database name part from `td_project`
to `bq_project` for all `database`, `schema`, `relation`, and `function`
objects that have fully qualified names.

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `td_project` | `bq_project` |
| Schema |   |   |
| Relation |   |   |
| Attribute |   |   |

<br />

**Type**

- `database`

**Example input**

- `SELECT * FROM td_project.schema.table;`
- `SELECT * FROM td_project.schema1.table1;`

**Example output**

- `SELECT * FROM bq_project.schema.table;`
- `SELECT * FROM bq_project.schema1.table1`

### Change the database name part for objects with varying levels of name completion

The following example renames database name part `project` to `bq_project`
for all object types, and also adds `bq_project` as the database name part
for objects that don't specify one.

To do this, you must specify a default database value when configuring the
translation job, in addition to specifying name mapping rules. For more
information on specifying a default database name, see
[Submit a translation job](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#submit_a_translation_job).

**Default database value**

- `project`

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `project` | `bq_project` |
| Schema |   |   |
| Relation |   |   |
| Attribute |   |   |

<br />

**Type**

- `database`

**Example input**

- `SELECT * FROM project.schema.table;`
- `SELECT * FROM schema1.table1;`

**Example output**

- `SELECT * FROM bq_project.schema.table;`
- `SELECT * FROM bq_project.schema1.table1`

### Change the database name part and the schema name part for fully qualified objects

The following example changes the database name part `warehouse1` to
`myproject`, and also changes the `database1` schema name part
to `mydataset`.

You can also change the parts of a `relation` object name in the
same manner, by using a `relation` type and specifying source and target
values for the relation name part.

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `warehouse1` | `myproject` |
| Schema | `database1` | `mydataset` |
| Relation |   |   |
| Attribute |   |   |

<br />

**Type**

- `schema`

**Example input**

- `SELECT * FROM warehouse1.database1.table1;`
- `SELECT * FROM database2.table2;`

**Example output**

- `SELECT * FROM myproject.mydataset.table1;`
- `SELECT * FROM __DEFAULT_DATABASE__.database2.table2;`

### Change a fully qualified `relation` object name

The following example renames `mydb.myschema.mytable` to
`mydb.myschema.table1`.

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `mydb` | `mydb` |
| Schema | `myschema` | `myschema` |
| Relation | `mytable` | `table1` |
| Attribute |   |   |

<br />

**Type**

- `relation`

**Example input**

- `CREATE table mydb.myschema.mytable(id int, name varchar(64));`

**Example output**

- `CREATE table mydb.myschema.table1(id integer, name string(64));`

### Change a partially qualified `relation` object name

The following example renames `myschema.mytable` to
`mydb.myschema.table1`.

**Default database value**

- `mydb`

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `mydb` | `mydb` |
| Schema | `myschema` | `myschema` |
| Relation | `mytable` | `table1` |
| Attribute |   |   |

<br />

**Type**

- `relation`

**Example input**

- `CREATE table myschema.mytable(id int, name varchar(64));`

**Example output**

- `CREATE table mydb.myschema.table1(id integer, name string(64));`

### Change a `relation alias` object name

The following example renames all instances of the `relation alias` object
`table` to `t`.

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database |   |   |
| Schema |   |   |
| Relation | `table` | `t` |
| Attribute |   |   |

<br />

**Type**

- `relation alias`

**Example input**

- `SELECT table.id, table.name FROM mydb.myschema.mytable table`

**Example output**

- `SELECT t.id, t.name FROM mydb.myschema.mytable AS t`

### Change a `function` object name

The following example renames `mydb.myschema.myfunction` to
`mydb.myschema.function1`.

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `mydb` | `mydb` |
| Schema | `myschema` | `myschema` |
| Relation | `myprocedure` | `procedure1` |
| Attribute |   |   |

<br />

**Type**

- `function`

**Example input**

- `CREATE PROCEDURE mydb.myschema.myprocedure(a int) BEGIN declare i int; SET i = a + 1; END;`
- `CALL mydb.myschema.myprocedure(7)`

**Example output**

- `CREATE PROCEDURE mydb.myschema.procedure1(a int) BEGIN declare i int; SET i = a + 1; END;`
- `CALL mydb.myschema.procedure1(7);`

### Change an `attribute` object name

The following example renames `mydb.myschema.mytable.myfield` to
`mydb.myschema.mytable.field1`. Because `attribute` objects are at the lowest
level of the object hierarchy, this name mapping does not change the name
of any other object.

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `mydb` |   |
| Schema | `myschema` |   |
| Relation | `mytable` |   |
| Attribute | `myfield` | `field1` |

<br />

**Type**

- `attribute`

**Example input**

- `CREATE table mydb.myschema.mytable(myfield int, name varchar(64), revenue int);`

**Example output**

- `CREATE table mydb.myschema.mytable(field1 int, name varchar(64), revenue int);`

### Change an `attribute alias` object name

The following example renames `mydb.myschema.mytable.myfield` to
`mydb.myschema.mytable.field1`. Because `attribute alias` objects are at the
lowest level of the object hierarchy, this name mapping does not change the
name of any other object.

**Source and target name parts**

| **Name part** | **Source** | **Target** |
|---|---|---|
| Database | `mydb` |   |
| Schema | `myschema` |   |
| Relation | `mytable` |   |
| Attribute | `myfield` | `field1` |

<br />

**Type**

- `attribute alias`

**Example input**

- `SELECT myfield, name FROM mydb.myschema.mytable;`

**Example output**

- `SELECT field1, name FROM mydb.myschema.mytable;`

## JSON file format

If you choose to specify name mapping rules by using a JSON file rather than
the Google Cloud console, the JSON file must follow this format:

    {
      "name_map": [
        {
          "source": {
            "type": "string",
            "database": "string",
            "schema": "string",
            "relation": "string",
            "attribute": "string"
          },
          "target": {
            "database": "string",
            "schema": "string",
            "relation": "string",
            "attribute": "string"
          }
        }
      ]
    }

The file size must be less than 5 MB.

For more information on specifying name mapping rules for a translation job, see
[Submit a translation job](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#submit_a_translation_job).

### JSON examples

The following examples show how to specify name mapping rules by using
JSON files.

#### Example 1

The name mapping rules in this example make the following object name changes:

- Rename instances of the `project.dataset2.table2` `relation` object to `bq_project.bq_dataset2.bq_table2`.
- Renames all instances of the `project` `database` object to `bq_project`. For example, `project.mydataset.table2` becomes `bq_project.mydataset.table2`, and `CREATE DATASET project.mydataset` becomes `CREATE DATASET bq_project.mydataset`.

    {
      "name_map": [{
        "source": {
          "type": "RELATION",
          "database": "project",
          "schema": "dataset2",
          "relation": "table2"
        },
        "target": {
          "database": "bq_project",
          "schema": "bq_dataset2",
          "relation": "bq_table2"
        }
      }, {
        "source": {
          "type": "DATABASE",
          "database": "project"
        },
        "target": {
          "database": "bq_project"
        }
      }]
    }

#### Example 2

The name mapping rules in this example make the following object name changes:

- Rename instances of the `project.dataset2.table2.field1` `attribute` object to `bq_project.bq_dataset2.bq_table2.bq_field` in both DDL and DML statements.

    {
      "name_map": [{
        "source": {
          "type": "ATTRIBUTE",
          "database": "project",
          "schema": "dataset2",
          "relation": "table2",
          "attribute": "field1"
        },
        "target": {
          "database": "bq_project",
          "schema": "bq_dataset2",
          "relation": "bq_table2",
          "attribute": "bq_field"
        }
      }, {
        "source": {
          "type": "ATTRIBUTE_ALIAS",
          "database": "project",
          "schema": "dataset2",
          "relation": "table2",
          "attribute": "field1"
        },
        "target": {
          "database": "bq_project",
          "schema": "bq_dataset2",
          "relation": "bq_table2",
          "attribute": "bq_field"
        }
      }]
    }