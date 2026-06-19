#!/usr/bin/env bash
set -euo pipefail

EMULATOR_URL="${EMULATOR_URL:-http://127.0.0.1:8080}"
UI_URL="${UI_URL:-http://127.0.0.1:8080}"
MAX_ATTEMPTS="${MAX_ATTEMPTS:-60}"
SLEEP_SECS="${SLEEP_SECS:-2}"

wait_for() {
  local label="$1"
  local cmd="$2"
  local attempt=1
  while [ "$attempt" -le "$MAX_ATTEMPTS" ]; do
    if eval "$cmd"; then
      echo "ready: $label"
      return 0
    fi
    echo "waiting for $label ($attempt/$MAX_ATTEMPTS)..."
    attempt=$((attempt + 1))
    sleep "$SLEEP_SECS"
  done
  echo "timeout waiting for $label" >&2
  return 1
}

wait_for "ui" "curl -fsS -o /dev/null '$UI_URL/'"
wait_for "bigquery datasets via nginx" "curl -fsS '$UI_URL/bigquery/v2/projects/local-project/datasets' | grep -q test-dataset"
