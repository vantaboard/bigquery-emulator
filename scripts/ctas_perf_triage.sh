#!/usr/bin/env bash
# CTAS performance triage: extract timing fields from an emulator log and print a one-line conclusion.
# See the "Next steps: CTAS still too slow" plan. Works with slog key=value (text) lines.
# Run the binary with --log-level info (or debug) so CTAS in-place and async query lines are present.
#
# If logs mix multiple jobs, set JOB_ID to the full value after job_id= (e.g. dataform-uuid) so
# this script only analyzes matching lines. Example:
#   JOB_ID=dataform-3ae91caf-bf99-4743-be49-d3daee2c5a6b ./scripts/ctas_perf_triage.sh tmp/air/emulator.log
# Or: grep "job_id=YOUR" emulator.log | ./scripts/ctas_perf_triage.sh
#
# Usage:
#   ./scripts/ctas_perf_triage.sh emulator.log
#   JOB_ID=dataform-... ./scripts/ctas_perf_triage.sh emulator.log
#   grep "job_id=abc" emulator.log | ./scripts/ctas_perf_triage.sh
#
set -euo pipefail

LOG="${1:--}"
if [ "$LOG" = "-" ] && [ -t 0 ]; then
  echo "Usage: $0 <logfile>  OR:  grep 'job_id=...' log | $0" >&2
  exit 2
fi

# Normalize log to a temp file, optionally filtered by JOB_ID=...
TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT
if [ "$LOG" = "-" ]; then
  if [ -n "${JOB_ID:-}" ]; then
    grep -F "job_id=${JOB_ID}" >"$TMP" || true
  else
    cat >"$TMP"
  fi
else
  if [ -n "${JOB_ID:-}" ]; then
    grep -F "job_id=${JOB_ID}" -- "$LOG" >"$TMP" || true
  else
    cat -- "$LOG" >"$TMP"
  fi
fi

pick() {
  # args: key (grep line in $LINE)
  local _k="$1"
  echo "$LINE" | grep -oE "${_k}=[^ ]+" 2>/dev/null | head -1 | cut -d= -f2- || true
}

LINE=$(grep "CTAS in-place phase timings" "$TMP" 2>/dev/null | tail -1 || true)
ex=$(pick "exec_ms")
sch=$(pick "schema_extract_ms")
rcm=$(pick "row_count_ms")
rcs=$(pick "row_count_source")
ra=$(pick "rows_affected")
dest=$(pick "destination")

LINE=$(grep "async query content query finished" "$TMP" 2>/dev/null | tail -1 || true)
cqfin=$(pick "elapsed_ms")

LINE=$(grep "async query catalog sync done" "$TMP" 2>/dev/null | tail -1 || true)
# breakdown lines may follow same job — take last
ms=$(pick "metadata_sync_ms")

LINE=$(grep "async query job completed" "$TMP" 2>/dev/null | grep "outcome" | tail -1 || true)
# Some logs use key=value; ok
jobe=$(pick "elapsed_ms")
cqj=$(pick "content_query_ms")
dwj=$(pick "destination_write_ms")

echo "=== CTAS perf capture (last lines in log) ==="
echo "in_place: destination=${dest:-?} exec_ms=${ex:-?} schema_extract_ms=${sch:-?} row_count_ms=${rcm:-?} row_count_source=${rcs:-?} rows_affected=${ra:-?}"
echo "content query finished (elapsed): ${cqfin:-?} ms"
echo "metadata_sync: ${ms:-?} ms"
echo "job completed: elapsed_ms=${jobe:-?} content_query_ms=${cqj:-?} destination_write_ms=${dwj:-?}"
echo ""

# Heuristic: pick largest numeric among exec, row_count, metadata_sync
best=""
bestv=-1
for nameval in "exec_ms:$ex" "row_count_ms:$rcm" "metadata_sync_ms:$ms" "content_query_ms:${cqj:-$cqfin}"; do
  n="${nameval%%:*}"
  v="${nameval#*:}"
  if [[ "$v" =~ ^[0-9]+$ ]]; then
    if [ "$v" -gt "$bestv" ]; then
      bestv=$v
      best=$n
    fi
  fi
done

echo "Largest among exec / row_count / metadata_sync / content_query (approx): ${best:-?} = ${bestv} ms"
case "${best}" in
  exec_ms)
    echo "Conclusion: engine-bound (Exec path dominates) — next: query rewrite, SQLite pragmas, pprof in go-googlesql-engine (Branch A)."
    ;;
  row_count_ms)
    echo "Conclusion: second scan for row count — check RowsAffected for CTAS; BQ_EMULATOR_CTAS_INPLACE_FORCE_COUNT, driver fix (Branch B)."
    ;;
  metadata_sync_ms)
    echo "Conclusion: post-commit catalog sync work — see detailed catalog sync log lines (Branch C)."
    ;;
  content_query_ms|*)
    echo "Conclusion: whole query phase dominates; cross-check exec_ms on CTAS in-place line; if wall clock differs, consider client/polling (Branch D)."
    ;;
esac
if [ "${rcs:-}" = "count_query" ] && [ "${ra:-0}" = "0" ] && [ "${ex:-0}" -gt 1000 ] 2>/dev/null; then
  echo "Note: large exec_ms with rows_affected=0 and count_query — verify whether driver should report changes() for this CTAS."
fi
