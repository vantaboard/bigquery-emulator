import { webEnv } from './env';

export type OffsetUnit = 'utf8' | 'utf16';

export interface SqlCapabilities {
    sqlTools: boolean;
    version: string;
    endpoints: string[];
    offsetUnits: string[];
}

export interface SqlDiagnostic {
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

export interface FormatRequest {
    sql: string;
    strict?: boolean;
    lineLengthLimit?: number;
    indentationSpaces?: number;
    offsetUnit?: OffsetUnit;
}

export interface FormatResponse {
    formattedSql: string;
    diagnostics: SqlDiagnostic[];
}

export interface ParseRequest {
    sql: string;
    offsetUnit?: OffsetUnit;
}

export interface ParseResponse {
    statementKinds: string[];
    diagnostics: SqlDiagnostic[];
}

export interface CompleteRequest {
    sql: string;
    cursorByteOffset: number;
    projectId?: string;
    defaultDatasetId?: string;
    offsetUnit?: OffsetUnit;
}

export interface CompletionCandidate {
    label: string;
    kind: string;
    insertText: string;
    detail?: string;
}

export interface CompleteResponse {
    candidates: CompletionCandidate[];
    replacementStart: number;
    replacementEnd: number;
}

export interface AnalyzeRequest {
    sql: string;
    projectId?: string;
    defaultDatasetId?: string;
    offsetUnit?: OffsetUnit;
}

export interface ReferencedTable {
    projectId: string;
    datasetId: string;
    tableId: string;
    alias?: string;
    kind: string;
}

export interface AnalyzeResponse {
    referencedTables: ReferencedTable[];
    statementKinds: string[];
    diagnostics: SqlDiagnostic[];
}

interface ProbeCache {
    available: boolean;
    capabilities?: SqlCapabilities;
}

let probeCache: ProbeCache | null = null;

export function resetSqlToolsProbe(): void {
    probeCache = null;
}

function sqlToolsToken(): string | undefined {
    return import.meta.env.VITE_SQL_TOOLS_TOKEN?.trim() || undefined;
}

export function sqlToolsHeaders(): Record<string, string> {
    const token = sqlToolsToken();
    if (!token) return {};
    return { 'X-BigQuery-Emulator-SqlTools-Token': token };
}

function url(path: string): string {
    const base = webEnv.apiBaseUrl;
    const p = path.startsWith('/') ? path : `/${path}`;
    return `${base}${p}`;
}

async function sqlToolsFetch<T>(path: string, init?: RequestInit): Promise<T> {
    const res = await fetch(url(path), {
        ...init,
        headers: {
            'Content-Type': 'application/json',
            ...sqlToolsHeaders(),
            ...init?.headers,
        },
    });
    const text = await res.text();
    const data = text ? (JSON.parse(text) as unknown) : null;
    if (!res.ok) {
        const message =
            data && typeof data === 'object' && data !== null && 'message' in data
                ? String((data as { message: unknown }).message)
                : `HTTP ${res.status}`;
        throw new Error(message);
    }
    return data as T;
}

export async function probeCapabilities(): Promise<boolean> {
    if (probeCache !== null) return probeCache.available;
    try {
        const capabilities = await sqlToolsFetch<SqlCapabilities>('/api/emulator/sql/capabilities', {
            method: 'GET',
        });
        probeCache = { available: capabilities.sqlTools === true, capabilities };
        return probeCache.available;
    } catch {
        probeCache = { available: false };
        return false;
    }
}

export async function getSqlToolsCapabilities(): Promise<SqlCapabilities | null> {
    const available = await probeCapabilities();
    return available ? (probeCache?.capabilities ?? null) : null;
}

export function isSqlToolsAvailable(): boolean {
    return probeCache?.available === true;
}

export async function formatSql(request: FormatRequest): Promise<FormatResponse> {
    return sqlToolsFetch<FormatResponse>('/api/emulator/sql/format', {
        method: 'POST',
        body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
}

export async function parseSql(request: ParseRequest): Promise<ParseResponse> {
    return sqlToolsFetch<ParseResponse>('/api/emulator/sql/parse', {
        method: 'POST',
        body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
}

export async function completeSql(request: CompleteRequest): Promise<CompleteResponse> {
    return sqlToolsFetch<CompleteResponse>('/api/emulator/sql/complete', {
        method: 'POST',
        body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
}

export async function analyzeSql(request: AnalyzeRequest): Promise<AnalyzeResponse> {
    return sqlToolsFetch<AnalyzeResponse>('/api/emulator/sql/analyze', {
        method: 'POST',
        body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
}

/** Convert a UTF-8 byte offset in `sql` to a UTF-16 code unit index (CodeMirror positions). */
export function utf8ByteOffsetToCodeUnit(sql: string, byteOffset: number): number {
    const bytes = new TextEncoder().encode(sql);
    const prefix = new TextDecoder().decode(bytes.slice(0, Math.max(0, byteOffset)));
    let units = 0;
    for (const char of prefix) {
        const code = char.codePointAt(0) ?? 0;
        units += code > 0xffff ? 2 : 1;
    }
    return units;
}

/** Convert a UTF-16 code unit index to a UTF-8 byte offset in `sql`. */
export function codeUnitToUtf8ByteOffset(sql: string, codeUnit: number): number {
    let units = 0;
    let i = 0;
    while (i < sql.length && units < codeUnit) {
        const code = sql.codePointAt(i) ?? 0;
        i += code > 0xffff ? 2 : 1;
        units += code > 0xffff ? 2 : 1;
    }
    return new TextEncoder().encode(sql.slice(0, i)).length;
}

export function completionKindToType(
    kind: string,
): 'text' | 'keyword' | 'function' | 'variable' | 'type' | 'property' | 'method' | 'class' {
    switch (kind) {
        case 'keyword':
            return 'keyword';
        case 'function':
            return 'function';
        case 'column':
            return 'property';
        case 'table':
        case 'view':
            return 'class';
        case 'dataset':
            return 'variable';
        case 'routine':
            return 'method';
        default:
            return 'text';
    }
}
