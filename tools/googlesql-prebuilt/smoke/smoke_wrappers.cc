// Bazel-wrappers smoke binary for the GoogleSQL prebuilt artifact.
//
// Linked by `tools/googlesql-prebuilt/verify.sh` in the
// `--smoke-mode=bazel` path: the verifier writes a synthetic Bazel
// workspace next to the unpacked artifact, builds this `cc_binary`
// against the prebuilt repo's wrapper `cc_library`
// (`@googlesql_prebuilt_linux_amd64//googlesql/public:error_helpers`),
// and confirms the build + link succeeds.
//
// This variant DOES depend on a real GoogleSQL header
// (`error_helpers.h`) so the verifier validates the full
// consumer-wiring consume path:
//
//   - The wrapper repo's `BUILD.bazel` declares the right `hdrs` set.
//   - `strip_include_prefix = "/include"` reroots the include path.
//   - The static archives in `lib/` link against the abseil + protobuf
//     versions pinned in the wrapper's `MODULE.bazel`.

#include <cstdint>

#include "googlesql/public/error_helpers.h"

int main(int /*argc*/, char** /*argv*/) {
  // Just take the address of one symbol from the included header so
  // the linker is FORCED to resolve at least one googlesql object.
  // FormatError is part of googlesql/public/error_helpers.h's
  // public surface (per the GoogleSQL prebuilt label inventory).
  auto* fn = &::googlesql::FormatError;
  return reinterpret_cast<std::intptr_t>(fn) != 0 ? 0 : 1;
}
