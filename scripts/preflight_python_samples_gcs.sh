#!/usr/bin/env bash
# Preflight for third_party/python-bigquery-tests nox snippets when
# STORAGE_EMULATOR_HOST is set (the python task always exports it; mirror
# of the node lane). Confirms fake-gcs-server is reachable at the
# normalized base URL so `storage.Client()` calls inside `test_extract_table`,
# `test_extract_table_json`, and `test_extract_table_compressed` route to
# fake-gcs instead of falling through to real `storage.googleapis.com`
# (which 403s on the bucket-list call when ADC resolves to the emulator's
# `dev` project).
#
# Unlike the node preflight, the python extract snippets create their own
# per-run buckets (e.g. `extract_shakespeare_<millis>`) and don't depend on
# a pre-seeded fixture, so this script only checks reachability + that the
# storage v1 surface answers — the same shape `storage.Client()` issues on
# first contact. Modeled on `preflight_dataframe_samples_gcs.sh`.
set -euo pipefail

st="${STORAGE_EMULATOR_HOST:-}"
if [[ -z "${st// }" ]]; then
	echo "preflight: STORAGE_EMULATOR_HOST unset; skipping fake-gcs check (extract snippets will use real GCS)."
	exit 0
fi

port="${FAKE_GCS_PORT:-4443}"
_h="${st#http://}"
_h="${_h#https://}"
_h="${_h#//}"
if [[ "$_h" == *:* ]]; then
	base="http://${_h}"
else
	base="http://${_h}:${port}"
fi

# `storage.Client.create_bucket()` first hits GET /storage/v1/b?project=...
# under the hood, which is exactly the call that 403s against real GCS in
# the failing log. fake-gcs-server returns 200 with a (possibly empty)
# bucket list. Any non-2xx here means the env var is wrong or fake-gcs is
# not actually serving on this port.
url="${base}/storage/v1/b?project=${GOOGLE_CLOUD_PROJECT:-dev}"

if curl -fsS -o /dev/null --connect-timeout 3 --max-time 10 "$url" 2>/dev/null; then
	echo "preflight: fake-gcs reachable at ${base}"
	exit 0
fi

echo "preflight: STORAGE_EMULATOR_HOST=${st} but fake-gcs not reachable at ${url}" >&2
echo "  Start: task thirdparty:fake-gcs-up" >&2
echo "  Or:    docker compose --profile thirdparty up -d fake-gcs-server" >&2
echo "  Verify the seed: ls testdata/fake-gcs-data/ (run 'task testdata:fake-gcs-sync' if empty)" >&2
exit 1
