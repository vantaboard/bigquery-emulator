#!/usr/bin/env bash
# Fast iteration loop for the GoogleSQL prebuilt wrapper-deps closure.
#
# The producer pipeline (`googlesql-prebuilt.yml`) takes ~2h 20m on a
# GitHub runner because the `Package the prebuilt artifact` step
# compiles the full ~8k C++ TU of GoogleSQL. The actual smoke that
# validates the artifact's wrapper-deps closure (step
# `Verify via Bazel wrappers (full consumer-wiring consume path)`)
# takes ~53s. Wrapper-deps work touches ONLY:
#
#   * `tools/googlesql-prebuilt/templates/BUILD.bazel`
#     (the artifact's root `:_archive` / `:_all_hdrs` `cc_library`s,
#     where third-party link-line deps are pinned)
#   * `tools/googlesql-prebuilt/wrapper_writer.py`
#     (the per-package wrapper `cc_library` deps that consumers see)
#
# Neither input affects the bytes inside `lib/libgooglesql.a` /
# `lib/libgooglesql_protos.a`, so a tarball staged for a previous
# `googlesql` commit is a valid fixture for new wrapper-deps edits
# until the source commit itself moves. This script reuses the
# already-staged cache at `.cache/googlesql-prebuilt/` (populated
# via `task googlesql:stage-bazel` or `task googlesql:fetch-prebuilt`),
# overlays the workspace's current wrapper templates, and runs the
# `--smoke-mode=bazel` flow from `verify.sh` against the result.
#
# Loop time:
#
#   - First run on a clean host:    ~5-10 min (cold Bazel cache for
#                                   the smoke workspace; fetches
#                                   protobuf + abseil + rules_foreign_cc
#                                   + transitive Bzlmod closure).
#   - Subsequent wrapper-deps runs: ~30-60s (warm cache; just
#                                   re-resolves the wrapper BUILD
#                                   files and re-links the smoke
#                                   binary).
#
# This is the iteration loop the producer pipeline does NOT support
# (its only knob is "rebuild from scratch on a fresh runner"). When
# the source commit moves (`googlesql` repo bumps), restage the cache
# (`task googlesql:stage-bazel` or a fresh fetch-prebuilt) and the
# loop continues.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT_OF_EMULATOR="$(cd "$SCRIPT_DIR/../.." && pwd)"
TEMPLATES_BUILD="$SCRIPT_DIR/templates/BUILD.bazel"
WRAPPER_WRITER="$SCRIPT_DIR/wrapper_writer.py"
SMOKE_DIR="$SCRIPT_DIR/smoke"
PREBUILT_REPO_NAME="googlesql_prebuilt_linux_amd64"
DEFAULT_CACHE="$REPO_ROOT_OF_EMULATOR/.cache/googlesql-prebuilt/$PREBUILT_REPO_NAME"

usage() {
    cat <<'EOF'
local_smoke.sh — fast wrapper-deps iteration loop for the GoogleSQL prebuilt

Validates the workspace's current wrapper-deps closure
(`templates/BUILD.bazel` + `wrapper_writer.py` output) against a
previously-staged prebuilt artifact, without rebuilding the bundled
static archives.

Optional:
  --cache-dir PATH             Staged prebuilt artifact root.
                               Default: .cache/googlesql-prebuilt/googlesql_prebuilt_linux_amd64/
  --keep-tmp                   Don't delete the overlay tmpdir on exit.
                               Useful for inspecting the synthesized
                               smoke workspace or re-running bazel by
                               hand against the overlay.
  --jobs N                     Bazel --jobs cap. Default: 2 (matches the
                               producer workflow's throttle).
  --mem-mb N                   Bazel --local_resources=memory cap.
                               Default: 4096.
  -h | --help                  Print this help and exit.

Exit codes:
  0 = smoke passed (the synthesized `//:smoke` binary linked).
  1 = smoke failed (link error, missing dep, or other Bazel failure).
  2 = bad CLI usage / missing prerequisites.

Prerequisites:
  - A staged prebuilt cache (populate via `task googlesql:stage-bazel`
    once for the current googlesql commit, or
    `task googlesql:fetch-prebuilt URL=... SHA256=...`).
  - Python 3, bazelisk OR bazel, clang-18 (or compatible) on PATH.
EOF
}

die() {
    printf 'local_smoke.sh: %s\n' "$*" >&2
    exit 1
}

log() {
    printf '[local_smoke.sh] %s\n' "$*" >&2
}

CACHE_DIR="$DEFAULT_CACHE"
KEEP_TMP=0
JOBS="${BAZEL_JOBS:-2}"
MEM_MB="${BAZEL_MEM_MB:-4096}"

while [ $# -gt 0 ]; do
    case "$1" in
        --cache-dir=*) CACHE_DIR="${1#--cache-dir=}"; shift ;;
        --cache-dir) CACHE_DIR="$2"; shift 2 ;;
        --keep-tmp) KEEP_TMP=1; shift ;;
        --jobs=*) JOBS="${1#--jobs=}"; shift ;;
        --jobs) JOBS="$2"; shift 2 ;;
        --mem-mb=*) MEM_MB="${1#--mem-mb=}"; shift ;;
        --mem-mb) MEM_MB="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; die "unknown argument: $1" ;;
    esac
