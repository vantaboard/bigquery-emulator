# Releases

> **Preview-grade.** The `v0.x` series is an explicit preview: the REST surface,
> gRPC contract, and on-disk format may break across releases. Stable promises
> arrive at `v1.0.0`. See [`ROADMAP.md`](../ROADMAP.md) for the active plan.

Releases are cut by tag push today. Tag the commit you want to release with
`vX.Y.Z` and push the tag; that triggers
[`.github/workflows/release.yml`](../.github/workflows/release.yml), which
builds the engine via Bazel, publishes the runtime Docker image to
[GHCR](https://github.com/users/vantaboard/packages/container/package/bigquery-emulator),
and uses [goreleaser](https://goreleaser.com) (config:
[`.goreleaser.yml`](../.goreleaser.yml)) to upload the gateway archives + SHA-256
checksums to the GitHub release page.

```bash
# Cut the very first release (preview).
git tag -a v0.0.1 -m 'release: v0.0.1 (preview)'
git push origin v0.0.1
```

`task release:tag VERSION=v0.0.1` is a foot-gun guard around the above: it
prints the exact `git tag` + `git push` lines and only executes them when
`CONFIRM=yes` is set (`task release:tag VERSION=v0.0.1 CONFIRM=yes`).

The semantic-release config at [`.releaserc.yml`](../.releaserc.yml) is parked
for the eventual switch to auto-release on push to `main`. It is not currently
driving any GitHub Actions job; the file exists so the conventional-commits
format documented in `.cursor/rules/auto-commit.mdc` has a target for the
future flip.

## Install via release archive

```bash
# Pick the right tarball for your OS/arch from the releases page:
# https://github.com/vantaboard/bigquery-emulator/releases
curl -fL https://github.com/vantaboard/bigquery-emulator/releases/download/v0.0.1/bigquery-emulator_0.0.1_linux_amd64.tar.gz \
    | tar xz
./bigquery-emulator-gateway --help
```

Each archive bundles `bigquery-emulator-gateway` (the Go REST gateway) plus
`bin/emulator_main` and `bin/libduckdb.so` (the C++ engine). The gateway's
`--engine_binary` flag defaults to discovering the engine beside the gateway
binary, so `./bigquery-emulator-gateway` works out of the tarball without extra
flags.

> **Engine binary is linux/amd64 only.** Upstream GoogleSQL's hermetic LLVM
> toolchain does not yet cross-build cleanly to linux/arm64, and macOS engine
> builds are out of scope for the preview series. The macOS + linux/arm64 archives
> still bundle the linux/amd64 engine binary so the layout stays uniform, but
> you cannot run those engine binaries on a non-linux/amd64 host. The recommended
> path on macOS or linux/arm64 is the published Docker image (next section).

## Install via Docker

```bash
docker pull ghcr.io/vantaboard/bigquery-emulator:v0.0.1
docker run --rm -p 9050:9050 ghcr.io/vantaboard/bigquery-emulator:v0.0.1
```

Each release publishes four tags to GHCR:

- `vX.Y.Z` — exact version (immutable).
- `vX.Y` — minor track (moves on patch releases).
- `vX` — major track (moves on minor + patch releases).
- `latest` — newest non-pre-release.

Pre-release tags (`v0.0.1-rc1`) skip the `latest` tag promotion so
`docker pull ...:latest` always lands on a non-pre-release version. The Docker
image is `linux/amd64` only for the same engine-binary reason as above.
