# Introduction to search in BigQuery

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

BigQuery search indexes let you use GoogleSQL to
efficiently find
unique data elements that are buried in unstructured text and semi-structured
JSON data, without having to know the table schemas in advance.

With search indexes, BigQuery provides a powerful columnar store
and text search in one platform, enabling efficient row lookups when you need to
find individual rows of data. A common use case is log analytics. For example,
you might want to identify the rows of data associated with a user for General
Data Protection Regulation (GDPR) reporting, or to find specific error codes in
a text payload.

BigQuery stores and manages your indexes, so that when data becomes
available in BigQuery, you can immediately retrieve it with the
[`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search)
or [other operators and functions](https://docs.cloud.google.com/bigquery/docs/search#operator_and_function_optimization),
such as the equal (`=`), `IN`, or `LIKE` operators and certain string and JSON
functions. To optimize your searches, read about
[best practices](https://docs.cloud.google.com/bigquery/docs/search#best_practices).

> [!IMPORTANT]
> **Important:** Join the [Search discussion group](https://groups.google.com/g/bq-search) to post questions and comments, and to follow the latest updates.

## Use cases

BigQuery search indexes help you perform the following tasks:

- Search system, network, or application logs stored in BigQuery tables.
- Identify data elements for deletion to comply with regulatory processes.
- Support developer troubleshooting.
- Perform security audits.
- Create a dashboard that requires highly selective search filters.
- Search pre-processed data for exact matches.

For more information, see
[Create a search index](https://docs.cloud.google.com/bigquery/docs/search-index) and
[Search with an index](https://docs.cloud.google.com/bigquery/docs/search).

## Pricing

There is no charge for the processing required to build and refresh your search
indexes when the total size of indexed tables in your organization is below
your region's
[limit](https://docs.cloud.google.com/bigquery/quotas#index_limits). To support indexing beyond this limit,
you need to
[provide your own reservation](https://docs.cloud.google.com/bigquery/docs/search-index#use_your_own_reservation)
for handling the index-management jobs.
Search indexes incur storage costs when they are active.
You can find the index storage size in the
[`INFORMATION_SCHEMA.SEARCH_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-indexes).

## Roles and permissions

To create a search index, you need the
[`bigquery.tables.createIndex` IAM permission](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
on the table where you're creating the index. To drop a search index, you need
the `bigquery.tables.deleteIndex` permission. Each of the following predefined
IAM roles includes the permissions that you need to work with
search indexes:

- BigQuery Data Owner (`roles/bigquery.dataOwner`)
- BigQuery Data Editor (`roles/bigquery.dataEditor`)
- BigQuery Admin (`roles/bigquery.admin`)

## Limitations

- You can't create a search index directly on a view or materialized view, but calling the [`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search) on a view of an indexed table makes use of the underlying search index.
- You can't create a search index on an external table.
- If you rename a table after you create a search index on it, the index becomes invalid.
- The `SEARCH` function is designed for point lookups. Fuzzy searching, typo correction, wildcards, and other types of document searches are not available.
- If the search index is not yet at 100% coverage, you are still charged for all index storage that is reported in the [`INFORMATION_SCHEMA.SEARCH_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-indexes).
- Queries that use the `SEARCH` function or are optimized by search indexes are not accelerated by [BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro).
- Search indexes are not used when the indexed table is modified by a DML
  statement, but they can be used when the predicate that is optimizable by
  search indexes is part of a subquery in a DML statement.

  - A search index is not used in the following query:

  ```googlesql
  DELETE FROM my_dataset.indexed_table
  WHERE SEARCH(user_id, '123');
  ```
  - A search index can be used in the following query:

  ```googlesql
  DELETE FROM my_dataset.other_table
  WHERE
    user_id IN (
      SELECT user_id
      FROM my_dataset.indexed_table
      WHERE SEARCH(user_id, '123')
    );
  ```
- Search indexes are not used when the query references [Materialized Views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).

- Search indexes are not used in a [multi-statement transaction query](https://docs.cloud.google.com/bigquery/docs/transactions).

- Search indexes are not used in a [time-travel query](https://docs.cloud.google.com/bigquery/docs/time-travel).

## What's next

- Learn more about [creating a search index](https://docs.cloud.google.com/bigquery/docs/search-index).
- Learn more about [searching in a table with a search index](https://docs.cloud.google.com/bigquery/docs/search).