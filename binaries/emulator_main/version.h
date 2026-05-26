// Engine-side `--version` source-of-truth.
//
// The three `extern const char* const` symbols below are defined in the
// generated `version.cc` (see the `:version_cc` genrule in
// `BUILD.bazel`). At build time `version_gen.sh` reads
// `EMULATOR_VERSION` / `EMULATOR_COMMIT` / `EMULATOR_BUILD_DATE` from
// the action environment and substitutes them into `version.cc.tmpl`;
// when the env vars are unset (a plain `bazel build` without
// `--action_env=EMULATOR_*`) the script falls back to
// `dev` / `none` / `unknown`, which mirrors the goreleaser-equivalent
// defaults in `binaries/gateway_main/main.go`.
//
// Why a separate TU instead of inlining `constexpr` strings: the values
// have to be assignable at link time (the genrule re-emits `version.cc`
// whenever `--action_env` flips), and `inline constexpr` would force
// the values to be header-baked. The genrule pattern also keeps the
// engine source tree free of generated state: `version.cc` lives under
// `bazel-bin/`, not in `binaries/emulator_main/`.

#ifndef BIGQUERY_EMULATOR_BINARIES_EMULATOR_MAIN_VERSION_H_
#define BIGQUERY_EMULATOR_BINARIES_EMULATOR_MAIN_VERSION_H_

namespace bigquery_emulator::binaries::emulator_main {

// Semantic version of this engine binary. `dev` for unstamped builds;
// `git describe --tags --always --dirty` (or the workflow's `${{
// github.ref_name }}`) for stamped builds.
extern const char* const kVersion;

// Git short sha at build time. `none` for unstamped builds.
extern const char* const kCommit;

// Build date in RFC 3339 UTC (`YYYY-MM-DDTHH:MM:SSZ`). `unknown` for
// unstamped builds.
extern const char* const kBuildDate;

}  // namespace bigquery_emulator::binaries::emulator_main

#endif  // BIGQUERY_EMULATOR_BINARIES_EMULATOR_MAIN_VERSION_H_
