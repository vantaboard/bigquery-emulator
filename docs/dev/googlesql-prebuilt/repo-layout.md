# GoogleSQL Prebuilt — External Repo Layout

This file freezes the on-disk layout of the unpacked prebuilt artifact and
the wrapper `BUILD.bazel` targets the consumer-wiring track sees. The
artifact-producer pipeline must produce exactly this layout; the consumer
must not assume anything beyond it.

The repo is named `@googlesql_prebuilt_linux_amd64` in the consumer's Bzlmod
graph. The consumer-wiring track wires it via `http_archive` (or, for local
dev, an equivalent local override) — that wiring is **not** designed here.

## File tree

```text
googlesql_prebuilt_linux_amd64/                            # repo root after unpack
├── BUILD.bazel                                            # top-level wrapper targets (see § Wrapper targets)
├── MODULE.bazel                                           # module(name = "googlesql_prebuilt_linux_amd64", version = ...)
├── manifest.json                                          # see manifest.md
├── LICENSES/
│   ├── googlesql-LICENSE                                  # upstream GoogleSQL license, copied verbatim
│   └── thirdparty-NOTICE                                  # surface of bundled compile-time-only deps
├── include/
│   └── googlesql/                                         # mirror of upstream source tree, headers only
│       ├── base/...
│       ├── common/...
│       ├── public/
│       │   ├── analyzer.h
│       │   ├── analyzer_options.h
│       │   ├── analyzer_output.h
│       │   ├── analyzer_output_properties.h
│       │   ├── builtin_function_options.h
│       │   ├── catalog.h
│       │   ├── catalog_helper.h
│       │   ├── convert_type_to_proto.h
│       │   ├── error_helpers.h
│       │   ├── error_location.pb.h
│       │   ├── evaluator.h
│       │   ├── evaluator_base.h
│       │   ├── evaluator_table_iterator.h
│       │   ├── function.h
│       │   ├── function_signature.h
│       │   ├── input_argument_type.h
│       │   ├── language_options.h
│       │   ├── options.pb.h
│       │   ├── proto_util.h
│       │   ├── procedure.h
│       │   ├── property_graph.h
│       │   ├── simple_catalog.h
│       │   ├── simple_property_graph.h
│       │   ├── table_from_proto.h
│       │   ├── table_valued_function.h
│       │   ├── type.h
│       │   ├── type.pb.h
│       │   ├── value.h
│       │   ├── functions/...                              # transitive (value depends on it)
│       │   ├── proto/...                                  # transitive (type/value depend on it)
│       │   └── types/                                     # transitive but heavily included
│       │       ├── array_type.h
│       │       ├── struct_type.h
│       │       ├── type_factory.h
│       │       └── ...                                    # full closure
│       ├── resolved_ast/
│       │   ├── resolved_ast.h
│       │   ├── resolved_ast_visitor.h
│       │   ├── resolved_collation.h
│       │   ├── resolved_column.h
│       │   ├── resolved_node.h
│       │   ├── resolved_node_kind.pb.h
│       │   └── ...                                        # full closure
│       └── scripting/
│           ├── script_executor.h
│           └── ...                                        # transitive scripting headers
└── lib/
    ├── libgooglesql.a                                     # combined static archive
    └── libgooglesql_protos.a                              # generated .pb.cc objects
```

## Wrapper targets (`BUILD.bazel` at the repo root)

The top-level `BUILD.bazel` is the only `BUILD` file in the prebuilt repo.
It declares the per-label `cc_library` wrappers that match each row in
[`label-inventory.md`](label-inventory.md). The wrappers are **not aliases**;
they are real `cc_library` targets that:

- Re-export only the headers from the matching label's row in the inventory
  (`hdrs = [...]`), so strict-deps narrowing on the consumer side stays
  identical to the source build.
- Set `includes = ["include"]` so `#include "googlesql/..."` resolves under
  the shipped include root.
- Depend on a single private `cc_import` of the combined static archives
  (`":_archive"`).
- Depend on the same Bzlmod-resolved third-party libraries the source labels
  do today (Abseil, Protobuf, gRPC, RE2, …), so a `cc_binary` linking against
  one of these wrappers sees the same `@com_google_absl//` and
  `@com_google_protobuf//` versions the source build sees.

### Per-label hdrs selection

The wrapper `cc_library` rows below are the canonical 1:1 mapping. The
`hdrs = [...]` column is exactly the upstream `hdrs` field from
[`label-inventory.md`](label-inventory.md); the `deps = [...]` column lists
the third-party Bzlmod deps that survive in the wrapper layer.

