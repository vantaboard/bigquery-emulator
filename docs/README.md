# Documentation

Guides for running, developing, and extending the BigQuery emulator.

## Getting started

| Guide | Contents |
|-------|----------|
| [Quickstart](https://github.com/vantaboard/bigquery-emulator/blob/main/README.md#quickstart) | Docker one-liner and local build/run (in the main README) |
| [Docker](./DOCKER.md) | `docker compose`, plain `docker run`, build notes |
| [Clients](./CLIENTS.md) | Point official client libraries at the emulator |
| [Releases](./RELEASES.md) | Tags, archives, GHCR images |

## Emulator behavior

| Guide | Contents |
|-------|----------|
| [REST API](./REST_API.md) | Endpoint → handler mapping and current status |
| [Engine policy](./ENGINE_POLICY.md) | Local-only execution policy and route catalog |
| [Seeding](./SEEDING.md) | Declarative YAML, templates, and seed REST API |
| [Runtime configuration](./DEVELOPMENT.md#runtime-configuration) | Engine and gateway flags |

## Development

| Guide | Contents |
|-------|----------|
| [Development setup](./DEVELOPMENT.md) | Toolchain, repo layout, building the engine |
| [C++ lint policy](./dev/cpp-lint.md) | Format, tidy, cppcheck gates |
| [GoogleSQL prebuilt](./dev/googlesql-prebuilt/README.md) | Prebuilt artifact cache, troubleshooting, maintainer runbooks |
| [Conformance](https://github.com/vantaboard/bigquery-emulator/blob/main/conformance/README.md) | YAML fixture runner and diff harness |
| [Third-party harnesses](https://github.com/vantaboard/bigquery-emulator/blob/main/third_party/README.md) | Client-library sample suites |

## Planning

| Guide | Contents |
|-------|----------|
| [ROADMAP](https://github.com/vantaboard/bigquery-emulator/blob/main/ROADMAP.md) | Capability-area plan and design rationale |
| [Benchmarks](https://github.com/vantaboard/bigquery-emulator/blob/main/bench/README.md) | Latency comparison harness (charts live in the main README) |
