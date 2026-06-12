#!/usr/bin/env bash
# Apply one-time patches to a sibling ../googlesql source checkout so
# bigquery-emulator source-mode Bazel builds match the prebuilt wrapper
# surface (see MODULE.bazel and docs/dev/googlesql-prebuilt/label-inventory.md).
#
# Usage: bash scripts/patch_googlesql_source_checkout.sh [googlesql_root]
set -euo pipefail

root="${1:-../googlesql}"
module_bazel="${root}/MODULE.bazel"
scripting_build="${root}/googlesql/scripting/BUILD"

if [ ! -f "$module_bazel" ]; then
  echo "patch_googlesql_source_checkout: missing ${module_bazel}" >&2
  exit 1
fi

# Gazelle strict semver rejects 2026.01.1 (leading zero in minor).
sed -i 's/version = "2026.01.1"/version = "2026.1.1"/' "$module_bazel"

if [ -f "$scripting_build" ]; then
  python3 - "$scripting_build" <<'PY'
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
text = path.read_text()
needle = 'name = "script_executor"'
if needle not in text:
    raise SystemExit(f"script_executor target not found in {path}")
if 'visibility = ["//visibility:public"]' in text.split(needle, 1)[1].split(")", 1)[0]:
    sys.exit(0)
marker = '    hdrs = [\n        "script_executor.h",\n    ],\n'
insert = marker + '    visibility = ["//visibility:public"],\n'
if marker not in text:
    raise SystemExit(f"unexpected script_executor hdrs block in {path}")
path.write_text(text.replace(marker, insert, 1))
PY
fi

echo "patch_googlesql_source_checkout: ok (${root})"
