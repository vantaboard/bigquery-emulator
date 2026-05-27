#!/usr/bin/env bash
# Populate the scaffold at third_party/python-bigquery-tests/ with `*.py`
# source from the upstream googleapis/python-bigquery repo.
#
# The vendored tree mirrors upstream's directory layout (LICENSE,
# pyproject.toml, setup.cfg, uv.lock, samples/, samples/snippets/,
# samples/tests/, tests/data/, docs/) but intentionally omits *.py source
# (see third_party/README.md, "python-bigquery-tests" section). This
# script clones the upstream repo at a configurable ref and rsyncs the
# missing *.py files into place so `task thirdparty:python-bigquery-tests`
# can run nox.
#
# Usage (from anywhere; the script resolves the repo root itself):
#
#     ./scripts/sync_python_bigquery_tests.sh                     # ref=main
#     PYTHON_BIGQUERY_REF=v3.40.1 \
#         ./scripts/sync_python_bigquery_tests.sh                 # pin to tag
#     PYTHON_BIGQUERY_REF=e8184fa3856 \
#         ./scripts/sync_python_bigquery_tests.sh                 # pin to SHA
#     ./scripts/sync_python_bigquery_tests.sh --dry-run           # preview only
#
# Environment overrides:
#   PYTHON_BIGQUERY_REF    Branch, tag, or SHA on googleapis/python-bigquery
#                          (default: main).
#   PYTHON_BIGQUERY_REPO   Override the upstream URL (default:
#                          https://github.com/googleapis/python-bigquery.git).
#   PYTHON_BIGQUERY_KEEP   Set to keep the temporary clone at $TMPDIR/...
#                          (default: cleaned on exit).
#
# Refresh contract: the script only ADDS / UPDATES *.py files under the
# directories listed in `SYNC_PATHS` below. It never overwrites the
# scaffold's curated non-.py files (LICENSE, pyproject.toml, setup.cfg,
# uv.lock, sample JSON schemas, tests/data/* fixtures). That keeps refreshes
# diff-bounded and matches the go-googlesql ignore policy referenced in
# third_party/README.md.

set -euo pipefail

REPO_URL="${PYTHON_BIGQUERY_REPO:-https://github.com/googleapis/python-bigquery.git}"
REF="${PYTHON_BIGQUERY_REF:-main}"

DRY_RUN=0
for arg in "$@"; do
	case "$arg" in
		--dry-run) DRY_RUN=1 ;;
		-h|--help)
			sed -n '2,/^set -euo pipefail$/p' "$0" | sed 's/^# \{0,1\}//' | head -n -2
			exit 0
			;;
		*)
			echo "sync_python_bigquery_tests.sh: unknown arg '${arg}' (try --help)" >&2
			exit 2
			;;
	esac
done

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DST="${ROOT}/third_party/python-bigquery-tests"

if [ ! -f "${DST}/pyproject.toml" ]; then
	echo "sync_python_bigquery_tests.sh: scaffold missing at ${DST}" >&2
	echo "  Expected pyproject.toml under that path. Re-vendor the scaffold first" >&2
	echo "  (see third_party/README.md, python-bigquery-tests section)." >&2
	exit 1
fi

for tool in git rsync; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		echo "sync_python_bigquery_tests.sh: '${tool}' not on PATH" >&2
		exit 1
	fi
done

# Sub-trees we actually want *.py from. Each entry is a path RELATIVE to
# the upstream repo root (and to ${DST}). The rsync block below uses
# include/exclude so only *.py inside these subtrees lands in the scaffold.
# Top-level files (noxfile.py, noxfile_config.py) are handled separately so
# we don't accidentally pick up upstream's setup.py / conftest.py at root.
SYNC_PATHS=(
	"google"
	"samples"
	"tests"
	"docs"
)

# Extra (non-*.py) trees the upstream noxfile.py glob/reads at runtime.
# Each entry below pairs an upstream path with a per-tree include glob:
#   "<path>:<include>"
# The include glob is passed directly as rsync --include='<glob>'. We
# intentionally keep this allowlist tiny so the diff stays bounded:
#   testing/constraints-*.txt  -> per-Python pip pin files referenced by
#                                 every nox session (unit/snippets/system/
#                                 cover/prerelease_deps).
#   samples/*/requirements*.txt -> the `mypy_samples` session iterates
#                                  CURRENT_DIRECTORY.glob("samples/*/requirements.txt").
#                                  We grab requirements*.txt (covers
#                                  requirements.txt + requirements-test.txt).
SYNC_EXTRA=(
	"testing:constraints-*.txt"
	"samples:*/requirements*.txt"
)

