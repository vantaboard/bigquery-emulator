#!/usr/bin/env bash
# Check that every failing Failsafe IT in a Maven snippets module is allowlisted.
#
# Usage:
#   java_bq_check_allowlisted_failures.sh <target_dir> <allowlist_csv>
#
# Exit 0 when target/failsafe-reports lists only allowlisted IT failures.
# Exit 1 when the allowlist is empty, reports are missing, no failures were
# found, or any failing IT is not on the allowlist.

set -euo pipefail

target="${1:?target_dir required}"
allow_raw="${2:-}"

allow_compact="$(printf '%s' "$allow_raw" | tr -d '[:space:],')"
if [ -z "$allow_compact" ]; then
  exit 1
fi

reports="${target}/target/failsafe-reports"
if [ ! -d "$reports" ]; then
  exit 1
fi

allow=""
IFS=','
for a in $allow_raw; do
  a="$(printf '%s' "$a" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
  if [ -n "$a" ]; then
    allow="${allow}${allow:+ }${a}"
  fi
done
unset IFS

failed_its=""
for f in "$reports"/TEST-*.xml; do
  [ -f "$f" ] || continue
  if grep -qE '<(failure|error)\b' "$f" 2>/dev/null; then
    bn=$(basename "$f" .xml)
    it_short="${bn##*.}"
    it_short="${it_short%%-sponge_log}"
    failed_its="${failed_its}${failed_its:+ }${it_short}"
  fi
done

if [ -z "$failed_its" ]; then
  exit 1
fi

for it in $failed_its; do
  found=0
  for a in $allow; do
    if [ "$it" = "$a" ]; then
      found=1
      break
    fi
  done
  if [ "$found" -eq 0 ]; then
    echo "java-bigquery-tests: unexpected failing IT ${it} in ${target} (not in JAVA_BQ_ALLOW_FAILING_ITS)" >&2
    exit 1
  fi
done

echo "java-bigquery-tests: ${target} failed only allowlisted ITs (${failed_its})" >&2
exit 0
