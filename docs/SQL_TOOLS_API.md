# SQL Tools API

The BigQuery emulator exposes GoogleSQL parser, formatter, tokenizer, analyzer,
and catalog-aware completion helpers for downstream UIs and linters. These routes
are **not** part of the public BigQuery REST surface.

Enable them on the gateway:

```bash
./bin/gateway_main --enable-sql-tools-api
```

The Docker entrypoint shim (`docker/gateway_main.sh`) injects `--enable-sql-tools-api`
by default unless the caller already passed the flag.

By default, only loopback callers may use the API. For remote access (CI, LAN
UI), combine `--sql-tools-api-allow-remote` with `--sql-tools-api-token` (or
`BIGQUERY_EMULATOR_SQL_TOOLS_TOKEN`) and send the header
`X-BigQuery-Emulator-SqlTools-Token` on each request.

## Capabilities probe

**GET** `/api/emulator/sql/capabilities`

Returns `404` when SQL Tools is disabled (lets UIs fall back to client-side
parsing). When enabled:

```json
{
  "sqlTools": true,
  "version": "1.0",
  "endpoints": ["format", "parse", "tokenize", "complete", "analyze", "capabilities"],
  "offsetUnits": ["utf8", "utf16"]
}
```

## Endpoints

POST routes accept `Content-Type: application/json` and return JSON.

| Method | Path | Purpose |
|--------|------|---------|
| GET | `/api/emulator/sql/capabilities` | Feature probe + API version |
| POST | `/api/emulator/sql/format` | Pretty-print SQL |
| POST | `/api/emulator/sql/parse` | Parse-only diagnostics + statement kinds |
| POST | `/api/emulator/sql/tokenize` | Token stream with byte ranges |
| POST | `/api/emulator/sql/complete` | Catalog-aware autocompletion |
| POST | `/api/emulator/sql/analyze` | Referenced tables/views for a query |

### HTTP status codes

| Code | Meaning |
|------|---------|
| 200 | Success |
| 400 | Invalid JSON or SQL argument |
| 403 | Remote caller denied (loopback-only or bad token) |
| 404 | SQL Tools disabled (route not registered) |
| 503 | Engine SqlTools client unavailable |

Errors use `{ "code": number, "status": "invalid", "message": "..." }`.

## Offset units (UTF-8 vs UTF-16)

The engine uses **UTF-8 byte offsets** (0-based). CodeMirror default positions
are **UTF-16 code units** (0-based). Non-ASCII SQL will drift if you mix them.

All POST bodies accept an optional `"offsetUnit": "utf8" | "utf16"` (default
`"utf8"`).

When `"offsetUnit": "utf16"`:

- **Complete:** `cursorByteOffset` is interpreted as a UTF-16 code unit index;
  `replacementStart` / `replacementEnd` are returned as UTF-16 code units.
- **Parse / tokenize / format / analyze:** diagnostics and tokens include
  `startUtf16` / `endUtf16` alongside `startByte` / `endByte`.

### JavaScript helpers (direct engine / gRPC clients)

```javascript
function utf8ByteOffsetToCodeUnit(sql, byteOffset) {
  const prefix = sql.slice(0, byteOffset);
  let units = 0;
  for (const char of prefix) {
    const code = char.codePointAt(0);
    units += code > 0xffff ? 2 : 1;
  }
  return units;
}

function codeUnitToUtf8ByteOffset(sql, codeUnit) {
  let units = 0;
  for (let i = 0; i < sql.length && units < codeUnit; ) {
    const code = sql.codePointAt(i);
    i += code > 0xffff ? 2 : 1;
    units += code > 0xffff ? 2 : 1;
  }
  return [...sql.slice(0, i)].join("").length; // use slice by index i
}
```

Prefer the gateway `"offsetUnit": "utf16"` field when calling REST from a
CodeMirror-based UI.

## Live linting

Keystroke-level lint via `POST /parse` should be debounced (300–500 ms). Parsing
full multi-statement scripts on every key is expensive; a future enhancement may
accept `statementIndex` or `cursorByteOffset` to scope parse work.

