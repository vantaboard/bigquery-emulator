#!/usr/bin/env bash
# Run a Bazel-backed Task target once; on CacheNotFoundException from a
# poisoned setup-bazel disk cache, wipe the cache dirs and retry exactly once.
#
# Usage: bazel_task_with_disk_cache_retry.sh task lint:cpp:test
set -euo pipefail

if [ "$#" -eq 0 ]; then
  echo "usage: $0 <command...>" >&2
  exit 2
fi

log="$(mktemp)"
cleanup() { rm -f "$log"; }
trap cleanup EXIT

clear_bazel_disk_caches() {
  # setup-bazel (build-engine) injects --disk_cache=$HOME/.cache/bazel-disk;
  # repo .bazelrc also names .cache/bazel-disk under the workspace root.
  if [ -d "${HOME}/.cache/bazel-disk" ]; then
    echo "Clearing ${HOME}/.cache/bazel-disk"
    rm -rf "${HOME}/.cache/bazel-disk"
  fi
  if [ -d .cache/bazel-disk ]; then
    echo "Clearing ${PWD}/.cache/bazel-disk"
    rm -rf .cache/bazel-disk
  fi
  timeout 60 bazel shutdown 2>/dev/null || true
}

run_cmd() {
  set +e
  "$@" 2>&1 | tee "$log"
  rc=${PIPESTATUS[0]}
  set -e
  return "$rc"
}

if run_cmd "$@"; then
  exit 0
fi
rc=$?

if grep -qE 'CacheNotFoundException|Missing digest:' "$log"; then
  echo "::warning::Bazel disk cache poisoned (CacheNotFoundException); clearing disk cache and retrying once"
  clear_bazel_disk_caches
  if run_cmd "$@"; then
    exit 0
  fi
fi

exit "$rc"
