---
name: distribution-release
overview: "Phase 9: package emulator for distribution — multi-stage Docker image, release binaries (linux/darwin amd64+arm64), semantic-release changelog, documented profiles (--profile=ci|duckdb|dev), and README quickstart validated in CI."
todos:
  - id: docker-multistage
    content: "Finalize Dockerfile: build gateway + emulator_main with Bazel/CMake, runtime image with both binaries, ENTRYPOINT gateway with --engine_binary, EXPOSE 9050, default --profile=ci"
    status: pending
  - id: compose-dev
    content: "Add docker-compose.yml for local dev: mount data_dir, env BIGQUERY_EMULATOR_HOST, healthcheck on GET /bigquery/v2/projects/test/datasets"
    status: pending
  - id: goreleaser
    content: "Add .goreleaser.yml: build gateway for 4 platforms; embed or sidecar emulator binary from CI artifact; archive tar.gz with LICENSE README"
    status: pending
  - id: semantic-release
    content: "Wire .releaserc.yml (already present) with GitHub release workflow; conventional commits from auto-commit rule; publish Docker on tag"
    status: pending
  - id: profile-docs
    content: "Document --profile=ci (reference_impl+memory), duckdb (duckdb+duckdb parquet), dev (verbose logging) in README and ROADMAP Phase 9"
    status: pending
  - id: quickstart-ci
    content: "CI smoke: docker run -d, curl datasets list + SELECT 1 query, assert 200"
    status: pending
  - id: version-flag
    content: "Add --version to gateway and emulator_main printing git sha + semver from build ldflags"
    status: pending
isProject: false
---

# Phase 9: Distribution & release

## Prerequisites

- [bootstrap-ci-docker_e0f1a2b3.plan.md](bootstrap-ci-docker_e0f1a2b3.plan.md) — initial Dockerfile/CI
- [reference-impl-execution_9c0d1e2f.plan.md](reference-impl-execution_9c0d1e2f.plan.md) — working default profile for smoke tests
- [conformance-harness_d9e0f1a2.plan.md](conformance-harness_d9e0f1a2.plan.md) — optional but recommended before first release tag

## Scope

**In:** Docker, GoReleaser, semantic-release, quickstart validation, version flags.

**Out:** Homebrew tap, GCP Marketplace, hosted SaaS emulator.

## Key files

- `Dockerfile`, `docker-compose.yml`
- `.goreleaser.yml`
- `.github/workflows/release.yml`
- `README.md`, `ROADMAP.md`
- `binaries/gateway_main/main.go` — version ldflags

## Verification

```bash
docker build -t bigquery-emulator:local .
docker run --rm -p 9050:9050 bigquery-emulator:local &
curl -s localhost:9050/bigquery/v2/projects/test/datasets | jq .
```

## Done criteria

- `docker pull`-able image documented in README (or ghcr.io path).
- Tagged release produces GitHub release + binaries for 4 platforms.
- Quickstart in README works copy-paste on clean machine with Docker only.
- `--version` reports consistent semver across gateway and engine.

## Roadmap complete

All phases 0–9 implemented per [bigquery-emulator-roadmap-index_a1b2c3d4.plan.md](bigquery-emulator-roadmap-index_a1b2c3d4.plan.md). Continue with conformance expansion and open questions in ROADMAP.
