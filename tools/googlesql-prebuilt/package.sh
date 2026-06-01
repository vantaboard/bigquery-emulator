#!/usr/bin/env bash
# Package the GoogleSQL prebuilt artifact (artifact-producer pipeline).
#
# Stages the exact `@googlesql_prebuilt_linux_amd64` repo layout frozen
# in docs/dev/googlesql-prebuilt/repo-layout.md, generates the wrapper
# BUILD.bazel files via tools/googlesql-prebuilt/wrapper_writer.py, and
# writes the manifest via tools/googlesql-prebuilt/manifest_writer.py.
# Tarballs the staged tree and emits a sidecar .sha256 with the tarball
# digest so the consumer's http_archive can pin it.
#
# Modes:
#
#   --mode=bazel    (production) Run `bazel build` against the
#                   GoogleSQL source tree and harvest .a + headers.
#                   Heavy: 25-55 min on a fresh cache; the C++ engine
#                   compiles ~8K TUs.
#
#   --mode=fixture  (testing)    Stage hand-picked headers from
#                   GOOGLESQL_SRC, produce STUB static archives via
#                   `ar rcS`, and emit a valid (but link-stub) artifact.
#                   Lets the workflow + verify.sh be exercised
#                   end-to-end without a full Bazel build.
#
# Required env / args:
#   --googlesql-src PATH        Sibling GoogleSQL checkout.
#   --emulator-src PATH         bigquery-emulator checkout (for finding
#                               templates + MODULE.bazel toolchain pins).
#   --artifact-version SEMVER   Strict semver (e.g. 0.1.0). Refusal-grade:
#                               this is the user-facing contract version.
#   --out-dir PATH              Where to stage the repo + write the .tar.gz.
#
# Optional:
#   --googlesql-commit SHA      Pin commit (default: derived from --googlesql-src).
#   --emulator-min-commit SHA   Pin emulator floor (default: HEAD of --emulator-src).
#   --workflow-id NAME          producer.workflow field (default: "manual").
#   --run-id ID                 producer.run_id field (default: "0").
#   --tarball-name NAME         Override default name shape.
#   -h | --help                 Print this help and exit.
#
# Outputs (under --out-dir):
#   googlesql_prebuilt_linux_amd64/   Staged repo (input to verify.sh).
#   googlesql-prebuilt-linux-amd64-clang18-<short_sha>-v<version>.tar.gz
#   googlesql-prebuilt-linux-amd64-clang18-<short_sha>-v<version>.tar.gz.sha256
#   googlesql-prebuilt-linux-amd64-clang18-<short_sha>-v<version>.manifest.json
#                                     Standalone copy of manifest.json
#                                     so a consumer can read it without
#                                     downloading the tarball.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEMPLATES_DIR="$SCRIPT_DIR/templates"
MANIFEST_WRITER="$SCRIPT_DIR/manifest_writer.py"
WRAPPER_WRITER="$SCRIPT_DIR/wrapper_writer.py"

REPO_NAME="googlesql_prebuilt_linux_amd64"

usage() {
    sed -n '2,/^set -euo pipefail/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

die() {
    printf 'package.sh: %s\n' "$*" >&2
    exit 1
}

log() {
    printf '[package.sh] %s\n' "$*" >&2
}

MODE=""
GOOGLESQL_SRC=""
EMULATOR_SRC=""
ARTIFACT_VERSION=""
OUT_DIR=""
GOOGLESQL_COMMIT=""
EMULATOR_MIN_COMMIT=""
WORKFLOW_ID="manual"
RUN_ID="0"
TARBALL_NAME=""

while [ $# -gt 0 ]; do
    case "$1" in
        --mode=*) MODE="${1#--mode=}"; shift ;;
        --mode) MODE="$2"; shift 2 ;;
        --googlesql-src=*) GOOGLESQL_SRC="${1#--googlesql-src=}"; shift ;;
        --googlesql-src) GOOGLESQL_SRC="$2"; shift 2 ;;
        --emulator-src=*) EMULATOR_SRC="${1#--emulator-src=}"; shift ;;
        --emulator-src) EMULATOR_SRC="$2"; shift 2 ;;
        --artifact-version=*) ARTIFACT_VERSION="${1#--artifact-version=}"; shift ;;
        --artifact-version) ARTIFACT_VERSION="$2"; shift 2 ;;
        --out-dir=*) OUT_DIR="${1#--out-dir=}"; shift ;;
        --out-dir) OUT_DIR="$2"; shift 2 ;;
        --googlesql-commit=*) GOOGLESQL_COMMIT="${1#--googlesql-commit=}"; shift ;;
        --googlesql-commit) GOOGLESQL_COMMIT="$2"; shift 2 ;;
        --emulator-min-commit=*) EMULATOR_MIN_COMMIT="${1#--emulator-min-commit=}"; shift ;;
        --emulator-min-commit) EMULATOR_MIN_COMMIT="$2"; shift 2 ;;
        --workflow-id=*) WORKFLOW_ID="${1#--workflow-id=}"; shift ;;
        --workflow-id) WORKFLOW_ID="$2"; shift 2 ;;
        --run-id=*) RUN_ID="${1#--run-id=}"; shift ;;
        --run-id) RUN_ID="$2"; shift 2 ;;
        --tarball-name=*) TARBALL_NAME="${1#--tarball-name=}"; shift ;;
        --tarball-name) TARBALL_NAME="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) die "unknown argument: $1" ;;
    esac
done

[ -n "$MODE" ] || die "--mode is required (bazel|fixture)"
case "$MODE" in
    bazel|fixture) ;;
    *) die "--mode must be 'bazel' or 'fixture'; got '$MODE'" ;;
esac
[ -n "$GOOGLESQL_SRC" ] || die "--googlesql-src is required"
[ -n "$EMULATOR_SRC" ] || die "--emulator-src is required"
[ -n "$ARTIFACT_VERSION" ] || die "--artifact-version is required (strict semver)"
[ -n "$OUT_DIR" ] || die "--out-dir is required"

GOOGLESQL_SRC="$(cd "$GOOGLESQL_SRC" && pwd)"
EMULATOR_SRC="$(cd "$EMULATOR_SRC" && pwd)"
mkdir -p "$OUT_DIR"
OUT_DIR="$(cd "$OUT_DIR" && pwd)"

[ -d "$GOOGLESQL_SRC/googlesql/public" ] \
    || die "googlesql source tree missing googlesql/public/ at $GOOGLESQL_SRC"
[ -f "$EMULATOR_SRC/MODULE.bazel" ] \
    || die "emulator checkout missing MODULE.bazel at $EMULATOR_SRC"

# Resolve commit identity. The producer pins the COMMIT explicitly so
# we never accidentally publish an artifact whose manifest says "main"
# while the on-disk tree was actually at a branch tip.
if [ -z "$GOOGLESQL_COMMIT" ]; then
    GOOGLESQL_COMMIT="$(git -C "$GOOGLESQL_SRC" rev-parse HEAD)" \
        || die "failed to read googlesql HEAD at $GOOGLESQL_SRC"
fi
[[ "$GOOGLESQL_COMMIT" =~ ^[0-9a-f]{40}$ ]] \
    || die "--googlesql-commit must be 40 hex chars; got $GOOGLESQL_COMMIT"
SHORT_SHA="${GOOGLESQL_COMMIT:0:12}"

if [ -z "$EMULATOR_MIN_COMMIT" ]; then
    EMULATOR_MIN_COMMIT="$(git -C "$EMULATOR_SRC" rev-parse HEAD 2>/dev/null || true)"
