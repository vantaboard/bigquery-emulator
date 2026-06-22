"""Authorize a view on a source dataset across multiple tenants.

Mirrors the reported ``client.update_dataset(..., ['access_entries'])`` flow
that previously aborted the engine when repeated (plan 03 /
``conformance/sessions/authorize_view_repeat.yaml``).
"""

from __future__ import annotations

from google.cloud import bigquery
from google.cloud.bigquery.enums import EntityTypes
import pytest

from conftest import make_dataset_id

NUM_TENANTS = 3

# Production oracle: three profile rows from authorize_view_repeat session fixture.
EXPECTED_ROWS = [
    {"id": 1, "name": "ada"},
    {"id": 2, "name": "linus"},
    {"id": 3, "name": "grace"},
]


def test_authorize_view_multi_tenant(
    client: bigquery.Client,
    project_id: str,
    run_id: str,
    datasets_to_delete: list[str],
) -> None:
    source_dataset_id = make_dataset_id("src", run_id)
    source_ref = f"{project_id}.{source_dataset_id}"
    datasets_to_delete.append(source_dataset_id)

    source_dataset = bigquery.Dataset(source_ref)
    source_dataset.location = "US"
    client.create_dataset(source_dataset)

    table_ref = f"{source_ref}.profiles"
    table = bigquery.Table(
        table_ref,
        schema=[
            bigquery.SchemaField("id", "INT64", mode="REQUIRED"),
            bigquery.SchemaField("name", "STRING"),
        ],
    )
    client.create_table(table)
    errors = client.insert_rows_json(table_ref, EXPECTED_ROWS)
    assert errors == []

    tenant_specs: list[tuple[str, bigquery.Table]] = []
    for tenant_idx in range(NUM_TENANTS):
        tenant_dataset_id = make_dataset_id("tenant", run_id, str(tenant_idx))
        tenant_ref = f"{project_id}.{tenant_dataset_id}"
        datasets_to_delete.append(tenant_dataset_id)

        tenant_dataset = bigquery.Dataset(tenant_ref)
        tenant_dataset.location = "US"
        client.create_dataset(tenant_dataset)

        view = bigquery.Table(f"{tenant_ref}.v_profiles")
        view.view_query = f"SELECT id, name FROM `{source_ref}.profiles`"
        view = client.create_table(view)
        tenant_specs.append((tenant_dataset_id, view))

        source_dataset = client.get_dataset(source_ref)
        access_entries = list(source_dataset.access_entries)
        access_entries.append(
            bigquery.AccessEntry(
                None,
                EntityTypes.VIEW,
                view.reference.to_api_repr(),
            )
        )
        source_dataset.access_entries = access_entries
        client.update_dataset(source_dataset, ["access_entries"])

        rows = list(client.query(f"SELECT id, name FROM `{tenant_ref}.v_profiles` ORDER BY id").result())
        assert [dict(row.items()) for row in rows] == EXPECTED_ROWS

        # Repeat the authorize + replace cycle that triggered the abort in production.
        view.view_query = f"SELECT id, name FROM `{source_ref}.profiles` WHERE id IS NOT NULL"
        client.update_table(view, ["view_query"])
        source_dataset = client.get_dataset(source_ref)
        client.update_dataset(source_dataset, ["access_entries"])

    # Final sanity: every tenant still reads through the authorized view.
    for tenant_dataset_id, _view in tenant_specs:
        tenant_ref = f"{project_id}.{tenant_dataset_id}"
        rows = list(client.query(f"SELECT COUNT(*) AS n FROM `{tenant_ref}.v_profiles`").result())
        assert rows[0]["n"] == len(EXPECTED_ROWS)
