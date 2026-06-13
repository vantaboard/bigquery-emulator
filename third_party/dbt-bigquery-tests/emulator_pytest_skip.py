"""Pytest plugin: skip dbt-bigquery functional tests the emulator lacks.

Initial skip matrix. Refine during triage once
``task thirdparty:dbt-bigquery-tests`` runs against a live emulator.
"""

from __future__ import annotations

import os
from pathlib import Path

import pytest

_REASON = (
    "skipped on BigQuery emulator (unsupported or out-of-scope; "
    "see third_party/dbt-bigquery-tests/FEASIBILITY.md and third_party/README.md)"
)

# Path substrings under tests/ that are never run against the emulator.
_MODULE_SKIP_SUBSTRINGS: tuple[str, ...] = (
    "python_model",
    "upload_file",
    "dataproc",
    "bigframes",
    "functions/test_js",
    "functions/test_udaf",
    "grant_access",
    "change_history",
    "catalog",
    "override_database",
    "quota_project",
    "json_keyfile",
    "hours_to_expiration",
    "location_change",
    "column_policy",
    "field_description",
    "simple_bigquery_view",
    "materialized",
    "store_test_failures",
    "sources_freshness",
    "unit_testing",
)

# Individual test classes known to need deferred surfaces before triage.
_CLASS_SKIP_MARKERS: dict[str, str] = {
    "TestDocsGenerateBigQuery": "docs generate / INFORMATION_SCHEMA catalog gaps",
    "TestSimpleMaterializationsBigQuery": "triage gate — remove when BaseSimpleMaterializations passes",
}


def _emulator_mode() -> bool:
    return bool(os.environ.get("BIGQUERY_EMULATOR_HOST"))


def _item_path(item: pytest.Item) -> Path:
    if hasattr(item, "path"):
        return Path(str(item.path))
    return Path(str(item.fspath))


def _path_matches(path: Path, substrings: tuple[str, ...]) -> bool:
    lower = str(path).lower().replace("\\", "/")
    return any(s in lower for s in substrings)


def pytest_collection_modifyitems(config: pytest.Config, items: list[pytest.Item]) -> None:
    if not _emulator_mode():
        return

    for item in items:
        path = _item_path(item)
        if _path_matches(path, _MODULE_SKIP_SUBSTRINGS):
            item.add_marker(pytest.mark.skip(reason=_REASON))
            continue
        class_name = getattr(item.cls, "__name__", None)
        if class_name in _CLASS_SKIP_MARKERS:
            # Keep TestSimpleMaterializationsBigQuery runnable when
            # DBT_BIGQUERY_RUN_TRIAGE=1 (feasibility / triage window).
            if class_name == "TestSimpleMaterializationsBigQuery":
                if os.environ.get("DBT_BIGQUERY_RUN_TRIAGE", "").lower() not in (
                    "1",
                    "true",
                    "yes",
                ):
                    item.add_marker(
                        pytest.mark.skip(
                            reason=(
                                f"{_REASON} ({_CLASS_SKIP_MARKERS[class_name]}; "
                                "set DBT_BIGQUERY_RUN_TRIAGE=1 to attempt)"
                            )
                        )
                    )
                continue
            item.add_marker(
                pytest.mark.skip(
                    reason=f"{_REASON} ({_CLASS_SKIP_MARKERS[class_name]})"
                )
            )
