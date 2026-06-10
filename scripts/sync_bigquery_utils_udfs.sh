#!/usr/bin/env bash
# Sync bigquery-utils UDF test cases into native conformance YAML fixtures.
#
# Clones (or reuses a local clone of) GoogleCloudPlatform/bigquery-utils,
# runs the Node extractor over udfs/community/ and udfs/migration/*/, pipes
# the JSON manifest into the Go generator, and writes fixtures under
# conformance/thirdparty-fixtures/bigquery_utils/known_failing/.
#
# Usage (from anywhere; the script resolves the repo root itself):
#
#     ./scripts/sync_bigquery_utils_udfs.sh
#     BIGQUERY_UTILS_LOCAL=/path/to/bigquery-utils ./scripts/sync_bigquery_utils_udfs.sh
#     BIGQUERY_UTILS_REF=v1.2.3 ./scripts/sync_bigquery_utils_udfs.sh
#     ./scripts/sync_bigquery_utils_udfs.sh --dry-run
#
# Environment overrides:
#   BIGQUERY_UTILS_REF    Branch, tag, or SHA (default: master).
#   BIGQUERY_UTILS_REPO   Upstream URL override.
#   BIGQUERY_UTILS_LOCAL  Reuse an existing clone instead of shallow-fetching.
#   BIGQUERY_UTILS_KEEP   Keep the temporary clone on exit (default: cleaned).
#
# Refresh contract: the generator wipes known_failing/ each run and rewrites
# every emitted fixture. passing/ is untouched (fixtures are promoted there
# manually after engine triage).

set -euo pipefail

REPO_URL="${BIGQUERY_UTILS_REPO:-https://github.com/GoogleCloudPlatform/bigquery-utils.git}"
REF="${BIGQUERY_UTILS_REF:-master}"
LOCAL="${BIGQUERY_UTILS_LOCAL:-}"

DRY_RUN=0
for arg in "$@"; do
	case "$arg" in
		--dry-run) DRY_RUN=1 ;;
		-h|--help)
			sed -n '2,/^set -euo pipefail$/p' "$0" | sed 's/^# \{0,1\}//' | head -n -2
			exit 0
			;;
		*)
			echo "sync_bigquery_utils_udfs.sh: unknown arg '${arg}' (try --help)" >&2
			exit 2
			;;
	esac
done

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXTRACTOR="${ROOT}/scripts/bigquery_utils/extract_test_cases.js"
GENERATOR_PKG="./conformance/cmd/genbqutils"
OUT_DIR="${ROOT}/conformance/thirdparty-fixtures/bigquery_utils/known_failing"
MANIFEST="${TMPDIR:-/tmp}/bqutils-manifest.$$.json"

for tool in node go; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		echo "sync_bigquery_utils_udfs.sh: '${tool}' not on PATH" >&2
		exit 1
	fi
done

if [ ! -f "${EXTRACTOR}" ]; then
	echo "sync_bigquery_utils_udfs.sh: extractor missing at ${EXTRACTOR}" >&2
	exit 1
fi

cleanup_manifest() {
	rm -f "${MANIFEST}"
}
trap cleanup_manifest EXIT INT TERM

SRC=""
KEEP="${BIGQUERY_UTILS_KEEP:-0}"
cleanup_clone() {
	if [ -n "${SRC}" ] && [ "${KEEP}" = "0" ] && [ -z "${LOCAL}" ]; then
		rm -rf "${SRC}"
	elif [ -n "${SRC}" ] && [ "${KEEP}" != "0" ] && [ -z "${LOCAL}" ]; then
		echo "sync_bigquery_utils_udfs.sh: keeping clone at ${SRC} (BIGQUERY_UTILS_KEEP set)" >&2
	fi
}
trap cleanup_clone EXIT INT TERM

