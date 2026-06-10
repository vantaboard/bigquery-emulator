#!/usr/bin/env bash
# Mirror public GCS sample objects into testdata/fake-gcs-data for fake-gcs-server
# (see docker-compose.yaml and third_party/README.md).
#
# Prefers gcloud (https://cloud.google.com/sdk/docs/install) when an active
# account is configured. Otherwise falls back to
# scripts/sync_fake_gcs_public_samples_http.py (stdlib JSON API + HTTPS).
# Set FAKE_GCS_SYNC_HTTP=1 to force the HTTP mirror. Public reads need no auth;
# if gcloud copy fails with credentials, try: gcloud auth application-default login
#
# Usage (from repo root):
#   ./scripts/sync_fake_gcs_public_samples.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="${ROOT}/testdata/fake-gcs-data"

_use_http_mirror() {
	if [ "${FAKE_GCS_SYNC_HTTP:-}" = "1" ]; then
		return 0
	fi
	if ! command -v gcloud >/dev/null 2>&1; then
		return 0
	fi
	if ! gcloud auth list --filter=status:ACTIVE --format='value(account)' 2>/dev/null | grep -q .; then
		return 0
	fi
	return 1
}

if _use_http_mirror; then
	echo "using Python HTTP mirror (no gcloud or no active gcloud account)" >&2
	exec python3 "${ROOT}/scripts/sync_fake_gcs_public_samples_http.py"
fi

echo "Syncing into ${DEST}"

mkdir -p "${DEST}/cloud-samples-data/bigquery/us-states"
mkdir -p "${DEST}/cloud-samples-data/bigquery/us-states/2021-07-02T16:04:48_70344/all_namespaces/kind_us-states"
FIRESTORE_DIR="${DEST}/cloud-samples-data/bigquery/us-states/2021-07-02T16:04:48_70344/all_namespaces/kind_us-states"
gcloud storage cp \
	"gs://cloud-samples-data/bigquery/us-states/2021-07-02T16:04:48_70344/all_namespaces/kind_us-states/output-0" \
	"${FIRESTORE_DIR}/" \
	2>/dev/null || true
# JSON entities fixture: the gateway parser reads inline entities from
# export_metadata (binary LevelDB metadata from GCS yields 0 rows).
cat >"${FIRESTORE_DIR}/all_namespaces_kind_us-states.export_metadata" <<'EOF'
{"entities":[{"properties":{"name":{"stringValue":"Alabama"},"post_abbr":{"stringValue":"AL"},"year":{"integerValue":"2020"}}},{"properties":{"name":{"stringValue":"Alaska"},"post_abbr":{"stringValue":"AK"},"year":{"integerValue":"2020"}}}]}
EOF
gcloud storage cp \
	"gs://cloud-samples-data/bigquery/us-states/us-states.csv" \
	"gs://cloud-samples-data/bigquery/us-states/us-states-by-date.csv" \
	"gs://cloud-samples-data/bigquery/us-states/us-states.json" \
	"gs://cloud-samples-data/bigquery/us-states/us-states.avro" \
	"gs://cloud-samples-data/bigquery/us-states/us-states.orc" \
	"gs://cloud-samples-data/bigquery/us-states/us-states.parquet" \
	"${DEST}/cloud-samples-data/bigquery/us-states/"

mkdir -p "${DEST}/cloud-samples-data/bigquery/sample-transactions"
gcloud storage cp \
	"gs://cloud-samples-data/bigquery/sample-transactions/transactions.csv" \
	"${DEST}/cloud-samples-data/bigquery/sample-transactions/"

mkdir -p "${DEST}/cloud-samples-data/bigquery"
gcloud storage cp --recursive \
	"gs://cloud-samples-data/bigquery/hive-partitioning-samples" \
	"${DEST}/cloud-samples-data/bigquery/"

mkdir -p "${DEST}/cloud-samples-data/vertex-ai/bigframe"
gcloud storage cp \
	"gs://cloud-samples-data/vertex-ai/bigframe/df.csv" \
	"${DEST}/cloud-samples-data/vertex-ai/bigframe/"

mkdir -p "${DEST}/cloud-samples-data/bigquery/ml/onnx"
gcloud storage cp \
	"gs://cloud-samples-data/bigquery/ml/onnx/pipeline_rf.onnx" \
	"${DEST}/cloud-samples-data/bigquery/ml/onnx/"

mkdir -p "${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/images"
gcloud storage cp --recursive \
	"gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/images/" \
	"${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/images/"

mkdir -p "${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/documents"
gcloud storage cp --recursive \
	"gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/documents/" \
	"${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/documents/"

mkdir -p "${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/document_chunks"
gcloud storage cp --recursive \
	"gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/document_chunks/" \
	"${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/document_chunks/"

mkdir -p "${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/tables/products"
gcloud storage cp \
	"gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/tables/products/products_*.avro" \
	"${DEST}/cloud-samples-data/bigquery/tutorials/cymbal-pets/tables/products/"

mkdir -p "${DEST}/cloud-training-demos/txtclass/export/exporter/1549825580"
gcloud storage cp --recursive \
	"gs://cloud-training-demos/txtclass/export/exporter/1549825580/" \
	"${DEST}/cloud-training-demos/txtclass/export/exporter/1549825580/"

mkdir -p "${DEST}/ibis-testing-libraries"
gcloud storage cp \
	"gs://ibis-testing-libraries/lodash.min.js" \
	"${DEST}/ibis-testing-libraries/"

echo "Done. Recreate fake-gcs to pick up changes: docker compose up -d --force-recreate fake-gcs-server"
