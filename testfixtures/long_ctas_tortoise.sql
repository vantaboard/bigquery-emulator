-- Long-running CTAS: counts rows of a large Cartesian product built from
-- UNNEST(GENERATE_ARRAY(...)) (same pattern as server/concurrency_stress_test.go).
-- Wall time grows with OUTER*INNER (engine work, not a wide result set).
--
-- bigquery-emulator: for the fast path, submit this job with the query destination
-- (BigQuery job QueryConfig.Dst) set to the same table you are creating. That uses
-- QueryCTASInPlace (Exec). If the job has no destination, the server uses the
-- materialize path and the same query can be orders of magnitude slower.
--
-- Tuning: increase OUTER and INNER to stress the engine (e.g. 6000, 350 ≈ 2.1M pairs).
-- Richer Dataform-style CTAS (CTEs, GROUP BY, pivot-ish branches): see
-- long_ctas_tortoise_rich.sql in this directory. The in-repo Go harness is
-- harnessTortoiseRichCTASSQL in server/ctas_engine_harness_test.go (tunable; same in-place Dst rule).
--
-- Replace YOUR_DATASET and your_table. Use the project/dataset your client sends.
CREATE TABLE `YOUR_DATASET.your_table` AS
SELECT COUNT(*) AS c
FROM UNNEST(GENERATE_ARRAY(1, 6000)) AS a
CROSS JOIN UNNEST(GENERATE_ARRAY(1, 350)) AS b;