if [ -n "${LOCAL}" ]; then
	if [ ! -d "${LOCAL}/udfs" ]; then
		echo "sync_bigquery_utils_udfs.sh: BIGQUERY_UTILS_LOCAL=${LOCAL} has no udfs/" >&2
		exit 1
	fi
	SRC="$(cd "${LOCAL}" && pwd)"
	echo "sync_bigquery_utils_udfs.sh: using local clone at ${SRC}"
	RESOLVED_SHA="$(git -C "${SRC}" rev-parse HEAD)"
	RESOLVED_DESC="$(git -C "${SRC}" describe --tags --always --dirty=+dirty 2>/dev/null || echo "${RESOLVED_SHA}")"
else
	for tool in git; do
		if ! command -v "$tool" >/dev/null 2>&1; then
			echo "sync_bigquery_utils_udfs.sh: '${tool}' not on PATH" >&2
			exit 1
		fi
	done
	TMP_BASE="${TMPDIR:-/tmp}"
	SRC="$(mktemp -d "${TMP_BASE}/bqutils-sync.XXXXXX")"
	echo "sync_bigquery_utils_udfs.sh: cloning ${REPO_URL} @ ${REF} into ${SRC}"
	(
		cd "${SRC}"
		git init --quiet
		git remote add origin "${REPO_URL}"
		if ! git fetch --depth=1 --quiet origin "${REF}" 2>/dev/null; then
			echo "  shallow fetch failed for ${REF}; retrying with full history" >&2
			git fetch --quiet origin
			git checkout --quiet "${REF}"
		else
			git checkout --quiet FETCH_HEAD
		fi
	)
	RESOLVED_SHA="$(git -C "${SRC}" rev-parse HEAD)"
	RESOLVED_DESC="$(git -C "${SRC}" describe --tags --always --dirty=+dirty 2>/dev/null || echo "${RESOLVED_SHA}")"
fi

echo "sync_bigquery_utils_udfs.sh: resolved ${REF:-local} -> ${RESOLVED_SHA} (${RESOLVED_DESC})"

if [ "${DRY_RUN}" -eq 1 ]; then
	echo "sync_bigquery_utils_udfs.sh: --dry-run requested; manifest only (no YAML written)"
	node "${EXTRACTOR}" --src "${SRC}" --sha "${RESOLVED_SHA}" --out "${MANIFEST}"
	EMITTED="$(node -e 'const m=require(process.argv[1]);console.log(m.emitted.length)' "${MANIFEST}")"
	SKIPPED="$(node -e 'const m=require(process.argv[1]);console.log(m.skipped.length)' "${MANIFEST}")"
	echo "  would emit ${EMITTED} fixtures, skip ${SKIPPED}"
	echo "  manifest preview: ${MANIFEST}"
	exit 0
fi

node "${EXTRACTOR}" --src "${SRC}" --sha "${RESOLVED_SHA}" --out "${MANIFEST}" 2>&1 | grep -v '^$' || true
EMITTED="$(node -e 'const m=require(process.argv[1]);console.log(m.emitted.length)' "${MANIFEST}")"
SKIPPED="$(node -e 'const m=require(process.argv[1]);console.log(m.skipped.length)' "${MANIFEST}")"

(
	cd "${ROOT}"
	go run "${GENERATOR_PKG}" --out-dir "${OUT_DIR}" < "${MANIFEST}"
)

echo
echo "sync_bigquery_utils_udfs.sh: done."
echo "  upstream:  ${REPO_URL:-${SRC}}"
echo "  ref:       ${REF:-local HEAD}"
echo "  resolved:  ${RESOLVED_SHA} (${RESOLVED_DESC})"
echo "  emitted:   ${EMITTED}"
echo "  skipped:   ${SKIPPED}"
echo "  fixtures:  ${OUT_DIR}"
echo
echo "  Skip reasons (unique):"
node -e '
const m = require(process.argv[1]);
const counts = {};
for (const s of m.skipped) {
  counts[s.reason] = (counts[s.reason] || 0) + 1;
}
for (const [reason, n] of Object.entries(counts).sort((a, b) => b[1] - a[1])) {
  console.log(`    ${n}\t${reason}`);
}
' "${MANIFEST}"
echo
echo "  Next: task conformance:bqutils  (non-gating; requires bin/emulator_main)"
