"""Pytest plugin: anonymous BigQuery auth when targeting the emulator.

dbt-bigquery's ``create_google_credentials`` always resolves real Google
auth. The emulator gateway accepts unauthenticated REST traffic, so patch
credential construction when ``BIGQUERY_EMULATOR_HOST`` is set.
"""

from __future__ import annotations

import os

import pytest


def _emulator_mode() -> bool:
    return bool(os.environ.get("BIGQUERY_EMULATOR_HOST"))


def _patch_bigquery_credentials() -> None:
    if not _emulator_mode():
        return

    from google.auth.credentials import AnonymousCredentials

    import dbt.adapters.bigquery.credentials as creds_mod

    if getattr(creds_mod, "_emulator_credentials_patched", False):
        return

    def _anonymous_google_credentials(_credentials):
        return AnonymousCredentials()

    creds_mod.create_google_credentials = _anonymous_google_credentials
    creds_mod._create_google_credentials = _anonymous_google_credentials
    creds_mod._emulator_credentials_patched = True


def pytest_configure(config: pytest.Config) -> None:
    _patch_bigquery_credentials()
