# The AI.SEARCH function

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
> **Note:** To provide feedback or request support for this feature during the preview, contact [bq-vector-search@google.com](mailto:bq-vector-search@google.com).

This document describes the `AI.SEARCH` function, which is a table-valued
function for semantic search on tables that have
[autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation) enabled.

For example, you could use a query like the following to search a table of
product descriptions for anything described as a fun toy. In this example,
the `product_description` column has autonomous embedding generation enabled.

    SELECT *
    FROM AI.SEARCH(TABLE product_table, product_description, "A really fun toy");

Embeddings are high-dimensional numerical vectors that represent a given entity.
Embeddings encode semantics about entities
to make it easier to reason about and compare them. If two entities are
semantically similar, then their respective embeddings are located near each
other in the embedding vector space. The `AI.SEARCH` function embeds your
search query and searches the table that you provide for embeddings in the input
table that are close to it. If your table has a vector index on the embedding
column, then `AI.SEARCH` uses it to optimize the search.

You can use `AI.SEARCH` to help with the following tasks:

- **Semantic search**: search entities ranked by semantic similarity.
- **Recommendation**: return entities with attributes similar to a given entity.
- **Classification**: return the class of entities whose attributes are similar to the given entity.
- **Clustering**: cluster entities whose attributes are similar to a given entity.
- **Outlier detection**: return entities whose attributes are least related to the given entity.

## Syntax

```googlesql
AI.SEARCH(
  { TABLE base_table | base_table_query },
  column_to_search,
  query_value
  [, top_k => top_k_value ]
  [, distance_type => distance_type_value ]
  [, options => options_value]
)
```

### Arguments

`AI.SEARCH` takes the following arguments:

