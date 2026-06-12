#!/usr/bin/env bash
# Poll until host:port accepts a TCP connection (gRPC readiness proxy).
#
# Usage:
#   bash scripts/wait_for_tcp.sh <host:port> [deadline_seconds]
#
# Defaults: host=localhost, deadline_seconds=120.
set -eo pipefail

endpoint="${1:?usage: wait_for_tcp.sh <host:port> [deadline_seconds]}"
deadline_secs="${2:-120}"
poll_interval="${WAIT_FOR_TCP_INTERVAL:-2}"

host="${endpoint%%:*}"
port="${endpoint##*:}"
if [ "$host" = "$port" ]; then
	host="localhost"
	port="$endpoint"
fi

deadline=$(( $(date +%s) + deadline_secs ))
attempts=0
while :; do
	attempts=$((attempts + 1))
	if (echo >"/dev/tcp/${host}/${port}") >/dev/null 2>&1; then
		echo "wait_for_tcp: ${host}:${port} accepting connections after ${attempts} attempt(s)"
		exit 0
	fi
	if [ "$(date +%s)" -ge "$deadline" ]; then
		echo "wait_for_tcp: ${host}:${port} not accepting connections within ${deadline_secs}s (${attempts} attempts)" >&2
		exit 1
	fi
	sleep "${poll_interval}"
done
