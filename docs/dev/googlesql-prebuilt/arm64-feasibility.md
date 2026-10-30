# linux/arm64 engine — feasibility note

This document records the toolchain spike for shipping a native
`emulator_main` on `linux/arm64`. It captures the spike's findings
whether or not the engine build is green yet.

## Summary (2026-10)

| Layer | Status | Notes |
|---|---|---|
| DuckDB `libduckdb.so` | **Ready** | Upstream v1.5.3 arm64 zip registered in `MODULE.bazel`; `//third_party/duckdb:duckdb` selects on `@platforms//cpu`. |
| Go gateway | **Ready** | Already cross-compiles to `linux/arm64` via goreleaser. |
| GoogleSQL prebuilt producer | **Ready** | `googlesql-prebuilt.yml` matrix leg on `ubuntu-24.04-arm` builds, verifies (link + Bazel smoke) and publishes `googlesql_prebuilt_linux_arm64/` tarballs. Repo vars: `GOOGLESQL_PREBUILT_URL_ARM64` / `GOOGLESQL_PREBUILT_SHA256_ARM64`. |
| GoogleSQL hermetic LLVM (amd64) | **Blocker for cross-build** | The toolchain GoogleSQL registers ships amd64 host tools only. Cross-compiling from an amd64 host to `linux/arm64` fails before first-party code compiles. |
| Native arm64 engine build | **Ready** | Non-blocking `build-engine-arm64` job on `ubuntu-24.04-arm` builds natively with system `clang` against the arm64 prebuilt. aarch64 links need compiler-rt (see below). |
| Release archives + Docker | **Ready** | `release.yml` builds the engine on native amd64 + arm64 runners, ships a `linux_arm64` archive with its own engine, and publishes a multi-arch GHCR manifest (`linux/amd64,linux/arm64`). |

**Conclusion:** arm64 ships as a **native build on arm64 runners** against an
arm64 GoogleSQL prebuilt. Cross-compiling the engine from amd64 is **not** the
plan of record and remains blocked by the hermetic toolchain.

## Toolchain spike findings

### amd64 host cross-build (stretch — failed as expected)

On an `x86_64` dev host, `bazel build --platforms=@platforms//cpu:aarch64`
does not reach GoogleSQL compile: Bazel rejects the bare constraint target
(`Target @@platforms//cpu:aarch64 was referenced as a platform, but does not
provide PlatformInfo`) unless a full platform definition is wired. Even with a
proper platform, GoogleSQL's hermetic LLVM toolchain is built for amd64 host
execution; the upstream tree assumes the hermetic compiler runs on the same arch
as the artifact.

This matches the roadmap posture: the pain is Apple Silicon / Graviton users
running amd64 emulation, not amd64 developers cross-compiling locally.

### Native arm64 build (plan of record)

The producer + consumer path for arm64 mirrors amd64:

1. **`googlesql-prebuilt.yml`** — matrix leg on `ubuntu-24.04-arm` runs
   `package.sh --target-arch=arm64`, producing
   `googlesql-prebuilt-linux-arm64-clang18-<sha>-v<version>.tar.gz` under
   `googlesql_prebuilt_linux_arm64/`.
2. **Repo vars** — maintainers set `GOOGLESQL_PREBUILT_URL_ARM64` and
   `GOOGLESQL_PREBUILT_SHA256_ARM64` alongside the existing amd64 pins.
3. **Consumer wiring** — `GOOGLESQL_PREBUILT_ARCH=arm64` (or an aarch64 host)
   selects `--config=googlesql-prebuilt-arm64` and cache path
   `.cache/googlesql-prebuilt/googlesql_prebuilt_linux_arm64/`.
4. **`build-engine-arm64`** — non-blocking CI job; promote to blocking after
   several weeks green, then add conformance-on-arm64 before release.

### Hermetic vs system clang on arm64

GoogleSQL's hermetic LLVM is the preferred producer toolchain on amd64 because
it pins compiler + sysroot. An arm64 hermetic sibling does not exist in-tree
today. The arm64 producer and engine spike therefore use **system `clang`** from
`ubuntu-24.04-arm` (`apt install clang`), matching how local emulator builds
already pass `CC=/usr/bin/clang CXX=/usr/bin/clang++`.

Tradeoff: slightly less hermetic than amd64, but the prebuilt manifest records
`platform.arch`, `toolchain.compiler`, and `toolchain.compiler_version` so
consumers can gate mismatches via `validate_artifact.py`.

### compiler-rt on aarch64

GoogleSQL compiles on arm64 with system clang-18 without source changes. The
link is the only arch-specific failure: `int128` multiplication in GoogleSQL
(for example `RescaleInterval<absl::int128>` in `date_time_util.cc`) emits
calls to `__muloti4`, which aarch64 libgcc does not provide. Every aarch64
link against the prebuilt therefore passes `-rtlib=compiler-rt
-unwindlib=libgcc` (`binaries/emulator_main/BUILD.bazel`,
`tools/googlesql-prebuilt/smoke/BUILD.bazel`) and needs `libclang-rt-18-dev`
installed.

### Risks still open

- **Float / decimal conformance** across ISAs — run the full conformance lane
  on arm64 before shipping release archives, not just `cc_test`.
- **Runner cost** — cold arm64 engine builds share the ~2 h profile of amd64;
  reuse Bazel disk caches (`engine-arm64-v1`, `googlesql-prebuilt-arm64`).

## What landed (infra)

- DuckDB arm64 `http_archive` + `select()` in `//third_party/duckdb`.
- `--config=googlesql-prebuilt-arm64` in `.bazelrc`.
- Arch-aware `task googlesql:*` + `setup-googlesql` composite action.
- `googlesql-prebuilt.yml` amd64 + arm64 matrix producer.
- `build-engine.yml` non-blocking `build-engine-arm64` job.
- Dockerfile `TARGETARCH` wiring, so `docker buildx --platform linux/arm64`
  builds the engine natively.
- `release.yml` engine matrix + multi-arch GHCR manifest; per-arch engines in
  the goreleaser linux archives.

## Follow-ups

1. First upstream arm64 prebuilt publish → set `GOOGLESQL_PREBUILT_*_ARM64`
   repo vars and the `RELEASE_GOOGLESQL_PREBUILT_*_ARM64` pins in `release.yml`.
2. Green `build-engine-arm64` for several weeks → make job blocking; add conformance matrix leg.

## References

- Prebuilt upgrade rules (sibling `@googlesql_prebuilt_linux_arm64`):
  [`upgrade-rules.md`](./upgrade-rules.md)
- DuckDB pin (both arch SHA-256): [`../../../third_party/duckdb/VERSION`](../../../third_party/duckdb/VERSION)
