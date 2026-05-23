#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_

// Phase 0 placeholder.
//
// This header will declare the in-memory backend (catalog + storage) the
// frontend/ handlers read and write through. Concretely, Phase 3 of
// ROADMAP.md will land:
//
//   class Catalog {
//    public:
//     // Dataset/table CRUD called from frontend/handlers/catalog.cc.
//     absl::Status CreateDataset(...);
//     absl::Status CreateTable(...);
//     absl::StatusOr<TableHandle> ResolveTable(absl::string_view dataset,
//                                              absl::string_view table);
//     ...
//   };
//
//   class Table {
//    public:
//     // Implements googlesql::Table so the analyzer can resolve names.
//     // Provides row iterators for the reference impl evaluator.
//   };
//
// Keeping it as a header-only placeholder for now so the directory exists
// in source control and the layout matches cloud-spanner-emulator.

namespace bigquery_emulator {
namespace backend {
namespace storage {

// Reserved namespace; intentionally empty in Phase 0.

}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_STORAGE_H_
