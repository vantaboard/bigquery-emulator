// Smoke tests for the link-time-stamped engine version constants.
//
// Hermetic by design: the assertions only check that the symbols are
// defined and that the strings are non-empty. We do NOT assert the
// values themselves because the genrule that emits `version.cc`
// substitutes in environment variables at build time (see
// `binaries/emulator_main/BUILD.bazel`'s `:version_cc` rule), and the
// test must pass for both stamped (`bazel build --action_env=...`)
// and unstamped (`bazel build` default `dev/none/unknown`) builds.
//
// The integration check that a stamped build actually shows the
// stamped values is performed by the `--version` invocation in plan
// 45's verification block (`./bin/emulator_main --version` after
// `task emulator:build-engine:bazel`).

#include "binaries/emulator_main/version.h"

#include <cstring>

#include "gtest/gtest.h"

namespace bigquery_emulator::binaries::emulator_main {
namespace {

TEST(VersionTest, SymbolsAreDefined) {
  ASSERT_NE(kVersion, nullptr);
  ASSERT_NE(kCommit, nullptr);
  ASSERT_NE(kBuildDate, nullptr);
}

TEST(VersionTest, SymbolsAreNonEmpty) {
  EXPECT_GT(std::strlen(kVersion), 0u);
  EXPECT_GT(std::strlen(kCommit), 0u);
  EXPECT_GT(std::strlen(kBuildDate), 0u);
}

}  // namespace
}  // namespace bigquery_emulator::binaries::emulator_main
