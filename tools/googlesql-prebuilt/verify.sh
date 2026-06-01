#!/usr/bin/env bash
# Verify a GoogleSQL prebuilt artifact (artifact-producer gate).
#
# Unpacks the tarball into a CLEAN tmp dir (never against the source
# checkout), validates `manifest.json` against the closed
# compatibility-surface schema, re-checksums every payload entry,
# asserts no path escapes
# the artifact root, and runs a smoke binary against the packaged
# wrapper layout.
#
# Smoke modes (mutually exclusive):
#
#   --smoke-mode=link        (default) Portable clang++-driven link.
#                            Compiles + links `smoke.cc` against
#                            `${root}/lib/libgooglesql.a` and
#                            `${root}/lib/libgooglesql_protos.a`.
#                            Symbol-free smoke: green for both fixture
#                            stub archives and production real archives.
#
#   --smoke-mode=bazel       Bazel-driven build against the unpacked
#                            artifact's wrapper `cc_library` targets.
#                            Requires Bazel/bazelisk on PATH and a
#                            working toolchain. Designed for the CI
#                            workflow; locally requires the right
#                            deps to be available.
#
#   --smoke-mode=hash-only   Skip smoke; just validate the manifest
#                            + checksums. Used when neither clang nor
#                            bazel is available.
#
# Refusal behaviour: any of the gates below failing is a HARD error
# (`exit 1`). This is the producer's "do not publish unverified
# payloads" contract.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Safety-gate validator: the closed-schema validator + identity / payload /
# wrapper gates all live in validate_artifact.py. manifest_writer.py is still
# the schema source of truth (and the producer's only writer), but it
# is no longer invoked directly from this script — the validator does.
VALIDATOR="$SCRIPT_DIR/validate_artifact.py"
SMOKE_DIR="$SCRIPT_DIR/smoke"

usage() {
    cat <<'EOF'
verify.sh — verify a GoogleSQL prebuilt artifact

Required:
  --tarball PATH               Path to the .tar.gz to verify.
  --tarball-sha256 SHA         Expected SHA-256 of the tarball (64 hex chars).
                               Mismatch is a hard error.

Optional:
  --smoke-mode MODE            link | bazel | hash-only (default: link).
  --expected-googlesql-sha SHA Refuse if manifest.googlesql.commit does
                               not match this (40 hex chars).
  --expected-artifact-version V Refuse if manifest.artifact_version
                               does not match this strict-semver value.
  --keep-tmp                   Don't delete the extract tmpdir on exit
                               (useful for debugging).
  -h | --help                  Print this help and exit.

Exit codes:
  0 = artifact verified, safe to publish.
  1 = verification failed (manifest, checksums, or smoke).
  2 = bad CLI usage.
EOF
}

die() {
    printf 'verify.sh: %s\n' "$*" >&2
    exit 1
}

log() {
    printf '[verify.sh] %s\n' "$*" >&2
}

TARBALL=""
EXPECTED_TARBALL_SHA=""
SMOKE_MODE="link"
EXPECTED_GS_SHA=""
EXPECTED_ARTIFACT_VERSION=""
KEEP_TMP=0

while [ $# -gt 0 ]; do
    case "$1" in
        --tarball=*) TARBALL="${1#--tarball=}"; shift ;;
        --tarball) TARBALL="$2"; shift 2 ;;
        --tarball-sha256=*) EXPECTED_TARBALL_SHA="${1#--tarball-sha256=}"; shift ;;
        --tarball-sha256) EXPECTED_TARBALL_SHA="$2"; shift 2 ;;
        --smoke-mode=*) SMOKE_MODE="${1#--smoke-mode=}"; shift ;;
        --smoke-mode) SMOKE_MODE="$2"; shift 2 ;;
        --expected-googlesql-sha=*) EXPECTED_GS_SHA="${1#--expected-googlesql-sha=}"; shift ;;
        --expected-googlesql-sha) EXPECTED_GS_SHA="$2"; shift 2 ;;
        --expected-artifact-version=*) EXPECTED_ARTIFACT_VERSION="${1#--expected-artifact-version=}"; shift ;;
        --expected-artifact-version) EXPECTED_ARTIFACT_VERSION="$2"; shift 2 ;;
        --keep-tmp) KEEP_TMP=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; die "unknown argument: $1" ;;
    esac
