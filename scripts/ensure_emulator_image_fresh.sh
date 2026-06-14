#!/usr/bin/env bash
# Make sure the bigquery-emulator:local image actually reflects the
# *current* source tree before a third-party suite spins it up. This is
# the guard that prevents the failure mode logged in
# `logs/task-thirdparty-20260528-063518.log`, where every Java sample
# tripped `invalid character '\x1f' looking for beginning of value` on
# dataset creation — the gateway middleware (`gateway/middleware/gunzip.go`)
# decompresses the gzipped JSON body the Java client sends by default,
# but the running container had been built from a tree that predated
# that fix, so the suite was effectively testing an old artifact.
#
# Behavior:
#
#   * If `bigquery-emulator:local` is missing, this script builds it
#     (via `docker compose build`) and exits.
#   * If the image exists and `THIRDPARTY_SKIP_BUILD=1` is set, exit 0
#     immediately (CI mode: the image was built in a separate stage).
#   * If the image exists and any source file that materially affects
#     the image is *newer* than the image's `.Created` timestamp,
#     rebuild it.
#   * Otherwise (image exists and is at least as new as every tracked
#     source path), exit 0 without invoking docker.
#
# `THIRDPARTY_REBUILD=1` forces a rebuild unconditionally; useful when
# the developer wants to flush a stale layer (e.g. after switching
# GoogleSQL prebuilt pins). `THIRDPARTY_FORCE_PULL=1` adds `--pull` so
# the base layers (`debian:bookworm-slim`, `ubuntu:24.04`) are
# re-resolved; without it docker reuses any locally-cached parent
# image. Both flags are passed through to `docker compose build` as
# appropriate.
#
# We do NOT just rebuild on every invocation. `docker compose build`
# with a warm BuildKit cache is fast for the Go gateway layer (~10s)
# but the engine layer still has to walk the cache mount and re-stat
# the world, which trips Docker daemon contention if the user has
# unrelated docker workloads running. The staleness check below is
# `find ... -newer ${stamp}` over a small allowlist of paths, which
# completes in <100ms on a warm filesystem cache.
#
# Usage:
#
#   bash scripts/ensure_emulator_image_fresh.sh
#
# Honors the same caller env the task wrappers use (BAZEL_JOBS /
# BAZEL_MEM_MB / GOOGLESQL_PREBUILT_*); these are read straight from
# the calling environment by docker-compose.yml's `build:` block.

set -eo pipefail

image="${BIGQUERY_EMULATOR_IMAGE:-bigquery-emulator:local}"
root="$(git rev-parse --show-toplevel)"
cd "$root"

# Paths whose mtime invalidates the runtime image. We list directories
# (so a touched file inside them counts) plus a few top-level files
# that drive the build context. Keep this list small: every entry adds
# a `find` recursion, and unrelated paths like `docs/` should NEVER
# bust the cache.
SOURCE_PATHS=(
	"gateway"
	"binaries"
	"backend"
	"proto"
	"frontend"
	"docker"
	"Dockerfile"
	"docker-compose.yml"
	"MODULE.bazel"
	"MODULE.bazel.lock"
	".bazelrc"
	".bazelversion"
	"BUILD.bazel"
	"googlesql_deps.bzl"
	"go.mod"
	"go.sum"
)

log() {
	printf '[ensure_emulator_image_fresh] %s\n' "$*" >&2
}

build_image() {
	local why="$1"
	shift
	log "rebuilding ${image}: ${why}"
	local args=()
	if [ "${THIRDPARTY_FORCE_PULL:-0}" = "1" ]; then
		args+=(--pull)
	fi
	engine_source="${ENGINE_SOURCE:-prebuilt}"
	if [ "$engine_source" = "prebuilt" ]; then
		bash "${root}/scripts/ensure_emulator_engine_staged.sh"
	fi
	eval "$(bash "${root}/scripts/load_googlesql_prebuilt_pin.sh")"
	export ENGINE_SOURCE="$engine_source"
	log "ENGINE_SOURCE=${ENGINE_SOURCE}"
	# `docker compose build` honors the build args declared in
	# docker-compose.yml; the caller's BAZEL_JOBS / BAZEL_MEM_MB /
	# GOOGLESQL_PREBUILT_URL / GOOGLESQL_PREBUILT_SHA256 are picked
	# up through that path so we don't need to forward them here.
	docker compose build "${args[@]}" bigquery-emulator >&2
}

