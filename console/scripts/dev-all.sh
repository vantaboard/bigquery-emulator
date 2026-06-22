#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
emulator_root="${EMULATOR_ROOT:-$root/../bigquery-emulator}"
emulator_root="$(cd "$emulator_root" && pwd)"
seed_file="${SEED_FILE:-$root/data/data.yaml}"
http_port="${EMULATOR_HTTP_PORT:-9050}"
grpc_port="${EMULATOR_GRPC_PORT:-9060}"
project_id="${PROJECT_ID:-local-project}"

if [ ! -x "$emulator_root/bin/gateway_main" ]; then
  echo "missing $emulator_root/bin/gateway_main — run: task emulator:build" >&2
  exit 1
fi

if [ ! -f "$seed_file" ]; then
  echo "missing seed file: $seed_file" >&2
  exit 1
fi

emulator_pid=""
cleanup() {
  if [ -n "$emulator_pid" ] && kill -0 "$emulator_pid" 2>/dev/null; then
    kill "$emulator_pid" 2>/dev/null || true
    wait "$emulator_pid" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

cd "$emulator_root"
./bin/gateway_main \
  --engine-binary=./bin/emulator_main \
  --http-port="$http_port" \
  --grpc-port="$grpc_port" \
  --project-id="$project_id" \
  --seed-data-file="$seed_file" \
  --enable-sql-tools-api \
  --sql-tools-api-allow-remote \
  --log-requests &
emulator_pid=$!

cd "$root"
EMULATOR_URL="http://127.0.0.1:$http_port" PROJECT_ID="$project_id" EXPECT_DATASET=test-dataset \
  bash "$root/scripts/wait-for-emulator.sh"

cd "$root"
exec npm run dev
