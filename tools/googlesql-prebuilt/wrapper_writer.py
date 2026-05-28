#!/usr/bin/env python3
"""Emit the wrapper-package BUILD.bazel files for the GoogleSQL prebuilt artifact.

Phase 1 freezes 18 direct `@googlesql//` labels (16 in `googlesql/public`,
2 in `googlesql/resolved_ast`) — see
`docs/dev/googlesql-prebuilt/label-inventory.md` and `repo-layout.md`.

The label inventory is STATIC, but the `hdrs = [...]` lists for the
`:type` and `:resolved_ast/resolved_ast` rules contain transitively-included
headers (`googlesql/public/types/*.h`, the resolved-AST generated set)
that vary with the upstream GoogleSQL commit. Bazel `glob()` is
package-relative, and those headers ship under the prebuilt repo's
root `include/` tree, so the wrapper packages can't glob them in.

This script walks the staged include tree and emits the
`googlesql/public/BUILD.bazel` and `googlesql/resolved_ast/BUILD.bazel`
files with EXPLICIT hdrs lists derived from the actual files present
in the artifact. The root `BUILD.bazel` is shipped verbatim from
`templates/BUILD.bazel`; everything dynamic lives here.

Usage:
    python3 wrapper_writer.py --repo-root PATH

PATH must contain `include/googlesql/public/...` and
`include/googlesql/resolved_ast/...` populated already; the script
WRITES `googlesql/public/BUILD.bazel` and
`googlesql/resolved_ast/BUILD.bazel` inside PATH.
"""

from __future__ import annotations

import argparse
import os
import pathlib
import sys
import textwrap


# Frozen `@googlesql//googlesql/public:*` wrapper definitions. Tuple
# layout: (target_name, static_hdrs, dynamic_hdrs_glob, deps).
#
# `static_hdrs` is the per-row `hdrs` field from `label-inventory.md`
# (hand-written headers that are guaranteed present at the pinned
# commit). `dynamic_hdrs_glob` is non-None only for `:type`, which
# additionally re-exports the `googlesql/public/types/*.h` closure
# (per `repo-layout.md`'s exception note). The script materialises the
# dynamic set at write time so the generated BUILD.bazel carries an
# EXPLICIT list — no glob() is needed at consume time.
PUBLIC_WRAPPERS: list[tuple[str, list[str], str | None, list[str]]] = [
    (
        "analyzer",
        ["analyzer.h"],
        None,
        [
            "@com_google_absl//absl/status:statusor",
            "@com_google_absl//absl/strings",
            "@com_google_protobuf//:protobuf",
        ],
    ),
    (
        "analyzer_options",
        ["analyzer_options.h"],
        None,
        [
            "@com_google_absl//absl/status",
            "@com_google_absl//absl/strings",
            "@com_google_protobuf//:protobuf",
        ],
    ),
    (
        "analyzer_output",
        ["analyzer_output.h", "analyzer_output_properties.h"],
        None,
        [
            "@com_google_absl//absl/status:statusor",
            "@com_google_absl//absl/strings",
        ],
    ),
    (
        "builtin_function_options",
        ["builtin_function_options.h"],
        None,
        [
            "@com_google_absl//absl/container:flat_hash_map",
            "@com_google_absl//absl/container:flat_hash_set",
        ],
    ),
    (
        "catalog",
        ["catalog.h", "catalog_helper.h", "property_graph.h"],
        None,
        [
            "@com_google_absl//absl/status:statusor",
            "@com_google_absl//absl/strings",
            "@com_google_absl//absl/types:span",
        ],
    ),
    (
        "error_helpers",
        ["error_helpers.h"],
        None,
        ["@com_google_absl//absl/status", "@com_google_absl//absl/strings"],
    ),
    (
        "error_location_cc_proto",
        ["error_location.pb.h"],
        None,
        ["@com_google_protobuf//:protobuf"],
    ),
    (
        "evaluator",
        ["evaluator.h"],
        None,
        ["@com_google_absl//absl/status:statusor"],
    ),
    ("evaluator_base", ["evaluator_base.h"], None, []),
    (
        "evaluator_table_iterator",
        ["evaluator_table_iterator.h"],
        None,
        ["@com_google_absl//absl/status:statusor"],
    ),
    (
        "function",
        [
            "function.h",
            "function_signature.h",
            "input_argument_type.h",
            "procedure.h",
            "table_valued_function.h",
        ],
        None,
        [
            "@com_google_absl//absl/status:statusor",
            "@com_google_absl//absl/strings",
        ],
    ),
    (
        "language_options",
        ["language_options.h"],
        None,
        ["@com_google_absl//absl/container:flat_hash_set"],
    ),
    (
        "options_cc_proto",
        ["options.pb.h"],
        None,
        ["@com_google_protobuf//:protobuf"],
    ),
    (
        "simple_catalog",
        ["simple_catalog.h", "simple_property_graph.h", "table_from_proto.h"],
        None,
        [
            "@com_google_absl//absl/status:statusor",
            "@com_google_absl//absl/synchronization",
            "@com_google_protobuf//:protobuf",
        ],
    ),
    (
        "type",
        ["convert_type_to_proto.h", "type.h", "type.pb.h"],
        # `:type` deliberately re-exports the upstream `:types` headers
        # in its own `hdrs`. See repo-layout.md "Notes" for why we do
        # this rather than expose `:types` (visibility-restricted
        # upstream) as a sibling wrapper.
        "googlesql/public/types",
        [
            "@com_google_absl//absl/status:statusor",
            "@com_google_protobuf//:protobuf",
        ],
    ),
    (
        "value",
        ["proto_util.h", "value.h"],
        None,
        [
            "@com_google_absl//absl/status:statusor",
            "@com_google_absl//absl/strings",
            "@com_google_absl//absl/time",
            "@com_google_protobuf//:protobuf",
        ],
    ),
]


