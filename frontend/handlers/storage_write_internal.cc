#include "frontend/handlers/storage_write_internal.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

// Splits a path like `projects/{p}/datasets/{d}/tables/{t}` into
// pieces. Returns false on any malformed shape; the caller maps that
// onto INVALID_ARGUMENT with a stable message.
bool SplitTablePath(absl::string_view path, backend::storage::TableId* out) {
  const std::vector<absl::string_view> parts = absl::StrSplit(path, '/');
  if (parts.size() != 6 || parts[0] != "projects" || parts[2] != "datasets" ||
      parts[4] != "tables" || parts[1].empty() || parts[3].empty() ||
      parts[5].empty()) {
    return false;
  }
  out->project_id = std::string(parts[1]);
  out->dataset_id = std::string(parts[3]);
  out->table_id = std::string(parts[5]);
  return true;
}

// Stitches a TableId back into its canonical path. Used to mint the
// stream name and the `_default` reserved id so the wire shape is
// stable regardless of which entry point built the value.
std::string TablePathFor(const backend::storage::TableId& id) {
  return absl::StrCat("projects/",
                      id.project_id,
                      "/datasets/",
                      id.dataset_id,
                      "/tables/",
                      id.table_id);
}

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