done

[ -n "$TARBALL" ] || { usage >&2; die "--tarball is required"; }
[ -f "$TARBALL" ] || die "tarball not found: $TARBALL"
[ -n "$EXPECTED_TARBALL_SHA" ] || { usage >&2; die "--tarball-sha256 is required"; }
[[ "$EXPECTED_TARBALL_SHA" =~ ^[0-9a-f]{64}$ ]] \
    || die "--tarball-sha256 must be 64 hex chars; got $EXPECTED_TARBALL_SHA"

case "$SMOKE_MODE" in
    link|bazel|hash-only) ;;
    *) die "--smoke-mode must be 'link', 'bazel', or 'hash-only'; got '$SMOKE_MODE'" ;;
esac

if [ -n "$EXPECTED_GS_SHA" ]; then
    [[ "$EXPECTED_GS_SHA" =~ ^[0-9a-f]{40}$ ]] \
        || die "--expected-googlesql-sha must be 40 hex chars"
fi

TARBALL="$(cd "$(dirname "$TARBALL")" && pwd)/$(basename "$TARBALL")"

# ---------------------------------------------------------------------------
# Gate 1: tarball SHA matches expectation.
# ---------------------------------------------------------------------------

log "checksumming tarball: $TARBALL"
ACTUAL_TARBALL_SHA="$(sha256sum "$TARBALL" | awk '{print $1}')"
if [ "$ACTUAL_TARBALL_SHA" != "$EXPECTED_TARBALL_SHA" ]; then
    die "tarball SHA-256 mismatch:
  expected: $EXPECTED_TARBALL_SHA
  actual:   $ACTUAL_TARBALL_SHA"
fi
log "tarball SHA-256 OK"

# ---------------------------------------------------------------------------
# Gate 2: extract into clean tmp dir.
# ---------------------------------------------------------------------------

TMPDIR="$(mktemp -d -t googlesql-verify-XXXXXX)"
if [ "$KEEP_TMP" -eq 0 ]; then
    trap 'rm -rf "$TMPDIR"' EXIT INT TERM
fi
log "extracting to $TMPDIR"

