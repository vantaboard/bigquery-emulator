#!/usr/bin/env bash
# Preflight for third_party/node-bigquery-tests Mocha against fake-gcs-server + BigQuery emulator.
# Fails fast when fake-gcs is down, or the *HTTP* object body for us-states.csv is empty/truncated.
#
# Important: this checks what fake-gcs-server returns over JSON API (?alt=media), not whether
# you have a valid file only on disk. After gsutil/cp into testdata/, recreate the container so
# the server reloads from the bind mount: docker compose up -d --force-recreate fake-gcs-server
set -euo pipefail

root="$(git rev-parse --show-toplevel 2>/dev/null || (cd "$(dirname "$0")/.." && pwd))"
fixture="${root}/testdata/fake-gcs-data/cloud-samples-data/bigquery/us-states/us-states.csv"

port="${FAKE_GCS_PORT:-4443}"
# Prefer STORAGE_EMULATOR_HOST so preflight hits the same host:port as client libraries.
if [[ -n "${STORAGE_EMULATOR_HOST:-}" ]]; then
	_h="${STORAGE_EMULATOR_HOST#http://}"
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
url="${base}/storage/v1/b/cloud-samples-data/o/bigquery%2Fus-states%2Fus-states.csv?alt=media"

# Minimum bytes for the real cloud-samples-data us-states fixture (header + 50 states).
# A tiny placeholder (e.g. tens of bytes) passes older checks but breaks external-table samples.
readonly MIN_US_STATES_CSV_BYTES="${MIN_US_STATES_CSV_BYTES:-400}"

tmp="$(mktemp)"
trap 'rm -f "$tmp"' EXIT
size="$(curl -fsS -o "$tmp" --connect-timeout 3 --max-time 30 -w '%{size_download}' "$url" 2>/dev/null || true)"
if [[ -z "$size" ]]; then
  echo "preflight: could not reach fake-gcs at ${base} (start: docker compose up -d fake-gcs-server)" >&2
  exit 1
fi
if [[ -z "$size" || "$size" == "0" ]]; then
  echo "preflight: ${url} returned empty body (${size:-0} bytes). Re-seed testdata (task testdata:fake-gcs-sync) or recreate the container:" >&2
  echo "  docker compose up -d --force-recreate fake-gcs-server" >&2
  exit 1
fi
if [[ "$size" -lt "$MIN_US_STATES_CSV_BYTES" ]]; then
	echo "preflight: ${url} body is only ${size} bytes (expected >= ${MIN_US_STATES_CSV_BYTES})." >&2
	if [[ -f "$fixture" ]]; then
		host_bytes="$(wc -c <"$fixture" | tr -d ' ')"
		echo "  On-disk fixture ${fixture} is ${host_bytes} bytes — so this is not a bad gsutil copy;" >&2
		echo "  fake-gcs is still serving a tiny body (stale in-memory data, wrong port, or empty mount)." >&2
	fi
	echo "  From repo root: docker compose up -d --force-recreate fake-gcs-server" >&2
	echo "  (and task testdata:fake-gcs-sync if testdata/fake-gcs-data is empty)" >&2
	exit 1
fi
if ! grep -q 'Washington,WA' "$tmp" 2>/dev/null; then
  echo "preflight: ${url} does not contain expected row Washington,WA (wrong object or truncated file)." >&2
  echo "  Re-seed: task testdata:fake-gcs-sync && docker compose up -d --force-recreate fake-gcs-server" >&2
  exit 1
fi

echo "preflight: fake-gcs us-states.csv ok (${size} bytes, Washington,WA present)"
