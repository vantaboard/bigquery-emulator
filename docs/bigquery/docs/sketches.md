GoogleSQL for BigQuery supports data sketches.
A data sketch is a compact summary of a data aggregation. It captures all the
necessary information to either extract an aggregation result, continue a
data aggregation, or merge it with another sketch, enabling re-aggregation.

Computing a metric using a sketch is substantially less expensive than computing
an exact value. If your computation is too slow or requires too much temporary
storage, use sketches to reduce query time and resources.

Additionally, computing [cardinalities](https://en.wikipedia.org/wiki/Cardinality), such as the
number of distinct users, or [quantiles](https://en.wikipedia.org/wiki/Quantile), such as
median visit duration, without sketches is usually only possible by running jobs
over the raw data because already-aggregated data can't be combined anymore.

Consider a table with the following data:

| Product | Number of users | Median visit duration |
|---|---|---|
| Product A | 500 million | 10 minutes |
| Product B | 20 million | 2 minutes |

Computing the total number of users for both products isn't possible because we
don't know how many users used both products in the table.
Likewise, computing the median visit duration isn't possible because the
distribution of the visit durations has been lost.

A solution is to store sketches in the table instead. Each sketch is an
approximate and compact representation of a particular input property, such as
cardinality, that you can store, merge (or re-aggregate), and query for
near-exact results. In the previous example, you can estimate the number of
distinct users for Product A and Product B by creating and merging
(re-aggregating) the sketches for each product. You
can also estimate the median
visit duration with quantile sketches that you can likewise merge and query.

For example, the following query uses [HLL++](https://docs.cloud.google.com/bigquery/docs/sketches#sketches_hll) and [KLL](https://docs.cloud.google.com/bigquery/docs/sketches#sketches_kll)
sketches to estimate distinct users and median visit duration for YouTube
(Product A) and Google Maps (Product B):

```sql
-- Build sketches for YouTube stats.
CREATE TABLE user.YOUTUBE_ACCESS_STATS
AS
SELECT
  HLL_COUNT.INIT(user_id) AS distinct_users_sketch,
  KLL_QUANTILES.INIT_INT64(visit_duration_ms) AS visit_duration_ms_sketch,
  hour_of_day
FROM YOUTUBE_ACCESS_LOG()
GROUP BY hour_of_day;

-- Build sketches for Maps stats.
CREATE TABLE user.MAPS_ACCESS_STATS
AS
SELECT
  HLL_COUNT.INIT(user_id) AS distinct_users_sketch,
  KLL_QUANTILES.INIT_INT64(visit_duration_ms) AS visit_duration_ms_sketch,
  hour_of_day
FROM MAPS_ACCESS_LOG()
GROUP BY hour_of_day;

-- Query YouTube hourly stats.
SELECT
  HLL_COUNT.EXTRACT(distinct_users_sketch) AS distinct_users,
  KLL_QUANTILES.EXTRACT_POINT_INT64(visit_duration_ms_sketch, 0.5)
  AS median_visit_duration, hour_of_day
FROM user.YOUTUBE_ACCESS_STATS;

-- Query YouTube daily stats.
SELECT
  HLL_COUNT.MERGE(distinct_users_sketch),
  KLL_QUANTILES.MERGE_POINT_INT64(visit_duration_ms_sketch, 0.5)
  AS median_visit_duration, date
FROM user.YOUTUBE_ACCESS_STATS
GROUP BY date;

-- Query total stats across YouTube and Maps.
SELECT
  HLL_COUNT.MERGE(distinct_users_sketch) AS unique_users_all_services,
  KLL_QUANTILES.MERGE_POINT_INT64(visit_duration_ms_sketch, 0.5)
    AS median_visit_duration_all_services,
FROM
  (
    SELECT * FROM user.YOUTUBE_ACCESS_STATS
    UNION ALL
    SELECT * FROM user.MAPS_ACCESS_STATS
  );
```

Because a sketch has lossy compression of the original data, it introduces a
statistical error that's represented by an error bound or confidence interval
(CI). For most applications, this uncertainty is small. For example, a typical
cardinality-counting sketch has a relative error of about 1% in 95% of all
cases. A sketch trades some accuracy, or *precision*, for faster and less
expensive computations, and less storage.

In summary, a sketch has these main properties:

- Represents an approximate aggregate for a specific metric
- Is compact
- Is a serialized form of an in-memory, sublinear data structure
- Is typically a fixed size and asymptotically smaller than the input
- Can introduce a statistical error that you determine with a precision level
- Can be merged with other sketches to summarize the union of the underlying data sets

## Re-aggregation with sketch merging

Sketches let you store and merge data for efficient re-aggregation. This makes
sketches particularly useful for materialized views of data sets. You can merge
sketches to construct a summary of multiple data streams based on partial
sketches created for each stream.

For example, if you create a sketch for the estimated number of distinct users
every day, you can get the number of distinct users during the last seven days
by merging daily sketches. Re-aggregating the merged daily sketches helps you
avoid reading the full input of the data set.

Sketch re-aggregation is also useful in online analytical processing (OLAP). You
can merge sketches to create a roll-up of an
[OLAP cube](https://en.wikipedia.org/wiki/OLAP_cube), where the
sketch summarizes data along one or more specific dimensions of the cube. OLAP
roll-ups aren't possible with true distinct counts.

## Which type of sketch should I use?

Different sketching algorithms are designed for different types of metrics, such
as [HLL++](https://docs.cloud.google.com/bigquery/docs/sketches#sketches_hll) for distinct counts and
[KLL](https://docs.cloud.google.com/bigquery/docs/sketches#sketches_kll) for quantiles. GoogleSQL also provides
[Approximate aggregate functions](https://docs.cloud.google.com/bigquery/docs/sketches#approx_functions) that you can use to query
these types of data without having to specify query details such as precision
level.

The sketch that you use depends on the type of data that you need to estimate.

### Estimate cardinality

If you need to estimate [cardinality](https://en.wikipedia.org/wiki/Cardinality), use an [HLL++ sketch](https://docs.cloud.google.com/bigquery/docs/sketches#sketches_hll).

For example, to get the number of unique users who actively used a product in a
given month (MAU or 28DAU metrics), use an HLL++ sketch.

### Compute a quantile

If you need to get a [quantile](https://en.wikipedia.org/wiki/Quantile) of a data set, use
a [KLL sketch](https://docs.cloud.google.com/bigquery/docs/sketches#sketches_kll).

For example, to get the median visit duration of customers in
a store, or to track the 95th percentile latency that tickets stay in a queue
before being addressed, use a KLL sketch.

## HLL++ sketches

HyperLogLog++ (HLL++) is a sketching algorithm for estimating cardinality. HLL++
is based on the paper [HyperLogLog in Practice](https://research.google.com/pubs/archive/40671.pdf), where the
*++* denotes the augmentations made to the HyperLogLog algorithm.

[Cardinality](https://en.wikipedia.org/wiki/Cardinality) is the number of distinct elements in the
input for a sketch. For example, you could use an HLL++ sketch to get the number
of unique users who have opened an application.

HLL++ estimates very small and very large cardinalities. HLL++ includes a
64-bit hash function, sparse representation to reduce memory requirements for
small cardinality estimates, and empirical bias correction for
small cardinality estimates.

**Precision**

HLL++ sketches support custom precision. The following table shows the supported
precision values, the maximum storage size, and the confidence interval (CI) of
typical precision levels:

| Precision | Max storage size | 65% CI | 95% CI | 99% CI |
|---|---|---|---|---|
| 10 | 1 KiB + 28 B | ±3.25% | ±6.50% | ±9.75% |
| 11 | 2 KiB + 28 B | ±2.30% | ±4.60% | ±6.89% |
| 12 | 4 KiB + 28 B | ±1.63% | ±3.25% | ±4.88% |
| 13 | 8 KiB + 28 B | ±1.15% | ±2.30% | ±3.45% |
| 14 | 16 KiB + 30 B | ±0.81% | ±1.63% | ±2.44% |
| 15 (default) | 32 KiB + 30 B | ±0.57% | ±1.15% | ±1.72% |
| 16 | 64 KiB + 30 B | ±0.41% | ±0.81% | ±1.22% |
| 17 | 128 KiB + 30 B | ±0.29% | ±0.57% | ±0.86% |
| 18 | 256 KiB + 30 B | ±0.20% | ±0.41% | ±0.61% |
| 19 | 512 KiB + 30 B | ±0.14% | ±0.29% | ±0.43% |
| 20 | 1024 KiB + 30 B | ±0.10% | ±0.20% | ±0.30% |
| 21 | 2048 KiB + 32 B | ±0.07% | ±0.14% | ±0.22% |
| 22 | 4096 KiB + 32 B | ±0.05% | ±0.10% | ±0.15% |
| 23 | 8192 KiB + 32 B | ±0.04% | ±0.07% | ±0.11% |
| 24 | 16384 KiB + 32 B | ±0.03% | ±0.05% | ±0.08% |

You can define precision for an HLL++ sketch when you initialize it with the
[`HLL_COUNT.INIT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions#hll_countinit) function.

**Deletion**

You can't delete values from an HLL++ sketch.

**Additional details**

For a list of functions that you can use with HLL++ sketches, see
[HLL++ functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions).

### Sketch integration

You can integrate HLL++ sketches with other systems. For example, you can build
sketches in external applications, like [Dataflow](https://cloud.google.com/dataflow),
[Apache Spark](https://spark.apache.org), and [ZetaSketch](https://github.com/google/zetasketch)
and then consume them in GoogleSQL or vice versa.

In addition to GoogleSQL, you can use HLL++ sketches
with [Java](https://github.com/google/zetasketch/blob/master/java/com/google/zetasketch/HyperLogLogPlusPlus.java).

## KLL sketches

KLL (short for Karnin-Lang-Liberty) is a streaming algorithm to compute sketches
for approximate [quantiles](https://docs.cloud.google.com/bigquery/docs/sketches#quantiles). It computes arbitrary quantiles far more
efficiently than exact computations at the price of a small approximation error.

**Precision**

KLL sketches support custom precision. Precision defines the exactness of a
returned approximate quantile *q*.

By default, the rank of an approximate quantile can be at most `±1/1000 * n` off
from `⌈Φ * n⌉`, where `n` is the number of rows in the input and `⌈Φ * n⌉` is
the rank of the exact quantile.

If you provide custom precision, the rank of the approximate quantile can be
at most `±1/precision * n` off from the rank of the exact quantile. The error
is within this error bound in 99.999% of cases. This error guarantee only
applies to the difference between exact and approximate ranks. The numerical
difference between the exact and approximated value for a quantile can be
arbitrarily large.

For example, suppose you want to find the median value, `Φ = 0.5`, and you use
the default precision of `1000`. Then the rank of the value returned by the
`KLL_QUANTILES.EXTRACT_POINT` function differs from the true rank by at most
`n/1000` in 99.999% of cases. In other words, the returned value is almost
always between the 49.9th and 50.1st percentiles. If you have 1,000,000 items in
your sketch, then the rank of the returned median is almost always between
499,000 and 501,000.

If you use a custom precision of `100` to find the median value, then the rank
of the value returned by the `KLL_QUANTILES.EXTRACT_POINT` function differs from
the true rank by at most `n/100` in 99.999% of cases. In other words, the
returned value is almost always between the 49th and 51st percentiles. If you
have 1,000,000 items in your sketch, then the rank of the returned median is
almost always between 490,000 and 510,000.

You can define precision for a KLL sketch when you initialize it with the
[`KLL_QUANTILES.INIT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesinit_int64) function.

**Size**

KLL sketch size depends on the precision parameter and the input type.
If your input type is `INT64`, the sketches can use additional optimization
that's especially helpful if the input values come from a small universe. The
following table contains two columns for `INT64`. One column provides an
upper bound on sketch size for items from a limited universe of size 1B, and a
second column provides an upper bound for arbitrary input values.

| Precision | FLOAT64 | INT64 (\<1B) | INT64 (Any) |
|---|---|---|---|
| 10 | 761 B | 360 B | 717 B |
| 20 | 1.46 KB | 706 B | 1.47 KB |
| 50 | 3.49 KB | 1.72 KB | 3.60 KB |
| 100 | 6.94 KB | 3.44 KB | 7.12 KB |
| 200 | 13.87 KB | 6.33 KB | 13.98 KB |
| 500 | 35.15 KB | 14.47 KB | 35.30 KB |
| 1000 | 71.18 KB | 27.86 KB | 71.28 KB |
| 2000 | 144.51 KB | 55.25 KB | 144.57 KB |
| 5000 | 368.87 KB | 139.54 KB | 368.96 KB |
| 10000 | 749.82 KB | 282.27 KB | 697.80 KB |
| 20000 | 1.52 MB | 573.16 KB | 1.37 MB |
| 50000 | 3.90 MB | 1.12 MB | 3.45 MB |
| 100000 | 7.92 MB | 2.18 MB | 6.97 MB |

**Phi**

Phi (Φ) represents the quantile to produce as a fraction of the total number of
rows in sketch input, normalized between 0 and 1. If a function supports
phi, the function returns a value *v* such that approximately Φ \* *n* inputs
are less than or equal to *v* , and (1-Φ) \* *n* inputs are greater than or
equal to *v*.

**Additional details**

For a list of functions that you can use with KLL sketches, see
[KLL quantile functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_functions).

The KLL algorithm is defined in the paper
[Optimal Quantile Approximation in Streams](https://arxiv.org/pdf/1603.05346v2.pdf), and is named
after its authors, Karnin, Lang, and Liberty, who published the paper in 2016.
The KLL algorithm improves the older
[MP80 algorithm](https://polylogblog.files.wordpress.com/2009/08/80munro-median.pdf) by using variable-size buffers to reduce
memory use for large data sets, reducing the sketch size from `O(log n)` to
`O(1)`. Due to the non-deterministic nature of the algorithm, sketches created
on the same set of data with the same precision might not be identical.

## Quantiles

[Quantiles](https://en.wikipedia.org/wiki/Quantile) are cut points dividing the range of a
probability distribution into continuous intervals with equal probabilities, or
dividing the observations in a sample in the same way. A quantile-supporting
sketch lets you estimate quantiles by summarizing those intervals and
probabilities into near-exact quantile results.

Quantiles are typically defined in two ways:

- For a positive integer `q`, `q`-quantiles are a set of values that partition
  an input set into `q` subsets of nearly equal size. Some of these have
  specific names: the single 2-quantile is the median, the 4-quantiles are
  quartiles, the 100-quantiles are percentiles, etc. KLL functions
  additionally return the (exact) minimum and the maximum of the input, so
  when querying for the 2-quantiles, three values are returned.

  > [!TIP]
  > **Tip:** To extract a set of `q`-quantiles where `q` is the `number` argument, use the `MERGE` and `EXTRACT` functions in the `KLL_QUANTILES.*` functions.

- Alternatively, quantiles might be considered individual `Φ`-quantiles, where
  `Φ` is a real number with `0 <= Φ <= 1`. The `Φ`-quantile `x` is an element
  of the input such that a `Φ` fraction of the input is less than or equal to
  `x`, and a `(1-Φ)` fraction is greater than or equal to `x`. In this
  notation, the median is the 0.5-quantile, and the 95th percentile is the
  0.95-quantile.

  > [!TIP]
  > **Tip:** To extract individual `Φ`-quantiles, use the quantile-supporting `MERGE_POINT` and `EXTRACT_POINT` functions, where `Φ` is the `phi` argument.

For example, you can use a quantiles-supporting sketch to get the median of the
number of times an application is opened by users.

## Approximate aggregate functions

As an alternative to specific sketch-based approximation functions,
GoogleSQL provides predefined approximate aggregate
functions. These approximate aggregate functions support sketches for common
estimations such as distinct count, quantiles, and top count, but they don't
allow custom precision. They also don't expose and store the sketch for
re-aggregation like other types of sketches. The approximate aggregate functions
are designed for running quick sketch-based queries without detailed
configuration.

For a list of approximate aggregate functions that you can use with
sketch-based approximation, see
[Approximate aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions).