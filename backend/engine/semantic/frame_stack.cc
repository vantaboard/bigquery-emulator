#include "backend/engine/semantic/frame_stack.h"

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

// BigQuery identifier comparison is case-insensitive. The
// scripting reference and the UDF / TVF argument-resolution rules
// both lower-case identifier names before lookup. Lower-case the
// name once on the way in so both reads and writes hit the same
// bucket.
std::string Canonicalize(absl::string_view name) {
  return absl::AsciiStrToLower(name);
}

}  // namespace

FrameStack::FrameStack() {
  // Open the outer (global) frame so callers do not have to
  // special-case the top-level binding. `frame_count()` is
  // therefore always >= 1 once construction returns.
  frames_.emplace_back();
}

FrameStack::~FrameStack() = default;

void FrameStack::PushFrame() {
  frames_.emplace_back();
}

absl::Status FrameStack::PopFrame() {
  if (frames_.size() <= 1) {
    return absl::FailedPreconditionError(
        "semantic::FrameStack: cannot pop the outer frame; "
        "PushFrame / PopFrame must be balanced");
  }
  frames_.pop_back();
  return absl::OkStatus();
}

absl::Status FrameStack::Declare(absl::string_view name, Value value) {
  if (frames_.empty()) {
    return absl::InternalError(
        "semantic::FrameStack::Declare: no frame on the stack");
  }
  std::string key = Canonicalize(name);
  Frame& top = frames_.back();
  if (top.contains(key)) {
    // BigQuery rejects `DECLARE x ...` after a previous
    // `DECLARE x ...` in the same block; UDF / TVF signatures
    // reject duplicate argument names. Either way the surface is
    // the same.
    return absl::AlreadyExistsError(absl::StrCat(
        "semantic::FrameStack: name already declared in this frame: ", name));
  }
  top.emplace(std::move(key), std::move(value));
  return absl::OkStatus();
}

absl::Status FrameStack::Set(absl::string_view name, Value value) {
  std::string key = Canonicalize(name);
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto found = it->find(key);
    if (found != it->end()) {
      found->second = std::move(value);
      return absl::OkStatus();
    }
  }
  return absl::NotFoundError(
      absl::StrCat("semantic::FrameStack: name not declared: ", name));
}

absl::StatusOr<Value> FrameStack::Lookup(absl::string_view name) const {
  std::string key = Canonicalize(name);
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto found = it->find(key);
    if (found != it->end()) {
      return found->second;
    }
  }
  return absl::NotFoundError(
      absl::StrCat("semantic::FrameStack: name not declared: ", name));
}

bool FrameStack::Has(absl::string_view name) const {
  std::string key = Canonicalize(name);
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    if (it->contains(key)) return true;
  }
  return false;
}

absl::flat_hash_map<std::string, Value> FrameStack::VisibleBindings() const {
  absl::flat_hash_map<std::string, Value> out;
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    for (const auto& [key, value] : *it) {
      out.try_emplace(key, value);
    }
  }
  return out;
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
