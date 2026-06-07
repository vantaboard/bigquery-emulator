#!/usr/bin/env bash
# Partition bigquery-utils conformance fixtures into passing/ vs known_failing/
# by running the conformance runner once and moving YAML files that PASS.
#
# Usage (from repo root):
#   ./scripts/triage_bqutils_fixtures.sh
#   EMULATOR_BIN=./bin/emulator_main ./scripts/triage_bqutils_fixtures.sh
#   ./scripts/triage_bqutils_fixtures.sh --dry-run
#
# Writes a JSON report to /tmp/bqutils-triage.json (override with TRIAGE_JSON=).

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${ROOT}/conformance/thirdparty-fixtures/bigquery_utils/known_failing"
DST="${ROOT}/conformance/thirdparty-fixtures/bigquery_utils/passing"
EMULATOR_BIN="${EMULATOR_BIN:-${ROOT}/bin/emulator_main}"
TRIAGE_JSON="${TRIAGE_JSON:-/tmp/bqutils-triage.json}"
DRY_RUN=0

for arg in "$@"; do
	case "$arg" in
		--dry-run) DRY_RUN=1 ;;
		-h | --help)
			sed -n '2,/^set -euo pipefail$/p' "$0" | sed 's/^# \{0,1\}//' | head -n -1
			exit 0
			;;
		*)
			echo "triage_bqutils_fixtures.sh: unknown arg '${arg}'" >&2
			exit 2
			;;
	esac
done

if [ ! -x "${EMULATOR_BIN}" ]; then
	echo "triage_bqutils_fixtures.sh: emulator binary not executable: ${EMULATOR_BIN}" >&2
	exit 1
fi

if [ ! -d "${SRC}" ]; then
	echo "triage_bqutils_fixtures.sh: no fixtures at ${SRC}" >&2
	exit 1
fi

mkdir -p "${DST}"

echo "triage_bqutils_fixtures.sh: running conformance runner on ${SRC}"
set +e
go run ./conformance/cmd/runner \
	--fixtures "${SRC}" \
	--engine-binary "${EMULATOR_BIN}" \
	--output json \
	--output-file "${TRIAGE_JSON}" \
	>"${TRIAGE_JSON}.stdout" 2>&1
runner_exit=$?
set -e

python3 - "${TRIAGE_JSON}" "${SRC}" "${DST}" "${DRY_RUN}" <<'PY'
import json
import os
import shutil
import sys

report_path, src_root, dst_root, dry_run = sys.argv[1:5]
dry = dry_run == "1"

with open(report_path, encoding="utf-8") as f:
    report = json.load(f)

# One row per fixture path; PASS on duckdb is enough to promote.
by_path: dict[str, str] = {}
for row in report.get("results", []):
    path = row.get("path") or ""
    status = row.get("status") or ""
    if not path:
        continue
    prev = by_path.get(path)
    if prev == "PASS":
        continue
    if status == "PASS":
        by_path[path] = "PASS"
    elif path not in by_path:
        by_path[path] = status

moved = 0
left = 0
for path, status in sorted(by_path.items()):
    if status != "PASS":
        left += 1
        continue
    rel = os.path.relpath(path, src_root)
    dest = os.path.join(dst_root, rel)
    if dry:
        print(f"  would move PASS {rel}")
        moved += 1
        continue
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    if os.path.exists(dest):
        os.remove(dest)
    shutil.move(path, dest)
    print(f"  moved PASS {rel}")
    moved += 1

summary = report.get("summary", {})
print()
print("triage_bqutils_fixtures.sh: done.")
print(f"  report:     {report_path}")
print(f"  runner:     total={summary.get('total', '?')} passed={summary.get('passed', '?')} failed={summary.get('failed', '?')}")
print(f"  promoted:   {moved} -> passing/")
print(f"  remaining:  {left} in known_failing/")
PY

echo "  runner exit: ${runner_exit} (non-zero expected when some fixtures fail)"
exit 0
