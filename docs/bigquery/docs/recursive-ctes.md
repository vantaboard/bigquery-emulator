In GoogleSQL for BigQuery, a `WITH` clause contains one or more common table
expressions (CTEs) that you can reference in a query
expression. CTEs can be [non-recursive](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#simple_cte),
[recursive](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_cte), or both. The [`RECURSIVE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_keyword)
keyword enables recursion in the `WITH` clause (`WITH RECURSIVE`).

A recursive CTE can reference itself, a preceding CTE, or a subsequent CTE. A
non-recursive CTE can reference only preceding CTEs and can't reference itself.
Recursive CTEs run continuously until no new results are found, while
non-recursive CTEs run once. For these reasons, recursive CTEs are commonly used
for querying hierarchical data and graph data.

For example, consider a graph where each row represents a node that can link to
other nodes. To find the transitive closure of all reachable nodes from a
particular start node without knowing the maximum number of hops, you would need
a recursive CTE in the query (`WITH RECURSIVE`). The recursive query would start
with the base case of the start node, and each step would compute the new unseen
nodes that can be reached from all the nodes seen so far up to the previous
step. The query concludes when no new nodes can be found.

However, recursive CTEs can be computationally expensive, so before you use
them, review this guide and the [`WITH` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause) section of the
GoogleSQL reference documentation.

## Create a recursive CTE

To create a recursive CTE in GoogleSQL, use the
[`WITH RECURSIVE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause) as shown in the following example:

    WITH RECURSIVE
      CTE_1 AS (
        (SELECT 1 AS iteration UNION ALL SELECT 1 AS iteration)
        UNION ALL
        SELECT iteration + 1 AS iteration FROM CTE_1 WHERE iteration < 3
      )
    SELECT iteration FROM CTE_1
    ORDER BY 1 ASC

The preceding example produces the following results:

    /*---+
     | iteration |
     +---+
     | 1         |
     | 1         |
     | 2         |
     | 2         |
     | 3         |
     | 3         |
     +---*/

A recursive CTE includes a base term, a union operator, and a recursive term.
The base term runs the first iteration of the recursive union operation. The
recursive term runs the remaining iterations and must include one self-reference
to the recursive CTE. Only the recursive term can include a self-reference.

In the preceding example, the recursive CTE contains the following components:

- Recursive CTE name: `CTE_1`
- Base term: `SELECT 1 AS iteration`
- Union operator: `UNION ALL`
- Recursive term: `SELECT iteration + 1 AS iteration FROM CTE_1 WHERE
  iteration < 3`

To learn more about the recursive CTE syntax, rules, and examples, see [`WITH`
clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause) in the GoogleSQL reference documentation.

## Explore reachability in a directed acyclic graph (DAG)

You can use a recursive query to explore reachability in a
directed acyclic graph (DAG). The following query finds all nodes that can be
reached from node `5` in a graph called `GraphData`:

    WITH RECURSIVE
      GraphData AS (
        --    1          5
        --   / \        / \
        --  2 - 3      6   7
        --      |       \ /
        --      4        8
        SELECT 1 AS from_node, 2 AS to_node UNION ALL
        SELECT 1, 3 UNION ALL
        SELECT 2, 3 UNION ALL
        SELECT 3, 4 UNION ALL
        SELECT 5, 6 UNION ALL
        SELECT 5, 7 UNION ALL
        SELECT 6, 8 UNION ALL
        SELECT 7, 8
      ),
      R AS (
        (SELECT 5 AS node)
        UNION ALL
        (
          SELECT GraphData.to_node AS node
          FROM R
          INNER JOIN GraphData
            ON (R.node = GraphData.from_node)
        )
      )
    SELECT DISTINCT node FROM R ORDER BY node;

The preceding example produces the following results:

    /*---+
     | node |
     +---+
     | 5    |
     | 6    |
     | 7    |
     | 8    |
     +---*/

## Troubleshoot iteration limit errors

Recursive CTEs can result in infinite recursion, which occurs when the recursive
term executes continuously without meeting a termination condition. To terminate
infinite recursions, a limit of iterations for each recursive CTE is
enforced. For BigQuery, the
iteration limit is 500 iterations. Once a recursive CTE reaches
the maximum number of iterations, the CTE execution is aborted with an error.

This limit exists because the computation of a recursive CTE can be expensive,
and running a CTE with a large number of iterations consumes a lot of system
resources and takes a much longer time to finish.

Queries that reach the iteration limit are usually missing a proper termination
condition, thus creating an infinite loop, or using recursive CTEs in
inappropriate scenarios.

If you experience a recursion iteration limit error, review the suggestions in
this section.

### Check for infinite recursion

To prevent infinite recursion, make sure the recursive term is
able to produce an empty result after executing a certain number of iterations.

One way to check for infinite recursion is to
convert your recursive CTE to a `TEMP TABLE` with a `REPEAT` loop for the
first `100` iterations, as follows:

```sql
DECLARE current_iteration INT64 DEFAULT 0;

CREATE TEMP TABLE recursive_cte_name AS
SELECT base_expression, current_iteration AS iteration;

REPEAT
  SET current_iteration = current_iteration + 1;
  INSERT INTO recursive_cte_name
    SELECT recursive_expression, current_iteration
    FROM recursive_cte_name
    WHERE termination_condition_expression
      AND iteration = current_iteration - 1
      AND current_iteration < 100;
  UNTIL NOT EXISTS(SELECT * FROM recursive_cte_name WHERE iteration = current_iteration)
END REPEAT;
```

Replace the following values:

- `recursive_cte_name`: The recursive CTE to debug.
- `base_expression`: The base term of the recursive CTE.
- `recursive_expression`: The recursive term of the recursive CTE.
- `termination_condition_expression`: The termination expression of the recursive CTE.

For example, consider the following recursive CTE called `TestCTE`:

    WITH RECURSIVE
      TestCTE AS (
        SELECT 1 AS n
        UNION ALL
        SELECT n + 3 FROM TestCTE WHERE MOD(n, 6) != 0
      )

This example uses the following values:

- `recursive_cte_name`: `TestCTE`
- `base_expression`: `SELECT 1`
- `recursive_expression`: `n + 3`
- `termination_condition_expression`: `MOD(n, 6) != 0`

The following code would therefore test the `TestCTE` for infinite recursion:

```sql
DECLARE current_iteration INT64 DEFAULT 0;

CREATE TEMP TABLE TestCTE AS
SELECT 1 AS n, current_iteration AS iteration;

REPEAT
SET current_iteration = current_iteration + 1;

INSERT INTO TestCTE
SELECT n + 3, current_iteration
FROM TestCTE
WHERE
  MOD(n, 6) != 0
  AND iteration = current_iteration - 1
  AND current_iteration < 10;

UNTIL
  NOT EXISTS(SELECT * FROM TestCTE WHERE iteration = current_iteration)
    END REPEAT;

-- Print the number of rows produced by each iteration

SELECT iteration, COUNT(1) AS num_rows
FROM TestCTE
GROUP BY iteration
ORDER BY iteration;

-- Examine the actual result produced for a specific iteration

SELECT * FROM TestCTE WHERE iteration = 2;
```

The preceding example produces the following results that include the
iteration ID and the number of rows that were produced during that iteration:

    /*---+---+
     | iteration | num_rows |
     +---+---+
     | 0         | 1        |
     | 1         | 1        |
     | 2         | 1        |
     | 3         | 1        |
     | 4         | 1        |
     | 5         | 1        |
     | 6         | 1        |
     | 7         | 1        |
     | 8         | 1        |
     | 9         | 1        |
     | 10        | 1        |
     +---+---*/

These are the actual results produced during iteration `2`:

    /*---+---+
     | n        | iteration |
     +---+---+
     | 7        | 2         |
     +---+---*/

If the number of rows is always greater than zero, which is true in this
example, then the sample likely has an infinite recursion.

### Verify the appropriate usage of the recursive CTE

Verify that you're using the recursive CTE in an appropriate scenario.
Recursive CTEs can be expensive to compute because they're designed to query
hierarchical data and graph data. If you aren't querying these two kinds of
data, consider alternatives, such as using the
[`LOOP` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#loop) with a non-recursive CTE.

### Split a recursive CTE into multiple recursive CTEs

If you think your recursive CTE needs more than the maximum allowed
iterations, you might be able to break down your recursive CTE into multiple
recursive CTEs.

You can split a recursive CTE with a query structure similar to the following:

```sql
WITH RECURSIVE
  CTE_1 AS (
    SELECT base_expression
    UNION ALL
    SELECT recursive_expression FROM CTE_1 WHERE iteration < 500
  ),
  CTE_2 AS (
    SELECT * FROM CTE_1 WHERE iteration = 500
    UNION ALL
    SELECT recursive_expression FROM CTE_2 WHERE iteration < 500 * 2
  ),
  CTE_3 AS (
    SELECT * FROM CTE_2 WHERE iteration = 500 * 2
    UNION ALL
    SELECT recursive_expression FROM CTE_3 WHERE iteration < 500 * 3
  ),
  [, ...]
SELECT * FROM CTE_1
UNION ALL SELECT * FROM CTE_2 WHERE iteration > 500
UNION ALL SELECT * FROM CTE_3 WHERE iteration > 500 * 2
[...]
```

Replace the following values:

- `base_expression`: The base term expression for the current CTE.
- `recursive_expression`: The recursive term expression for the current CTE.

For example, the following code splits a CTE into three distinct CTEs:

    WITH RECURSIVE
      CTE_1 AS (
        SELECT 1 AS iteration
        UNION ALL
        SELECT iteration + 1 AS iteration FROM CTE_1 WHERE iteration < 10
      ),
      CTE_2 AS (
        SELECT * FROM CTE_1 WHERE iteration = 10
        UNION ALL
        SELECT iteration + 1 AS iteration FROM CTE_2 WHERE iteration < 10 * 2
      ),
      CTE_3 AS (
        SELECT * FROM CTE_2 WHERE iteration = 10 * 2
        UNION ALL
        SELECT iteration + 1 AS iteration FROM CTE_3 WHERE iteration < 10 * 3
      )
    SELECT iteration FROM CTE_1
    UNION ALL
    SELECT iteration FROM CTE_2 WHERE iteration > 10
    UNION ALL
    SELECT iteration FROM CTE_3 WHERE iteration > 20
    ORDER BY 1 ASC

In the preceding example, 500 iterations is replaced with 10 iterations
so that it's faster to see the results of the query. The query produces 30 rows,
but each recursive CTE only iterates 10 times. The output looks like the
following:

    /*---+
     | iteration |
     +---+
     | 2         |
     | ...       |
     | 30        |
     +---*/

You could test the previous query on much larger iterations.

### Use a loop instead of a recursive CTE

To avoid iteration limits, consider using a loop instead of a recursive CTE.
You can create a loop with one of several loop statements,
such as `LOOP`, `REPEAT`, or `WHILE`. For more information, see
[Loops](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#loops).

### Change the recursive limit

If you think the following factors apply, contact Customer Care to
raise the recursive limit:

- You have a valid reason for your recursive CTE to run more than 500 iterations.
- You're OK with a much longer execution.

Keep in mind that raising the recursive limit has potential risks:

- Your CTE might fail with a different error message, such as memory exceeded or timeout.
- If your project is using the on-demand pricing model, your CTE might still fail with a billing tier error until you switch to the capacity-based pricing model.
- A recursive CTE with a large number of iterations consumes a lot of resources. This might impact other queries that are running within the same reservation, since they compete for shared resources.

## Pricing

If you use [on-demand billing](https://cloud.google.com/bigquery/pricing#on_demand_pricing),
BigQuery charges based on the number of bytes
that are processed during execution of a query with a recursive CTE.

For more information, see
[Query size calculation](https://docs.cloud.google.com/bigquery/docs/estimate-costs#query_size_calculation).

## Quotas

For information about recursive CTE quotas and limits, see
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas#recursive-ctes-limits).