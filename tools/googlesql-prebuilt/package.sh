#!/usr/bin/env bash
# Package the GoogleSQL prebuilt artifact (Phase 2 producer).
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

BAZEL_VERSION_FILE="$EMULATOR_SRC/.bazelversion"
if [ -r "$BAZEL_VERSION_FILE" ]; then
    BAZEL_VERSION="$(tr -d '[:space:]' < "$BAZEL_VERSION_FILE")"
else
    BAZEL_VERSION="unknown"
fi

# Detect compiler version. The clang in the producer's PATH must match
# the host the artifact will run on. Phase 5 (consume-time validation)
# warns on mismatch, but for now we just record what we saw.
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

# Detect libc / OS release. Phase 1 manifest schema expects glibc-<MAJ.MIN>.
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

# Generate a NOTICE listing the consumer-resolved third-party deps.
# Per Phase 1 doc, these are NOT statically bundled into libgooglesql.a;
# the NOTICE just records what the wrapper cc_library targets `deps=`
# down into so a downstream license auditor knows what to look at.
cat > "$REPO_STAGE/LICENSES/thirdparty-NOTICE" <<EOF
GoogleSQL prebuilt artifact ($REPO_NAME) — third-party dependencies.

The static archives shipped under lib/ contain only object code from
the upstream GoogleSQL repository (https://github.com/google/googlesql,
commit $GOOGLESQL_COMMIT). All of the following libraries are
Bzlmod-resolved by the consumer's MODULE.bazel at link time and are
NOT bundled into lib/libgooglesql.a or lib/libgooglesql_protos.a:

  - abseil-cpp $ABSEIL_VERSION (Apache-2.0)
  - protobuf $PROTOBUF_VERSION (BSD-3-Clause)
  - grpc $GRPC_VERSION (Apache-2.0)

Phase 1 manifest schema (\`bundled_thirdparty_deps\`) reports an empty
list, matching this layout. If a future producer starts bundling any
of these, the manifest field will be updated accordingly.
EOF

# ---------------------------------------------------------------------------
# Stage payload (mode-specific)
# ---------------------------------------------------------------------------

stage_fixture() {
    log "MODE=fixture: copying hand-picked headers from $GOOGLESQL_SRC"
    # The 25 direct headers frozen by Phase 1
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

    # The compatibility surface to build. Phase 1
    # (`headers-and-libraries.md`) says: closure of `:analyzer`,
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
            "${umbrella_targets[@]}"
    )

    local bazel_bin
    bazel_bin="$(cd "$GOOGLESQL_SRC" && bazel info bazel-bin)"
    [ -d "$bazel_bin" ] || die "bazel-bin not found at $bazel_bin"

    log "harvesting hand-written headers from $GOOGLESQL_SRC/googlesql/"
    local pkg
    for pkg in public public/types public/proto public/functions \
               base common resolved_ast; do
        local src_dir="$GOOGLESQL_SRC/googlesql/$pkg"
        local dst_dir="$REPO_STAGE/include/googlesql/$pkg"
        if [ -d "$src_dir" ]; then
            mkdir -p "$dst_dir"
            # `find ... -exec cp` rather than glob so we don't choke on
            # zero-header directories.
            find "$src_dir" -maxdepth 1 -type f -name '*.h' -exec cp {} "$dst_dir/" \;
        fi
    done

    log "harvesting generated .pb.h headers from $bazel_bin/googlesql"
    if [ -d "$bazel_bin/googlesql" ]; then
        find "$bazel_bin/googlesql" -type f -name '*.pb.h' | while read -r genhdr; do
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
        local obj_dir="$tmpdir/$pattern_inc"
        mkdir -p "$obj_dir"
        local idx=0
        # Find every PIC archive under bazel-bin/googlesql. Phase 1
        # decision (headers-and-libraries.md): PIC is enabled and
        # recorded in the manifest's `cflags`.
        local archives
        if [ "$pattern_inc" = "proto" ]; then
            archives="$(find "$bazel_bin/googlesql" -type f -name '*_cc_proto*.pic.a' 2>/dev/null || true)"
        else
            # Exclude proto archives by name pattern.
            archives="$(find "$bazel_bin/googlesql" -type f -name '*.pic.a' \
                ! -name '*_cc_proto*.pic.a' 2>/dev/null || true)"
        fi
        [ -n "$archives" ] || die "no .pic.a files matched for $pattern_inc under $bazel_bin/googlesql"
        local ar_file
        while IFS= read -r ar_file; do
            [ -z "$ar_file" ] && continue
            idx=$((idx + 1))
            local sub="$obj_dir/$idx"
            mkdir -p "$sub"
            # `ar x` extracts into cwd. Use per-archive subdirs to
            # avoid filename collisions (many archives have `foo.o`).
            (cd "$sub" && ar x "$ar_file")
        done <<< "$archives"
        local obj_count
        obj_count="$(find "$obj_dir" -type f -name '*.o' | wc -l)"
        [ "$obj_count" -gt 0 ] || die "extracted zero objects for $pattern_inc"
        log "  $out_archive: combining $obj_count objects from $idx archives"
        # `find -print0 | xargs -0 ar rcs` keeps the order deterministic
        # (find's BFS) and survives spaces in obj paths.
        find "$obj_dir" -type f -name '*.o' -print0 \
            | xargs -0 ar rcs "$out_archive"
        ranlib "$out_archive"
    }

    extract_to_combined "$REPO_STAGE/lib/libgooglesql.a" main
    extract_to_combined "$REPO_STAGE/lib/libgooglesql_protos.a" proto

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
# we used to check out the source. Phase 1 docs pin 2026.01.1.
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
  "bundled_thirdparty_deps": []
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
