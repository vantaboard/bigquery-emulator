# BigQuery GoogleSQL — VSCode / Cursor extension

Language support for GoogleSQL (BigQuery dialect) via a Language Server that talks
to:

- the **BigQuery emulator** SQL Tools API (`/api/emulator/sql/*` on port `9050`), or
- **production BigQuery** dry-run + REST catalog (Application Default Credentials).

## Features

- Live diagnostics (debounced parse / dry-run)
- Catalog-aware completion (tables, columns, routines, keywords, functions)
- Document formatting (SQL Tools or `sql-formatter` fallback)
- Quick fixes (missing delimiters, did-you-mean, keyword casing)
- Hover (function docs + table schemas via analyze + `tables.get`)
- TextMate grammar for `.bqsql` / `.googlesql`

## Prerequisites

### Emulator backend

Start the gateway with SQL Tools enabled:

```bash
./bin/gateway_main \
  --engine_binary=./bin/emulator_main \
  --enable-sql-tools-api \
  --http_port=9050
```

### Production backend

Set `bigquery.backendMode` to `bigquery` and authenticate with ADC:

```bash
gcloud auth application-default login
```

Configure `bigquery.projectId` to your GCP project.

## Development

```bash
cd extensions/vscode
npm install
npm run build
npm test
```

Launch the extension from `extensions/vscode/extension` using the VSCode
**Run Extension** debug configuration, or install the folder into your editor.

### Settings

| Setting | Default | Description |
|---------|---------|-------------|
| `bigquery.backendMode` | `auto` | `auto`, `emulator`, or `bigquery` |
| `bigquery.emulatorBaseUrl` | `http://127.0.0.1:9050` | Emulator gateway base URL |
| `bigquery.sqlToolsToken` | `""` | Optional SQL Tools token header |
| `bigquery.projectId` | `local-project` | Project for completion / analysis |
| `bigquery.defaultDatasetId` | `""` | Default dataset for unqualified names |
| `bigquery.strictFormat` | `false` | Strict SQL Tools formatter |
| `bigquery.formatIndentationSpaces` | `2` | Formatter indent width |
| `bigquery.formatLineLengthLimit` | `80` | Formatter line length |

## Parity testing

Production error strings are captured in
[`test/parity/errors.yaml`](test/parity/errors.yaml) (for example the console
message `Syntax error: Expected ")" but got end of script at [1:17]` for
`SELECT SAFE_ADD(`). Run:

```bash
npm run test --workspace=@bigquery-emulator/vscode-test
```

## Architecture

```
extension (client) --stdio--> server (LSP)
                                 ├─ SqlToolsBackend → /api/emulator/sql/*
                                 └─ BigQueryBackend → jobs.query dryRun + REST catalog
```

See also [`docs/SQL_TOOLS_API.md`](../../docs/SQL_TOOLS_API.md).
