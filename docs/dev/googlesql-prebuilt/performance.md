# GoogleSQL Prebuilt — Cache and Performance Expectations

This doc records what the flip to a prebuilt-default GoogleSQL actually
changes about build-time, cache, Docker, and CI behavior — and,
importantly, what it does **not** change. The point is to set honest
expectations so a maintainer triaging a slow build can tell "prebuilt
didn't help here because <X>" apart from "prebuilt is broken."

No specific wall-clock numbers are promised. Cold-vs-warm
multipliers do not scale linearly with `--jobs` or RAM, and
GitHub-hosted runner cost (`ubuntu-latest`, 4 vCPU / 16 GB) is not
representative of every maintainer host. Treat the order-of-magnitude
ranges below as guidance, not contracts.

## What gets faster

### Local engine rebuilds

`task emulator:build-engine:bazel` defaults to `GOOGLESQL_SOURCE=prebuilt`
(per [`taskfiles/emulator.yml`](../../../taskfiles/emulator.yml)). On
a warm cache:

- The ~8K C++ TU GoogleSQL link is skipped entirely. Bazel resolves
  `@googlesql//...` against the unpacked prebuilt artifact at
  `.cache/googlesql-prebuilt/googlesql_prebuilt_linux_amd64/` via
  the `--config=googlesql-prebuilt` group in `.bazelrc`.
- The emulator's own C++ source (under `backend/`, `frontend/`,
  `binaries/emulator_main/`) still compiles normally — that's what
  the build is actually for.
- The pre-flight gate runs the centralized validator
  ([`tools/googlesql-prebuilt/validate_artifact.py`](../../../tools/googlesql-prebuilt/validate_artifact.py))
  once over the staged cache. Skim this in milliseconds; it's not in
  the hot path of incremental rebuilds.

What that means in practice: a typical "I changed one C++ file in
`backend/`" rebuild lands in the same shape as any incremental Bazel
build — bounded by the touched-TU count, not by GoogleSQL.

The cold-cache local case (first ever `task emulator:build-engine:bazel`
on a fresh checkout, prebuilt artifact already staged) still has to
compile every emulator C++ TU plus DuckDB, gRPC, abseil, protobuf,
etc. that the prebuilt artifact doesn't carry. It is faster than the
old "compile GoogleSQL too" cold case, but it is not free.

### PR CI

Every CI lane that touches the engine binary now goes through the
`setup-googlesql` composite action
([`.github/actions/setup-googlesql/action.yml`](../../../.github/actions/setup-googlesql/action.yml)):

| Lane | File |
|---|---|
| Main build + test | [`ci.yml`](../../../.github/workflows/ci.yml) |
| Conformance fixture lanes | [`conformance.yml`](../../../.github/workflows/conformance.yml) |
| Coverage (Bazel) | [`build-engine.yml`](../../../.github/workflows/build-engine.yml) (main push) |
| Third-party samples | [`thirdparty-samples.yml`](../../../.github/workflows/thirdparty-samples.yml) |
| Docker smoke | [`docker-smoke.yml`](../../../.github/workflows/docker-smoke.yml) |
| Release | [`release.yml`](../../../.github/workflows/release.yml) (with `fail-on-source-fallback: true`) |
| Source-vs-prebuilt parity (prebuilt leg) | [`googlesql-parity.yml`](../../../.github/workflows/googlesql-parity.yml) |

When `vars.GOOGLESQL_PREBUILT_URL` + `vars.GOOGLESQL_PREBUILT_SHA256`
are populated, every lane above runs against the prebuilt artifact:

- One `task googlesql:fetch-prebuilt URL=... SHA256=...` download per
  job (about the size of `libgooglesql.a` + `libgooglesql_protos.a`
  combined — currently a few hundred MB).
- No GoogleSQL source compile, no GoogleSQL source checkout, no
  upstream patch step. The composite action's "Checkout sibling
  googlesql (source fallback)" step is skipped.
- The setup-bazel disk cache (`bazelisk-cache: true`, distinct
  `disk-cache:` key per workflow) holds the emulator's own TUs across
  runs.

The release lane in particular benefits the most: it used to do the
cold-cache GoogleSQL link on every release (or twice, once on host
and once in the engine-builder container -- see the historical
`gh#26485361700` thread for the original triage).

