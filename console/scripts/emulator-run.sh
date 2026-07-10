#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=emulator-common.sh
source "$root/scripts/emulator-common.sh"

emulator_root="${EMULATOR_ROOT:-$root/..}"
emulator_root="$(cd "$emulator_root" && pwd)"
seed_file="${SEED_FILE:-$root/data/data.yaml}"
http_port="${EMULATOR_HTTP_PORT:-9050}"
grpc_port="${EMULATOR_GRPC_PORT:-9060}"
project_id="${PROJECT_ID:-local-project}"
gateway="$emulator_root/bin/gateway_main"

if [ ! -x "$gateway" ]; then
  echo "missing $gateway — run: task emulator:build-all" >&2
  exit 1
fi

if [ -f "$seed_file" ]; then
  seed_file="$(cd "$(dirname "$seed_file")" && pwd)/$(basename "$seed_file")"
fi
if [ ! -f "$seed_file" ]; then
  echo "seed file not found: $seed_file" >&2
  exit 1
fi

cd "$emulator_root"
mapfile -t args < <(emulator_gateway_args "$gateway" ./bin/emulator_main "$http_port" "$grpc_port" "$project_id" "$seed_file")
exec "$gateway" "${args[@]}"
