#include "backend/storage/storage.h"

#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace bigquery_emulator {
namespace backend {
namespace storage {

namespace {

// Singleton empties returned from accessors when the active kind does
// not hold the requested type. Static-storage so the references stay
// valid for the program's lifetime; safe because Value's accessors are
// not allowed to mutate them.
const std::string& EmptyString() {
  static const std::string* const kEmpty = new std::string();
  return *kEmpty;
}

const std::vector<Value>& EmptyValueVector() {
  static const std::vector<Value>* const kEmpty = new std::vector<Value>();
  return *kEmpty;
}

}  // namespace

Value Value::Bool(bool v) {
  Value out;
  out.kind_ = Kind::kBool;
  out.data_ = v;
  return out;
}

Value Value::Int64(int64_t v) {
  Value out;
  out.kind_ = Kind::kInt64;
  out.data_ = v;
  return out;
}

Value Value::Float64(double v) {
  Value out;
  out.kind_ = Kind::kFloat64;
  out.data_ = v;
  return out;
}

Value Value::String(std::string v) {
  Value out;
  out.kind_ = Kind::kString;
  out.data_ = std::move(v);
  return out;
}

Value Value::Bytes(std::string v) {
  Value out;
  out.kind_ = Kind::kBytes;
  out.data_ = std::move(v);
  return out;
}

Value Value::Array(std::vector<Value> elements) {
  Value out;
  out.kind_ = Kind::kArray;
  out.data_ = std::move(elements);
  return out;
}

Value Value::Struct(std::vector<Value> fields) {
  Value out;
  out.kind_ = Kind::kStruct;
  out.data_ = std::move(fields);
  return out;
}

bool Value::bool_value() const {
  if (const auto* p = std::get_if<bool>(&data_)) return *p;
  return false;
}

int64_t Value::int64_value() const {
  if (const auto* p = std::get_if<int64_t>(&data_)) return *p;
  return 0;
}

double Value::float64_value() const {
  if (const auto* p = std::get_if<double>(&data_)) return *p;
  return 0.0;
}

const std::string& Value::string_value() const {
  if (const auto* p = std::get_if<std::string>(&data_)) return *p;
  return EmptyString();
}

const std::vector<Value>& Value::array_value() const {
  if (kind_ != Kind::kArray) return EmptyValueVector();
  if (const auto* p = std::get_if<std::vector<Value>>(&data_)) return *p;
  return EmptyValueVector();
}

const std::vector<Value>& Value::struct_value() const {
  if (kind_ != Kind::kStruct) return EmptyValueVector();
  if (const auto* p = std::get_if<std::vector<Value>>(&data_)) return *p;
  return EmptyValueVector();
}

}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
