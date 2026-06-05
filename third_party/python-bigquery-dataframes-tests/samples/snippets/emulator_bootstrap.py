# Emulator harness: wire bigframes clients to the local gateway + storage gRPC
# listeners without TLS (the docker-compose stack serves plaintext gRPC).

from __future__ import annotations

import os
from typing import Optional

import grpc
from google.cloud.bigquery_storage_v1 import BigQueryReadClient
from google.cloud.bigquery_storage_v1.services.big_query_read.transports import (
    BigQueryReadGrpcTransport,
)

_PATCHED = False
_ORIGINAL_PROPERTY = None


def _insecure_storage_client(endpoint: str) -> BigQueryReadClient:
    transport = BigQueryReadGrpcTransport(
        channel=grpc.insecure_channel(endpoint),
    )
    return BigQueryReadClient(transport=transport)


def _patched_bqstoragereadclient(self):
    endpoint = os.environ.get("BIGQUERY_STORAGE_GRPC_ENDPOINT", "localhost:9060")
    if os.environ.get("BIGQUERY_EMULATOR_HOST"):
        return _insecure_storage_client(endpoint)
    return _ORIGINAL_PROPERTY(self)


def apply_emulator_client_patches() -> None:
    global _PATCHED, _ORIGINAL_PROPERTY
    if _PATCHED or not os.environ.get("BIGQUERY_EMULATOR_HOST"):
        return
    from bigframes.session import clients

    if _ORIGINAL_PROPERTY is None:
        _ORIGINAL_PROPERTY = clients.ClientsProvider.bqstoragereadclient.fget
        clients.ClientsProvider.bqstoragereadclient = property(
            _patched_bqstoragereadclient
        )
    _PATCHED = True


def configure_bigframes_emulator_endpoints() -> None:
    host = os.environ.get("BIGQUERY_EMULATOR_HOST", "")
    if not host:
        return
    if not host.startswith("http"):
        host = f"http://{host.lstrip('/')}"
    storage_endpoint: Optional[str] = os.environ.get(
        "BIGQUERY_STORAGE_GRPC_ENDPOINT", "localhost:9060"
    )
    import bigframes.pandas as bpd

    bpd.options.bigquery.client_endpoints_override = {
        "bqclient": host,
        "bqstoragereadclient": storage_endpoint,
    }
    apply_emulator_client_patches()
