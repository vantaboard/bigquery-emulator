# GoogleSQL Label Inventory (Phase 1 — Section 1)

This file freezes the **exact** set of `@googlesql//` labels referenced by the
emulator's Bazel build. The prebuilt artifact's wrapper repo (Phase 3) must
preserve every label in [§ Direct labels](#direct-labels) as an importable
target. Indirect / transitive labels (§ Indirect) are packaged in the artifact's
combined archive but not exposed as wrapper targets.

## Source of truth

```bash
rg '@googlesql//' \
  /home/brighten-tompkins/Code/bigquery-emulator \
  --glob '*.bazel' --glob 'BUILD' --glob 'BUILD.bazel'
```

Run from a clean checkout, that query yields **18 distinct labels** spanning
6 emulator BUILD files:

| Owning emulator BUILD file                                          | Targets that reference `@googlesql//` |
|---------------------------------------------------------------------|---------------------------------------|
| `backend/catalog/BUILD.bazel`                                       | `storage_table`, `googlesql_catalog`  |
| `backend/schema/BUILD.bazel`                                        | `googlesql_to_bq`, `googlesql_to_bq_test` |
| `backend/engine/reference_impl/BUILD.bazel`                         | `reference_impl_engine`, `reference_impl_engine_test` |
| `backend/engine/duckdb/BUILD.bazel`                                 | `duckdb_engine`, `duckdb_engine_test` |
| `backend/engine/duckdb/transpiler/BUILD.bazel`                      | `types`, `transpiler`, `transpiler_test` |
| `frontend/handlers/BUILD.bazel`                                     | `query_handler`                       |

If a future commit lands a new direct label not in [§ Direct labels](#direct-labels),
Phase 1 has been broken and the change must be classified before it can ship.
See [`upgrade-rules.md`](upgrade-rules.md) for the procedure.

## Direct labels

The 18 unique labels below are sorted by package, then by name. For each one we
record the upstream public headers it advertises (`hdrs = [...]` in the upstream
`BUILD` file, with the source-relative path the emulator's `#include` lines use),
the **prebuilt disposition** ("Expose" — surface as a wrapper `cc_library`,
"Alias" — surface via a renamed wrapper alias, or "Defer" — package the
archive but skip the wrapper), and **why** the emulator needs the label today.

### Package `@googlesql//googlesql/public`

| Label                                       | Upstream `hdrs`                                                          | Prebuilt disposition | Why the emulator depends on it |
|---------------------------------------------|--------------------------------------------------------------------------|----------------------|-------------------------------- |
| `@googlesql//googlesql/public:analyzer`               | `analyzer.h`                                                              | Expose | Engine `Analyze*` entry points used by `duckdb_engine`, `reference_impl_engine`, `query_handler`, transpiler tests. |
| `@googlesql//googlesql/public:analyzer_options`       | `analyzer_options.h`                                                      | Expose | Builds the `AnalyzerOptions` the engines + handler pass into `AnalyzeStatement`. |
| `@googlesql//googlesql/public:analyzer_output`        | `analyzer_output.h`, `analyzer_output_properties.h`                       | Expose | Carries the resolved-AST + per-statement output back from the analyzer; consumed by both engines and the query handler. |
| `@googlesql//googlesql/public:builtin_function_options` | `builtin_function_options.h`                                            | Expose | `googlesql_catalog` registers the BigQuery-enabled builtin-function set; `transpiler_test` constructs its own options for the test harness. |
| `@googlesql//googlesql/public:catalog`                | `catalog.h`, `catalog_helper.h`, `property_graph.h`                       | Expose | Interface the `googlesql_catalog` adapter implements; transpiler resolves `Table`/`Column` references against it. |
| `@googlesql//googlesql/public:error_helpers`          | `error_helpers.h`                                                         | Expose | Query handler maps GoogleSQL `absl::Status` errors into gRPC codes. |
| `@googlesql//googlesql/public:error_location_cc_proto` | `error_location.pb.h` (generated)                                        | Expose | Query handler reads `ErrorLocation` payloads to render `position` in the BigQuery error responses. |
| `@googlesql//googlesql/public:evaluator`              | `evaluator.h`                                                             | Expose | `reference_impl_engine` runs the reference evaluator end-to-end. |
| `@googlesql//googlesql/public:evaluator_base`         | `evaluator_base.h`                                                        | Expose | Shared base for `Evaluator` + iterator types used by `reference_impl_engine`. |
| `@googlesql//googlesql/public:evaluator_table_iterator` | `evaluator_table_iterator.h`                                            | Expose | `storage_table` returns one of these from `CreateEvaluatorTableIterator`. |
| `@googlesql//googlesql/public:function`               | `function.h`, `function_signature.h`, `input_argument_type.h`, `procedure.h`, `table_valued_function.h` | Expose | Transpiler walks `Function::Name()` on `ResolvedFunctionCall` nodes. |
| `@googlesql//googlesql/public:language_options`       | `language_options.h`                                                      | Expose | All engines + handler + transpiler tests snapshot the same `LanguageOptions` between analyzer and evaluator. |
| `@googlesql//googlesql/public:options_cc_proto`       | `options.pb.h` (generated)                                                | Expose | Carries `ProductMode::PRODUCT_EXTERNAL` and `LanguageFeature` enums into every engine + the transpiler. |
| `@googlesql//googlesql/public:simple_catalog`         | `simple_catalog.h`, `simple_property_graph.h`, `table_from_proto.h`       | Expose | Concrete catalog the adapter subclasses. Also (intentionally) transitively re-exports `type_factory.h` / `struct_type.h` because `@googlesql//googlesql/public/types:types` is package-visibility-restricted. |
| `@googlesql//googlesql/public:type`                   | `type.h`, `convert_type_to_proto.h`                                       | Expose | Type identity referenced everywhere the schema, catalog, engines, or transpiler must reason about a `googlesql::Type*`. |
| `@googlesql//googlesql/public:value`                  | `value.h`, `proto_util.h`                                                 | Expose | `storage_table` materialises `googlesql::Value` cells; transpiler emits SQL literals via `Value::GetSQLLiteral`. |

### Package `@googlesql//googlesql/resolved_ast`

| Label                                                                    | Upstream `hdrs`                                       | Prebuilt disposition | Why the emulator depends on it |
|--------------------------------------------------------------------------|-------------------------------------------------------|----------------------|--------------------------------|
| `@googlesql//googlesql/resolved_ast` (default target)                   | `resolved_ast.h`, `resolved_ast_visitor.h`, `resolved_node.h`, `resolved_column.h`, `resolved_collation.h`, … (generated `resolved_ast.{h,cc}` from `gen_resolved_ast.py`) | Expose | All resolved-AST node types + the visitor base the transpiler subclasses. |
| `@googlesql//googlesql/resolved_ast:resolved_node_kind_cc_proto`         | `resolved_node_kind.pb.h` (generated)                | Expose | `ResolvedNodeKind` enum used in engine dispatch. |

## Indirect (transitive-only) labels

The emulator never names these labels directly, but the prebuilt artifact's
combined archive must contain their compiled objects and the prebuilt headers
must include their `hdrs`, because the **direct** labels' headers `#include`
them.

The complete transitive list is large and frozen by the Phase 2 producer
(it materialises whatever `bazel build @googlesql//...` resolves at the pinned
upstream commit). For Phase 1, the load-bearing transitive packages are:

- `@googlesql//googlesql/public/types` (`type_factory.h`, `struct_type.h`,
  `array_type.h`, `enum_type.h`, `proto_type.h`, `range_type.h`, …) — pulled in
  by `simple_catalog`. The package's `:types` Bazel label has visibility
  restricted to `//googlesql/base:googlesql_implementation`, so the wrapper
  must **not** expose `:types` directly. The headers ship under
  `include/googlesql/public/types/` regardless; the wrapper rule simply does
  not advertise a `:types` target.
- `@googlesql//googlesql/public/proto` (`type_annotation.pb.h`, …).
- `@googlesql//googlesql/common` (`errors`, `utf_util`, `thread_stack`, …).
- `@googlesql//googlesql/base` (`status`, `case`, `ret_check`, `map_util`, …).
- `@googlesql//googlesql/public/functions` (used transitively by `value`).
- `@googlesql//googlesql/resolved_ast:resolved_ast_cc_proto`.
- The third-party deps GoogleSQL pulls in (Abseil, Protobuf, gRPC, RE2,
  GoogleTest, ICU, FlatBuffers, …). These remain Bzlmod-resolved by the
  consumer (`MODULE.bazel`) — the prebuilt artifact does **not** ship Abseil /
  Protobuf objects. See [`headers-and-libraries.md`](headers-and-libraries.md)
  for the static-link boundary.

## Counts (must match `rg` output)

- 6 emulator BUILD files reference GoogleSQL directly.
- 18 unique direct `@googlesql//` labels (16 in `googlesql/public`, 2 in
  `googlesql/resolved_ast`).
- All 18 are classified **Expose** in this phase (no Alias / Defer).

The "all Expose" decision is deliberate: the emulator BUILDs already name these
labels, the surface is small, and Phase 3 must not silently move emulator code
to new label names. Aliasing would only buy us future renaming flexibility, and
we explicitly do not want renaming flexibility this rollout — that's what the
[upgrade rules](upgrade-rules.md) gate.

## How to keep this file honest

If you change a `BUILD.bazel` in the emulator to add or remove a
`@googlesql//` label:

1. Run the source-of-truth `rg` query above.
2. Add or remove the matching row in this file.
3. Update [`repo-layout.md`](repo-layout.md) so the wrapper-target mapping
   stays 1:1 with the inventory.
4. Bump the prebuilt artifact's **compatibility** version in
   [`manifest.md`](manifest.md) — adding or removing an exposed label is a
   Phase-3-breaking change.
