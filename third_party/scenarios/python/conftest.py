"""Shared fixtures for first-party google-cloud-bigquery workflow scenarios."""

from __future__ import annotations

import os
import uuid
from typing import Iterator, List

from google.cloud import bigquery
import pytest

pytest_plugins = ["emulator_bootstrap"]


def _normalize_emulator_host(raw: str) -> str:
    host = raw.strip()
    if host and not host.startswith(("http://", "https://")):
        host = f"http://{host.lstrip('/')}"
    return host


@pytest.fixture(scope="session")
def emulator_host() -> str:
    return _normalize_emulator_host(
        os.environ.get("BIGQUERY_EMULATOR_HOST", "http://localhost:9050")
    )


@pytest.fixture(scope="session")
def client(emulator_host: str) -> bigquery.Client:
    if not os.environ.get("BIGQUERY_EMULATOR_HOST"):
        pytest.skip("BIGQUERY_EMULATOR_HOST is unset")
    os.environ["BIGQUERY_EMULATOR_HOST"] = emulator_host
    os.environ.setdefault("GOOGLE_CLOUD_PROJECT", "dev")
    return bigquery.Client()


@pytest.fixture(scope="session")
def project_id(client: bigquery.Client) -> str:
    return client.project


@pytest.fixture
def run_id() -> str:
    return uuid.uuid4().hex[:12]


@pytest.fixture
def datasets_to_delete(client: bigquery.Client) -> Iterator[List[str]]:
    doomed: List[str] = []
    yield doomed
    for dataset_id in doomed:
        client.delete_dataset(dataset_id, delete_contents=True, not_found_ok=True)


def make_dataset_id(prefix: str, run_id: str, suffix: str = "") -> str:
    base = f"scn_{prefix}_{run_id}"
    if suffix:
        base = f"{base}_{suffix}"
    return base[:1024]
