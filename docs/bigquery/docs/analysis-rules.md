# Restrict data access using analysis rules

This document provides general information about analysis rules in
GoogleSQL for BigQuery.

## What is an analysis rule?

An analysis rule enforces [policies](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#privacy_policy)
for sharing data. A policy represents a condition that needs to be met before a
query can be run. With BigQuery, you can enforce an analysis rule on
a view, using a [data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) or by
applying the analysis rule directly to the view. When you enforce an
analysis rule, you enforce that everyone querying that view must abide by that
analysis rule on the view. If the analysis rule is satisfied, the query produces
output that satisfies the analysis rule. If the query doesn't satisfy the
analysis rule, an error is produced.

## Supported analysis rules

The following analysis rules are supported:

- [Aggregation threshold analysis rule](https://docs.cloud.google.com/bigquery/docs/analysis-rules#agg_analysis_rules): Enforces the
  minimum number of distinct entities that must be present in a dataset.
  You can enforce this rule on a view, using DDL statements or
  data clean rooms. This rule supports the aggregation threshold policy
  and the join restriction policy.

- [Differential privacy analysis rule](https://docs.cloud.google.com/bigquery/docs/analysis-rules#dp_analysis_rules): Enforces a
  privacy budget, which limits the data that is revealed to a subscriber when
  the data is protected with [differential privacy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/differential-privacy).
  You can enforce this rule on a view, using DDL statements or
  data clean rooms. This rule supports the differential privacy policy
  and the join restriction policy.

- [List overlap analysis rule](https://docs.cloud.google.com/bigquery/docs/analysis-rules#list_overlap_rules): Overlapping rows can only
  be queried after a join operation, which conforms to the rule. You can
  enforce this rule on a view, using DDL statements or data clean rooms.
  This rule supports the join restriction policy.

## Aggregation threshold analysis rule

An aggregation threshold analysis rule enforces the minimum number of
distinct entities that must contribute to an output row of a query, so that
the output row is included in the query result.

When enforced, the aggregation threshold analysis rule groups data across
dimensions, while ensuring the aggregation threshold is met. It counts the
number of distinct privacy units (represented by the privacy unit column) for
each group, and only outputs the groups where the distinct privacy unit count
satisfies the aggregation threshold.

A view that includes this analysis rule must include the
[aggregation threshold policy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#privacy_policy)
and can optionally include the [join restriction policy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#privacy_policy).

### Define an aggregation threshold analysis rule for a view

You can define an aggregation threshold analysis rule for a view in a
[data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) or with the
`CREATE VIEW` statement:

```googlesql
CREATE OR REPLACE VIEW VIEW_NAME
  OPTIONS (
    privacy_policy= '''{
      "aggregation_threshold_policy": {
        "threshold" : THRESHOLD,
        "privacy_unit_column": "PRIVACY_UNIT_COLUMN"
      },
      "join_restriction_policy": {
        "join_condition": "JOIN_CONDITION",
        "join_allowed_columns": JOIN_ALLOWED_COLUMNS
      }
    }'''
  )
  AS QUERY;
```

Definitions:

- `aggregation_threshold_policy`: The aggregation threshold policy for the
  aggregation threshold analysis rule.

  - <var translate="no">VIEW_NAME</var>: The path and name of the view.

  - <var translate="no">THRESHOLD</var>: The minimum number of distinct privacy units
    that need to contribute to each row in the query results. If a potential
    row doesn't satisfy this threshold, that row is omitted from the
    query results.

  - <var translate="no">PRIVACY_UNIT_COLUMN</var>: Represents the privacy unit column. A
    *privacy unit column* is a unique identifier for a privacy unit.
    A *privacy unit* is a value from the privacy unit column
    that represents the entity in a set of data that is being protected.

    You can use only one privacy unit column,
    and the data type for the privacy unit column must be
    [groupable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#groupable_data_types).

    The values in the privacy unit column cannot be directly projected
    through a query, and you can use only
    [analysis rule-supported aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_policy_functions)
    to aggregate the data in this column.
- `join_restriction_policy` (optional): The optional join restriction policy
  for the aggregation threshold analysis rule.

  - <var translate="no">JOIN_CONDITION</var>: The type of join restriction to enforce
    on a view. This can be one of the following values:

    - `JOIN_ALL`: All columns in `join_allowed_columns` must be
      inner joined upon for this view to be queried.

    - `JOIN_ANY`: At least one column in `join_allowed_columns` must be
      joined upon for this view to be queried.

    - `JOIN_BLOCKED`: This view can't be joined along any column.
      Don't set `join_allowed_columns` in this case.

    - `JOIN_NOT_REQUIRED`: A join is not required to query this view. If a
      join is used, only the columns in `join_allowed_columns` can be
      used.

  - <var translate="no">JOIN_ALLOWED_COLUMNS</var>: The columns that can be part of a
    join operation.

- <var translate="no">QUERY</var>: The query for the view.

Example:

In the following example, an aggregation threshold analysis rule is created on
a view called `ExamView`. `ExamView` references a table called
[`ExamTable`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#example-tables):

    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 3, "privacy_unit_column": "last_name"}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

To review the `privacy_policy` syntax for `CREATE VIEW`, see the
`OPTIONS` list in [`CREATE VIEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement).

### Update an aggregation threshold analysis rule for a view

You can change the aggregation threshold analysis rule for a view in a
[data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) or with the `ALTER VIEW` statement:

```googlesql
ALTER VIEW VIEW_NAME
SET OPTIONS (
  privacy_policy= '''{
    "aggregation_threshold_policy": {
      "threshold" : THRESHOLD,
      "privacy_unit_column": "PRIVACY_UNIT_COLUMN"
    }
  }'''
)
```

For more information about the values you can set for the privacy policies
in the preceding syntax, see
[Define an aggregation threshold analysis rule for a view](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view).

Example:

In the following example, an aggregation threshold analysis rule is updated
on a view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view).

    ALTER VIEW mydataset.ExamView
    SET OPTIONS (
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 50, "privacy_unit_column": "last_name"}}'
    );

To review the `privacy_policy` syntax for `ALTER VIEW`, see
the `OPTIONS` list in [`ALTER VIEW SET OPTIONS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_view_set_options_statement).

### Query an aggregation threshold analysis rule--enforced view

You can query a view that has an aggregation threshold analysis rule with
the [`AGGREGATION_THRESHOLD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause)
clause. The query must include aggregation functions, and you can use only
[aggregation threshold-supported aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_policy_functions)
in this query.

Example:

In the following example, an aggregation threshold analysis rule is queried on
a view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view):

    SELECT WITH AGGREGATION_THRESHOLD
      test_id, COUNT(DISTINCT last_name) AS student_count
    FROM mydataset.ExamView
    GROUP BY test_id;

    /*---+---*
     | test_id | student_count |
     +---+---+
     | P91     | 3             |
     | U25     | 4             |
     *---+---*/

The aggregation threshold analysis rule can also optionally include
the join restriction policy. To learn how to use the
join restriction policy with an analysis rule, see
[Join restriction policy in analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules#join-restriction-policy).

To review additional examples for the
`AGGREGATION_THRESHOLD` clause, see
[`AGGREGATION_THRESHOLD` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause).

## Differential privacy analysis rule

The differential privacy analysis rule enforces a privacy budget, which limits
the data that is revealed to a subscriber when the data is protected with
[differential privacy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/differential-privacy).
A privacy budget prevents any subscriber from querying shared data when the sum
of all queries' epsilon or delta reaches the total epsilon or total delta value.
You can use this analysis rule in a view.

A view that includes this analysis rule must include the
[differential privacy policy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#privacy_policy)
and can optionally include the [join restriction policy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#privacy_policy).

> [!WARNING]
> **Preview**
>
>
> Parameter-driven privacy budgeting for differential privacy is in
> [preview](https://cloud.google.com/products#product-launch-stages) while
> differential privacy enforcement in BigQuery data clean rooms is
> now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
>
>
> This preview product or feature is subject to the "Pre-GA Offerings Terms"
> in the General Service Terms section of the
> [Service Specific Terms](https://cloud.google.com/terms/service-terms).
> Pre-GA products and features are available "as is" and might have
> limited support. For more information, see the
> [launch stage descriptions](https://cloud.google.com/products#product-launch-stages).
>
>
> To provide feedback or request support for features in preview, send an
> email to [bq-dcr-feedback@google.com](mailto:bq-dcr-feedback@google.com).

### Define a differential privacy analysis rule for a view

> [!NOTE]
> **Note:** In this section, the privacy parameters in the examples are not recommendations. You should work with your privacy or security officer to determine the optimal privacy parameters for your dataset and organization.

You can define a differential privacy analysis rule for a view in a
[data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) or with the
`CREATE VIEW` statement:

```googlesql
CREATE OR REPLACE VIEW VIEW_NAME
  OPTIONS (
    privacy_policy= '''{
      "differential_privacy_policy": {
        "privacy_unit_column": "PRIVACY_UNIT_COLUMN",
        "max_epsilon_per_query": MAX_EPSILON_PER_QUERY,
        "epsilon_budget": EPSILON_BUDGET,
        "delta_per_query": DELTA_PER_QUERY,
        "delta_budget": DELTA_BUDGET,
        "max_groups_contributed": MAX_GROUPS_CONTRIBUTED
      },
      "join_restriction_policy": {
        "join_condition": "JOIN_CONDITION",
        "join_allowed_columns": JOIN_ALLOWED_COLUMNS
      }
    }'''
  )
  AS QUERY;
```

Definitions:

- `differential_privacy_policy`: The differential privacy policy for the
  differential privacy analysis rule.

  - <var translate="no">PRIVACY_UNIT_COLUMN</var>: The
    [column](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_privacy_unit_id)
    that identifies the entity in a dataset that is protected using a
    privacy analysis rule. This value is a JSON string.

  - <var translate="no">MAX_EPSILON_PER_QUERY</var>: Determines the amount of noise added
    to the results per query and prevents the total [epsilon](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_epsilon)
    from being reached by a single query. This value is a JSON number from
    `0.001` to `1e+15`.

  - <var translate="no">EPSILON_BUDGET</var>: The
    [epsilon](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_epsilon)
    budget that represents the total epsilon that can be used across all
    differentially private queries on the view.
    This value must be larger than `MAX_EPSILON_PER_QUERY`, and is a
    JSON number from `0.001` to `1e+15`.

  - <var translate="no">DELTA_PER_QUERY</var>: The probability that any row in the result
    fails to be epsilon-differentially private.
    This value is a JSON number from `1e-15` to `1`.

  - <var translate="no">DELTA_BUDGET</var>: The
    [delta](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_delta)
    budget, which represents the total delta that can be used across all
    differentially private queries on the view. This value must be larger than
    `DELTA_PER_QUERY`, and is a JSON number from `1e-15` to `1000`.

  - <var translate="no">MAX_GROUPS_CONTRIBUTED</var> (optional): Limits the
    [number of groups](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_max_groups)
    to which an entity in a privacy unit column can contribute. This value
    must be a non-negative JSON integer.

- `join_restriction_policy` (optional): The optional join restriction policy
  for the differential privacy analysis rule.

  - <var translate="no">JOIN_CONDITION</var>: The type of join restriction to enforce
    on a view. This can be one of the following values:

    - `JOIN_ALL`: All columns in `join_allowed_columns` must be
      inner joined upon for this view to be queried.

    - `JOIN_ANY`: At least one column in `join_allowed_columns` must be
      joined upon for this view to be queried.

    - `JOIN_BLOCKED`: This view can't be joined along any column.
      Don't set `join_allowed_columns` in this case.

    - `JOIN_NOT_REQUIRED`: A join is not required to query this view. If a
      join is used, only the columns in `join_allowed_columns` can be
      used.

  - <var translate="no">JOIN_ALLOWED_COLUMNS</var>: The columns that can be part of a
    join operation.

- <var translate="no">QUERY</var>: The query for the view.

Example:

In the following example, a differential privacy analysis rule is created on
a view called `ExamView`. `ExamView` references a table called
[`ExamTable`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#example-tables):

    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"differential_privacy_policy": {"privacy_unit_column": "last_name", "max_epsilon_per_query": 1000.0, "epsilon_budget": 10000.1, "delta_per_query": 0.01, "delta_budget": 0.1, "max_groups_contributed": 2}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- NOTE: Delta and epsilon parameters are set very high due to the small
    -- dataset. In practice, these should be much smaller.

To review the `privacy_policy` syntax for `CREATE VIEW`, see the
`OPTIONS` list in [`CREATE VIEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement).

### Update a differential privacy analysis rule for a view

> [!NOTE]
> **Note:** In this section, the privacy parameters in the examples are not recommendations. You should work with your privacy or security officer to determine the optimal privacy parameters for your dataset and organization.

You can change the differential privacy analysis rule for a view in a
[data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) or with the
`ALTER VIEW` statement:

```googlesql
ALTER VIEW VIEW_NAME
SET OPTIONS (
  privacy_policy= '''{
    "differential_privacy_policy": {
      "privacy_unit_column": "PRIVACY_UNIT_COLUMN",
      "max_epsilon_per_query": MAX_EPSILON_PER_QUERY,
      "epsilon_budget": EPSILON_BUDGET,
      "delta_per_query": DELTA_PER_QUERY,
      "delta_budget": DELTA_BUDGET,
      "max_groups_contributed": MAX_GROUPS_CONTRIBUTED
    }
  }'''
)
```

For more information about the values you can set for the privacy policies
in the preceding syntax, see
[Define a differential privacy analysis rule for a view](https://docs.cloud.google.com/bigquery/docs/analysis-rules#dp_analysis_rules).

> [!NOTE]
> **Note:** When you update a differential privacy analysis rule, the privacy budgets are reset.

Example:

In the following example, a differential privacy analysis rule is updated
on a view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#dp_define_privacy_view).

    ALTER VIEW mydataset.ExamView
    SET OPTIONS(
      privacy_policy= '{"differential_privacy_policy": {"privacy_unit_column": "last_name", "max_epsilon_per_query": 0.01, "epsilon_budget": 1000.0, "delta_per_query": 0.05, "delta_budget": 0.1, "max_groups_contributed": 2}}'
    );

    -- NOTE: Delta and epsilon parameters are set very high due to the small
    -- dataset. In practice, these should be much smaller.

To review the `privacy_policy` syntax for `ALTER VIEW`, see
the `OPTIONS` list in [`ALTER VIEW SET OPTIONS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_view_set_options_statement).

### Query a differential privacy analysis rule--enforced view

> [!NOTE]
> **Note:** In this section, the privacy parameters in the examples are not recommendations. You should work with your privacy or security officer to determine the optimal privacy parameters for your dataset and organization.

You can query a view that has a differential privacy analysis rule with the
`DIFFERENTIAL_PRIVACY` clause. To review the syntax and additional examples
for the `DIFFERENTIAL_PRIVACY` clause, see
[`DIFFERENTIAL_PRIVACY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_clause).

> [!NOTE]
> **Note:** If you've just created a view with a differential privacy analysis rule, briefly wait before running any queries on it.

Example:

In the following example, a differential privacy analysis rule is queried on
a view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view). The differentially private
data should be successfully returned from `ExamView` because `epsilon`, `delta`,
and `max_groups_contributed` all satisfy the conditions of the differential
analysis rule in `ExamView`.

    -- Query an analysis--rule enforced view called ExamView.
    SELECT
      WITH DIFFERENTIAL_PRIVACY
        test_id,
        AVG(test_score) AS average_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

    -- Results will vary.
    /*---+---*
     | test_id | average_test_score |
     +---+---+
     | P91     | 512.627693163311   |
     | C83     | 506.01565971561649 |
     | U25     | 524.81202728847893 |
     *---+---*/

> [!WARNING]
> **Warning:** When a privacy budget is exhausted over one or multiple queries, the view can no longer be queried and must be updated or re-created.

#### Block a query with an out-of-bounds epsilon

Epsilon can be used to add or remove noise. More epsilon means less noise will
be added. If you want to ensure that a differentially private query has a
minimal amount of noise, pay close attention to the value for
`max_epsilon_per_query` in your differential privacy analysis rule.

Example:

In the following query, the query is blocked with an error because
`epsilon` in the `DIFFERENTIAL_PRIVACY` clause is higher than
`max_epsilon_per_query` in [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view):

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"differential_privacy_policy": {"privacy_unit_column": "last_name", "max_epsilon_per_query": 10.01, "epsilon_budget": 1000.0, "delta_per_query": 0.01, "delta_budget": 0.1, "max_groups_contributed": 2}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- NOTE: Delta and epsilon parameters are set very high due to the small
    -- dataset. In practice, these should be much smaller.

After you've created your view, briefly wait, and then
run the following query:

    -- Error: Epsilon is too high: 1e+20, policy for table mydataset.
    -- ExamView allows max 10.01
    SELECT
      WITH DIFFERENTIAL_PRIVACY
        OPTIONS(epsilon=1e20)
        test_id,
        AVG(test_score) AS average_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

> [!WARNING]
> **Warning:** When a privacy budget is exhausted over one or multiple queries, the view can no longer be queried and must be updated or re-created.

#### Block queries that have exceeded an epsilon budget

Epsilon can be used to add or remove noise. Less epsilon increases noise,
more epsilon reduces noise. Even when noise is high, multiple queries over the
same data can eventually reveal the un-noised version of the data. To stop
this from happening, you can create an epsilon budget. If you want to add an
epsilon budget, review the value for `epsilon_budget` in the
differential privacy analysis rule for your view.

Example:

Run the following query three times. On the third time, the query is blocked
because the total epsilon used is `30`, but `epsilon_budget` in
[`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view) only allows `25.6`:

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"differential_privacy_policy": {"privacy_unit_column": "last_name", "max_epsilon_per_query": 10.01, "epsilon_budget": 25.6, "delta_per_query": 0.01, "delta_budget": 0.1, "max_groups_contributed": 2}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- NOTE: Delta and epsilon parameters are set very high due to the small
    -- dataset. In practice, these should be much smaller.

After you've created your view, briefly wait, and then
run the following query three times:

    -- Error after three query runs: Privacy budget is not sufficient for
    -- table 'mydataset.ExamView' in this query.

    SELECT
      WITH DIFFERENTIAL_PRIVACY
        OPTIONS(epsilon=10)
        test_id,
        AVG(test_score) AS average_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

> [!WARNING]
> **Warning:** When a privacy budget is exhausted over one or multiple queries, the view can no longer be queried and must be updated or re-created.

## List overlap analysis rule

Only overlapping rows can be queried after a join operation, which conforms to
the list overlap rule. You can enforce this rule on a view, using DDL statements
or data clean rooms.

A view that includes this analysis rule must only include the
[join restriction policy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#privacy_policy).

### Define a list overlap analysis rule for a view

You can define a list overlap analysis rule for a view in a
[data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) or with the
`CREATE VIEW` statement:

```googlesql
CREATE OR REPLACE VIEW VIEW_NAME
  OPTIONS (
    privacy_policy= '''{
      "join_restriction_policy": {
        "join_condition": "JOIN_CONDITION",
        "join_allowed_columns": JOIN_ALLOWED_COLUMNS
      }
    }'''
  )
  AS QUERY;
```

Definitions:

- `join_restriction_policy`: The join restriction policy for the
  list overlap analysis rule.

  - <var translate="no">JOIN_CONDITION</var>: The type of list overlap to enforce
    on a view. This can be one of the following values:

    - `JOIN_ALL`: All columns in `join_allowed_columns` must be
      inner joined upon for this view to be queried.

    - `JOIN_ANY`: At least one column in `join_allowed_columns` must be
      joined upon for this view to be queried.

  - <var translate="no">JOIN_ALLOWED_COLUMNS</var>: The columns that can be part of a
    join operation.

- <var translate="no">QUERY</var>: The query for the view.

Example:

In the following example, a list overlap analysis rule is created on
a view called `ExamView`. `ExamView` references a table called
[`ExamTable`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#example-tables):

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"join_restriction_policy": {"join_condition": "JOIN_ANY", "join_allowed_columns": ["test_id", "test_score"]}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

### Update a list overlap analysis rule for a view

You can change the list overlap analysis rule for a view with a
[data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) or with the
`ALTER VIEW` statement:

```googlesql
ALTER VIEW VIEW_NAME
SET OPTIONS (
  privacy_policy= '''{
    "join_restriction_policy": {
      "join_condition": "JOIN_CONDITION",
      "join_allowed_columns": JOIN_ALLOWED_COLUMNS
    }
  }'''
)
```

For more information about the values you can set for the privacy policy
in the preceding syntax, see
[Define a list overlap analysis rule for a view](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_list_overlap_view).

Example:

In the following example, a list overlap analysis rule is updated
on a view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_list_overlap_view).

    ALTER VIEW mydataset.ExamView
    SET OPTIONS(
      privacy_policy= '{"join_restriction_policy": {"join_condition": "JOIN_ALL", "join_allowed_columns": ["test_id", "test_score"]}}'
    );

To review the `privacy_policy` syntax for `ALTER VIEW`, see
the `OPTIONS` list in [`ALTER VIEW SET OPTIONS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_view_set_options_statement).

### Query a list overlap analysis rule--enforced view

You can perform a join operation on a view that has a
list overlap analysis rule.
To review the syntax for the `JOIN` operation, see
[Join operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types).

#### Block a join operation with no overlap

You can block a join operation if it doesn't include at least one
overlap with a required column.

Example:

In the following query, a view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_list_overlap_view) is
joined with a table called [`StudentTable`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#example-tables). Because
the view contains the `JOIN_ANY` list overlap analysis rule, at least
one overlapping row from `ExamView` and `StudentTable` is required. Because
there is at least one overlap, the query runs successfully.

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"join_restriction_policy": {"join_condition": "JOIN_ANY", "join_allowed_columns": ["test_score", "last_name"]}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query a view called ExamView and a table called StudentTable.
    SELECT *
    FROM mydataset.ExamView INNER JOIN mydataset.StudentTable USING (test_score);

    /*---+---+---+---*
     | test_score | last_name | test_id | last_name_1 |
     +---+---+---+---+
     | 490        | Ivanov    | U25     | Ivanov      |
     | 500        | Wang      | U25     | Wang        |
     | 510        | Hansen    | P91     | Hansen      |
     | 550        | Silva     | U25     | Silva       |
     | 580        | Devi      | U25     | Devi        |
     *---+---+---+---*/

#### Block an inner join operation without entire overlap

You can block a join operation if it doesn't include an overlap
with all required columns.

Example:

In the following example, a join operation is attempted on a view called
[`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_list_overlap_view) and a table called
[`StudentTable`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#example-tables), but the query fails. The failure occurs
because the `ExamView` list overlap analysis rule requires joining on all
columns present in the join restriction policy. Because the table called
[`StudentTable`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#example-tables) doesn't contain these
columns, not all rows overlap and an error is produced.

    -- Create a view that includes ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"join_restriction_policy": {"join_condition": "JOIN_ALL", "join_allowed_columns": ["test_score", "last_name"]}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query error: Joining must occur on all of the following columns
    -- [test_score, last_name] on table mydataset.ExamView.
    SELECT *
    FROM mydataset.ExamView INNER JOIN mydataset.StudentTable USING (last_name);

## Use a join restriction policy with another policy

The join restriction policy can be used with other policies in the
aggregation threshold and differential privacy analysis rules. However, once
you've used a join restriction policy with another policy, you can't
change that other policy afterwards.

Example:

In the following example, a join restriction policy is used in an
aggregation threshold analysis rule:

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy":{"threshold": 3, "privacy_unit_column": "last_name"}, "join_restriction_policy": {"join_condition": "JOIN_ANY", "join_allowed_columns": ["test_id", "test_score"]}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

### Block a join operation with no required column

You can block a join operation if it doesn't include at least one
required column. To do this include the following parts in your list overlap
analysis rule:

```googlesql
"join_restriction_policy": {
  "join_condition": "JOIN_ANY",
  "join_allowed_columns": ["column_name", ...]
}
```

Example:

In the following query, the query is blocked with an error because
the query does not contain any join operations on the `test_score` or
`test_id` column in [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view) and
[`StudentTable`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#example-tables):

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 3, "privacy_unit_column": "last_name"}, "join_restriction_policy": {"join_condition": "JOIN_ANY", "join_allowed_columns": ["test_score", "test_id"]}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query error: Joining must occur on at least one of the following columns
    -- [test_score, test_id] on table mydataset.ExamView.
    SELECT *
    FROM mydataset.ExamView INNER JOIN mydataset.StudentTable USING (last_name);

To get the preceding query to run, in the `USING` clause, replace `last_name`
with `test_score`.

### Block a query with no join operation

If the query must have a join operation, you can block the query if no join
operation is present by using one of the following
list overlap analysis rules:

```googlesql
"join_restriction_policy": {
  "join_condition": "JOIN_NOT_REQUIRED"
}
```

```googlesql
"join_restriction_policy": {
  "join_condition": "JOIN_NOT_REQUIRED",
  "join_allowed_columns": []
}
```

Example:

In the following query, the query is blocked because there is no
join operation with [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view) in the query:

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 3, "privacy_unit_column": "last_name"}, "join_restriction_policy": {"join_condition": "JOIN_NOT_REQUIRED"}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query error: At least one allowed column must be specified with
    -- join_condition = 'JOIN_NOT_REQUIRED'.
    SELECT *
    FROM mydataset.ExamView;

### Block a query with no join operation and no required column

If the query must have a join operation and the join operation must have at
least one required column, include the following parts in your list overlap
analysis rule:

```googlesql
"join_restriction_policy": {
  "join_condition": "JOIN_NOT_REQUIRED",
  "join_allowed_columns": ["column_name", ...]
}
```

Example:

In the following query, the query is blocked because the
join operation does not include a column in the
[`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view) `join_allowed_columns` array:

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 3, "privacy_unit_column": "last_name"}, "join_restriction_policy": {"join_condition": "JOIN_NOT_REQUIRED", "join_allowed_columns": ["test_score"]}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query error: Join occurring on a restricted column.
    SELECT *
    FROM mydataset.ExamView INNER JOIN mydataset.StudentTable USING (last_name);

To get the preceding query to run, in the `USING` clause, replace `last_name`
with `test_score`.

### Block all join operations

You can block all join operations. To do this, only include the following parts
in your list overlap analysis rule:

```googlesql
"join_restriction_policy": {
  "join_condition": "JOIN_BLOCKED",
}
```

Example:

In the following query, the query is blocked because there is a
join operation with a view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view):

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 3, "privacy_unit_column": "last_name"}, "join_restriction_policy": {"join_condition": "JOIN_BLOCKED"}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query error: Join occurring on a restricted column.
    SELECT *
    FROM mydataset.ExamView INNER JOIN mydataset.StudentTable USING (last_name);

To get the preceding query to run, remove the `INNER JOIN` operation.

### Block an inner join operation without all required columns

You can block an inner join operation if it doesn't include all
required columns. To do this, include the following parts in your list overlap
analysis rule:

```googlesql
"join_restriction_policy": {
  "join_condition": "JOIN_ALL",
  "join_allowed_columns": ["column_name", ...]
}
```

Example:

In the following query, the query is blocked with an error because
the query does not include `test_score` in the join operation with
the view called [`ExamView`](https://docs.cloud.google.com/bigquery/docs/analysis-rules#define_privacy_view):

    -- Create a view that includes a table called ExamTable.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 3, "privacy_unit_column": "last_name"}, "join_restriction_policy": {"join_condition": "JOIN_ALL", "join_allowed_columns": ["test_score", "last_name"]}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query error: Joining must occur on all of the following columns
    -- [test_score, last_name] on table mydataset.ExamView.
    SELECT *
    FROM mydataset.ExamView INNER JOIN mydataset.StudentTable USING (last_name);

To get the preceding query to run, replace `USING (last_name)`
with `USING (last_name, test_score)`.

## Example tables

Several examples in this document reference two tables called `ExamTable` and
`StudentTable`. `ExamTable` contains a list of test scores produced by students
and `StudentTable` contains a list of students and their test scores.

To test the examples in this document, first add the following sample tables to
your project:

    -- Create a table called ExamTable.
    CREATE OR REPLACE TABLE mydataset.ExamTable AS (
      SELECT "Hansen" AS last_name, "P91" AS test_id, 510 AS test_score UNION ALL
      SELECT "Wang", "U25", 500 UNION ALL
      SELECT "Wang", "C83", 520 UNION ALL
      SELECT "Wang", "U25", 460 UNION ALL
      SELECT "Hansen", "C83", 420 UNION ALL
      SELECT "Hansen", "C83", 560 UNION ALL
      SELECT "Devi", "U25", 580 UNION ALL
      SELECT "Devi", "P91", 480 UNION ALL
      SELECT "Ivanov", "U25", 490 UNION ALL
      SELECT "Ivanov", "P91", 540 UNION ALL
      SELECT "Silva", "U25", 550);

    -- Create a table called StudentTable.
    CREATE OR REPLACE TABLE mydataset.StudentTable AS (
      SELECT "Hansen" AS last_name, 510 AS test_score UNION ALL
      SELECT "Wang", 500 UNION ALL
      SELECT "Devi", 580 UNION ALL
      SELECT "Ivanov", 490 UNION ALL
      SELECT "Silva", 550);

## Limitations

Analysis rules have the following limitations:

- If you've already added an analysis rule to a view, you can't switch between aggregation threshold analysis rules and differential privacy analysis rules.

An aggregation threshold analysis rule has the following limitations:

- You can only use [supported aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_policy_functions) in a query on an [aggregation threshold analysis rule--enforced view](https://docs.cloud.google.com/bigquery/docs/analysis-rules#view_in_privacy_query).
- You can't add an aggregation threshold analysis rule to a materialized view.
- If you use an aggregation threshold analysis rule--enforced view in an aggregation threshold query, they must both have the same value for the privacy unit column.
- If you use an aggregation threshold analysis rule--enforced view in an aggregation threshold query, the threshold in the query must be greater than or equal to the threshold in the view.
- [Time travel](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel) is disabled on any view that has an aggregation threshold analysis rule.

A differential privacy analysis rule has the following limitations:

- Once a privacy budget is exhausted for a view, that view can't be used and you must create a new view.

A list overlap analysis rule has the following limitations:

- If you combine an aggregation threshold analysis rule or a differential privacy analysis rule with a list overlap analysis rule and you don't place the `privacy_unit_column` as a `join_allowed_column` in the list overlap analysis rule, you might not be able to join any columns in certain situations.

## Pricing

- There is no additional cost to attach an analysis rule to a view.
- Standard [BigQuery pricing](https://cloud.google.com/bigquery/pricing) for analysis applies.