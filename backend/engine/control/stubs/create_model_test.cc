#include "backend/engine/control/stubs/create_model.h"

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "gmock/gmock.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace stubs {
namespace {

using ::testing::HasSubstr;

// Build a minimal `ResolvedCreateModelStmt` directly via the
// `MakeResolvedCreateModelStmt` factory so the test does not
// depend on the analyzer's `CREATE MODEL` feature being enabled
// in the prebuilt artifact. The handler only reads the statement
// kind (and marks every field accessed); the actual contents
// are unused by today's metadata-only contract.
std::unique_ptr<::googlesql::ResolvedCreateModelStmt>
MakeMinimalCreateModelStmt() {
  return ::googlesql::MakeResolvedCreateModelStmt(
      /*name_path=*/{"my_model"},
      /*create_scope=*/
      ::googlesql::ResolvedCreateStatement::CREATE_DEFAULT_SCOPE,
      /*create_mode=*/
      ::googlesql::ResolvedCreateStatement::CREATE_DEFAULT,
      /*option_list=*/{},
      /*output_column_list=*/{},
      /*query=*/nullptr,
      /*aliased_query_list=*/{},
      /*transform_input_column_list=*/{},
      /*transform_list=*/{},
      /*transform_output_column_list=*/{},
      /*transform_analytic_function_group_list=*/{},
      /*input_column_definition_list=*/{},
      /*output_column_definition_list=*/{},
      /*is_remote=*/false,
      /*connection=*/nullptr);
}

TEST(RunCreateModelTest, AcceptsValidStatementAndReturnsOk) {
  // Metadata-only contract: the handler accepts a well-formed
  // `ResolvedCreateModelStmt` and returns OK. No storage write,
  // no model catalog entry -- the local-stub posture for ML
  // explicitly does NOT model the BigQuery `models/<id>`
  // namespace. A client that issues `CREATE MODEL` as a setup
  // step succeeds; downstream `ML.PREDICT` / `ML.EVALUATE` /
  // `ML.FORECAST` are `local_stub` TVFs that return schema-correct
  // NULL placeholders (`backend/engine/semantic/stubs/ml.cc`).
  auto stmt = MakeMinimalCreateModelStmt();
  EXPECT_TRUE(RunCreateModel(*stmt).ok());
}

TEST(RunCreateModelTest, RejectsWrongStatementKind) {
  // Defense-in-depth: the coordinator pre-dispatches to this
  // handler based on `node_kind()`, so this branch is unreachable
  // through the normal path. If a future regression mis-routes a
  // non-CREATE-MODEL statement here, we surface INTERNAL rather
  // than silently OK'ing the wrong shape (which would mask the
  // routing bug as "metadata-only stub accepted everything").
  // `MakeResolvedRollbackStmt` is the cheapest non-CREATE-MODEL
  // ResolvedStatement to construct in a unit test (it has no
  // required fields), and it is unambiguously a different kind.
  auto rollback = ::googlesql::MakeResolvedRollbackStmt();
  absl::Status s = RunCreateModel(*rollback);
  EXPECT_EQ(s.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(std::string(s.message()), HasSubstr("ResolvedCreateModelStmt"));
}

}  // namespace
}  // namespace stubs
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
