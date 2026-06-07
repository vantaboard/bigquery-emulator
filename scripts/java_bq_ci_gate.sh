#!/usr/bin/env bash
# CI gate for Java third-party live ITs: fail on any non-allowlisted Failsafe failure.
#
# Usage:
#   java_bq_ci_gate.sh <allowlist_csv> <target_dir>...
#
# Exit 0 when every module produced Failsafe reports and all failing ITs (if any)
# are on JAVA_BQ_ALLOW_FAILING_ITS. Exit 1 on missing reports or unexpected failures.

set -euo pipefail

allow_raw="${1:?allowlist_csv required}"
shift
if [ "$#" -eq 0 ]; then
  echo "java-bq-ci-gate: at least one target_dir required" >&2
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

unexpected=""
allowlisted=""
missing_reports=""

for target in "$@"; do
  reports="${target}/target/failsafe-reports"
  if [ ! -d "$reports" ]; then
    missing_reports="${missing_reports}${missing_reports:+ }${target}"
    continue
  fi
  found_report=0
  for f in "$reports"/TEST-*.xml; do
    [ -f "$f" ] || continue
    found_report=1
    if grep -qE '<(failure|error)\b' "$f" 2>/dev/null; then
      bn=$(basename "$f" .xml)
      it_short="${bn##*.}"
      it_short="${it_short%%-sponge_log}"
      on_allowlist=0
      for a in $allow; do
        if [ "$it_short" = "$a" ]; then
          on_allowlist=1
          allowlisted="${allowlisted}${allowlisted:+ }${it_short}@${target}"
          break
        fi
      done
      if [ "$on_allowlist" -eq 0 ]; then
        unexpected="${unexpected}${unexpected:+ }${it_short}@${target}"
      fi
    fi
  done
  if [ "$found_report" -eq 0 ]; then
    missing_reports="${missing_reports}${missing_reports:+ }${target}"
  fi
done

if [ -n "$unexpected" ]; then
  echo "::error::Non-allowlisted Java IT failures: ${unexpected}" >&2
  exit 1
fi

if [ -n "$missing_reports" ]; then
  echo "::error::Missing Failsafe reports (Maven may not have completed): ${missing_reports}" >&2
  exit 1
fi

if [ -n "$allowlisted" ]; then
  echo "::notice::Only allowlisted Java IT failures (expected until gRPC backends land): ${allowlisted}"
fi

echo "java-bq-ci-gate: OK"
exit 0
