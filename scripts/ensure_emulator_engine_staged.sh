#!/usr/bin/env bash
# Stage bin/emulator_main + bin/libduckdb.so on the host when engine
# sources are newer than the staged binaries. Mirrors the CI thirdparty
# lane (build-engine artifact → ENGINE_SOURCE=prebuilt Docker image).
#
# Usage: bash scripts/ensure_emulator_engine_staged.sh
set -eo pipefail

root="$(git rev-parse --show-toplevel)"
cd "$root"

ENGINE_BIN="${ENGINE_BIN:-bin/emulator_main}"
DUCKDB_BIN="${DUCKDB_BIN:-bin/libduckdb.so}"

log() {
	printf '[ensure_emulator_engine_staged] %s\n' "$*" >&2
}

# Paths that invalidate the staged engine binary.
ENGINE_SOURCE_PATHS=(
	"backend"
	"binaries/emulator_main"
	"proto"
	"frontend"
	"MODULE.bazel"
	".bazelrc"
	".bazelversion"
	"BUILD.bazel"
	"googlesql_deps.bzl"
)

needs_build=0
if [ "${ENGINE_REBUILD:-0}" = "1" ]; then
	needs_build=1
	log "forced rebuild (ENGINE_REBUILD=1)"
elif [ ! -f "$ENGINE_BIN" ] || [ ! -f "$DUCKDB_BIN" ]; then
	needs_build=1
	log "missing staged engine (${ENGINE_BIN} or ${DUCKDB_BIN})"
else
	existing_paths=()
	for p in "${ENGINE_SOURCE_PATHS[@]}"; do
		if [ -e "$p" ]; then
			existing_paths+=("$p")
		fi
	done
	newer="$(find "${existing_paths[@]}" -type f -newer "$ENGINE_BIN" \
		-not -path '*/.git/*' \
		-not -path '*/.cache/*' \
		-not -path '*/bazel-*/*' \
		-print -quit 2>/dev/null || true)"
	if [ -n "$newer" ]; then
		needs_build=1
		log "engine source '${newer}' is newer than ${ENGINE_BIN}"
	fi
fi

if [ "$needs_build" = "0" ]; then
	log "staged engine is up to date"
	exit 0
fi

log "building engine on host (task emulator:build-engine:bazel)"
task emulator:build-engine:bazel
