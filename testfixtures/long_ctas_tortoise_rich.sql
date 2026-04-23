-- Rich CTAS (Dataform-style): multiple CTEs, unused CTE, GROUP BY, HAVING, window + filter, pivot-ish
-- aggregates, unpivot via UNION. tunable: OUTER, INNER in the "heavy" cross join.
--
-- Automated harness (same semantics, tunable, CTAS-detectable for the emulator) lives in
-- server/ctas_engine_harness_test.go (harnessTortoiseRichCTASSQL) — it uses ROW_NUMBER + WHERE
-- instead of QUALIFY on the final CreateTable+Query, because QUALIFY in the outer select breaks
-- IsCTASQuery (handler would fall back to Query + bad destination schema).
--
-- For raw BigQuery or googlesqlengine CLI experiments, the forms below are valid. Replace placeholders.

-- 1) Minimal "heavy" (same as long_ctas_tortoise.sql)
-- CREATE TABLE `YOUR_DATASET.your_table` AS
-- SELECT COUNT(*) AS c
-- FROM UNNEST(GENERATE_ARRAY(1, 6000)) a
-- CROSS JOIN UNNEST(GENERATE_ARRAY(1, 350)) b;

-- 2) QUALIFY (BigQuery) — often clearer than wrapping ROW_NUMBER; see note above re: emulator + Dst.
-- WITH heavy AS (SELECT 1) SELECT 1;  -- (template only)

-- 3) PIVOT / UNPIVOT: supported in googlesqlengine (go-googlesql-engine/query_test.go). Example shape:
-- WITH q_sales AS (SELECT 'item' product, 10 s, 'Q1' quarter)
-- SELECT * FROM q_sales PIVOT(SUM(s) FOR quarter IN ('Q1', 'Q2'));
--
-- 4) Optional: GROUP BY over the *large* `heavy` CTE (very slow vs minimal CTAS; use to stress
--    hash-aggregate on huge joins). For default automated tortoise, harnessTortoiseRichCTASSQL uses
--    a small UNNEST for `bucketed` so wall time is dominated by the same cross join as the minimal
--    tortoise, not a second O(N) pass over N = OUTER*INNER.

CREATE TABLE `YOUR_DATASET.your_table_rich` AS
WITH
  dead_cte AS (
    SELECT 999 AS never_used, 'unused' AS marker
  ),
  heavy AS (
    SELECT a, b
    FROM UNNEST(GENERATE_ARRAY(1, 5000)) a
    CROSS JOIN UNNEST(GENERATE_ARRAY(1, 300)) b
  ),
  bucketed AS (
    SELECT MOD(x, 3) AS bucket, COUNT(*) AS n
    FROM UNNEST(GENERATE_ARRAY(1, 99)) x
    GROUP BY 1
    HAVING COUNT(*) > 0
  ),
  bucket_qualified AS (
    SELECT bucket, n
    FROM (
      SELECT bucket, n, ROW_NUMBER() OVER (ORDER BY n DESC) AS rn
      FROM bucketed
    ) z
    WHERE z.rn <= 3
  ),
  q_sales AS (
    SELECT 'item' AS product, 10 AS s, 'Q1' AS quarter
    UNION ALL
    SELECT 'item', 20, 'Q2'
  ),
  pivotish AS (
    SELECT
      product,
      SUM(CASE quarter WHEN 'Q1' THEN s END) AS q1,
      SUM(CASE quarter WHEN 'Q2' THEN s END) AS q2
    FROM q_sales
    GROUP BY product
  ),
  unpivish AS (
    SELECT v AS val, t AS qname
    FROM (
      SELECT q1 AS v, 'Q1' AS t FROM pivotish
      UNION ALL
      SELECT q2, 'Q2' FROM pivotish
    )
  )
SELECT
  (SELECT COUNT(*) AS cnt FROM heavy) + 0
    * COALESCE((SELECT MAX(n) FROM bucket_qualified), 0)
  + 0
    * COALESCE((SELECT MAX(val) FROM unpivish), 0) AS c;
