#include "backend/engine/coordinator/sql_preprocess.h"

#include <string>

#include "absl/strings/match.h"
#include "backend/catalog/create_function_util.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {

::googlesql::AnalyzerOptions MakeOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  language.SetSupportsAllStatementKinds();
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

TEST(SqlPreprocessTest, FromHexFixtureAnalyzesAfterPreprocess) {
  const std::string sql = R"(/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

-- from_hex:
CREATE FUNCTION from_hex(value STRING) 

AS
(
  (
    SELECT 
      SUM(
      	CAST(
      	  CONCAT('0x', SUBSTR(value, byte * 2 + 1, 2)) 
      	    AS INT64) << ((LENGTH(value) - (byte + 1) * 2) * 4))
    FROM UNNEST(GENERATE_ARRAY(1, LENGTH(value) / 2)) WITH OFFSET byte
  )
);)";

  const std::string once = PreprocessSqlForAnalyzer(sql);
  const std::string twice = PreprocessSqlForAnalyzer(once);
  EXPECT_EQ(once, twice);

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog("test", &type_factory);
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ASSERT_TRUE(catalog
                  .AddBuiltinFunctionsAndTypes(
                      ::googlesql::BuiltinFunctionOptions(language))
                  .ok());

  ::googlesql::AnalyzerOptions options = MakeOptions();
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed = ::googlesql::AnalyzeStatement(
      twice, options, &catalog, &type_factory, &output);
  EXPECT_TRUE(analyzed.ok()) << analyzed << "\npreprocessed:\n" << twice;
}

TEST(SqlPreprocessTest, FromHexFunctionBodyAnalyzesAfterPreprocess) {
  const std::string body = R"((
  (
    SELECT
      SUM(
        CAST(
          CONCAT('0x', SUBSTR(value, byte * 2 + 1, 2))
            AS INT64) << ((LENGTH(value) - (byte + 1) * 2) * 4))
    FROM UNNEST(GENERATE_ARRAY(1, LENGTH(value) / 2)) WITH OFFSET byte
  )
);)";

  const std::string once = PreprocessFunctionBodyForAnalyzer(body);
  const std::string twice = PreprocessFunctionBodyForAnalyzer(once);
  EXPECT_EQ(once, twice);
  EXPECT_TRUE(absl::StartsWith(once, "((")) << once;
  EXPECT_TRUE(absl::EndsWith(once, "))")) << once;
  EXPECT_EQ(once.find(';'), std::string::npos) << once;
}

TEST(SqlPreprocessTest, FromHexFixtureRegistersAfterPreprocess) {
  const std::string sql = R"(/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

-- from_hex:
-- Input: STRING representing a number in hexadecimal form
-- Output: INT64 number in decimal form
CREATE FUNCTION from_hex(value STRING) 

AS
(
  (
    SELECT 
      SUM(
      	CAST(
      	  CONCAT('0x', SUBSTR(value, byte * 2 + 1, 2)) 
      	    AS INT64) << ((LENGTH(value) - (byte + 1) * 2) * 4))
    FROM UNNEST(GENERATE_ARRAY(1, LENGTH(value) / 2)) WITH OFFSET byte
  )
);)";

  const std::string preprocessed = PreprocessSqlForAnalyzer(sql);

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog("test", &type_factory);
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ASSERT_TRUE(catalog
                  .AddBuiltinFunctionsAndTypes(
                      ::googlesql::BuiltinFunctionOptions(language))
                  .ok());

  ::googlesql::AnalyzerOptions options = MakeOptions();
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  ASSERT_TRUE(::googlesql::AnalyzeStatement(
                  preprocessed, options, &catalog, &type_factory, &output)
                  .ok());
  const auto* create_fn =
      output->resolved_statement()
          ->GetAs<::googlesql::ResolvedCreateFunctionStmt>();
  ASSERT_NE(create_fn, nullptr);
  const std::string code = create_fn->code();
  EXPECT_FALSE(code.empty()) << "preprocessed:\n" << preprocessed;
  EXPECT_NE(code[0], ')') << "code():\n" << code;
  absl::StatusOr<std::unique_ptr<const ::googlesql::Function>> fn =
      bigquery_emulator::backend::catalog::MakeFunctionFromCreateFunction(
          *create_fn, /*function_options=*/nullptr);
  EXPECT_TRUE(fn.ok()) << fn.status() << "\ncode():\n" << code;
}

