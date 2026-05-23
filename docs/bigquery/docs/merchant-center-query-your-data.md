# Query your Google Merchant Center Transfers data

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

When your data is transferred to BigQuery, the data is
written to ingestion-time partitioned tables. For more information, see
[Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

When you query your Google Merchant Center table, you
must use the `_PARTITIONTIME` or `_PARTITIONDATE` pseudocolumn in your query.
For more information, see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

The `Products_` table contains nested and repeated fields. For information on
handling nested and repeated data, see
[Differences in repeated field handling](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql#differences_in_repeated_field_handling)
in the GoogleSQL documentation.

## Google Merchant Center sample queries

You can use the following Google Merchant Center sample queries to analyze your
transferred data. You can also use the queries in a visualization tool such as
[Data Studio](https://www.google.com/analytics/data-studio/).

In each of the following queries, replace <var translate="no">dataset</var> with your dataset
name. Replace <var translate="no">merchant_id</var> with your Merchant ID. If you're using an
MCA, replace <var translate="no">merchant_id</var> with your MCA ID.

### Products and product issues statistics

The following SQL sample query provides the number of products, products with
issues, and issues by day.

```googlesql
SELECT
  _PARTITIONDATE AS date,
  COUNT(*) AS num_products,
  COUNTIF(ARRAY_LENGTH(issues) > 0) AS num_products_with_issues,
  SUM(ARRAY_LENGTH(issues)) AS num_issues
FROM
  dataset.Products_merchant_id
WHERE
  _PARTITIONDATE >= 'YYYY-MM-DD'
GROUP BY
  date
ORDER BY
  date DESC
```

### Products disapproved for Shopping Ads

The following SQL sample query provides the number of products that are not
approved for display in Shopping Ads, separated by country. Disapproval
can result from the destination being
[excluded](https://support.google.com/merchants/answer/6324486)
or because of an issue with the product.

```googlesql
SELECT
  _PARTITIONDATE AS date,
  disapproved_country,
  COUNT(*) AS num_products
FROM
  dataset.Products_merchant_id,
  UNNEST(destinations) AS destination,
  UNNEST(disapproved_countries) AS disapproved_country
WHERE
  _PARTITIONDATE >= 'YYYY-MM-DD'
GROUP BY
  date, disapproved_country
ORDER BY
  date DESC
```

### Products with disapproved issues

The following SQL sample query retrieves the number of products with disapproved
issues, separated by country.

```googlesql
SELECT
  _PARTITIONDATE AS date,
  applicable_country,
  COUNT(DISTINCT CONCAT(CAST(merchant_id AS STRING), ':', product_id))
      AS num_distinct_products
FROM
  dataset.Products_merchant_id,
  UNNEST(issues) AS issue,
  UNNEST(issue.applicable_countries) as applicable_country
WHERE
  _PARTITIONDATE >= 'YYYY-MM-DD' AND
  issue.servability = 'disapproved'
GROUP BY
  date, applicable_country
ORDER BY
  date DESC
```

> [!NOTE]
> **Note:** This query constructs a unique key by using `merchant_id` and `product_id`. This is only required if you have an MCA account. When you use an MCA account, there is the potential for `product_id` collisions across multiple sub-accounts.