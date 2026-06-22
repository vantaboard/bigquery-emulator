"""CREATE OR REPLACE TABLE … AS SELECT dedup CTAS workflow.

Mirrors the reported DDL single-segment-name regression
(``conformance/fixtures/core_usage/qualified_names/create_or_replace_table_backtick_qualified.yaml``).
"""

from __future__ import annotations

from google.cloud import bigquery
import pytest

from conftest import make_dataset_id

# Production-validated expected row (verified_production: true on fixture above).
EXPECTED_ROWS = [{"id": 1, "inserted_at": 20}]


def test_dedup_ctas(
    client: bigquery.Client,
    project_id: str,
    run_id: str,
    datasets_to_delete: list[str],
) -> None:
    dataset_id = make_dataset_id("dedup", run_id)
    dataset_ref = f"{project_id}.{dataset_id}"
    datasets_to_delete.append(dataset_id)

    client.create_dataset(bigquery.Dataset(dataset_ref))

    src_ref = f"{dataset_ref}.src"
    client.create_table(
        bigquery.Table(
            src_ref,
            schema=[
                bigquery.SchemaField("id", "INT64"),
                bigquery.SchemaField("inserted_at", "INT64"),
            ],
        )
    )
    errors = client.insert_rows_json(
        src_ref,
        [
            {"id": 1, "inserted_at": 10},
            {"id": 1, "inserted_at": 20},
        ],
    )
    assert errors == []

    dedup_sql = f"""
        CREATE OR REPLACE TABLE `{dataset_id}.t_dedup` AS
        SELECT * EXCEPT(rn) FROM (
          SELECT
            *,
            ROW_NUMBER() OVER (PARTITION BY id ORDER BY inserted_at DESC) AS rn
          FROM `{src_ref}`
        )
        WHERE rn = 1
    """
    client.query(dedup_sql).result()

    rows = list(
        client.query(
            f"SELECT id, inserted_at FROM `{dataset_id}.t_dedup` ORDER BY id"
        ).result()
    )
    assert [dict(row.items()) for row in rows] == EXPECTED_ROWS