# Use `tar -t` first to assert no path escapes the artifact root. The
# compatibility-surface contract is "no unpacked path escapes the artifact root".
# Any entry starting with `/`, or containing `..`, fails the gate.
while IFS= read -r path; do
    case "$path" in
        /*|*..*)
            die "tarball contains path-escape entry: $path"
            ;;
    esac
done < <(tar -tzf "$TARBALL")

tar -xzf "$TARBALL" -C "$TMPDIR"

REPO_NAME="googlesql_prebuilt_linux_amd64"
REPO_ROOT="$TMPDIR/$REPO_NAME"
[ -d "$REPO_ROOT" ] \
    || die "tarball did not unpack a top-level '$REPO_NAME/' directory"

log "extracted repo root: $REPO_ROOT"

# ---------------------------------------------------------------------------
# Gate 3 + 4: manifest schema + identity pins + per-file payload integrity.
# Delegated to tools/googlesql-prebuilt/validate_artifact.py (the safety-gate
# single-source-of-truth validator). The validator applies the same gates
# the local task, CI composite action, and Dockerfile invoke, so producer
# verification cannot diverge from consumer expectations.
# ---------------------------------------------------------------------------

MANIFEST="$REPO_ROOT/manifest.json"
[ -f "$MANIFEST" ] || die "missing $MANIFEST in unpacked tarball"

log "running centralized validator (validate_artifact.py)"
validator_args=( --repo-root "$REPO_ROOT" --summary-line )
if [ -n "$EXPECTED_GS_SHA" ]; then
    validator_args+=( --expected-googlesql-sha "$EXPECTED_GS_SHA" )
fi
if [ -n "$EXPECTED_ARTIFACT_VERSION" ]; then
    validator_args+=( --expected-artifact-version "$EXPECTED_ARTIFACT_VERSION" )
fi
python3 "$VALIDATOR" "${validator_args[@]}"

# ---------------------------------------------------------------------------
# Gate 5: smoke.
# ---------------------------------------------------------------------------

run_smoke_link() {
    log "smoke (link): clang++ ${REPO_ROOT}/include + ${REPO_ROOT}/lib"
    if ! command -v clang++ >/dev/null 2>&1; then
        die "smoke (link) requires clang++ on PATH; install clang or use --smoke-mode=hash-only"
    fi
    local smoke_out="$TMPDIR/smoke_bin"
    clang++ \
        -std=c++17 \
        -I"$REPO_ROOT/include" \
        "$SMOKE_DIR/smoke.cc" \
        -L"$REPO_ROOT/lib" \
        "$REPO_ROOT/lib/libgooglesql.a" \
        "$REPO_ROOT/lib/libgooglesql_protos.a" \
        -o "$smoke_out"
    log "smoke (link): built $smoke_out; executing"
    "$smoke_out"
    log "smoke (link): OK"
}

run_smoke_bazel() {
    log "smoke (bazel): wiring tmp workspace + building //smoke:smoke"
    if ! command -v bazelisk >/dev/null 2>&1 && ! command -v bazel >/dev/null 2>&1; then
        die "smoke (bazel) requires bazelisk or bazel on PATH"
    fi
    local bazel_bin="bazel"
    command -v bazelisk >/dev/null 2>&1 && bazel_bin="bazelisk"

    local manifest_json
    manifest_json="$(python3 -c "import json; print(json.dumps(json.load(open('$MANIFEST'))))" )"
    local absl_version protobuf_version grpc_version artifact_version
    absl_version="$(python3 -c "import json,sys; m=json.loads('''$manifest_json'''); \
        print([dep.split('==')[1] if '==' in dep else '' for dep in m['bundled_thirdparty_deps']] or [''])[0]" 2>/dev/null || echo "")"
    # The manifest does not currently carry the Bzlmod abseil/protobuf
    # version pins (the wrapper repo's MODULE.bazel does). Read them
    # from the unpacked MODULE.bazel directly.
    absl_version="$(grep -E 'bazel_dep\(name = "abseil-cpp"' "$REPO_ROOT/MODULE.bazel" | sed -E 's/.*version = "([^"]+)".*/\1/' | head -1)"
    protobuf_version="$(grep -E 'bazel_dep\(name = "protobuf"' "$REPO_ROOT/MODULE.bazel" | sed -E 's/.*version = "([^"]+)".*/\1/' | head -1)"
    grpc_version="$(grep -E 'bazel_dep\(name = "grpc"' "$REPO_ROOT/MODULE.bazel" | sed -E 's/.*version = "([^"]+)".*/\1/' | head -1)"
    artifact_version="$(grep -E '^\s*version = ' "$REPO_ROOT/MODULE.bazel" | head -1 | sed -E 's/.*"([^"]+)".*/\1/')"

    [ -n "$absl_version" ] && [ -n "$protobuf_version" ] && [ -n "$grpc_version" ] \
        || die "failed to read absl/protobuf/grpc pins from $REPO_ROOT/MODULE.bazel"
    [ -n "$artifact_version" ] || die "failed to read artifact_version from $REPO_ROOT/MODULE.bazel"

    local smoke_workspace="$TMPDIR/smoke_workspace"
    mkdir -p "$smoke_workspace"
    cp "$SMOKE_DIR/smoke_wrappers.cc" "$smoke_workspace/smoke_wrappers.cc"
    cp "$SMOKE_DIR/BUILD.bazel" "$smoke_workspace/BUILD.bazel"

    # Pin the smoke workspace to the artifact's declared Bazel version
    # so bazelisk doesn't fetch the bleeding-edge Bazel for the smoke
    # while the artifact was built against an older one. Without this
    # pin, bazelisk resolves to its default-latest (Bazel 9.x as of
    # writing), which (a) prints `--check_direct_dependencies` warnings
    # because it re-resolves rules_cc / protobuf / etc. transitively
    # past the artifact's MODULE.bazel pins, and (b) fails analysis of
    # `rules_foreign_cc@0.10.1` (transitively required by ICU under
    # this artifact's MODULE.bazel) with
    # `rule() got unexpected keyword argument 'incompatible_use_toolchain_transition'`
    # because that keyword was removed in Bazel 9 (the transition is
    # the default now). The smoke is meant to mirror the consumer's
    # wiring, not invent a fresh Bazel + transitive set; pinning to
    # `manifest.toolchain.bazel_version` exactly matches what the
    # producer built with and what real consumers should be using.
    local bazel_version
    bazel_version="$(python3 -c "import json; print(json.load(open('$MANIFEST'))['toolchain']['bazel_version'])")"
    [ -n "$bazel_version" ] \
        || die "failed to read toolchain.bazel_version from $MANIFEST"
    printf '%s\n' "$bazel_version" > "$smoke_workspace/.bazelversion"
    log "smoke (bazel): pinned smoke workspace to Bazel $bazel_version (from manifest.toolchain.bazel_version)"

    # Substitute MODULE.bazel template.
    python3 - "$SMOKE_DIR/MODULE.bazel.tmpl" "$smoke_workspace/MODULE.bazel" <<EOF
import sys
text = open(sys.argv[1]).read()
text = text.replace("{{ARTIFACT_VERSION}}", "$artifact_version")
text = text.replace("{{ARTIFACT_PATH}}", "$REPO_ROOT")
text = text.replace("{{ABSEIL_VERSION}}", "$absl_version")
text = text.replace("{{PROTOBUF_VERSION}}", "$protobuf_version")
text = text.replace("{{GRPC_VERSION}}", "$grpc_version")
open(sys.argv[2], "w").write(text)
EOF

    log "smoke (bazel): workspace ready at $smoke_workspace"
    (
        cd "$smoke_workspace"
        # Throttled defaults: the smoke is tiny but `bazel build`
        # still spins up the daemon; cap parallelism so we don't
        # collide with whatever the user is doing on the same host.
        local jobs="${BAZEL_JOBS:-2}"
        local mem_mb="${BAZEL_MEM_MB:-2048}"
        # shellcheck disable=SC2064  # we *want* "$bazel_bin" expanded now;
        # by the time the trap fires we may be in a subshell whose
        # local-var binding has gone out of scope. Single quotes would
        # break that.
        trap "$bazel_bin shutdown 2>/dev/null || true" EXIT INT TERM
        CC=/usr/bin/clang CXX=/usr/bin/clang++ PATH=/usr/bin:$PATH \
        "$bazel_bin" build \
            --jobs="$jobs" \
            --local_resources=cpu="$jobs" \
            --local_resources=memory="$mem_mb" \
            --action_env=CC=/usr/bin/clang \
            --action_env=CXX=/usr/bin/clang++ \
            //:smoke
    )
    log "smoke (bazel): OK"
}

case "$SMOKE_MODE" in
    link) run_smoke_link ;;
    bazel) run_smoke_bazel ;;
    hash-only) log "smoke skipped (hash-only mode)" ;;
esac

# ---------------------------------------------------------------------------
# Done.
# ---------------------------------------------------------------------------

log "VERIFIED: $TARBALL"
log "  sha256:       $ACTUAL_TARBALL_SHA"
log "  smoke mode:   $SMOKE_MODE"
log "  extract path: $REPO_ROOT$([ "$KEEP_TMP" -eq 1 ] && echo "  (kept; cleanup is your job)")"
