---
name: linux/arm64 engine build
overview: Produce an arm64 engine binary by fixing the GoogleSQL toolchain story (native arm64 LLVM or cross-build), then extend the prebuilt artifact, release archives, and Docker manifest to both architectures.
todos:
  - id: toolchain-spike
    content: "Spike: build GoogleSQL on a linux/arm64 runner with a native arm64 LLVM toolchain (registered alongside the amd64 hermetic one); document what breaks"
    status: pending
  - id: duckdb-arm64
    content: Add the linux/arm64 libduckdb prebuilt to third_party/duckdb with platform select()
    status: pending
  - id: prebuilt-arm64
    content: Extend googlesql-prebuilt.yml to publish an arm64 artifact + repo vars for its URL/SHA
    status: pending
  - id: release-matrix
    content: "goreleaser + release.yml: linux/arm64 archive and multi-arch Docker manifest (buildx) once engine builds"
    status: pending
  - id: ci-lane
    content: Non-blocking arm64 build lane (GitHub arm64 runners) promoted to blocking when stable
    status: pending
isProject: false
---

# 05 — linux/arm64 engine (infra)

- **Roadmap rows:** §Repo bootstrap CI bullet and §Build systems — "Linux/amd64
  only — the GoogleSQL hermetic LLVM toolchain does not cross-build cleanly to
  linux/arm64 yet"; §Distribution — "linux/amd64 engine only — see README
  §Releases"

## Current state at HEAD (grounded)

- The GoogleSQL build pins a hermetic LLVM toolchain that ships amd64
  binaries; `bazel build //binaries/emulator_main` on an arm64 host or with
  `--platforms` cross-config fails in the toolchain, before any first-party
  code compiles.
- DuckDB is consumed as a prebuilt linux/amd64 `libduckdb.so` zip
  (`third_party/duckdb/`); upstream publishes linux/arm64 zips too, so this
  half is easy.
- The prebuilt-GoogleSQL pipeline (`googlesql-prebuilt.yml` →
  `GOOGLESQL_PREBUILT_URL`/`SHA256` repo vars → `.cache/googlesql-prebuilt/`)
  is single-arch; the artifact name and vars have no arch dimension.
- goreleaser bundles `bin/emulator_main` + `bin/libduckdb.so` into the
  linux/amd64 archive only; the Go gateway itself already cross-compiles
  trivially. Docker images are amd64-only.
- Non-amd64 users are told to use the published Docker image (under
  emulation) — the actual pain this plan removes is Apple-Silicon and
  Graviton CI users running amd64 emulation.

## Done-criteria

1. `emulator_main` builds and passes the C++ test suite on a linux/arm64
   runner (native build; cross-build is a stretch goal, not required).
2. `googlesql-prebuilt.yml` publishes amd64 **and** arm64 artifacts; CI
   consumers pick by arch.
3. Releases ship a `linux_arm64` archive with engine + `libduckdb.so`, and
   `ghcr.io/vantaboard/bigquery-emulator` is a multi-arch manifest.
4. ROADMAP / README / RELEASES prose drops the "amd64 only" caveats.

## Implementation steps

### Step 1 — toolchain spike (the actual unknown)

On a GitHub `ubuntu-24.04-arm` runner, attempt the source GoogleSQL build
with a **native arm64 LLVM** toolchain instead of the amd64 hermetic one:
either an arm64 build of the same hermetic toolchain (preferred — keeps
hermeticity) or system clang pinned via `--action_env` the way local builds
already do. Catalog every failure (toolchain registration, `-march` flags,
absl/protobuf config, GoogleSQL's own arch assumptions). Timebox; the output
is a written feasibility note in `docs/dev/googlesql-prebuilt/` either way.

### Step 2 — DuckDB arm64

Add the upstream linux/arm64 `libduckdb` zip as a second `http_archive` with
`select()` on `@platforms//cpu`, and make the `$ORIGIN` rpath staging in
`taskfiles/emulator.yml` arch-agnostic.

### Step 3 — prebuilt artifact matrix

Give the prebuilt artifact an arch suffix
(`googlesql-prebuilt-linux-arm64.tar.zst`), publish both from
`googlesql-prebuilt.yml` (arm64 job on the arm runner), add
`GOOGLESQL_PREBUILT_URL_ARM64` / `..._SHA256_ARM64` repo vars, and teach
`task googlesql:fetch-prebuilt` + `build-engine.yml` to select by
`runner.arch`.

### Step 4 — release + Docker

Extend `.goreleaser.yml` with a linux/arm64 archive (gateway cross-compiled;
engine + libduckdb staged from the arm64 build job), and switch `release.yml`
Docker publishing to `buildx` with `--platform linux/amd64,linux/arm64`,
respecting the existing `BAZEL_JOBS` / `BAZEL_MEM_MB` caps per
`.cursor/rules/process-hygiene.mdc`.

### Step 5 — CI lane

Add a non-blocking `build-engine-arm64` job first; promote to blocking and
add conformance-on-arm64 once it has been green for a few weeks.

## Risks

- The hermetic toolchain may be genuinely amd64-locked upstream; fallback is
  a pinned system-clang toolchain on the arm runner (less hermetic — document
  the tradeoff in the feasibility note before committing to it).
- arm64 runner minutes cost / availability; the ~long engine build needs the
  same disk-cache strategy amd64 CI uses.
- Float/decimal conformance drift across ISAs — run the full fixture lane on
  arm64 before shipping, not just the C++ unit tests.

## Out of scope

- macOS (darwin) engine binaries; Windows.
- Cross-compiling from amd64 to arm64 (native build on arm runners is the
  plan of record; cross is a follow-up if runner cost forces it).

## Touch list

`MODULE.bazel` (+ toolchain registration), `third_party/duckdb/`,
`taskfiles/{bazel,emulator,googlesql}.yml`,
`.github/workflows/{googlesql-prebuilt,build-engine,release,ci}.yml`,
`.goreleaser.yml`, `Dockerfile`, `docs/dev/googlesql-prebuilt/`, `README.md`,
`docs/RELEASES.md`, `ROADMAP.md`.
