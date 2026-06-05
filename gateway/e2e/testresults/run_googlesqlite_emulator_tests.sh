#!/usr/bin/env bash
# Run the ported googlesqlite query tests against emulator_main and save logs.
set -euo pipefail

E2E_DIR="$(cd "$(dirname "$0")/.." && pwd)"
REPO_ROOT="$(cd "${E2E_DIR}/../.." && pwd)"
cd "${REPO_ROOT}"

LOG_DIR="${E2E_DIR}/testresults"
mkdir -p "$LOG_DIR"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
LOG="${LOG_DIR}/googlesqlite-emulator-${STAMP}.log"
JSON="${LOG_DIR}/googlesqlite-emulator-${STAMP}.json"
SUMMARY="${LOG_DIR}/googlesqlite-emulator-${STAMP}-summary.txt"

RUN="$(awk '/^func Test/{sub(/\(.*/,"",$2); print $2}' "${E2E_DIR}/googlesqlite_query_test.go" | paste -sd'|' -)"

: "${BIGQUERY_EMULATOR_BIN:=${REPO_ROOT}/bin/emulator_main}"
export BIGQUERY_EMULATOR_BIN

{
  echo "# googlesqlite emulator port test run"
  echo "# started: $(date -u -Iseconds)"
  echo "# emulator: ${BIGQUERY_EMULATOR_BIN}"
  echo "# -run: ^(${RUN})$"
  echo
} | tee "$LOG"

# Write JSON to disk first. A live go test | tee | python | tee chain can deadlock
# when stdout backpressure fills the pipe (go test blocks after tests finish).
echo "Running go test (writing ${JSON})..." >&2
set +e
go test -tags=integration ./gateway/e2e/ \
  -run "^(${RUN})$" \
  -count=1 \
  -parallel 1 \
  -timeout 120m \
  -json >"$JSON" 2>&1
EXIT=$?
set -e

echo "Formatting human log from JSON..." >&2
python3 - "$JSON" >>"$LOG" <<'PY'
import json, sys

path = sys.argv[1]
for raw in open(path):
    line = raw.strip()
    if not line:
        continue
    try:
        ev = json.loads(line)
    except json.JSONDecodeError:
        print(line)
        continue
    a = ev.get("Action", "")
    pkg = ev.get("Package", "")
    test = ev.get("Test", "")
    out = ev.get("Output", "")
    if a == "run" and test:
        print(f"=== RUN {test}")
    elif a == "output" and out:
        sys.stdout.write(out)
    elif a in ("pass", "fail", "skip") and test:
        print(f"--- {a.upper()}: {test} ({ev.get('Elapsed', 0):.2f}s)")
    elif a == "fail" and not test:
        print(f"FAIL\t{pkg}")
    elif a == "pass" and not test:
        print(f"PASS\t{pkg}")
PY

python3 - "$JSON" "$SUMMARY" <<'PY'
import json, sys
from collections import Counter

path, out = sys.argv[1], sys.argv[2]
c = Counter()
failed = []
skipped = []
for line in open(path):
    line = line.strip()
    if not line:
        continue
    try:
        ev = json.loads(line)
    except json.JSONDecodeError:
        continue
    a = ev.get("Action")
    if a in ("pass", "fail", "skip"):
        c[a] += 1
    if a == "fail" and ev.get("Test"):
        failed.append(ev["Test"])
    if a == "skip" and ev.get("Test"):
        skipped.append(ev["Test"])
with open(out, "w") as f:
    f.write(f"pass={c['pass']} fail={c['fail']} skip={c['skip']}\n")
    if failed:
        f.write("\nFailed tests:\n")
        for t in failed:
            f.write(f"  {t}\n")
    if skipped:
        f.write("\nSkipped tests:\n")
        for t in skipped:
            f.write(f"  {t}\n")
print(open(out).read(), end="")
PY

{
  echo
  echo "# finished: $(date -u -Iseconds)"
  echo "# exit: ${EXIT}"
  echo "# log: ${LOG}"
  echo "# json: ${JSON}"
  echo "# summary: ${SUMMARY}"
} | tee -a "$LOG"

ln -sfn "$(basename "$LOG")" "${LOG_DIR}/googlesqlite-emulator-latest.log"
ln -sfn "$(basename "$JSON")" "${LOG_DIR}/googlesqlite-emulator-latest.json"
ln -sfn "$(basename "$SUMMARY")" "${LOG_DIR}/googlesqlite-emulator-latest-summary.txt"

exit "$EXIT"
