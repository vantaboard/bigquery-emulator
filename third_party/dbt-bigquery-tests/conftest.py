"""Emulator-aware pytest wiring for vendored dbt-bigquery functional tests.

Upstream oauth/service-account profiles require live GCP credentials.
When ``BIGQUERY_EMULATOR_HOST`` is set, ``--profile=emulator`` (the task
default) supplies ``api_endpoint`` and anonymous auth via
``emulator_bootstrap.py``.
"""

from __future__ import annotations

import os

import pytest

pytest_plugins = [
    "dbt.tests.fixtures.project",
    "emulator_bootstrap",
    "emulator_pytest_skip",
]


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption("--profile", action="store", default="emulator", type=str)


def _normalize_emulator_host(raw: str) -> str:
    host = raw.strip()
    if host and not host.startswith(("http://", "https://")):
        host = f"http://{host.lstrip('/')}"
    return host


def emulator_target() -> dict:
    host = _normalize_emulator_host(
        os.environ.get("BIGQUERY_EMULATOR_HOST", "http://localhost:9050")
    )
    project = os.environ.get("GOOGLE_CLOUD_PROJECT", "dev")
    # BigQuery profile aliases: project->database. Schema/dataset is injected by
    # dbt.tests.fixtures.project::dbt_profile_data (unique_schema per class).
    return {
        "type": "bigquery",
        "method": "oauth",
        "threads": 1,
        "job_retries": 0,
        "project": project,
        "execution_project": project,
        "api_endpoint": host,
        "location": os.environ.get("DBT_BIGQUERY_LOCATION", "US"),
    }


def oauth_target() -> dict:
    return {
        "type": "bigquery",
        "method": "oauth",
        "threads": 4,
        "job_retries": 2,
        "compute_region": os.getenv("COMPUTE_REGION") or os.getenv("DATAPROC_REGION"),
        "dataproc_cluster_name": os.getenv("DATAPROC_CLUSTER_NAME"),
        "gcs_bucket": os.getenv("GCS_BUCKET"),
    }


def service_account_target() -> dict:
    import json

    from dbt.adapters.bigquery.credentials import _base64_to_string, _is_base64

    credentials_json_str = os.getenv("BIGQUERY_TEST_SERVICE_ACCOUNT_JSON", "").replace("'", "")
    if not credentials_json_str:
        raise ValueError(
            "BIGQUERY_TEST_SERVICE_ACCOUNT_JSON is required for --profile=service_account"
        )
    if _is_base64(credentials_json_str):
        credentials_json_str = _base64_to_string(credentials_json_str)
    credentials = json.loads(credentials_json_str)
    project_id = os.getenv("BIGQUERY_TEST_PROJECT") or credentials.get("project_id")
    execution_project = os.getenv("BIGQUERY_TEST_EXECUTION_PROJECT") or project_id
    return {
        "type": "bigquery",
        "method": "service-account-json",
        "threads": 4,
        "job_retries": 2,
        "project": project_id,
        "execution_project": execution_project,
        "keyfile_json": credentials,
        "compute_region": os.getenv("COMPUTE_REGION") or os.getenv("DATAPROC_REGION"),
        "dataproc_cluster_name": os.getenv("DATAPROC_CLUSTER_NAME"),
        "gcs_bucket": os.getenv("GCS_BUCKET"),
    }


@pytest.fixture(scope="class")
def dbt_profile_target(request: pytest.FixtureRequest) -> dict:
    profile_type = request.config.getoption("--profile")
    if profile_type == "emulator":
        return emulator_target()
    if profile_type == "oauth":
        return oauth_target()
    if profile_type == "service_account":
        return service_account_target()
    raise ValueError(f"Invalid profile type '{profile_type}'")
