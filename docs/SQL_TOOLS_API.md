# SQL Tools API

The BigQuery emulator exposes GoogleSQL parser, formatter, tokenizer, and
catalog-aware completion helpers for downstream UIs and linters. These routes
are **not** part of the public BigQuery REST surface.

Enable them on the gateway:

```bash
./bin/gateway_main --enable-sql-tools-api
```

By default, only loopback callers may use the API. For remote access (CI, LAN
UI), combine `--sql-tools-api-allow-remote` with `--sql-tools-api-token` (or
`BIGQUERY_EMULATOR_SQL_TOOLS_TOKEN`).

## Endpoints

All routes accept `Content-Type: application/json` and return JSON.

| Method | Path | Purpose |
|--------|------|---------|
| POST | `/api/emulator/sql/format` | Pretty-print SQL |
| POST | `/api/emulator/sql/parse` | Parse-only diagnostics + statement kinds |
| POST | `/api/emulator/sql/tokenize` | Token stream with byte ranges |
| POST | `/api/emulator/sql/complete` | Catalog-aware autocompletion |

Errors use `{ "code": number, "status": "invalid", "message": "..." }`.

## Format

**Request**

```json
{
  "sql": "select 1",
  "strict": false,
  "lineLengthLimit": 80,
  "indentationSpaces": 2
}
```

- `strict: false` (default) uses the lenient formatter (preserves comments).
- `strict: true` uses strict `FormatSql` (strips comments).

**Response**

```json
{
  "formattedSql": "SELECT\n  1",
  "diagnostics": []
}
```

## Parse

**Request**

```json
{ "sql": "SELECT 1" }
```

**Response**

```json
{
  "statementKinds": ["QUERY_STATEMENT"],
  "diagnostics": []
}
```

## Tokenize

**Request**

```json
{
  "sql": "SELECT 1",
  "includeComments": false
}
```

**Response**

```json
{
  "tokens": [
    { "kind": "keyword", "image": "SELECT", "startByte": 0, "endByte": 6 },
    { "kind": "value", "image": "1", "startByte": 7, "endByte": 8 }
  ],
  "diagnostics": []
}
```

## Complete

Uses the live emulator catalog (datasets/tables from storage plus GoogleSQL
builtins/UDFs registered on the engine).

**Request**

```json
{
  "projectId": "local-project",
  "defaultDatasetId": "analytics",
  "sql": "SELECT * FROM ev",
  "cursorByteOffset": 18
}
```

**Response**

```json
{
  "candidates": [
    { "label": "events", "kind": "table", "insertText": "events" }
  ],
  "replacementStart": 16,
  "replacementEnd": 18
}
```

Candidate `kind` values include `keyword`, `function`, `dataset`, `table`, and
`column`.

## TypeScript shapes

```typescript
interface SqlDiagnostic {
  line: number;
  column: number;
  message: string;
  severity: string;
}

interface FormatRequest {
  sql: string;
  strict?: boolean;
  lineLengthLimit?: number;
  indentationSpaces?: number;
}

interface FormatResponse {
  formattedSql: string;
  diagnostics?: SqlDiagnostic[];
}

interface ParseRequest {
  sql: string;
}

interface ParseResponse {
  statementKinds: string[];
  diagnostics?: SqlDiagnostic[];
}

interface TokenizeRequest {
  sql: string;
  includeComments?: boolean;
}

interface SqlToken {
  kind: string;
  image: string;
  startByte: number;
  endByte: number;
}

interface TokenizeResponse {
  tokens: SqlToken[];
  diagnostics?: SqlDiagnostic[];
}

interface CompleteRequest {
  projectId: string;
  defaultDatasetId?: string;
  sql: string;
  cursorByteOffset: number;
}

interface CompletionCandidate {
  label: string;
  kind: string;
  insertText: string;
}

interface CompleteResponse {
  candidates: CompletionCandidate[];
  replacementStart: number;
  replacementEnd: number;
}
```

## Architecture

```
UI  --HTTP JSON-->  gateway /api/emulator/sql/*
       --gRPC-->    engine SqlTools service
                      --> backend/sqltools (GoogleSQL format/parse/tokenize)
                      --> GoogleSqlCatalog + storage (completion)
```

The C++ core in `backend/sqltools/` is intentionally storage-agnostic so it can
be reused in standalone tools or a future WASM build.
