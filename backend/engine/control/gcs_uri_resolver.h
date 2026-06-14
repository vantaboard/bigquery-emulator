#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_GCS_URI_RESOLVER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_GCS_URI_RESOLVER_H_

#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

struct GCSObjectParts {
  std::string bucket;
  std::string object;
};

// ParseGsUri splits gs://bucket/object into bucket and object.
absl::StatusOr<GCSObjectParts> ParseGsUri(absl::string_view uri);

// GcsCachePath returns the on-disk cache path for a gs:// object under
// data_dir/external/gcs-cache/.
std::string GcsCachePath(absl::string_view data_dir,
                         const GCSObjectParts& parts);

// MaterializeGCSObjectToCache ensures the object bytes exist at the cache
// path (reading an existing snapshot or fetching from STORAGE_EMULATOR_HOST).
absl::StatusOr<std::string> MaterializeGCSObjectToCache(
    absl::string_view gs_uri, absl::string_view data_dir);

// UploadGCSObjectFromFile uploads a local file to gs:// via the storage
// emulator JSON API when STORAGE_EMULATOR_HOST is set.
absl::Status UploadGCSObjectFromFile(absl::string_view gs_uri,
                                     absl::string_view local_path,
                                     absl::string_view content_type);

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_GCS_URI_RESOLVER_H_
