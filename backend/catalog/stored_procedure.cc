#include "backend/catalog/stored_procedure.h"

#include <string>
#include <utility>

namespace bigquery_emulator {
namespace backend {
namespace catalog {

StoredSQLProcedure::StoredSQLProcedure(
    std::vector<std::string> name_path,
    ::googlesql::FunctionSignature signature,
    std::vector<std::string> argument_name_list,
    std::string procedure_body)
    : ::googlesql::Procedure(name_path, signature),
      argument_name_list_(std::move(argument_name_list)),
      procedure_body_(std::move(procedure_body)) {}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
