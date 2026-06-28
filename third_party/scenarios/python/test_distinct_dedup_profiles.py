"""DISTINCT over deduped profiles — mirrors Chris email R11 flows."""

from __future__ import annotations

from google.cloud import bigquery

from conftest import make_dataset_id

# Production oracle: run the two queries below against BigQuery and paste results.
# bq query --use_legacy_sql=false --format=prettyjson 'SELECT DISTINCT city ...'
EXPECTED_DISTINCT_CITIES = ["Lyon", "Paris"]
EXPECTED_DISTINCT_TAGS = ["news", "vip"]


def test_distinct_dedup_profiles(
    client: bigquery.Client,
    project_id: str,
    run_id: str,
    datasets_to_delete: list[str],
) -> None:
    dataset_id = make_dataset_id("distinct_dedup", run_id)
    dataset_ref = f"{project_id}.{dataset_id}"
    datasets_to_delete.append(dataset_id)

    client.create_dataset(bigquery.Dataset(dataset_ref))

    profiles_ref = f"{dataset_ref}.profiles"
    client.create_table(
        bigquery.Table(
            profiles_ref,
            schema=[
                bigquery.SchemaField("id", "INT64"),
                bigquery.SchemaField("city", "STRING"),
                bigquery.SchemaField("tags", "STRING", mode="REPEATED"),
                bigquery.SchemaField("is_deleted", "BOOL"),
                bigquery.SchemaField("source_updated_at", "TIMESTAMP"),
            ],
        )
    )
    assert client.insert_rows_json(
        profiles_ref,
        [
            {
                "id": 1,
                "city": "Paris",
                "tags": ["vip", "news"],
                "is_deleted": False,
                "source_updated_at": "2025-06-01T00:00:00",
            },
            {
                "id": 1,
                "city": "Paris",
                "tags": ["vip"],
                "is_deleted": False,
                "source_updated_at": "2025-01-01T00:00:00",
            },
            {
                "id": 2,
                "city": "Lyon",
                "tags": ["vip"],
                "is_deleted": False,
                "source_updated_at": "2025-01-01T00:00:00",
            },
            {
                "id": 3,
                "city": None,
                "tags": ["orphan"],
                "is_deleted": False,
                "source_updated_at": "2025-01-01T00:00:00",
            },
        ],
    ) == []

    v_profiles = bigquery.Table(f"{dataset_ref}.v_profiles")
    v_profiles.view_query = f"""
        SELECT * FROM `{profiles_ref}`
        QUALIFY ROW_NUMBER() OVER (
          PARTITION BY id ORDER BY source_updated_at DESC
        ) = 1
    """
    client.create_table(v_profiles)

    city_rows = list(
        client.query(
            f"""
            SELECT DISTINCT city
            FROM `{dataset_ref}.v_profiles`
            WHERE COALESCE(is_deleted, FALSE) = FALSE
              AND city IS NOT NULL
            ORDER BY city
            """
        ).result()
    )
    assert [row["city"] for row in city_rows] == EXPECTED_DISTINCT_CITIES

    tag_rows = list(
        client.query(
            f"""
            SELECT DISTINCT value
            FROM `{dataset_ref}.v_profiles`, UNNEST(tags) AS value
            WHERE COALESCE(is_deleted, FALSE) = FALSE
            ORDER BY value
            """
        ).result()
    )
    assert [row["value"] for row in tag_rows] == EXPECTED_DISTINCT_TAGS
