#!/usr/bin/env python3
"""Centralized GoogleSQL prebuilt artifact validator (safety gate).

This is the single validator every prebuilt-consumer points at. The
existing surfaces that previously open-coded slices of this work:

  - `taskfiles/googlesql.yml::fetch-prebuilt` (SHA, module name, schema)
  - `tools/googlesql-prebuilt/verify.sh` (same plus payload SHAs)
  - `Dockerfile::engine-builder-bazel` (SHA, module name)
  - `.github/actions/setup-googlesql/action.yml` (manifest field lift)

All now delegate to `validate_artifact.py validate` so the gates are
applied uniformly and a compatibility-surface schema bump only needs to
flow through this file. The validator never recompiles GoogleSQL; it
operates on the already-unpacked artifact root (typically
`.cache/googlesql-prebuilt/googlesql_prebuilt_linux_amd64/`).

Failure model (the safety-gate "no silent prebuilt->source fallback" contract):

  - Every gate is a hard error. The CLI exits non-zero on the first
    failing gate.
  - Errors are tagged with a stable token (`FAIL_*`) so log consumers
    can grep them and the `--summary` JSON renders an `error_kind` field
    callers can route on.
  - Every failure block prints, in order:
      * Selected mode (`prebuilt` / `source`) and what the consumer
        expected.
      * The exact failing field / payload entry / checksum pair.
      * The explicit escape-hatch command for switching to source mode
        (`GOOGLESQL_SOURCE=local task emulator:build-engine:bazel` for
        local; `gh workflow run release.yml ... -f googlesql_source=true`
        for the release path; etc.).

The validator's design is "stay quiet on the happy path, scream loudly
on the unhappy path" — a passing run prints one summary line so log
consumers can pin artifact identity into CI summaries / Docker build
logs / release notes without parsing the manifest themselves.
"""

from __future__ import annotations

import argparse
import dataclasses
import hashlib
import json
import pathlib
import re
import sys
from typing import Iterable

# Re-use the closed-schema validator + canonical compat-label set the
# producer already enforces. The safety gate reuses these so a schema
# drift can only flow through `manifest_writer.py`.
SCRIPT_DIR = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))
import manifest_writer  # noqa: E402  (sibling import after sys.path tweak)


# The compatibility surface freezes platform = linux/amd64 and
# toolchain.compiler = clang. The safety gate enforces "consumer expects
# what the artifact claims"; the defaults below are the consumer-side
# expectations and match what every entrypoint (local task, CI, Docker,
# release) targets today. A future
# arm64 lane (per upgrade-rules.md) lights up by extending this map.
EXPECTED_PLATFORM = {
    "os": "linux",
    "arch": "amd64",
}
EXPECTED_COMPILER = "clang"

REPO_NAME = "googlesql_prebuilt_linux_amd64"

WRAPPER_REQUIRED_FILES = (
    "MODULE.bazel",
    "BUILD.bazel",
    "googlesql/public/BUILD.bazel",
    "googlesql/resolved_ast/BUILD.bazel",
)

# Frozen by docs/dev/googlesql-prebuilt/label-inventory.md § Direct labels.
# Each entry must appear as `name = "<target>"` in the BUILD file so
# consumer `@googlesql//...` labels resolve under prebuilt mode.
WRAPPER_REQUIRED_TARGETS: dict[str, tuple[str, ...]] = {
    "googlesql/public/BUILD.bazel": (
        "analyzer",
        "analyzer_options",
        "analyzer_output",
        "builtin_function_options",
        "catalog",
        "civil_time",
        "error_helpers",
        "error_location_cc_proto",
        "evaluator_table_iterator",
        "function",
        "interval_value",
        "language_options",
        "numeric_value",
        "options_cc_proto",
        "simple_catalog",
        "type",
        "type_cc_proto",
        "uuid_value",
        "value",
    ),
    "googlesql/resolved_ast/BUILD.bazel": (
        "resolved_ast",
        "resolved_node_kind_cc_proto",
    ),
}

