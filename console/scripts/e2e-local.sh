#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=emulator-common.sh
source "$root/scripts/emulator-common.sh"

emulator_root="${EMULATOR_ROOT:-$root/../bigquery-emulator}"
emulator_root="$(cd "$emulator_root" && pwd)"
seed_file="${SEED_FILE:-$root/e2e/fixtures/seed.yaml}"
http_port="${EMULATOR_HTTP_PORT:-9050}"
grpc_port="${EMULATOR_GRPC_PORT:-9060}"
project_id="${PROJECT_ID:-local-project}"
vite_port="${VITE_DEV_PORT:-5173}"
gateway="$emulator_root/bin/gateway_main"

if [ ! -x "$gateway" ]; then
  echo "missing $gateway — run: task emulator:build" >&2
  exit 1
fi

if [ -f "$seed_file" ]; then
  seed_file="$(cd "$(dirname "$seed_file")" && pwd)/$(basename "$seed_file")"
fi

emulator_pid=""
vite_pid=""
cleanup() {
  if [ -n "$vite_pid" ] && kill -0 "$vite_pid" 2>/dev/null; then
    kill "$vite_pid" 2>/dev/null || true
    wait "$vite_pid" 2>/dev/null || true
  fi
  if [ -n "$emulator_pid" ] && kill -0 "$emulator_pid" 2>/dev/null; then
    kill "$emulator_pid" 2>/dev/null || true
    wait "$emulator_pid" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

cd "$emulator_root"
mapfile -t args < <(emulator_gateway_args "$gateway" ./bin/emulator_main "$http_port" "$grpc_port" "$project_id" "$seed_file")
"$gateway" "${args[@]}" &
emulator_pid=$!

cd "$root"
EMULATOR_URL="http://127.0.0.1:$http_port" PROJECT_ID="$project_id" EXPECT_DATASET=test-dataset \
  bash "$root/scripts/wait-for-emulator.sh"

cd "$root"
npm run dev &
vite_pid=$!

wait_for_vite() {
  local attempt=1
  while [ "$attempt" -le 60 ]; do
    if curl -fsS -o /dev/null "http://127.0.0.1:$vite_port/"; then
      echo "ready: vite dev server"
      return 0
    fi
    echo "waiting for vite ($attempt/60)..."
    attempt=$((attempt + 1))
    sleep 2
  done
  echo "timeout waiting for vite dev server" >&2
  return 1
}
wait_for_vite

cd "$root"
E2E_LOCAL=1 PLAYWRIGHT_BASE_URL="http://127.0.0.1:$vite_port" npx playwright test "$@"
