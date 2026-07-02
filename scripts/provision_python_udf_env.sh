#!/usr/bin/env bash
# Provision a dedicated Python venv for LANGUAGE python UDFs under
# $BIGQUERY_EMULATOR_DATA_DIR/python-udf-env. Pip runs only when the
# operator opts in via BIGQUERY_EMULATOR_PYTHON_ALLOW_PIP=1.
set -euo pipefail

if [[ "${BIGQUERY_EMULATOR_PYTHON_ALLOW_PIP:-}" != "1" ]]; then
  echo "python-udf:provision: set BIGQUERY_EMULATOR_PYTHON_ALLOW_PIP=1 to enable pip installs" >&2
  echo "See docs/guides/python-udfs.md" >&2
  exit 1
fi

data_dir="${BIGQUERY_EMULATOR_DATA_DIR:-${HOME:-}/.bigquery-emulator}"
if [[ -z "$data_dir" ]]; then
  data_dir="./.bigquery-emulator"
fi

venv="${data_dir}/python-udf-env"
mkdir -p "$data_dir"

if [[ ! -x "${venv}/bin/python3" ]]; then
  python3 -m venv "$venv"
fi

if [[ "$#" -eq 0 ]]; then
  echo "python-udf:provision: no packages specified; venv ready at ${venv}" >&2
  exit 0
fi

"${venv}/bin/pip" install --upgrade pip
"${venv}/bin/pip" install "$@"
echo "python-udf:provision: installed packages into ${venv}"
