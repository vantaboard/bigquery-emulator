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
err="$(mktemp)"
trap 'rm -f "$tmp" "$err" "$hive_tmp" "$hive_err"' EXIT

# curl -w '%{size_download}' writes "0" even when the connection is refused or
# DNS fails, so an empty $size never fires; capture the exit code separately
# (set +e around curl) and inspect it for transport-level failures (couldn't
# connect, DNS, timeout) before falling through to the body-length checks.
# This split lets the script tell "fake-gcs is down" apart from "fake-gcs is
# up but the object body is empty", which were both being reported as
# "returned empty body (0 bytes)" before.
set +e
size="$(curl -fsS -o "$tmp" --connect-timeout 3 --max-time 30 \
  -w '%{size_download}' "$url" 2>"$err")"
curl_rc=$?
set -e

# curl exit codes 6 (resolve host), 7 (connect), 28 (timeout) -- and the
# catch-all for "anything other than a successful HTTP response" -- mean the
# server isn't talking to us at all. Distinguish from "talking, but empty"
# by always preferring the transport diagnostic when curl failed AND
# nothing landed in $tmp; an HTTP error response (e.g. 404) still leaves
# size_download=0 but is a different failure class than no listener.
if [[ "$curl_rc" -ne 0 && ! -s "$tmp" ]]; then
  case "$curl_rc" in
    6|7|28)
      echo "preflight: could not reach fake-gcs at ${base} (curl rc=${curl_rc})." >&2
      echo "  Start it: docker compose --profile thirdparty up -d fake-gcs-server" >&2
      ;;
    *)
      echo "preflight: HTTP request to ${url} failed (curl rc=${curl_rc})." >&2
      if [[ -s "$err" ]]; then
        sed 's/^/  curl: /' "$err" >&2
      fi
      echo "  Verify fake-gcs is healthy: docker ps --filter name=fake-gcs" >&2
      ;;
  esac
  exit 1
fi
if [[ -z "$size" || "$size" == "0" ]]; then
  echo "preflight: ${url} returned empty body (${size:-0} bytes)." >&2
  echo "  fake-gcs is reachable but the object is missing or zero-length." >&2
  echo "  Re-seed testdata and recreate the container:" >&2
  echo "    task testdata:fake-gcs-sync" >&2
  echo "    docker compose --profile thirdparty up -d --force-recreate fake-gcs-server" >&2
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

# Hive-partitioning samples (autolayout parquet) used by external-table and load samples.
hive_fixture="${root}/testdata/fake-gcs-data/cloud-samples-data/bigquery/hive-partitioning-samples/autolayout/dt=2020-11-15/file1.parquet"
hive_url="${base}/storage/v1/b/cloud-samples-data/o/bigquery%2Fhive-partitioning-samples%2Fautolayout%2Fdt%3D2020-11-15%2Ffile1.parquet?alt=media"
readonly MIN_HIVE_PARQUET_BYTES="${MIN_HIVE_PARQUET_BYTES:-100}"

hive_tmp="$(mktemp)"
hive_err="$(mktemp)"

set +e
hive_size="$(curl -fsS -o "$hive_tmp" --connect-timeout 3 --max-time 30 \
  -w '%{size_download}' "$hive_url" 2>"$hive_err")"
hive_curl_rc=$?
set -e

if [[ "$hive_curl_rc" -ne 0 && ! -s "$hive_tmp" ]]; then
  echo "preflight: could not reach hive-partitioning sample at ${hive_url} (curl rc=${hive_curl_rc})." >&2
  echo "  Re-seed: task testdata:fake-gcs-sync && docker compose --profile thirdparty up -d --force-recreate fake-gcs-server" >&2
  exit 1
fi
if [[ -z "$hive_size" || "$hive_size" == "0" || "$hive_size" -lt "$MIN_HIVE_PARQUET_BYTES" ]]; then
  echo "preflight: ${hive_url} body is only ${hive_size:-0} bytes (expected >= ${MIN_HIVE_PARQUET_BYTES})." >&2
  if [[ -f "$hive_fixture" ]]; then
    host_bytes="$(wc -c <"$hive_fixture" | tr -d ' ')"
    echo "  On-disk fixture ${hive_fixture} is ${host_bytes} bytes — fake-gcs may be serving stale data." >&2
  fi
  echo "  Re-seed: task testdata:fake-gcs-sync && docker compose --profile thirdparty up -d --force-recreate fake-gcs-server" >&2
  exit 1
fi
if [[ "$(head -c 4 "$hive_tmp")" != $'PAR1' ]]; then
  echo "preflight: ${hive_url} does not look like parquet (missing PAR1 magic)." >&2
  exit 1
fi

echo "preflight: fake-gcs hive-partitioning autolayout parquet ok (${hive_size} bytes)"
