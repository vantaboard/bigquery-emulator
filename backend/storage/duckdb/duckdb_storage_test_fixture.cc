#include "backend/storage/duckdb/duckdb_storage_test_fixture.h"

#include <cstdint>

#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

schema::TableSchema PeopleSchema() {
  schema::TableSchema s;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kInt64;
  id.mode = schema::ColumnMode::kRequired;
  schema::ColumnSchema name;
  name.name = "name";
  name.type = schema::ColumnType::kString;
  name.mode = schema::ColumnMode::kNullable;
  s.columns = {id, name};
  return s;
}

Row MakePerson(int64_t id, absl::string_view name) {
  Row r;
  r.cells = {Value::Int64(id), Value::String(std::string(name))};
  return r;
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
