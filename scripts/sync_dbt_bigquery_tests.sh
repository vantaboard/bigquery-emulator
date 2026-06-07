#!/usr/bin/env bash
# Populate the scaffold at third_party/dbt-bigquery-tests/ with functional
# test sources from dbt-labs/dbt-adapters (dbt-bigquery/tests/).
#
# The vendored tree keeps curated emulator wiring (conftest.py,
# emulator_bootstrap.py, emulator_pytest_skip.py, profiles/, upstream_ref.txt)
# and syncs upstream *.py plus small non-Python fixtures on demand — same
# policy as scripts/sync_python_bigquery_tests.sh.
#
# Runtime packages (dbt-core, dbt-bigquery, dbt-tests-adapter) install from
# requirements-test.txt at the pinned SHA; only test tree files are rsync'd.
#
# Usage (from anywhere; the script resolves the repo root itself):
#
#     ./scripts/sync_dbt_bigquery_tests.sh                     # ref from upstream_ref.txt or main
#     DBT_ADAPTERS_REF=a82c766c ./scripts/sync_dbt_bigquery_tests.sh
#     DBT_ADAPTERS_LOCAL=/path/to/dbt-adapters ./scripts/sync_dbt_bigquery_tests.sh
#     ./scripts/sync_dbt_bigquery_tests.sh --dry-run           # preview only
#
# Environment overrides:
#   DBT_ADAPTERS_REF        Branch, tag, or SHA on dbt-labs/dbt-adapters
#                           (default: first line in third_party/dbt-bigquery-tests/upstream_ref.txt,
#                           else main).
#   DBT_ADAPTERS_REPO       Override the upstream URL (default:
#                           https://github.com/dbt-labs/dbt-adapters.git).
#   DBT_ADAPTERS_LOCAL      Use an existing clone instead of a temp fetch
#                           (must contain dbt-bigquery/tests/).
#   DBT_ADAPTERS_KEEP       Set to keep the temporary clone at $TMPDIR/...
#                           (default: cleaned on exit).
#
# Refresh contract: never overwrites curated scaffold files (conftest.py,
# emulator_*.py, profiles/, pyproject.toml, requirements-test.txt,
# upstream_ref.txt, FEASIBILITY.md, LICENSE).

set -euo pipefail

REPO_URL="${DBT_ADAPTERS_REPO:-https://github.com/dbt-labs/dbt-adapters.git}"

DRY_RUN=0
for arg in "$@"; do
	case "$arg" in
		--dry-run) DRY_RUN=1 ;;
		-h|--help)
			sed -n '2,/^set -euo pipefail$/p' "$0" | sed 's/^# \{0,1\}//' | head -n -2
			exit 0
			;;
		*)
			echo "sync_dbt_bigquery_tests.sh: unknown arg '${arg}' (try --help)" >&2
			exit 2
			;;
	esac
done

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DST="${ROOT}/third_party/dbt-bigquery-tests"
REF_FILE="${DST}/upstream_ref.txt"

if [ ! -f "${DST}/pyproject.toml" ]; then
	echo "sync_dbt_bigquery_tests.sh: scaffold missing at ${DST}" >&2
	echo "  Expected pyproject.toml under that path (see third_party/README.md)." >&2
	exit 1
fi

_default_ref() {
	if [ -f "${REF_FILE}" ]; then
		# First non-comment, non-blank line.
		awk '!/^#/ && NF { print $1; exit }' "${REF_FILE}"
	fi
}

REF="${DBT_ADAPTERS_REF:-$(_default_ref)}"
REF="${REF:-main}"

for tool in rsync; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		echo "sync_dbt_bigquery_tests.sh: '${tool}' not on PATH" >&2
		exit 1
	fi
done

# Non-Python fixtures the functional suite embeds in SQL/models strings.
FIXTURE_GLOBS=(
	"*.csv"
	"*.ndjson"
	"*.json"
	"*.sql"
	"*.yml"
	"*.yaml"
)

USE_LOCAL=0
SRC=""
if [ -n "${DBT_ADAPTERS_LOCAL:-}" ]; then
	if [ ! -d "${DBT_ADAPTERS_LOCAL}/dbt-bigquery/tests" ]; then
		echo "sync_dbt_bigquery_tests.sh: DBT_ADAPTERS_LOCAL missing dbt-bigquery/tests" >&2
		exit 1
	fi
	USE_LOCAL=1
	SRC="${DBT_ADAPTERS_LOCAL}"
	echo "sync_dbt_bigquery_tests.sh: using local clone ${SRC}"