done

# ---------------------------------------------------------------------------
# Preconditions.
# ---------------------------------------------------------------------------

[ -d "$CACHE_DIR" ] \
    || die "staged prebuilt cache not found at $CACHE_DIR
  Run 'task googlesql:stage-bazel' once for the current googlesql commit,
  or 'task googlesql:fetch-prebuilt URL=... SHA256=...' to populate it."

[ -f "$CACHE_DIR/MODULE.bazel" ] \
    || die "$CACHE_DIR is not a staged prebuilt root (missing MODULE.bazel)"
[ -f "$CACHE_DIR/manifest.json" ] \
    || die "$CACHE_DIR is not a staged prebuilt root (missing manifest.json)"
[ -f "$CACHE_DIR/lib/libgooglesql.a" ] \
    || die "$CACHE_DIR is not a staged prebuilt root (missing lib/libgooglesql.a)"

[ -f "$TEMPLATES_BUILD" ] \
    || die "missing workspace template: $TEMPLATES_BUILD"
[ -f "$WRAPPER_WRITER" ] \
    || die "missing workspace tool: $WRAPPER_WRITER"
[ -d "$SMOKE_DIR" ] \
    || die "missing smoke fixture dir: $SMOKE_DIR"

command -v python3 >/dev/null 2>&1 \
    || die "python3 is required on PATH"

bazel_bin=""
if command -v bazelisk >/dev/null 2>&1; then
    bazel_bin="bazelisk"
elif command -v bazel >/dev/null 2>&1; then
    bazel_bin="bazel"
else
    die "bazel or bazelisk is required on PATH"
fi

# ---------------------------------------------------------------------------
# Stage the overlay.
# ---------------------------------------------------------------------------

TMPDIR="$(mktemp -d -t gsql-local-smoke-XXXXXX)"
trap_handler() {
    # Release the smoke workspace's bazel daemon FIRST (while the
    # workspace dir still exists — `bazel shutdown` requires it).
    # This honours the bazel-process-hygiene rule's
    # "do not leave a daemon alive past your work" invariant; without
    # it, the smoke daemon's ~1 GB heap sits idle for 3h between
    # iterations.
    (cd "$TMPDIR/smoke_workspace" 2>/dev/null && "$bazel_bin" shutdown >/dev/null 2>&1) || true
    if [ "$KEEP_TMP" -eq 0 ]; then
        rm -rf "$TMPDIR"
    else
        printf '[local_smoke.sh] kept tmp at %s (--keep-tmp)\n' "$TMPDIR" >&2
    fi
}
trap trap_handler EXIT INT TERM

log "overlay tmpdir: $TMPDIR"

OVERLAY_ROOT="$TMPDIR/$PREBUILT_REPO_NAME"
log "copying staged cache -> overlay: $CACHE_DIR -> $OVERLAY_ROOT"
# `cp -R` is intentional: we deep-copy so that wrapper_writer.py + the
# BUILD.bazel overlay below do NOT mutate the user's staged cache.
cp -R "$CACHE_DIR" "$OVERLAY_ROOT"

log "overlaying templates/BUILD.bazel"
cp "$TEMPLATES_BUILD" "$OVERLAY_ROOT/BUILD.bazel"

log "regenerating per-package wrapper BUILD files via wrapper_writer.py"
python3 "$WRAPPER_WRITER" --repo-root "$OVERLAY_ROOT" >/dev/null

