#include "backend/engine/disposition.h"

#include <set>
#include <string>

#include "absl/strings/string_view.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace {

// The full closed set of Disposition values. Centralised so each
// test below can iterate over the same vector instead of
// hand-listing the enum members. Adding a new enum value here AND
// updating `kDispositionCount` triggers every assertion below to
// exercise the new value as well.
const std::vector<Disposition>& AllDispositions() {
  static const auto* kAll = new std::vector<Disposition>{
      Disposition::kDuckdbNative,
      Disposition::kDuckdbRewrite,
      Disposition::kDuckdbUdf,
      Disposition::kSemanticExecutor,
      Disposition::kControlOp,
      Disposition::kLocalStub,
      Disposition::kUnsupported,
  };
  return *kAll;
}

TEST(DispositionTest, ToStringRoundsTripCanonicalSpellings) {
  // The spellings here must match the YAML disposition column AND
  // the SHAPE_TRACKER.md `status` column verbatim; the parity
  // checker (`tools/check_disposition_parity`) consumes the same
  // strings.
  EXPECT_EQ(DispositionToString(Disposition::kDuckdbNative), "duckdb_native");
  EXPECT_EQ(DispositionToString(Disposition::kDuckdbRewrite), "duckdb_rewrite");
  EXPECT_EQ(DispositionToString(Disposition::kDuckdbUdf), "duckdb_udf");
  EXPECT_EQ(DispositionToString(Disposition::kSemanticExecutor),
            "semantic_executor");
  EXPECT_EQ(DispositionToString(Disposition::kControlOp), "control_op");
  EXPECT_EQ(DispositionToString(Disposition::kLocalStub), "local_stub");
  EXPECT_EQ(DispositionToString(Disposition::kUnsupported), "unsupported");
}

TEST(DispositionTest, ToStringIsTotalAndUnique) {
  // Every enum value maps to a non-empty, distinct lowercase
  // identifier-safe string. An accidental duplicate would silently
  // make `tools/check_disposition_parity` treat two routes as the
  // same.
  std::set<std::string> seen;
  for (Disposition d : AllDispositions()) {
    absl::string_view s = DispositionToString(d);
    SCOPED_TRACE(s);
    EXPECT_FALSE(s.empty());
    EXPECT_NE(s, "unknown");
    seen.insert(std::string(s));
  }
  EXPECT_EQ(seen.size(), AllDispositions().size());
}

TEST(DispositionTest, EnumCountMatchesAllDispositions) {
  // `kDispositionCount` is the build-time anchor the parity checker
  // and the generated `node_dispositions_table.inc` test consult.
  // Whenever a new disposition lands, this assertion is the canary
  // that fires first; bumping `kDispositionCount` then forces
  // updates everywhere else that iterates over the closed set.
  EXPECT_EQ(static_cast<int>(AllDispositions().size()), kDispositionCount);
}

}  // namespace
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