TARGET_NAME_RE = re.compile(r'^\s*name\s*=\s*"([^"]+)"', re.MULTILINE)

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
SHA40_RE = re.compile(r"^[0-9a-f]{40}$")
SEMVER_RE = re.compile(r"^\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?$")


# --- failure taxonomy -------------------------------------------------------
#
# `error_kind` values surface in `--summary` JSON output and the human
# diagnostic header. Each value is a stable token; downstream consumers
# (e.g. release dashboards, parity jobs) may match on them.

FAIL_REPO_ROOT = "FAIL_REPO_ROOT"  # repo root missing / not a directory
FAIL_MANIFEST_MISSING = "FAIL_MANIFEST_MISSING"  # manifest.json absent
FAIL_MANIFEST_PARSE = "FAIL_MANIFEST_PARSE"  # manifest.json not JSON
FAIL_SCHEMA = "FAIL_SCHEMA"  # closed-schema validator rejected
FAIL_IDENTITY_PIN = "FAIL_IDENTITY_PIN"  # commit/version pin mismatch
FAIL_PLATFORM = "FAIL_PLATFORM"  # os/arch/libc/compiler mismatch
FAIL_WRAPPER_MISSING = "FAIL_WRAPPER_MISSING"  # required BUILD/MODULE file absent
FAIL_WRAPPER_TARGET = "FAIL_WRAPPER_TARGET"  # required cc_library name absent
FAIL_PAYLOAD_MISSING = "FAIL_PAYLOAD_MISSING"  # manifest entry has no file
FAIL_PAYLOAD_SHA = "FAIL_PAYLOAD_SHA"  # SHA-256 mismatch on payload
FAIL_PAYLOAD_SIZE = "FAIL_PAYLOAD_SIZE"  # size_bytes mismatch on library
FAIL_PAYLOAD_ESCAPE = "FAIL_PAYLOAD_ESCAPE"  # payload path escapes root
FAIL_PAYLOAD_UNACCOUNTED = "FAIL_PAYLOAD_UNACCOUNTED"  # on-disk file missing from manifest


class ValidationError(Exception):
    """Raised when any gate fails.

    Carries both a stable token (`kind`) and the full multi-line
    diagnostic the CLI prints. `field` names the specific knob that
    failed; `actual`/`expected` are stringified for the diagnostic.
    """

    def __init__(
        self,
        kind: str,
        field: str,
        expected: object,
        actual: object,
        details: str = "",
    ) -> None:
        self.kind = kind
        self.field = field
        self.expected = expected
        self.actual = actual
        self.details = details
        super().__init__(f"{kind}: {field}")


@dataclasses.dataclass(frozen=True)
class ExpectedPin:
    """Consumer-side pin set the validator checks the manifest against.

    Every field is optional: callers that only need a payload-integrity
    pass (e.g. local `task googlesql:fetch-prebuilt` immediately after
    download) skip the identity pins. Callers that need release-grade
    gating (e.g. `.github/workflows/release.yml`) fill them all in.
    """

    schema_version: str | None = None  # default: whatever manifest_writer pins
    artifact_version: str | None = None
    googlesql_commit: str | None = None  # 40 hex; matched case-insensitively
    googlesql_tag: str | None = None  # upstream_tag string match
    os: str | None = None  # default: linux
    arch: str | None = None  # default: amd64
    libc: str | None = None  # exact string match when set
    cxx_abi: str | None = None
    compiler: str | None = None  # default: clang
    compiler_version: str | None = None  # prefix match (e.g. "18" == "18.1.8")
    bazel_version: str | None = None  # prefix match


