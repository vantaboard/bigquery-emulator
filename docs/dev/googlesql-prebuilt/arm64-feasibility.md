# linux/arm64 engine feasibility

The shipping engine binary is **linux/amd64**. A parallel **linux/arm64** lane is
in flight so native arm64 hosts can eventually run the bundled engine without
Docker's amd64 emulation layer.

## Current posture

| Surface | Status |
| --- | --- |
| Release archives | Ship **linux/amd64** engine only |
| Gateway archives (macOS / linux/arm64) | Bundle the **linux/amd64** engine until arm64 releases land |
| Docker image | Recommended path on macOS and linux/arm64 today |
| CI `build-engine-arm64` | Non-blocking spike on `ubuntu-24.04-arm` |

The arm64 CI job consumes a dedicated prebuilt artifact when repository variables
`GOOGLESQL_PREBUILT_URL_ARM64` and `GOOGLESQL_PREBUILT_SHA256_ARM64` are set.
When those vars are unset, the job logs a notice and exits successfully without
starting a multi-hour source GoogleSQL build.

## Operator guidance

- **Develop on linux/amd64:** use `task emulator:build-engine:bazel` or the
  published release archive.
- **Develop on macOS or linux/arm64:** prefer the published Docker image (see
  [`docs/DOCKER.md`](../../DOCKER.md#install-via-docker)).
- **Maintainers:** after the first arm64 prebuilt publish, set the arm64 repo
  variables and watch the non-blocking `build-engine-arm64` job on `main`.

See also [`docs/RELEASES.md`](../../RELEASES.md) and the GoogleSQL prebuilt
[`README.md`](./README.md).
