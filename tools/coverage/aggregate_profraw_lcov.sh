#!/usr/bin/env bash
# aggregate_profraw_lcov.sh — rebuild Bazel's combined LCOV after coverage runs.
#
# Bazel's collect_cc_coverage.sh merges every *.profraw in a test's coverage dir
# without llvm-profdata's -sparse flag. On GoogleSQL-linked cc_test targets that
# link hundreds of instrumented objects, the default merge fails with "No
# executable lines" and leaves per-test coverage.dat empty (LF:0/LH:0 combined).
#
# Run bazel coverage with --test_env=LCOV_MERGER=/usr/bin/true (see .bazelrc) so
# profraw files are retained under <bazel-testlogs>/<pkg>/<test>/_coverage/.
# This script walks those dirs, sparse-merges each test's profraws, exports lcov
# via llvm-cov for the matching cc_test binary, and lcov -a combines the result.
set -eo pipefail

if [ "$#" -ne 1 ]; then
  echo "usage: aggregate_profraw_lcov.sh <output.dat>" >&2
  exit 2
fi

out="$1"
root="$(git rev-parse --show-toplevel)"
cd "$root"

LLVM_PROFDATA="${LLVM_PROFDATA:-llvm-profdata}"
LLVM_COV="${LLVM_COV:-llvm-cov}"
if ! command -v "$LLVM_PROFDATA" >/dev/null 2>&1; then
  echo "aggregate_profraw_lcov.sh: $LLVM_PROFDATA not on PATH" >&2
  exit 1
fi
if ! command -v "$LLVM_COV" >/dev/null 2>&1; then
  echo "aggregate_profraw_lcov.sh: $LLVM_COV not on PATH" >&2
  exit 1
fi
if ! command -v lcov >/dev/null 2>&1; then
  echo "aggregate_profraw_lcov.sh: lcov not on PATH" >&2
  exit 1
fi

# Prefer the workspace bazel-testlogs symlink so aggregation still works when
# the Bazel daemon exits after a long coverage run (CI hit this after ~80 min).
resolve_testlogs() {
  local candidate=""
  if [ -e "$root/bazel-testlogs" ]; then
    candidate="$(readlink -f "$root/bazel-testlogs" 2>/dev/null || true)"
    if [ -n "$candidate" ] && [ -d "$candidate" ]; then
      echo "$candidate"
      return 0
    fi
  fi
  if command -v bazel >/dev/null 2>&1; then
    local bazel_args=()
    if [ -n "${BAZEL_CONFIG:-}" ]; then
      bazel_args+=(--config="$BAZEL_CONFIG")
    fi
    candidate="$(bazel "${bazel_args[@]}" info bazel-testlogs 2>/dev/null || true)"
    if [ -n "$candidate" ] && [ -d "$candidate" ]; then
      echo "$candidate"
      return 0
    fi
  fi
  return 1
}

if ! testlogs="$(resolve_testlogs)"; then
  echo "aggregate_profraw_lcov.sh: could not resolve bazel-testlogs (symlink missing and bazel info failed)" >&2
  exit 1
fi

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT
combined=""
merged_any=false

# testlogs layout: <pkg/path>/<test_name>/_coverage/*.profraw
while IFS= read -r covdir; do
  shopt -s nullglob
  profraws=("$covdir"/*.profraw)
  shopt -u nullglob
  if [ ${#profraws[@]} -eq 0 ]; then
    continue
  fi

  relpath="${covdir#"$testlogs"/}"
  relpath="${relpath%/_coverage}"
  test_bin="bazel-bin/$relpath"
  if [ ! -x "$test_bin" ]; then
    echo "aggregate_profraw_lcov.sh: skip $covdir (binary missing: $test_bin)" >&2
    continue
  fi

  profdata="$tmpdir/$(echo -n "$relpath" | md5sum | cut -c1-16).profdata"
  if ! "$LLVM_PROFDATA" merge -sparse -output="$profdata" "${profraws[@]}" 2>/dev/null; then
    echo "aggregate_profraw_lcov.sh: profdata merge failed for $relpath" >&2
    continue
  fi

  per_test="$tmpdir/$(basename "$relpath").dat"
  if ! "$LLVM_COV" export -instr-profile="$profdata" -object "$test_bin" \
      -format=lcov \
      --ignore-filename-regex='(^external/|.*/external/|/virtual_includes/)' \
      "$test_bin" >"$per_test" 2>/dev/null; then
    echo "aggregate_profraw_lcov.sh: llvm-cov export failed for $relpath" >&2
    continue
  fi
  if [ ! -s "$per_test" ]; then
    continue
  fi

  if [ -z "$combined" ]; then
    combined="$per_test"
  else
    next="$tmpdir/combined-$(basename "$relpath").dat"
    lcov -q -a "$combined" -a "$per_test" -o "$next" 2>/dev/null || continue
    combined="$next"
  fi
  merged_any=true
done < <(find "$testlogs" -type d -name _coverage 2>/dev/null)

if [ "$merged_any" = false ] || [ -z "$combined" ] || [ ! -s "$combined" ]; then
  echo "aggregate_profraw_lcov.sh: no per-test lcov data found under $testlogs" >&2
  exit 1
fi

mkdir -p "$(dirname "$out")"
cp "$combined" "$out"
echo "aggregate_profraw_lcov.sh: wrote $out"
