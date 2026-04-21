#!/usr/bin/env bash
# Used by air (.air.toml). Builds the emulator with the same tags/GOWORK rules as task emulator:build.
set -euo pipefail
_REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$_REPO_ROOT"

if [[ -z "${GOOGLESQL_BUILD_TAGS:-}" && -f .envrc ]]; then
	# shellcheck disable=SC1091
	source .envrc
fi

if [[ -f go.work ]]; then
	export GOWORK="$_REPO_ROOT/go.work"
else
	export GOWORK=off
fi

mkdir -p tmp/air
go build -tags "$GOOGLESQL_BUILD_TAGS" -o ./tmp/air/bigquery-emulator ./cmd/bigquery-emulator
