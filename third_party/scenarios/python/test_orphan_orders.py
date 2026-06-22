"""Orphan orders anti-join over QUALIFY-deduped views.

Mirrors the reported transpiler binding-loss flow
(``conformance/differential/corpus/orphan_orders_antijoin.yaml``).
"""

from __future__ import annotations

from google.cloud import bigquery

from conftest import make_dataset_id

# Production oracle: conformance/differential/oracle/orphan_orders_antijoin.json
EXPECTED_ORPHAN_ORDER_IDS = [2]


def test_orphan_orders(
    client: bigquery.Client,
    project_id: str,
    run_id: str,
    datasets_to_delete: list[str],
) -> None:
    dataset_id = make_dataset_id("orphan", run_id)
    dataset_ref = f"{project_id}.{dataset_id}"
    datasets_to_delete.append(dataset_id)

    client.create_dataset(bigquery.Dataset(dataset_ref))

    orders_ref = f"{dataset_ref}.orders"
    profiles_ref = f"{dataset_ref}.profiles"
    client.create_table(
        bigquery.Table(
            orders_ref,
            schema=[
                bigquery.SchemaField("order_id", "INT64"),
                bigquery.SchemaField("customer_id", "INT64"),
            ],
        )
    )
    client.create_table(
        bigquery.Table(
            profiles_ref,
            schema=[
                bigquery.SchemaField("id", "INT64"),
                bigquery.SchemaField("name", "STRING"),
            ],
        )
    )
    assert client.insert_rows_json(
        orders_ref,
        [
            {"order_id": 1, "customer_id": 10},
            {"order_id": 2, "customer_id": 99},
        ],
    ) == []
    assert client.insert_rows_json(
        profiles_ref,
        [{"id": 10, "name": "alice"}],
    ) == []

    # tables.insert registers views in the engine view registry (same path as
    # authorize_view); CREATE VIEW via jobs.query only updates gateway metadata.
    v_orders = bigquery.Table(f"{dataset_ref}.v_orders")
    v_orders.view_query = f"""
        SELECT * FROM `{orders_ref}`
        QUALIFY ROW_NUMBER() OVER (PARTITION BY order_id ORDER BY order_id) = 1
    """
    client.create_table(v_orders)

    v_profiles = bigquery.Table(f"{dataset_ref}.v_profiles")
    v_profiles.view_query = f"""
        SELECT * FROM `{profiles_ref}`
        QUALIFY ROW_NUMBER() OVER (PARTITION BY id ORDER BY id) = 1
    """
    client.create_table(v_profiles)

    rows = list(
        client.query(
            f"""
            SELECT o.order_id
            FROM `{dataset_ref}.v_orders` o
            LEFT JOIN `{dataset_ref}.v_profiles` p ON o.customer_id = p.id
            WHERE p.id IS NULL
            ORDER BY o.order_id
            """
        ).result()
    )
    assert [row["order_id"] for row in rows] == EXPECTED_ORPHAN_ORDER_IDS