| Wrapper target (inside `@googlesql_prebuilt_linux_amd64//`) | Source label                                                        | `hdrs`                                                                                                                                              | Public `deps` (third-party, beyond `:_archive`)                                                                |
|-------------------------------------------------------------|---------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------|
| `//googlesql/public:analyzer`                              | `@googlesql//googlesql/public:analyzer`                            | `analyzer.h`                                                                                                                                        | `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/strings`, `@com_google_protobuf//:protobuf`     |
| `//googlesql/public:analyzer_options`                      | `@googlesql//googlesql/public:analyzer_options`                    | `analyzer_options.h`                                                                                                                                | `@com_google_absl//absl/status`, `@com_google_absl//absl/strings`, `@com_google_protobuf//:protobuf`              |
| `//googlesql/public:analyzer_output`                       | `@googlesql//googlesql/public:analyzer_output`                     | `analyzer_output.h`, `analyzer_output_properties.h`                                                                                                | `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/strings`                                       |
| `//googlesql/public:builtin_function_options`              | `@googlesql//googlesql/public:builtin_function_options`            | `builtin_function_options.h`                                                                                                                        | `@com_google_absl//absl/container:flat_hash_map`, `@com_google_absl//absl/container:flat_hash_set`               |
| `//googlesql/public:catalog`                               | `@googlesql//googlesql/public:catalog`                             | `catalog.h`, `catalog_helper.h`, `property_graph.h`                                                                                                | `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/strings`, `@com_google_absl//absl/types:span`  |
| `//googlesql/public:error_helpers`                         | `@googlesql//googlesql/public:error_helpers`                       | `error_helpers.h`                                                                                                                                   | `@com_google_absl//absl/status`, `@com_google_absl//absl/strings`                                                |
| `//googlesql/public:error_location_cc_proto`               | `@googlesql//googlesql/public:error_location_cc_proto`             | `error_location.pb.h`                                                                                                                               | `@com_google_protobuf//:protobuf`                                                                               |
| `//googlesql/public:formatter_options`                     | `@googlesql//googlesql/public:formatter_options`                   | `formatter_options.h`                                                                                                                               | `@com_google_absl//absl/strings`, `@com_google_protobuf//:protobuf`                                              |
| `//googlesql/public:evaluator`                             | `@googlesql//googlesql/public:evaluator`                           | `evaluator.h`                                                                                                                                       | `@com_google_absl//absl/status:statusor`                                                                         |
| `//googlesql/public:evaluator_base`                        | `@googlesql//googlesql/public:evaluator_base`                      | `evaluator_base.h`                                                                                                                                  | (none beyond `:_archive`)                                                                                       |
| `//googlesql/public:evaluator_table_iterator`              | `@googlesql//googlesql/public:evaluator_table_iterator`            | `evaluator_table_iterator.h`                                                                                                                        | `@com_google_absl//absl/status:statusor`                                                                         |
| `//googlesql/public:function`                              | `@googlesql//googlesql/public:function`                            | `function.h`, `function_signature.h`, `input_argument_type.h`, `procedure.h`, `table_valued_function.h`                                            | `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/strings`                                       |
| `//googlesql/public:language_options`                      | `@googlesql//googlesql/public:language_options`                    | `language_options.h`                                                                                                                                | `@com_google_absl//absl/container:flat_hash_set`                                                                  |
| `//googlesql/public:lenient_formatter`                     | `@googlesql//googlesql/public:lenient_formatter`                   | `lenient_formatter.h`                                                                                                                               | `@com_google_absl//absl/status`, `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/strings`       |
| `//googlesql/public:parse_helpers`                         | `@googlesql//googlesql/public:parse_helpers`                       | `parse_helpers.h`, `parse_tokens.h`                                                                                                                 | `@com_google_absl//absl/status`, `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/strings`, `@com_google_protobuf//:protobuf` |
| `//googlesql/public:options_cc_proto`                      | `@googlesql//googlesql/public:options_cc_proto`                    | `options.pb.h`                                                                                                                                      | `@com_google_protobuf//:protobuf`                                                                               |
| `//googlesql/public:simple_catalog`                        | `@googlesql//googlesql/public:simple_catalog`                      | `simple_catalog.h`, `simple_property_graph.h`, `table_from_proto.h`                                                                                | `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/synchronization`, `@com_google_protobuf//:protobuf` |
| `//googlesql/public:sql_formatter`                         | `@googlesql//googlesql/public:sql_formatter`                       | `sql_formatter.h`                                                                                                                                   | `@com_google_absl//absl/status`, `@com_google_absl//absl/strings`                                                |
| `//googlesql/public:type`                                  | `@googlesql//googlesql/public:type`                                | `type.h`, `convert_type_to_proto.h`, `type.pb.h`, **plus** the `googlesql/public/types/*.h` headers (`type_factory.h`, `struct_type.h`, `array_type.h`, …) that emulator code transitively `#include`s | `@com_google_absl//absl/status:statusor`, `@com_google_protobuf//:protobuf`                                       |
| `//googlesql/public:value`                                 | `@googlesql//googlesql/public:value`                               | `value.h`, `proto_util.h`                                                                                                                           | `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/strings`, `@com_google_absl//absl/time`, `@com_google_protobuf//:protobuf` |
| `//googlesql/resolved_ast:resolved_ast` (default target) | `@googlesql//googlesql/resolved_ast`                              | `resolved_ast.h`, `resolved_ast_visitor.h`, `resolved_column.h`, `resolved_node.h`, `resolved_collation.h`, … (generated closure)                  | `@com_google_absl//absl/status:statusor`, `@com_google_absl//absl/types:span`, `@com_google_protobuf//:protobuf`  |
| `//googlesql/resolved_ast:resolved_node_kind_cc_proto`   | `@googlesql//googlesql/resolved_ast:resolved_node_kind_cc_proto`  | `resolved_node_kind.pb.h`                                                                                                                           | `@com_google_protobuf//:protobuf`                                                                               |

