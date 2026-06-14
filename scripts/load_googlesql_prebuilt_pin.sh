#!/usr/bin/env bash
# Populate GOOGLESQL_PREBUILT_URL / GOOGLESQL_PREBUILT_SHA256 when unset.
#
# Callers that build the runtime image (docker compose, task docker:build,
# task thirdparty:emulator-up) should source this before `docker compose build`
# so the engine-builder-bazel stage uses the published prebuilt artifact
# instead of cloning googlesql in source mode (which requires extra patches).
#
# Resolution order:
#   1. Caller env (both vars already set) — no-op
#   2. `.github/workflows/release.yml` env.RELEASE_GOOGLESQL_PREBUILT_* pins
#
# Usage:
#   source scripts/load_googlesql_prebuilt_pin.sh
#   # or: bash scripts/load_googlesql_prebuilt_pin.sh  (exports in subshell via eval)
set -euo pipefail

load_googlesql_prebuilt_pin() {
	if [ -n "${GOOGLESQL_PREBUILT_URL:-}" ] && [ -n "${GOOGLESQL_PREBUILT_SHA256:-}" ]; then
		return 0
	fi

	local root release_yml url sha
	root="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
	release_yml="${root}/.github/workflows/release.yml"
	if [ ! -f "$release_yml" ]; then
		return 0
	fi

	url="$(
		grep -E '^\s+RELEASE_GOOGLESQL_PREBUILT_URL:' "$release_yml" \
			| head -1 \
			| sed -E 's/.*: *"([^"]+)".*/\1/'
	)"
	sha="$(
		grep -E '^\s+RELEASE_GOOGLESQL_PREBUILT_SHA256:' "$release_yml" \
			| head -1 \
			| sed -E 's/.*: *"([^"]+)".*/\1/'
	)"

	if [ -z "$url" ] || [ -z "$sha" ]; then
		return 0
	fi

	export GOOGLESQL_PREBUILT_URL="$url"
	export GOOGLESQL_PREBUILT_SHA256="$sha"
	printf '[load_googlesql_prebuilt_pin] using release.yml pins (sha256=%s…)\n' \
		"${sha:0:12}" >&2
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
	load_googlesql_prebuilt_pin
	printf 'export GOOGLESQL_PREBUILT_URL=%q\n' "${GOOGLESQL_PREBUILT_URL:-}"
	printf 'export GOOGLESQL_PREBUILT_SHA256=%q\n' "${GOOGLESQL_PREBUILT_SHA256:-}"
fi