### Docker `ENGINE_SOURCE=bazel`

`docker build` runs the canonical Bazel build of `emulator_main`
inside the [`engine-builder-bazel` stage](../../../Dockerfile) by
default (`ARG ENGINE_SOURCE=bazel`). When the build args
`GOOGLESQL_PREBUILT_URL` + `GOOGLESQL_PREBUILT_SHA256` are passed:

- The stage downloads + verifies the artifact and unpacks it at
  `/src/.cache/googlesql-prebuilt/`. The same SHA gate + path-escape
  gate + module-name gate + validator the host-side task runs.
- Bazel then runs `--config=googlesql-prebuilt`, skipping the
  GoogleSQL source compile (the most expensive piece of the
  in-container build).
- The BuildKit cache mount on `/root/.cache/bazel` carries the
  emulator's TUs across builds; warm rebuilds land in a couple of
  minutes once the layer above it (the Bazel install + apt deps)
  is cached too.

When `GOOGLESQL_PREBUILT_URL` is empty, the stage falls back to a
sibling `google/googlesql` clone and the legacy
`--config=googlesql-source` path. That path still works — Docker
builds are not forced to consume the prebuilt — but they pay the
cold-cache GoogleSQL cost.

### Release builds are deterministic

The release pipeline pins the artifact via
`env.RELEASE_GOOGLESQL_PREBUILT_URL` / `_SHA256` in
[`release.yml`](../../../.github/workflows/release.yml) and the
"Validate GoogleSQL prebuilt pin" step refuses to start when either
is empty. The release notes carry the GoogleSQL provenance block
(URL, SHA-256, upstream tag, manifest schema version, libc, compiler)
so consumers can reproduce the build from the same artifact.

Two consequences:

- The artifact in `GHCR` and the gateway archives is byte-for-byte
  reproducible from the published prebuilt pin (modulo timestamps).
- A release operator can never "accidentally" cut a release against
  a different GoogleSQL than the prebuilt pin — the validator's
  `FAIL_IDENTITY_PIN` token guards against republish-under-the-same-URL
  drift.

## What does NOT change

### Source mode is still slow

`GOOGLESQL_SOURCE=local task emulator:build-engine:bazel` still
compiles GoogleSQL from the sibling `../googlesql/` checkout — the
same ~8K C++ TU workload that motivated the prebuilt rollout in the
first place. Cold-cache figures depend on `--jobs` and RAM but
historically have landed in the 25–55 minute range for the project.

