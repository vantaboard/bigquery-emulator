#!/usr/bin/env python3
"""Emit `manifest.json` for the GoogleSQL prebuilt artifact.

Schema is frozen in `docs/dev/googlesql-prebuilt/manifest.md` (Phase 1).
This script walks the staged repo root, computes SHA-256 of every file
shipped under `include/`, `lib/`, plus the wrapper extras (BUILD.bazel,
MODULE.bazel, LICENSES/), and writes a single `manifest.json` with the
closed schema.

The script is also the schema validator: it refuses to write a manifest
with unknown top-level fields, missing required fields, wrong types,
empty `compat.labels`, or unsorted payload lists (the manifest is
deterministic-by-construction).

Usage:
    python3 manifest_writer.py --repo-root PATH --config CONFIG.json

CONFIG.json is the producer's configuration (per-run inputs that the
script does not derive from disk). See manifest.md for the field
descriptions; example layout:

    {
      "artifact_version": "0.1.0",
      "googlesql": {
        "module_version": "2026.1.1",
        "upstream_tag": "2026.01.1",
        "commit": "36dd14aa0657ea299725504bc0f938732f58f380",
        "repo_url": "https://github.com/google/googlesql",
        "patches": [{"sha256": "...", "description": "..."}]
      },
      "emulator": {"min_commit": "...", "max_commit": null},
      "platform": {"os": "linux", "arch": "amd64",
                   "libc": "glibc-2.31", "cxx_abi": "cxx11"},
      "toolchain": {"compiler": "clang", "compiler_version": "18.1.8",
                    "bazel_version": "7.6.1",
                    "cflags": ["-O2", "-fPIC"], "linkflags": []},
      "producer": {"workflow": "googlesql-prebuilt.yml",
                   "run_id": "0",
                   "build_timestamp": "2026-05-27T00:00:00Z",
                   "host_os_release": "Ubuntu 24.04 LTS"},
      "bundled_thirdparty_deps": []
    }
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import sys


SCHEMA_VERSION = "1"


# Frozen `compat.labels` set. Per Phase 1
# (`docs/dev/googlesql-prebuilt/label-inventory.md` + `manifest.md`),
# these are the 18 direct labels the prebuilt artifact must expose.
# Adding or removing a row here without also touching the label
# inventory and the wrapper BUILD files is a Phase 1 break.
COMPAT_LABELS: list[str] = [
    "//googlesql/public:analyzer",
    "//googlesql/public:analyzer_options",
    "//googlesql/public:analyzer_output",
    "//googlesql/public:builtin_function_options",
    "//googlesql/public:catalog",
    "//googlesql/public:error_helpers",
    "//googlesql/public:error_location_cc_proto",
    "//googlesql/public:evaluator",
    "//googlesql/public:evaluator_base",
    "//googlesql/public:evaluator_table_iterator",
    "//googlesql/public:function",
    "//googlesql/public:language_options",
    "//googlesql/public:options_cc_proto",
    "//googlesql/public:simple_catalog",
    "//googlesql/public:type",
    "//googlesql/public:value",
    "//googlesql/resolved_ast:resolved_ast",
    "//googlesql/resolved_ast:resolved_node_kind_cc_proto",
]


# Closed top-level field set. The manifest schema is deliberately
# closed (Phase 1 manifest.md: "unknown top-level fields are a Phase-5
# validation error"); the producer refuses to emit a manifest with an
# unknown key, and the verifier refuses to load one.
TOP_LEVEL_FIELDS = frozenset(
    {
        "schema_version",
        "artifact_version",
        "googlesql",
        "emulator",
        "compat",
        "platform",
        "toolchain",
        "payload",
        "producer",
        "bundled_thirdparty_deps",
    }
)

SEMVER_RE = re.compile(r"^\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?$")
SHA40_RE = re.compile(r"^[0-9a-f]{40}$")
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")


def sha256_file(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def sorted_payload(
    repo_root: pathlib.Path,
    rel_paths: list[pathlib.Path],
    *,
    include_size: bool,
) -> list[dict]:
    """Return a deterministic sorted list of payload entries.

    Each entry is `{path, sha256[, size_bytes]}`. Paths are stored
    POSIX-style (forward slashes) regardless of host platform so the
    manifest is byte-identical between runs on different hosts.
    """
    out: list[dict] = []
    for rel in sorted(rel_paths, key=lambda p: p.as_posix()):
        full = repo_root / rel
        if not full.is_file():
            raise SystemExit(
                f"manifest_writer: payload path {rel} missing under {repo_root}"
            )
        entry: dict = {
            "path": rel.as_posix(),
            "sha256": sha256_file(full),
        }
        if include_size:
            entry["size_bytes"] = full.stat().st_size
        out.append(entry)
    return out


def collect_payload(repo_root: pathlib.Path) -> dict:
    """Walk the staged repo root and build the `payload` field."""
    headers: list[pathlib.Path] = []
    libraries: list[pathlib.Path] = []
    extras: list[pathlib.Path] = []
    for f in sorted(repo_root.rglob("*")):
        if not f.is_file():
            continue
        rel = f.relative_to(repo_root)
        # `manifest.json` is NOT in the payload (its authenticity is
        # established by the surrounding tarball's SHA, see manifest.md).
        if rel.as_posix() == "manifest.json":
            continue
        if rel.parts and rel.parts[0] == "include":
            headers.append(rel)
        elif rel.parts and rel.parts[0] == "lib":
            libraries.append(rel)
        else:
            extras.append(rel)
    if not headers:
        raise SystemExit(
            "manifest_writer: include/ tree is empty; refusing to write manifest"
        )
    if not libraries:
        raise SystemExit(
            "manifest_writer: lib/ tree is empty; refusing to write manifest"
        )
    return {
        "headers": sorted_payload(repo_root, headers, include_size=False),
        "libraries": sorted_payload(repo_root, libraries, include_size=True),
        "extras": sorted_payload(repo_root, extras, include_size=False),
    }


def validate_config(config: dict) -> None:
    """Per-field validation. Refuses unknown / missing / malformed fields."""
    required = {
        "artifact_version",
        "googlesql",
        "emulator",
        "platform",
        "toolchain",
        "producer",
        "bundled_thirdparty_deps",
    }
    missing = required - config.keys()
    if missing:
        raise SystemExit(
            f"manifest_writer: config missing required fields: "
            f"{sorted(missing)}"
        )
    unknown = config.keys() - (required | {"compat", "schema_version"})
    if unknown:
        raise SystemExit(
            f"manifest_writer: config has unknown top-level fields: "
            f"{sorted(unknown)}"
        )

    if not isinstance(config["artifact_version"], str) or not SEMVER_RE.match(
        config["artifact_version"]
    ):
        raise SystemExit(
            f"manifest_writer: artifact_version must be strict semver "
            f"(MAJOR.MINOR.PATCH[-prerelease]); got "
            f"{config['artifact_version']!r}"
        )

    gs = config["googlesql"]
    gs_keys = {"module_version", "upstream_tag", "commit", "repo_url", "patches"}
    if not isinstance(gs, dict) or set(gs.keys()) != gs_keys:
        raise SystemExit(
            f"manifest_writer: googlesql must contain exactly {sorted(gs_keys)}; "
            f"got {sorted(gs.keys()) if isinstance(gs, dict) else type(gs)}"
        )
    if not SHA40_RE.match(gs["commit"]):
        raise SystemExit(
            f"manifest_writer: googlesql.commit must be 40 hex chars; "
            f"got {gs['commit']!r}"
        )
    if not isinstance(gs["patches"], list):
        raise SystemExit("manifest_writer: googlesql.patches must be a list")
    for patch in gs["patches"]:
        if (
            not isinstance(patch, dict)
            or set(patch.keys()) != {"sha256", "description"}
            or not SHA256_RE.match(patch["sha256"])
        ):
            raise SystemExit(
                "manifest_writer: each patch must be "
                "{sha256: <64 hex>, description: <str>}"
            )

    em = config["emulator"]
    if not isinstance(em, dict) or set(em.keys()) != {"min_commit", "max_commit"}:
        raise SystemExit(
            "manifest_writer: emulator must contain exactly "
            "{min_commit, max_commit}"
        )
    if not SHA40_RE.match(em["min_commit"]):
        raise SystemExit(
            f"manifest_writer: emulator.min_commit must be 40 hex chars; "
            f"got {em['min_commit']!r}"
        )
    if em["max_commit"] is not None and not SHA40_RE.match(em["max_commit"]):
        raise SystemExit(
            "manifest_writer: emulator.max_commit must be null or 40 hex chars"
        )

    pf = config["platform"]
    pf_keys = {"os", "arch", "libc", "cxx_abi"}
    if not isinstance(pf, dict) or set(pf.keys()) != pf_keys:
        raise SystemExit(
            f"manifest_writer: platform must contain exactly {sorted(pf_keys)}"
        )
    if pf["os"] != "linux" or pf["arch"] != "amd64":
        raise SystemExit(
            "manifest_writer: Phase 1 freezes platform to linux/amd64; "
            f"got {pf['os']}/{pf['arch']}"
        )

    tc = config["toolchain"]
    tc_keys = {
        "compiler",
        "compiler_version",
        "bazel_version",
        "cflags",
        "linkflags",
    }
    if not isinstance(tc, dict) or set(tc.keys()) != tc_keys:
        raise SystemExit(
            f"manifest_writer: toolchain must contain exactly {sorted(tc_keys)}"
        )
    if tc["compiler"] != "clang":
        raise SystemExit(
            "manifest_writer: Phase 1 freezes toolchain.compiler to clang"
        )
    for fld in ("cflags", "linkflags"):
        if not isinstance(tc[fld], list):
            raise SystemExit(f"manifest_writer: toolchain.{fld} must be a list")

    pr = config["producer"]
    pr_keys = {"workflow", "run_id", "build_timestamp", "host_os_release"}
    if not isinstance(pr, dict) or set(pr.keys()) != pr_keys:
        raise SystemExit(
            f"manifest_writer: producer must contain exactly {sorted(pr_keys)}"
        )

    if not isinstance(config["bundled_thirdparty_deps"], list):
        raise SystemExit(
            "manifest_writer: bundled_thirdparty_deps must be a list"
        )


def validate_manifest(manifest: dict) -> None:
    """Validate an already-assembled manifest dict.

    Used by `verify.sh` (via `--validate-only PATH`) to gate the verifier
    on the same closed-schema rules the producer applies. Raises SystemExit
    with a precise diagnostic naming the offending field on failure.
    """
    if not isinstance(manifest, dict):
        raise SystemExit("manifest_writer: manifest must be a JSON object")
    unknown = manifest.keys() - TOP_LEVEL_FIELDS
    if unknown:
        raise SystemExit(
            f"manifest_writer: manifest has unknown top-level fields: "
            f"{sorted(unknown)}"
        )
    missing = TOP_LEVEL_FIELDS - manifest.keys()
    if missing:
        raise SystemExit(
            f"manifest_writer: manifest missing required top-level fields: "
            f"{sorted(missing)}"
        )
    if manifest["schema_version"] != SCHEMA_VERSION:
        raise SystemExit(
            f"manifest_writer: schema_version must be {SCHEMA_VERSION!r}; "
            f"got {manifest['schema_version']!r}"
        )
    if not isinstance(manifest["compat"], dict) or set(
        manifest["compat"].keys()
    ) != {"labels"}:
        raise SystemExit(
            "manifest_writer: compat must contain exactly {labels}"
        )
    if list(manifest["compat"]["labels"]) != COMPAT_LABELS:
        raise SystemExit(
            "manifest_writer: compat.labels diverged from the Phase 1 set "
            "(label-inventory.md). Expected sequence:\n"
            + "\n".join(f"  - {label}" for label in COMPAT_LABELS)
            + "\nGot:\n"
            + "\n".join(
                f"  - {label}" for label in manifest["compat"]["labels"]
            )
        )
    payload = manifest["payload"]
    if not isinstance(payload, dict) or set(payload.keys()) != {
        "headers",
        "libraries",
        "extras",
    }:
        raise SystemExit(
            "manifest_writer: payload must contain exactly "
            "{headers, libraries, extras}"
        )
    for key in ("headers", "libraries", "extras"):
        items = payload[key]
        if not isinstance(items, list):
            raise SystemExit(
                f"manifest_writer: payload.{key} must be a list"
            )
        for it in items:
            if (
                not isinstance(it, dict)
                or "path" not in it
                or "sha256" not in it
                or not SHA256_RE.match(it["sha256"])
            ):
                raise SystemExit(
                    f"manifest_writer: payload.{key} entries must include "
                    "{path, sha256:<64 hex>}; offender: " + repr(it)
                )
            if key == "libraries" and not isinstance(
                it.get("size_bytes"), int
            ):
                raise SystemExit(
                    "manifest_writer: payload.libraries entries require "
                    "an integer size_bytes; offender: " + repr(it)
                )
            # Disallow path traversal: no leading slash, no .. segment.
            rel = pathlib.PurePosixPath(it["path"])
            if rel.is_absolute() or any(part == ".." for part in rel.parts):
                raise SystemExit(
                    f"manifest_writer: payload.{key} path {it['path']!r} "
                    "escapes the artifact root (refusing)"
                )
    if not isinstance(manifest["bundled_thirdparty_deps"], list):
        raise SystemExit(
            "manifest_writer: bundled_thirdparty_deps must be a list"
        )


def build_manifest(repo_root: pathlib.Path, config: dict) -> dict:
    """Assemble the final manifest dict from validated config + payload walk."""
    validate_config(config)
    manifest: dict = {
        "schema_version": SCHEMA_VERSION,
        "artifact_version": config["artifact_version"],
        "googlesql": config["googlesql"],
        "emulator": config["emulator"],
        "compat": {"labels": list(COMPAT_LABELS)},
        "platform": config["platform"],
        "toolchain": config["toolchain"],
        "payload": collect_payload(repo_root),
        "producer": config["producer"],
        "bundled_thirdparty_deps": config["bundled_thirdparty_deps"],
    }
    # Final sanity gate: every key must be in the closed top-level set.
    unknown = manifest.keys() - TOP_LEVEL_FIELDS
    if unknown:
        raise SystemExit(
            f"manifest_writer: assembled manifest has unknown fields: "
            f"{sorted(unknown)}"
        )
    return manifest


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        type=pathlib.Path,
        help="Path to the staged @googlesql_prebuilt_linux_amd64 repo root",
    )
    parser.add_argument(
        "--config",
        type=pathlib.Path,
        help="Path to JSON config with per-run producer inputs",
    )
    parser.add_argument(
        "--out",
        type=pathlib.Path,
        help="Output manifest.json path (defaults to <repo-root>/manifest.json)",
    )
    parser.add_argument(
        "--validate-only",
        type=pathlib.Path,
        help=(
            "Validate an existing manifest.json against the closed schema "
            "and exit. Mutually exclusive with --repo-root/--config."
        ),
    )
    args = parser.parse_args(argv)
    if args.validate_only:
        if args.repo_root or args.config or args.out:
            print(
                "--validate-only is mutually exclusive with --repo-root/"
                "--config/--out",
                file=sys.stderr,
            )
            return 2
        manifest = json.loads(args.validate_only.read_text())
        validate_manifest(manifest)
        print(f"{args.validate_only}: manifest schema OK")
        return 0
    if not args.repo_root or not args.config:
        print(
            "--repo-root and --config are required (unless --validate-only)",
            file=sys.stderr,
        )
        return 2
    root = args.repo_root.resolve()
    if not root.is_dir():
        print(f"--repo-root {root} is not a directory", file=sys.stderr)
        return 2
    config = json.loads(args.config.read_text())
    manifest = build_manifest(root, config)
    out_path = args.out or (root / "manifest.json")
    # Deterministic emit: sort_keys=False (we control insertion order
    # above so the field order matches manifest.md's example exactly),
    # explicit indent + separators so reformatters can't drift this.
    out_path.write_text(
        json.dumps(manifest, indent=2, separators=(",", ": ")) + "\n"
    )
    print(f"wrote {out_path} ({out_path.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