else
	if ! command -v git >/dev/null 2>&1; then
		echo "sync_dbt_bigquery_tests.sh: 'git' not on PATH (or set DBT_ADAPTERS_LOCAL)" >&2
		exit 1
	fi
	TMP_BASE="${TMPDIR:-/tmp}"
	SRC="$(mktemp -d "${TMP_BASE}/dbt-adapters-sync.XXXXXX")"
	KEEP="${DBT_ADAPTERS_KEEP:-0}"
	cleanup() {
		if [ "${KEEP}" = "0" ] || [ "${KEEP}" = "" ]; then
			rm -rf "${SRC}"
		else
			echo "sync_dbt_bigquery_tests.sh: keeping clone at ${SRC} (DBT_ADAPTERS_KEEP set)" >&2
		fi
	}
	trap cleanup EXIT INT TERM

	echo "sync_dbt_bigquery_tests.sh: cloning ${REPO_URL} @ ${REF} into ${SRC}"
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
fi

if [ ! -d "${SRC}/dbt-bigquery/tests" ]; then
	echo "sync_dbt_bigquery_tests.sh: ${SRC}/dbt-bigquery/tests not found; upstream layout may have changed." >&2
	exit 1
fi

RESOLVED_SHA="$(git -C "${SRC}" rev-parse HEAD 2>/dev/null || echo unknown)"
RESOLVED_DESC="$(git -C "${SRC}" describe --tags --always --dirty=+dirty 2>/dev/null || echo "${RESOLVED_SHA}")"
echo "sync_dbt_bigquery_tests.sh: resolved ${REF} -> ${RESOLVED_SHA} (${RESOLVED_DESC})"

RSYNC_FLAGS=("-a" "--prune-empty-dirs")
if [ "${DRY_RUN}" -eq 1 ]; then
	RSYNC_FLAGS+=("--dry-run" "-v")
	echo "sync_dbt_bigquery_tests.sh: --dry-run requested; no files will be written"
fi

TESTS_SRC="${SRC}/dbt-bigquery/tests"
TESTS_DST="${DST}/tests"

echo "  rsync dbt-bigquery/tests/**/*.py -> ${TESTS_DST}/"
mkdir -p "${TESTS_DST}"
rsync "${RSYNC_FLAGS[@]}" \
	--exclude='conftest.py' \
	--include='*/' \
	--include='*.py' \
	--exclude='*' \
	"${TESTS_SRC}/" "${TESTS_DST}/"

for pat in "${FIXTURE_GLOBS[@]}"; do
	echo "  rsync dbt-bigquery/tests/**/${pat} -> ${TESTS_DST}/"
	rsync "${RSYNC_FLAGS[@]}" \
		--exclude='conftest.py' \
		--include='*/' \
		--include="${pat}" \
		--exclude='*' \
		"${TESTS_SRC}/" "${TESTS_DST}/"
done

# Sanity: functional entrypoint the task checks.
if [ "${DRY_RUN}" -eq 0 ] && [ ! -f "${TESTS_DST}/functional/adapter/test_basic.py" ]; then
	echo "sync_dbt_bigquery_tests.sh: tests/functional/adapter/test_basic.py was not produced." >&2
	echo "  Upstream layout may have changed." >&2
	exit 1
fi

if [ "${DRY_RUN}" -eq 0 ] && [ "${USE_LOCAL}" -eq 0 ]; then
	# Record resolved SHA for requirements-test.txt / upstream_ref.txt refresh.
	echo "  hint: bump third_party/dbt-bigquery-tests/upstream_ref.txt and"
	echo "        requirements-test.txt git URLs to ${RESOLVED_SHA} when pinning."
fi

py_count="$(find "${TESTS_DST}" -type f -name '*.py' 2>/dev/null | wc -l | tr -d ' ')"
echo
echo "sync_dbt_bigquery_tests.sh: done."
echo "  upstream:  ${REPO_URL}"
echo "  ref:       ${REF}"
echo "  resolved:  ${RESOLVED_SHA} (${RESOLVED_DESC})"
echo "  scaffold:  ${DST}"
echo "  tests/*.py: ${py_count}"
echo
echo "  Next: task thirdparty:dbt-bigquery-tests"
echo "        (feasibility only: DBT_BIGQUERY_RUN_TRIAGE=1 task thirdparty:dbt-bigquery-tests)"
