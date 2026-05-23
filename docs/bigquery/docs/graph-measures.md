# Work with measures

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

This document shows you how to define and query measures on your graphs.
You can use measures to ensure that aggregations are performed correctly
across joins.

## Overview

A *measure* is an aggregate property defined within the `PROPERTIES` clause
of a node or edge table. Measures are defined using the `MEASURE` keyword and
one of the following supported aggregate functions:

- `SUM`
- `AVG`
- `COUNT`
- `COUNT(DISTINCT)`
- `MIN`
- `MAX`

Measures define their aggregation in reference to the `KEY` of the node or
edge table on which they're defined. This means that when you query a measure,
the aggregation is performed correctly even if the underlying table is joined in
a way that causes rows to be duplicated.

You can't reference a property defined by a measure in a GQL query. Instead,
you access measures by calling the
[`GRAPH_EXPAND` TVF](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-queries#graph_expand)
to create a flattened table representation of your graph. This function doesn't
accept all types of graphs. For more information about
which graphs form valid input, see the [input limitations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-queries#input_limitations).

You can call the
[`AGG` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#agg)
on output from the `GRAPH_EXPAND` TVF to aggregate
the properties according to the aggregation function you defined in
the measure.

## Define measures

You define measures within the
[`PROPERTIES` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#element_table_property_definition)
of a node or edge table definition by using the `MEASURE()` keyword around a
supported aggregate function.

The following example creates a dataset called `university` and tables
for colleges, departments, and courses:

    CREATE SCHEMA IF NOT EXISTS university;

    CREATE OR REPLACE TABLE university.College (
      college_id INT64 PRIMARY KEY NOT ENFORCED,
      college_name STRING
    );

    CREATE OR REPLACE TABLE university.Department (
      dept_id INT64 PRIMARY KEY NOT ENFORCED,
      dept_name STRING,
      college_id INT64,
      budget FLOAT64,
      FOREIGN KEY (college_id) REFERENCES university.College(college_id) NOT ENFORCED
    );

    CREATE OR REPLACE TABLE university.Course (
      course_id INT64 PRIMARY KEY NOT ENFORCED,
      course_name STRING,
      dept_id INT64,
      credits INT64,
      FOREIGN KEY (dept_id) REFERENCES university.Department(dept_id) NOT ENFORCED
    );

    INSERT INTO university.College (college_id, college_name)
    VALUES (101, 'College of Engineering'),
          (102, 'College of Arts');

    INSERT INTO university.Department (dept_id, dept_name, college_id, budget)
    VALUES (1001, 'Computer Science', 101, 500000),
          (1002, 'Mechanical Engineering', 101, 400000),
          (1003, 'Fine Arts', 102, 200000),
          (1004, 'Research', 101, 50000);

    INSERT INTO university.Course (course_id, course_name, dept_id, credits)
    VALUES (1, 'Intro to CS', 1001, 3),
           (2, 'Data Structures', 1001, 4),
           (3, 'Thermodynamics', 1002, 3),
           (4, 'Oil Painting', 1003, 2);

The following statement creates a graph called `SchoolGraph` that defines
measures on some of the properties in the `Department` and `Course` nodes.
You must provide an alias for properties defined by a measure.

    CREATE OR REPLACE PROPERTY GRAPH university.SchoolGraph
      NODE TABLES (
        university.College
          KEY(college_id)
          PROPERTIES(college_id, college_name),
        university.Department
          KEY(dept_id)
          PROPERTIES(dept_id, dept_name, college_id,
            budget OPTIONS(description="Department budget in USD"),
            MEASURE(SUM(budget)) AS total_budget),
        university.Course
          KEY(course_id)
          PROPERTIES(
            course_id,
            course_name,
            credits,
            dept_id,
            MEASURE(AVG(credits)) AS avg_credits,
            MEASURE(SUM(credits)) AS total_credits,
            MEASURE(COUNT(course_id)) AS course_count)
      )
      EDGE TABLES (
        university.Department AS CollegeDept
          SOURCE KEY (college_id) REFERENCES College (college_id)
          DESTINATION KEY (dept_id) REFERENCES Department (dept_id),
        university.Course AS DeptCourse
          SOURCE KEY (dept_id) REFERENCES Department (dept_id)
          DESTINATION KEY (course_id) REFERENCES Course (course_id)
      );

![A visualization of the SchoolGraph graph.](https://docs.cloud.google.com/static/bigquery/images/graph-university-example.png)

The `total_budget` measure is defined as `MEASURE(SUM(budget))`. The measure
locks the aggregation to the `KEY`, which is `dept_id`.

## Understanding overcounting

When you join tables, data is repeated every time there is a one-to-many
relationship in your data. For example, if you join the `Course`, `Department`,
and `College` tables, a department with multiple courses appears in multiple
rows in the output:

    SELECT
      college_name,
      dept_name,
      course_name,
      budget
    FROM university.Course
    LEFT JOIN university.Department
      ON Course.dept_id = Department.dept_id
    LEFT JOIN university.College
      ON Department.college_id = College.college_id;

    /*---+---+---+---+
     | college_name           | dept_name              | course_name     | budget   |
     +---+---+---+---+
     | College of Engineering | Computer Science       | Intro to CS     | 500000.0 |
     | College of Engineering | Computer Science       | Data Structures | 500000.0 |
     | College of Engineering | Mechanical Engineering | Thermodynamics  | 400000.0 |
     | College of Arts        | Fine Arts              | Oil Painting    | 200000.0 |
     +---+---+---+---*/

If you try to compute the total budget per college using `SUM(budget)`,
the Computer Science department's budget is counted twice. You can avoid this
problem by querying the `Department` table directly, but this approach doesn't
work if you want to compute multiple aggregations from different tables that
contribute to your joined data. The following section shows how measures solve
this problem.

## Query a graph with measures

You can query a graph with measures using GQL, but your query can't directly
reference any property defined by a measure. For example, the following
query references nodes that have measure properties defined, but doesn't use
or return any of the measure fields:

    GRAPH university.SchoolGraph
    MATCH (c:College)-[]-(d:Department)-[]->(course:Course)
    RETURN c.college_name, d.dept_name, course.course_name;

    /*---+---+---+
     | college_name           | dept_name              | course_name     |
     +---+---+---+
     | College of Engineering | Computer Science       | Intro to CS     |
     | College of Engineering | Computer Science       | Data Structures |
     | College of Engineering | Mechanical Engineering | Thermodynamics  |
     | College of Arts        | Fine Arts              | Oil Painting    |
     +---+---+---*/

### Work with measures

To work with measures, you use the
[`GRAPH_EXPAND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-queries#graph_expand)
table-valued function (TVF)
to query your graph as a single flattened table.
Columns in the output table are derived from the properties defined in the
graph for each node and edge table. To prevent naming conflicts, columns are
named by concatenating the node or edge table label and the property name---for
example, `Course_course_name` or `Department_total_budget`. The following query
shows some sample output from the `GRAPH_EXPAND` TVF:

    SELECT
      College_college_name,
      Department_dept_name,
      Department_budget,
      Course_course_name
    FROM
      GRAPH_EXPAND("university.SchoolGraph");

    /*---+---+---+---+
     | College_college_name   | Department_dept_name   | Department_budget | Course_course_name |
     +---+---+---+---+
     | College of Engineering | Computer Science       | 500000.0          | Intro to CS        |
     | College of Engineering | Computer Science       | 500000.0          | Data Structures    |
     | College of Engineering | Mechanical Engineering | 400000.0          | Thermodynamics     |
     | College of Arts        | Fine Arts              | 200000.0          | Oil Painting       |
     +---+---+---+---+

The `GRAPH_EXPAND` function produces the flattened table by applying a series
of `LEFT JOIN` operations to node and edge tables. A valid input graph must
have exactly one *root node table* , which is a table whose `KEY` value doesn't
appear in any other table. Data that isn't reachable from the root node table
through a series of joins doesn't appear in the output. In the previous
example, the `Course` table is the root node table. The `Research` department
is omitted from the output because it doesn't have any courses.

You can't directly select a column for a property defined by a measure.
Instead, you must wrap them in the
[`AGG()` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#agg).
This function ensures that the aggregation defined on the measures is performed
exactly once per key.

The following query simultaneously calculates the total budget and total number
of courses for each college:

    SELECT
      College_college_name,
      AGG(Department_total_budget) AS college_budget,
      AGG(Course_course_count) AS total_courses
    FROM
      GRAPH_EXPAND("university.SchoolGraph")
    GROUP BY
      College_college_name;

    /*---+---+---+
     | College_college_name   | college_budget | total_courses |
     +---+---+---+
     | College of Engineering | 900000.0       | 3             |
     | College of Arts        | 200000.0       | 1             |
     +---+---+---*/

> [!CAUTION]
> **Caution:** Changes to the underlying graph don't invalidate [cached results](https://docs.cloud.google.com/bigquery/docs/cached-results) for queries that call the `GRAPH_EXPAND` function. To ensure correct output, disable the retrieval of cached results when you call this function.

## View `GRAPH_EXPAND` schema

To view the schema of the table returned by the `GRAPH_EXPAND` function
without calling the function, use the
[`BQ.SHOW_GRAPH_EXPAND_SCHEMA` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqshow_graph_expand_schema):

    DECLARE schema STRING DEFAULT '';
    CALL BQ.SHOW_GRAPH_EXPAND_SCHEMA('university.SchoolGraph', schema);
    SELECT schema;

This procedure populates the `schema` variable with the name, type, and mode
of each column. It also indicates whether the property referenced by the column
is a measure, and lists any description or synonyms that you defined on it.

## What's next

- Learn how to [create and query a property graph](https://docs.cloud.google.com/bigquery/docs/graph-create).
- Learn about [graph schemas](https://docs.cloud.google.com/bigquery/docs/graph-schema-overview).