# `@googlesql//googlesql/resolved_ast:*` wrappers. The default target
# (`:resolved_ast`) re-exports the whole `googlesql/resolved_ast/`
# header closure (generated + hand-written), MINUS the `.pb.h` headers
# owned by the proto wrapper.
RESOLVED_AST_PROTO_HEADER = "resolved_node_kind.pb.h"


HEADER_DEPS_COMMENT = textwrap.dedent("""\
    # Wrapper `cc_library` targets generated by
    # `tools/googlesql-prebuilt/wrapper_writer.py` from the staged
    # artifact's include tree. The label set is FROZEN by
    # `docs/dev/googlesql-prebuilt/label-inventory.md`; the `hdrs`
    # contents are emitted with EXPLICIT paths so consumers do not
    # need to glob at build time.
    #
    # `strip_include_prefix = "/include"` (repo-root relative; leading
    # slash is load-bearing) reroots `#include "googlesql/..."` under
    # the shipped `include/` tree. The cross-package hdrs references
    # rely on the `exports_files(glob(["include/**/*.h"]))` declaration
    # in the artifact's root BUILD.bazel.
    """)


def render_hdrs(paths: list[str]) -> str:
    """Render a `hdrs = [...]` list of cross-package //:include/... labels."""
    if not paths:
        return '    hdrs = [],'
    if len(paths) == 1:
        return f'    hdrs = ["//:{paths[0]}"],'
    body = "\n".join(f'        "//:{p}",' for p in sorted(paths))
    return f"    hdrs = [\n{body}\n    ],"


def render_deps(deps: list[str]) -> str:
    """Render the `deps = [...]` list including the private archive labels.

    `:_all_hdrs` is added to every wrapper so that any
    transitively-included header (notably the `*.pb.h` files generated
    from `cc_proto_library` rules in the source build) resolves under
    strict-deps + sandboxing without each wrapper having to enumerate
    them in its own `hdrs`. See the `_all_hdrs` rationale in the
    artifact root `BUILD.bazel` (rendered from
    `tools/googlesql-prebuilt/templates/BUILD.bazel`). The wrappers'
    `hdrs` lists remain the narrow consumer-facing public API frozen
    in `docs/dev/googlesql-prebuilt/label-inventory.md`; `:_all_hdrs`
    is purely a deps-side mechanism for transitive header coverage.
    """
    full_deps = ["//:_all_hdrs", "//:_archive", "//:_archive_protos", *deps]
    body = "\n".join(f'        "{d}",' for d in sorted(set(full_deps)))
    return f"    deps = [\n{body}\n    ],"