Notes:

- The `:type` wrapper deliberately includes the `googlesql/public/types/*.h`
  headers in its `hdrs = [...]` even though upstream's `:type` does not. This
  is what avoids leaking the visibility-restricted upstream `:types` label
  while still letting the consumer's strict-deps see `:type` as the owning
  label for those transitive headers. The same trick is unnecessary on
  `:simple_catalog` because `simple_catalog.h` in upstream transitively
  re-exports the same headers — we keep both ergonomic paths.
- The third-party `deps` column is the **minimum** set the wrapper must
  expose to its consumers. The combined archive (`:_archive`) statically
  carries everything beyond that, so a label not listed here is hidden from
  strict-deps in the consumer (intentional — the source build behaves the
  same way today).
- `MODULE.bazel` inside the prebuilt repo declares the same Abseil /
  Protobuf / gRPC version pins as this repo's root `MODULE.bazel`. Mismatches
  are rejected at consume time by the safety-gate manifest validator.

### Repo-internal `:_archive` target

The private wrapper:

```python
cc_import(
    name = "_archive",
    static_library = "lib/libgooglesql.a",
    visibility = ["//:__subpackages__"],
)

cc_import(
    name = "_archive_protos",
    static_library = "lib/libgooglesql_protos.a",
    visibility = ["//:__subpackages__"],
)
```

Every public wrapper `cc_library` includes both in its `deps`. They are
**not** independently consumable from outside the prebuilt repo (visibility
restricts them to `//:__subpackages__`).

## Naming conventions

- Wrapper directories mirror upstream packages exactly: a label that lives
  at `@googlesql//googlesql/public:catalog` becomes
  `//googlesql/public:catalog` inside the prebuilt repo. The consumer can
  swap `@googlesql//` for `@googlesql_prebuilt_linux_amd64//` in a single
  `bazel mod` extension and everything resolves.
- The combined-archive files use the upstream module name (`libgooglesql`)
  rather than a versioned name (`libgooglesql_2026_1_1.a`). The version
  identity lives in `manifest.json`; embedding it in the file name would only
  make sense if multiple GoogleSQL versions had to coexist in the same Bazel
  graph (they don't).
- The repo name is `googlesql_prebuilt_linux_amd64`. The
  `linux_amd64` suffix is load-bearing: the compatibility surface explicitly
  scopes to that platform. When (if) an `arm64` artifact ships later, it will
  be a **sibling** repo named `googlesql_prebuilt_linux_arm64`; the consumer's
  `select()` chooses between them. Renaming the existing `linux_amd64` repo
  is a breaking change for every consumer-wiring and downstream consumer.

## Consumer-side wiring (preview, owned by the consumer-wiring track)

This is **not** decided here, but the wrapper layout above forces a specific
consume-time shape. For context:

- `MODULE.bazel` in this repo gains a `bazel_dep(name = "googlesql_prebuilt_linux_amd64", ...)`
  (or equivalent `http_archive` + repo override) behind a build setting that
  defaults to prebuilt and can be flipped to source for debugging.
- Emulator `BUILD.bazel` files do **not** change. They still reference
  `@googlesql//googlesql/public:catalog` etc.; a Bzlmod
  `multiple_version_override` (or repo alias) maps the old `@googlesql//`
  label space onto the prebuilt repo's wrappers.
- The `local_path_override` to `../googlesql/` survives behind an explicit
  "source mode" knob, off by default.

The consumer-wiring track will design the alias / override mechanic. The
compatibility surface only freezes the wrapper-side shape it has to plug into.
