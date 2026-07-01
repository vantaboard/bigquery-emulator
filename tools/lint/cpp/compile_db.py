#!/usr/bin/env python3
"""Generate compile_commands.json from a Bazel `aquery` for the engine.

`task lint:cpp:tidy` needs a JSON compilation database at the repo
root. Most Bazel C++ projects pull in `hedronvision/bazel-compile-
commands-extractor` to produce one, but adding a third-party Bazel
module solely to feed clang-tidy would duplicate dependency surface
the rest of the repo does not need yet (and would slow down cold
builds). This script keeps the rollout build-system-light: it shells
out to `bazel aquery --output=jsonproto` for the canonical engine
target, walks the resulting `CppCompile` actions, and writes the
standard `compile_commands.json` format that clang-tidy / clangd
consume.

The implementation is intentionally narrow:

  * The Python file has no third-party deps; everything below uses
    only the stdlib so a fresh checkout can run it without
    bootstrapping a venv.
  * It calls `bazel aquery` directly rather than via `task bazel:*`
    because it does not need the throttled-build wrappers — `aquery`
    is a static analysis pass that does NOT execute compile actions.
  * The output is filtered to first-party translation units (under
    `binaries/`, `backend/`, `frontend/`, or
    `tools/googlesql-prebuilt/smoke/`). Vendored / generated /
    GoogleSQL TUs would otherwise generate noise during clang-tidy
    runs.
  * If `bazel aquery` reports zero `CppCompile` actions for the
    target the script exits non-zero — almost always because the
    target failed to configure.

Usage:

    tools/lint/cpp/compile_db.py \
        --target //binaries/emulator_main:emulator_main \
        --output compile_commands.json

The script is idempotent. Re-running it overwrites the previous
output atomically (write to `<output>.tmp`, then rename).
"""

from __future__ import annotations

import argparse
import json
import os
import shlex
import subprocess
import sys
from pathlib import Path

# First-party include roots. Mirrors
# `tools/lint/cpp/sources.go::firstPartyIncludeRoots`. Translation
# units whose source path does not start with one of these prefixes
# are dropped before writing the compilation database, so clang-tidy
# does not iterate over upstream code.
FIRST_PARTY_PREFIXES = (
    "backend/",
    "binaries/",
    "frontend/",
    "tools/googlesql-prebuilt/smoke/",
)


def _eprint(msg: str) -> None:
    print(msg, file=sys.stderr)


