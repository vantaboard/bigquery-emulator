#include "backend/catalog/emulator_builtin_extensions.h"
#include "backend/catalog/googlesql_catalog.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

TEST(EmulatorKeysStubCatalogTest, AnalyzeKeysEncryptDecryptRoundTrip) {
  ::googlesql::TypeFactory type_factory;
  ::googlesql::LanguageOptions language = MakeCatalogLanguageOptions();
  ::googlesql::SimpleCatalog catalog("test_catalog", &type_factory);
  ASSERT_TRUE(catalog
                  .AddBuiltinFunctionsAndTypes(
                      ::googlesql::BuiltinFunctionOptions(language))
                  .ok());
  RegisterEmulatorBuiltinFunctions(catalog);
  RegisterEmulatorKeysStubFunctions(catalog);

  const ::googlesql::Function* encrypt_fn = nullptr;
  const ::googlesql::Function* decrypt_fn = nullptr;
  const ::googlesql::Function* new_keyset_fn = nullptr;
  const absl::Status encrypt_found =
      catalog.FindFunction({"KEYS", "ENCRYPT"}, &encrypt_fn);
  const absl::Status decrypt_found =
      catalog.FindFunction({"KEYS", "DECRYPT_BYTES"}, &decrypt_fn);
  const absl::Status new_keyset_found =
      catalog.FindFunction({"KEYS", "NEW_KEYSET"}, &new_keyset_fn);
  EXPECT_TRUE(encrypt_found.ok()) << encrypt_found;
  EXPECT_NE(encrypt_fn, nullptr);
  EXPECT_TRUE(decrypt_found.ok()) << decrypt_found;
  EXPECT_NE(decrypt_fn, nullptr);
  EXPECT_TRUE(new_keyset_found.ok()) << new_keyset_found;
  EXPECT_NE(new_keyset_fn, nullptr);

  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.CreateDefaultArenasIfNotSet();

  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  const absl::Status analyzed = ::googlesql::AnalyzeStatement(
      R"(WITH ks AS (SELECT KEYS.NEW_KEYSET('AEAD_AES_GCM_256') AS keyset)
SELECT TO_BASE64(
  KEYS.DECRYPT_BYTES(
    keyset,
    KEYS.ENCRYPT(keyset, FROM_BASE64('YWJj'))
  )
) AS plain
FROM ks)",
      options,
      &catalog,
      &type_factory,
      &output);
  EXPECT_TRUE(analyzed.ok()) << analyzed;
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
