# Manage vector indexes

This document describes how to create and manage vector indexes to
accelerate your vector searches.

A vector index is a data structure designed to let the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
and [`AI.SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search)
execute more efficiently, especially on large datasets.
When using an index, these search functions use
[Approximate Nearest Neighbor (ANN)](https://en.wikipedia.org/wiki/Nearest_neighbor_search#Approximation_methods)
algorithms to reduce query latency and computational cost. While ANN introduces
a degree of approximation, meaning that
[recall](https://developers.google.com/machine-learning/crash-course/classification/precision-and-recall#recallsearch_term_rules)
might not be 100%, the performance improvements typically offer an advantage
for most applications.

## Roles and permissions

To create a vector index, you need the
[`bigquery.tables.createIndex` IAM permission](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
on the table where you're creating the index. To drop a vector index, you need
the `bigquery.tables.deleteIndex` permission. Each of the following predefined
IAM roles includes the permissions that you need to work with
vector indexes:

- BigQuery Data Owner (`roles/bigquery.dataOwner`)
- BigQuery Data Editor (`roles/bigquery.dataEditor`)

## Choose a vector index type

BigQuery offers two vector index types, [IVF](https://docs.cloud.google.com/bigquery/docs/vector-index#ivf-index) and
[TreeAH](https://docs.cloud.google.com/bigquery/docs/vector-index#tree-ah-index), each supporting
different use cases. BigQuery supports batching for vector
search by processing multiple rows of the input data in the
[`VECTOR_SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search).
For small query batches, IVF indexes are preferred. For large query batches,
TreeAH indexes, which are built with Google's
[ScaNN algorithm](https://github.com/google-research/google-research/blob/master/scann/docs/algorithms.md),
are preferred.

### IVF Index

IVF is an inverted file index, which uses a k-means algorithm to cluster the
vector data, and then partitions the vector data based on those clusters. The
`VECTOR_SEARCH` and `AI.SEARCH` functions
can use these partitions to reduce the amount of data it needs to read in order
to determine a result.

### TreeAH Index

The TreeAH index type is named for its combination of a tree-like structure
and its use of Asymmetric Hashing (AH), a core quantization technique from the
underlying
[ScaNN algorithm](https://github.com/google-research/google-research/blob/master/scann/docs/algorithms.md).
A TreeAH index works as follows:

1. The base table is divided into smaller, more manageable shards.
2. A clustering model is trained, with the number of clusters derived from the `leaf_node_embedding_count` option in the `tree_ah_options` argument of the `CREATE VECTOR INDEX` statement.
3. The vectors are compressed with product quantization, a technique that reduces their memory usage. The compressed vectors are then stored in the index tables instead of the original vectors, thus reducing vector index sizes.
4. When the `VECTOR_SEARCH` or `AI.SEARCH` function runs, a candidate list for each query vector is efficiently computed using asymmetric hashing, which is hardware-optimized for approximate distance calculations. These candidates are then re-scored and re-ranked using exact embeddings.

The TreeAH algorithm is optimized for batch queries that process hundreds or
more query vectors. The use of product quantization can significantly reduce
latency and cost, potentially by orders of magnitude compared to IVF. However,
due to increased overhead, the IVF algorithm might be better when you have a
smaller number of query vectors.

We suggest you try the TreeAH index type if your use case meets the following
criteria:

- Your table contains 200 million rows or fewer.

- You frequently execute large batch queries involving hundreds or more query
  vectors.

For small batch queries with the TreeAH index type, `VECTOR_SEARCH` or
`AI.SEARCH` might
revert to
[brute-force search](https://wikipedia.org/wiki/Brute-force_search).
When this occurs, an
[IndexUnusedReason](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#IndexUnusedReason)
is provided to explain why the vector index was not utilized.

## Create an IVF vector index

To create an IVF vector index, use the
[`CREATE VECTOR INDEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement)
data definition language (DDL) statement:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement:

   To create an [IVF](https://docs.cloud.google.com/bigquery/docs/vector-index#ivf-index) vector index:

   ```googlesql
   CREATE [ OR REPLACE ] VECTOR INDEX [ IF NOT EXISTS ] INDEX_NAME
   ON DATASET_NAME.TABLE_NAME(COLUMN_NAME)
   STORING(STORED_COLUMN_NAME [, ...])
   OPTIONS(index_type = 'IVF',
     distance_type = 'DISTANCE_TYPE',
     ivf_options = '{"num_lists":NUM_LISTS}')
   ```

   Replace the following:
   - `INDEX_NAME`: the name of the vector index you're creating. Since the index is always created in the same project and dataset as the base table, there is no need to specify these in the name.
   - `DATASET_NAME`: the name of the dataset that contains the table.
   - `TABLE_NAME`: the name of the table that contains the column with embeddings data.
   - `COLUMN_NAME`: the name of a column that contains
     the embeddings data. The column must have a type of `ARRAY<FLOAT64>`, or
     if you are using
     [autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation),
     a type of `STRUCT<result ARRAY<FLOAT64>, status STRING>`.

     In all cases, all elements in the
     embedding array must be non-`NULL`, and all values in the column must
     have the same array dimensions.

     If the column type is `STRUCT<result ARRAY<FLOAT64>, status STRING>`, then
     the `STRUCT` value can be `NULL` or the `result` array can be `NULL`. Any
     rows with `NULL` for these values are ignored.
   - `STORED_COLUMN_NAME`: the name of a top-level column
     in the table to store in the vector index. The column type can't be
     `RANGE`. Stored columns are not used if the table has a row-level access
     policy or the column has a policy tag. For information about how to
     enable stored columns, see
     [Store columns and pre-filter](https://docs.cloud.google.com/bigquery/docs/vector-index#stored-columns).

   - `DISTANCE_TYPE`: specifies the default distance type
     to use when performing a vector search using this index. The supported
     values are
     [`EUCLIDEAN`](https://en.wikipedia.org/wiki/Euclidean_distance),
     [`COSINE`](https://en.wikipedia.org/wiki/Cosine_similarity#Cosine_Distance),
     and
     [`DOT_PRODUCT`](https://en.wikipedia.org/wiki/Dot_product).
     `EUCLIDEAN` is the default.

     The index creation itself always uses `EUCLIDEAN` distance for training
     but the distance used in your search function can be different.

     If you specify a value for the `distance_type` argument of the
     `VECTOR_SEARCH` or `AI.SEARCH` function, that value is used instead of
     the `DISTANCE_TYPE` value.
   - `NUM_LISTS`: an `INT64` value that specifies the
     number of lists that the IVF index clusters and then partitions your
     vector data into. This value must be 5,000 or less. During indexing,
     vectors are assigned to the list corresponding to their nearest cluster
     centroid. If you omit this argument, BigQuery determines a
     default value based on your data characteristics. The default value works
     well for most use cases.

     `NUM_LISTS` controls query tuning granularity.
     Higher values create more lists, so you can set the
     `fraction_lists_to_search` option of your search function to scan
     a smaller percentage of the index. For example, scanning 1% of 100 lists
     as opposed to scanning 10% of 10 lists. This enables finer control of the
     search speed and recall but slightly increases the indexing cost. Set this
     argument value based on how precisely you need to tune query scope.

The following example creates a vector index on the `embedding` column
of `my_table`:

```googlesql
CREATE TABLE my_dataset.my_table(embedding ARRAY<FLOAT64>);

CREATE VECTOR INDEX my_index ON my_dataset.my_table(embedding)
OPTIONS(index_type = 'IVF');
```

The following example creates a vector index on the `embedding` column
of `my_table`, and specifies the distance type to use and the IVF options:

```googlesql
CREATE TABLE my_dataset.my_table(embedding ARRAY<FLOAT64>);

CREATE VECTOR INDEX my_index ON my_dataset.my_table(embedding)
OPTIONS(index_type = 'IVF', distance_type = 'COSINE',
ivf_options = '{"num_lists": 2500}')
```

The following example creates a table with
[autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation)
enabled and creates a vector index on the table. The `description_embedding`
embedding column is automatically generated based on the `description` column.

```googlesql
CREATE TABLE mydataset.products (
  description STRING,
  description_embedding STRUCT<result ARRAY<FLOAT64>, status STRING>
    GENERATED ALWAYS AS (
      AI.EMBED(description, connection_id => 'us.example_connection',
        endpoint => 'text-embedding-005'))
    STORED OPTIONS( asynchronous = TRUE ));

CREATE VECTOR INDEX my_index ON my_dataset.my_table(description_embedding)
OPTIONS(index_type = 'IVF');
```

## Create a TreeAH vector index

To create a TreeAH vector index, use the
[`CREATE VECTOR INDEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement)
data definition language (DDL) statement:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement:

   ```googlesql
   CREATE [ OR REPLACE ] VECTOR INDEX [ IF NOT EXISTS ] INDEX_NAME
   ON DATASET_NAME.TABLE_NAME(COLUMN_NAME)
   STORING(STORED_COLUMN_NAME [, ...])
   OPTIONS(index_type = 'TREE_AH',
     distance_type = 'DISTANCE_TYPE',
     tree_ah_options = '{"leaf_node_embedding_count":LEAF_NODE_EMBEDDING_COUNT,
       "normalization_type":"NORMALIZATION_TYPE"}')
   ```

   Replace the following:
   - `INDEX_NAME`: the name of the vector index that you are creating. Since the index is always created in the same project and dataset as the base table, there is no need to specify these in the name.
   - `DATASET_NAME`: the name of the dataset that contains the table.
   - `TABLE_NAME`: the name of the table that contains the column with embeddings data.
   - `COLUMN_NAME`: the name of a column that contains
     the embeddings data. The column must have a type of `ARRAY<FLOAT64>`, or
     if you are using
     [autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation),
     a type of `STRUCT<result ARRAY<FLOAT64>, status STRING>`.

     In all cases, all elements in the
     embedding array must be non-`NULL`, and all values in the column must
     have the same array dimensions. The array dimension must be at least 2.

     If the column type is `STRUCT<result ARRAY<FLOAT64>, status STRING>`, then
     the `STRUCT` value can be `NULL` or the `result` array can be `NULL`. Any
     rows with `NULL` for these values are ignored.
   - `STORED_COLUMN_NAME`: the name of a top-level column
     in the table to store in the vector index. The column type can't be
     `RANGE`. Stored columns are not used if the table has a row-level access
     policy or the column has a policy tag. For information about how to
     enable stored columns, see
     [Store columns and pre-filter](https://docs.cloud.google.com/bigquery/docs/vector-index#stored-columns).

   - `DISTANCE_TYPE`: an optional argument that
     specifies the default distance type
     to use when performing a vector search using this index. The supported
     values are
     [`EUCLIDEAN`](https://en.wikipedia.org/wiki/Euclidean_distance),
     [`COSINE`](https://en.wikipedia.org/wiki/Cosine_similarity#Cosine_Distance),
     and
     [`DOT_PRODUCT`](https://en.wikipedia.org/wiki/Dot_product).
     `EUCLIDEAN` is the default.

     The index creation itself always uses `EUCLIDEAN` distance for training
     but the distance used in the search function can be different.

     If you specify a value for the `distance_type` argument of the
     `VECTOR_SEARCH` or `AI.SEARCH` function, that value is used instead of
     the `DISTANCE_TYPE` value.
   - `LEAF_NODE_EMBEDDING_COUNT`: an `INT64` value
     greater than or equal to 500 that specifies the approximate number of
     vectors in each leaf node of the tree that the TreeAH algorithm creates.
     The TreeAH algorithm divides the whole data space into a number of lists,
     with each list containing approximately `LEAF_NODE_EMBEDDING_COUNT`
     data points. A lower value creates more lists with fewer data points, while a
     larger value creates fewer lists with more data points. The default is
     1,000, which is appropriate for most datasets.

   - `NORMALIZATION_TYPE`: a `STRING` value. The
     supported values are `NONE` or
     [`L2`](https://en.wikipedia.org/wiki/Norm_(mathematics)#Euclidean_norm).
     The default is `NONE`. Normalization happens before any processing, for
     both the base table data and the query data, but doesn't modify the
     embedding column `COLUMN_NAME` in
     `TABLE_NAME`. Depending on the dataset, the
     embedding model, and the distance type used during
     search, normalizing the embeddings might improve recall.

The following example creates a vector index on the `embedding` column
of `my_table`, and specifies the distance type to use and the TreeAH options:

```googlesql
CREATE TABLE my_dataset.my_table(id INT64, embedding ARRAY<FLOAT64>);

CREATE VECTOR INDEX my_index ON my_dataset.my_table(embedding)
OPTIONS (index_type = 'TREE_AH', distance_type = 'EUCLIDEAN',
tree_ah_options = '{"normalization_type": "L2"}');
```

## Filtering

The following sections explain how pre-filters and post-filters affect
vector search results, and also how to pre-filter by using stored columns
and partitions in the vector index.

### Pre-filters and post-filters

In BigQuery `VECTOR_SEARCH` or `AI.SEARCH` calls,
both pre-filtering and
post-filtering serve to refine search results, by applying conditions based on
metadata columns associated with the vector embeddings. It is important to
understand their differences, implementation, and impact in order to optimize
query performance, cost, and accuracy.

Pre-filtering and post-filtering are defined as follows:

- **Pre-filtering:** Applies filter conditions before the approximate nearest neighbor (ANN) search performs distance calculations on candidate vectors. This narrows the pool of vectors that are considered during the search. Consequently, pre-filtering often results in faster query times and reduced computational cost, as the ANN search evaluates fewer potential candidates.
- **Post-filtering:** Applies filter conditions after the initial `top_k` nearest neighbors have been identified by the ANN search. This refines the final result set based on the specified criteria.

The placement of your `WHERE` clause determines whether a filter acts as a
pre-filter or a post-filter.

To create a pre-filter, the `WHERE` clause of the query must apply to the
base table argument of the search function.
The predicate must apply to a stored column, otherwise it effectively becomes
a post-filter.

The following example shows how to create a pre-filter:

```googlesql
-- Pre-filter on a stored column. The index speeds up the query.
SELECT *
FROM
  VECTOR_SEARCH(
    (SELECT * FROM my_dataset.my_table WHERE type = 'animal'),
    'embedding',
    TABLE my_dataset.my_testdata);

SELECT *
FROM
  AI.SEARCH(
    (SELECT * FROM my_dataset.my_table WHERE type = 'animal'),
    'content',
    'dog');

-- Filter on a column that isn't stored. The index is used to search the
-- entire table, and then the results are post-filtered. You might see fewer
-- than 5 matches returned for some embeddings.
SELECT query.test_id, base.type, distance
FROM
  VECTOR_SEARCH(
    (SELECT * FROM my_dataset.my_table WHERE id = 123),
    'embedding',
    TABLE my_dataset.my_testdata,
    top_k => 5);

-- Use pre-filters with brute force. The data is filtered and then searched
-- with brute force for exact results.
SELECT query.test_id, base.type, distance
FROM
  VECTOR_SEARCH(
    (SELECT * FROM my_dataset.my_table WHERE id = 123),
    'embedding',
    TABLE my_dataset.my_testdata,
    options => '{"use_brute_force":true}');
```

To create a post-filter, the `WHERE` clause of the query must be applied outside
of the `VECTOR_SEARCH` function, so that it filters the results returned by
the search.

The following example shows how to create a post-filter:

```googlesql
-- Use post-filters. The index is used, but the entire table is searched and
-- the post-filtering might reduce the number of results.
SELECT query.test_id, base.type, distance
FROM
  VECTOR_SEARCH(
    TABLE my_dataset.my_table,
    'embedding',
    TABLE my_dataset.my_testdata,
    top_k => 5)
WHERE base.type = 'animal';

SELECT base.id, distance
FROM
  VECTOR_SEARCH(
    TABLE mydataset.base_table,
    'embedding',
    (SELECT embedding FROM mydataset.query_table),
    top_k => 10
  )
WHERE type = 'document' AND year > 2022
```

When you use post-filtering, or when the base table filters you specify
reference non-stored columns and thus act as post-filters, the final result
set might contain fewer than `top_k` rows, potentially even zero rows,
if the predicate is selective. If you require a specific number of results
after filtering, consider specifying a larger `top_k` value or increasing the
`fraction_lists_to_search` value in the search function call.

In some cases, especially if the pre-filter is very selective, pre-filtering
can also reduce the size of the result set. If this happens, try increasing the
`fraction_lists_to_search` value in the search function call.

### Pre-filter with stored columns

To further improve the efficiency of your vector index, you can specify columns
from your base table to store in your vector index. Using stored columns can
optimize queries that call the `VECTOR_SEARCH` or `AI.SEARCH`
functions in the following ways:

- Instead of searching an entire table, you can call a search
  function on a query statement that *pre-filters* the base table with a
  `WHERE` clause. If your table has an index and you filter on only stored
  columns, then BigQuery optimizes the query by filtering the
  data before searching, and then using the index to search the smaller result
  set. If you filter on columns that aren't stored, then
  BigQuery applies the filter after the table is searched, or
  *post-filters*.

- The `VECTOR_SEARCH` and `AI.SEARCH` functions output a struct called `base`
  that contains
  all columns from the base table. Without stored columns, a potentially
  expensive join is needed to retrieve the columns stored in `base`. If
  you use an IVF index and your query only selects stored columns
  from `base`, then BigQuery optimizes your query to eliminate
  that join. For TreeAH indexes, the join with the base table is not removed.
  Stored columns in TreeAH indexes are only used for pre-filtering purposes.

To store columns, list them in the `STORING` clause of the
[`CREATE VECTOR INDEX` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement).
Storing columns increases the size of the vector index, so it's best
to store only the most frequently used or filtered columns.

The following example creates a vector index with stored columns, and then runs
a vector search query that only selects stored columns:

```googlesql
-- Create a table that contains an embedding.
CREATE TABLE my_dataset.my_table(embedding ARRAY<FLOAT64>, type STRING, creation_time DATETIME, id INT64);

-- Create a query table that contains an embedding.
CREATE TABLE my_dataset.my_testdata(embedding ARRAY<FLOAT64>, test_id INT64);

-- Create a vector index with stored columns.
CREATE VECTOR INDEX my_index ON my_dataset.my_table(embedding)
STORING (type, creation_time)
OPTIONS (index_type = 'IVF');

-- Select only stored columns from a vector search to avoid an expensive join.
SELECT query, base.type, distance
FROM
  VECTOR_SEARCH(
    TABLE my_dataset.my_table,
    'embedding'
    TABLE my_dataset.my_testdata);
```

#### Stored column limitations

- If the mode, type, or schema of a column is changed in the base table, and if it is a stored column in the vector index, then there can be a delay before that change is reflected in the vector index. Until the updates have been applied to the index, the vector search queries use the modified stored columns from the base table.
- If you select a column of type `STRUCT` from the `query` output of a search query on a table that has an index with stored columns, then the whole query might fail.

### Pre-filter with partitions

If the table that you are creating the vector index on is partitioned, you can
choose to also partition the vector index. Partitioning the vector index has
the following benefits:

- Partition pruning is applied to the vector indexes in addition to the table partitions. Partition pruning occurs when the vector search uses a qualifying filter on the value of the partitioning column. This allows BigQuery to scan the partitions that match the filter and skip the remaining partitions. Partition pruning can decrease I/O costs. For more information on partition pruning, see [Query partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).
- The vector search is less likely to miss results if you pre-filter on the partitioning column.

You can only partition TreeAH vector indexes. You can't create a partitioned
vector index on an
[automatically generated embedding column](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation).

Partitioning a vector index is only recommended if you use pre-filtering to
limit most of your vector searches to a few partitions.

To create a partitioned index, use the `PARTITION BY` clause of the
[`CREATE VECTOR INDEX` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement). The
`PARTITION BY` clause that you specify in the `CREATE VECTOR INDEX` statement
must be the same as the `PARTITION BY` clause specified in the
[`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
of the table that you are creating the vector index on, as shown in the
following example:

```googlesql
-- Create a date-partitioned table.
CREATE TABLE my_dataset.my_table(
  embeddings ARRAY,
  id INT64,
  date DATE,
)
PARTITION BY date;

-- Create a partitioned vector index on the table.
CREATE VECTOR INDEX my_index ON my_dataset.my_table(embeddings)
PARTITION BY date
OPTIONS(index_type='TREE_AH', distance_type='COSINE');
```

If the table uses integer range or time-unit column partitioning, the
partitioning column is stored in the vector index, which increases storage cost.
If a table column is used in both the `STORING` and `PARTITION BY` clauses of
the `CREATE VECTOR INDEX` statement, the column is stored only once.

To use the vector index partition, filter on the partitioning column in the
base table subquery of the `VECTOR_SEARCH` or `AI.SEARCH` call.
In the following example,
the `samples.items` table is partitioned by the `produced_date` column,
so the base table subquery in the `VECTOR_SEARCH` statement filters on the
`produced_date` column:

```googlesql
SELECT query.id, base.id, distance
FROM VECTOR_SEARCH(
  (SELECT * FROM my_dataset.my_table WHERE produced_date = '2025-01-01'),
  'embedding',
  TABLE samples.test,
  distance_type => 'COSINE',
  top_k => 10
);
```

#### Examples

Create a partitioned vector index on a datetime-partitioned table:

```googlesql
-- Create a datetime-partitioned table.
CREATE TABLE my_dataset.my_table(
  id INT64,
  produced_date DATETIME,
  embeddings ARRAY
)
PARTITION BY produced_date;

-- Create a partitioned vector index on the table.
CREATE VECTOR INDEX index0 ON my_dataset.my_table(embeddings)
PARTITION BY produced_date
OPTIONS(index_type='TREE_AH', distance_type='COSINE');
```

Create a partitioned vector index on a timestamp-partitioned table:

```googlesql
-- Create a timestamp-partitioned table.
CREATE TABLE my_dataset.my_table(
  id INT64,
  produced_time TIMESTAMP,
  embeddings ARRAY
)
PARTITION BY TIMESTAMP_TRUNC(produced_time, HOUR);

-- Create a partitioned vector index on the table.
CREATE VECTOR INDEX index0 ON my_dataset.my_table(embeddings)
PARTITION BY TIMESTAMP_TRUNC(produced_time, HOUR)
OPTIONS(index_type='TREE_AH', distance_type='COSINE');
```

Create a partitioned vector index on an integer range-partitioned table:

```googlesql
-- Create an integer range-partitioned table.
CREATE TABLE my_dataset.my_table(
  id INT64,
  embeddings ARRAY
)
PARTITION BY RANGE_BUCKET(id, GENERATE_ARRAY(-100, 100, 20));

-- Create a partitioned vector index on the table.
CREATE VECTOR INDEX index0 ON my_dataset.my_table(embeddings)
PARTITION BY RANGE_BUCKET(id, GENERATE_ARRAY(-100, 100, 20))
OPTIONS(index_type='TREE_AH', distance_type='COSINE');
```

Create a partitioned vector index on an ingestion time-partitioned table:

```googlesql
-- Create an ingestion time-partitioned table.
CREATE TABLE my_dataset.my_table(
  id INT64,
  embeddings ARRAY
)
PARTITION BY TIMESTAMP_TRUNC(_PARTITIONTIME, DAY);

-- Create a partitioned vector index on the table.
CREATE VECTOR INDEX index0 ON my_dataset.my_table(embeddings)
PARTITION BY TIMESTAMP_TRUNC(_PARTITIONTIME, DAY)
OPTIONS(index_type='TREE_AH', distance_type='COSINE');
```

### Pre-filtering limitations

- You can't use [logical views](https://docs.cloud.google.com/bigquery/docs/views-intro) in your pre-filter.
- If your pre-filter contains a [subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries), it might interfere with index usage.

## Understanding when data is indexed

Vector indexes are fully managed by BigQuery and are
automatically refreshed when the indexed table changes.

Indexing is asynchronous. There is a delay between adding new rows to the
base table and the new rows being reflected in the index. However, the
`VECTOR_SEARCH` and `AI.SEARCH` functions still take all rows into account and
don't miss
unindexed rows. The functions search using the index for indexed records, and
use brute force search for the records that aren't yet indexed.

If you create a vector index on an
[automatically generated embedding column](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation),
then index training starts as soon as at least 80% of the rows
have generated embeddings.

If you create a vector index on a table that is smaller than 10 MB, then the
vector index isn't populated. Similarly, if you delete data from an indexed
table and the table size falls below 10 MB, then the vector index is temporarily
disabled. In this case, vector search queries don't use the index and the
`indexUnusedReasons` code in the
[`vectorSearchStatistics`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#vectorsearchstatistics)
section of the `Job` resource is `BASE_TABLE_TOO_SMALL`. Without the index,
your search function automatically falls back to using brute force to find the
nearest neighbors of embeddings.

If you delete the indexed column in a table, or rename the table itself, the
vector index is automatically deleted.

## Monitoring the status of vector indexes

You can monitor the health of your vector indexes by querying
`INFORMATION_SCHEMA` views. The following views contain metadata on vector
indexes:

- The
  [`INFORMATION_SCHEMA.VECTOR_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes)
  has information about the vector indexes in a dataset.

  After the `CREATE VECTOR INDEX` statement completes, the index must still
  be populated before you can use it. You can use the `last_refresh_time`
  and `coverage_percentage` columns to verify the readiness of a vector
  index. If the vector index isn't ready, you can still use the
  `VECTOR_SEARCH` and `AI.SEARCH` functions on a table, they just might
  run more slowly without the index.
- The
  [`INFORMATION_SCHEMA.VECTOR_INDEX_COLUMNS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-index-columns)
  has information about the vector-indexed columns for all tables in a dataset.

- The
  [`INFORMATION_SCHEMA.VECTOR_INDEX_OPTIONS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-index-options)
  has information about the options used by the vector indexes in a dataset.

### Vector index examples

The following example shows all active vector indexes on tables in the dataset
`my_dataset`, located in the project `my_project`. It includes their names, the
DDL statements used to create them, and their coverage percentage. If an
indexed base table is less than 10 MB, then its index is not populated, in
which case the `coverage_percentage` value is 0.

```googlesql
SELECT table_name, index_name, ddl, coverage_percentage
FROM my_project.my_dataset.INFORMATION_SCHEMA.VECTOR_INDEXES
WHERE index_status = 'ACTIVE';
```

The result is similar to the following:

```
+---+---+---+---+
| table_name | index_name | ddl                                                                                             | coverage_percentage |
+---+---+---+---+
| table1     | indexa     | CREATE VECTOR INDEX `indexa` ON `my_project.my_dataset.table1`(embeddings)                      | 100                 |
|            |            | OPTIONS (distance_type = 'EUCLIDEAN', index_type = 'IVF', ivf_options = '{"num_lists": 100}')   |                     |
+---+---+---+---+
| table2     | indexb     | CREATE VECTOR INDEX `indexb` ON `my_project.my_dataset.table2`(vectors)                         | 42                  |
|            |            | OPTIONS (distance_type = 'COSINE', index_type = 'IVF', ivf_options = '{"num_lists": 500}')      |                     |
+---+---+---+---+
| table3     | indexc     | CREATE VECTOR INDEX `indexc` ON `my_project.my_dataset.table3`(vectors)                         | 98                  |
|            |            | OPTIONS (distance_type = 'DOT_PRODUCT', index_type = 'TREE_AH',                                 |                     |
|            |            |          tree_ah_options = '{"leaf_node_embedding_count": 1000, "normalization_type": "NONE"}') |                     |
+---+---+---+---+
```

### Vector index columns examples

The following query extracts information on columns that have vector indexes:

```googlesql
SELECT table_name, index_name, index_column_name, index_field_path
FROM my_project.dataset.INFORMATION_SCHEMA.VECTOR_INDEX_COLUMNS;
```

The result is similar to the following:

```
+---+---+---+---+
| table_name | index_name | index_column_name | index_field_path |
+---+---+---+---+
| table1     | indexa     | embeddings        | embeddings       |
| table2     | indexb     | vectors           | vectors          |
| table3     | indexc     | vectors           | vectors          |
+---+---+---+---+
```

### Vector index options examples

The following query extracts information on vector index options:

```googlesql
SELECT table_name, index_name, option_name, option_type, option_value
FROM my_project.dataset.INFORMATION_SCHEMA.VECTOR_INDEX_OPTIONS;
```

The result is similar to the following:

```
+---+---+---+---+---+
| table_name | index_name | option_name      | option_type      | option_value                                                      |
+---+---+---+---+---+
| table1     | indexa     | index_type       | STRING           | IVF                                                               |
| table1     | indexa     | distance_type    | STRING           | EUCLIDEAN                                                         |
| table1     | indexa     | ivf_options      | STRING           | {"num_lists": 100}                                                |
| table2     | indexb     | index_type       | STRING           | IVF                                                               |
| table2     | indexb     | distance_type    | STRING           | COSINE                                                            |
| table2     | indexb     | ivf_options      | STRING           | {"num_lists": 500}                                                |
| table3     | indexc     | index_type       | STRING           | TREE_AH                                                           |
| table3     | indexc     | distance_type    | STRING           | DOT_PRODUCT                                                       |
| table3     | indexc     | tree_ah_options  | STRING           | {"leaf_node_embedding_count": 1000, "normalization_type": "NONE"} |
+---+---+---+---+---+
```

## Verifying vector index usage

Information on vector index usage is available in the job metadata of the
job that ran the vector search query. You can
[view job metadata](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job) by using
the Google Cloud console, the bq command-line tool, the BigQuery API, or the
client libraries.

When you use the Google Cloud console, you can find vector index usage
information in the **Vector Index Usage Mode** and
**Vector Index Unused Reasons** fields.

When you use the bq tool or the BigQuery API, you can
find vector index usage information in the
[`VectorSearchStatistics`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#vectorsearchstatistics)
section of the `Job` resource.

The index usage mode indicates whether a vector index was used by providing
one of the following values:

- `UNUSED`: No vector index was used.
- `PARTIALLY_USED`: Some search functions in the query used vector indexes and some didn't.
- `FULLY_USED`: Every search function in the query used a vector index.

When the index usage mode value is `UNUSED` or `PARTIALLY_USED`,
the index unused reasons indicate why vector indexes weren't used in the query.

For example, the following results returned by
`bq show --format=prettyjson -j my_job_id` shows that the index was not used
because the `use_brute_force` option was specified in the `VECTOR_SEARCH`
function:

```
"vectorSearchStatistics": {
  "indexUnusedReasons": [
    {
      "baseTable": {
        "datasetId": "my_dataset",
        "projectId": "my_project",
        "tableId": "my_table"
      },
      "code": "INDEX_SUPPRESSED_BY_FUNCTION_OPTION",
      "message": "No vector index was used for the base table `my_project:my_dataset.my_table` because use_brute_force option has been specified."
    }
  ],
  "indexUsageMode": "UNUSED"
}
```

Known issue: The vector index usage might be inaccurate when the query is
running, is canceled, or has failed.

## Index management options

To create indexes and have BigQuery maintain them,
you have two options:

- [Use the default shared slot pool](https://docs.cloud.google.com/bigquery/docs/vector-index#use_shared_slots): When the data you plan to index is below your per-organization [limit](https://docs.cloud.google.com/bigquery/quotas#index_limits), you can use the free shared slot pool for index management.
- [Use your own reservation](https://docs.cloud.google.com/bigquery/docs/vector-index#use_your_own_reservation): To achieve more predictable and consistent indexing progress on your larger production workloads, you can use your own reservations for index management.

### Use shared slots

If you have not configured your project to use a
[dedicated reservation](https://docs.cloud.google.com/bigquery/docs/vector-index#use_your_own_reservation) for indexing,
index management is handled in the free, shared slot pool, subject to the
following constraints.

If you add data to a table which causes the total size of indexed
tables to exceed your organization's [limit](https://docs.cloud.google.com/bigquery/quotas#index_limits),
BigQuery pauses index management
for that table. When this happens, the `index_status` field in the

[`INFORMATION_SCHEMA.VECTOR_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes)

displays `PENDING DISABLEMENT` and the index is queued for deletion. While
the index is pending disablement, it is
still used in queries and you are charged for the index storage.
After the index is deleted, the `index_status` field shows
the index as `TEMPORARILY DISABLED`. In this state, queries don't use the index,
and you are not charged for index storage. In this case, the
[`IndexUnusedReason` code](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#indexunusedreason)
is `BASE_TABLE_TOO_LARGE`.

If you delete data from the table and the total size of indexed tables
falls below the per-organization limit, then index management is resumed. The
`index_status` field in the

`INFORMATION_SCHEMA.VECTOR_INDEXES`

view is `ACTIVE`, queries can use the index, and you are charged for the
index storage.

You can use the [`INFORMATION_SCHEMA.SEARCH_INDEXES_BY_ORGANIZATION` view](https://docs.cloud.google.com/bigquery/docs/information-schema-indexes-by-organization)
to understand your current consumption towards the per-organization limit in a
given region, broken down by projects and tables.

BigQuery does not make guarantees about the available
capacity of the shared pool or the throughput of indexing you see.
For production applications, you might want to use
dedicated slots for your index processing.

### Use your own reservation

Instead of using the default shared slot pool, you can optionally designate your
own reservation to index your tables. Using your own reservation ensures
predictable and consistent performance of index-management jobs, such as
creation, refresh, and background optimizations.

- There are no table size limits when an indexing job runs in your reservation.
- Using your own reservation gives you flexibility in your index management. If you need to create a very large index or make a major update to an indexed table, you can temporarily add more slots to the assignment.

To index the tables in a project with a designated reservation,
[create a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks)
in the region where your tables are located. Then, assign the project to the
reservation with the `job_type` set to `BACKGROUND`, which shares resources
across background optimization jobs:

### SQL

Use the
[`CREATE ASSIGNMENT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_assignment_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE ASSIGNMENT
     `ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME.ASSIGNMENT_ID`
   OPTIONS (
     assignee = 'projects/PROJECT_ID',
     job_type = 'BACKGROUND');
   ```


   Replace the following:
   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
   - `RESERVATION_NAME`: the name of the reservation
   - `ASSIGNMENT_ID`: the ID of the assignment

     The ID must be unique to the project and location,
     start and end with a lowercase letter or a number,
     and contain only lowercase letters, numbers, and dashes.
   - `PROJECT_ID`: the ID of the project containing the tables to index. This project is assigned to the reservation.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq mk` command:

```
bq mk \
    --project_id=ADMIN_PROJECT_ID \
    --location=LOCATION \
    --reservation_assignment \
    --reservation_id=RESERVATION_NAME \
    --assignee_id=PROJECT_ID \
    --job_type=BACKGROUND \
    --assignee_type=PROJECT
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation
- `RESERVATION_NAME`: the name of the reservation
- `PROJECT_ID`: the ID of the project to assign to this reservation

#### View your indexing jobs

A new indexing job is created every time an index is created or updated on
a single table. To view information about the job, query the
[`INFORMATION_SCHEMA.JOBS*` views](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs). You
can filter for indexing jobs by
setting ``job_type IS NULL AND SEARCH(job_id, '`search_index`')`` in the `WHERE`
clause of your query. The following example lists the five most recent indexing
jobs in the project `my_project`:

```googlesql
SELECT *
FROM
 region-us.INFORMATION_SCHEMA.JOBS
WHERE
  project_id  = 'my_project'
  AND job_type IS NULL
  AND SEARCH(job_id, '`search_index`')
ORDER BY
 creation_time DESC
LIMIT 5;
```

> [!NOTE]
> **Note:** You can't view information about indexing jobs run in the default shared slot pool.

#### Choose your reservation size

To choose the right number of slots for your reservation, you should consider
when index-management jobs are run, how many slots they use, and what your usage
looks like over time. BigQuery triggers an index-management job
in the following situations:

- You create an index on a table.
- Data is modified in an indexed table.
- The schema of a table changes and this affects which columns are indexed.
- Index data and metadata are periodically optimized or updated.

The number of slots you need for an index-management job on a table depends on
the following factors:

- The size of the table
- The rate of data ingestion to the table
- The rate of DML statements applied to the table
- The acceptable delay for building and maintaining the index
- The complexity of the index, typically determined by attributes of the data, such as the number of duplicate terms

##### Monitor Usage and Progress

The best way to assess the number of slots you need to efficiently run your
index-management jobs is to monitor your slot utilization and adjust the
reservation size accordingly. The following query produces the daily slot usage
for index-management jobs. Only the past 30 days are included in the
region `us-west1`:

```googlesql
SELECT
  TIMESTAMP_TRUNC(job.creation_time, DAY) AS usage_date,
  -- Aggregate total_slots_ms used for index-management jobs in a day and divide
  -- by the number of milliseconds in a day. This value is most accurate for
  -- days with consistent slot usage.
  SAFE_DIVIDE(SUM(job.total_slot_ms), (1000 * 60 * 60 * 24)) AS average_daily_slot_usage
FROM
  `region-us-west1`.INFORMATION_SCHEMA.JOBS job
WHERE
  project_id = 'my_project'
  AND job_type IS NULL
  AND SEARCH(job_id, '`search_index`')
GROUP BY
  usage_date
ORDER BY
  usage_date DESC
limit 30;
```

When there are insufficient slots to run index-management jobs, an index can
become out of sync with its table and indexing jobs might fail.
In this case, BigQuery rebuilds the index from scratch. To
avoid having an out-of-sync index, ensure you have enough slots to support index
updates from data ingestion and optimization. For more information on
monitoring slot usage, see
[admin resource charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts).

## Rebuild a vector index

When table data changes significantly after a vector index is created, the
vector index can become less efficient. When a vector index is less efficient,
a vector search query that initially had high
[recall](https://developers.google.com/machine-learning/glossary#recall)
when using the index will have lower recall, because the data distribution
shift in the base table isn't represented in the vector index.

If you want to improve recall without increasing search query latency, rebuild
the vector index. Alternatively, you can increase the value of the vector
search's `fraction_lists_to_search` option to improve recall, but this
typically makes the search query slower. To find out when an index was last
built, run the following query:

```googlesql
SELECT last_model_build_time
FROM DATASET_NAME.INFORMATION_SCHEMA.VECTOR_INDEXES
WHERE table_name = TABLE_NAME;
```

You can use the
[`VECTOR_INDEX.STATISTICS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/vectorindex_functions#vector_indexstatistics)
to calculate how much an indexed table's data has drifted between when a
vector index was created and the present.

If table data has changed enough to require a vector index rebuild, you can use
the [`ALTER VECTOR INDEX REBUILD` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_vector_index_rebuild_statement)
to rebuild the vector index without having to drop the vector index, and without
any index downtime. When you run the statement, BigQuery creates
a *shadow index*, a background-only index that inherits the configuration of the
current index, on the table and trains it in the background.
BigQuery promotes the shadow index to be the active index when
the shadow index has enough coverage.

Follow these steps to rebuild a vector index:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement to check the indexed
   table's data drift:

   ```googlesql
   SELECT * FROM VECTOR_INDEX.STATISTICS(TABLE DATASET_NAME.TABLE_NAME);
   ```

   Replace the following:
   - `DATASET_NAME`: the name of the dataset that contains the indexed table.
   - `TABLE_NAME`: the name of the table that contains the vector index.

   The function returns a `FLOAT64` value in the range `[0,1)`. A lower value
   indicates less drift. Typically, a value of `0.3` or greater is considered
   a significant enough change to indicate that a vector index rebuild might
   be beneficial.
3. If the `VECTOR_INDEX.STATISTICS` function indicates that table data drift is
   significant, run the following SQL statement to rebuild the vector index:

   ```googlesql
   ALTER VECTOR INDEX IF EXISTS INDEX_NAME
   ON DATASET_NAME.TABLE_NAME
   REBUILD;
   ```

   Replace the following:
   - `INDEX_NAME`: the name of the vector index that you are rebuilding.
   - `DATASET_NAME`: the name of the dataset that contains the indexed table.
   - `TABLE_NAME`: the name of the table that contains the vector index.

### Monitor a rebuild

To monitor your index rebuild, query the
[`INFORMATION_SCHEMA.VECTOR_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes).
For example, the following query shows the rebuild status of all indexes on
tables in the dataset `my_dataset` in the project `my_project`. If an index
has no ongoing rebuild, then the value for `last_index_alteration_info` is
`NULL`.

    SELECT
      table_name,
      index_name,
      last_index_alteration_info.status AS status,
      last_index_alteration_info.new_coverage_percentage AS coverage
    FROM my_project.my_dataset.INFORMATION_SCHEMA.VECTOR_INDEXES

The result looks similar to the following:

    +---+---+---+---+
    | table_name | index_name | status      | coverage |
    +---+---+---+---+
    | table1     | index_a    | IN_PROGRESS | 50       |
    | table2     | index_b    | null        | null     |
    +---+---+---+---+

### Cancel a rebuild

To cancel a vector index rebuild, use the
[`BQ.CANCEL_INDEX_ALTERATION` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqcancel_index_alteration).
An index alteration operation might complete before the cancellation request is
processed. To confirm whether the operation was successfully canceled, query
the
[`INFORMATION_SCHEMA.VECTOR_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes).
If the cancellation was
successful, the `last_model_build_time` field isn't updated and the
`last_index_alteration_info` field isn't present for that index.

## Delete a vector index

When you no longer need a vector index or want to change which column is
indexed on a table, you can delete the index on that table by using the
[`DROP VECTOR INDEX` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_vector_index):

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement:

   ```googlesql
   DROP VECTOR INDEX INDEX_NAME ON DATASET_NAME.TABLE_NAME;
   ```

   Replace the following:
   - `INDEX_NAME`: the name of the vector index that you are deleting.
   - `DATASET_NAME`: the name of the dataset that contains the indexed table.
   - `TABLE_NAME`: the name of the table that contains the vector index.

If an indexed table is deleted, its index is deleted automatically.

## Export embeddings to Vertex AI Vector Search

To enable ultra-low latency online applications, use BigQuery
integration with Vertex AI [Vector Search](https://docs.cloud.google.com/vertex-ai/docs/vector-search/overview)
to import your BigQuery embeddings into Vector Search
and deploy low latency endpoints. For more information, see [Import index data from BigQuery](https://docs.cloud.google.com/vertex-ai/docs/vector-search/import-index-data-from-big-query).

## What's next

- For an overview of vector index use cases, pricing, and limitations, see the [Introduction to vector search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro).
- Learn how to perform a vector search using the [`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search).
- Learn how to perform semantic search using the [`AI.SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search).
- Learn more about the [`CREATE VECTOR INDEX` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement).
- Try the [Search embeddings with vector search](https://docs.cloud.google.com/bigquery/docs/vector-search) tutorial.