# GoogleSQL Prebuilt — Manifest Contract (Phase 1 — Section 4)

This file freezes the schema of `manifest.json`, the machine-readable contract
that ships next to every published GoogleSQL prebuilt artifact. The manifest
exists so the consumer (Phase 3) and the parity gates (Phase 5) can verify
**source identity**, **platform compatibility**, and **payload integrity**
without performing a network query or rebuilding GoogleSQL from source.

## Field schema

`manifest.json` is a single JSON object. Every field below is required unless
explicitly marked optional. The schema is **closed**: unknown top-level fields
are a Phase-5 validation error.

| Field                                 | Type    | Description |
|---------------------------------------|---------|-------------|
| `schema_version`                      | string  | Manifest schema version. Pin to `"1"` for the initial rollout; bumps require a Phase 5 validator change. |
| `artifact_version`                    | string  | Producer-assigned version of *this* prebuilt artifact. Strict-semver-aliased (`MAJOR.MINOR.PATCH`). Bumped whenever any header, library, or wrapper layout changes (see [`upgrade-rules.md`](upgrade-rules.md)). |
| `googlesql.module_version`            | string  | Upstream GoogleSQL `MODULE.bazel` version (`2026.1.1`). |
| `googlesql.upstream_tag`              | string  | Upstream git tag (`2026.01.1` — leading zero preserved, distinct from the strict-semver alias). |
| `googlesql.commit`                    | string  | Immutable upstream commit SHA the artifact was built from (40 hex chars). |
| `googlesql.repo_url`                  | string  | Upstream remote URL (`https://github.com/google/googlesql`). |
| `googlesql.patches`                   | array   | Ordered list of patch SHA-256s (and short descriptions) that were applied to the upstream tree before build. Empty array if no patches. The Gazelle leading-zero workaround patch documented in `MODULE.bazel` lives here if applied. |
| `emulator.min_commit`                 | string  | Minimum bigquery-emulator git commit that is compatible with this artifact. Phase 3 enforces this — older emulator checkouts refuse to consume the artifact. |
| `emulator.max_commit`                 | string\|null | Optional inclusive upper bound. Use `null` to mean "no known upper bound at publish time". Phase 5 may rewrite this as part of an incompatibility annotation. |
| `compat.labels`                       | array   | Verbatim list of exposed wrapper labels (e.g. `"//googlesql/public:analyzer"`). Must match exactly the labels enumerated in [`label-inventory.md`](label-inventory.md). |
| `platform.os`                         | string  | `linux` only for Phase 1. |
| `platform.arch`                       | string  | `amd64` only for Phase 1. |
| `platform.libc`                       | string  | `glibc-2.31` (or whichever the producer's hermetic toolchain links against). Phase 5 checks the consumer's libc against this; mismatch is a hard error. |
| `platform.cxx_abi`                    | string  | C++ ABI tag the artifact was compiled with (e.g. `cxx11`). |
| `toolchain.compiler`                  | string  | `clang` only for Phase 1. |
| `toolchain.compiler_version`          | string  | Full compiler version string (e.g. `18.1.8`). |
| `toolchain.bazel_version`             | string  | Bazel/bazelisk version used to build the artifact (e.g. `7.6.1` matching upstream `.bazelversion`). |
| `toolchain.cflags`                    | array   | The compile flags pinned by the producer (`["-O2", "-fPIC", "-fno-omit-frame-pointer", ...]`). |
| `toolchain.linkflags`                 | array   | Linker flags. Empty array is allowed. |
| `payload.headers`                     | array   | List of `{path, sha256}` objects for every header shipped under `include/`. |
| `payload.libraries`                   | array   | List of `{path, sha256, size_bytes}` objects for every static archive shipped under `lib/`. |
| `payload.extras`                      | array   | List of `{path, sha256}` for any other files shipped (e.g. `LICENSES/*`, `BUILD.bazel`, `MODULE.bazel`). |
| `producer.workflow`                   | string  | Identifier of the CI workflow that produced the artifact (e.g. `googlesql-prebuilt-producer-linux-amd64.yml`). |
| `producer.run_id`                     | string  | CI run identifier (GitHub `${{ github.run_id }}` or equivalent). |
| `producer.build_timestamp`            | string  | RFC 3339 UTC timestamp the artifact was finalised. |
| `producer.host_os_release`            | string  | `lsb_release -ds` (or equivalent) of the producer host. Useful for diagnosing libc-mismatch reports. |
| `bundled_thirdparty_deps`             | array   | List of third-party libraries **statically** bundled into `lib/libgooglesql.a`, in `"<name>@<version-or-commit>"` form. Phase 1 default ships `["icu@76.1", "farmhash@<commit>", "differential-privacy@<version>"]` — all three are bundled because none has a Bzlmod-resolvable equivalent at the producer-pinned ABI (BCR `icu` is 78.x; no BCR `farmhash`; no BCR `differential-privacy`). All other transitively-linked third-party deps (Abseil, Protobuf, gRPC, BoringSSL, RE2, GoogleTest, googleapis) are NOT bundled and are listed as `bazel_dep`s in the prebuilt repo's `MODULE.bazel` for the consumer to resolve. |

## Example manifest

The example below is illustrative — all SHAs and bytes are dummies. The
producer (Phase 2) populates them mechanically from the actual build outputs.

```json
{
  "schema_version": "1",
  "artifact_version": "0.1.0",
  "googlesql": {
    "module_version": "2026.1.1",
    "upstream_tag": "2026.01.1",
    "commit": "36dd14aa0657ea299725504bc0f938732f58f380",
    "repo_url": "https://github.com/google/googlesql",
    "patches": [
      {
        "sha256": "0000000000000000000000000000000000000000000000000000000000000000",
        "description": "Rewrite MODULE.bazel version 2026.01.1 -> 2026.1.1 for gazelle semver compatibility."
      }
    ]
  },
  "emulator": {
    "min_commit": "ad6802d000000000000000000000000000000000",
    "max_commit": null
  },
  "compat": {
    "labels": [
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
      "//googlesql/resolved_ast:resolved_node_kind_cc_proto"
    ]
  },
  "platform": {
    "os": "linux",
    "arch": "amd64",
    "libc": "glibc-2.31",
    "cxx_abi": "cxx11"
  },
  "toolchain": {
    "compiler": "clang",
    "compiler_version": "18.1.8",
    "bazel_version": "7.6.1",
    "cflags": ["-O2", "-fPIC", "-fno-omit-frame-pointer", "-DNDEBUG"],
    "linkflags": []
  },
  "payload": {
    "headers": [
      {
        "path": "include/googlesql/public/analyzer.h",
        "sha256": "1111111111111111111111111111111111111111111111111111111111111111"
      },
      {
        "path": "include/googlesql/public/type.h",
        "sha256": "2222222222222222222222222222222222222222222222222222222222222222"
      }
    ],
    "libraries": [
      {
        "path": "lib/libgooglesql.a",
        "sha256": "3333333333333333333333333333333333333333333333333333333333333333",
        "size_bytes": 538291712
      },
      {
        "path": "lib/libgooglesql_protos.a",
        "sha256": "4444444444444444444444444444444444444444444444444444444444444444",
        "size_bytes": 41812944
      }
    ],
    "extras": [
      {
        "path": "BUILD.bazel",
        "sha256": "5555555555555555555555555555555555555555555555555555555555555555"
      },
      {
        "path": "MODULE.bazel",
        "sha256": "6666666666666666666666666666666666666666666666666666666666666666"
      },
      {
        "path": "LICENSES/googlesql-LICENSE",
        "sha256": "7777777777777777777777777777777777777777777777777777777777777777"
      }
    ]
  },
  "producer": {
    "workflow": "googlesql-prebuilt-producer-linux-amd64.yml",
    "run_id": "1234567890",
    "build_timestamp": "2026-02-15T17:43:21Z",
    "host_os_release": "Ubuntu 22.04.4 LTS"
  },
  "bundled_thirdparty_deps": [
    "icu@76.1",
    "farmhash@816a4ae622e964763ca0862d9dbd19324a1eaf45",
    "differential-privacy@4.0.0"
  ]
}
```

## Fields pinned by Phase 3 (consume-time)

Phase 3 wires the consumer side. At consume time, the wrapper repo's loader
**must** pin / verify the following fields. Any mismatch is a Phase 5 fatal
error (`bazel build` fails, with a diagnostic that names the offending field).

| Field                          | Phase 3 / 5 behaviour |
|--------------------------------|-----------------------|
| `schema_version`               | Phase 3 hard-codes `"1"`. Mismatch = the consumer is older than the artifact. |
| `googlesql.commit`             | Phase 5 parity check compares this against the consumer-pinned source SHA when source-vs-prebuilt parity testing runs. Mismatch is informational unless the consumer is in parity-CI mode (where it is fatal). |
| `compat.labels`                | Phase 3 asserts the wrapper repo's actual `BUILD.bazel` declares exactly this set of labels. Mismatch = the producer drifted from this design doc. |
| `emulator.min_commit`          | Phase 3 reads it once at extension load. If the consumer's emulator HEAD is older, the load fails with a fatal "artifact too new for this emulator checkout". |
| `platform.os`, `.arch`         | Phase 3 selects the artifact based on `@platforms//os:linux` and `@platforms//cpu:x86_64`. Mismatch = the wrong artifact was downloaded. |
| `platform.libc`                | Phase 5 checks the consumer host's libc against this. Mismatch on a release build is fatal; on a developer build it's a warning. |
| `toolchain.compiler`, `.compiler_version` | Phase 5 parity check; mismatch logged but not fatal (cross-toolchain builds are explicitly allowed). |
| `payload.libraries[*].sha256`  | Phase 3 wraps each library in a `cc_import` that is registered with `urls = [...]` and `sha256 = ...`. **A checksum failure is a hard error**: `bazel build` refuses to start. |
| `payload.headers[*].sha256`    | Phase 5 spot-checks (full audit on parity-CI runs); a header mismatch fails the lane. |

## Checksum semantics

- **Every** file shipped under the artifact's repo root has an entry in
  `payload.headers`, `payload.libraries`, or `payload.extras`.
- **Every** entry includes a SHA-256 digest of the file's raw bytes.
- Phase 3 verifies the top-level archive's SHA-256 via the `http_archive`'s
  `sha256` attribute. **Inside** the unpacked archive, Phase 5's `goldens`
  job verifies file-level SHAs against `manifest.json`.
- A checksum mismatch — at the archive level or any file inside it — is a
  **hard error**. The wrapper repo refuses to load and no build proceeds.
  This rule is non-negotiable for the entire rollout. The error message must
  name the offending path and its expected vs. actual SHA-256.
- The manifest itself is **not** in `payload.{headers,libraries,extras}`. Its
  authenticity is established by the top-level archive's SHA (which the
  consumer pins via `http_archive(sha256 = ...)` in `MODULE.bazel`). A
  separate detached signature (e.g. `manifest.json.minisig`) may ship later
  but is **not** required for Phase 1.

## Distribution format

The producer publishes a single `.tar.gz` per release. The archive's
top-level directory matches the repo name
(`googlesql_prebuilt_linux_amd64/`). Phase 3 consumes it via Bzlmod's
`http_archive` rule with `strip_prefix = "googlesql_prebuilt_linux_amd64"`.

Publication channel (GitHub Releases vs. a generic asset store) is Phase 2's
decision. Phase 1 only freezes the on-disk shape of what gets published.
