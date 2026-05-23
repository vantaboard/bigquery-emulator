---
name: docker-compose-smoke
overview: "Phase 9a: finalize Docker image, docker-compose, CI quickstart smoke."
todos:
  - id: docker-multistage
    content: "Finalize Dockerfile: build both binaries, ENTRYPOINT gateway, EXPOSE 9050, default --profile=ci"
    status: pending
  - id: compose-dev
    content: "Add docker-compose.yml: mount data_dir, BIGQUERY_EMULATOR_HOST, healthcheck"
    status: pending
  - id: quickstart-ci
    content: "CI smoke: docker run, curl datasets + SELECT 1, assert 200"
    status: pending
isProject: false
---

# Phase 9a: Docker Compose Smoke

## Prerequisites

- [bootstrap-docker_a0b1c2d3.plan.md](bootstrap-docker_a0b1c2d3.plan.md)
- [conformance-seed-docs_n1e2f3a4.plan.md](conformance-seed-docs_n1e2f3a4.plan.md)

## Verification

```bash
docker build -t bigquery-emulator:local . && docker compose up -d
```

## Done criteria

- Docker quickstart works copy-paste

## Next plan(s)

- [goreleaser-release_p3a4b5c6.plan.md](goreleaser-release_p3a4b5c6.plan.md)
