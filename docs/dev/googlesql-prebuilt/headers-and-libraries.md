# GoogleSQL Prebuilt — Headers and Libraries (Phase 1 — Section 2)

This file freezes the **shape** of the payload the Phase 2 producer must
publish for the prebuilt artifact. Phase 3 (consume wiring) will encode the
matching `cc_import` / `cc_library` wrappers; this file is what both sides
agree on.

## Decision summary

| Question                                              | Decision                                        |
|-------------------------------------------------------|-------------------------------------------------|
| Static or shared libraries?                           | **Static** (`.a` archives).                     |
| One combined archive or per-label archives?           | **One** combined archive (`libgooglesql.a`) + one matching `proto_combined.a` for generated protobuf objects. |
| Include root layout?                                  | `include/googlesql/...` mirroring upstream source paths. |
| Generated `.pb.h` headers?                            | Ship with the package, under the same `include/googlesql/...` prefix. |
| Third-party deps (Abseil, Protobuf, gRPC, RE2, …)?   | **Not bundled**. Resolved by the consumer through Bzlmod (`MODULE.bazel`) as today. |

## Rationale

### Static linking

GoogleSQL today (`linux/amd64`, `clang-18`, GoogleSQL's hermetic LLVM
toolchain) does not publish a stable shared-object ABI. The reference-impl
evaluator, the analyzer, and the resolved-AST library all share heavy template
glue with Abseil and Protobuf; the safest contract for the consumer is a
static `.a` archive that gets linked into the consumer's `cc_binary` /
`cc_library` exactly the same way the source build does today.

If a measured rebuild-time or binary-size blocker appears, we may revisit
shared linking in a later phase, but Phase 1 freezes static as the default.
Changing to shared is a **breaking** change (see
[`upgrade-rules.md`](upgrade-rules.md)).

### One combined archive

Per-label archives (one `.a` per `cc_library` in `@googlesql//googlesql/public:*`)
would surface upstream's internal dependency graph in the prebuilt repo. We do
not want consumers to depend on that graph — it changes between GoogleSQL
commits — so the producer flattens it into a single archive.

The combined archive carries every object file that `bazel build` produces
under `@googlesql//googlesql/...` plus the load-bearing transitive packages
listed in [`label-inventory.md`](label-inventory.md) (`googlesql/base`,
`googlesql/common`, `googlesql/public/types`, `googlesql/public/proto`,
`googlesql/public/functions`, `googlesql/resolved_ast`).

The combined archive does **not** contain Abseil, Protobuf, gRPC, RE2,
GoogleTest, ICU, or FlatBuffers objects. Those repos are still resolved by the
consumer's `MODULE.bazel` exactly as they are today; the prebuilt artifact's
wrapper `cc_library` targets declare them via `deps = [...]` so the linker
sees a single resolved version of each. This is what keeps the `slot_type`-era
Abseil pin in [`MODULE.bazel`](../../../MODULE.bazel) load-bearing — both the
source and the prebuilt path must end up with the same Abseil, otherwise the
linker resolves to mismatched symbol sets.

### Include-root layout (`include/googlesql/...`)

The emulator's `.cc` / `.h` files `#include "googlesql/public/type.h"` etc.
The prebuilt repo must therefore expose include roots so the same paths
resolve under `include/`:

```text
include/
  googlesql/
    base/...
    common/...
    public/
      *.h
      proto/...
      types/...
      functions/...
    resolved_ast/...
```

