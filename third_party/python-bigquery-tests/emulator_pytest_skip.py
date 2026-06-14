"""Pytest plugin: skip out-of-scope sample tests when BIGQUERY_EMULATOR_HOST is set.

Families skipped here mirror the golang third-party matrix and
docs/ENGINE_POLICY.md (BQML, legacy SQL, public-data fixtures,
Gemini/multimodal imports). Loaded via ``-p emulator_pytest_skip`` from
``task thirdparty:python-bigquery-tests`` when the emulator is targeted.
"""

from __future__ import annotations

import os
from pathlib import Path

import pytest

_REASON = (
    "skipped on BigQuery emulator (unsupported or out-of-scope; "
    "see docs/ENGINE_POLICY.md and third_party/README.md)"
)

# Module path substrings that are never run against the emulator.
# samples/tests that require Google Sheets API (no emulator stub).
_SAMPLES_EMULATOR_SKIP: frozenset[str] = frozenset(
    {
        "test_query_external_sheets_permanent_table",
        "test_query_external_sheets_temporary_table",
    }
)

_MODULE_SKIP_SUBSTRINGS: tuple[str, ...] = (
    "model",
    "legacy",
    "download_public",
    "gemini",
    "bqml",
    "multimodal",
    "onnx",
    "tensorflow",
)

# BQML inference TVFs are local_stub (NULL placeholders), but most
# model/bqml modules also assert model metadata APIs or real prediction
# values — keep skipping those on the emulator.

# Fixtures whose setup implies BQML or pre-seeded public catalog tables.
_SKIP_FIXTURES: frozenset[str] = frozenset({"model_id", "table_with_data_id"})

_PUBLIC_DATA_MARKER = "bigquery-public-data"
_LEGACY_SQL_MARKERS = ("use_legacy_sql", "useLegacySql")

# Tables seeded at gateway startup (testdata/public-data/bigquery-public-data.yaml).
_SEEDED_PUBLIC_TABLES: frozenset[str] = frozenset(
    {
        "bigquery-public-data.usa_names.usa_1910_2013",
        "bigquery-public-data.samples.shakespeare",
        "bigquery-public-data.stackoverflow.posts_questions",
    }
)

# docs/snippets.py tests that run without public-data fixtures.
_DOCS_SNIPPETS_EMULATOR_ALLOW: frozenset[str] = frozenset(
    {
        "test_create_client_default_credentials",
        "test_update_table_description",
        "test_update_table_cmek",
        "test_load_table_add_column",
        "test_load_table_relax_column",
    }
)

# docs/snippets.py tests that require resumable upload or other REST gaps.
_DOCS_SNIPPETS_EMULATOR_SKIP: frozenset[str] = frozenset()

_docs_snippets_source: str | None = None


def _docs_snippets_module_source() -> str:
    global _docs_snippets_source
    if _docs_snippets_source is None:
        path = Path(__file__).resolve().parent / "docs" / "snippets.py"
        try:
            _docs_snippets_source = path.read_text(encoding="utf-8")
        except OSError:
            _docs_snippets_source = ""
    return _docs_snippets_source


def _docs_snippets_test_body(test_name: str) -> str:
    source = _docs_snippets_module_source()
    if not source:
        return ""
    needle = f"def {test_name}"
    start = source.find(needle)
    if start < 0:
        return ""
    next_def = source.find("\ndef ", start + len(needle))
    if next_def < 0:
        return source[start:]
    return source[start:next_def]


def _docs_snippets_test_should_skip(test_name: str) -> bool:
    if test_name in _DOCS_SNIPPETS_EMULATOR_SKIP:
        return True
    if test_name in _DOCS_SNIPPETS_EMULATOR_ALLOW:
        return False
    body = _docs_snippets_test_body(test_name)
    if not body:
        return True
    return _public_data_text_should_skip(body)


def _emulator_mode() -> bool:
    return bool(os.environ.get("BIGQUERY_EMULATOR_HOST"))


def _item_path(item: pytest.Item) -> Path:
    if hasattr(item, "path"):
        return Path(str(item.path))
    return Path(str(item.fspath))


def _path_matches(path: Path, substrings: tuple[str, ...]) -> bool:
    lower = str(path).lower().replace("\\", "/")
    return any(s in lower for s in substrings)


def _sample_module_for_test(test_path: Path) -> Path | None:
    """Map samples/tests/test_foo.py -> samples/foo.py."""
    name = test_path.name
    if not name.startswith("test_") or test_path.suffix != ".py":
        return None
    if test_path.parent.name != "tests":
        return None
    sample_name = name[len("test_") :]
    return test_path.parent.parent / sample_name


def _public_data_refs_in_text(text: str) -> set[str]:
    import re

    refs: set[str] = set()
    for match in re.finditer(
        r"bigquery-public-data[.:]([a-zA-Z0-9_]+)[.:]([a-zA-Z0-9_]+)", text
    ):
        refs.add(
            f"bigquery-public-data.{match.group(1)}.{match.group(2)}"
        )
    return refs


def _public_data_text_should_skip(text: str) -> bool:
    if _PUBLIC_DATA_MARKER not in text:
        return any(marker in text for marker in _LEGACY_SQL_MARKERS)
    refs = _public_data_refs_in_text(text)
    if not refs:
        return True
    return bool(refs - _SEEDED_PUBLIC_TABLES) or any(
        marker in text for marker in _LEGACY_SQL_MARKERS
    )


def _sample_source_indicates_skip(sample_path: Path) -> bool:
    try:
        text = sample_path.read_text(encoding="utf-8")
    except OSError:
        return False
    return _public_data_text_should_skip(text)


def pytest_collection_modifyitems(config: pytest.Config, items: list[pytest.Item]) -> None:
    if not _emulator_mode():
        return

    for item in items:
        path = _item_path(item)
        if _path_matches(path, _MODULE_SKIP_SUBSTRINGS):
            item.add_marker(pytest.mark.skip(reason=_REASON))
            continue
        if _SKIP_FIXTURES.intersection(getattr(item, "fixturenames", ())):
            item.add_marker(pytest.mark.skip(reason=_REASON))
            continue
        if path.name == "snippets.py" and "docs" in path.parts:
            if _docs_snippets_test_should_skip(item.name):
                item.add_marker(pytest.mark.skip(reason=_REASON))
                continue
        if item.name in _SAMPLES_EMULATOR_SKIP:
            item.add_marker(pytest.mark.skip(reason=_REASON))
            continue
        sample = _sample_module_for_test(path)
        if sample is not None and _sample_source_indicates_skip(sample):
            item.add_marker(pytest.mark.skip(reason=_REASON))
