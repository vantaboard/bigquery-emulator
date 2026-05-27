#!/usr/bin/env bash
# List BigQuery-related third_party sample locations for docs and CI.
# Run from repository root.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

echo "=== python-bigquery-tests (vendored python-bigquery) — samples/ subdirs ==="
if [[ -d third_party/python-bigquery-tests/samples ]]; then
  find third_party/python-bigquery-tests/samples -mindepth 1 -maxdepth 1 -type d | LC_ALL=C sort
else
  echo "(missing: third_party/python-bigquery-tests)"
fi

echo ""
echo "=== python-bigquery-dataframes-tests (vendored) — samples/snippets ==="
if [[ -d third_party/python-bigquery-dataframes-tests/samples/snippets ]]; then
  find third_party/python-bigquery-dataframes-tests/samples/snippets -maxdepth 1 -name '*_test.py' | LC_ALL=C sort
  echo "(nox session py; task thirdparty:python-bigquery-dataframes-tests)"
else
  echo "(missing: third_party/python-bigquery-dataframes-tests)"
fi

echo ""
echo "=== node-bigquery-tests (vendored nodejs-bigquery samples) ==="
if [[ -d third_party/node-bigquery-tests ]]; then
  find third_party/node-bigquery-tests -maxdepth 2 \( -name 'package.json' -o -name '*.test.js' \) | LC_ALL=C sort | head -200
  echo "(package.json and test/*.js at vendored root)"
else
  echo "(missing: third_party/node-bigquery-tests)"
fi
