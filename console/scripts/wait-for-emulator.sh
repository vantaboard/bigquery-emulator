#!/usr/bin/env bash
set -euo pipefail

EMULATOR_URL="${EMULATOR_URL:-http://127.0.0.1:9050}"
PROJECT_ID="${PROJECT_ID:-local-project}"
EXPECT_DATASET="${EXPECT_DATASET:-}"
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

wait_for "emulator health" "curl -fsS '$EMULATOR_URL/healthz' | grep -q ok"

if [ -n "$EXPECT_DATASET" ]; then
  wait_for "seed dataset $EXPECT_DATASET" \
    "curl -fsS '$EMULATOR_URL/bigquery/v2/projects/$PROJECT_ID/datasets' | grep -q '$EXPECT_DATASET'"
fi
