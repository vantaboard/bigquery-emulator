"""Dashboard-style parameterized rollup query.

Combines COUNTIF, JSON_EXTRACT_SCALAR, TIMESTAMP_SUB, UNION DISTINCT inside a
CTE, plus a naive TIMESTAMP query parameter
(``conformance/differential/corpus/timestamp_param_naive.yaml`` and
``dashboard_cte_rollup`` shape with parenthesized set ops for production SQL).
"""

from __future__ import annotations

from datetime import datetime, timezone

from google.cloud import bigquery
import pytest

# Oracle: conformance/differential/oracle/timestamp_param_naive.json
# 1782122400000000 microseconds since epoch == 2026-06-22 10:00:00 UTC
EXPECTED_REFERENCE_DT = datetime(2026, 6, 22, 10, 0, 0, tzinfo=timezone.utc)

DASHBOARD_SQL = """
WITH events AS (
  SELECT * FROM (
    SELECT TIMESTAMP '2026-01-01 08:00:00 UTC' AS ts, 'click' AS kind
    UNION ALL
    SELECT TIMESTAMP '2026-01-02 08:00:00 UTC', 'view'
  )
  UNION DISTINCT
  SELECT TIMESTAMP '2026-01-01 08:00:00 UTC', 'click'
)
SELECT
  COUNTIF(kind = 'click') AS clicks,
  JSON_EXTRACT_SCALAR('{"region":"us"}', '$.region') AS region,
  FORMAT_TIMESTAMP('%Y-%m-%d', TIMESTAMP_SUB(MAX(ts), INTERVAL 1 DAY)) AS day_before_max,
  @reference_dt AS reference_dt
FROM events
"""


@pytest.mark.xfail(
    reason="naive TIMESTAMP param rejection (plan 04-params / timestamp_param_naive)",
    strict=False,
)
def test_dashboard_params(client: bigquery.Client) -> None:
    job_config = bigquery.QueryJobConfig(
        query_parameters=[
            bigquery.ScalarQueryParameter(
                "reference_dt",
                "TIMESTAMP",
                "2026-06-22T10:00:00",
            )
        ]
    )
    rows = list(client.query(DASHBOARD_SQL, job_config=job_config).result())
    assert len(rows) == 1
    row = rows[0]
    assert row["clicks"] == 1
    assert row["region"] == "us"
    assert row["day_before_max"] == "2026-01-01"
    assert row["reference_dt"] == EXPECTED_REFERENCE_DT
