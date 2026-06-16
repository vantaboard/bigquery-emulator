#!/usr/bin/env bash
# aggregate_profraw_lcov.sh — rebuild Bazel's combined LCOV after coverage runs.
#
# Bazel's collect_cc_coverage.sh merges every *.profraw in a test's coverage dir
# without llvm-profdata's -sparse flag. On GoogleSQL-linked cc_test targets that
# link hundreds of instrumented objects, the default merge fails with "No
# executable lines" and leaves per-test coverage.dat empty (LF:0/LH:0 combined).
#
# Run bazel coverage with --test_env=LCOV_MERGER=/usr/bin/true (see .bazelrc) so
# profraw files are retained under the per-test coverage dir. Bazel may place
# that tree at testlogs/_coverage/<pkg>/<test> or testlogs/<pkg>/<test>/_coverage
# (or test/coverage during the run). This script walks those layouts, sparse-merges
# each test's profraws, exports lcov via llvm-cov for the matching cc_test binary,
# and lcov -a combines the result.
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

covdir_to_relpath() {
  local covdir="$1"
  local rel="${covdir#"$testlogs"/}"
  if [[ "$rel" == _coverage/* ]]; then
    rel="${rel#_coverage/}"
  else
    rel="${rel%/_coverage}"
    rel="${rel%/coverage}"
    rel="${rel%/test}"
  fi
  echo "$rel"
}

lcov_has_hits() {
  local dat="$1"
  [ -s "$dat" ] && grep -q '^DA:[1-9]' "$dat" 2>/dev/null
}

merge_dat() {
  local dat="$1"
  local label="$2"
  if ! lcov_has_hits "$dat"; then
    return 1
  fi
  if [ -z "$combined" ]; then
    combined="$dat"
  else
    local next="$tmpdir/combined-${label}.dat"
    lcov -q -a "$combined" -a "$dat" -o "$next" 2>/dev/null || return 1
    combined="$next"
  fi
  merged_any=true
  return 0
}

merge_profraw_dir() {
  local covdir="$1"
  shopt -s nullglob
  local profraws=("$covdir"/*.profraw)
  shopt -u nullglob
  if [ ${#profraws[@]} -eq 0 ]; then
    return 1
  fi

  local relpath
  relpath="$(covdir_to_relpath "$covdir")"
  local test_bin="bazel-bin/$relpath"
  if [ ! -x "$test_bin" ]; then
    echo "aggregate_profraw_lcov.sh: skip $covdir (binary missing: $test_bin)" >&2
    return 1
  fi

  local profdata="$tmpdir/$(echo -n "$relpath" | md5sum | cut -c1-16).profdata"
  if ! "$LLVM_PROFDATA" merge -sparse -output="$profdata" "${profraws[@]}" 2>/dev/null; then
    echo "aggregate_profraw_lcov.sh: profdata merge failed for $relpath" >&2
    return 1
  fi

  local per_test="$tmpdir/$(basename "$relpath").dat"
  if ! "$LLVM_COV" export -instr-profile="$profdata" -object "$test_bin" \
      -format=lcov \
      --ignore-filename-regex='(^external/|.*/external/|/virtual_includes/)' \
      "$test_bin" >"$per_test" 2>/dev/null; then
    echo "aggregate_profraw_lcov.sh: llvm-cov export failed for $relpath" >&2
    return 1
  fi
  merge_dat "$per_test" "$(basename "$relpath")"
}

# 1) Sparse-merge retained profraws (preferred for large GoogleSQL-linked tests).
declare -A seen_covdirs=()
while IFS= read -r profraw; do
  covdir="$(dirname "$profraw")"
  if [ -n "${seen_covdirs[$covdir]+x}" ]; then
    continue
  fi
  seen_covdirs[$covdir]=1
  merge_profraw_dir "$covdir" || true
done < <(find "$testlogs" -name '*.profraw' -type f 2>/dev/null)

# 2) Fall back to per-test _cc_coverage.dat when Bazel already ran llvm-cov export.
while IFS= read -r cc_dat; do
  merge_dat "$cc_dat" "$(basename "$(dirname "$cc_dat")")" || true
done < <(find "$testlogs" -name '_cc_coverage.dat' -type f 2>/dev/null)

# 3) Per-target coverage.dat (testlogs/<pkg>/<test>/coverage.dat).
while IFS= read -r per_test_dat; do
  case "$per_test_dat" in
    *"/_coverage/"*) continue ;;
  esac
  merge_dat "$per_test_dat" "$(basename "$(dirname "$per_test_dat")")" || true
done < <(find "$testlogs" -mindepth 2 -maxdepth 4 -name 'coverage.dat' -type f 2>/dev/null)

if [ "$merged_any" = false ] || [ -z "$combined" ] || [ ! -s "$combined" ]; then
  local_profraws="$(find "$testlogs" -name '*.profraw' -type f 2>/dev/null | wc -l | tr -d ' ')"
  local_cc_dats="$(find "$testlogs" -name '_cc_coverage.dat' -type f 2>/dev/null | wc -l | tr -d ' ')"
  local_cov_dats="$(find "$testlogs" -name 'coverage.dat' -type f 2>/dev/null | wc -l | tr -d ' ')"
  echo "aggregate_profraw_lcov.sh: no per-test lcov data found under $testlogs" >&2
  echo "aggregate_profraw_lcov.sh: saw ${local_profraws} profraw(s), ${local_cc_dats} _cc_coverage.dat, ${local_cov_dats} coverage.dat" >&2
  exit 1
fi

mkdir -p "$(dirname "$out")"
cp "$combined" "$out"
echo "aggregate_profraw_lcov.sh: wrote $out"
