#!/usr/bin/env python3
"""Mirror public GCS objects into testdata/fake-gcs-data using the JSON API (no gcloud).

Uses anonymous HTTPS against storage.googleapis.com. Same on-disk layout as
scripts/sync_fake_gcs_public_samples.sh for fake-gcs-server.
"""
from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path


def _repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def _list_objects(bucket: str, prefix: str) -> list[str]:
    """Return object names under prefix (recursive, no delimiter)."""
    names: list[str] = []
    page_token: str | None = None
    base = "https://storage.googleapis.com/storage/v1/b/"
    while True:
        q: dict[str, str] = {"prefix": prefix, "maxResults": "1000"}
        if page_token:
            q["pageToken"] = page_token
        url = f"{base}{urllib.parse.quote(bucket, safe='')}/o?" + urllib.parse.urlencode(
            q
        )
        with urllib.request.urlopen(url, timeout=120) as resp:
            data = json.load(resp)
        for item in data.get("items") or []:
            n = item.get("name")
            if n and not n.endswith("/"):
                names.append(n)
        page_token = data.get("nextPageToken")
        if not page_token:
            break
    return names


def _download_object(bucket: str, name: str, dest_file: Path) -> None:
    dest_file.parent.mkdir(parents=True, exist_ok=True)
    enc = urllib.parse.quote(name, safe="/")
    url = f"https://storage.googleapis.com/{urllib.parse.quote(bucket, safe='')}/{enc}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=300) as resp:
        body = resp.read()
    dest_file.write_bytes(body)


def _sync_prefix(bucket: str, prefix: str, dest_root: Path) -> None:
    if not prefix.endswith("/"):
        prefix = prefix + "/"
    names = _list_objects(bucket, prefix)
    if not names:
        print(f"warning: no objects under gs://{bucket}/{prefix}", file=sys.stderr)
        return
    root = dest_root / bucket
    for name in names:
        rel = name
        out = root / rel
        print(f"  {name}")
        _download_object(bucket, name, out)


def _sync_file(bucket: str, name: str, dest_root: Path) -> None:
    root = dest_root / bucket
    out = root / name
    print(f"  {name}")
    _download_object(bucket, name, out)


def _sync_glob_avro(bucket: str, prefix_dir: str, dest_root: Path) -> None:
    """prefix_dir ends with /; download objects matching products_*.avro."""
    names = [n for n in _list_objects(bucket, prefix_dir) if n.endswith(".avro")]
    names = [n for n in names if Path(n).name.startswith("products_")]
    root = dest_root / bucket
    for name in names:
        out = root / name
        print(f"  {name}")
        _download_object(bucket, name, out)


def main() -> int:
    dest_root = _repo_root() / "testdata" / "fake-gcs-data"
    print(f"Syncing (HTTP) into {dest_root}")

    # cloud-samples-data — single files
    for name in (
        "bigquery/us-states/us-states.csv",
        "bigquery/sample-transactions/transactions.csv",
        "vertex-ai/bigframe/df.csv",
        "bigquery/ml/onnx/pipeline_rf.onnx",
    ):
        _sync_file("cloud-samples-data", name, dest_root)

    # cymbal-pets prefixes
    for prefix in (
        "bigquery/tutorials/cymbal-pets/images/",
        "bigquery/tutorials/cymbal-pets/documents/",
        "bigquery/tutorials/cymbal-pets/document_chunks/",
    ):
        print(f"gs://cloud-samples-data/{prefix}")
        _sync_prefix("cloud-samples-data", prefix, dest_root)

    print("gs://cloud-samples-data/.../tables/products/products_*.avro")
    _sync_glob_avro(
        "cloud-samples-data",
        "bigquery/tutorials/cymbal-pets/tables/products/",
        dest_root,
    )

    print("gs://cloud-training-demos/txtclass/export/exporter/1549825580/")
    _sync_prefix(
        "cloud-training-demos",
        "txtclass/export/exporter/1549825580/",
        dest_root,
    )

    print("gs://ibis-testing-libraries/lodash.min.js")
    _sync_file("ibis-testing-libraries", "lodash.min.js", dest_root)

    print(
        "Done. Recreate fake-gcs: docker compose up -d --force-recreate fake-gcs-server"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except urllib.error.HTTPError as e:
        print(f"HTTP error: {e.code} {e.reason}", file=sys.stderr)
        raise SystemExit(1) from e
