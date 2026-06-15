#!/usr/bin/env bash
# render_cpp_html.sh — render an LCOV tracefile to HTML via genhtml, or
# write a placeholder page when the tracefile is empty or summary-only.
#
# Bazel's combined LCOV report often lists SF records with LH/LF summaries
# but no per-line DA records (or reports LF:0 everywhere when instrumentation
# did not produce line data). lcov 2.x genhtml then exits with
# "unexpected empty worklist??" even when --ignore-errors empty,source is set.
# The publish pipeline still has aggregate percentages from summary.json; this
# script keeps html/cpp/index.html present so gh-pages links do not 404.
set -eo pipefail

if [ "$#" -ne 2 ]; then
  echo "usage: render_cpp_html.sh <lcov.dat> <output-dir>" >&2
  exit 2
fi

lcov_path="$1"
out_dir="$2"

if [ ! -f "$lcov_path" ]; then
  echo "render_cpp_html.sh: tracefile not found: $lcov_path" >&2
  exit 1
fi

if ! command -v genhtml >/dev/null 2>&1; then
  echo "render_cpp_html.sh: genhtml not on PATH; install the lcov package." >&2
  exit 1
fi

mkdir -p "$out_dir"
root="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
# llvm-cov emits SF paths under /proc/self/cwd/...; remap to the workspace so
# genhtml can open first-party sources. Ignore missing external/googlesql headers.
if genhtml --ignore-errors empty,source --synthesize-missing --quiet \
    --substitute "s|^/proc/self/cwd/|${root}/|" \
    --output-directory "$out_dir" "$lcov_path"; then
  exit 0
fi

echo "render_cpp_html.sh: genhtml could not render $lcov_path; writing placeholder report." >&2
rm -rf "${out_dir:?}"/*
cat > "$out_dir/index.html" <<'HTML'
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <title>C++ coverage (summary only)</title>
    <style>
      body { font-family: system-ui, sans-serif; max-width: 720px;
             margin: 2rem auto; padding: 0 1rem; line-height: 1.5; }
      code { background: #f4f4f4; padding: 0 0.25rem; border-radius: 3px; }
    </style>
  </head>
  <body>
    <h1>C++ engine coverage</h1>
    <p>
      The Bazel combined LCOV tracefile for this run did not contain
      line-level coverage records that <code>genhtml</code> can render
      (empty or summary-only data). Aggregate percentages are still
      available in <a href="../summary.json"><code>summary.json</code></a>
      and the README badge endpoint
      <a href="../badge-cpp.json"><code>badge-cpp.json</code></a>.
    </p>
    <p>
      Re-run <code>task coverage:html</code> locally after
      <code>task bazel:coverage</code> if you need a browsable report.
    </p>
  </body>
</html>
HTML
