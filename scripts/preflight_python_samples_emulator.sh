#!/usr/bin/env bash
# Preflight for third_party/python-bigquery-tests nox snippets/system/prerelease_deps.
# When BIGQUERY_EMULATOR_HOST is set (go-googlesql .envrc), fail fast if nothing
# listens — otherwise pytest can hang for minutes per test on connection retries.
set -euo pipefail

host="${BIGQUERY_EMULATOR_HOST:-}"
if [[ -z "${host// }" ]]; then
	echo "preflight: BIGQUERY_EMULATOR_HOST unset; skipping emulator check (snippets may use real GCP)."
	exit 0
fi

_h="${host#http://}"
_h="${_h#https://}"
_h="${_h#//}"
base="http://${_h}"
url="${base}/bigquery/v2/projects"

if curl -fsS -o /dev/null --connect-timeout 2 --max-time 5 "$url" 2>/dev/null; then
	echo "preflight: BigQuery emulator reachable at ${base}"
	exit 0
fi

echo "preflight: BIGQUERY_EMULATOR_HOST=${host} but emulator not reachable at ${url}" >&2
echo "  Start: task emulator:start  (see repo .envrc)" >&2
echo "  Quick check: curl -fsS ${url}" >&2
echo "  For production BigQuery instead, run from an environment without BIGQUERY_EMULATOR_HOST (needs ADC)." >&2
exit 1
