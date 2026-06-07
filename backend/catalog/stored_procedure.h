#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_STORED_PROCEDURE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_STORED_PROCEDURE_H_

#include <string>
#include <vector>

#include "googlesql/public/function_signature.h"
#include "googlesql/public/procedure.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// In-memory SQL procedure registered via CREATE PROCEDURE. The
// analyzer needs a `googlesql::Procedure` for CALL resolution; body
// execution is handled by the semantic script driver.
class StoredSQLProcedure : public ::googlesql::Procedure {
 public:
  StoredSQLProcedure(std::vector<std::string> name_path,
                     ::googlesql::FunctionSignature signature,
                     std::vector<std::string> argument_name_list,
                     std::string procedure_body);

  const std::string& procedure_body() const {
    return procedure_body_;
  }
  const std::vector<std::string>& argument_name_list() const {
    return argument_name_list_;
  }

 private:
  std::vector<std::string> argument_name_list_;
  std::string procedure_body_;
};

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_STORED_PROCEDURE_H_