- `base_table`: The table to search for nearest neighbor embeddings. The table must have [autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation) enabled.
- `base_table_query`: A query that you can use to pre-filter the base table. Only `SELECT`, `FROM`, and `WHERE` clauses are allowed in this query. Don't apply any filters to the embedding column. You can't use [logical views](https://docs.cloud.google.com/bigquery/docs/views-intro) in this query. Using a [subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries) might interfere with index usage or cause your query to fail. If the base table is indexed and the `WHERE` clause contains columns that are not stored in the index, then `AI.SEARCH` uses post-filters on those columns instead. To learn more, see [Store columns and pre-filter](https://docs.cloud.google.com/bigquery/docs/vector-index#stored-columns).
- `column_to_search`: A `STRING` literal that contains the name of the string column to search. This must be the name of the source column that the automatically generated embedding column is based on, but it's not the name of the generated embedding column itself. If the column has a vector index, BigQuery attempts to use it. To determine if an index was used in the vector search, see [Vector index usage](https://docs.cloud.google.com/bigquery/docs/vector-index#vector_index_usage).
- `query_value`: A string literal that represents the search query. This value is embedded at runtime using the same connection and endpoint specified for the base table's embedding generation. You must have the BigQuery Connection User role (`roles/bigquery.connectionUser`) on the connection that the base table uses for background embedding generation. If embedding generation fails for `query_value`, then the whole query fails. Rows with missing embeddings in the base table are skipped during the search.
- `top_k`: A named argument with an `INT64` value. `top_k_value` specifies the number of nearest neighbors to return. The default is `10`. If the value is negative, all values are counted as neighbors and returned.
- `distance_type`: A named argument with a `STRING` value.
  `distance_type_value` specifies the type of metric to use to
  compute the distance between two vectors. Supported distance types are
  [`EUCLIDEAN`](https://en.wikipedia.org/wiki/Euclidean_distance),
  [`COSINE`](https://en.wikipedia.org/wiki/Cosine_similarity#Cosine_Distance),
  and
  [`DOT_PRODUCT`](https://en.wikipedia.org/wiki/Dot_product).
  The default is `EUCLIDEAN`.

  If you don't specify `distance_type_value` and the `column_to_search`
  column has a vector index that's used, then `AI.SEARCH` uses the distance
  type specified in the
  [`distance_type` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#vector_index_option_list)
  of the `CREATE VECTOR INDEX` statement.
- `options`: A named argument with a JSON-formatted `STRING` value.
  `options_value` is a literal that specifies the following search
  options:

  - `fraction_lists_to_search`: A JSON number that specifies the
    percentage of lists to search. For example,
    `options => '{"fraction_lists_to_search":0.15}'`. The
    `fraction_lists_to_search` value must be in the range `0.0` to `1.0`,
    exclusive.

    Specifying a higher percentage leads to higher recall and slower
    performance, and the converse is true when specifying a lower percentage.

    `fraction_lists_to_search` is only used when a vector index is also used.
    If you don't specify a `fraction_lists_to_search` value but an index is
    matched, an appropriate value is picked.

    The number of available lists to search is determined by the
    [`num_lists` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#vector_index_option_list)
    in the `ivf_options` option or derived from
    the [`leaf_node_embedding_count` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#vector_index_option_list)
    in the
    `tree_ah_options` option of the `CREATE VECTOR INDEX` statement if
    specified. Otherwise, BigQuery calculates an appropriate number.

    You can't specify `fraction_lists_to_search` when `use_brute_force` is
    set to `true`.
  - `use_brute_force`: A JSON boolean that determines whether to use brute
    force search by skipping the vector index if one is available. For
    example, `options => '{"use_brute_force":true}'`. The
    default is `false`. If you specify `use_brute_force=false` and there is
    no useable vector index available, brute force is used anyway.

  `options` defaults to `'{}'` to denote that all underlying options use their
  corresponding default values.

## Details

You can optionally use `AI.SEARCH` with a
[vector index](https://docs.cloud.google.com/bigquery/docs/vector-index). When
a vector index is used, `AI.SEARCH` uses the [Approximate Nearest
Neighbor](https://en.wikipedia.org/wiki/Nearest_neighbor_search#Approximation_methods)
search technique to help improve vector search performance, with
the trade-off of reducing
[recall](https://developers.google.com/machine-learning/crash-course/classification/precision-and-recall#recallsearch_term_rules)
and so returning more approximate
results. When a base table is large, the use of an index typically improves
performance without significantly sacrificing recall. Brute force is used to
return exact results when a vector index isn't available, and you can
choose to use brute force to get exact results even when a vector index
is available.

## Output

The output includes the following columns:

- `base`: A `STRUCT` value that contains all columns from `base_table` or a subset of the columns from `base_table` that you selected in the `base_table_query` query.
- `distance`: A `FLOAT64` value that represents the distance between the `query_value` and the embedding in `column_to_search`.

Rows that are missing a generated embedding are skipped during the search.

## Example

The following example shows how to create a table of products and descriptions
with autonomous embedding enabled on the description column,
add some data to the table, and then search it for products that would be
fun to play with.

    # Create a table of products and descriptions with a generated embedding column.
    CREATE TABLE mydataset.products (
      name STRING,
      description STRING,
      description_embedding STRUCT<result ARRAY<FLOAT64>, status STRING>
        GENERATED ALWAYS AS (AI.EMBED(
          description,
          connection_id => 'us.example_connection',
          endpoint => 'text-embedding-005'
        ))
        STORED OPTIONS( asynchronous = TRUE )
    );

    # Insert product descriptions into the table.
    # The description_embedding column is automatically updated.
    INSERT INTO mydataset.products (name, description) VALUES
      ("Lounger chair", "A comfortable chair for relaxing in."),
      ("Super slingers", "An exciting board game for the whole family."),
      ("Encyclopedia set", "A collection of informational books.");

    # Search for products that are fun to play with.
    SELECT base.name, base.description, distance
    FROM AI.SEARCH(TABLE mydataset.products, 'description', "A really fun toy");

    /*---+---+---+
     | name             | description                                  | distance             |
     +---+---+---+
     | Super slingers   | An exciting board game for the whole family. | 0.80954913893618929  |
     | Lounger chair    | A comfortable chair for relaxing in.         | 0.938933930620146    |
     | Encyclopedia set | A collection of informational books.         | 1.1119297739353384   |
     +---+---+---*/

## Related functions

The `AI.SEARCH` and
[`VECTOR_SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
functions support overlapping use cases. In general, you should use `AI.SEARCH`
when your base table has autonomous embedding generation enabled and you want
to search for results close to a single string literal. It offers a simplified
syntax compared to `VECTOR_SEARCH` and doesn't require you to embed your
search query. You should use
`VECTOR_SEARCH` when you want to batch your search queries, when you want
to generate your
own embeddings as input, or if your base table doesn't use autonomous embedding
generation.

## Locations

You can run `AI.SEARCH` in all of the
[locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations)
that support Vertex AI embedding models, and also in the `US`
and `EU` multi-regions.

## Quotas

See [Generative AI functions quotas and limits](https://docs.cloud.google.com/bigquery/quotas#generative_ai_functions).

## What's next

- Learn more about [autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation).
- Learn more about [creating and managing vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-index).
- Learn more about [embeddings and search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro).