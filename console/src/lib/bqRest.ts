import type { QueryResponse, TableMetadata, TableSchemaField } from '@/types/api';

interface BqProjectList {
    projects?: Array<{ id?: string; projectReference?: { projectId?: string } }>;
}

interface BqDatasetList {
    datasets?: Array<{ datasetReference?: { datasetId?: string } }>;
}

interface BqTableList {
    tables?: Array<{ tableReference?: { tableId?: string } }>;
}

interface BqSchemaField {
    name?: string;
    type?: string;
    mode?: string;
    description?: string;
}

interface BqTable {
    schema?: { fields?: BqSchemaField[] };
    numRows?: string;
    numBytes?: string;
    creationTime?: string;
    lastModifiedTime?: string;
    description?: string;
    type?: string;
    location?: string;
}

interface BqQueryResponse {
    schema?: { fields?: BqSchemaField[] };
    rows?: Array<{ f?: Array<{ v?: unknown }> }>;
    totalRows?: string;
}

function parseBqValue(raw: unknown, type?: string): unknown {
    if (raw === null || raw === undefined) return null;
    const upper = (type ?? '').toUpperCase();
    if (upper.includes('INT') || upper === 'NUMERIC' || upper === 'BIGNUMERIC') {
        const n = Number(raw);
        return Number.isNaN(n) ? raw : n;
    }
    if (upper === 'FLOAT' || upper === 'FLOAT64') {
        const n = Number(raw);
        return Number.isNaN(n) ? raw : n;
    }
    if (upper === 'BOOLEAN' || upper === 'BOOL') {
        return raw === 'true' || raw === true;
    }
    return raw;
}

function bqTimestamp(ms?: string): string {
    if (!ms) return new Date(0).toISOString();
    const n = Number(ms);
    return Number.isNaN(n) ? new Date(0).toISOString() : new Date(n).toISOString();
}

export function projectIdsFromList(data: BqProjectList): string[] {
    return (data.projects ?? [])
        .map((p) => p.id ?? p.projectReference?.projectId)
        .filter((id): id is string => Boolean(id));
}

export function datasetIdsFromList(data: BqDatasetList): string[] {
    return (data.datasets ?? [])
        .map((d) => d.datasetReference?.datasetId)
        .filter((id): id is string => Boolean(id));
}

export function tableIdsFromList(data: BqTableList): string[] {
    return (data.tables ?? [])
        .map((t) => t.tableReference?.tableId)
        .filter((id): id is string => Boolean(id));
}

export function tableMetadataFromBq(
    projectId: string,
    datasetId: string,
    tableId: string,
    table: BqTable,
): TableMetadata {
    const schema: TableSchemaField[] = (table.schema?.fields ?? []).map((f) => ({
        name: f.name ?? '',
        type: f.type ?? 'STRING',
        mode: f.mode ?? 'NULLABLE',
        description: f.description ?? null,
    }));
    return {
        schema,
        numRows: Number(table.numRows ?? 0),
        numBytes: Number(table.numBytes ?? 0),
        creationTime: bqTimestamp(table.creationTime),
        lastModified: bqTimestamp(table.lastModifiedTime ?? table.creationTime),
        description: table.description ?? '',
        type: table.type ?? 'TABLE',
        location: table.location ?? '',
        fullyQualifiedName: `${projectId}.${datasetId}.${tableId}`,
    };
}

export function queryResponseFromBq(data: BqQueryResponse): QueryResponse {
    const fields = data.schema?.fields ?? [];
    const columns = fields.map((f) => f.name ?? '');
    const rows = (data.rows ?? []).map((row) => {
        const out: Record<string, unknown> = {};
        columns.forEach((col, i) => {
            const cell = row.f?.[i];
            out[col] = parseBqValue(cell?.v ?? null, fields[i]?.type);
        });
        return out;
    });
    return {
        columns,
        rows,
        total_rows: Number(data.totalRows ?? rows.length),
    };
}

export type { BqDatasetList, BqProjectList, BqQueryResponse, BqTable, BqTableList };