fi
if [ -z "$EMULATOR_MIN_COMMIT" ]; then
    # Dirty / no-git checkout: synthesize a zero SHA so the manifest
    # still parses. A consumer pin to this will refuse to match any
    # real commit; that's the right failure mode for a local exercise.
    EMULATOR_MIN_COMMIT="0000000000000000000000000000000000000000"
fi
[[ "$EMULATOR_MIN_COMMIT" =~ ^[0-9a-f]{40}$ ]] \
    || die "emulator min_commit must be 40 hex chars; got $EMULATOR_MIN_COMMIT"

# Validate strict-semver artifact version. The manifest validator also
# enforces this, but failing here gives a friendlier diagnostic.
[[ "$ARTIFACT_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.-]+)?$ ]] \
    || die "--artifact-version must be strict semver (got '$ARTIFACT_VERSION')"

# Resolve toolchain pins from the emulator's MODULE.bazel so the
# manifest's `MODULE.bazel.tmpl` substitutions match the source build.
read_bazel_dep_version() {
    local module_name="$1"
    grep -E "^\\s*bazel_dep\\(\\s*name = \"${module_name}\"" "$EMULATOR_SRC/MODULE.bazel" \
        | sed -E 's/.*version = "([^"]+)".*/\1/' \
        | head -n1
}

ABSEIL_VERSION="$(read_bazel_dep_version 'abseil-cpp')"
PROTOBUF_VERSION="$(read_bazel_dep_version 'protobuf')"
GRPC_VERSION="$(read_bazel_dep_version 'grpc')"
[ -n "$ABSEIL_VERSION" ] && [ -n "$PROTOBUF_VERSION" ] && [ -n "$GRPC_VERSION" ] \
    || die "failed to read abseil/protobuf/grpc pins from $EMULATOR_SRC/MODULE.bazel"

# Consumer-wiring verification surfaced three more transitive third-party
# Bzlmod modules referenced by the artifact's exported header tree
# (gtest_prod.h, openssl/bn.h, re2/re2.h). Read them from the
# upstream googlesql/MODULE.bazel rather than the consumer's, since
# the consumer doesn't directly depend on re2 / boringssl. Reading
# from upstream also makes the version pin self-consistent: the
# static archives in `lib/` were linked against THESE versions.
read_bazel_dep_version_from() {
    local file="$1"
    local module_name="$2"
    grep -E "^\\s*bazel_dep\\(\\s*name = \"${module_name}\"" "$file" \
        | sed -E 's/.*version = "([^"]+)".*/\1/' \
        | head -n1
}
GOOGLETEST_VERSION="$(read_bazel_dep_version_from "$GOOGLESQL_SRC/MODULE.bazel" googletest)"
RE2_VERSION="$(read_bazel_dep_version_from "$GOOGLESQL_SRC/MODULE.bazel" re2)"
BORINGSSL_VERSION="$(read_bazel_dep_version_from "$GOOGLESQL_SRC/MODULE.bazel" boringssl)"
GOOGLEAPIS_VERSION="$(read_bazel_dep_version_from "$GOOGLESQL_SRC/MODULE.bazel" googleapis)"
GOOGLEAPIS_CC_VERSION="$(read_bazel_dep_version_from "$GOOGLESQL_SRC/MODULE.bazel" googleapis-cc)"
[ -n "$GOOGLETEST_VERSION" ] && [ -n "$RE2_VERSION" ] && [ -n "$BORINGSSL_VERSION" ] \
    && [ -n "$GOOGLEAPIS_VERSION" ] && [ -n "$GOOGLEAPIS_CC_VERSION" ] \
    || die "failed to read googletest/re2/boringssl/googleapis pins from $GOOGLESQL_SRC/MODULE.bazel"

# ICU, farmhash, and differential-privacy are fetched by upstream
# googlesql via `googlesql/bazel/http_archive_deps.bzl` (NOT BCR
# `bazel_dep`s). Parse their pinned versions from the same file so the
# manifest's `bundled_thirdparty_deps` field reports the exact source
# identity of the bytes we just statically linked into
# `libgooglesql.a`.
HTTP_ARCHIVE_DEPS="$GOOGLESQL_SRC/bazel/http_archive_deps.bzl"
if [ -r "$HTTP_ARCHIVE_DEPS" ]; then
    # `strip_prefix = "icu"` doesn't carry the version. The URL does:
    # `release-76-1/icu4c-76_1-src.tgz` -> 76.1.
    ICU_VERSION="$(grep -oE 'release-[0-9]+-[0-9]+/icu4c-[0-9]+_[0-9]+-src' "$HTTP_ARCHIVE_DEPS" \
        | head -1 \
        | sed -E 's|release-([0-9]+)-([0-9]+)/.*|\1.\2|' \
        || echo unknown)"
    # `strip_prefix = "farmhash-<sha>"` is the commit pin.
    FARMHASH_COMMIT="$(grep -oE 'farmhash-[0-9a-f]{40}' "$HTTP_ARCHIVE_DEPS" \
        | head -1 \
        | sed -E 's|farmhash-||' \
        || echo unknown)"
    # `strip_prefix = "differential-privacy-<x.y.z>"` (and the same
    # version for the cc/ subdirectory `com_google_cc_differential_privacy`
    # repo). Read one and use it for both since they pin the same
    # upstream tarball.
    DIFFERENTIAL_PRIVACY_VERSION="$(grep -oE 'differential-privacy-[0-9]+\.[0-9]+\.[0-9]+' "$HTTP_ARCHIVE_DEPS" \
        | head -1 \
        | sed -E 's|differential-privacy-||' \
        || echo unknown)"
else
    ICU_VERSION="unknown"
    FARMHASH_COMMIT="unknown"
    DIFFERENTIAL_PRIVACY_VERSION="unknown"
fi
[ -n "$ICU_VERSION" ] && [ "$ICU_VERSION" != "unknown" ] \
    || die "failed to parse ICU version from $HTTP_ARCHIVE_DEPS"
[ -n "$FARMHASH_COMMIT" ] && [ "$FARMHASH_COMMIT" != "unknown" ] \
    || die "failed to parse farmhash commit from $HTTP_ARCHIVE_DEPS"
[ -n "$DIFFERENTIAL_PRIVACY_VERSION" ] && [ "$DIFFERENTIAL_PRIVACY_VERSION" != "unknown" ] \
    || die "failed to parse differential-privacy version from $HTTP_ARCHIVE_DEPS"

BAZEL_VERSION_FILE="$EMULATOR_SRC/.bazelversion"
if [ -r "$BAZEL_VERSION_FILE" ]; then
    BAZEL_VERSION="$(tr -d '[:space:]' < "$BAZEL_VERSION_FILE")"
else
    BAZEL_VERSION="unknown"
fi

# Detect compiler version. The clang in the producer's PATH must match
# the host the artifact will run on. The safety-gate consume-time
# validator warns on mismatch, but for now we just record what we saw.
#
# Prefer system clang at /usr/bin/clang (pinned to clang-18 per
# bazel-process-hygiene.mdc — the mise toolchain's clang ships without
# C++ stdlib headers). Fall back to PATH clang if /usr/bin/clang is
# absent.
detect_version() {
    # Pre-read into a variable to avoid pipefail + SIGPIPE from
    # `command | head -1` truncating the upstream writer.
    local cmd="$1"
    local line
    line="$("$cmd" --version 2>/dev/null || true)"
    line="${line%%$'\n'*}"
    sed -E 's/.*version ([0-9]+\.[0-9]+\.[0-9]+).*/\1/' <<<"$line" 2>/dev/null \
        || echo "unknown"
}

if [ -x /usr/bin/clang ]; then
    CLANG_VERSION="$(detect_version /usr/bin/clang)"
elif command -v clang >/dev/null; then
    CLANG_VERSION="$(detect_version clang)"
else
    CLANG_VERSION="unknown"
fi

# Detect libc / OS release. The manifest schema expects glibc-<MAJ.MIN>.
# Same pipefail caveat as above: read the full ldd --version output into
# a variable before slicing it.
LIBC_VERSION="unknown"
if command -v ldd >/dev/null 2>&1; then
    ldd_out="$(ldd --version 2>/dev/null || true)"
    ldd_first="${ldd_out%%$'\n'*}"
    if [[ "$ldd_first" =~ GLIBC ]]; then
        # ldd output shape (Ubuntu): "ldd (Ubuntu GLIBC 2.39-…) 2.39"
        # The trailing token is the version we want.
        LIBC_VERSION="glibc-$(sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' <<<"$ldd_first")"
    fi
fi

if [ -r /etc/os-release ]; then
    # shellcheck disable=SC1091
    HOST_OS_RELEASE="$(. /etc/os-release && echo "${PRETTY_NAME:-$NAME}")"
else
    HOST_OS_RELEASE="$(uname -sr)"
fi

REPO_STAGE="$OUT_DIR/$REPO_NAME"
log "staging repo at $REPO_STAGE"
rm -rf "$REPO_STAGE"
mkdir -p "$REPO_STAGE/include/googlesql/public/types"
mkdir -p "$REPO_STAGE/include/googlesql/public/proto"
mkdir -p "$REPO_STAGE/include/googlesql/public/functions"
mkdir -p "$REPO_STAGE/include/googlesql/base"
mkdir -p "$REPO_STAGE/include/googlesql/common"
mkdir -p "$REPO_STAGE/include/googlesql/resolved_ast"
mkdir -p "$REPO_STAGE/lib"
mkdir -p "$REPO_STAGE/LICENSES"

# Copy upstream license verbatim. Refuse to ship without it.
[ -f "$GOOGLESQL_SRC/LICENSE" ] \
    || die "googlesql source missing LICENSE (refusing to ship without it)"
cp "$GOOGLESQL_SRC/LICENSE" "$REPO_STAGE/LICENSES/googlesql-LICENSE"

# Generate a NOTICE listing both the bundled and the consumer-resolved
# third-party deps. ICU and farmhash are statically linked into
# lib/libgooglesql.a (their objects appear under `_thirdparty_icu76__`
# and `_thirdparty_farmhash__` names inside the archive). Everything
# else is Bzlmod-resolved by the consumer's MODULE.bazel at link time.
cat > "$REPO_STAGE/LICENSES/thirdparty-NOTICE" <<EOF
GoogleSQL prebuilt artifact ($REPO_NAME) — third-party dependencies.

The static archive lib/libgooglesql.a contains object code from the
upstream GoogleSQL repository (https://github.com/google/googlesql,
commit $GOOGLESQL_COMMIT) PLUS the following STATICALLY BUNDLED
third-party libraries (see manifest's bundled_thirdparty_deps):

  - icu $ICU_VERSION (Unicode license) — libicuuc, libicui18n, libicudata
    Bundled because upstream GoogleSQL fetches it via http_archive
    with a custom BUILD file; the closest BCR module (icu 78.x) uses
    incompatible \`icu_78::\` namespace versioning.
  - farmhash $FARMHASH_COMMIT (MIT) — farmhash.pic.o
    Bundled because farmhash is not published in the Bazel Central
    Registry; upstream GoogleSQL fetches it via http_archive.
  - differential-privacy $DIFFERENTIAL_PRIVACY_VERSION (Apache-2.0) —
    com_google_differential_privacy + com_google_cc_differential_privacy
    cc_library objects plus the four cc_proto_library outputs
    (confidence_interval, data, numerical_mechanism, summary), and
    the cephes inverse_gaussian_cdf object. Bundled because the
    differential-privacy repo is not published in the Bazel Central
    Registry; upstream GoogleSQL fetches it via http_archive at the
    pinned 4.x release tarball.

The following libraries are NOT bundled — they are Bzlmod-resolved by
the consumer's MODULE.bazel at link time (the prebuilt repo's own
MODULE.bazel declares matching \`bazel_dep\` lines so the consumer
resolves the same versions the producer linked against):

  - abseil-cpp $ABSEIL_VERSION (Apache-2.0)
  - protobuf $PROTOBUF_VERSION (BSD-3-Clause)
  - grpc $GRPC_VERSION (Apache-2.0)
  - boringssl $BORINGSSL_VERSION (OpenSSL / ISC)
  - re2 $RE2_VERSION (BSD-3-Clause)
  - googletest $GOOGLETEST_VERSION (BSD-3-Clause)

See docs/dev/googlesql-prebuilt/headers-and-libraries.md for the
rationale behind the bundle / consumer-resolved split.
EOF

# ---------------------------------------------------------------------------
# Stage payload (mode-specific)
# ---------------------------------------------------------------------------

stage_fixture() {
    log "MODE=fixture: copying hand-picked headers from $GOOGLESQL_SRC"
    # The 25 direct headers frozen by the compatibility surface
    # (headers-and-libraries.md "Today that yields:"). We additionally
    # copy a generous closure of types/, proto/, resolved_ast/, base/,
    # common/ headers so the manifest payload looks realistic. The
    # closure isn't the full transitive set the production producer
    # would emit, but it's enough to exercise the verifier + wrapper
    # writer end-to-end.
    local public_hdrs=(
        analyzer.h
        analyzer_options.h
        analyzer_output.h
        analyzer_output_properties.h
        builtin_function_options.h
        catalog.h
        catalog_helper.h
        convert_type_to_proto.h
        error_helpers.h
        evaluator.h
        evaluator_base.h
        evaluator_table_iterator.h
        function.h
        function_signature.h
        input_argument_type.h
        language_options.h
        procedure.h
        property_graph.h
        proto_util.h
        simple_catalog.h
        simple_property_graph.h
        table_from_proto.h
        table_valued_function.h
        type.h
        value.h
    )
    local h
    for h in "${public_hdrs[@]}"; do
        local src="$GOOGLESQL_SRC/googlesql/public/$h"
        if [ -f "$src" ]; then
            cp "$src" "$REPO_STAGE/include/googlesql/public/$h"
        else
            log "WARNING: $h missing in upstream; synthesizing a stub"
            printf '// fixture stub for missing upstream header\n#pragma once\n' \
                > "$REPO_STAGE/include/googlesql/public/$h"
        fi
    done

    # Generated headers (.pb.h). Real producer would harvest these from
    # bazel-bin; fixture mode emits self-contained stubs so the manifest
    # has a deterministic payload and the verifier can re-hash them.
    cat > "$REPO_STAGE/include/googlesql/public/error_location.pb.h" <<'EOF'
// fixture stub for error_location.pb.h
#pragma once
namespace googlesql { class ErrorLocation; }
EOF
    cat > "$REPO_STAGE/include/googlesql/public/options.pb.h" <<'EOF'
// fixture stub for options.pb.h
#pragma once
namespace googlesql { enum ProductMode { PRODUCT_INTERNAL = 0, PRODUCT_EXTERNAL = 1 }; }
EOF
    cat > "$REPO_STAGE/include/googlesql/public/type.pb.h" <<'EOF'
// fixture stub for type.pb.h
#pragma once
namespace googlesql { class TypeProto; }
EOF

    # Types/ closure (load-bearing per repo-layout.md).
    local type_hdrs=(
        array_type.h
        struct_type.h
        type_factory.h
        enum_type.h
        proto_type.h
        range_type.h
    )
    for h in "${type_hdrs[@]}"; do
        local src="$GOOGLESQL_SRC/googlesql/public/types/$h"
        if [ -f "$src" ]; then
            cp "$src" "$REPO_STAGE/include/googlesql/public/types/$h"
        else
            log "WARNING: types/$h missing; stubbing"
            printf '// fixture stub for missing upstream types/%s\n#pragma once\n' \
                "$h" > "$REPO_STAGE/include/googlesql/public/types/$h"
        fi
    done

    # Resolved AST closure.
    cat > "$REPO_STAGE/include/googlesql/resolved_ast/resolved_ast.h" <<'EOF'
// fixture stub for resolved_ast.h
#pragma once
namespace googlesql { class ResolvedNode; }
EOF
    cat > "$REPO_STAGE/include/googlesql/resolved_ast/resolved_ast_visitor.h" <<'EOF'
// fixture stub for resolved_ast_visitor.h
#pragma once
namespace googlesql { class ResolvedASTVisitor {}; }
EOF
    cat > "$REPO_STAGE/include/googlesql/resolved_ast/resolved_collation.h" <<'EOF'
// fixture stub for resolved_collation.h
#pragma once
namespace googlesql { class ResolvedCollation {}; }
EOF
    cat > "$REPO_STAGE/include/googlesql/resolved_ast/resolved_column.h" <<'EOF'
// fixture stub for resolved_column.h
#pragma once
namespace googlesql { class ResolvedColumn {}; }
EOF
    cat > "$REPO_STAGE/include/googlesql/resolved_ast/resolved_node.h" <<'EOF'
// fixture stub for resolved_node.h
#pragma once
namespace googlesql { class ResolvedNode {}; }
EOF
    cat > "$REPO_STAGE/include/googlesql/resolved_ast/resolved_node_kind.pb.h" <<'EOF'
// fixture stub for resolved_node_kind.pb.h
#pragma once
namespace googlesql { enum ResolvedNodeKind { RESOLVED_LITERAL = 1 }; }
EOF

    # Stub static archives via `ar rcS`. Empty archives are valid `ar`
    # files; verify.sh's link-smoke uses an empty-main.cc that doesn't
    # reference any googlesql symbols so the link succeeds.
    log "writing stub libgooglesql.a and libgooglesql_protos.a"
    ar rcS "$REPO_STAGE/lib/libgooglesql.a"
    ar rcS "$REPO_STAGE/lib/libgooglesql_protos.a"
    ranlib "$REPO_STAGE/lib/libgooglesql.a" 2>/dev/null || true
    ranlib "$REPO_STAGE/lib/libgooglesql_protos.a" 2>/dev/null || true
}

stage_bazel() {
    # MODE=bazel runs a throttled `bazel build` inside the GoogleSQL
    # source tree and harvests the per-cc_library .a outputs + the
    # hand-written + generated headers into the staged repo layout.
    #
    # Throttling defaults: BAZEL_JOBS=$(nproc) - 2 (floor 2),
    # BAZEL_MEM_MB ~= 75% of MemTotal (floor 4 GiB). Override via env
    # vars on the command line. Mirrors `taskfiles/bazel.yml` so the
    # producer respects bazel-process-hygiene.mdc on the same host
    # the emulator is built on.
    local jobs="${BAZEL_JOBS:-}"
    if [ -z "$jobs" ]; then
        local n
        n="$(nproc 2>/dev/null || echo 4)"
        if [ "$n" -gt 4 ]; then jobs=$((n - 2)); else jobs=2; fi
    fi
    local memory_mb="${BAZEL_MEM_MB:-}"
    if [ -z "$memory_mb" ] && [ -r /proc/meminfo ]; then
        memory_mb="$(awk '/^MemTotal:/ { mb = int($2 * 3 / 4 / 1024); if (mb < 4096) mb = 4096; print mb }' /proc/meminfo)"
    fi
    [ -n "$memory_mb" ] || memory_mb=8192

    log "MODE=bazel: building googlesql in-place at $GOOGLESQL_SRC"
    log "  jobs=$jobs memory_mb=$memory_mb cc=/usr/bin/clang"

    # The compatibility surface to build. The compatibility-surface
    # docs (`headers-and-libraries.md`) say: closure of `:analyzer`,
    # `:evaluator`, `:resolved_ast`. Add `:simple_catalog`, `:type`,
    # `:value` so their headers are visible at link time (they would
    # come in transitively via deps, but listing them explicitly makes
    # the build a no-op if upstream restructures internal deps).
    local umbrella_targets=(
        //googlesql/public:analyzer
        //googlesql/public:analyzer_options
        //googlesql/public:analyzer_output
        //googlesql/public:builtin_function_options
        //googlesql/public:catalog
        //googlesql/public:error_helpers
        //googlesql/public:error_location_cc_proto
        //googlesql/public:evaluator
        //googlesql/public:evaluator_base
        //googlesql/public:evaluator_table_iterator
        //googlesql/public:function
        //googlesql/public:language_options
        //googlesql/public:options_cc_proto
        //googlesql/public:simple_catalog
        //googlesql/public:type
        //googlesql/public:value
        //googlesql/resolved_ast
        //googlesql/resolved_ast:resolved_node_kind_cc_proto
    )

    # Trap-driven shutdown — releases the daemon JVM on signal or
    # success per bazel-process-hygiene.mdc. The script is bash, which
    # accepts symbolic signal names.
    trap '(cd "'"$GOOGLESQL_SRC"'" && bazel shutdown 2>/dev/null) || true' EXIT INT TERM

    # Toolchain pin: googlesql/MODULE.bazel registers the
    # `@llvm_toolchain//:all` C++ toolchain (clang-21 + libc++) as
    # `dev_dependency = True`. When `bazel build` runs INSIDE the
    # googlesql repo, googlesql IS the root module, so that
    # registration fires and Bazel resolves the C++ toolchain to the
    # LLVM hermetic toolchain. The resulting `.pic.o` files use the
    # libc++ ABI (`std::__1::basic_string` etc.). The bigquery-emulator
    # consumer builds with `/usr/bin/clang` + libstdc++ (`std::__cxx11::
    # basic_string`), so producer + consumer end up with incompatible
    # C++ runtime layouts and the emulator_main link fails with
    # thousands of "undefined reference to absl::lts_20240722::...
    # std::__1::basic_string..." errors. Consumer-wiring prebuilt-mode
    # link verification caught it.
    #
    # `--ignore_dev_dependency` would skip the registration, but it
    # also drops the `single_version_override` overrides marked
    # `dev_dependency` (Bazel applies them as a single bundle), which
    # in googlesql's case includes the abseil-cpp 20240722.1 pin. Once
    # that pin is dropped, Bzlmod resolves abseil-cpp to a much newer
    # version that breaks googlesql's analysis (`absl/utility:
    # if_constexpr` is gone in newer abseil).
    #
    # Workaround: patch the upstream `register_toolchains(...)` block
    # OUT of `googlesql/MODULE.bazel` for the duration of the producer
    # build, then restore it. The patch is a single-block delete
    # bracketed by markers we add ourselves, so the restore is a hard
    # rollback to the file's original byte content (we save/restore
    # via a side file). Bazel still picks up the abseil pin (the
    # `single_version_override` is not a dev_dep) but resolves the C++
    # toolchain to the auto-detected `/usr/bin/clang` host toolchain
    # (libstdc++ ABI, matching the consumer).
    local module_path module_backup module_backup_dir
    module_path="$GOOGLESQL_SRC/MODULE.bazel"
    # The harvest stage's `$tmpdir` is created later; allocate a
    # private dir here for the MODULE.bazel save/restore so the trap
    # below has somewhere to read from on signal.
    module_backup_dir="$(mktemp -d -t googlesql-prebuilt-module-XXXXXX)"
    module_backup="$module_backup_dir/MODULE.bazel.orig"
    [ -f "$module_path" ] || die "missing $module_path"
    cp "$module_path" "$module_backup"
    # Undo on exit / signal; the broader trap below also triggers
    # `bazel shutdown`.
    trap '[ -f "'"$module_backup"'" ] && cp "'"$module_backup"'" "'"$module_path"'"; (cd "'"$GOOGLESQL_SRC"'" && bazel shutdown 2>/dev/null) || true' EXIT INT TERM
    # Remove the `register_toolchains("@llvm_toolchain//:all", ...)`
    # block. It's a multi-line `register_toolchains(...)` ending in a
    # closing paren on its own line; we use Python to scope the edit
    # rather than fragile sed.
    python3 - <<'PY' "$module_path"
import re, sys, pathlib
p = pathlib.Path(sys.argv[1])
text = p.read_text()
# Match `register_toolchains(\n    "@llvm_toolchain//:all",\n    dev_dependency = True,\n)`.
patched, n = re.subn(
    r'register_toolchains\(\s*"@llvm_toolchain//:all"\s*,\s*dev_dependency\s*=\s*True\s*,?\s*\)\n?',
    "# register_toolchains(\"@llvm_toolchain//:all\", dev_dependency = True)  "
    "# patched out by tools/googlesql-prebuilt/package.sh for ABI parity\n",
    text,
)
if n != 1:
    sys.exit(f"register_toolchains patch failed: matched {n} times")
p.write_text(patched)
PY

    # Enumerate every `cc_proto_library` target under //googlesql/...
    # and add it to the build set. Reason: in modern Bazel,
    # building a `cc_library` X that depends on `cc_proto_library` Y
    # does NOT force Y's `.pb.cc` to compile into a `.pic.o` — Y only
    # contributes a header CcInfo for X's TU compile. The actual
    # `.pb.pic.o` materializes only when something asks for Y's
    # archive output (i.e. a `cc_binary` link, or building Y directly).
    # Since we only build `cc_library` umbrellas here, none of the
    # `cc_proto_library` `.pb.pic.o` files get created, and the
    # downstream `libgooglesql.a` is missing every proto message's
    # compiled code — every `FooProto::Clear()` / `_default_instance_`
    # ends up undefined at consumer link time. Explicitly enumerating
    # and building each `cc_proto_library` target forces the proto
    # `.pic.o` to materialize, so the harvest step picks them up.
    #
    # `bazel query` runs in a separate invocation (no `--features=pic`
    # etc.) so the analysis cache is shared with the build below.
    log "enumerating //googlesql/... cc_proto_library targets via bazel query"
    local proto_targets_raw
    proto_targets_raw="$(
        cd "$GOOGLESQL_SRC"
        CC=/usr/bin/clang CXX=/usr/bin/clang++ PATH=/usr/bin:$PATH \
        bazel query 'kind("cc_proto_library", //googlesql/...)'
    )"
    local proto_targets=()
    local proto_target
    while IFS= read -r proto_target; do
        [ -n "$proto_target" ] || continue
        proto_targets+=("$proto_target")
    done <<<"$proto_targets_raw"
    [ "${#proto_targets[@]}" -gt 0 ] \
        || die "bazel query found 0 cc_proto_library targets under //googlesql/..."
    log "  ${#proto_targets[@]} cc_proto_library targets queued"

    # External `cc_proto_library` targets that the bundled GoogleSQL
    # object code references at link time. These live under
    # `@_main~googlesql_http_archive_deps~*` (i.e. the GoogleSQL-side
    # http_archive_deps extension), NOT in BCR — same rationale as
    # ICU + farmhash. Their `.pic.o` files won't materialize unless
    # something explicitly requests their `cc_proto_library` target,
    # and the bundled `libgooglesql.a` archive carries unresolved
    # references into them. Listed explicitly because querying
    # external repos in Bzlmod is brittle (`@<canonical_name>//...`
    # changes per resolution).
    local external_proto_targets=(
        '@com_google_differential_privacy//proto:confidence_interval_cc_proto'
        '@com_google_differential_privacy//proto:data_cc_proto'
        '@com_google_differential_privacy//proto:numerical_mechanism_cc_proto'
        '@com_google_differential_privacy//proto:summary_cc_proto'
    )

    # `--output_groups=+compilation_outputs` is load-bearing for the
    # raw `.pic.o` harvest below. `cc_library`'s DEFAULT outputs are
    # the per-target `lib<target>.[pic.]a` archive — the intermediate
    # `_objs/<target>/<src>.pic.o` objects are NOT declared outputs,
    # so on a Bazel disk-cache hit they remain in the CAS directory
    # and never get materialized into `bazel-out/.../_objs/`. The
    # harvest's `find ... -name '*.pic.o'` then matches zero files
    # and `extract_to_combined` dies with
    # `no .pic.o objects matched for main under bazel-bin/googlesql`.
    # Adding `+compilation_outputs` widens the requested-output set to
    # include the `.pic.o`s, so Bazel restores them from the disk
    # cache (or produces them on a cold build) into `_objs/` where the
    # harvest expects them. Same fix applies to the `umbrella_targets`,
    # `proto_targets`, and `external_proto_targets` — all `cc_library`
    # / `cc_proto_library` outputs flow through the same group.
    (
        cd "$GOOGLESQL_SRC"
        CC=/usr/bin/clang CXX=/usr/bin/clang++ PATH=/usr/bin:$PATH \
        bazel build \
            --jobs="$jobs" \
            --local_resources=cpu="$jobs" \
            --local_resources=memory="$memory_mb" \
            --action_env=CC=/usr/bin/clang \
            --action_env=CXX=/usr/bin/clang++ \
            --features=pic \
            --copt=-O2 \
            --copt=-fPIC \
            --copt=-fno-omit-frame-pointer \
            --copt=-DNDEBUG \
            --output_groups=+compilation_outputs \
            "${umbrella_targets[@]}" \
            "${proto_targets[@]}" \
            "${external_proto_targets[@]}"
    )

    # Restore the upstream MODULE.bazel as soon as the build is done.
    # The trap above is a defense-in-depth safety net for crash paths.
    cp "$module_backup" "$module_path"
    rm -rf "$module_backup_dir"

    local bazel_bin
    bazel_bin="$(cd "$GOOGLESQL_SRC" && bazel info bazel-bin)"
    [ -d "$bazel_bin" ] || die "bazel-bin not found at $bazel_bin"

    log "harvesting hand-written headers from $GOOGLESQL_SRC/googlesql/"
    # Walk the full upstream `googlesql/` source tree and copy every
    # hand-written `.h` (NOT `.h.template` — those expand into bazel-bin).
    # The earlier explicit allowlist (`public/types`, `base`, ...) missed
    # transitive headers like `googlesql/parser/parse_tree.h`, which the
    # public-facing `:analyzer` wrapper reaches via the analyzer's
    # `analyzer.h` `#include`. Consumer-wiring prebuilt-mode link
    # verification caught the gap.
    #
    # Excluded directories: `bazel-*` symlinks (output trees) — we
    # harvest those separately as "generated" headers below. The
    # `bazel-` prefix matches `bazel-bin`, `bazel-out`, `bazel-testlogs`,
    # and the workspace's own `bazel-googlesql` convenience symlink.
    if [ -d "$GOOGLESQL_SRC/googlesql" ]; then
        (
            cd "$GOOGLESQL_SRC/googlesql"
            find . -type f -name '*.h' \
                ! -path './bazel-*' \
                | while read -r relpath; do
                    rel_clean="${relpath#./}"
                    dst="$REPO_STAGE/include/googlesql/$rel_clean"
                    mkdir -p "$(dirname "$dst")"
                    cp "$rel_clean" "$dst"
                done
        )
    fi

    log "harvesting generated headers from $bazel_bin/googlesql"
    # Two classes of generated header live under bazel-bin:
    #   1. `*.pb.h` from `cc_proto_library` rules.
    #   2. Hand-written templates expanded by upstream build helpers
    #      (e.g. `googlesql/resolved_ast/resolved_ast.h` is generated
    #      from `resolved_ast.h.template` by `gen_resolved_ast.py`,
    #      `resolved_node_kind.h` from `resolved_node_kind.h.template`,
    #      the `resolved_ast_*_visitor.h` family, etc.). These are
    #      `*.h` (not `*.pb.h`) and are required by consumers via the
    #      `:resolved_ast` wrapper. The earlier `*.pb.h`-only filter
    #      missed them and consumer-wiring prebuilt-mode link verification
    #      caught the gap (`fatal error: 'googlesql/resolved_ast/
    #      resolved_ast.h' file not found`).
    #
    # We harvest BOTH classes by scanning every `*.h` (.pb.h, .h)
    # under `bazel-bin/googlesql/` whose path does NOT contain a
    # Bazel-internal directory (`_objs/`, `_virtual_includes/`,
    # `_virtual_imports/`, `_objs.bin/`, etc.). Bazel's intermediate
    # output dirs hide internal copies that would clobber the
    # producer-friendly include layout.
    if [ -d "$bazel_bin/googlesql" ]; then
        find "$bazel_bin/googlesql" -type f \( -name '*.h' -o -name '*.pb.h' \) \
            ! -path '*/_objs/*' \
            ! -path '*/_objs.bin/*' \
            ! -path '*/_virtual_includes/*' \
            ! -path '*/_virtual_imports/*' \
        | while read -r genhdr; do
            local rel="${genhdr#"$bazel_bin"/}"
            local dst="$REPO_STAGE/include/$rel"
            mkdir -p "$(dirname "$dst")"
            cp "$genhdr" "$dst"
        done
    fi

    log "harvesting cc_library .a archives from bazel-bin"
    local tmpdir
    tmpdir="$(mktemp -d -t googlesql-prebuilt-XXXXXX)"
    local cleanup_tmpdir="$tmpdir"
    # Defensive: don't leak even if stage_bazel errors out mid-way.
    trap '(cd "'"$GOOGLESQL_SRC"'" && bazel shutdown 2>/dev/null) || true; rm -rf "'"$cleanup_tmpdir"'"' EXIT INT TERM

    extract_to_combined() {
        local out_archive="$1"
        local pattern_inc="$2"  # 'main' or 'proto'
        # Harvest raw `.pic.o` object files from `bazel-bin/googlesql/.../_objs/`.
        #
        # Why raw objects rather than the per-target `lib*.a` archives:
        # `bazel build @googlesql//googlesql/public:analyzer` (and the
        # other umbrella targets) emits one `lib<target>.a` per umbrella
        # `cc_library`, but the umbrella's transitive deps (every other
        # `cc_library` rule under `googlesql/...`) compile their `.pic.o`
        # files into `_objs/<dep_target>/...` directories WITHOUT producing
        # a per-dep archive. Iterating `lib*.a` would therefore miss the
        # transitive object code (e.g. everything under `googlesql/base/`,
        # `googlesql/common/`, most of `googlesql/public/types/`) and the
        # resulting `libgooglesql.a` would be uselessly small. Globbing
        # raw `.pic.o` files captures the full transitive closure.
        #
        # Proto / main split: `cc_proto_library` emits objects whose
        # filenames end in `.pb.pic.o` (e.g. `options.pb.pic.o`,
        # `error_location.pb.pic.o`). Hand-written sources (including
        # files with `proto` in their *target* name like `proto_helper.cc`)
        # compile to plain `<base>.pic.o`. Distinguishing on filename
        # rather than directory name keeps hand-written proto helpers
        # (`proto_helper.pic.o`, `proto_value_conversion.pic.o`) inside
        # the main archive where consumers expect them per
        # `docs/dev/googlesql-prebuilt/repo-layout.md`.
        #
        # Name collision: many sub-targets share `<source>.pic.o`
        # basenames across different `_objs/<target>/` directories
        # (e.g. `function.pic.o` exists in both `_objs/function/` and
        # `_objs/templated_sql_function/`). `ar` keys archive entries
        # by basename, so feeding both via `ar rcsD <archive>
        # path/a/function.pic.o path/b/function.pic.o` records two
        # entries with the same name and the linker resolves to the
        # FIRST one, masking some symbols. We avoid that by COPYING
        # each object into a per-archive tmpdir under a unique
        # `<srcdir>__<name>.pic.o` name (path components flattened with
        # `__`) before invoking `ar`.
        local obj_list_raw obj_list_renamed obj_dir
        obj_dir="$tmpdir/$pattern_inc"
        mkdir -p "$obj_dir"
        obj_list_raw="$tmpdir/${pattern_inc}.raw"
        obj_list_renamed="$tmpdir/${pattern_inc}.list"
        if [ "$pattern_inc" = "proto" ]; then
            find "$bazel_bin/googlesql" -type f -name '*.pb.pic.o' \
                > "$obj_list_raw" 2>/dev/null || true
        else
            find "$bazel_bin/googlesql" -type f -name '*.pic.o' \
                ! -name '*.pb.pic.o' > "$obj_list_raw" 2>/dev/null || true
        fi
        sort -o "$obj_list_raw" "$obj_list_raw"
        local obj_count
        obj_count="$(wc -l < "$obj_list_raw" 2>/dev/null || echo 0)"
        [ "$obj_count" -gt 0 ] \
            || die "no .pic.o objects matched for $pattern_inc under $bazel_bin/googlesql"
        # Build the renamed copy. `bazel-bin/googlesql/public/_objs/
        # analyzer/analyzer.pic.o` -> `public__analyzer__analyzer.pic.o`.
        # Strip `bazel-bin/googlesql/` prefix and `_objs/` markers; the
        # remaining path components are deterministic and unique.
        : > "$obj_list_renamed"
        local src
        while IFS= read -r src; do
            [ -z "$src" ] && continue
            local rel="${src#"$bazel_bin/googlesql/"}"
            # Drop "_objs/" segments to keep the flat name short while
            # preserving the disambiguating target name immediately
            # before the basename.
            local flat
            flat="$(printf '%s\n' "$rel" | sed 's|/_objs/|/|g; s|/|__|g')"
            local dst="$obj_dir/$flat"
            cp "$src" "$dst"
            printf '%s\n' "$dst" >> "$obj_list_renamed"
        done < "$obj_list_raw"
        local renamed_count
        renamed_count="$(wc -l < "$obj_list_renamed")"
        local unique_count
        unique_count="$(sed 's|.*/||' "$obj_list_renamed" | sort -u | wc -l)"
        [ "$renamed_count" = "$unique_count" ] \
            || die "post-rename object basenames still collide: $renamed_count rows, $unique_count unique"
        log "  $out_archive: combining $obj_count objects (renamed for unique basenames)"
        # `ar rcsD` (D = deterministic mode) zeroes timestamps / uids /
        # gids in the archive header so subsequent rebuilds with the same
        # inputs produce byte-identical archives (matches the manifest's
        # SHA-256 reproducibility intent in `docs/dev/googlesql-prebuilt/
        # manifest.md`).
        rm -f "$out_archive"
        xargs -a "$obj_list_renamed" ar rcsD "$out_archive"
        ranlib "$out_archive"
    }

    extract_to_combined "$REPO_STAGE/lib/libgooglesql.a" main
    extract_to_combined "$REPO_STAGE/lib/libgooglesql_protos.a" proto

    # Bundle ICU + farmhash object code into `libgooglesql.a`.
    #
    # Why bundle these specific deps and not, e.g., abseil / protobuf:
    # ICU and farmhash are NOT BCR-resolvable to the versions GoogleSQL
    # actually links against. GoogleSQL fetches them via its own
    # `googlesql/bazel/http_archive_deps.bzl` extension (ICU 76.1 from
    # the unicode.org source tarball with a custom BUILD wrapper;
    # farmhash from an upstream commit, also with a custom BUILD).
    # The closest BCR module — `icu 78.2` — uses different namespace
    # versioning (`icu_78::` vs the producer's `icu_76::`) and would
    # not satisfy the consumer's link. Bundling these objects into
    # the same `libgooglesql.a` removes the consumer's burden of
    # vendoring an extension.
    #
    # This violates the original compatibility-surface default of
    # `bundled_thirdparty_deps = []` (see
    # `docs/dev/googlesql-prebuilt/headers-and-libraries.md` and
    # `manifest.md`). The compatibility-surface docs have been updated in
    # the same change set to allow ICU + farmhash bundling; the
    # manifest's `bundled_thirdparty_deps` field below reports the
    # bundled versions so the safety-gate parity check can diff source
    # vs. prebuilt linkage.
    #
    # Abseil, Protobuf, gRPC, BoringSSL, RE2, GoogleTest remain
    # consumer-resolved through Bzlmod (`bazel_dep` in the prebuilt
    # repo's `MODULE.bazel`); the consumer's `MODULE.bazel` pins them
    # to the same versions the producer linked against.
    log "bundling ICU + farmhash objects into $REPO_STAGE/lib/libgooglesql.a"
    local thirdparty_dir="$tmpdir/_thirdparty"
    local thirdparty_list="$tmpdir/_thirdparty.list"
    mkdir -p "$thirdparty_dir"
    : > "$thirdparty_list"

    # ICU 76 archives: foreign_cc copies them into
    # `bazel-bin/external/<repo>/icu/lib/lib*.a`. The consumer needs
    # `uc` (common), `i18n` (collation, regex, format), and `data` (the
    # binary tables those depend on). `io`/`tu`/`test` are not reached
    # by the googlesql functions we ship.
    local icu_lib_dir
    icu_lib_dir="$(find "$bazel_bin/external" -maxdepth 5 -type d \
        -path '*~icu/icu/lib' 2>/dev/null | head -1)"
    [ -d "$icu_lib_dir" ] \
        || die "ICU lib dir not found under bazel-bin/external (looked for */~icu/icu/lib)"
    log "  ICU archives sourced from $icu_lib_dir"
    local icu_lib
    for icu_lib in libicuuc.a libicui18n.a libicudata.a; do
        [ -f "$icu_lib_dir/$icu_lib" ] \
            || die "ICU archive missing: $icu_lib_dir/$icu_lib"
        # Extract into a per-archive subdir to keep `ar x`'s output
        # contained and predictable. ICU's .a archives mix `.ao` member
        # names (foreign_cc CMake convention for libicuuc / libicui18n)
        # and a plain `.o` for the data table (`icudt76l_dat.o` in
        # libicudata.a — the giant ~30 MiB symbol table that ICU's
        # runtime requires for collation / locale / format data).
        # Both are ELF relocatable objects; the extension is just
        # ICU's build convention.
        local sub="$thirdparty_dir/icu_${icu_lib%.a}"
        mkdir -p "$sub"
        (cd "$sub" && ar x "$icu_lib_dir/$icu_lib")
        # Flatten with an `_thirdparty_icu76__` prefix to namespace the
        # objects away from googlesql `.pic.o` basenames (e.g. ICU's
        # `format.ao` vs googlesql's `format.pic.o` would collide
        # otherwise — see the `extract_to_combined` collision note).
        # Match BOTH `.ao` (libicuuc/libicui18n members) and `.o`
        # (libicudata's single `icudt76l_dat.o`); without the `.o`
        # match the ICU data symbol `icudt76_dat` ends up undefined
        # at consumer link time.
        local obj
        for obj in "$sub"/*.ao "$sub"/*.o; do
            [ -f "$obj" ] || continue
            local base
            base="$(basename "$obj")"
            local dst="$thirdparty_dir/_thirdparty_icu76__$base"
            mv "$obj" "$dst"
            printf '%s\n' "$dst" >> "$thirdparty_list"
        done
        rm -rf "$sub"
    done

    # Everything else under `bazel-bin/external/_main~googlesql_http_archive_deps~*`
    # is a non-BCR external repo fetched by GoogleSQL's own
    # `http_archive_deps.bzl` extension (farmhash, differential-privacy
    # core C++ + protos, cephes inverse_gaussian, etc.). Their `.pic.o`
    # objects need to be bundled into `libgooglesql.a` for the same
    # reason as ICU + farmhash: they cannot be `bazel_dep`-resolved by
    # the consumer because they don't exist in BCR. Walk every `.pic.o`
    # under those repos and rename with a `_thirdparty_<repo>__<flat>`
    # prefix to avoid basename collisions with googlesql's own
    # `.pic.o` (see the `extract_to_combined` collision note).
    #
    # Excludes ICU (handled above; uses `.ao` archives extracted from
    # `lib/lib*.a` rather than walking `_objs/`).
    local extdep_root
    extdep_root="$bazel_bin/external"
    # Find every repo dir matching the http_archive_deps namespace.
    local repo_dir
    while IFS= read -r repo_dir; do
        [ -z "$repo_dir" ] && continue
        local repo_basename
        repo_basename="$(basename "$repo_dir")"
        # Strip `_main~googlesql_http_archive_deps~` prefix → e.g.
        # `com_google_differential_privacy`.
        local short_repo="${repo_basename#_main~googlesql_http_archive_deps~}"
        # ICU was handled separately above (foreign_cc / .ao layout).
        [ "$short_repo" = "icu" ] && continue
        # Walk every `.pic.o` under the repo, rename with a unique
        # `_thirdparty_<short_repo>__<path-flattened>` name so basenames
        # never collide. (`differential-privacy` ships `util.pic.o`,
        # `summary.pb.pic.o`, etc.; without the flatten the .pic.o
        # basenames will collide both with each other and with
        # googlesql's `_objs/util/util.pic.o`.)
        local obj
        while IFS= read -r obj; do
            [ -z "$obj" ] && continue
            local rel="${obj#"$repo_dir/"}"
            local flat
            flat="$(printf '%s\n' "$rel" | sed 's|/_objs/|/|g; s|/|__|g')"
            local dst="$thirdparty_dir/_thirdparty_${short_repo}__$flat"
            cp "$obj" "$dst"
            printf '%s\n' "$dst" >> "$thirdparty_list"
        done < <(find "$repo_dir" -type f -name '*.pic.o' 2>/dev/null)
    done < <(find "$extdep_root" -maxdepth 1 -type d \
                 -name '_main~googlesql_http_archive_deps~*' 2>/dev/null)

    local thirdparty_count
    thirdparty_count="$(wc -l < "$thirdparty_list")"
    log "  appending $thirdparty_count third-party objects to libgooglesql.a"
    # Validate basenames are unique across the bundled set (defense
    # against future http_archive_deps additions whose `.pic.o` paths
    # collapse to the same flat name).
    local unique_count
    unique_count="$(sed 's|.*/||' "$thirdparty_list" | sort -u | wc -l)"
    [ "$thirdparty_count" = "$unique_count" ] \
        || die "third-party object basenames collide ($thirdparty_count rows, $unique_count unique)"
    xargs -a "$thirdparty_list" ar rcsD "$REPO_STAGE/lib/libgooglesql.a"
    ranlib "$REPO_STAGE/lib/libgooglesql.a"

    # Cleanup tmpdir explicitly so the EXIT trap doesn't double-rm.
    rm -rf "$tmpdir"
    cleanup_tmpdir=""
    trap '(cd "'"$GOOGLESQL_SRC"'" && bazel shutdown 2>/dev/null) || true' EXIT INT TERM
}

if [ "$MODE" = "fixture" ]; then
    stage_fixture
else
    stage_bazel
fi

# ---------------------------------------------------------------------------
# Wrapper BUILD.bazel + MODULE.bazel
# ---------------------------------------------------------------------------

log "writing root BUILD.bazel from template"
cp "$TEMPLATES_DIR/BUILD.bazel" "$REPO_STAGE/BUILD.bazel"

log "writing MODULE.bazel with version substitutions"
python3 - <<EOF >"$REPO_STAGE/MODULE.bazel"
import re, sys
text = open("$TEMPLATES_DIR/MODULE.bazel.tmpl").read()
text = text.replace("{{ARTIFACT_VERSION}}", "$ARTIFACT_VERSION")
text = text.replace("{{ABSEIL_VERSION}}", "$ABSEIL_VERSION")
text = text.replace("{{PROTOBUF_VERSION}}", "$PROTOBUF_VERSION")
text = text.replace("{{GRPC_VERSION}}", "$GRPC_VERSION")
text = text.replace("{{GOOGLETEST_VERSION}}", "$GOOGLETEST_VERSION")
text = text.replace("{{RE2_VERSION}}", "$RE2_VERSION")
text = text.replace("{{BORINGSSL_VERSION}}", "$BORINGSSL_VERSION")
text = text.replace("{{GOOGLEAPIS_VERSION}}", "$GOOGLEAPIS_VERSION")
text = text.replace("{{GOOGLEAPIS_CC_VERSION}}", "$GOOGLEAPIS_CC_VERSION")
sys.stdout.write(text)
EOF

log "generating per-package wrapper BUILD.bazel files via wrapper_writer.py"
python3 "$WRAPPER_WRITER" --repo-root "$REPO_STAGE"

# ---------------------------------------------------------------------------
# Manifest
# ---------------------------------------------------------------------------

BUILD_TIMESTAMP="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
MANIFEST_CONFIG="$OUT_DIR/manifest_config.json"

# Determine the GoogleSQL module_version from the sibling MODULE.bazel.
# It is the strict-semver-aliased form (2026.1.1), distinct from the
# upstream_tag (2026.01.1).
GS_MODULE_VERSION="$(grep -E '^\s*version = ' "$GOOGLESQL_SRC/MODULE.bazel" \
    | head -1 \
    | sed -E 's/.*"([^"]+)".*/\1/')"
[ -n "$GS_MODULE_VERSION" ] || die "could not parse googlesql module version"
# Fall back to upstream_tag heuristic: the leading-zero form is the tag
# we used to check out the source. Compatibility-surface docs pin 2026.01.1.
GS_UPSTREAM_TAG="2026.01.1"

cat > "$MANIFEST_CONFIG" <<EOF
{
  "artifact_version": "$ARTIFACT_VERSION",
  "googlesql": {
    "module_version": "$GS_MODULE_VERSION",
    "upstream_tag": "$GS_UPSTREAM_TAG",
    "commit": "$GOOGLESQL_COMMIT",
    "repo_url": "https://github.com/google/googlesql",
    "patches": []
  },
  "emulator": {
    "min_commit": "$EMULATOR_MIN_COMMIT",
    "max_commit": null
  },
  "platform": {
    "os": "linux",
    "arch": "amd64",
    "libc": "$LIBC_VERSION",
    "cxx_abi": "cxx11"
  },
  "toolchain": {
    "compiler": "clang",
    "compiler_version": "$CLANG_VERSION",
    "bazel_version": "$BAZEL_VERSION",
    "cflags": ["-O2", "-fPIC", "-fno-omit-frame-pointer", "-DNDEBUG"],
    "linkflags": []
  },
  "producer": {
    "workflow": "$WORKFLOW_ID",
    "run_id": "$RUN_ID",
    "build_timestamp": "$BUILD_TIMESTAMP",
    "host_os_release": "$HOST_OS_RELEASE"
  },
  "bundled_thirdparty_deps": [
    "icu@${ICU_VERSION}",
    "farmhash@${FARMHASH_COMMIT}",
    "differential-privacy@${DIFFERENTIAL_PRIVACY_VERSION}"
  ]
}
EOF

log "writing manifest.json via manifest_writer.py"
python3 "$MANIFEST_WRITER" \
    --repo-root "$REPO_STAGE" \
    --config "$MANIFEST_CONFIG"

# Validate the just-written manifest against the closed schema as a
# self-check. Refuses to proceed if the producer somehow emitted an
# invalid manifest (defense-in-depth — the same validator runs in
# verify.sh, but failing here gives a cleaner diagnostic).
python3 "$MANIFEST_WRITER" --validate-only "$REPO_STAGE/manifest.json"

# ---------------------------------------------------------------------------
# Tarball
# ---------------------------------------------------------------------------

if [ -z "$TARBALL_NAME" ]; then
    TARBALL_NAME="googlesql-prebuilt-linux-amd64-clang18-${SHORT_SHA}-v${ARTIFACT_VERSION}.tar.gz"
fi
TARBALL_PATH="$OUT_DIR/$TARBALL_NAME"

log "writing tarball at $TARBALL_PATH"
# Deterministic tar: sorted file order, no owner/group, fixed mtime.
# --mtime, --owner, --group, --sort, --numeric-owner are all GNU-tar
# specific; CI runs Linux so this is fine.
tar \
    --create \
    --gzip \
    --sort=name \
    --owner=0 --group=0 --numeric-owner \
    --mtime="$BUILD_TIMESTAMP" \
    -C "$OUT_DIR" \
    -f "$TARBALL_PATH" \
    "$REPO_NAME"

TARBALL_SHA="$(sha256sum "$TARBALL_PATH" | awk '{print $1}')"
echo "$TARBALL_SHA  $TARBALL_NAME" > "$TARBALL_PATH.sha256"

# Copy the manifest out next to the tarball so consumers (or a release
# asset uploader) can fetch it without downloading the tarball.
SIDECAR_MANIFEST="$OUT_DIR/${TARBALL_NAME%.tar.gz}.manifest.json"
cp "$REPO_STAGE/manifest.json" "$SIDECAR_MANIFEST"

log "tarball SHA-256: $TARBALL_SHA"
log "wrote sidecar manifest: $SIDECAR_MANIFEST"
log "package complete:"
log "  $TARBALL_PATH"
log "  $TARBALL_PATH.sha256"
log "  $SIDECAR_MANIFEST"

# Print machine-parsable summary on stdout for the GitHub Actions step
# to capture into outputs.
printf 'TARBALL_PATH=%s\n' "$TARBALL_PATH"
printf 'TARBALL_NAME=%s\n' "$TARBALL_NAME"
printf 'TARBALL_SHA256=%s\n' "$TARBALL_SHA"
printf 'SIDECAR_MANIFEST=%s\n' "$SIDECAR_MANIFEST"
printf 'GOOGLESQL_COMMIT=%s\n' "$GOOGLESQL_COMMIT"
printf 'GOOGLESQL_SHORT_SHA=%s\n' "$SHORT_SHA"
printf 'ARTIFACT_VERSION=%s\n' "$ARTIFACT_VERSION"
