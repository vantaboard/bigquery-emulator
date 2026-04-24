#!/usr/bin/env bash
# Poll Go's /debug/pprof/heap on an interval while a long query runs (e.g. bigquery-emulator-duck
# with --pprof-addr=127.0.0.1:6060). Writes timestamped heap profiles for later: go tool pprof -inuse_space ...
#
# Usage:
#   PPROF=http://127.0.0.1:6060 INTERVAL=5 OUT=/tmp/hpoll ./scripts/pprof-heap-poll.sh
#   MAX_SAMPLES=20 ./scripts/pprof-heap-poll.sh   # stop after 20 samples
#
# Environment:
#   PPROF        Base URL (default http://127.0.0.1:6060)
#   INTERVAL     Seconds between samples (default 5)
#   OUT          Output directory (default ./pprof-heap-poll-YYYYMMDD-HHMMSS in cwd)
#   MAX_SAMPLES  If set, exit after this many successful downloads (optional)
set -euo pipefail

PPROF="${PPROF:-http://127.0.0.1:6060}"
INTERVAL="${INTERVAL:-5}"
# Strip trailing slash from PPROF
PPROF="${PPROF%/}"
OUT="${OUT:-./pprof-heap-poll-$(date +%Y%m%d-%H%M%S)}"
HEAP_URL="${PPROF}/debug/pprof/heap"

mkdir -p "$OUT"
echo "Heap poll: ${HEAP_URL} every ${INTERVAL}s -> ${OUT}"
if [[ -n "${MAX_SAMPLES:-}" ]]; then
  echo "MAX_SAMPLES=${MAX_SAMPLES} (will stop after that many good pulls)"
else
  echo "Ctrl-C to stop"
fi
echo "---"

i=0
ok=0
while true; do
  i=$((i + 1))
  ts=$(date +%Y%m%dT%H%M%S)
  f="${OUT}/heap-$(printf '%04d' "$i")-${ts}.pb.gz"
  if curl -sS -f --connect-timeout 2 -o "$f" "$HEAP_URL"; then
    ok=$((ok + 1))
    bytes=$(wc -c <"$f" | tr -d ' ')
    echo "$(date '+%Y-%m-%d %H:%M:%S')  [${ok}] ${f} (${bytes} B)"
  else
    echo "$(date '+%Y-%m-%d %H:%M:%S')  curl failed (${HEAP_URL}); retrying..." >&2
  fi
  if [[ -n "${MAX_SAMPLES:-}" && "$ok" -ge "${MAX_SAMPLES}" ]]; then
    echo "Reached MAX_SAMPLES=${MAX_SAMPLES}, done."
    break
  fi
  sleep "$INTERVAL"
done

echo "Wrote under: ${OUT}"
echo "Example:  go tool pprof -text -inuse_space -nodecount=40 <path/to/bigquery-emulator-duck> ${OUT}/heap-0001-*.pb.gz"
