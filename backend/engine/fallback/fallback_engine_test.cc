// Unit tests for the FallbackEngine wrapper. The tests use trivial
// in-process `Engine` stubs (no GoogleSQL on the link line) so they
// run fast and stay independent of the analyzer's startup cost.

#include "backend/engine/fallback/fallback_engine.h"

#include <memory>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace fallback {
namespace {

// Stub `AnalyzedQuery` so tests can return a non-null analyzed query
// without going through the analyzer.
class StubAnalyzedQuery : public AnalyzedQuery {
 public:
  explicit StubAnalyzedQuery(schema::TableSchema s) : schema_(std::move(s)) {}
  const schema::TableSchema& output_schema() const override { return schema_; }

 private:
  schema::TableSchema schema_;
};

// Stub `RowSource` so tests can return a non-null row source without
// touching real storage.
class StubRowSource : public RowSource {
 public:
  explicit StubRowSource(schema::TableSchema s) : schema_(std::move(s)) {}
  const schema::TableSchema& schema() const override { return schema_; }
  absl::StatusOr<bool> Next(storage::Row* /*row*/) override { return false; }

 private:
  schema::TableSchema schema_;
};

// Configurable stub engine: each method returns the status configured
// on the stub, with a counter so tests can verify call counts.
class StubEngine : public Engine {
 public:
  absl::Status analyze_status = absl::OkStatus();
  absl::Status dryrun_status = absl::OkStatus();
  absl::Status execute_status = absl::OkStatus();
  std::string tag;

  int analyze_calls = 0;
  int dryrun_calls = 0;
  int execute_calls = 0;

  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& /*request*/,
      ::googlesql::Catalog* /*catalog*/) override {
    ++analyze_calls;
    if (!analyze_status.ok()) return analyze_status;
    schema::TableSchema s;
    schema::ColumnSchema c;
    c.name = tag;
    s.columns.push_back(c);
    return std::unique_ptr<AnalyzedQuery>(new StubAnalyzedQuery(std::move(s)));
  }

  absl::StatusOr<DryRunResult> DryRun(
      const QueryRequest& /*request*/,
      ::googlesql::Catalog* /*catalog*/) override {
    ++dryrun_calls;
    if (!dryrun_status.ok()) return dryrun_status;
    DryRunResult r;
    schema::ColumnSchema c;
    c.name = tag;
    r.schema.columns.push_back(c);
    return r;
  }

  absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& /*request*/,
      ::googlesql::Catalog* /*catalog*/) override {
    ++execute_calls;
    if (!execute_status.ok()) return execute_status;
    schema::TableSchema s;
    schema::ColumnSchema c;
    c.name = tag;
    s.columns.push_back(c);
    return std::unique_ptr<RowSource>(new StubRowSource(std::move(s)));
  }
};

TEST(FallbackEngineTest, AnalyzePrimaryOkSkipsFallback) {
  StubEngine primary, fallback;
  primary.tag = "primary";
  fallback.tag = "fallback";
  FallbackEngine engine(&primary, &fallback);

  QueryRequest req;
  auto result = engine.Analyze(req, nullptr);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ((*result)->output_schema().columns.at(0).name, "primary");
  EXPECT_EQ(primary.analyze_calls, 1);
  EXPECT_EQ(fallback.analyze_calls, 0);
}

TEST(FallbackEngineTest, AnalyzeUnimplementedDelegatesToFallback) {
  StubEngine primary, fallback;
  primary.tag = "primary";
  fallback.tag = "fallback";
  primary.analyze_status = absl::UnimplementedError("nope");
  FallbackEngine engine(&primary, &fallback);

  QueryRequest req;
  auto result = engine.Analyze(req, nullptr);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ((*result)->output_schema().columns.at(0).name, "fallback");
  EXPECT_EQ(primary.analyze_calls, 1);
  EXPECT_EQ(fallback.analyze_calls, 1);
}

TEST(FallbackEngineTest, AnalyzeNonUnimplementedSurfacedUnchanged) {
  StubEngine primary, fallback;
  primary.tag = "primary";
  fallback.tag = "fallback";
  primary.analyze_status = absl::InvalidArgumentError("syntax");
  FallbackEngine engine(&primary, &fallback);

  QueryRequest req;
  auto result = engine.Analyze(req, nullptr);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(primary.analyze_calls, 1);
  EXPECT_EQ(fallback.analyze_calls, 0);
}

TEST(FallbackEngineTest, DryRunUnimplementedDelegatesToFallback) {
  StubEngine primary, fallback;
  primary.tag = "primary";
  fallback.tag = "fallback";
  primary.dryrun_status = absl::UnimplementedError("nope");
  FallbackEngine engine(&primary, &fallback);

  QueryRequest req;
  auto result = engine.DryRun(req, nullptr);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result->schema.columns.at(0).name, "fallback");
  EXPECT_EQ(primary.dryrun_calls, 1);
  EXPECT_EQ(fallback.dryrun_calls, 1);
}

TEST(FallbackEngineTest, ExecuteQueryPrimaryOkSkipsFallback) {
  StubEngine primary, fallback;
  primary.tag = "primary";
  fallback.tag = "fallback";
  FallbackEngine engine(&primary, &fallback);

  QueryRequest req;
  auto result = engine.ExecuteQuery(req, nullptr);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ((*result)->schema().columns.at(0).name, "primary");
  EXPECT_EQ(primary.execute_calls, 1);
  EXPECT_EQ(fallback.execute_calls, 0);
}

TEST(FallbackEngineTest, ExecuteQueryUnimplementedDelegatesToFallback) {
  StubEngine primary, fallback;
  primary.tag = "primary";
  fallback.tag = "fallback";
  primary.execute_status = absl::UnimplementedError("nope");
  FallbackEngine engine(&primary, &fallback);

  QueryRequest req;
  auto result = engine.ExecuteQuery(req, nullptr);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ((*result)->schema().columns.at(0).name, "fallback");
  EXPECT_EQ(primary.execute_calls, 1);
  EXPECT_EQ(fallback.execute_calls, 1);
}

TEST(FallbackEngineTest, ExecuteQueryFallbackUnimplementedSurfacesUnimplemented) {
  StubEngine primary, fallback;
  primary.execute_status = absl::UnimplementedError("primary nope");
  fallback.execute_status = absl::UnimplementedError("fallback nope");
  FallbackEngine engine(&primary, &fallback);

  QueryRequest req;
  auto result = engine.ExecuteQuery(req, nullptr);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kUnimplemented);
}

}  // namespace
}  // namespace fallback
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