# Top-level *.py allowlist (do not rsync upstream's setup.py — our scaffold
# uses pyproject.toml + setup.cfg only).
TOP_PY_FILES=(
	"noxfile.py"
	"noxfile_config.py"
	"conftest.py"
)

TMP_BASE="${TMPDIR:-/tmp}"
SRC="$(mktemp -d "${TMP_BASE}/python-bigquery-sync.XXXXXX")"
KEEP="${PYTHON_BIGQUERY_KEEP:-0}"
cleanup() {
	if [ "${KEEP}" = "0" ] || [ "${KEEP}" = "" ]; then
		rm -rf "${SRC}"
	else
		echo "sync_python_bigquery_tests.sh: keeping clone at ${SRC} (PYTHON_BIGQUERY_KEEP set)" >&2
	fi
}
trap cleanup EXIT INT TERM

echo "sync_python_bigquery_tests.sh: cloning ${REPO_URL} @ ${REF} into ${SRC}"
(
	cd "${SRC}"
	git init --quiet
	git remote add origin "${REPO_URL}"
	# Try shallow fetch first (works for branches, tags, and most SHAs since
	# GitHub allows reachable-SHA1 uploadpack). Fall back to a full fetch if
	# the server refuses the shallow request (rare; private mirrors).
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
echo "sync_python_bigquery_tests.sh: resolved ${REF} -> ${RESOLVED_SHA} (${RESOLVED_DESC})"

RSYNC_FLAGS=("-a" "--prune-empty-dirs")
if [ "${DRY_RUN}" -eq 1 ]; then
	RSYNC_FLAGS+=("--dry-run" "-v")
	echo "sync_python_bigquery_tests.sh: --dry-run requested; no files will be written"
fi

# Per-subtree rsync. Include directories so rsync recurses; include only
# *.py at file level; exclude everything else. --prune-empty-dirs keeps the
# diff small when an upstream subdir has no .py descendants.
for rel in "${SYNC_PATHS[@]}"; do
	if [ ! -d "${SRC}/${rel}" ]; then
		echo "  skipping ${rel}/ (not present upstream)" >&2
		continue
	fi
	echo "  rsync ${rel}/**/*.py -> ${DST}/${rel}/"
	mkdir -p "${DST}/${rel}"
	rsync "${RSYNC_FLAGS[@]}" \
		--include='*/' \
		--include='*.py' \
		--exclude='*' \
		"${SRC}/${rel}/" "${DST}/${rel}/"
done

# Extra non-*.py allowlists (per SYNC_EXTRA). We do these as separate
# rsyncs so each pair gets its own include glob without leaking patterns
# across subtrees. The include glob is anchored relative to the source
# subdirectory, so e.g. "samples:*/requirements*.txt" means
# samples/*/requirements*.txt (one level deep) — exactly what
# mypy_samples globs for.
for entry in "${SYNC_EXTRA[@]}"; do
	rel="${entry%%:*}"
	pat="${entry#*:}"
	if [ ! -d "${SRC}/${rel}" ]; then
		echo "  skipping ${rel}/${pat} (not present upstream)" >&2
		continue
	fi
	echo "  rsync ${rel}/${pat} -> ${DST}/${rel}/"
	mkdir -p "${DST}/${rel}"
	rsync "${RSYNC_FLAGS[@]}" \
		--include='*/' \
		--include="${pat}" \
		--exclude='*' \
		"${SRC}/${rel}/" "${DST}/${rel}/"
done

# Top-level allowlist (one file at a time so a missing upstream file is
# silently skipped without aborting the whole sync).
for f in "${TOP_PY_FILES[@]}"; do
	if [ -f "${SRC}/${f}" ]; then
		echo "  copy ${f} -> ${DST}/${f}"
		if [ "${DRY_RUN}" -eq 0 ]; then
			install -m 644 "${SRC}/${f}" "${DST}/${f}"
		fi
	fi
done

# Sanity: noxfile.py is the precondition the task checks.
if [ "${DRY_RUN}" -eq 0 ] && [ ! -f "${DST}/noxfile.py" ]; then
	echo "sync_python_bigquery_tests.sh: noxfile.py was not produced; upstream layout may have changed." >&2
	exit 1
fi

py_count="$(find "${DST}" -type f -name '*.py' 2>/dev/null | wc -l | tr -d ' ')"
echo
echo "sync_python_bigquery_tests.sh: done."
echo "  upstream:  ${REPO_URL}"
echo "  ref:       ${REF}"
echo "  resolved:  ${RESOLVED_SHA} (${RESOLVED_DESC})"
echo "  scaffold:  ${DST}"
echo "  *.py now:  ${py_count}"
echo
echo "  Next: task thirdparty:python-bigquery-tests"
echo "        (override session via PYTHON_SAMPLES_NOX_SESSION; see taskfiles/thirdparty.yml)"
