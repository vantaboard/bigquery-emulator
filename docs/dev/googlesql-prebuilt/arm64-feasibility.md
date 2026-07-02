# linux/arm64 engine — feasibility note (plan 05)

This document records the toolchain spike for shipping a native
`emulator_main` on `linux/arm64`. It is the written output of plan
05 step 1 whether or not the engine build is green yet.

## Summary (2026-07)

| Layer | Status | Notes |
|---|---|---|
| DuckDB `libduckdb.so` | **Ready** | Upstream v1.5.3 arm64 zip registered in `MODULE.bazel`; `//third_party/duckdb:duckdb` selects on `@platforms//cpu`. |
| Go gateway | **Ready** | Already cross-compiles to `linux/arm64` via goreleaser. |
| GoogleSQL prebuilt producer | **In flight** | `googlesql-prebuilt.yml` matrix leg on `ubuntu-24.04-arm` publishes `googlesql_prebuilt_linux_arm64/` tarballs. Repo vars: `GOOGLESQL_PREBUILT_URL_ARM64` / `GOOGLESQL_PREBUILT_SHA256_ARM64`. |
| GoogleSQL hermetic LLVM (amd64) | **Blocker for cross-build** | The toolchain GoogleSQL registers ships amd64 host tools only. Cross-compiling from an amd64 host to `linux/arm64` fails before first-party code compiles. |
| Native arm64 engine build | **Unknown — CI spike** | Non-blocking `build-engine-arm64` job on `ubuntu-24.04-arm` attempts a **native** build with system `clang` (same posture as local amd64 builds) once an arm64 prebuilt exists, or source fallback until repo vars are populated. |
| Release archives + Docker | **Deferred** | goreleaser / `release.yml` still ship a linux/amd64 engine only. Multi-arch GHCR manifest waits for a green arm64 engine lane + conformance on arm64. |

**Conclusion:** arm64 is viable as a **native build on arm64 runners** once the
GoogleSQL dependency resolves (arm64 prebuilt artifact or patched source build
with system clang). Cross-compiling the engine from amd64 is **not** the plan of
record and remains blocked by the hermetic toolchain. Release packaging is
intentionally deferred until CI proves the native path.

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

### Risks still open

- **First green arm64 GoogleSQL compile** may surface arch-specific failures
  (`-march`, Abseil/protobuf flags, GoogleSQL internals). Catalog failures in
  CI logs from `build-engine-arm64` and the prebuilt matrix leg.
- **Float / decimal conformance** across ISAs — run the full conformance lane
  on arm64 before shipping release archives, not just `cc_test`.
- **Runner cost** — cold arm64 engine builds share the ~2 h profile of amd64;
  reuse Bazel disk caches (`engine-arm64-v1`, `googlesql-prebuilt-arm64`).

## What landed in plan 05 (infra)

- DuckDB arm64 `http_archive` + `select()` in `//third_party/duckdb`.
- `--config=googlesql-prebuilt-arm64` in `.bazelrc`.
- Arch-aware `task googlesql:*` + `setup-googlesql` composite action.
- `googlesql-prebuilt.yml` amd64 + arm64 matrix producer.
- `build-engine.yml` non-blocking `build-engine-arm64` job.

## Follow-ups (not in plan 05)

1. First successful arm64 prebuilt publish → set `GOOGLESQL_PREBUILT_*_ARM64` repo vars.
2. Green `build-engine-arm64` for several weeks → make job blocking; add conformance matrix leg.
3. Extend `.goreleaser.yml` + `release.yml` with a `linux_arm64` engine archive and GHCR `buildx` multi-arch manifest (`linux/amd64,linux/arm64`).
4. Update `docs/RELEASES.md` / README install story once (3) ships.

## References

- Plan: `.cursor/plans/roadmap/05-linux-arm64-engine.plan.md`
- Prebuilt upgrade rules (sibling `@googlesql_prebuilt_linux_arm64`):
  [`upgrade-rules.md`](./upgrade-rules.md)
- DuckDB pin (both arch SHA-256): [`../../../third_party/duckdb/VERSION`](../../../third_party/duckdb/VERSION)
