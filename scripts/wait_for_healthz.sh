#!/usr/bin/env bash
# Poll an HTTP /healthz endpoint until it returns 200, suppressing the
# cosmetic curl errors that appear during a fresh container's startup
# race. Replaces the inline `while :; do curl -fsS ... ; sleep 2; done`
# blocks that previously lived in taskfiles/{thirdparty,docker}.yml.
#
# Usage:
#   bash scripts/wait_for_healthz.sh <url> [deadline_seconds] [service_name]
#
# Defaults: deadline_seconds=120, service_name="<url>" basename.
#
# On success: prints one line on stdout, exits 0.
# On timeout: prints the failure plus `docker compose ps` and
#   `docker compose logs --tail=200 <service>` to stderr, exits 1.
#
# Why the silence matters:
#
# When `docker compose up -d` returns "Started", Docker's userland
# port forwarder (`docker-proxy`) is already bound to the host port
# and accepts incoming TCP handshakes. The container's process,
# however, may not have called `Listen()` on its side yet — the
# gateway in this repo explicitly waits for the engine subprocess's
# gRPC health to flip to SERVING (engineReadyTimeout = 30s in
# gateway/gateway.go) before opening its HTTP listener, so the
# invariant "the HTTP listener never accepts traffic before the
# engine is actually able to answer it" holds. During that startup
# window, curl's TCP handshake completes (with docker-proxy), the
# proxy then fails to forward inside the container, closes the
# curl-side socket, and curl reports
# "(56) Recv failure: Connection reset by peer". That error is
# misleading — it looks like a real failure but is exactly the
# transient state this poll loop is designed to absorb — so we
# silence per-poll output and only emit a diagnostic dump if the
# overall deadline fires.
set -eo pipefail

url="${1:?usage: wait_for_healthz.sh <url> [deadline_seconds] [service_name]}"
deadline_secs="${2:-120}"
service="${3:-}"

# Per-poll cap so a hung connection cannot stall the whole loop. 3s is
# generous (the gateway's /healthz handler returns in microseconds once
# the engine is SERVING) but short enough that the 2s sleep dominates
# the poll cadence on a happy path.
per_poll_timeout="${WAIT_FOR_HEALTHZ_POLL_TIMEOUT:-3}"
poll_interval="${WAIT_FOR_HEALTHZ_INTERVAL:-2}"

deadline=$(( $(date +%s) + deadline_secs ))
attempts=0
while :; do
	attempts=$((attempts + 1))
	# `-f` fail on >=400, `-s` silent (no progress meter), NO `-S` so
	# transient connect/recv errors do not pollute the operator's
	# terminal during the startup race. `--connect-timeout` bounds the
	# TCP handshake step; `--max-time` bounds the full request.
	if curl -fs -o /dev/null \
		--connect-timeout "${per_poll_timeout}" \
		--max-time "${per_poll_timeout}" \
		"$url" 2>/dev/null; then
		echo "wait_for_healthz: ${url} healthy after ${attempts} attempt(s)"
		exit 0
	fi
	if [ "$(date +%s)" -ge "$deadline" ]; then
		{
			echo "wait_for_healthz: ${url} did not return 200 within ${deadline_secs}s (${attempts} attempts)"
			# Replay the failing curl one final time with `-S` so the
			# operator sees the exact error that won't go away.
			echo "  final curl attempt (with -S):"
			curl -fsS -o /dev/null \
				--connect-timeout "${per_poll_timeout}" \
				--max-time "${per_poll_timeout}" \
				"$url" 2>&1 | sed 's/^/    /' || true
			if command -v docker >/dev/null 2>&1; then
				echo "  docker compose ps:"
				docker compose ps 2>&1 | sed 's/^/    /' || true
				if [ -n "$service" ]; then
					echo "  docker compose logs --tail=200 ${service}:"
					docker compose logs --tail=200 "$service" 2>&1 | sed 's/^/    /' || true
				else
					echo "  docker compose logs --tail=200:"
					docker compose logs --tail=200 2>&1 | sed 's/^/    /' || true
				fi
			fi
		} >&2
		exit 1
	fi
	sleep "${poll_interval}"
done
