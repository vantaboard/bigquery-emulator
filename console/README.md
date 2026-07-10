# BigQuery Emulator Console

React + TypeScript web console for exploring BigQuery (emulator or real), living in this monorepo as `console/`.

The console talks to the **BigQuery REST API** (`/bigquery/v2/*`) on port **9050** and, when SQL Tools is enabled, the **SQL Tools API** (`/api/emulator/sql/*`). The query editor uses **Monaco** with the BigQuery GoogleSQL **LSP** (`extensions/vscode/server`) running in a browser web worker for diagnostics, completion, formatting, and hover.

## Features

- Browse projects, datasets, and tables
- View table metadata and schema
- Run SQL with Monaco + LSP (production-style markers and Alt+F8 navigation)
- Format via LSP / SQL Tools with `sql-formatter` fallback
- Shareable URLs (query encoded in the query string)

See [docs/api-contract.md](./docs/api-contract.md) and [docs/rollout-checklist.md](./docs/rollout-checklist.md).

## Prerequisites

- **Node.js 22+** and **pnpm**
- Emulator binaries built in this repo (`task emulator:build-all`)
- **[direnv](https://direnv.net/)** (recommended)

## Quick start (from repo root)

```bash
pnpm install
task emulator:build-all   # once
task console:dev:all      # local emulator + Vite on :5173
```

Or separately:

```bash
task console:emulator:run   # terminal 1
task console:dev            # terminal 2
```

## Docker

```bash
docker compose --profile console up --build
```

Emulator on **9050**, console nginx UI on **8080**.

## Tests

```bash
task console:lint
task console:test
task console:e2e:local    # native emulator + Vite + Playwright
task console:e2e          # docker-compose.e2e stack
```

## Related packages

- LSP shared types/client: `extensions/vscode/shared` (`@bigquery-emulator/vscode-shared`)
- LSP server (node + browser): `extensions/vscode/server` (`@bigquery-emulator/vscode-server`)
- VS Code extension: `extensions/vscode/extension`
