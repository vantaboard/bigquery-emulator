// String-family functions evaluated by the semantic executor.
//
// These functions either have no thin DuckDB equivalent
// (`SOUNDEX` — DuckDB v1.5.3 does not ship the scalar at all) or
// expose a variadic surface DuckDB cannot model with a macro
// wrapper (`INSTR` — BigQuery accepts 2..4 args with negative
// `position` and Nth `occurrence`). The dispatch entries live in
// `backend/engine/semantic/functions/dispatch.cc`; the
// `functions.yaml` rows route the analyzer here.

#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_FUNCS_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> Soundex(const std::vector<Value>& args);
absl::StatusOr<Value> Instr(const std::vector<Value>& args);
absl::StatusOr<Value> RegexpContains(const std::vector<Value>& args);
absl::StatusOr<Value> RegexpExtract(const std::vector<Value>& args);
absl::StatusOr<Value> RegexpExtractAll(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type);
absl::StatusOr<Value> RegexpReplace(const std::vector<Value>& args);
absl::StatusOr<Value> RegexpInstr(const std::vector<Value>& args);
absl::StatusOr<Value> Format(const std::vector<Value>& args);
absl::StatusOr<Value> FormatString(const std::vector<Value>& args);
absl::StatusOr<Value> ToJson(const std::vector<Value>& args);

absl::StatusOr<Value> Ascii(const std::vector<Value>& args);
absl::StatusOr<Value> ByteLength(const std::vector<Value>& args);
absl::StatusOr<Value> CharLength(const std::vector<Value>& args);
absl::StatusOr<Value> OctetLength(const std::vector<Value>& args);
absl::StatusOr<Value> Chr(const std::vector<Value>& args);
absl::StatusOr<Value> Unicode(const std::vector<Value>& args);
absl::StatusOr<Value> Concat(const std::vector<Value>& args);
absl::StatusOr<Value> Length(const std::vector<Value>& args);
absl::StatusOr<Value> Lower(const std::vector<Value>& args);
absl::StatusOr<Value> Upper(const std::vector<Value>& args);
absl::StatusOr<Value> Trim(const std::vector<Value>& args);
absl::StatusOr<Value> Ltrim(const std::vector<Value>& args);
absl::StatusOr<Value> Rtrim(const std::vector<Value>& args);
absl::StatusOr<Value> Lpad(const std::vector<Value>& args);
absl::StatusOr<Value> Rpad(const std::vector<Value>& args);
absl::StatusOr<Value> Replace(const std::vector<Value>& args);
absl::StatusOr<Value> Reverse(const std::vector<Value>& args);
absl::StatusOr<Value> StartsWith(const std::vector<Value>& args);
absl::StatusOr<Value> EndsWith(const std::vector<Value>& args);
absl::StatusOr<Value> Left(const std::vector<Value>& args);
absl::StatusOr<Value> Right(const std::vector<Value>& args);
absl::StatusOr<Value> Substr(const std::vector<Value>& args);
absl::StatusOr<Value> Strpos(const std::vector<Value>& args);
absl::StatusOr<Value> Split(const std::vector<Value>& args,
                            const ::googlesql::Type* return_type);
absl::StatusOr<Value> ToHex(const std::vector<Value>& args);
absl::StatusOr<Value> FromHex(const std::vector<Value>& args);
absl::StatusOr<Value> ToBase64(const std::vector<Value>& args);
absl::StatusOr<Value> FromBase64(const std::vector<Value>& args);
absl::StatusOr<Value> ToBase32(const std::vector<Value>& args);
absl::StatusOr<Value> FromBase32(const std::vector<Value>& args);
absl::StatusOr<Value> Md5(const std::vector<Value>& args);
absl::StatusOr<Value> Sha1(const std::vector<Value>& args);
absl::StatusOr<Value> Sha256(const std::vector<Value>& args);
absl::StatusOr<Value> Sha512(const std::vector<Value>& args);
absl::StatusOr<Value> FarmFingerprintFunc(const std::vector<Value>& args);
absl::StatusOr<Value> InitcapFunc(const std::vector<Value>& args);
absl::StatusOr<Value> NormalizeFunc(const std::vector<Value>& args);
absl::StatusOr<Value> NormalizeAndCasefoldFunc(const std::vector<Value>& args);
absl::StatusOr<Value> SafeConvertBytesToString(const std::vector<Value>& args);
absl::StatusOr<Value> CodePointsToString(const std::vector<Value>& args);
absl::StatusOr<Value> CodePointsToBytes(const std::vector<Value>& args);
absl::StatusOr<Value> ToCodePoints(const std::vector<Value>& args,
                                   const ::googlesql::Type* return_type);
absl::StatusOr<Value> Least(const std::vector<Value>& args,
                            const ::googlesql::Type* return_type);
absl::StatusOr<Value> Greatest(const std::vector<Value>& args,
                               const ::googlesql::Type* return_type);
absl::StatusOr<Value> ParseNumericFunc(const std::vector<Value>& args);
absl::StatusOr<Value> ParseBignumeric(const std::vector<Value>& args,
                                      const EvalContext* ctx);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_FUNCS_H_
