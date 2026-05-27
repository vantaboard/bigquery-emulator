#!/usr/bin/env bash
# Preflight for third_party/python-bigquery-dataframes-tests snippets when STORAGE_EMULATOR_HOST is set.
# Conftest creates GCS buckets via the Storage client; fail fast if fake-gcs is down.
set -euo pipefail

st="${STORAGE_EMULATOR_HOST:-}"
if [[ -z "${st// }" ]]; then
	echo "preflight: STORAGE_EMULATOR_HOST unset; skipping fake-gcs check."
	exit 0
fi

port="${FAKE_GCS_PORT:-4443}"
if [[ -n "$st" ]]; then
	_h="${st#http://}"
	_h="${_h#https://}"
	_h="${_h#//}"
	if [[ "$_h" == *:* ]]; then
		base="http://${_h}"
	else
		base="http://${_h}:${port}"
	fi
else
	base="http://127.0.0.1:${port}"
fi
url="${base}/storage/v1/b?maxResults=1"

if curl -fsS -o /dev/null --connect-timeout 3 --max-time 10 "$url" 2>/dev/null; then
	echo "preflight: fake-gcs reachable at ${base}"
	exit 0
fi

echo "preflight: STORAGE_EMULATOR_HOST=${st} but fake-gcs not reachable at ${url}" >&2
echo "  Start: docker compose up -d fake-gcs-server  (see repo .envrc)" >&2
exit 1
