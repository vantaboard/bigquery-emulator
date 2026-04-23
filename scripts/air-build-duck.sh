#!/usr/bin/env bash
# Used by air (.air.duck.toml). Same flags as task emulator:build-duck; output under tmp/air/.
set -euo pipefail
_REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$_REPO_ROOT"

if [[ -z "${GOOGLESQL_BUILD_TAGS:-}" && -f .envrc ]]; then
	# shellcheck disable=SC1091
	source .envrc
fi

if [[ -z "${DUCKDB_LIB_DIR:-}" ]]; then
	echo "air-build-duck: set DUCKDB_LIB_DIR (e.g. in .envrc.local). Same as task emulator:build-duck." >&2
	exit 1
fi

if [[ -f go.work ]]; then
	export GOWORK="$_REPO_ROOT/go.work"
else
	export GOWORK=off
fi

export CGO_ENABLED=1
mkdir -p tmp/air

if [[ "$(uname -s)" == "Darwin" ]]; then
	DUCK_SHLIB="libduckdb.dylib"
	export CGO_LDFLAGS="${CGO_LDFLAGS:-} -lduckdb -L${DUCKDB_LIB_DIR} -Wl,-rpath,@executable_path"
	if [[ -n "${DYLD_LIBRARY_PATH:-}" ]]; then
		export DYLD_LIBRARY_PATH="${DUCKDB_LIB_DIR}:${DYLD_LIBRARY_PATH}"
	else
		export DYLD_LIBRARY_PATH="${DUCKDB_LIB_DIR}"
	fi
else
	DUCK_SHLIB="libduckdb.so"
	export CGO_LDFLAGS="${CGO_LDFLAGS:-} -lduckdb -L${DUCKDB_LIB_DIR} -Wl,-rpath,\$ORIGIN"
	if [[ -n "${LD_LIBRARY_PATH:-}" ]]; then
		export LD_LIBRARY_PATH="${DUCKDB_LIB_DIR}:${LD_LIBRARY_PATH}"
	else
		export LD_LIBRARY_PATH="${DUCKDB_LIB_DIR}"
	fi
fi

if [[ ! -f "${DUCKDB_LIB_DIR}/${DUCK_SHLIB}" ]]; then
	echo "air-build-duck: missing ${DUCKDB_LIB_DIR}/${DUCK_SHLIB}" >&2
	exit 1
fi

TAGS="${GOOGLESQL_BUILD_TAGS},duckdb,duckdb_use_lib"
go build -tags "${TAGS}" -o ./tmp/air/bigquery-emulator-duck ./cmd/bigquery-emulator
cp -f "${DUCKDB_LIB_DIR}/${DUCK_SHLIB}" ./tmp/air/