def render_public_build(repo_root: pathlib.Path) -> str:
    """Render `googlesql/public/BUILD.bazel` for the artifact at repo_root."""
    public_dir = repo_root / "include" / "googlesql" / "public"
    if not public_dir.is_dir():
        raise SystemExit(f"missing include/googlesql/public/ under {repo_root}")
    rules: list[str] = []
    for name, static_hdrs, dynamic_subdir, deps in PUBLIC_WRAPPERS:
        rooted = [f"include/googlesql/public/{h}" for h in static_hdrs]
        if dynamic_subdir is not None:
            sub = repo_root / "include" / dynamic_subdir
            if not sub.is_dir():
                raise SystemExit(
                    f"missing dynamic header dir {sub} required by :{name}"
                )
            extra = sorted(
                str(p.relative_to(repo_root))
                for p in sub.iterdir()
                if p.is_file() and p.suffix == ".h"
            )
            if not extra:
                raise SystemExit(
                    f"dynamic header dir {sub} is empty; refusing to ship"
                )
            rooted.extend(extra)
        # Confirm every header path exists on disk before writing the BUILD.
        for p in rooted:
            if not (repo_root / p).is_file():
                raise SystemExit(
                    f"declared header {p} is missing under {repo_root}"
                )
        rules.append(
            "cc_library(\n"
            f'    name = "{name}",\n'
            f"{render_hdrs(rooted)}\n"
            '    strip_include_prefix = "/include",\n'
            f"{render_deps(deps)}\n"
            ")"
        )
    body = "\n\n".join(rules)
    return (
        f'{HEADER_DEPS_COMMENT}\n'
        'load("@rules_cc//cc:defs.bzl", "cc_library")\n\n'
        'package(default_visibility = ["//visibility:public"])\n\n'
        f"{body}\n"
    )


def render_resolved_ast_build(repo_root: pathlib.Path) -> str:
    """Render `googlesql/resolved_ast/BUILD.bazel` for the artifact at repo_root."""
    resolved_dir = repo_root / "include" / "googlesql" / "resolved_ast"
    if not resolved_dir.is_dir():
        raise SystemExit(
            f"missing include/googlesql/resolved_ast/ under {repo_root}"
        )
    proto_path = (
        resolved_dir / RESOLVED_AST_PROTO_HEADER
    )
    if not proto_path.is_file():
        raise SystemExit(
            f"missing {RESOLVED_AST_PROTO_HEADER} under {resolved_dir}"
        )
    closure = sorted(
        str(p.relative_to(repo_root))
        for p in resolved_dir.iterdir()
        if p.is_file() and p.suffix == ".h"
    )
    # The default :resolved_ast wrapper carries everything under
    # resolved_ast/ EXCEPT the .pb.h owned by the proto wrapper.
    rule_a_hdrs = [
        p for p in closure if not p.endswith(f"/{RESOLVED_AST_PROTO_HEADER}")
    ]
    rule_b_hdrs = [f"include/googlesql/resolved_ast/{RESOLVED_AST_PROTO_HEADER}"]
    rule_a = (
        "cc_library(\n"
        '    name = "resolved_ast",\n'
        f"{render_hdrs(rule_a_hdrs)}\n"
        '    strip_include_prefix = "/include",\n'
        f"{render_deps([
            '@com_google_absl//absl/status:statusor',
            '@com_google_absl//absl/types:span',
            '@com_google_protobuf//:protobuf',
        ])}\n"
        ")"
    )
    rule_b = (
        "cc_library(\n"
        '    name = "resolved_node_kind_cc_proto",\n'
        f"{render_hdrs(rule_b_hdrs)}\n"
        '    strip_include_prefix = "/include",\n'
        f"{render_deps(['@com_google_protobuf//:protobuf'])}\n"
        ")"
    )
    return (
        f"{HEADER_DEPS_COMMENT}\n"
        'load("@rules_cc//cc:defs.bzl", "cc_library")\n\n'
        'package(default_visibility = ["//visibility:public"])\n\n'
        f"{rule_a}\n\n{rule_b}\n"
    )


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        required=True,
        type=pathlib.Path,
        help="Path to the staged @googlesql_prebuilt_linux_amd64 repo root",
    )
    args = parser.parse_args(argv)
    root = args.repo_root.resolve()
    if not root.is_dir():
        print(f"--repo-root {root} is not a directory", file=sys.stderr)
        return 2
    public_build = render_public_build(root)
    resolved_build = render_resolved_ast_build(root)
    (root / "googlesql" / "public").mkdir(parents=True, exist_ok=True)
    (root / "googlesql" / "resolved_ast").mkdir(parents=True, exist_ok=True)
    (root / "googlesql" / "public" / "BUILD.bazel").write_text(public_build)
    (root / "googlesql" / "resolved_ast" / "BUILD.bazel").write_text(
        resolved_build
    )
    print(f"wrote {root}/googlesql/public/BUILD.bazel")
    print(f"wrote {root}/googlesql/resolved_ast/BUILD.bazel")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
