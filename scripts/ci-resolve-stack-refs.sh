#!/usr/bin/env bash
# Read pinned *root* stack versions from go.mod (require lines only) and emit Git tag refs for
# actions/checkout. Avoids matching github.com/vantaboard/go-googlesql/lib/... nested modules and
# ensures we never pass an empty ref (checkout would use the repo default branch, e.g. main).
set -euo pipefail
MOD="${1:?path to go.mod}"
if [[ -z "${GITHUB_OUTPUT:-}" ]]; then
	echo "error: GITHUB_OUTPUT must be set (run from GitHub Actions)" >&2
	exit 1
fi
gsql=$(awk '$1=="github.com/vantaboard/go-googlesql" && $2 ~ /^v[0-9]/ {print $2; exit}' "$MOD")
gsqlengine=$(awk '$1=="github.com/vantaboard/go-googlesql-engine" && $2 ~ /^v[0-9]/ {print $2; exit}' "$MOD")
if [[ -z "$gsql" || -z "$gsqlengine" ]]; then
	echo "error: require semver tags for github.com/vantaboard/go-googlesql and go-googlesql-engine in $MOD" >&2
	exit 1
fi
{
	echo "gsql=${gsql}"
	echo "gsqlengine=${gsqlengine}"
	echo "gsql_ref=refs/tags/${gsql}"
	echo "gsqlengine_ref=refs/tags/${gsqlengine}"
} >>"$GITHUB_OUTPUT"
echo "stack: go-googlesql@${gsql} go-googlesql-engine@${gsqlengine}"