## Format

**Request**

```json
{
  "sql": "select 1",
  "strict": false,
  "lineLengthLimit": 80,
  "indentationSpaces": 2,
  "offsetUnit": "utf8"
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
{ "sql": "SELECT 1", "offsetUnit": "utf8" }
```

**Response**

```json
{
  "statementKinds": ["QueryStatement"],
  "diagnostics": []
}
```

Diagnostics include optional span fields: `endLine`, `endColumn`, `startByte`,
`endByte` (and `startUtf16` / `endUtf16` when `offsetUnit` is `utf16`).

## Tokenize

**Request**

```json
{
  "sql": "SELECT 1",
  "includeComments": false,
  "offsetUnit": "utf8"
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

Uses the live emulator catalog (datasets, tables, views, routines, columns from
storage plus GoogleSQL builtins/UDFs registered on the engine). Empty `sql` is
allowed when `cursorByteOffset === 0` (blank editor tab).

**Request**

```json
{
  "projectId": "local-project",
  "defaultDatasetId": "analytics",
  "sql": "SELECT * FROM ev",
  "cursorByteOffset": 18,
  "offsetUnit": "utf8"
}
```

**Response**

```json
{
  "candidates": [
    {
      "label": "events",
      "kind": "table",
      "insertText": "events",
      "detail": "table"
    }
  ],
  "replacementStart": 16,
  "replacementEnd": 18
}
```

Candidate `kind` values include `keyword`, `function`, `dataset`, `table`,
`view`, `routine`, and `column`.

## Analyze

Extracts tables/views referenced by a query so UIs can fetch schemas via the
existing BigQuery REST surface without client-side parsing.

**Request**

```json
{
  "projectId": "local-project",
  "defaultDatasetId": "analytics",
  "sql": "SELECT * FROM analytics.events"
}
```

**Response**

```json
{
  "referencedTables": [
    {
      "projectId": "local-project",
      "datasetId": "analytics",
      "tableId": "events",
      "alias": "",
      "kind": "table"
    }
  ],
  "statementKinds": ["QueryStatement"],
  "diagnostics": []
}
```

## Dev proxy (Vite / nginx)

Point `/api/emulator` at the gateway alongside `/bigquery`:

```javascript
// vite.config.ts
export default defineConfig({
  server: {
    proxy: {
      "/bigquery": { target: "http://127.0.0.1:9050", changeOrigin: true },
      "/api/emulator": { target: "http://127.0.0.1:9050", changeOrigin: true },
    },
  },
});
```

For LAN/CI with `--sql-tools-api-allow-remote`, add the token header in your
fetch wrapper.

## TypeScript shapes

```typescript
type OffsetUnit = "utf8" | "utf16";

interface SqlDiagnostic {
  line: number;
  column: number;
  message: string;
  severity: string;
  endLine?: number;
  endColumn?: number;
  startByte?: number;
  endByte?: number;
  startUtf16?: number;
  endUtf16?: number;
}

interface OffsetRequest {
  offsetUnit?: OffsetUnit;
}

interface FormatRequest extends OffsetRequest {
  sql: string;
  strict?: boolean;
  lineLengthLimit?: number;
  indentationSpaces?: number;
}

interface CompletionCandidate {
  label: string;
  kind: string;
  insertText: string;
  detail?: string;
}

interface ReferencedTable {
  projectId: string;
  datasetId: string;
  tableId: string;
  alias?: string;
  kind: string;
}
```

## Architecture

```
UI  --HTTP JSON-->  gateway /api/emulator/sql/*
       --gRPC-->    engine SqlTools service
                      --> backend/sqltools (GoogleSQL format/parse/tokenize/analyze)
                      --> GoogleSqlCatalog + storage (completion + analyze)
```

The C++ core in `backend/sqltools/` is intentionally storage-agnostic so it can
be reused in standalone tools or a future WASM build.