This is intentional. Source mode is the escape hatch for upstream
upgrades, producer debugging, and emergency rollback when the
prebuilt path is broken. It is **not** meant to be a daily-driver
default — but it is always available and the
[validator's escape hatch](../../../tools/googlesql-prebuilt/validate_artifact.py)
explicitly names the command in every failure block.

### First-time artifact download has a network cost

A fresh checkout (or a CI runner with no cached artifact) needs to
download the tarball before the first build. That's a network round
trip against `https://github.com/<owner>/<repo>/releases/download/...`,
plus the SHA gate, plus extraction.

The cost is paid **once per cache lifetime**. On a developer machine,
`task googlesql:fetch-prebuilt` lands the artifact under
`.cache/googlesql-prebuilt/` and subsequent rebuilds reuse it
verbatim. On CI, each job pays the download once and then reuses the
unpacked cache across the rest of its steps (`setup-bazel`'s
disk-cache covers the post-extraction work).

If the download itself becomes a bottleneck (e.g. you're behind a
slow link), consider:

- Building locally once with `task googlesql:stage-bazel`, which
  produces the artifact from your sibling `../googlesql/` checkout
  without touching the network. The build cost is the same as
  source mode, but it lets you cache the resulting tarball
  off-machine and reuse it.
- Caching the tarball next to your checkout (somewhere outside the
  workspace) and pointing `task googlesql:fetch-prebuilt` at a
  `file://` URL for it.

### Non-GoogleSQL C++ still compiles normally

The prebuilt artifact only carries the GoogleSQL surface. The
emulator's own C++ source under `backend/`, `frontend/`,
`binaries/emulator_main/`, plus the parts of DuckDB, gRPC, abseil,
protobuf, etc. that the emulator pulls in directly, all still
compile on every cold build. A `bazel clean` will still cost
multiple minutes even with the prebuilt artifact in place.

`--jobs` and `--local_resources` (driven by `BAZEL_JOBS` /
`BAZEL_MEM_MB`, auto-detected from `nproc` and `/proc/meminfo`)
still apply to that compile — see the comments in
[`taskfiles/emulator.yml`](../../../taskfiles/emulator.yml) for the
heuristic and the throttling rules.

### Prebuilt emulator engine binary is the fastest Docker path

If the caller doesn't actually need to build the engine themselves,
the published Docker image
(`ghcr.io/<owner>/bigquery-emulator:vX.Y.Z`) is the fastest path.
It already carries the linked `emulator_main` + `libduckdb.so` —
no Bazel, no GoogleSQL, no prebuilt artifact resolution at runtime.
`docker run` lands in seconds.

The two unrelated "prebuilt" surfaces are easy to confuse — see
[`README.md`](../../../README.md) §"Prebuilt GoogleSQL vs prebuilt
emulator engine binary" for the rule of thumb.

Likewise, `docker build --build-arg ENGINE_SOURCE=prebuilt` (used by
[`release.yml`](../../../.github/workflows/release.yml)) skips the
in-container Bazel build entirely and COPYs `bin/emulator_main` +
`bin/libduckdb.so` from the build context. This is faster than
`ENGINE_SOURCE=bazel + GOOGLESQL_PREBUILT_URL=...` because it pays
zero compile cost, but it requires the caller to stage the binaries
on the host first (via `task emulator:build-engine:bazel`).

## When the prebuilt path is the wrong tool

The prebuilt artifact is **only** about the GoogleSQL slice. If the
slow piece of your build is elsewhere, the prebuilt path won't help.
Some examples:

| Symptom | Likely root cause | Where to look |
|---|---|---|
| `bazel build` link step takes minutes on every rebuild | The emulator's own `cc_binary` link, not GoogleSQL. | Check `bazel-bin/binaries/emulator_main/emulator_main.runfiles_manifest`'s churn; consider `--config=opt`. |
| Docker image build still takes 30+ minutes warm | BuildKit cache mount not warm, or the layer ordering invalidates the cache. | `docker buildx du --verbose` to inspect cache hits; ensure `DOCKER_BUILDKIT=1`. |
| `task emulator:build-engine:bazel` cold-builds GoogleSQL on every invocation | `GOOGLESQL_SOURCE` is being overridden, OR the cache is being cleared, OR you're running with `--config=googlesql-source` from `.bazelrc` somehow. | Run `task googlesql:status`; check that the prebuilt cache contains a valid `MODULE.bazel` declaring `module(name = "googlesql", ...)`. |
| CI lane reports "falling back to source GoogleSQL build (slower)" | `vars.GOOGLESQL_PREBUILT_URL` / `_SHA256` are unset at the repo level. | See [`maintainer-runbook.md`](maintainer-runbook.md) §6.1; populate the vars. |
| Release run took 95+ minutes | `RELEASE_GOOGLESQL_PREBUILT_URL` / `_SHA256` are empty in `release.yml`. | The "Validate GoogleSQL prebuilt pin" step should have refused; if it didn't, you're on the `workflow_dispatch -f googlesql_source=true` debug path. |

## See also

- [`README.md`](README.md) — compatibility surface index.
- [`maintainer-runbook.md`](maintainer-runbook.md) — publish / pin / verify / roll back.
- [`troubleshooting.md`](troubleshooting.md) — `FAIL_*` validator-token map.
- [`taskfiles/emulator.yml`](../../../taskfiles/emulator.yml) — `task emulator:build-engine:bazel` definition + throttling.
- [`taskfiles/googlesql.yml`](../../../taskfiles/googlesql.yml) — `task googlesql:fetch-prebuilt` / `:status` / `:validate` / `:clean`.
- [`.cursor/rules/bazel-process-hygiene.mdc`](../../../.cursor/rules/bazel-process-hygiene.mdc) — heavy-process hygiene rules that apply to every engine build.
