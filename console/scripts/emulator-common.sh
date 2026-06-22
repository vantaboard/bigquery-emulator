#!/usr/bin/env bash
# Shared helpers for starting a locally built bigquery-emulator gateway.

emulator_gateway_supports_sql_tools() {
  local gateway="$1"
  "$gateway" --help 2>&1 | grep -q -- 'enable-sql-tools-api'
}

emulator_append_sql_tools_flags() {
  local gateway="$1"
  local -n _out=$2
  if emulator_gateway_supports_sql_tools "$gateway"; then
    _out+=(--enable-sql-tools-api --sql-tools-api-allow-remote)
  else
    echo "note: gateway lacks SQL Tools API flags; query editor uses client-side fallbacks" >&2
  fi
}

emulator_gateway_args() {
  local gateway="$1"
  local engine_binary="$2"
  local http_port="$3"
  local grpc_port="$4"
  local project_id="$5"
  local seed_file="$6"
  local -a args=(
    --engine-binary="$engine_binary"
    --http-port="$http_port"
    --grpc-port="$grpc_port"
    --project-id="$project_id"
    --seed-data-file="$seed_file"
  )
  emulator_append_sql_tools_flags "$gateway" args
  args+=(--log-requests)
  printf '%s\n' "${args[@]}"
}