# ---------------------------------------------------------------------------
# Smoke wiring (mirrors verify.sh's run_smoke_bazel, minus the gates that
# validate identity-of-the-artifact-as-shipped — those don't change between
# wrapper-deps iterations).
# ---------------------------------------------------------------------------

MANIFEST="$OVERLAY_ROOT/manifest.json"

absl_version="$(grep -E 'bazel_dep\(name = "abseil-cpp"' "$OVERLAY_ROOT/MODULE.bazel" | sed -E 's/.*version = "([^"]+)".*/\1/' | head -1)"
protobuf_version="$(grep -E 'bazel_dep\(name = "protobuf"' "$OVERLAY_ROOT/MODULE.bazel" | sed -E 's/.*version = "([^"]+)".*/\1/' | head -1)"
grpc_version="$(grep -E 'bazel_dep\(name = "grpc"' "$OVERLAY_ROOT/MODULE.bazel" | sed -E 's/.*version = "([^"]+)".*/\1/' | head -1)"
artifact_version="$(grep -E '^\s*version = ' "$OVERLAY_ROOT/MODULE.bazel" | head -1 | sed -E 's/.*"([^"]+)".*/\1/')"

[ -n "$absl_version" ] && [ -n "$protobuf_version" ] && [ -n "$grpc_version" ] \
    || die "failed to read absl/protobuf/grpc pins from $OVERLAY_ROOT/MODULE.bazel"
[ -n "$artifact_version" ] \
    || die "failed to read artifact_version from $OVERLAY_ROOT/MODULE.bazel"

bazel_version="$(python3 -c "import json; print(json.load(open('$MANIFEST'))['toolchain']['bazel_version'])")"
[ -n "$bazel_version" ] \
    || die "failed to read toolchain.bazel_version from $MANIFEST"

SMOKE_WORKSPACE="$TMPDIR/smoke_workspace"
mkdir -p "$SMOKE_WORKSPACE"
cp "$SMOKE_DIR/smoke_wrappers.cc" "$SMOKE_WORKSPACE/smoke_wrappers.cc"
cp "$SMOKE_DIR/BUILD.bazel" "$SMOKE_WORKSPACE/BUILD.bazel"
printf '%s\n' "$bazel_version" > "$SMOKE_WORKSPACE/.bazelversion"

log "pinned smoke workspace to Bazel $bazel_version (from manifest.toolchain.bazel_version)"

python3 - "$SMOKE_DIR/MODULE.bazel.tmpl" "$SMOKE_WORKSPACE/MODULE.bazel" <<EOF
import sys
text = open(sys.argv[1]).read()
text = text.replace("{{ARTIFACT_VERSION}}", "$artifact_version")
text = text.replace("{{ARTIFACT_PATH}}", "$OVERLAY_ROOT")
text = text.replace("{{ABSEIL_VERSION}}", "$absl_version")
text = text.replace("{{PROTOBUF_VERSION}}", "$protobuf_version")
text = text.replace("{{GRPC_VERSION}}", "$grpc_version")
open(sys.argv[2], "w").write(text)
EOF

log "smoke workspace ready at $SMOKE_WORKSPACE"

# ---------------------------------------------------------------------------
# Run the smoke.
# ---------------------------------------------------------------------------

# Force /usr/bin/clang-18 toolchain (matches the producer pipeline's
# `CC=/usr/bin/clang CXX=/usr/bin/clang++`). The smoke workspace's
# rules_cc autoconfig honours --action_env.
log "building //:smoke (jobs=$JOBS mem_mb=$MEM_MB bazel=$bazel_version)"
(
    cd "$SMOKE_WORKSPACE"
    CC=/usr/bin/clang CXX=/usr/bin/clang++ PATH=/usr/bin:$PATH \
    "$bazel_bin" build \
        --jobs="$JOBS" \
        --local_resources=cpu="$JOBS" \
        --local_resources=memory="$MEM_MB" \
        --action_env=CC=/usr/bin/clang \
        --action_env=CXX=/usr/bin/clang++ \
        //:smoke
)
log "smoke (bazel): OK"
log
log "RESULT: wrapper-deps closure verified against $CACHE_DIR"
log "  overlay path: $OVERLAY_ROOT$([ "$KEEP_TMP" -eq 1 ] && echo "  (kept; cleanup is your job)")"