Every `#include "googlesql/..."` line that the emulator emits today must
resolve to a header that ships in this tree. The full list of emulator
include lines is enumerated in
[`label-inventory.md`](label-inventory.md#direct-labels); the wrapper
`cc_library` targets in [`repo-layout.md`](repo-layout.md) translate each
direct label into the matching `hdrs = [...]` plus
`strip_include_prefix = "include"` (or equivalent `includes = ["include"]`).

### Generated headers (`.pb.h`)

Protobuf-generated headers (`googlesql/public/options.pb.h`,
`googlesql/public/type.pb.h`, `googlesql/public/error_location.pb.h`,
`googlesql/resolved_ast/resolved_node_kind.pb.h`, …) ship **pre-generated**
under the same `include/googlesql/...` tree. The corresponding `.pb.cc`
object code is part of the combined archive. The consumer does **not** rerun
`protoc` at consume time — the producer pins both the `.proto` source and the
exact `protoc` / `protobuf` versions in the manifest.

If the consumer's `MODULE.bazel` Protobuf pin diverges from the producer's
manifest pin, the wrapper repo refuses to build (Phase 5 validation). For
Phase 1, the matching pin is `protobuf 29.0` (from this repo's `MODULE.bazel`).

## Header coverage requirement

The producer must ship **every** header referenced by an emulator
`#include "googlesql/..."` line (direct), **plus** every header those headers
transitively include from the GoogleSQL tree. The Phase 1 grep that proves
direct coverage:

```bash
rg '#include "googlesql/' \
  /home/brighten-tompkins/Code/bigquery-emulator \
  --glob '*.cc' --glob '*.h' \
  -o --no-filename | sort -u
```

Today that yields:

- `googlesql/public/analyzer.h`
- `googlesql/public/analyzer_options.h`
- `googlesql/public/analyzer_output.h`
- `googlesql/public/builtin_function_options.h`
- `googlesql/public/catalog.h`
- `googlesql/public/error_helpers.h`
- `googlesql/public/error_location.pb.h`
- `googlesql/public/evaluator.h`
- `googlesql/public/evaluator_base.h`
- `googlesql/public/evaluator_table_iterator.h`
- `googlesql/public/function.h`
- `googlesql/public/language_options.h`
- `googlesql/public/options.pb.h`
- `googlesql/public/simple_catalog.h`
- `googlesql/public/type.h`
- `googlesql/public/type.pb.h`
- `googlesql/public/types/array_type.h`
- `googlesql/public/types/struct_type.h`
- `googlesql/public/types/type_factory.h`
- `googlesql/public/value.h`
- `googlesql/resolved_ast/resolved_ast.h`
- `googlesql/resolved_ast/resolved_ast_visitor.h`
- `googlesql/resolved_ast/resolved_column.h`
- `googlesql/resolved_ast/resolved_node.h`
- `googlesql/resolved_ast/resolved_node_kind.pb.h`

These 25 headers are the **direct** emulator surface. The transitive set
(closure of `#include`s reachable from these 25 headers within the GoogleSQL
source tree) is what the producer must materialise under
`include/googlesql/...`. Phase 2 will enumerate the closure mechanically;
Phase 1 freezes the rule: **transitive closure of these 25, no more, no
less**.

If a new emulator `.cc` / `.h` adds a `#include "googlesql/..."` line outside
this set, Phase 1 has been broken and the new include must be reconciled
before the prebuilt artifact is published. See
[`upgrade-rules.md`](upgrade-rules.md) for the procedure.

## Library files in the artifact

The Phase 2 producer ships exactly two static libraries:

| File                      | Contains                                                                                                                                                                                                                                                                                                                                          |
|---------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `lib/libgooglesql.a`     | Compiled object code for every `cc_library` reachable from `bazel build @googlesql//googlesql/public:analyzer @googlesql//googlesql/public:evaluator @googlesql//googlesql/resolved_ast` at the pinned upstream commit, **excluding** Abseil, Protobuf, gRPC, RE2, GoogleTest, ICU, FlatBuffers (i.e. excluding everything under `@com_*`/`@boringssl`). |
| `lib/libgooglesql_protos.a` | Compiled object code for every `cc_proto_library` reachable from the same query (the `.pb.cc` generated objects). Kept separate so the wrapper repo can build a Protobuf descriptor-only `cc_library` without pulling all of `libgooglesql.a`. |

Both are static archives. PIC is enabled (the consumer's `cc_binary` may be
position-independent, e.g. when linking into a shared object loader). The
producer pins `-fPIC` and records it in the manifest's `compile_flags` field.

## Linking contract for the consumer

The Phase 3 wrapper `cc_library` targets that wrap each of the 16 direct
labels do **not** redeclare the per-label header subset — they all expose the
**same** `include/` root via `includes = ["include"]`. Per-label `hdrs = [...]`
selection is what produces the strict-deps narrowing the emulator
`BUILD.bazel` targets rely on. Each wrapper `cc_library` lists exactly the
headers from its row in [`label-inventory.md`](label-inventory.md) plus a
`deps = [":googlesql_archive"]` pointing at a single `cc_import` of
`lib/libgooglesql.a` + `lib/libgooglesql_protos.a`.

This means strict-deps inside the emulator still rejects an
`#include "googlesql/foo.h"` line that the calling `cc_library`'s row does
not authorise — the prebuilt and the source builds enforce the same Bazel
contract.
