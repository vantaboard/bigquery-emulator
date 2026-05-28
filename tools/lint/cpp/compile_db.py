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


def _run_aquery(target: str) -> dict:
    """Run `bazel aquery --output=jsonproto` and return the parsed JSON.

    We deliberately avoid `--output=textproto` here — the JSON
    encoding is stable across Bazel versions and lets us walk the
    response with the stdlib alone.
    """

    cmd = [
        "bazel",
        "aquery",
        "--output=jsonproto",
        f'mnemonic("CppCompile", deps({target}))',
    ]
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


def _extract_actions(aquery: dict, repo_root: Path) -> list[dict]:
    """Translate `aquery` actions into a JSON compilation database.

    The aquery JSON encoding stores artifacts and arguments
    separately so a single string-table reuses paths across
    actions. We resolve each action's source TU by matching its
    `inputDepSetIds` against the artifact table.
    """

    artifacts = {a["id"]: a["execPath"] for a in aquery.get("artifacts", [])}
    path_fragments = {f["id"]: f for f in aquery.get("pathFragments", [])}

    def resolve(frag_id: int) -> str:
        # Path fragments form a linked list of components; walking
        # them yields the full repo-relative artifact path that
        # `compile_commands.json` needs for clang-tidy to find the
        # source file.
        parts: list[str] = []
        while frag_id:
            frag = path_fragments[frag_id]
            parts.append(frag["label"])
            frag_id = frag.get("parentId", 0)
        return "/".join(reversed(parts))

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
        # `compile_commands.json` requires either a `command` string
        # or an `arguments` list. We use the latter so quoting stays
        # unambiguous on long invocations with embedded spaces.
        out.append(
            {
                "directory": str(repo_root),
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
        help="Bazel label whose dependency graph clang-tidy should index",
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Destination path for the generated compile_commands.json",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[3]
    if not (repo_root / "MODULE.bazel").exists():
        _eprint(f"compile_db: cannot find MODULE.bazel above {__file__}")
        return 1

    os.chdir(repo_root)
    aquery = _run_aquery(args.target)
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