TEST(SqlPreprocessTest, CwToBaseWithBodyAnalyzesAfterPreprocess) {
  const std::string sql =
      R"(CREATE FUNCTION cw_to_base(number INT64, base INT64) RETURNS STRING 

AS (
    (WITH chars AS (
        SELECT 1 AS ch, 1 AS idx
    )
    SELECT 'x' AS to_base FROM chars)
);)";

  const std::string once = PreprocessSqlForAnalyzer(sql);
  const std::string twice = PreprocessSqlForAnalyzer(once);
  EXPECT_EQ(once, twice);
  EXPECT_TRUE(absl::StrContains(once, "AS (( WITH")) << once;

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog("test", &type_factory);
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ASSERT_TRUE(catalog
                  .AddBuiltinFunctionsAndTypes(
                      ::googlesql::BuiltinFunctionOptions(language))
                  .ok());

  ::googlesql::AnalyzerOptions options = MakeOptions();
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed = ::googlesql::AnalyzeStatement(
      twice, options, &catalog, &type_factory, &output);
  EXPECT_TRUE(analyzed.ok()) << analyzed << "\npreprocessed:\n" << twice;
}

TEST(SqlPreprocessTest, CwFromBaseFixtureAnalyzesAfterPreprocess) {
  const std::string sql =
      R"(CREATE FUNCTION cw_from_base(number STRING, base INT64) RETURNS INT64 

AS (
    (WITH chars AS (
        SELECT IF(ch >= 48 AND ch <= 57, ch - 48, IF(ch >= 65 AND ch <= 90, ch - 65 + 10, ch - 97 + 10)) pos, offset + 1 AS idx
        FROM UNNEST(TO_CODE_POINTS(number)) AS ch WITH OFFSET
    )
    SELECT SAFE_CAST(SUM(pos*CAST(POW(base, CHAR_LENGTH(number) - idx) AS NUMERIC)) AS INT64) from_base FROM chars)
);)";

  const std::string preprocessed = PreprocessSqlForAnalyzer(sql);
  const std::string twice = PreprocessSqlForAnalyzer(preprocessed);
  EXPECT_EQ(preprocessed, twice);
  EXPECT_TRUE(absl::StrContains(preprocessed, "AS (( WITH")) << preprocessed;

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog("test", &type_factory);
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ASSERT_TRUE(catalog
                  .AddBuiltinFunctionsAndTypes(
                      ::googlesql::BuiltinFunctionOptions(language))
                  .ok());

  ::googlesql::AnalyzerOptions options = MakeOptions();
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  ASSERT_TRUE(::googlesql::AnalyzeStatement(
                  preprocessed, options, &catalog, &type_factory, &output)
                  .ok())
      << preprocessed;
}

TEST(SqlPreprocess, RewritesIntegerTypeAliasInQuery) {
  const std::string sql =
      "SELECT STRUCT(CAST(2.89 AS FLOAT64) AS t_value, CAST(21 AS INTEGER) AS "
      "dof)";
  const std::string out = PreprocessSqlForAnalyzer(sql);
  EXPECT_TRUE(absl::StrContains(out, "21 AS dof")) << out;
  EXPECT_FALSE(absl::StrContains(out, "INTEGER")) << out;
  EXPECT_FALSE(absl::StrContains(out, "CAST(21 AS")) << out;
}

TEST(SqlPreprocess, RewritesStructInt64LiteralCastsInTTestQuery) {
  const std::string sql =
      "SELECT TO_JSON_STRING(STRUCT(CAST(2.8957935572829476 AS FLOAT64) AS "
      "t_value, CAST(21 AS INTEGER) AS dof))";
  const std::string out = PreprocessSqlForAnalyzer(sql);
  EXPECT_TRUE(
      absl::StrContains(out, "CAST(2.8957935572829476 AS FLOAT64) AS t_value"))
      << out;
  EXPECT_TRUE(absl::StrContains(out, "21 AS dof")) << out;
  EXPECT_FALSE(absl::StrContains(out, "CAST(21 AS")) << out;

  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog("test", &type_factory);
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ASSERT_TRUE(catalog
                  .AddBuiltinFunctionsAndTypes(
                      ::googlesql::BuiltinFunctionOptions(language))
                  .ok());
  ::googlesql::AnalyzerOptions options = MakeOptions();
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  EXPECT_TRUE(::googlesql::AnalyzeStatement(
                  out, options, &catalog, &type_factory, &output)
                  .ok())
      << out;
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
