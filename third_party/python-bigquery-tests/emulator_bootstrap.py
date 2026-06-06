"""Pytest plugin: route BigQuery Storage reads to the emulator gRPC listener.

``list_rows(...).to_dataframe()`` and similar paths call
``Client._ensure_bqstorage_client()``, which otherwise dials production
``bigquerystorage.googleapis.com``. When ``BIGQUERY_EMULATOR_HOST`` is set,
patch that hook to use an insecure channel to ``BIGQUERY_STORAGE_GRPC_ENDPOINT``
(the gateway storage shim on :9060 in docker-compose).
"""

from __future__ import annotations

import os

import grpc
import pytest

_PATCHED = False


def _storage_endpoint() -> str:
    return os.environ.get("BIGQUERY_STORAGE_GRPC_ENDPOINT", "localhost:9060")


def _insecure_bqstorage_client():
    from google.cloud.bigquery_storage_v1 import BigQueryReadClient
    from google.cloud.bigquery_storage_v1.services.big_query_read.transports import (
        BigQueryReadGrpcTransport,
    )

    transport = BigQueryReadGrpcTransport(
        channel=grpc.insecure_channel(_storage_endpoint()),
    )
    return BigQueryReadClient(transport=transport)


def _patch_client_bqstorage() -> None:
    global _PATCHED
    if _PATCHED or not os.environ.get("BIGQUERY_EMULATOR_HOST"):
        return

    from google.cloud import bigquery

    original = bigquery.Client._ensure_bqstorage_client

    def patched(self, bqstorage_client=None, client_options=None, client_info=None):
        if bqstorage_client is not None:
            return original(self, bqstorage_client, client_options, client_info)
        return _insecure_bqstorage_client()

    bigquery.Client._ensure_bqstorage_client = patched
    _PATCHED = True


def pytest_configure(config: pytest.Config) -> None:
    _patch_client_bqstorage()
