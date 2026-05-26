#!/usr/bin/env bash
# version_gen.sh
#
# Substitutes EMULATOR_VERSION / EMULATOR_COMMIT / EMULATOR_BUILD_DATE
# into version.cc.tmpl, writing the result to stdout. Used by the
# `:version_cc` genrule in `BUILD.bazel`; reads its inputs from the
# action environment so the goreleaser-equivalent
# `--action_env=EMULATOR_VERSION=…` flags set by
# `task emulator:build-engine:bazel` and `.github/workflows/release.yml`
# flow straight through to the generated C++ TU.
#
# Defaults mirror the Go gateway's defaults
# (`binaries/gateway_main/main.go`): a plain `bazel build` with no
# stamping yields `dev` / `none` / `unknown`, signalling to operators
# that the binary is not from a release archive.
#
# Single positional argument: path to the .tmpl source. Output goes
# to stdout so the genrule can redirect into Bazel's $@.
#
# This script is intentionally portable to dash/bash/zsh -- no arrays,
# no [[ ]], no process substitution -- because Bazel's hermetic
# toolchain on some hosts swaps the system /bin/sh underneath us.

set -eu

if [ "$#" -ne 1 ]; then
  echo "usage: version_gen.sh <path/to/version.cc.tmpl>" >&2
  exit 2
fi

tmpl="$1"
if [ ! -r "$tmpl" ]; then
  echo "version_gen.sh: cannot read template: $tmpl" >&2
  exit 2
fi

version="${EMULATOR_VERSION:-dev}"
commit="${EMULATOR_COMMIT:-none}"
build_date="${EMULATOR_BUILD_DATE:-unknown}"

# Reject embedded quotes / backslashes so a malicious env var can't
# break out of the generated string literal. The release surface is
# CI-controlled (the values come from `git describe` + `git rev-parse`
# + GitHub Actions' `github.ref_name`), but defensive escaping costs
# nothing and prevents an accidental string-break if someone later
# threads a free-form `--build-arg` through.
for v in "$version" "$commit" "$build_date"; do
  case "$v" in
    *'"'*|*\\*)
      echo "version_gen.sh: refusing to embed value containing quote or backslash: $v" >&2
      exit 3
      ;;
  esac
done

sed \
  -e "s|@EMULATOR_VERSION@|${version}|g" \
  -e "s|@EMULATOR_COMMIT@|${commit}|g" \
  -e "s|@EMULATOR_BUILD_DATE@|${build_date}|g" \
  "$tmpl"
