"""Pytest plugin: emulator wiring for first-party workflow scenarios.

When ``BIGQUERY_EMULATOR_HOST`` is set:

- Route BigQuery Storage reads to the emulator gRPC listener (insecure channel).
- Patch ``google.auth.default`` to return ``AnonymousCredentials`` so
  ``bigquery.Client()`` works without Application Default Credentials.
"""

from __future__ import annotations

import os

import grpc
import pytest

_PATCHED = False
_ADC_PATCHED = False


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


def _emulator_project() -> str:
    return (
        os.environ.get("GOOGLE_CLOUD_PROJECT")
        or os.environ.get("GCLOUD_PROJECT")
        or "dev"
    )


def _patch_anonymous_adc() -> None:
    global _ADC_PATCHED
    if _ADC_PATCHED or not os.environ.get("BIGQUERY_EMULATOR_HOST"):
        return

    import google.auth
    from google.auth.credentials import AnonymousCredentials

    def _emulator_default(*args, **kwargs):
        return AnonymousCredentials(), _emulator_project()

    google.auth.default = _emulator_default
    google.auth._bq_emulator_adc_patched = True  # type: ignore[attr-defined]
    _ADC_PATCHED = True


def pytest_configure(config: pytest.Config) -> None:
    _patch_anonymous_adc()
    _patch_client_bqstorage()