def _run_aquery(targets: list[str], bazel_config: str | None) -> dict:
    """Run `bazel aquery --output=jsonproto` and return the parsed JSON.

    `targets` is a list of Bazel labels — the script unions their
    transitive `deps()` so the resulting compilation database covers
    both the production binary and every first-party `cc_test`. The
    test-target half matters because `task lint:cpp:tidy` lints
    `*_test.cc` files alongside production sources; without the test
    deps the test TUs would either be missing from the DB entirely
    or fall back to clang's default include chain (no `gtest/gtest.h`,
    no `gmock/gmock.h`).

    We deliberately avoid `--output=textproto` here — the JSON
    encoding is stable across Bazel versions and lets us walk the
    response with the stdlib alone.
    """

    if not targets:
        _eprint("compile_db: at least one --target is required")
        sys.exit(2)
    expr_inner = " + ".join(f"deps({t})" for t in targets)
    cmd = [
        "bazel",
        "aquery",
        "--output=jsonproto",
        f'mnemonic("CppCompile", {expr_inner})',
    ]
    if bazel_config:
        cmd[2:2] = ["--config", bazel_config]
    proc = subprocess.run(
        cmd,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if proc.returncode != 0:
        _eprint("compile_db: `bazel aquery` failed:")
        _eprint("  command: " + " ".join(shlex.quote(c) for c in cmd))
        _eprint(proc.stderr.decode("utf-8", errors="replace"))
        sys.exit(proc.returncode or 1)
    raw = proc.stdout.decode("utf-8", errors="replace").strip() or "{}"
    return json.loads(raw)


def _promote_external_includes_to_system(args: list[str]) -> list[str]:
    """Outrank `/usr/include`-resident copies of bazel-built deps.

    Bazel emits external deps as `-iquote <path>` (quote-only). That
    is fine for first-party `#include "absl/log/log.h"` style
    references, but angle-form `#include <absl/...>` inside protobuf
    /  googlesql falls back to clang's default system search path —
    which on a stock Ubuntu box is `/usr/include/absl/...` (system
    Abseil 20220623, well older than what protobuf expects). The
    resulting compile-time `static_assert` errors and `enum-cast` ana-
    lyzer hits flood `task lint:cpp:tidy`.

    The fix promotes every `-iquote <path>` whose `<path>` lives
    under `external/` or `bazel-out/.../external/` to also be emitted
    as `-isystem <path>` at the front of the args (right after the
    compiler), so angle-include lookup resolves through the
    bazel-built copy before clang falls back to its default include
    chain. Treating them as "system" headers also suppresses
    warnings from those translation units, which we never want to
    surface anyway.
    """

    if not args:
        return args
    compiler, rest = args[0], args[1:]
    system_prefix: list[str] = []
    seen: set[str] = set()
    i = 0
    while i < len(rest):
        a = rest[i]
        if a == "-iquote" and i + 1 < len(rest):
            path = rest[i + 1]
            if (
                "external/" in path or path.startswith("external/")
            ) and path not in seen:
                system_prefix.extend(["-isystem", path])
                seen.add(path)
            i += 2
            continue
        i += 1
    return [compiler, *system_prefix, *rest]


_PATH_FLAG_OPTS = frozenset(
    {
        "-I",
        "-iquote",
        "-isystem",
        "-idirafter",
        "-include",
        "-include-pch",
    }
)

# Flags newer Bazel/clang toolchains emit that older clang-tidy builds
# reject outright (before include paths are parsed).
_CLANG_TIDY_UNSUPPORTED_OPTS = frozenset(
    {
        "-fno-canonical-system-headers",
    }
)


def _sanitize_args_for_clang_tidy(args: list[str]) -> list[str]:
    """Drop compile-only flags clang-tidy cannot parse."""

    return [a for a in args if a not in _CLANG_TIDY_UNSUPPORTED_OPTS]


def _bazel_output_base(repo_root: Path) -> Path:
    """Return Bazel's `output_base` (where `external/` repos are stored)."""

    proc = subprocess.run(
        ["bazel", "info", "output_base"],
        cwd=repo_root,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if proc.returncode != 0:
        _eprint("compile_db: `bazel info output_base` failed:")
        _eprint(proc.stderr.decode("utf-8", errors="replace"))
        sys.exit(proc.returncode or 1)
    return Path(proc.stdout.strip())


def _resolve_bazel_relative_path(
    path: str, exec_root: Path, output_base: Path, repo_root: Path
) -> str:
    """Map execroot-relative `external/` / `bazel-out/` paths to absolutes.

    Bazel's execroot only symlinks a subset of `external/` repos at any
    given time (whatever the last configure/build touched). The canonical
    checkout lives under `<output_base>/external/`, which persists across
    builds. clang-tidy runs can happen hours after `aquery`, so we
    absolutize include paths against both locations.
    """

    if not path or path.startswith("/"):
        return path
    if path.startswith("external/"):
        for base in (exec_root, output_base):
            candidate = base / path
            if candidate.exists():
                return str(candidate.resolve())
        return str((output_base / path).resolve())
    if path.startswith("bazel-out/"):
        for base in (
            exec_root,
            output_base / "execroot" / "_main",
            repo_root,
        ):
            candidate = base / path
            if candidate.exists():
                return str(candidate.resolve())
        return str((exec_root / path).resolve())
    # Workspace-relative paths (`.cache/googlesql-prebuilt/...`, etc.).
    for base in (exec_root, repo_root):
        candidate = base / path
        if candidate.exists():
            return str(candidate.resolve())
    return str((repo_root / path).resolve())


def _canonicalize_compile_args(
    args: list[str], exec_root: Path, output_base: Path, repo_root: Path
) -> list[str]:
    """Rewrite `-I`/`-isystem`/`-iquote` operands as absolute paths."""

    out: list[str] = []
    i = 0
    while i < len(args):
        arg = args[i]
        if arg in _PATH_FLAG_OPTS and i + 1 < len(args):
            out.append(arg)
            out.append(
                _resolve_bazel_relative_path(
                    args[i + 1], exec_root, output_base, repo_root
                )
            )
            i += 2
            continue
        if (
            arg.startswith("-I")
            and len(arg) > 2
            and not arg.startswith("-isystem")
        ):
            out.append(
                "-I"
                + _resolve_bazel_relative_path(
                    arg[2:], exec_root, output_base, repo_root
                )
            )
            i += 1
            continue
        out.append(arg)
        i += 1
    return out


def _bazel_exec_root(repo_root: Path) -> Path:
    """Resolve the path that every `external/...` / `bazel-out/...`
    argument in a `bazel aquery` response is relative to.

    `bazel aquery` emits compile-action paths the same way Bazel
    spawns the actual compile subprocess: relative to the **execroot**
    (`<bazel cache>/execroot/_main`), not the workspace root. The
    workspace root only contains `bazel-out/...` indirectly (via the
    `bazel-out` convenience symlink) and never contains `external/...`
    at all — the source headers under `external/abseil-cpp~/absl/log/`
    live inside the execroot's `external/` directory, while the
    workspace-root `bazel-out/.../bin/external/abseil-cpp~/` only
    holds the compiled outputs (`.so` + `.cppmap`).

    Without this resolution clang-tidy CDs into the workspace root,
    `-iquote external/abseil-cpp~` resolves to a non-existent
    directory, and angle-form `#include <absl/log/absl_check.h>`
    inside protobuf either falls back to a stale `/usr/include/absl`
    or fails to find the header at all.

    Prefer the `bazel-<workspace>` convenience symlink Bazel writes
    next to the `MODULE.bazel`; fall back to the workspace root if
    the symlink is missing (cold checkout where no `bazel build` has
    materialised it yet — the caller surfaces the resulting error
    when aquery itself fails).
    """

    candidate = repo_root / f"bazel-{repo_root.name}"
    if candidate.is_symlink() or candidate.is_dir():
        return candidate.resolve()
    return repo_root


def _extract_actions(aquery: dict, repo_root: Path) -> list[dict]:
    """Translate `aquery` actions into a JSON compilation database.

    Each `CppCompile` action carries the source translation unit
    inline as the `-c <path>` argument pair, so we can resolve the
    source path from `arguments` alone without walking the
    `artifacts` / `pathFragments` tables. That keeps the code
    forward-compatible across Bazel releases (the artifact-table
    schema has changed several times — e.g. older versions emit
    `execPath` inline while newer ones emit `pathFragmentId`
    references that need a separate walk).
    """

    exec_root = _bazel_exec_root(repo_root)
    output_base = _bazel_output_base(repo_root)
    directory = str(exec_root)
    out: list[dict] = []
    for action in aquery.get("actions", []):
        if action.get("mnemonic") != "CppCompile":
            continue
        args = list(action.get("arguments", []))
        if not args:
            continue
        source = _source_from_args(args)
        if source is None:
            continue
        if not any(source.startswith(p) for p in FIRST_PARTY_PREFIXES):
            continue
        args = _promote_external_includes_to_system(args)
        args = _canonicalize_compile_args(args, exec_root, output_base, repo_root)
        args = _sanitize_args_for_clang_tidy(args)
        # `compile_commands.json` requires either a `command` string
        # or an `arguments` list. We use the latter so quoting stays
        # unambiguous on long invocations with embedded spaces.
        out.append(
            {
                "directory": directory,
                "file": source,
                "arguments": args,
            }
        )
    return out


def _source_from_args(args: list[str]) -> str | None:
    """Return the first `-c <source>` argument's `<source>` operand.

    `bazel aquery` emits the source TU via `-c <path>` for every
    `CppCompile` action. We pick that path verbatim; the absolute
    repo path resolution lives in `compile_commands.json`'s
    `directory` field.
    """

    for i, arg in enumerate(args):
        if arg == "-c" and i + 1 < len(args):
            return args[i + 1]
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--target",
        required=True,
        action="append",
        help=(
            "Bazel label whose dependency graph clang-tidy should index. "
            "Pass `--target` repeatedly to union the deps of several labels "
            "(e.g. once for the production binary and once per test target)."
        ),
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Destination path for the generated compile_commands.json",
    )
    parser.add_argument(
        "--bazel-config",
        default=None,
        help=(
            "Optional Bazel --config group (e.g. googlesql-prebuilt). "
            "Must match the config used for `bazel query` when discovering "
            "test targets so module resolution sees the same @googlesql graph."
        ),
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[3]
    if not (repo_root / "MODULE.bazel").exists():
        _eprint(f"compile_db: cannot find MODULE.bazel above {__file__}")
        return 1

    os.chdir(repo_root)
    aquery = _run_aquery(args.target, args.bazel_config)
    entries = _extract_actions(aquery, repo_root)
    if not entries:
        _eprint("compile_db: aquery produced 0 first-party CppCompile actions.")
        _eprint(
            "  This usually means the target failed to configure. "
            "Re-run `task emulator:build-engine:bazel` and try again."
        )
        return 1

    tmp = Path(args.output).with_suffix(".json.tmp")
    tmp.write_text(json.dumps(entries, indent=2) + "\n")
    tmp.replace(args.output)
    print(f"compile_db: wrote {len(entries)} entries to {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