if [ "${THIRDPARTY_REBUILD:-0}" = "1" ]; then
	build_image "THIRDPARTY_REBUILD=1 forced"
	exit 0
fi

if ! image_created="$(docker image inspect --format '{{.Created}}' "$image" 2>/dev/null)"; then
	build_image "image ${image} missing"
	exit 0
fi

if [ "${THIRDPARTY_SKIP_BUILD:-0}" = "1" ]; then
	log "image ${image} exists; THIRDPARTY_SKIP_BUILD=1 — skipping staleness check"
	exit 0
fi

# Convert the image timestamp into seconds-since-epoch. Docker prints
# RFC 3339 with nanoseconds (e.g. 2026-05-28T13:35:01.123456789Z).
# GNU date `-d` parses that directly; BSD `date` needs the timezone
# stripped. Try GNU first, fall back to a Python parse so the script
# stays portable across the CI lanes (Ubuntu runners) and developer
# macs.
if image_epoch="$(date -d "$image_created" +%s 2>/dev/null)"; then
	:
elif image_epoch="$(python3 - "$image_created" <<'PY' 2>/dev/null
import sys, datetime
ts = sys.argv[1]
# Strip trailing Z, replace with +00:00 so fromisoformat accepts it.
if ts.endswith("Z"):
	ts = ts[:-1] + "+00:00"
# Drop sub-second precision past microseconds (datetime.fromisoformat
# refuses nanoseconds on Pythons older than 3.11).
if "." in ts:
	head, tail = ts.split(".", 1)
	# Tail looks like "123456789+00:00" or "123456789Z" — split on the
	# first non-digit, then re-attach the timezone.
	digits = ""
	for ch in tail:
		if ch.isdigit():
			digits += ch
		else:
			tz = tail[len(digits):]
			break
	else:
		tz = ""
	# Truncate to microseconds; pad if shorter.
	digits = (digits[:6]).ljust(6, "0") if digits else "0"
	ts = f"{head}.{digits}{tz}"
print(int(datetime.datetime.fromisoformat(ts).timestamp()))
PY
)"; then
	:
else
	log "cannot parse image creation timestamp '${image_created}'; rebuilding to be safe"
	build_image "image timestamp unparseable"
	exit 0
fi

# Build a small stamp file at the image_epoch mtime so we can use a
# single `find ... -newer ${stamp}` instead of issuing one find per
# entry. This is also robust against clock skew between the docker
# daemon and the host filesystem — if the stamp ends up "in the
# future" relative to the host, `find -newer` simply matches nothing
# and we exit clean.
stamp="$(mktemp)"
trap 'rm -f "$stamp"' EXIT
# touch -d accepts the same format date(1) does on GNU; fall back to
# `-t` for BSD if needed. We already have a numeric epoch, so use the
# strict-form `@<epoch>` syntax (GNU) and a CCYYMMDDhhmm.SS conversion
# as the portable fallback.
if ! touch -d "@${image_epoch}" "$stamp" 2>/dev/null; then
	# BSD `touch -t [[CC]YY]MMDDhhmm[.SS]`
	stamp_t="$(date -r "$image_epoch" +%Y%m%d%H%M.%S 2>/dev/null || true)"
	if [ -n "$stamp_t" ]; then
		touch -t "$stamp_t" "$stamp"
	else
		log "cannot stamp image_epoch=${image_epoch}; rebuilding to be safe"
		build_image "stamp creation failed"
		exit 0
	fi
fi

# Collect existing paths only; missing directories are harmless (e.g.
# `frontend` may be absent in some checkouts). `find -newer` returns
# the first match; we short-circuit with `-print -quit` so big trees
# don't dominate runtime.
existing_paths=()
for p in "${SOURCE_PATHS[@]}"; do
	if [ -e "$p" ]; then
		existing_paths+=("$p")
	fi
done

if [ "${#existing_paths[@]}" -eq 0 ]; then
	log "no source paths found under ${root}; nothing to compare against"
	exit 0
fi

newer="$(find "${existing_paths[@]}" -type f -newer "$stamp" \
	-not -path '*/.git/*' \
	-not -path '*/node_modules/*' \
	-not -path '*/.nox/*' \
	-not -path '*/.cache/*' \
	-not -path '*/target/*' \
	-not -path '*/bin/*' \
	-not -name '*.tmp' \
	-print -quit 2>/dev/null || true)"

if [ -n "$newer" ]; then
	build_image "source path '${newer}' is newer than image (${image_created})"
	exit 0
fi

log "image ${image} (built ${image_created}) is up to date with tracked source paths"