@dataclasses.dataclass
class ValidationSummary:
    """One-line summary surfaced to logs / CI step output."""

    mode: str  # always "prebuilt" today; placeholder for future modes
    artifact_version: str
    googlesql_commit: str  # 40 hex
    googlesql_tag: str
    schema_version: str
    platform_os: str
    platform_arch: str
    platform_libc: str
    compiler: str
    compiler_version: str
    bazel_version: str
    artifact_root: str
    headers: int
    libraries: int
    extras: int

    def render_line(self) -> str:
        return (
            f"googlesql-prebuilt: mode=prebuilt "
            f"artifact_version={self.artifact_version} "
            f"googlesql={self.googlesql_commit[:12]} "
            f"tag={self.googlesql_tag} "
            f"schema={self.schema_version} "
            f"platform={self.platform_os}/{self.platform_arch} "
            f"libc={self.platform_libc} "
            f"compiler={self.compiler}-{self.compiler_version} "
            f"bazel={self.bazel_version} "
            f"payload=h{self.headers}/l{self.libraries}/e{self.extras}"
        )

    def to_json(self) -> dict:
        return dataclasses.asdict(self)


def sha256_file(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def _check_pin(
    field: str,
    expected: str | None,
    actual: object,
    *,
    match: str = "equal",
) -> None:
    """Compare an expected pin against the manifest value.

    `match` is one of:
      - "equal":        exact string match (case-insensitive for hex).
      - "prefix":       expected is a prefix of actual (e.g. "18" matches "18.1.8").
      - "hex40":        expected and actual are both 40 hex chars; case-insensitive.
    """
    if expected is None:
        return
    actual_str = "" if actual is None else str(actual)
    ok: bool
    if match == "equal":
        ok = actual_str == expected
    elif match == "prefix":
        ok = actual_str.startswith(expected)
    elif match == "hex40":
        ok = actual_str.lower() == expected.lower() and SHA40_RE.match(
            actual_str.lower()
        ) is not None
    else:
        raise ValueError(f"unknown match mode {match!r}")
    if not ok:
        raise ValidationError(
            kind=FAIL_IDENTITY_PIN,
            field=field,
            expected=expected,
            actual=actual_str,
        )


def _check_platform(manifest: dict, pin: ExpectedPin) -> None:
    """OS/arch/libc/compiler gates. Defaults stay linux/amd64/clang per the compatibility surface."""
    platform = manifest.get("platform", {})
    expected_os = pin.os or EXPECTED_PLATFORM["os"]
    expected_arch = pin.arch or EXPECTED_PLATFORM["arch"]
    actual_os = platform.get("os")
    actual_arch = platform.get("arch")
    if actual_os != expected_os:
        raise ValidationError(
            kind=FAIL_PLATFORM,
            field="platform.os",
            expected=expected_os,
            actual=actual_os,
        )
    if actual_arch != expected_arch:
        raise ValidationError(
            kind=FAIL_PLATFORM,
            field="platform.arch",
            expected=expected_arch,
            actual=actual_arch,
        )
    if pin.libc is not None and platform.get("libc") != pin.libc:
        raise ValidationError(
            kind=FAIL_PLATFORM,
            field="platform.libc",
            expected=pin.libc,
            actual=platform.get("libc"),
        )
    if pin.cxx_abi is not None and platform.get("cxx_abi") != pin.cxx_abi:
        raise ValidationError(
            kind=FAIL_PLATFORM,
            field="platform.cxx_abi",
            expected=pin.cxx_abi,
            actual=platform.get("cxx_abi"),
        )

    toolchain = manifest.get("toolchain", {})
    expected_compiler = pin.compiler or EXPECTED_COMPILER
    if toolchain.get("compiler") != expected_compiler:
        raise ValidationError(
            kind=FAIL_PLATFORM,
            field="toolchain.compiler",
            expected=expected_compiler,
            actual=toolchain.get("compiler"),
        )
    if pin.compiler_version is not None:
        _check_pin(
            "toolchain.compiler_version",
            pin.compiler_version,
            toolchain.get("compiler_version"),
            match="prefix",
        )
    if pin.bazel_version is not None:
        _check_pin(
            "toolchain.bazel_version",
            pin.bazel_version,
            toolchain.get("bazel_version"),
            match="prefix",
        )


def _check_identity(manifest: dict, pin: ExpectedPin) -> None:
    """Pin gates for schema_version, artifact_version, googlesql.*."""
    if pin.schema_version is not None:
        _check_pin(
            "schema_version",
            pin.schema_version,
            manifest.get("schema_version"),
        )
    if pin.artifact_version is not None:
        _check_pin(
            "artifact_version",
            pin.artifact_version,
            manifest.get("artifact_version"),
        )
    gs = manifest.get("googlesql", {})
    if pin.googlesql_commit is not None:
        _check_pin(
            "googlesql.commit",
            pin.googlesql_commit,
            gs.get("commit"),
            match="hex40",
        )
    if pin.googlesql_tag is not None:
        _check_pin(
            "googlesql.upstream_tag",
            pin.googlesql_tag,
            gs.get("upstream_tag"),
        )


def _check_wrappers(repo_root: pathlib.Path) -> None:
    """Every required BUILD/MODULE wrapper file lives inside the artifact root."""
    for rel in WRAPPER_REQUIRED_FILES:
        path = repo_root / rel
        try:
            resolved = path.resolve(strict=True)
        except FileNotFoundError as exc:
            raise ValidationError(
                kind=FAIL_WRAPPER_MISSING,
                field=f"wrapper:{rel}",
                expected=str(repo_root / rel),
                actual="(missing)",
            ) from exc
        # Defense in depth: refuse paths that resolve outside the root
        # (symlink farms etc.). repo-layout.md forbids these.
        try:
            resolved.relative_to(repo_root.resolve())
        except ValueError as exc:
            raise ValidationError(
                kind=FAIL_PAYLOAD_ESCAPE,
                field=f"wrapper:{rel}",
                expected=f"inside {repo_root}",
                actual=str(resolved),
            ) from exc


def _check_wrapper_targets(repo_root: pathlib.Path) -> None:
    """Every direct label from label-inventory.md is declared in BUILD.bazel."""
    for rel, required in WRAPPER_REQUIRED_TARGETS.items():
        path = repo_root / rel
        text = path.read_text(encoding="utf-8")
        declared = set(TARGET_NAME_RE.findall(text))
        missing = [name for name in required if name not in declared]
        if missing:
            raise ValidationError(
                kind=FAIL_WRAPPER_TARGET,
                field=f"wrapper:{rel}",
                expected=", ".join(required),
                actual=f"missing targets: {', '.join(missing)}",
            )


def _check_payload(repo_root: pathlib.Path, manifest: dict) -> None:
    """Existence + SHA-256 + size + path-escape gates for every payload entry.

    Also enforces "no file on disk under the artifact root is left
    unaccounted for" — the producer's contract is a closed manifest
    (see manifest.md), so a stray file is a producer drift bug.
    """
    payload = manifest.get("payload", {})
    repo_root_resolved = repo_root.resolve()
    accounted: set[str] = set()
    for section in ("headers", "libraries", "extras"):
        entries = payload.get(section, [])
        for entry in entries:
            rel = entry.get("path", "")
            posix = pathlib.PurePosixPath(rel)
            if posix.is_absolute() or any(part == ".." for part in posix.parts):
                raise ValidationError(
                    kind=FAIL_PAYLOAD_ESCAPE,
                    field=f"payload.{section}",
                    expected="inside artifact root",
                    actual=rel,
                )
            full = (repo_root / rel).resolve()
            try:
                full.relative_to(repo_root_resolved)
            except ValueError as exc:
                raise ValidationError(
                    kind=FAIL_PAYLOAD_ESCAPE,
                    field=f"payload.{section}",
                    expected=f"inside {repo_root_resolved}",
                    actual=str(full),
                ) from exc
            if not full.is_file():
                raise ValidationError(
                    kind=FAIL_PAYLOAD_MISSING,
                    field=f"payload.{section}",
                    expected=rel,
                    actual="(missing on disk)",
                )
            actual_sha = sha256_file(full)
            if actual_sha != entry.get("sha256"):
                raise ValidationError(
                    kind=FAIL_PAYLOAD_SHA,
                    field=f"payload.{section}:{rel}",
                    expected=entry.get("sha256"),
                    actual=actual_sha,
                )
            if section == "libraries":
                expected_size = entry.get("size_bytes")
                actual_size = full.stat().st_size
                if expected_size != actual_size:
                    raise ValidationError(
                        kind=FAIL_PAYLOAD_SIZE,
                        field=f"payload.libraries:{rel}.size_bytes",
                        expected=expected_size,
                        actual=actual_size,
                    )
            accounted.add(rel)
    # Closed-manifest check: every on-disk file (except manifest.json)
    # must be referenced in payload.{headers,libraries,extras}.
    on_disk: set[str] = set()
    for p in repo_root.rglob("*"):
        if not p.is_file():
            continue
        rel = p.relative_to(repo_root).as_posix()
        if rel == "manifest.json":
            continue
        on_disk.add(rel)
    extra = sorted(on_disk - accounted)
    if extra:
        raise ValidationError(
            kind=FAIL_PAYLOAD_UNACCOUNTED,
            field="payload.unaccounted",
            expected="(empty set)",
            actual=", ".join(extra[:5]) + (" ..." if len(extra) > 5 else ""),
            details=(
                "manifest is closed: every file on disk under the artifact "
                "root must be referenced in payload.{headers,libraries,extras}."
            ),
        )


def _load_manifest(manifest_path: pathlib.Path) -> dict:
    if not manifest_path.is_file():
        raise ValidationError(
            kind=FAIL_MANIFEST_MISSING,
            field="manifest.json",
            expected=str(manifest_path),
            actual="(missing)",
        )
    try:
        return json.loads(manifest_path.read_text())
    except json.JSONDecodeError as exc:
        raise ValidationError(
            kind=FAIL_MANIFEST_PARSE,
            field="manifest.json",
            expected="valid JSON",
            actual=f"json.JSONDecodeError: {exc}",
        ) from exc


def validate(repo_root: pathlib.Path, pin: ExpectedPin) -> ValidationSummary:
    """Run every gate against an unpacked artifact root.

    Returns a populated `ValidationSummary` on success. Raises
    `ValidationError` on the FIRST failing gate so the CLI exits with
    actionable diagnostics instead of dumping a wall of unrelated
    failures.
    """
    if not repo_root.is_dir():
        raise ValidationError(
            kind=FAIL_REPO_ROOT,
            field="--repo-root",
            expected="directory",
            actual=str(repo_root),
        )
    manifest_path = repo_root / "manifest.json"
    manifest = _load_manifest(manifest_path)
    # Schema gate first: every later gate dereferences fields we trust the
    # schema to have shape-checked. Delegate to manifest_writer so the
    # closed-schema rules are defined in exactly one place.
    try:
        manifest_writer.validate_manifest(manifest)
    except SystemExit as exc:
        raise ValidationError(
            kind=FAIL_SCHEMA,
            field="manifest.json",
            expected="manifest_writer.validate_manifest OK",
            actual=str(exc),
        ) from exc

    _check_identity(manifest, pin)
    _check_platform(manifest, pin)
    _check_wrappers(repo_root)
    _check_wrapper_targets(repo_root)
    _check_payload(repo_root, manifest)

    payload = manifest["payload"]
    return ValidationSummary(
        mode="prebuilt",
        artifact_version=manifest["artifact_version"],
        googlesql_commit=manifest["googlesql"]["commit"],
        googlesql_tag=manifest["googlesql"]["upstream_tag"],
        schema_version=manifest["schema_version"],
        platform_os=manifest["platform"]["os"],
        platform_arch=manifest["platform"]["arch"],
        platform_libc=manifest["platform"]["libc"],
        compiler=manifest["toolchain"]["compiler"],
        compiler_version=manifest["toolchain"]["compiler_version"],
        bazel_version=manifest["toolchain"]["bazel_version"],
        artifact_root=str(repo_root.resolve()),
        headers=len(payload["headers"]),
        libraries=len(payload["libraries"]),
        extras=len(payload["extras"]),
    )


def _render_failure(
    err: ValidationError,
    *,
    mode: str,
    expected_summary: list[str],
    escape_hatches: Iterable[str],
) -> str:
    """Multi-line human diagnostic block."""
    lines = [
        f"validate_artifact: {err.kind}",
        f"  selected mode:    {mode}",
    ]
    for line in expected_summary:
        lines.append(f"  expected:         {line}")
    lines.append(f"  failing field:    {err.field}")
    lines.append(f"  expected value:   {err.expected}")
    lines.append(f"  actual value:     {err.actual}")
    if err.details:
        lines.append(f"  notes:            {err.details}")
    lines.append("  escape hatch:")
    for hatch in escape_hatches:
        lines.append(f"    - {hatch}")
    return "\n".join(lines)


def _parse_pin_args(args: argparse.Namespace) -> ExpectedPin:
    """Build an `ExpectedPin` from CLI args."""
    if args.expected_googlesql_sha and not SHA40_RE.match(
        args.expected_googlesql_sha.lower()
    ):
        print(
            f"validate_artifact: --expected-googlesql-sha must be 40 hex chars; "
            f"got {args.expected_googlesql_sha!r}",
            file=sys.stderr,
        )
        sys.exit(2)
    if args.expected_artifact_version and not SEMVER_RE.match(
        args.expected_artifact_version
    ):
        print(
            f"validate_artifact: --expected-artifact-version must be strict "
            f"semver; got {args.expected_artifact_version!r}",
            file=sys.stderr,
        )
        sys.exit(2)
    return ExpectedPin(
        schema_version=args.expected_schema_version,
        artifact_version=args.expected_artifact_version,
        googlesql_commit=args.expected_googlesql_sha,
        googlesql_tag=args.expected_googlesql_tag,
        os=args.expected_os,
        arch=args.expected_arch,
        libc=args.expected_libc,
        cxx_abi=args.expected_cxx_abi,
        compiler=args.expected_compiler,
        compiler_version=args.expected_compiler_version,
        bazel_version=args.expected_bazel_version,
    )


def _default_escape_hatches() -> list[str]:
    return [
        "GOOGLESQL_SOURCE=local task emulator:build-engine:bazel "
        "(local: rebuild from sibling ../googlesql/ checkout)",
        "gh workflow run release.yml -f ref=<tag> -f googlesql_source=true "
        "(release: explicit source rebuild)",
        "task googlesql:clean && task googlesql:fetch-prebuilt URL=<url> "
        "SHA256=<sha> (re-stage a known-good artifact)",
    ]


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        required=True,
        type=pathlib.Path,
        help=(
            "Path to the unpacked artifact root, typically "
            ".cache/googlesql-prebuilt/googlesql_prebuilt_linux_amd64/"
        ),
    )
    parser.add_argument(
        "--expected-schema-version",
        default=manifest_writer.SCHEMA_VERSION,
        help=(
            "Required schema_version in manifest.json (default: matches the "
            "producer's SCHEMA_VERSION)."
        ),
    )
    parser.add_argument(
        "--expected-artifact-version",
        default=None,
        help="Pin to a specific strict-semver artifact_version. Optional.",
    )
    parser.add_argument(
        "--expected-googlesql-sha",
        default=None,
        help="Pin to a specific upstream GoogleSQL commit (40 hex chars).",
    )
    parser.add_argument(
        "--expected-googlesql-tag",
        default=None,
        help="Pin to a specific upstream GoogleSQL tag (e.g. 2026.01.1).",
    )
    parser.add_argument(
        "--expected-os",
        default=None,
        help=f"Pin platform.os (default: {EXPECTED_PLATFORM['os']}).",
    )
    parser.add_argument(
        "--expected-arch",
        default=None,
        help=f"Pin platform.arch (default: {EXPECTED_PLATFORM['arch']}).",
    )
    parser.add_argument(
        "--expected-libc",
        default=None,
        help="Pin platform.libc (e.g. glibc-2.31).",
    )
    parser.add_argument(
        "--expected-cxx-abi",
        default=None,
        help="Pin platform.cxx_abi (e.g. cxx11).",
    )
    parser.add_argument(
        "--expected-compiler",
        default=None,
        help=f"Pin toolchain.compiler (default: {EXPECTED_COMPILER}).",
    )
    parser.add_argument(
        "--expected-compiler-version",
        default=None,
        help="Pin toolchain.compiler_version (prefix match, e.g. '18').",
    )
    parser.add_argument(
        "--expected-bazel-version",
        default=None,
        help="Pin toolchain.bazel_version (prefix match, e.g. '7').",
    )
    parser.add_argument(
        "--mode",
        default="prebuilt",
        choices=("prebuilt",),
        help=(
            "Selected mode this validator is gating. Reserved for future "
            "modes; only 'prebuilt' is meaningful today (source mode skips "
            "the validator entirely)."
        ),
    )
    parser.add_argument(
        "--summary-json",
        type=pathlib.Path,
        default=None,
        help=(
            "Write the validation summary as JSON to this path. On failure, "
            "writes {error_kind, field, expected, actual}. On success, "
            "writes the full ValidationSummary dataclass."
        ),
    )
    parser.add_argument(
        "--summary-line",
        action="store_true",
        help=(
            "On success, print a single self-describing line to stdout for "
            "log surfacing (CI summaries, Docker build logs, etc.). On "
            "failure, prints nothing on stdout (diagnostic goes to stderr)."
        ),
    )
    args = parser.parse_args(argv)

    pin = _parse_pin_args(args)
    expected_summary = _expected_pin_summary(pin)

    try:
        summary = validate(args.repo_root, pin)
    except ValidationError as err:
        msg = _render_failure(
            err,
            mode=args.mode,
            expected_summary=expected_summary,
            escape_hatches=_default_escape_hatches(),
        )
        print(msg, file=sys.stderr)
        if args.summary_json:
            args.summary_json.write_text(
                json.dumps(
                    {
                        "error_kind": err.kind,
                        "field": err.field,
                        "expected": str(err.expected),
                        "actual": str(err.actual),
                        "mode": args.mode,
                    },
                    indent=2,
                )
                + "\n"
            )
        return 1

    if args.summary_line:
        print(summary.render_line())
    if args.summary_json:
        args.summary_json.write_text(
            json.dumps({"ok": True, **summary.to_json()}, indent=2) + "\n"
        )
    return 0


def _expected_pin_summary(pin: ExpectedPin) -> list[str]:
    out: list[str] = []
    if pin.schema_version:
        out.append(f"schema_version = {pin.schema_version}")
    if pin.artifact_version:
        out.append(f"artifact_version = {pin.artifact_version}")
    if pin.googlesql_commit:
        out.append(f"googlesql.commit = {pin.googlesql_commit}")
    if pin.googlesql_tag:
        out.append(f"googlesql.upstream_tag = {pin.googlesql_tag}")
    expected_os = pin.os or EXPECTED_PLATFORM["os"]
    expected_arch = pin.arch or EXPECTED_PLATFORM["arch"]
    out.append(f"platform = {expected_os}/{expected_arch}")
    if pin.libc:
        out.append(f"libc = {pin.libc}")
    if pin.cxx_abi:
        out.append(f"cxx_abi = {pin.cxx_abi}")
    out.append(f"compiler = {pin.compiler or EXPECTED_COMPILER}")
    if pin.compiler_version:
        out.append(f"compiler_version startswith {pin.compiler_version}")
    if pin.bazel_version:
        out.append(f"bazel_version startswith {pin.bazel_version}")
    return out


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
