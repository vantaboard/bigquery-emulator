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

collect_registered_packages() {
  local catalog="${data_dir}/catalog.duckdb"
  if [[ ! -f "$catalog" ]]; then
    return 0
  fi
  if ! "${venv}/bin/python3" -c "import duckdb" 2>/dev/null; then
    "${venv}/bin/pip" install --quiet duckdb
  fi
  "${venv}/bin/python3" - "$catalog" <<'PY'
import json
import re
import sys

import duckdb

catalog = sys.argv[1]
con = duckdb.connect(catalog, read_only=True)
rows = con.execute(
    "SELECT ddl_sql FROM main.__bqemu_routines "
    "WHERE lower(language) = 'python'"
).fetchall()
packages = []
seen = set()
for (ddl,) in rows:
    for match in re.finditer(r"packages\s*=\s*\[([^\]]*)\]", ddl, re.I):
        for quoted in re.findall(r"'([^']+)'|\"([^\"]+)\"", match.group(1)):
            pkg = quoted[0] or quoted[1]
            if pkg and pkg not in seen:
                seen.add(pkg)
                packages.append(pkg)
print(json.dumps(packages))
PY
}

if [[ "$#" -eq 0 ]]; then
  packages_json="$(collect_registered_packages || true)"
  if [[ -z "$packages_json" || "$packages_json" == "[]" ]]; then
    echo "python-udf:provision: no Python UDF packages found in __bqemu_routines; venv ready at ${venv}" >&2
    exit 0
  fi
  mapfile -t packages < <("${venv}/bin/python3" -c 'import json,sys; print("\n".join(json.load(sys.stdin)))' <<<"$packages_json")
  if [[ ${#packages[@]} -eq 0 ]]; then
    echo "python-udf:provision: no Python UDF packages found in __bqemu_routines; venv ready at ${venv}" >&2
    exit 0
  fi
  echo "python-udf:provision: installing union from __bqemu_routines: ${packages[*]}" >&2
  set -- "${packages[@]}"
fi

"${venv}/bin/pip" install --upgrade pip
"${venv}/bin/pip" install "$@"
echo "python-udf:provision: installed packages into ${venv}"
