#!/usr/bin/env bash
# Snippets for sampling heap/goroutine while a long query runs against the duck emulator.
# Assumes pprof is bound, e.g. --pprof-addr=127.0.0.1:6060 or GOOGLESQL_ENGINE_PPROF_ADDR.
set -euo pipefail
PPROF="${PPROF:-http://127.0.0.1:6060}"
OUT="${OUT:-/tmp/duckdb-profile-$(date +%Y%m%d-%H%M%S)}"
mkdir -p "$OUT"
echo "Writing samples to $OUT (PPROF=$PPROF)"
curl -sS "$PPROF/debug/pprof/heap?debug=1" -o "$OUT/heap.txt"
curl -sS "$PPROF/debug/pprof/heap" -o "$OUT/heap.pb.gz" || true
curl -sS "$PPROF/debug/pprof/goroutine?debug=2" -o "$OUT/goroutine.txt"
curl -sS "$PPROF/debug/pprof/profile?seconds=5" -o "$OUT/cpu.pb.gz" || true
echo "Done. For text heap top-N: go tool pprof -text -nodecount=30 <binary> $OUT/heap.pb.gz"
